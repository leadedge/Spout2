//
//
//			spoutDX9.cpp
//
//		Functions to manage DirectX9 texture sharing
//
// ====================================================================================
//		Revisions :
//
//		09.10.20	- separated from SpoutDirectX class
//		23.01.21	- Create DX9 examples
//		23.01.21	- Change d3dpp.EnableAutoDepthStencil to TRUE so examples work
//		28.01.21	- Conversion for 2.007 support class
//					  Meshes sender completed
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

#include "SpoutDX9.h"

spoutDX9::spoutDX9() {

	m_pD3D = nullptr;
	m_pDevice = nullptr;
	m_bSpoutInitialized = false;
	m_dxShareHandle = NULL;
	m_pSharedTexture = nullptr;
	m_dwFormat = D3DFMT_A8R8G8B8;
	m_SenderNameSetup[0]=0;
	m_SenderName[0]=0;
	m_Width = 0;
	m_Height = 0;

}

spoutDX9::~spoutDX9() {

	ReleaseDX9sender();
	CloseDirectX9();

}


bool spoutDX9::SendDX9surface(IDirect3DSurface9* pSurface)
{
	// Quit if no data
	if (!pSurface)
		return false;

	// Get the surface details
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pSurface->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0)
		return false;

	// Create or update the sender
	if (!CheckDX9sender(desc.Width, desc.Height, (DWORD)desc.Format))
		return false;

	// Check the sender mutex for access the shared texture
	if (frame.CheckTextureAccess()) {
		// Copy the surface to the sender's shared texture
		// Both must have the same size and format
		WriteDX9surface(m_pDevice, pSurface, m_pSharedTexture);
		// Signal a new frame while the mutex is locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowTextureAccess();
	}

	return true;
}


// --------------------------------------------------------
// If a sender has not been created yet
//    o Make sure DirectX is initialized
//    o Create a shared texture for the sender
//    o Create a sender using the DX11 shared texture handle
// If the sender exists, test for size or format change
//    o Re-create the class shared texture to the new size
//    o Update the sender and class variables
bool spoutDX9::CheckDX9sender(unsigned int width, unsigned int height, DWORD dwFormat)
{
	if (width == 0 || height == 0)
		return false;

	// The sender needs a name
	// Default is the executable name
	if (!m_SenderName[0]) {
		SetSenderName();
	}

	if (!m_bSpoutInitialized) {

		// Make sure DirectX is initialized
		if (!OpenDirectX9())
			return false;

		// Save width and height to test for sender size changes
		m_Width = width;
		m_Height = height;
		m_dwFormat = dwFormat;

		// Create a shared texture for the sender
		// A sender creates a new texture with a new share handle
		m_dxShareHandle = nullptr;
		CreateSharedDX9Texture(m_pDevice, m_Width, m_Height, (D3DFORMAT)m_dwFormat, m_pSharedTexture, m_dxShareHandle);
		// Create a sender using the DX11 shared texture handle (m_dxShareHandle)
		// and specifying the same texture format
		m_bSpoutInitialized = sendernames.CreateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, m_dwFormat);

		// This could be a separate function SetHostPath
		SharedTextureInfo info;
		if (!sendernames.getSharedInfo(m_SenderName, &info)) {
			SpoutLogWarning("spoutDX9::SetHostPath(%s) - could not get sender info", m_SenderName);
			// printf("spoutGL::SetHostPath(%s) - could not get sender info\n", m_SenderName);
			return false;
		}
		char exepath[256];
		GetModuleFileNameA(NULL, exepath, sizeof(exepath));
		// Description is defined as wide chars, but the path is stored as byte chars
		strcpy_s((char*)info.description, 256, exepath);
		if (!sendernames.setSharedInfo(m_SenderName, &info)) {
			SpoutLogWarning("spoutDX9::SetHostPath(%s) - could not set sender info", m_SenderName);
		}

		// Create a sender mutex for access to the shared texture
		frame.CreateAccessMutex(m_SenderName);

		// Enable frame counting so the receiver gets frame number and fps
		frame.EnableFrameCount(m_SenderName);
	}
	// Initialized but has the source texture changed size ?
	else if (m_Width != width || m_Height != height || m_dwFormat != dwFormat) {

		// Re-create the class shared texture with the new size
		if (m_pSharedTexture) m_pSharedTexture->Release();
		m_pSharedTexture = nullptr;
		m_dxShareHandle = nullptr;
		CreateSharedDX9Texture(m_pDevice, m_Width, m_Height, (D3DFORMAT)m_dwFormat, m_pSharedTexture, m_dxShareHandle);
		// Update the sender and class variables
		sendernames.UpdateSender(m_SenderName, width, height, m_dxShareHandle, dwFormat);
		m_Width = width;
		m_Height = height;
		m_dwFormat = dwFormat;

	} // endif initialization or size checks

	return true;

}

