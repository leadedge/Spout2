//
//
//			spoutDirectX.cpp
//
//		DirectX functions to manage DirectX9 and DirectX11 texture sharing
//
// ====================================================================================
//		Revisions :
//
//		22.07.14	- added option for DX9 or DX11
//		21.09.14	- included keyed mutex texture access lock in CreateSharedDX11Texture
//		23.09.14	- moved general mutex texture access lock to this class
//		23.09.14	- added DX11available() to verify operating system support for DirectX 11
//		15.10.14	- added debugging aid for texture access locks
//		17.10.14	- flush before release immediate context in CloseDX11
//		21.10.14	- removed keyed mutex lock due to reported driver problems
//					  TODO - cleanup all functions using it
//		10.02.15	- removed functions relating to DirectX 11 keyed mutex lock
//		14.02.15	- added UNREFERENCED_PARAMETER(pSharedTexture) to CheckAceess and AllowAccess
//		29.05.15	- Included SetAdapter for multiple adapters - Franz Hildgen.
//		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
//		08.06.15	- removed dx9 flag from setadapter
//		04.08.15	- cleanup
//		11.08.15	- removed GetAdapterName return if Intel. For use with Intel HD4400/5000 graphics
//		22.10.15	- removed DX11available and shifted it to the interop class
//		19.11.15	- fixed return value in CreateDX9device after caps error (was false instead of NULL)
//		20.11.15	- Registry read/write moved from SpoutGLDXinterop class
//		16.02.16	- IDXGIFactory release - from https://github.com/jossgray/Spout2
//		29.02.16	- cleanup
//		05.04.16	- removed unused texture pointer from mutex access functions
//		16.06.16	- fixed null device release in SetAdapter - https://github.com/leadedge/Spout2/issues/17
//		01.07.16	- restored hFocusWindow in CreateDX9device (was set to NULL for testing)
//		04.09.16	- Add create DX11 staging texture
//		16.01.17	- Add WriteDX9surface
//		23.01.17	- pEventQuery->Release() for writeDX9surface
//		02.06.17	- Registry functions moved to SpoutUtils
//					- Added Spout error log console output
//		17.03.18	- More error log notices
//					- protect against an adapter with no outputs for SetAdapter
//		16.06.18	- change all class variable name prefix from g_ to m_
//					  Add GetImmediateContext();
//					- Add ReleaseDX11Texture and ReleaseDX11Device
//		13.11.18	- Remove staging texture functons
//		16.12.18	- Move FlushWait from interop class
//		03.01.19	- Changed to revised registry functions in SpoutUtils
//		27.06.19	- Restored release of existing texture in CreateSharedDX11Texture
//		03.07.19	- Added pointer checks in OpenDX11shareHandle
//		18.09.19	- Changed initial log from notice to to verbose
//					  for CreateSharedDX9Texture and CreateSharedDX11Texture
//		08.11.19	- removed immediate context check from OpenDX11shareHandle
//
// ====================================================================================
/*

	Copyright (c) 2014-2020. Lynn Jarvis. All rights reserved.

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

#include "spoutDirectX.h"

spoutDirectX::spoutDirectX() {

	// DX11
	m_pImmediateContext = NULL;
	m_driverType		= D3D_DRIVER_TYPE_NULL;
	m_featureLevel		= D3D_FEATURE_LEVEL_11_0;

	// Output graphics adapter
	// Programmer can set for an application
	m_AdapterIndex  = D3DADAPTER_DEFAULT; // DX9
	m_pAdapterDX11  = nullptr; // DX11
}

spoutDirectX::~spoutDirectX() {

}

//
// =========================== DX9 ================================
//

// Create a DX9 object
IDirect3D9Ex* spoutDirectX::CreateDX9object()
{
	IDirect3D9Ex* pD3D;

	HRESULT res = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D);
	if ( res != D3D_OK ) return NULL;

	return pD3D;
}

// Create a DX9 device
IDirect3DDevice9Ex* spoutDirectX::CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd)
{
	IDirect3DDevice9Ex* pDevice;
    D3DPRESENT_PARAMETERS d3dpp;
	D3DCAPS9 d3dCaps;
	int AdapterIndex = m_AdapterIndex;

	// SpoutLogNotice("spoutDirectX::CreateDX9device - adapter = %d, hWnd = 0x%x", AdapterIndex, hWnd);

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed		= true;						// windowed and not full screen
    d3dpp.SwapEffect	= D3DSWAPEFFECT_DISCARD;	// discard old frames
    d3dpp.hDeviceWindow	= hWnd;						// set the window to be used by D3D

	// D3DFMT_UNKNOWN can be specified for the BackBufferFormat while in windowed mode. 
	// This tells the runtime to use the current display-mode format and eliminates
	// the need to call GetDisplayMode. 
	d3dpp.BackBufferFormat		 = D3DFMT_UNKNOWN;

	// Set a dummy resolution - we don't render anything
    d3dpp.BackBufferWidth		 = 1920;
    d3dpp.BackBufferHeight		 = 1080;
	d3dpp.EnableAutoDepthStencil = false;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.BackBufferCount		 = 1;

	// Test for hardware vertex processing capability and set up as needed
	// D3DCREATE_MULTITHREADED required by interop spec
	if(pD3D->GetDeviceCaps( AdapterIndex, D3DDEVTYPE_HAL, &d3dCaps) != S_OK ) {
		SpoutLogFatal("spoutDirectX::CreateDX9device - GetDeviceCaps error");
		return NULL;
	}

	// | D3DCREATE_NOWINDOWCHANGES
	DWORD dwBehaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED; 
	if ( d3dCaps.VertexProcessingCaps != 0 )
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Create a DirectX9 device - we use directx only for accessing the handle
	// LJ notes - hwnd seems to have no effect - maybe because we do not render anything.
	// Note here that we are setting up for Windowed mode but it seems not to be affected
	// by fullscreen, probably because we are not rendering to it.
	HRESULT res = pD3D->CreateDeviceEx(	AdapterIndex,   // D3DADAPTER_DEFAULT
								D3DDEVTYPE_HAL, // Hardware rasterization. 
								hWnd,			// hFocusWindow (can be NULL)
								dwBehaviorFlags,
								&d3dpp,			// d3dpp.hDeviceWindow should be valid if hFocusWindow is NULL
								NULL,			// pFullscreenDisplayMode must be NULL for windowed mode
								&pDevice);
	
	if ( res != D3D_OK ) {
		SpoutLogFatal("spoutDirectX::CreateDX9device - CreateDeviceEx returned error %d (%x)", res, res);
		return NULL;
	}

	return pDevice;

} // end CreateDX9device


// Create a shared DirectX9 texture
// by giving it a sharehandle variable - dxShareHandle
// For a SENDER : the sharehanlde is NULL and a new texture is created
// For a RECEIVER : the sharehandle is valid and a handle to the existing shared texture is created
bool spoutDirectX::CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle)
{
	if (!pDevice) {
		SpoutLogError("spoutDirectX::CreateSharedDX9Texture - NULL DX9 device");
		return false;
	}

	if(dxTexture != NULL) dxTexture->Release();

	HRESULT res = pDevice->CreateTexture(width,
										 height,
										 1,
										 D3DUSAGE_RENDERTARGET, 
										 format,	// default is D3DFMT_A8R8G8B8 - may be set externally
										 D3DPOOL_DEFAULT,	// Required by interop spec
										 &dxTexture,
										 &dxShareHandle);	// local share handle to allow type casting for 64bit

	// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
	// USAGE, format and size for sender and receiver must all match
	if ( res != D3D_OK ) {
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDirectX::CreateSharedDX9Texture error (%x) - ", res);
		switch (res) {
			case D3DERR_INVALIDCALL:
				strcat_s(tmp, 256, "D3DERR_INVALIDCALL");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				strcat_s(tmp, 256, "D3DERR_OUTOFVIDEOMEMORY");
				break;
			case E_OUTOFMEMORY:
				strcat_s(tmp, 256, "E_OUTOFMEMORY");
				break;
			default :
				strcat_s(tmp, 256, "Unknown error");
				break;
		}
		SpoutLogFatal("%s", tmp);
		return false;
	}

	return true;

} // end CreateSharedDX9Texture


bool spoutDirectX::WriteDX9memory(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 source_surface, LPDIRECT3DTEXTURE9 dxTexture)
{
	IDirect3DSurface9* texture_surface = NULL;
	IDirect3DQuery9* pEventQuery = NULL;
	HRESULT hr = 0;
	hr = dxTexture->GetSurfaceLevel(0, &texture_surface); // shared texture surface
	if (SUCCEEDED(hr)) {
		// UpdateSurface
		// https://msdn.microsoft.com/en-us/library/windows/desktop/bb205857%28v=vs.85%29.aspx
		//    The source surface must have been created with D3DPOOL_SYSTEMMEM.
		//    The destination surface must have been created with D3DPOOL_DEFAULT.
		//    Neither surface can be locked or holding an outstanding device context.
		hr = pDevice->UpdateSurface(source_surface, NULL, texture_surface, NULL);
		if (SUCCEEDED(hr)) {
			// It is necessary to flush the command queue 
			// or the data is not ready for the receiver to read.
			// Adapted from : https://msdn.microsoft.com/en-us/library/windows/desktop/bb172234%28v=vs.85%29.aspx
			// Also see : http://www.ogre3d.org/forums/viewtopic.php?f=5&t=50486
			pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);
			if (pEventQuery != NULL) {
				pEventQuery->Issue(D3DISSUE_END);
				while (S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH));
				pEventQuery->Release(); // Must be released or causes a leak and reference count increment
			}
			return true;
		}
	}

	SpoutLogError("spoutDirectX::WriteDX9surface(%d, %d, %d) failed", pDevice, dxTexture, source_surface);

	return false;
} // end WriteDX9surface

//
// COPY FROM A GPU DX9 SURFACE TO THE SHARED DX9 TEXTURE
//
//    The source surface must have been created using the same device as the texture
//
bool spoutDirectX::WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture)
{
	IDirect3DSurface9* texture_surface = NULL;
	HRESULT hr = dxTexture->GetSurfaceLevel(0, &texture_surface); // destination texture surface
	if (SUCCEEDED(hr)) {
		// StretchRect is a GPU copy
		hr = pDevice->StretchRect(surface, NULL, texture_surface, NULL, D3DTEXF_NONE);
		if(SUCCEEDED(hr)) {
			// It is necessary to flush the command queue
			// or the data is not ready for the receiver to read.
			// Adapted from : https://msdn.microsoft.com/en-us/library/windows/desktop/bb172234%28v=vs.85%29.aspx
			// Also see : http://www.ogre3d.org/forums/viewtopic.php?f=5&t=50486
			IDirect3DQuery9* pEventQuery = NULL;
			pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery) ;
			if(pEventQuery!=NULL) {
				pEventQuery->Issue(D3DISSUE_END) ;
				while(S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH)) ;
				pEventQuery->Release(); // Must be released or causes a leak and reference count increment
			}
			return true;
		}
	}

	SpoutLogError("spoutDirectX::WriteDX9surface(0x%x, 0x%x) failed", surface, dxTexture);

	return false;
}


// =========================== end DX9 =============================


//
// =========================== DX11 ================================
//

//
// Notes for DX11 : https://www.opengl.org/registry/specs/NV/DX_interop2.txt
//
// Valid device types for the <dxDevice> parameter of wglDXOpenDeviceNV and associated restrictions
// DirectX device type ID3D11Device - can only be used on WDDM operating systems; XXX Must be multithreaded
// TEXTURE_2D - ID3D11Texture2D - Usage flags must be D3D11_USAGE_DEFAULT
// wglDXSetResourceShareHandle does not need to be called for DirectX version
// 10 and 11 resources. Calling this function for DirectX 10 and 11 resources
// is not an error but has no effect.

// Create DX11 device
ID3D11Device* spoutDirectX::CreateDX11device()
{
	ID3D11Device* pd3dDevice = NULL;
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;
	IDXGIAdapter* pAdapterDX11 = m_pAdapterDX11;

	// SpoutLogNotice("spoutDirectX::CreateDX11device - pAdapterDX11 (%d)", m_pAdapterDX11);

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// GL/DX interop Spec
	// ID3D11Device can only be used on WDDM operating systems : Must be multithreaded
	// D3D11_CREATE_DEVICE_FLAG createDeviceFlags
	D3D_DRIVER_TYPE driverTypes[] =	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	// These are the feature levels that we will accept.
	// m_featureLevel is the feature level used
	// 11.0 = 0xb000
	// 11.1 = 0xb001
	// TODO - check for 11.1 and multiple passes if feature level fails
	D3D_FEATURE_LEVEL featureLevels[] =	{
		// D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	// To allow for multiple graphics cards we will use m_pAdapterDX11
	// Which is set by SetAdapter before initializing DirectX

	if(pAdapterDX11) {
			hr = D3D11CreateDevice( pAdapterDX11,
									D3D_DRIVER_TYPE_UNKNOWN,
									NULL,
									createDeviceFlags,
									featureLevels,
									numFeatureLevels,
									D3D11_SDK_VERSION,
									&pd3dDevice,
									&m_featureLevel,
									&m_pImmediateContext );
	} // endif adapter set
	else {
		
		// Possible Optimus problem : is the default adapter (NULL) always Intel ?
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476082%28v=vs.85%29.aspx
		// pAdapter : a pointer to the video adapter to use when creating a device. 
		// Pass NULL to use the default adapter, which is the first adapter that is
		// enumerated by IDXGIFactory1::EnumAdapters. 
		// http://www.gamedev.net/topic/645920-d3d11createdevice-returns-wrong-feature-level/

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ ) {

			m_driverType = driverTypes[driverTypeIndex];

			hr = D3D11CreateDevice(	NULL,
									m_driverType,
									NULL,
									createDeviceFlags,
									featureLevels,
									numFeatureLevels,
									D3D11_SDK_VERSION, 
									&pd3dDevice,
									&m_featureLevel,
									&m_pImmediateContext);

			// Break as soon as something passes
			if(SUCCEEDED(hr))
				break;
		}
	} // endif no adapter set
	
	// Quit if nothing worked
	if (FAILED(hr)) {
		SpoutLogFatal("spoutDirectX::CreateDX11device NULL device");
		return NULL;
	}

	// All OK

	return pd3dDevice;

} // end CreateDX11device


bool spoutDirectX::CreateSharedDX11Texture(ID3D11Device* pd3dDevice, 
											unsigned int width, 
											unsigned int height, 
											DXGI_FORMAT format, 
											ID3D11Texture2D** pSharedTexture,
											HANDLE &dxShareHandle)
{
	ID3D11Texture2D* pTexture;
	
	if (pd3dDevice == NULL) {
		SpoutLogFatal("spoutDirectX::CreateSharedDX11Texture NULL device");
		return false; // 12.06.18
	}

	//
	// Create a new shared DX11 texture
	//

	pTexture = *pSharedTexture; // The texture pointer

	// 27.06.19
	// Release the texture if it already exists
	if (pTexture) ReleaseDX11Texture(pd3dDevice, pTexture);

	// SpoutLogVerbose("spoutDirectX::CreateSharedDX11Texture");
	// SpoutLogVerbose("    pDevice = 0x%Ix, width = %d, height = %d, format = %d", (intptr_t)pd3dDevice, width, height, format);

	// Textures being shared from D3D9 to D3D11 have the following restrictions (LJ - D3D11 to D3D9 ?).
	//		Textures must be 2D
	//		Only 1 mip level is allowed
	//		Texture must have default usage
	//		Texture must be write only	- ?? LJ ??
	//		MSAA textures are not allowed
	//		Bind flags must have SHADER_RESOURCE and RENDER_TARGET set
	//		Only R10G10B10A2_UNORM, R16G16B16A16_FLOAT and R8G8B8A8_UNORM formats are allowed - ?? LJ ??
	//		If a shared texture is updated on one device ID3D11DeviceContext::Flush must be called on that device **

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476903%28v=vs.85%29.aspx
	// To share a resource between two Direct3D 11 devices the resource must have been created
	// with the D3D11_RESOURCE_MISC_SHARED flag, if it was created using the ID3D11Device interface.
	//
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width				= width;
	desc.Height				= height;
	desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags			= D3D11_RESOURCE_MISC_SHARED; // This texture will be shared
	// A DirectX 11 texture with D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX is not compatible with DirectX 9
	// so a general named mutex is used for all texture types
	desc.Format				= format;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	// Multisampling quality and count
	// The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0.
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count	= 1;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);
	
	if (res != S_OK) {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476174%28v=vs.85%29.aspx
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDirectX::CreateSharedDX11Texture ERROR - [0x%x] : ", res);
		switch (res) {
			case D3DERR_INVALIDCALL:
				strcat_s(tmp, 256, "D3DERR_INVALIDCALL");
				break;
			case E_INVALIDARG:
				strcat_s(tmp, 256, "E_INVALIDARG");
				break;
			case E_OUTOFMEMORY:
				strcat_s(tmp, 256, "E_OUTOFMEMORY");
				break;
			default :
				strcat_s(tmp, 256, "Unlisted error");
				break;
		}
		SpoutLogFatal("%s", tmp);
		return false;
	}

	// The DX11 texture is created OK
	// Get the texture share handle so it can be saved in shared memory for receivers to pick up
	// When sharing a resource between two Direct3D 10/11 devices the unique handle 
	// of the resource can be obtained by querying the resource for the IDXGIResource 
	// interface and then calling GetSharedHandle.
	IDXGIResource* pOtherResource(NULL);
	if(pTexture->QueryInterface( __uuidof(IDXGIResource), (void**)&pOtherResource) != S_OK) {
		SpoutLogFatal("spoutDirectX::CreateSharedDX11Texture - QueryInterface error");
		return false;
	}

	// Return the shared texture handle
	pOtherResource->GetSharedHandle(&dxShareHandle); 
	pOtherResource->Release();

	*pSharedTexture = pTexture;

	SpoutLogNotice("    pTexture = 0x%Ix", pTexture);

	return true;

}

bool spoutDirectX::OpenDX11shareHandle(ID3D11Device* pDevice, ID3D11Texture2D** ppSharedTexture, HANDLE dxShareHandle)
{

	if (!pDevice || !ppSharedTexture || !dxShareHandle) {
		SpoutLogError("spoutDirectX::OpenDX11shareHandle - null sources");
		return false;
	}

	// To share a resource between a Direct3D 9 device and a Direct3D 11 device 
	// the texture must have been created using the pSharedHandle argument of CreateTexture.
	// The shared Direct3D 9 handle is then passed to OpenSharedResource in the hResource argument.
	// printf("OpenDX11shareHandle - pDevice [%x] %x, %x\n", pDevice, dxShareHandle, ppSharedTexture);
	HRESULT hr = pDevice->OpenSharedResource(dxShareHandle, __uuidof(ID3D11Resource), (void**)(ppSharedTexture));
	if(hr != S_OK) {
		SpoutLogError("spoutDirectX::OpenDX11shareHandle failed");
		return false;
	}

	/*
	// Can get sender format here
	ID3D11Texture2D * texturePointer = *ppSharedTexture;
	D3D11_TEXTURE2D_DESC td;
	texturePointer->GetDesc(&td);
	printf("td.Format = %d\n", td.Format); // 87
	printf("td.Width = %d\n", td.Width);
	printf("td.Height = %d\n", td.Height);
	printf("td.MipLevels = %d\n", td.MipLevels);
	printf("td.Usage = %d\n", td.Usage);
	printf("td.ArraySize = %d\n", td.ArraySize);
	printf("td.SampleDesc = %d\n", td.SampleDesc);
	printf("td.BindFlags = %d\n", td.BindFlags);
	printf("td.MiscFlags = %d\n", td.MiscFlags); // D3D11_RESOURCE_MISC_SHARED
	*/

	return true;

}

