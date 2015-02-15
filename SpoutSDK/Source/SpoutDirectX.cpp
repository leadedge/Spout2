//
//
//			spoutDirectX.cpp
//
//		DirectX functions to manage DirectX 11 texture sharing
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
//
// ====================================================================================
/*

		Copyright (c) 2014, Lynn Jarvis. All rights reserved.

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
	g_pImmediateContext = NULL;
	g_driverType		= D3D_DRIVER_TYPE_NULL;
	g_featureLevel		= D3D_FEATURE_LEVEL_11_0;

	// For debugging only - to toggle texture access locks disable/enable
	bUseAccessLocks     = true; // use texture access locks by default

}

spoutDirectX::~spoutDirectX() {

}

//
// =========================== DX9 ================================
//

// Create a DX9 object
IDirect3D9Ex* spoutDirectX::CreateDX9object()
{
	HRESULT res;
	IDirect3D9Ex* pD3D;
    
	res = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D);
	if ( res != D3D_OK ) return NULL;

	return pD3D;
}

// Create a DX9 device
IDirect3DDevice9Ex* spoutDirectX::CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd)
{
	HRESULT res;
	IDirect3DDevice9Ex* pDevice;
    D3DPRESENT_PARAMETERS d3dpp;
	D3DCAPS9 d3dCaps;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed		= TRUE;						// windowed and not full screen
    d3dpp.SwapEffect	= D3DSWAPEFFECT_DISCARD;	// discard old frames
    d3dpp.hDeviceWindow	= hWnd;						// set the window to be used by D3D

	// D3DFMT_UNKNOWN can be specified for the BackBufferFormat while in windowed mode. 
	// This tells the runtime to use the current display-mode format and eliminates
	// the need to call GetDisplayMode. 
	d3dpp.BackBufferFormat		 = D3DFMT_UNKNOWN;

	// Set a dummy resolution - we don't render anything
    d3dpp.BackBufferWidth		 = 1920;
    d3dpp.BackBufferHeight		 = 1080;
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.BackBufferCount		 = 1;

	// Test for hardware vertex processing capability and set up as needed
	// D3DCREATE_MULTITHREADED required by interop spec
	if(pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps) != S_OK ) return false;
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
    res = pD3D->CreateDeviceEx(	D3DADAPTER_DEFAULT,
								D3DDEVTYPE_HAL, // Hardware rasterization. 
								hWnd,			// hFocusWindow (can be NULL)
								dwBehaviorFlags,
								&d3dpp,			// d3dpp.hDeviceWindow should be valid if hFocusWindow is NULL
								NULL,			// pFullscreenDisplayMode must be NULL for windowed mode
								&pDevice);
	
	if ( res != D3D_OK ) {
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

	// LJ DEBUG
	// if(dxTexture) dxTexture->Release();

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
		printf("DX9 CreateTexture error : ");
		switch (res) {
			case D3DERR_INVALIDCALL:
				printf("    D3DERR_INVALIDCALL \n");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				printf("    D3DERR_OUTOFVIDEOMEMORY \n");
				break;
			case E_OUTOFMEMORY:
				printf("    E_OUTOFMEMORY \n");
				break;
			default :
				printf("    Unknown error\n");
				break;
		}
		return false;
	}

	return true;

} // end CreateSharedDX9Texture
// =========================== end DX9 =============================


//
// =========================== DX11 ================================
//

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
ID3D11Device* spoutDirectX::CreateDX11device()
{
	ID3D11Device* pd3dDevice = NULL;
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;

	// GL/DX interop Spec
	// ID3D11Device can only be used on WDDM operating systems : Must be multithreaded
	// D3D11_CREATE_DEVICE_FLAG createDeviceFlags
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
	// A DirectX 11 texture with D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX is not compatible with DirectX 9
	// so a general named mutext is used for all texture types
	desc.Format				= format;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count	= 1;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture); // pSharedTexture);

	if (res != S_OK) {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476174%28v=vs.85%29.aspx
		printf("CreateTexture2D ERROR : [0x%x]\n", res);
		switch (res) {
			case D3DERR_INVALIDCALL:
				printf("    D3DERR_INVALIDCALL \n");
				break;
			case E_INVALIDARG:
				printf("    E_INVALIDARG \n");
				break;
			case E_OUTOFMEMORY:
				printf("    E_OUTOFMEMORY \n");
				break;
			default :
				printf("    Unlisted error\n");
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
	if(pTexture->QueryInterface( __uuidof(IDXGIResource), (void**)&pOtherResource) != S_OK) {
		printf("    QueryInterface error\n");
		return false;
	}

	// Return the shared texture handle
	pOtherResource->GetSharedHandle(&dxShareHandle); 
	pOtherResource->Release();

	*pSharedTexture = pTexture;

	return true;

}

bool spoutDirectX::OpenDX11shareHandle(ID3D11Device* pDevice, ID3D11Texture2D** ppSharedTexture, HANDLE dxShareHandle)
{
	HRESULT hr;

	// To share a resource between a Direct3D 9 device and a Direct3D 11 device 
	// the texture must have been created using the pSharedHandle argument of CreateTexture.
	// The shared Direct3D 9 handle is then passed to OpenSharedResource in the hResource argument.
	hr = pDevice->OpenSharedResource(dxShareHandle, __uuidof(ID3D11Resource), (void**)(ppSharedTexture));
	if(hr != S_OK) {
		return false;
	}
	
	// Can get sender format here

	return true;

}



// =================================================================
// Texture access mutex locks
//
// A general mutex lock for DirectX 9 and for DirectX11 textures
//
// =================================================================
bool spoutDirectX::CreateAccessMutex(const char *name, HANDLE &hAccessMutex)
{
	DWORD errnum;
	char szMutexName[256]; // name of the mutex

	// printf("spoutDirectX::CreateAccessMutex\n");

	// Create the mutex name to control access to the shared texture
	sprintf_s((char*)szMutexName,  256, "%s_SpoutAccessMutex", name);

	// Create or open mutex depending, on whether it already exists or not
    hAccessMutex = CreateMutexA ( NULL,   // default security
						  FALSE,  // No initial owner
						  (LPCSTR)szMutexName);

	if (hAccessMutex == NULL) {
		printf("CreateAccessMutex : failed\n");
        return false;
	}
	else {
		errnum = GetLastError();
		// printf("read event GetLastError() = %d\n", errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			printf("access mutex [%s] invalid handle\n", szMutexName);
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			// printf("access mutex [%s] already exists\n", szMutexName);
		}
		else {
			// printf("access mutex [%s] created\n", szMutexName);
		}
	}

	return true;

}

void spoutDirectX::CloseAccessMutex(HANDLE &hAccessMutex)
{
	// printf("CloseAccessMutex [%x]\n", hAccessMutex);
	if(hAccessMutex) CloseHandle(hAccessMutex);
	hAccessMutex = NULL; // makes sure the passed handle is set to NULL
}


//
// Checks whether any other process is holding the lock and waits for access for 4 frames if so.
// For receiving from Version 1 apps with no mutex lock, a reader will have created the mutex and
// will have sole access and rely on the interop locks
bool spoutDirectX::CheckAccess(HANDLE hAccessMutex, ID3D11Texture2D* pSharedTexture)
{
	DWORD dwWaitResult;

	UNREFERENCED_PARAMETER(pSharedTexture);


	// LJ DEBUG
	if(!bUseAccessLocks) return true;

	// General mutex lock
	// DirectX 11 keyed mutex lock removed due to compatibility problems
	if(!hAccessMutex) return true; 

	// printf("Mutex lock\n");
	dwWaitResult = WaitForSingleObject(hAccessMutex, 67); // 4 frames at 60fps
	if (dwWaitResult == WAIT_OBJECT_0 ) {
		// The state of the object is signalled.
		return true;
	}
	else {
		switch(dwWaitResult) {
			case WAIT_ABANDONED : // Could return here
				printf("CheckAccess : WAIT_ABANDONED\n");
				break;
			case WAIT_TIMEOUT : // The time-out interval elapsed, and the object's state is nonsignaled.
				printf("CheckAccess : WAIT_TIMEOUT\n");
				break;
			case WAIT_FAILED : // Could use call GetLastError
				printf("CheckAccess : WAIT_FAILED\n");
				break;
			default :
				printf("CheckAccess : unknown error\n");
				break;
		}
	}
	return false;

}


void spoutDirectX::AllowAccess(HANDLE hAccessMutex, ID3D11Texture2D* pSharedTexture)
{

	UNREFERENCED_PARAMETER(pSharedTexture);

	// LJ DEBUG
	if(!bUseAccessLocks) return;

	if(hAccessMutex) ReleaseMutex(hAccessMutex);

}


void spoutDirectX::CloseDX11()
{

	if( g_pImmediateContext != NULL) {
		g_pImmediateContext->Flush();
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Release();
	}
	g_pImmediateContext = NULL;

}

//
// Verifying the System Version
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms725491%28v=vs.85%29.aspx
// GetVersion function
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724439%28v=vs.85%29.aspx
// Version Helper functions
// http://msdn.microsoft.com/en-us/library/windows/desktop/dn424972%28v=vs.85%29.aspx
//
bool spoutDirectX::DX11available()
{
	DWORD dwVersion = 0; 
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0; 
    DWORD dwBuild = 0;

    dwVersion = GetVersion();
 
    // Get the Windows version.
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.
    if (dwVersion < 0x80000000) dwBuild = (DWORD)(HIWORD(dwVersion));
    // printf("Version is %d.%d Build (%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);

	// DirectX 11 only available for Windows 7 (6.1) and higher
	if(dwMajorVersion >= 6 && dwMinorVersion >= 1) {
		// printf("DirectX 11 available\n");
		return true;
	}
	else {
		// printf("No DirectX 11\n");
		return false;
	}

}

/*
//
// http://www.nvidia.com/object/device_ids.html
//
// Example code to retrieve vendor and device ID's for the primary display device.
//    #include <windows.h>
//    #include <string>
//    #include <iostream>
//    using namespace std;
bool spoutDirectX::GetDeviceIdentification(char *vendorID, char *deviceID)
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);
	int i = 0;
	string id;
	char idchars[256];
	size_t charsConverted = 0;

	idchars[0] = NULL;

	// locate primary display device
	while (EnumDisplayDevices(NULL, 0, &dd, 0)) {
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
			wcstombs_s(&charsConverted, idchars, 129, dd.DeviceID, 128);
			printf("[%s]\n", idchars);
			break;
		}
		i++;
	}

	if(!idchars[0]) return false;
	
	// get vendor ID
	// vendorID = id.substr(8, 4);
	if(idchars[8] && strlen(idchars) > 12) {
		strcpy_s(vendorID, 256, &idchars[8]);
		vendorID[4] = 0;
	}

	// get device ID
	// deviceID = id.substr(17, 4);
	if(idchars[17] && strlen(idchars) > 21) {
		strcpy_s(deviceID, 256, &idchars[17]);
		deviceID[4] = 0;
	}

	return true;
}
*/



 