//---------------------------------------------------------
// Function: ReleaseSender
// Close receiver and release resources.
//
// A new sender is created or updated by all sending functions
void spoutDX9::ReleaseDX9sender()
{
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (m_bSpoutInitialized)
		sendernames.ReleaseSenderName(m_SenderName);

	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	m_bSpoutInitialized = false;

}


//---------------------------------------------------------
// Function: SetSenderName
// Set name for sender creation
//
//   If no name is specified, the executable name is used. 
bool spoutDX9::SetSenderName(const char* sendername)
{
	if (!sendername) {
		// Get executable name as default
		GetModuleFileNameA(NULL, m_SenderName, 256);
		PathStripPathA(m_SenderName);
		PathRemoveExtensionA(m_SenderName);
	}
	else {
		strcpy_s(m_SenderName, 256, sendername);
	}

	// If a sender with this name is already registered, create an incremented name
	int i = 1;
	char name[256];
	strcpy_s(name, 256, m_SenderName);
	if (sendernames.FindSenderName(name)) {
		do {
			sprintf_s(name, 256, "%s_%d", m_SenderName, i);
			i++;
		} while (sendernames.FindSenderName(name));
	}
	// Re-set the global sender name
	strcpy_s(m_SenderName, 256, name);

	return true;
}


// Initialize and prepare DirectX 9
bool spoutDX9::OpenDirectX9(HWND hWnd)
{
	HWND fgWnd = NULL;
	char fgwndName[MAX_PATH];

	SpoutLogNotice("spoutDX9::OpenDirectX9 - hWnd = 0x%.7X", PtrToUint(hWnd) );

	// Already initialized ?
	if (m_pD3D) {
		SpoutLogNotice("    Device already initialized");
		return true;
	}

	// Create a IDirect3D9Ex object if not already created
	m_pD3D = CreateDX9object();

	if (!m_pD3D) {
		SpoutLogWarning("    Could not create DX9 object");
		return false;
	}

	// Create DX9 device
	if (!m_pDevice) {
		m_pDevice = CreateDX9device(m_pD3D, hWnd);
	}

	if (!m_pDevice) {
		SpoutLogWarning("    Could not create DX9 device");
		return false;
	}

	// Problem for FFGL plugins - might be a problem for other FFGL hosts or applications.
	// DirectX 9 device initialization creates black areas and the host window has to be redrawn.
	// But this causes a crash for a sender in Magic when the render window size is changed.
	//   o Not a problem for DirectX 11.
	//   o Not needed in Isadora.
	//   o Needed for Resolume.
	// For now, limit this to Resolume only.
	// TODO : re-test this old issue
	fgWnd = GetForegroundWindow();
	if (fgWnd) {
		// SMTO_ABORTIFHUNG : The function returns without waiting for the time-out
		// period to elapse if the receiving thread appears to not respond or "hangs."
		if (SendMessageTimeoutA(fgWnd, WM_GETTEXT, MAX_PATH, (LPARAM)fgwndName, SMTO_ABORTIFHUNG, 128, NULL) != 0) {
			// Returns the full path - get just the window name
			PathStripPathA(fgwndName);
			if (fgwndName[0]) {
				if (strstr(fgwndName, "Resolume") != NULL // Is resolume in the window title ?
					&& strstr(fgwndName, "magic") == NULL) { // Make sure it is not a user named magic project.
						// DirectX device initialization needs the window to be redrawn (creates black areas)
						// 03.05.15 - user observation that UpDateWindow does not work and Resolume GUI is still corrupted
						// 28.08.15 - user observation of a crash with Windows 10 
						// try RedrawWindow again (with InvalidateRect as well) - confirmed working with Win 7 32bit
						// https://msdn.microsoft.com/en-us/library/windows/desktop/dd145213%28v=vs.85%29.aspx
						// The WM_PAINT message is generated by the system and should not be sent by an application.					
						// SendMessage(fgWnd, WM_PAINT, NULL, NULL ); // causes problems
					InvalidateRect(fgWnd, NULL, false); // make sure
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_INTERNALPAINT);
				}
			}
		}
	}

	return true;
}