// Get context
ID3D11DeviceContext* spoutDirectX::GetImmediateContext()
{
	return m_pImmediateContext;
}

// Set required graphics adapter for output
bool spoutDirectX::SetAdapter(int index)
{
	char adaptername[128];
	IDXGIAdapter* pAdapter = nullptr;

	// SpoutLogNotice("spoutDirectX::SetAdapter(%d)\n", index);

	m_AdapterIndex = D3DADAPTER_DEFAULT; // DX9
	m_pAdapterDX11 = nullptr; // DX11

	// Reset
	if(index == -1)
		return true;

	// Is the requested adapter available
	if(index > GetNumAdapters()-1) {
		SpoutLogError("spoutDirectX::SetAdapter(%d) - Index greater than number of adapters", index);
		return false;
	}

	if(!GetAdapterName(index, adaptername, 128)) {
		SpoutLogError("spoutDirectX::SetAdapter(%d) - Incompatible adapter", index);
		return false;
	}

	// 17.03.18 - test for a valid pointer
	if (!GetAdapterPointer(index)) {
		SpoutLogError("spoutDirectX::SetAdapter(%d) - Incompatible adapter", index);
		return false;
	}

	// Set the global adapter pointer for DX11
	pAdapter = GetAdapterPointer(index);
	if(pAdapter == nullptr) {
		SpoutLogError("spoutDirectX::SetAdapter - Could not get pointer for adapter %d", index);
		return false;
	}
	m_pAdapterDX11 = pAdapter;

	// Set the global adapter index for DX9
	m_AdapterIndex = index;

	// In case of remaining incompatibility - test everything here

	// For >= 2.005 check the directX sharing mode
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);

	if(dwDX9 == 1) {

		// Try to create a DX9 object and device
		IDirect3D9Ex* pD3D; // DX9 object
		IDirect3DDevice9Ex* pDevice;     // DX9 device
		pD3D = CreateDX9object(); 
		if(pD3D == NULL) {
			SpoutLogError("spoutDirectX::SetAdapter(%d) - could not create DX9 object", index);
			// Reset to default adapter
			m_AdapterIndex = D3DADAPTER_DEFAULT; // DX9
			m_pAdapterDX11 = nullptr; // DX11
			return false;
		}
		pDevice = CreateDX9device(pD3D, NULL); 
		if(pDevice == NULL) {
			SpoutLogError("spoutDirectX::SetAdapter(%d) - could not create DX9 device", index);
			pD3D->Release();
			// Reset to default adapter
			m_AdapterIndex = D3DADAPTER_DEFAULT; // DX9
			m_pAdapterDX11 = nullptr; // DX11
			return false;
		}
		pD3D->Release();
		pDevice->Release();
	}
	else {
		// Try to create a DirectX 11 device
		ID3D11Device* pd3dDevice;
		pd3dDevice = CreateDX11device();
		if(pd3dDevice == NULL) {
			SpoutLogError("spoutDirectX::SetAdapter(%d) - could not create DX11 device", index);
			m_AdapterIndex = D3DADAPTER_DEFAULT; // DX9
			m_pAdapterDX11 = nullptr; // DX11
			return false;
		}
		// Close it because this is just a test
		// See : https://github.com/leadedge/Spout2/issues/17
		ReleaseDX11Device(pd3dDevice);
	}

	return true;

}

