//
//		spoutDX12.cpp
//
//		Functions to manage DirectX 12 texture sharing by way of D3D11On12
//		Base class spoutDX for sending and receiving functions.
//
// ====================================================================================
//		Revisions :
//		04.12.20	- start class
//		26.01.21	- First working version
//
// ====================================================================================
/*

	Copyright (c) 2014-2021. Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "SpoutDX12.h"

spoutDX12::spoutDX12() {

	m_pd3dDevice12 = nullptr;// D3D12 device
	m_pd3dDevice11 = nullptr; // D3D11 device
	m_pd3dDeviceContext11 = nullptr; // D3D11 context
	m_pd3d11On12Device = nullptr; // D3D11on12 device
	m_bClassDevice = false; // External or class D3D12 device
	m_pReceivedResource11 = nullptr; // The wrapped D3D11 resource for D3D12

}

spoutDX12::~spoutDX12() {

	if (m_pd3dDevice12) m_pd3dDevice12->Release();
	if (m_pd3dDevice11) m_pd3dDevice11->Release();
	if (m_pd3dDeviceContext11) m_pd3dDeviceContext11->Release();
	if (m_pd3d11On12Device) m_pd3d11On12Device->Release();

}

bool spoutDX12::OpenDirectX12()
{
	return OpenDirectX12(nullptr, nullptr);
}

bool spoutDX12::OpenDirectX12(ID3D12Device* pd3dDevice12, IUnknown** ppCommandQueue)
{
	if (!m_pd3dDevice12) {
		if (pd3dDevice12) {
			SpoutLogNotice("spoutDX12::OpenDirectX12(0x%.7X)", PtrToUint(pd3dDevice12));
			m_pd3dDevice12 = pd3dDevice12;
			// Create a class 11on12 device using the dx12 device and command queue
			if (!CreateDX11on12device(m_pd3dDevice12, ppCommandQueue)) {
				SpoutLogWarning("spoutDX12::OpenDirectX12 - CreateDX11on12device 1 failed");
				CloseDirectX12();
				return false;
			}
			m_bClassDevice = false; // An existing device pointer was used
		}
		else {
			// Create a class DirectX 12 device
			m_pd3dDevice12 = CreateDX12device();
			if (!m_pd3dDevice12) {
				SpoutLogWarning("spoutDX12::OpenDirectX12 - Could not create DX12 device");
				return false;
			}
			// Create a class 11on12 device using the class dx12 device and no command queue
			if (!CreateDX11on12device(m_pd3dDevice12)) {
				CloseDirectX12();
				SpoutLogWarning("spoutDX12::OpenDirectX12 - CreateDX11on12device 2 failed");
				return false;
			}
			SpoutLogNotice("spoutDX12::OpenDirectX12() - (%.7X)", PtrToUint(pd3dDevice12));
		}
	}
	else {
		SpoutLogNotice("spoutDX12::OpenDirectX12 device already created - (%.7X)", PtrToUint(pd3dDevice12));
	}

	// Initialize DirectX11 using the D3D11 device created by CreateDX11on12device
	// so that textures are created with that device and can be copied.
	OpenDirectX11(m_pd3dDevice11);

	return true;
}


void spoutDX12::CloseDirectX12()
{
	SpoutLogNotice("spoutDX12::CloseDirectX12()");

	// Quit if already released
	if (!m_pd3dDevice12) {
		SpoutLogNotice("    Device already released");
		return;
	}

	// Release class D3D11on12
	if (m_pd3dDevice11) m_pd3dDevice11->Release(); // D3D11 device
	if (m_pd3dDeviceContext11) m_pd3dDeviceContext11->Release(); // D3D11 context
	if (m_pd3d11On12Device) m_pd3d11On12Device->Release(); // D3D11on12 device
	m_pd3dDevice11 = nullptr;
	m_pd3dDeviceContext11 = nullptr;
	m_pd3d11On12Device = nullptr;

	// Release class D3D12
	if(m_pd3dDevice12) m_pd3dDevice12->Release();
	m_pd3dDevice12 = nullptr;

	// Release D3D11
	CloseDirectX11();

}

// Send wrapped D3D11on12 D3D11 texture resource
bool spoutDX12::SendDX11Resource(ID3D11Resource *pWrappedResource)
{
	if (!pWrappedResource || !m_pd3d11On12Device || !m_pd3dDeviceContext11)
		return false;

	m_pd3d11On12Device->AcquireWrappedResources(&pWrappedResource, 1);
	// SendTexture handles sender creation and resizing
	bool bRet = SendTexture((ID3D11Texture2D*)pWrappedResource);
	m_pd3d11On12Device->ReleaseWrappedResources(&pWrappedResource, 1);
	// Flush to submit the 11 command list to the shared command queue.
	m_pd3dDeviceContext11->Flush();

	return bRet;

}


// Receive a texture from a sender to a D3D12 texture resource
bool spoutDX12::ReceiveDX12Resource(ID3D12Resource** ppDX12Resource)
{
	// Return if flagged for update
	// The update flag is reset when the receiving application calls IsUpdated()
	if (m_bUpdated)
		return true;

	// Try to receive texture details from a sender
	if (ReceiveSenderData()) {

		// Was the shared texture pointer retrieved ?
		if (!m_pSharedTexture) {
			return false;
		}

		// The sender name, width, height, format, shared texture handle and pointer have been retrieved.
		if (m_bUpdated) {

			// If the sender is new or changed, return to update the receiving D3D12 texture resource.
			// The application detects the change with IsUpdated().

			// Release the wrapped 11On12 D3D11 resource because
			// it has to be re-created for the new receiving texture
			if (m_pReceivedResource11)
				m_pReceivedResource11->Release();
			m_pReceivedResource11 = nullptr;
			return true;
		}

		// The application receiving texture is created
		// by the application on the first update above
		// ready for copy from the sender's shared texture.
		ID3D12Resource* pDX12Resource = *ppDX12Resource;
		if (!pDX12Resource) {
			// SpoutLog
			return false;
		}

		// Is a wrapped resource created yet?
		if (!m_pReceivedResource11) {
			if (!WrapDX12Resource(pDX12Resource, &m_pReceivedResource11)) {
				// SpoutLog
				return false;
			}
		}


		//
		// Found a sender
		//

		// Access the sender shared texture
		if (frame.CheckTextureAccess(m_pSharedTexture)) {
			m_bNewFrame = false; // For query of new frame
			// Check if the sender has produced a new frame.
			if (frame.GetNewFrame()) {
				// Copy from the sender's shared texture to the wrapped receiving texture.
				UpdateWrappedResource(m_pReceivedResource11, m_pSharedTexture);
				m_bNewFrame = true; // The application can query IsNewFrame()
			}
		}
		// Allow access to the shared texture
		frame.AllowTextureAccess(m_pSharedTexture);

		m_bConnected = true;

	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// Release the wrapped 11On12 D3D11 resource
		if (m_pReceivedResource11)
			m_pReceivedResource11->Release();
		m_pReceivedResource11 = nullptr;
		// Let the application know.
		m_bConnected = false;
	}

	// ReceiveTexture fails if there is no sender or the connected sender closed.
	return m_bConnected;

}


// Create a D3D11on12 device
ID3D11On12Device* spoutDX12::CreateDX11on12device(ID3D12Device* pDevice12, IUnknown** ppCommandQueue)
{
	ID3D11Device* pDevice11 = nullptr;
	ID3D11DeviceContext* pContext11 = nullptr;
	ID3D11On12Device* pd3d11On12Device = nullptr; // D3D11on12 device

	// Only allowing for one queue
	int numQueues = 0;
	if (ppCommandQueue)
		numQueues = 1;

	// Release the class device
	if (m_pd3d11On12Device)
		m_pd3d11On12Device->Release();
	m_pd3d11On12Device = nullptr;

	UINT DeviceFlags11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	HRESULT hRes = D3D11On12CreateDevice(
		pDevice12, // DX12 device
		DeviceFlags11, // FLAGS
		nullptr, // Feature level table
		0, // Feature levels
		ppCommandQueue, // command queues
		numQueues, // Num queues
		0, // NodeMask
		&pDevice11, // D3D11 device
		&pContext11, // D3D11 context
		nullptr
	);

	if (FAILED(hRes)) {
		SpoutLogError("spoutDX12::CreateDX11on12device fail");
		return false;
	}

	// Grab interface to the d3d11on12 device from the newly created d3d11 device
	hRes = pDevice11->QueryInterface(__uuidof(ID3D11On12Device), (void**)&pd3d11On12Device);
	if (FAILED(hRes)) {
		SpoutLogError("failed to query 11on12 device");
		return false;
	}

	SpoutLogNotice("spoutDX12::CreateDX11on12device - (0x%.7X)", PtrToUint(pd3d11On12Device));

	// Update globals
	m_pd3d11On12Device = pd3d11On12Device;
	m_pd3dDevice11 = pDevice11;
	m_pd3dDeviceContext11 = pContext11;

	return pd3d11On12Device;

}

// Wrap a D3D12 resource for use with D3D11
bool spoutDX12::WrapDX12Resource(ID3D12Resource* pDX12Resource, ID3D11Resource** ppWrapped11Resource)
{
	HRESULT hr = S_OK;

	if (!m_pd3d11On12Device || !pDX12Resource || !ppWrapped11Resource)
		return false;

	// A D3D11_RESOURCE_FLAGS structure that enables an application to override flags
	// that would be inferred by the resource/heap properties.
	// The D3D11_RESOURCE_FLAGS structure contains bind flags, misc flags, and CPU access flags.
	D3D11_RESOURCE_FLAGS d3d11Flags = {}; // { D3D11_BIND_RENDER_TARGET };

	// Create a wrapped resource to access our d3d12 resource from the d3d11 device
	// Note: D3D12_RESOURCE_STATE variables are: 
	//    (1) the state of the d3d12 resource when we acquire it
	//        (when the d3d12 pipeline is finished with it and we are ready to use it in d3d11)
	//    (2) when we are done using it in d3d11 (we release it back to d3d12) 
	//        these are the states our resource will be transitioned into
	hr = m_pd3d11On12Device->CreateWrappedResource(
							pDX12Resource, // A pointer to an already-created D3D12 resource or heap.
							&d3d11Flags, 
							D3D12_RESOURCE_STATE_COPY_DEST, // D3D12_RESOURCE_STATE_RENDER_TARGET, // InState
							D3D12_RESOURCE_STATE_PRESENT, // OutState
							IID_PPV_ARGS(ppWrapped11Resource)); // Lazy

	if (FAILED(hr)) {
		SpoutLogError("spoutDX12::WrapDX12Resource - failed to create wrapped resource (%d 0x%.7X)", LOWORD(hr), UINT(hr));
		// TODO - error reports
		// E_INVALIDARG 0x57
		// E_NOTIMPL 0x4001
		// DXGI_ERROR_INVALID_CALL 0xa001
		// D3DERR_INVALIDCALL
		return false;
	}

	SpoutLogNotice("spoutDX12::WrapDX12Resource (0x%.7X, 0x%.7X)", PtrToUint(pDX12Resource), PtrToUint(*ppWrapped11Resource));

	return true;
}


// Update a wrapped D3D11 texture resource with a D3D11 texture
void spoutDX12::UpdateWrappedResource(ID3D11Resource* pWrappedResource, ID3D11Resource *pResource)
{
	if (!pResource || !pWrappedResource || !m_pd3d11On12Device || !m_pd3dDeviceContext11)
		return;

	m_pd3d11On12Device->AcquireWrappedResources(&pWrappedResource, 1);
	m_pd3dDeviceContext11->CopyResource(pWrappedResource, pResource);
	m_pd3d11On12Device->ReleaseWrappedResources(&pWrappedResource, 1);
	m_pd3dDeviceContext11->Flush();
}


// Create a D3D12 texture resource
bool spoutDX12::CreateDX12texture(ID3D12Device* pd3dDevice12, 
										unsigned int width, 
										unsigned int height,
										D3D12_RESOURCE_STATES InitialState,
										DXGI_FORMAT *format, 
										ID3D12Resource** ppTexture)
{
	if (!pd3dDevice12) {
		SpoutLogFatal("spoutDX12::CreateSharedDX12Texture NULL device");
		return false;
	}

	if (!ppTexture) {
		SpoutLogWarning("spoutDX12::CreateDX12Texture NULL texture pointer");
		return false;
	}
	SpoutLogNotice("spoutDX12::CreateDX12Texture");

	ID3D12Resource* pTexture12 = *ppTexture;

	// Release the texture if it already exists
	if (pTexture12)
		pTexture12->Release();

	// Use the format passed in. If that is zero or DX9, use the default format.
	DXGI_FORMAT texformat = DXGI_FORMAT_B8G8R8A8_UNORM;
	if (*format != 0 && *format != 21) // D3DFMT_A8R8G8B8 = 21
		texformat = *format;

	// Describe and create a Texture2D
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = texformat;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	DX12_HEAP_PROPERTIES heapprop = DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	try {
		pd3dDevice12->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			InitialState,
			nullptr,
			IID_PPV_ARGS(&pTexture12));
	}
	catch (...) {
		SpoutLogError("spoutDX12::CreateDX12Texture - exception");
		return false;
	}

	SpoutLogNotice("spoutDX12::CreateDX12Texture - 0x%.7X", PtrToUint(pTexture12));

	*format = texformat;
	*ppTexture = pTexture12;

	return true;

}

// D3D12 device
ID3D12Device* spoutDX12::GetD3D12device()
{
	return m_pd3dDevice12;
}

// D3D11on12 D3D11 device
ID3D11Device* spoutDX12::GetD3D11device()
{
	return m_pd3dDevice11;
}

// D3D11on12 D3D11 context
ID3D11DeviceContext* spoutDX12::GetD3D11context()
{
	return m_pd3dDeviceContext11;
}

// D3D11on12 device
ID3D11On12Device* spoutDX12::GetD3D11On12device()
{
	return m_pd3d11On12Device;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Private functions
//

// Create DX12 device
ID3D12Device* spoutDX12::CreateDX12device()
{

	ID3D12Device* pd3dDevice12 = nullptr;
	HRESULT hRes = S_OK;

	IDXGIFactory1* factory1 = nullptr;
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory1)))
		return false;

	// Release the class device
	if (m_pd3dDevice12)
		m_pd3dDevice12->Release();
	m_pd3dDevice12 = nullptr;

	IDXGIAdapter1* hardwareAdapter = nullptr;
	GetHardwareAdapter(factory1, &hardwareAdapter);
	factory1->Release();

	DXGI_ADAPTER_DESC desc;
	hardwareAdapter->GetDesc(&desc);
	// printf("hardwareAdapter : %S\n", desc.Description);

	hRes = D3D12CreateDevice(hardwareAdapter,
		D3D_FEATURE_LEVEL_11_0,
		_uuidof(ID3D12Device),
		(void **)&pd3dDevice12);

	if (FAILED(hRes)) {
		SpoutLogError("spoutDX12::CreateDX12device fail");
		return false;
	}
	SpoutLogNotice("spoutDX12::CreateDX12device - 0x%.7X", PtrToUint(pd3dDevice12));

	// Set the class device
	m_pd3dDevice12 = pd3dDevice12;

	return pd3dDevice12;

} // end CreateDX12device


// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
// _Use_decl_annotations_
// Copied from Microsoft examples.
void spoutDX12::GetHardwareAdapter(IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	IDXGIAdapter1* adapter;
	IDXGIFactory6* factory6;

	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			// printf("adapter %d : %S\n", adapterIndex, desc.Description);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}
	else
	{
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			HRESULT hr = D3D12CreateDevice(adapter,
				D3D_FEATURE_LEVEL_11_0,
				_uuidof(ID3D12Device),
				nullptr);

			if (SUCCEEDED(hr))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter;

}