// Create a DX9 object
IDirect3D9Ex* spoutDX9::CreateDX9object()
{
	IDirect3D9Ex* pD3D;

	HRESULT res = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D);
	if (FAILED(res)) return NULL;

	return pD3D;
}

// Create a DX9 device
IDirect3DDevice9Ex* spoutDX9::CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd, unsigned int AdapterIndex)
{
	IDirect3DDevice9Ex* pDevice;
    D3DPRESENT_PARAMETERS d3dpp;
	D3DCAPS9 d3dCaps;

	SpoutLogNotice("spoutDirectX::CreateDX9device - adapter = %u, hWnd = 0x%.7X", AdapterIndex, PtrToUint(hWnd) );

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed		= true;						// windowed and not full screen
    d3dpp.SwapEffect	= D3DSWAPEFFECT_DISCARD;	// discard old frames
    d3dpp.hDeviceWindow	= hWnd;						// set the window to be used by D3D

	// D3DFMT_UNKNOWN can be specified for the BackBufferFormat while in windowed mode. 
	// This tells the runtime to use the current display-mode format and eliminates
	// the need to call GetDisplayMode. 
	d3dpp.BackBufferFormat		 = D3DFMT_UNKNOWN;
	// Don't specify a backbuffer size here
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.BackBufferCount		 = 1;

	// Test for hardware vertex processing capability and set up as needed
	// D3DCREATE_MULTITHREADED required by interop spec
	if (FAILED(pD3D->GetDeviceCaps(AdapterIndex, D3DDEVTYPE_HAL, &d3dCaps))) {
		SpoutLogFatal("spoutDX9::CreateDX9device - GetDeviceCaps error");
		return NULL;
	}

	// | D3DCREATE_NOWINDOWCHANGES
	DWORD dwBehaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED; 
	if ( d3dCaps.VertexProcessingCaps != 0 )
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Create a DirectX9 device - we use directx only for accessing the handle
	// Note : hwnd seems to have no effect - maybe because we do not render anything.
	// Note here that we are setting up for Windowed mode but it seems not to be affected
	// by fullscreen, probably because we are not rendering to it.
	HRESULT res = pD3D->CreateDeviceEx(	AdapterIndex,   // D3DADAPTER_DEFAULT
								D3DDEVTYPE_HAL, // Hardware rasterization. 
								hWnd,			// hFocusWindow (can be NULL)
								dwBehaviorFlags,
								&d3dpp,			// d3dpp.hDeviceWindow should be valid if hFocusWindow is NULL
								NULL,			// pFullscreenDisplayMode must be NULL for windowed mode
								&pDevice);
	
	if (FAILED(res)) {
		SpoutLogFatal("spoutDX9::CreateDX9device - CreateDeviceEx returned error %u (0x%.X)", LOWORD(res), LOWORD(res) );
		return NULL;
	}

	SpoutLogNotice("    device (0x%.7X)", PtrToUint(pDevice) );

	return pDevice;

} // end CreateDX9device


IDirect3D9Ex* spoutDX9::GetDX9object()
{
	return m_pD3D;
}

IDirect3DDevice9Ex* spoutDX9::GetDX9device()
{
	return m_pDevice;
}

void spoutDX9::SetDX9device(IDirect3DDevice9Ex* pDevice)
{
	// The Spout DX9 object is not used if the device is set externally
	if (m_pD3D) {
		m_pD3D->Release();
		// If set externally, the device is also released externally,
		// so it must not be released in this class.
		// The Spout DX9 device can be released here because
		// it will not be released again if m_pD3D is NULL
		if (m_pDevice)
			m_pDevice->Release();
		m_pD3D = nullptr;
		m_pDevice = nullptr;
	}

	SpoutLogNotice("spoutDX9::SetDX9device (0x%.7X)", PtrToUint(pDevice) );

	// Already initialized ?
	if (pDevice && m_pDevice == pDevice) {
		SpoutLogWarning("spoutDX9::SetDX9device -(0x%.7X) already initialized", PtrToUint(pDevice) );
	}

	// Set the Spout DX9 device to the application device
	m_pDevice = pDevice;

}