// Get the global adapter index
int spoutDirectX::GetAdapter()
{
	return m_AdapterIndex;
}


// FOR DEBUGGING 
bool spoutDirectX::FindNVIDIA(int &nAdapter)
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;
	DXGI_ADAPTER_DESC desc;
	UINT32 i;
	bool bFound = false;

	if ( FAILED( CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&_dxgi_factory1 ) ) )
		return false;

	for ( i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{
		adapter1_ptr->GetDesc( &desc );
		// printf("spoutDirectX::FindNVIDIA - Adapter(%d) : %S\n", i, desc.Description );
		/*
		DXGI_OUTPUT_DESC desc_out;
		IDXGIOutput* p_output = nullptr;
		if(adapter1_ptr->EnumOutputs(0, &p_output ) == DXGI_ERROR_NOT_FOUND) {
			printf("  No outputs\n");
			continue;
		}

		for ( UINT32 j = 0; adapter1_ptr->EnumOutputs( j, &p_output ) != DXGI_ERROR_NOT_FOUND; j++ ) {
			p_output->GetDesc( &desc_out );
			// printf( "  Output : %d\n", j );
			// printf( "    Name %S\n", desc_out.DeviceName );
			// printf( "    Attached to desktop : (%d) %s\n", desc_out.AttachedToDesktop, desc_out.AttachedToDesktop ? "yes" : "no" );
			// printf( "    Rotation : %d\n", desc_out.Rotation );
			// printf( "    Left     : %d\n", desc_out.DesktopCoordinates.left );
			// printf( "    Top      : %d\n", desc_out.DesktopCoordinates.top );
			// printf( "    Right    : %d\n", desc_out.DesktopCoordinates.right );
			// printf( "    Bottom   : %d\n", desc_out.DesktopCoordinates.bottom );
			if( p_output )
				p_output->Release();
		}
		*/
		if(wcsstr(desc.Description, L"NVIDIA")) {
			// printf("spoutDirectX::FindNVIDIA - Found NVIDIA adapter %d (%S)\n", i, desc.Description);
			bFound = true;
			break;
		}

	}

	_dxgi_factory1->Release();

	if(bFound) {
		// printf// ("spoutDirectX::FindNVIDIA - Found NVIDIA adapter %d (%S)\n", i, desc.Description);
		nAdapter = i;
		//	0x10DE	NVIDIA
		//	0x163C	intel
		//	0x8086  Intel
		//	0x8087  Intel
		// printf("Vendor    = %d [0x%X]\n", desc.VendorId, desc.VendorId);
		// printf("Revision  = %d [0x%X]\n", desc.Revision, desc.Revision);
		// printf("Device ID = %d [0x%X]\n", desc.DeviceId, desc.DeviceId);
		// printf("SubSys ID = %d [0x%X]\n", desc.SubSysId, desc.SubSysId);
		return true;
	}

	return false;

}


