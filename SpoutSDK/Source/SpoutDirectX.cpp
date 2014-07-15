/**

	spoutDirectX.cpp


		Copyright (c) 2014>, Lynn Jarvis. All rights reserved.

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

	g_pImmediateContext = NULL;
	g_driverType		= D3D_DRIVER_TYPE_NULL;
	g_featureLevel		= D3D_FEATURE_LEVEL_11_0;

}

spoutDirectX::~spoutDirectX() {

}

//
// Notes for DX11 : https://www.opengl.org/registry/specs/NV/DX_interop2.txt
//
// Valid device types for the <dxDevice> parameter of wglDXOpenDeviceNV and associated restrictions
// DirectX device type : ID3D11Device - can only be used on WDDM operating systems; XXX Must be multithreaded
// TEXTURE_2D - ID3D11Texture2D - Usage flags must be D3D11_USAGE_DEFAULT
// wglDXSetResourceShareHandle does not need to be called for DirectX
// version 10 and 11 resources. Calling this function for DirectX 10
// and 11 resources is not an error but has no effect.

// Create DX11 device
ID3D11Device* spoutDirectX::CreateDX11device(HWND hWnd)
{
	ID3D11Device* pd3dDevice = NULL;
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;

	// GL/DX interop Spec
	// ID3D11Device can only be used on WDDM operating systems : Must be multithreaded
	// D3D11_CREATE_DEVICE_FLAG createDeviceFlags
	// malcolm - derivative reported possible NVIDIA memory leak bug unless D3D11_CREATE_DEVICE_SINGLETHREADED is set
	// but the spec requires multithreaded
	D3D_DRIVER_TYPE driverTypes[] =	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ ) {
		g_driverType = driverTypes[driverTypeIndex];

		// Createdevice only method, not the full swap chain
		hr = D3D11CreateDevice(	NULL,
								g_driverType,
								NULL,
								createDeviceFlags,
								featureLevels,
								numFeatureLevels,
								D3D11_SDK_VERSION, 
								&pd3dDevice,
								&g_featureLevel,
								&g_pImmediateContext);
		
		// Break as soon as something passes
		if(SUCCEEDED(hr))
			break;

	}
	
	// Quit if nothing worked
	if( FAILED(hr))
		return NULL;

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
	
	if(pd3dDevice == NULL)
		MessageBoxA(NULL, "CreateSharedDX11Texture NULL device", "SpoutSender", MB_OK);

	//
	// Create a new shared DX11 texture
	//

	pTexture = *pSharedTexture; // The texture pointer

	// if(pTexture == NULL) MessageBoxA(NULL, "CreateSharedDX11Texture NULL texture", "SpoutSender", MB_OK);

	// Textures being shared from D3D9 to D3D11 have the following restrictions (LJ - D3D11 to D3D9 ?).
	//		Textures must be 2D
	//		Only 1 mip level is allowed
	//		Texture must have default usage
	//		Texture must be write only	- ?? LJ ??
	//		MSAA textures are not allowed
	//		Bind flags must have SHADER_RESOURCE and RENDER_TARGET set
	//		Only R10G10B10A2_UNORM, R16G16B16A16_FLOAT and R8G8B8A8_UNORM formats are allowed - ?? LJ ??
	//		** If a shared texture is updated on one device ID3D11DeviceContext::Flush must be called on that device **

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
	desc.Format				= format;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count	= 1;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture); // pSharedTexture);

	if (res != S_OK) {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476174%28v=vs.85%29.aspx
		// printf("CreateTexture2D ERROR : [0x%x]\n", res);
		switch (res) {
			case D3DERR_INVALIDCALL:
				// printf("    D3DERR_INVALIDCALL \n");
				break;
			case E_INVALIDARG:
				// printf("    E_INVALIDARG \n");
				break;
			case E_OUTOFMEMORY:
				// printf("    E_OUTOFMEMORY \n");
				break;
			default :
				// printf("    Unlisted error\n");
				break;
		}
		return false;
	}

	// The DX11 texture is created OK
	// Get the texture share handle so it can be saved in shared memory for receivers to pick up
	// When sharing a resource between two Direct3D 10/11 devices the unique handle 
	// of the resource can be obtained by querying the resource for the IDXGIResource 
	// interface and then calling GetSharedHandle.
	IDXGIResource* pOtherResource(NULL);
	if(pTexture->QueryInterface( __uuidof(IDXGIResource), (void**)&pOtherResource) != S_OK)
		return false;

	// Return the shared texture handle
	pOtherResource->GetSharedHandle(&dxShareHandle); 
	pOtherResource->Release();

	*pSharedTexture = pTexture;

	return true;

}

bool spoutDirectX::OpenDX11shareHandle(ID3D11Device* pDevice, ID3D11Texture2D** pSharedTexture, HANDLE dxShareHandle)
{
	HRESULT hr;

	// To share a resource between a Direct3D 9 device and a Direct3D 11 device 
	// the texture must have been created using the pSharedHandle argument of CreateTexture.
	// The shared Direct3D 9 handle is then passed to OpenSharedResource in the hResource argument.
	hr = pDevice->OpenSharedResource(dxShareHandle, __uuidof(ID3D11Resource), (void**)(pSharedTexture));
	if(hr != S_OK) {
		// printf("OpenDX11shareHandle error 1\n");
		return false;
	}
	
	// Can get sender format here

	return true;

}


void spoutDirectX::CloseDX11()
{
	if( g_pImmediateContext != NULL) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Release();
	}
	g_pImmediateContext = NULL;

}