void spoutDX9::CloseDirectX9()
{
	SpoutLogNotice("spoutDX9::CloseDirectX9");

	if (m_pD3D) {
		// Release device before the object
		if (m_pDevice)
			m_pDevice->Release();
		m_pD3D->Release();
	}
	m_pDevice = nullptr;
	m_pD3D = nullptr;

}


// Create a shared DirectX9 texture
// by giving it a sharehandle variable - dxShareHandle
// For a SENDER : the sharehandle is NULL and a new texture is created
// For a RECEIVER : the sharehandle is valid and a handle to the existing shared texture is created
bool spoutDX9::CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle)
{
	if (!pDevice) {
		SpoutLogError("spoutDX9::CreateSharedDX9Texture - NULL DX9 device");
		return false;
	}

	// printf("spoutDX9::CreateSharedDX9Texture(0x%7X, %d, %d, %d, 0x%.7X, 0x%.7X)\n", PtrToUint(pDevice), width, height, format, PtrToUint(dxTexture), LOWORD(dxShareHandle) );

	if(dxTexture) dxTexture->Release();

	HRESULT res = pDevice->CreateTexture(width,
										 height,
										 1,
										 D3DUSAGE_RENDERTARGET, 
										 format, // default is D3DFMT_A8R8G8B8 - may be set externally
										 D3DPOOL_DEFAULT, // Required by interop spec
										 &dxTexture,
										 &dxShareHandle);	// local share handle to allow type casting for 64bit

	// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
	// USAGE, format and size for sender and receiver must all match
	if (FAILED(res)) {
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDX9::CreateSharedDX9Texture error %d (0x%.X) - ", LOWORD(res), LOWORD(res) );
		switch (LOWORD(res)) {
			case ERROR_INVALID_PARAMETER:
				strcat_s(tmp, 256, "ERROR_INVALID_PARAMETER");
				break;
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

	SpoutLogNotice("spoutDX9::CreateSharedDX9Texture %dx%d - format %d - handle 0x%.7X", width, height, (int)format, LOWORD(dxShareHandle) );


	return true;

} // end CreateSharedDX9Texture


bool spoutDX9::WriteDX9memory(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 source_surface, LPDIRECT3DTEXTURE9 dxTexture)
{
	IDirect3DSurface9* texture_surface = nullptr;
	IDirect3DQuery9* pEventQuery = nullptr;
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
			if (pEventQuery) {
				pEventQuery->Issue(D3DISSUE_END);
				while (S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH));
				pEventQuery->Release(); // Must be released or causes a leak and reference count increment
			}
			return true;
		}
	}

	SpoutLogError("spoutDX9::WriteDX9memory((0x%.7X), (0x%.7X), (0x%.7X) failed", PtrToUint(pDevice), PtrToUint(dxTexture), PtrToUint(source_surface) );

	return false;

} // end WriteDX9memory


//
// COPY FROM A GPU DX9 SURFACE TO THE SHARED DX9 TEXTURE
//
//    The source surface must have been created using the same device as the texture
//
bool spoutDX9::WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture)
{
	IDirect3DSurface9* texture_surface = nullptr;
	HRESULT hr = dxTexture->GetSurfaceLevel(0, &texture_surface); // destination texture surface
	if (SUCCEEDED(hr)) {
		// StretchRect is a GPU copy
		hr = pDevice->StretchRect(surface, NULL, texture_surface, NULL, D3DTEXF_NONE);
		if(SUCCEEDED(hr)) {
			// It is necessary to flush the command queue
			// or the data is not ready for the receiver to read.
			// Adapted from : https://msdn.microsoft.com/en-us/library/windows/desktop/bb172234%28v=vs.85%29.aspx
			// Also see : http://www.ogre3d.org/forums/viewtopic.php?f=5&t=50486
			IDirect3DQuery9* pEventQuery = nullptr;
			pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery) ;
			if(pEventQuery) {
				pEventQuery->Issue(D3DISSUE_END) ;
				while(S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH)) ;
				pEventQuery->Release(); // Must be released or causes a leak and reference count increment
			}
			return true;
		}
	}

	return false;
}


//---------------------------------------------------------
// Function: HoldFps
// Frame rate control
void spoutDX9::HoldFps(int fps)
{
	frame.HoldFps(fps);
}