// Get the number of graphics adapters in the system
int spoutDirectX::GetNumAdapters()
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;
	UINT32 i;

	// Enum Adapters first : multiple video cards
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&_dxgi_factory1))) {
		SpoutLogError("spoutDirectX::GetNumAdapters - No adapters found");
		return 0;
	}

	for (i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{

		DXGI_ADAPTER_DESC desc;
		adapter1_ptr->GetDesc( &desc );
		// printf("Adapter(%d) : %S\n", i, desc.Description );
		// printf("  Vendor Id : %d\n", desc.VendorId );
		// printf("  Dedicated System Memory : %.0f MiB\n", (float)desc.DedicatedSystemMemory / (1024.f * 1024.f) );
		// printf("  Dedicated Video Memory : %.0f MiB\n", (float)desc.DedicatedVideoMemory / (1024.f * 1024.f) );
		// printf("  Shared System Memory : %.0f MiB", (float)desc.SharedSystemMemory / (1024.f * 1024.f) );
		IDXGIOutput* p_output = nullptr;
		
		// 24-10-18 change from error to warning
		if(adapter1_ptr->EnumOutputs(0, &p_output ) == DXGI_ERROR_NOT_FOUND) {
			SpoutLogWarning("spoutDirectX::GetNumAdapters Adapter(%d) :  No outputs", i);
		}

		for ( UINT32 j = 0; adapter1_ptr->EnumOutputs( j, &p_output ) != DXGI_ERROR_NOT_FOUND; j++ ) {
			DXGI_OUTPUT_DESC desc_out;
			p_output->GetDesc( &desc_out );
			// printf("  Output : %d\n", j );
			// printf("    Name %S\n", desc_out.DeviceName );
			// printf("    Attached to desktop : (%d) %s\n", desc_out.AttachedToDesktop, desc_out.AttachedToDesktop ? "yes" : "no" );
			// printf("    Rotation : %d\n", desc_out.Rotation );
			// printf("    Left     : %d\n", desc_out.DesktopCoordinates.left );
			// printf("    Top      : %d\n", desc_out.DesktopCoordinates.top );
			// printf("    Right    : %d\n", desc_out.DesktopCoordinates.right );
			// printf("    Bottom   : %d\n", desc_out.DesktopCoordinates.bottom );
			if( p_output )
				p_output->Release();
		}
	}

	_dxgi_factory1->Release();

	return (int)i;

}

