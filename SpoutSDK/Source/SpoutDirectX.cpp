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
//		29.05.15	- Included SetAdapter for multiple adapters - Franz Hildgen.
//		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
//		08.06.15	- removed dx9 flag from setadapter
//		04.08.15	- cleanup
//		11.08.15	- removed GetAdapterName return if Intel. For use with Intel HD4400/5000 graphics
// ====================================================================================
/*

		Copyright (c) 2014-2015. Lynn Jarvis. All rights reserved.

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

	// Output graphics adapter
	// Programmer can set for an application
	g_AdapterIndex  = D3DADAPTER_DEFAULT; // DX9
	g_pAdapterDX11  = nullptr; // DX11


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
	// int AdapterIndex = 0; // DEBUG disable temp
	int AdapterIndex = g_AdapterIndex;

	// printf("CreateDX9device : g_AdapterIndex = %d\n", g_AdapterIndex);

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
	if(pD3D->GetDeviceCaps( AdapterIndex, D3DDEVTYPE_HAL, &d3dCaps) != S_OK ) {
		printf("spoutDirectX::CreateDX9device - GetDeviceCaps error\n");
		return false;
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
    res = pD3D->CreateDeviceEx(	AdapterIndex, // D3DADAPTER_DEFAULT
								D3DDEVTYPE_HAL, // Hardware rasterization. 
								hWnd,			// hFocusWindow (can be NULL)
								dwBehaviorFlags,
								&d3dpp,			// d3dpp.hDeviceWindow should be valid if hFocusWindow is NULL
								NULL,			// pFullscreenDisplayMode must be NULL for windowed mode
								&pDevice);
	
	if ( res != D3D_OK ) {
		printf("spoutDirectX::CreateDX9device - CreateDeviceEx returned error %d (%x)\n", res, res);
		return NULL;
	}

	// printf("spoutDirectX::CreateDX9device - pDevice = %d (%x)\n", pDevice, pDevice);

	return pDevice;

} // end CreateDX9device


// Create a shared DirectX9 texture
// by giving it a sharehandle variable - dxShareHandle
// For a SENDER : the sharehanlde is NULL and a new texture is created
// For a RECEIVER : the sharehandle is valid and a handle to the existing shared texture is created
bool spoutDirectX::CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle)
{

	// DEBUG
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
	// IDXGIAdapter* pAdapterDX11 = nullptr; // DEBUG temp disable
	IDXGIAdapter* pAdapterDX11 = g_pAdapterDX11;

	// printf("CreateDX11device : g_AdapterIndex = %d, pAdapterDX11 = [%x]\n", g_AdapterIndex, g_pAdapterDX11);

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

	// To allow for multiple graphics cards we will use g_pAdapterDX11
	// Which is set by SetAdapter before initializing DirectX
	// printf("CreateDX11device : pAdapterDX11 = %x\n", pAdapterDX11);

	if(pAdapterDX11) {
			hr = D3D11CreateDevice( pAdapterDX11,
									D3D_DRIVER_TYPE_UNKNOWN,
									NULL,
									createDeviceFlags,
									featureLevels,
									numFeatureLevels,
									D3D11_SDK_VERSION,
									&pd3dDevice,
									&g_featureLevel,
									&g_pImmediateContext );
	} // endif adapter set
	else {
		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ ) {
			g_driverType = driverTypes[driverTypeIndex];

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
	} // endif no adapter set
	
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
	// printf("OpenDX11shareHandle - pDevice [%x] %x, %x\n", pDevice, dxShareHandle, ppSharedTexture);
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


	// For debugging
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

	// For debugging
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

// Set required graphics adapter for output
bool spoutDirectX::SetAdapter(int index)
{
	char adaptername[128];

	// printf("spoutDirectX::SetAdapter(%d)\n", index);

	g_AdapterIndex = D3DADAPTER_DEFAULT; // DX9
	g_pAdapterDX11 = nullptr; // DX11

	// Reset
	if(index == -1) {
		return true;
	}

	// Is the requested adapter available
	if(index > GetNumAdapters()) {
		printf("Index greater than number of adapters\n");
		return false;
	}

	if(!GetAdapterName(index, adaptername, 128)) {
		printf("Incompatible adapter\n");
		return false;
	}

	// Set the global adapter index for DX9
	g_AdapterIndex = index;

	// Set the global adapter pointer for DX11
	g_pAdapterDX11 = GetAdapterPointer(index);

	return true;

}

// Get the global adapter index
int spoutDirectX::GetAdapter()
{
	return g_AdapterIndex;
}



// Get the number of graphics adapters in the system
int spoutDirectX::GetNumAdapters()
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;
	UINT32 i;

	// Enum Adapters first : multiple video cards
	if ( FAILED( CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&_dxgi_factory1 ) ) )
		return 0;

	for ( i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{
		// DXGI_ADAPTER_DESC	desc;
		// adapter1_ptr->GetDesc( &desc );
		// printf( "Adapter : %S\n", desc.Description );
		// adapter1_ptr->Release();

		/*
		// printf( "bdd_spout : D3D11 Adapter %d found\n", i );
		DXGI_ADAPTER_DESC	desc;
		adapter1_ptr->GetDesc( &desc );
		printf( "Adapter(%d) : %S\n", i, desc.Description );
		printf( "  Vendor Id : %d\n", desc.VendorId );
		printf( "  Dedicated System Memory : %.0f MiB\n", (float)desc.DedicatedSystemMemory / (1024.f * 1024.f) );
		printf( "  Dedicated Video Memory : %.0f MiB\n", (float)desc.DedicatedVideoMemory / (1024.f * 1024.f) );
		printf( "  Shared System Memory : %.0f MiB\n", (float)desc.SharedSystemMemory / (1024.f * 1024.f) );
		
		IDXGIOutput*	p_output = nullptr;
		for ( UINT32 j = 0; adapter1_ptr->EnumOutputs( j, &p_output ) != DXGI_ERROR_NOT_FOUND; j++ ) {
			DXGI_OUTPUT_DESC	desc_out;
			p_output->GetDesc( &desc_out );
			printf( "  Output : %d\n", j );
			printf( "    Name %S\n", desc_out.DeviceName );
			printf( "    Attached to desktop : %s\n", desc_out.AttachedToDesktop ? "yes" : "no" );
			//printf( "    Rotation", desc_out.DeviceName );
			printf( "    Left : %d\n", desc_out.DesktopCoordinates.left );
			printf( "    Top : %d\n", desc_out.DesktopCoordinates.top );
			printf( "    Right : %d\n", desc_out.DesktopCoordinates.right );
			printf( "    Bottom : %d\n", desc_out.DesktopCoordinates.bottom );
			if( p_output )
				p_output->Release();
		}
		*/


	}


	return (int)i;

}