// Get an adapter name
bool spoutDirectX::GetAdapterName(int index, char *adaptername, int maxchars)
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&_dxgi_factory1))) {
		SpoutLogError("spoutDirectX::GetAdapterName - Could not create CreateDXGIFactory1");
		return false;
	}
	
	for (UINT32 i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ ) {
		if((int)i == index) {
			DXGI_ADAPTER_DESC	desc;
			adapter1_ptr->GetDesc( &desc );
			adapter1_ptr->Release();
			size_t charsConverted = 0;
			size_t maxBytes = maxchars;
			wcstombs_s(&charsConverted, adaptername, maxBytes, desc.Description, maxBytes - 1);
			// Is the adapter compatible ? TODO
			_dxgi_factory1->Release();
			return true;
		}
	}

	_dxgi_factory1->Release();

	return false;
}


IDXGIAdapter* spoutDirectX::GetAdapterPointer(int index)
{
	// Enum Adapters first : multiple video cards
	IDXGIFactory1*	_dxgi_factory1;
	if ( FAILED( CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&_dxgi_factory1 ) ) )	{
		SpoutLogError("spoutDirectX::GetAdapterPointer - Could not create CreateDXGIFactory1" );
		return nullptr;
	}

	IDXGIAdapter* adapter1_ptr = nullptr;
	for ( UINT32 i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{
		if ( index == (int)i ) {
			// Now we have the requested adapter, but does it support the required extensions
			// 17-03-18 test for an output on the adapter
			IDXGIOutput* p_output = nullptr;
			if (adapter1_ptr->EnumOutputs(0, &p_output) == DXGI_ERROR_NOT_FOUND) {
				SpoutLogError("spoutDirectX::GetAdapterPointer(%d) :  No outputs", i);
				_dxgi_factory1->Release();
				return nullptr;
			}
			_dxgi_factory1->Release();
			return adapter1_ptr;
		}
		adapter1_ptr->Release();
	}
	_dxgi_factory1->Release();

	return nullptr;
}


bool spoutDirectX::GetAdapterInfo(char *adapter, char *display, int maxchars)
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;
	UINT32 i;
	size_t charsConverted = 0;
	size_t maxBytes = maxchars;

	// Enum Adapters first : multiple video cards
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&_dxgi_factory1))) {
		SpoutLogError("spoutDirectX::GetAdapterInfo - Could not create CreateDXGIFactory1");
		return false;
	}

	for ( i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{

		DXGI_ADAPTER_DESC	desc;
		adapter1_ptr->GetDesc( &desc );
		// Return the current adapter - max of 2 assumed
		wcstombs_s(&charsConverted, adapter, maxBytes, desc.Description, maxBytes-1);

		IDXGIOutput*	p_output = nullptr;
		for ( UINT32 j = 0; adapter1_ptr->EnumOutputs( j, &p_output ) != DXGI_ERROR_NOT_FOUND; j++ ) {
			DXGI_OUTPUT_DESC	desc_out;
			p_output->GetDesc( &desc_out );
			if(desc_out.AttachedToDesktop)
				wcstombs_s(&charsConverted, display, maxBytes, desc.Description, maxBytes-1);
			if( p_output )
				p_output->Release();
		}
	}

	_dxgi_factory1->Release();

	return true;
}


unsigned long spoutDirectX::ReleaseDX11Texture(ID3D11Device* pd3dDevice, ID3D11Texture2D* pTexture)
{
	if (!pd3dDevice || !pTexture)
		return 0;

	unsigned long refcount = pTexture->Release();

	if (m_pImmediateContext) {
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
	}

	if (refcount > 0) 
		SpoutLogWarning("spoutDirectX::ReleaseDX11Texture - refcount = %d", refcount);

	return refcount;
}