// Get an adapter name
bool spoutDirectX::GetAdapterName(int index, char *adaptername, int maxchars)
{
	IDXGIFactory1* _dxgi_factory1;
	IDXGIAdapter* adapter1_ptr = nullptr;
	UINT32 i;

	if ( FAILED( CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&_dxgi_factory1 ) ) )
		return false;
	
	for ( i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ ) {
		if((int)i == index) {
			DXGI_ADAPTER_DESC	desc;
			adapter1_ptr->GetDesc( &desc );
			adapter1_ptr->Release();
			size_t charsConverted = 0;
			wcstombs_s(&charsConverted, adaptername, maxchars, desc.Description, maxchars-1);
			// Is the adapter compatible ?
			// For now - just test for Intel graphics
			// 11.08.15 - removed for use with Intel HD4400/5000 graphics
			// if(strstr(adaptername, "Intel")) {
				// printf("Intel graphics not supported\n");
				// return false;
			// }
			return true;
		}
	}

	return false;
}


IDXGIAdapter* spoutDirectX::GetAdapterPointer(int index)
{
	// Enum Adapters first : multiple video cards
	IDXGIFactory1*	_dxgi_factory1;
	if ( FAILED( CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&_dxgi_factory1 ) ) )	{
		printf( "Could not create CreateDXGIFactory1\n" );
		return nullptr;
	}

	IDXGIAdapter* adapter1_ptr = nullptr;
	for ( UINT32 i = 0; _dxgi_factory1->EnumAdapters( i, &adapter1_ptr ) != DXGI_ERROR_NOT_FOUND; i++ )	{
		
		/*
		printf( "bdd_spout : D3D11 Adapter %d found\n", i );
		DXGI_ADAPTER_DESC	desc;
		adapter1_ptr->GetDesc( &desc );
		printf( "Adapter : %S\n", desc.Description );
		printf( "  Vendor Id : %d\n", desc.VendorId );
		printf( "  Dedicated System Memory : %.0f MiB\n", (float)desc.DedicatedSystemMemory / (1024.f * 1024.f) );
		printf( "  Dedicated Video Memory : %.0f MiB\n", (float)desc.DedicatedVideoMemory / (1024.f * 1024.f) );
		printf( "  Shared System Memory : %.0f MiB\n", (float)desc.SharedSystemMemory / (1024.f * 1024.f) );
		
		IDXGIOutput*	p_output = nullptr;
		for ( UINT32 j = 0; adapter1_ptr->EnumOutputs( j, &p_output ) != DXGI_ERROR_NOT_FOUND; j++ ) {
			DXGI_OUTPUT_DESC	desc_out;
			p_output->GetDesc( &desc_out );
			printf( "  Output : %d\n", j );
			printf( "    Name %S\n", desc_out.DeviceName );
			printf( "    Attached to desktop : %s\n", desc_out.AttachedToDesktop ? "yes" : "no" );
			//printf( "    Rotation", desc_out.DeviceName );
			printf( "    Left : %d\n", desc_out.DesktopCoordinates.left );
			printf( "    Top : %d\n", desc_out.DesktopCoordinates.top );
			printf( "    Right : %d\n", desc_out.DesktopCoordinates.right );
			printf( "    Bottom : %d\n", desc_out.DesktopCoordinates.bottom );
			if( p_output )
				p_output->Release();
		}
		*/

		if ( index == (int)i ) {
			// Now we have the requested adapter, but does it support the required extensions
			return adapter1_ptr;
		}

		adapter1_ptr->Release();
	}

	return nullptr;
}