unsigned long spoutDirectX::ReleaseDX11Device(ID3D11Device* pd3dDevice)
{
	if (!pd3dDevice)
		return 0;

	// Release the global context or there is an outstanding ref count
	// when the device is released
	if (m_pImmediateContext) {
		// Clear state and flush context to prevent deferred device release
		// From Microsoft docs :
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476425(v=vs.85).aspx
		// Microsoft Direct3D 11 defers the destruction of objects. 
		// Therefore, an application can't rely upon objects immediately being destroyed.
		// By calling Flush, you destroy any objects whose destruction was deferred.
		// If an application requires synchronous destruction of an object, we recommend
		// that the application release all its references, call 
		// ID3D11DeviceContext::ClearState, and then call Flush.
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
		m_pImmediateContext->Release();
		m_pImmediateContext = NULL;
	}
	unsigned long refcount = pd3dDevice->Release();

	// Device release warning
	if (refcount > 0)
		SpoutLogWarning("spoutDirectX::ReleaseDX11Device - refcount = %d", refcount);

	pd3dDevice = NULL;

	return refcount;
}


void spoutDirectX::FlushWait(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
	D3D11_QUERY_DESC queryDesc;
	ID3D11Query * pQuery = NULL;

	if (!pImmediateContext)
		return;

	// ==================================================
	// Tests confirm that for a sender the following code
	// eliminates jerky texture access by a receiver.
	// ==================================================

	//
	// CopyResource is an asynchronous call.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb205132%28v=vs.85%29.aspx#Performance_Considerations
	// "the copy has not necessarily executed by the time the method returns".
	//

	// A flush is necessary to finish the command queue.
	pImmediateContext->Flush();

	// Make sure the CopyResource function has completed before
	// the receiver application accesses the shared texture.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476578%28v=vs.85%29.aspx
	//
	ZeroMemory(&queryDesc, sizeof(queryDesc));
	queryDesc.Query = D3D11_QUERY_EVENT;
	// When the GPU is finished, ID3D11DeviceContext::GetData will return S_OK.
	// When using this type of query, ID3D11DeviceContext::Begin is disabled.
	ZeroMemory(&queryDesc, sizeof(queryDesc));
	queryDesc.Query = D3D11_QUERY_EVENT;
	pd3dDevice->CreateQuery(&queryDesc, &pQuery);
	if (pQuery) {
		pImmediateContext->End(pQuery);
		while (S_OK != pImmediateContext->GetData(pQuery, NULL, 0, 0));
		pQuery->Release();
	}

}
