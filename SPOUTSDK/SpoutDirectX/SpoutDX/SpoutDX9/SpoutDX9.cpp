//
//			SpoutDX9.cpp
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
//		01.03.21	- Vertices sener and receiver completed
//		07.03.21	- Change HoldFps to allow numerator and denominator
//		11.06.21	- Add documentation
//		27.10.21	- ReceiveSenderData - use LongToHandle before cast
//		26.09.23	- CheckDX9sender - CreateSharedDX9Texture to new width and height for size change
//					- SendDX9surface - add update flag to allow a fixed sender size if false
//		06.12.23	- OpenDirectX9 uses string find instead of shlwapi PathStripPath
//					- SelectSenderPanel use _access instead of PathFileExists
//					- OpenDirectX9 - revise old plugin workaround.
//		11.07.24	- SelectSenderPanel - allow for UNICODE build : _wcsicmp / _tcsicmp
//		14.07.24	- Add sendernames functions from SpoutDX.cpp
//					- Update SelectSender for list box and dialog centre position
//		15.07.24	- SelectSender - after cast of window handle to long 
//					  convert to a string of 8 characters without new line
//		29.08.24	- ReadDX9texture - remove !frame.IsFrameCountEnabled() condition
//		11.10.25	- CreateSharedDX9Texture - change switch (LOWORD(res)) to switch (res)
//
// ====================================================================================
/*

	Copyright (c) 2020-2025. Lynn Jarvis. All rights reserved.

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

//
// Class: spoutDX9
//
// Functions to manage DirectX9 texture sharing.
//
// Refer to source code for further details.
//

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


//
// Group: DirectX9
//

// Function: OpenDirectX9
// Initialize and prepare DirectX 9.
// Retain a class object and device.
bool spoutDX9::OpenDirectX9(HWND hWnd)
{
	HWND fgWnd = NULL;
	char fgwndName[MAX_PATH];

	// Already initialized ?
	if (m_pD3D != nullptr) {
		return true;
	}

	SpoutLogNotice("spoutDX9::OpenDirectX9 - hWnd = 0x%.7X", PtrToUint(hWnd));

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

	// This section is historical due to a GUI interface problem
	// for FFGL plugins and could be removed in future.

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
			if (fgwndName[0]) {
				// If it's a full path - get just the window name
				std::string name = fgwndName;
				size_t pos = name.rfind("\\");
				if (pos == std::string::npos)
					pos = name.rfind("/");
				if (pos != std::string::npos) {
					name = name.substr(pos, name.length() - pos);
					pos = name.rfind(".");
					name = name.substr(0, pos);
					strcpy_s(fgwndName, 256, name.c_str());
				}

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

// Function: CloseDirectX9
// Close DirectX9 and free resources.
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

// Function: CreateDX9object
// Create a DX9 object.
IDirect3D9Ex* spoutDX9::CreateDX9object()
{
	IDirect3D9Ex* pD3D;

	HRESULT res = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D);
	if (FAILED(res)) return NULL;

	return pD3D;
}

// Function: CreateDX9device
// Create a DX9 device.
IDirect3DDevice9Ex* spoutDX9::CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd, unsigned int AdapterIndex)
{
	IDirect3DDevice9Ex* pDevice;
	D3DPRESENT_PARAMETERS d3dpp;
	D3DCAPS9 d3dCaps;

	SpoutLogNotice("spoutDirectX::CreateDX9device - adapter = %u, hWnd = 0x%.7X", AdapterIndex, PtrToUint(hWnd));

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = true;						// windowed and not full screen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// discard old frames
	d3dpp.hDeviceWindow = hWnd;						// set the window to be used by D3D

	// D3DFMT_UNKNOWN can be specified for the BackBufferFormat while in windowed mode. 
	// This tells the runtime to use the current display-mode format and eliminates
	// the need to call GetDisplayMode. 
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	// Don't specify a backbuffer size here
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.BackBufferCount = 1;

	// Test for hardware vertex processing capability and set up as needed
	// D3DCREATE_MULTITHREADED required by interop spec
	if (FAILED(pD3D->GetDeviceCaps(AdapterIndex, D3DDEVTYPE_HAL, &d3dCaps))) {
		SpoutLogFatal("spoutDX9::CreateDX9device - GetDeviceCaps error");
		return NULL;
	}

	// | D3DCREATE_NOWINDOWCHANGES
	DWORD dwBehaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED;
	if (d3dCaps.VertexProcessingCaps != 0)
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Create a DirectX9 device - we use directx only for accessing the handle
	// Note : hwnd seems to have no effect - maybe because we do not render anything.
	// Note here that we are setting up for Windowed mode but it seems not to be affected
	// by fullscreen, probably because we are not rendering to it.
	HRESULT res = pD3D->CreateDeviceEx(AdapterIndex,   // D3DADAPTER_DEFAULT
		D3DDEVTYPE_HAL, // Hardware rasterization. 
		hWnd,			// hFocusWindow (can be NULL)
		dwBehaviorFlags,
		&d3dpp,			// d3dpp.hDeviceWindow should be valid if hFocusWindow is NULL
		NULL,			// pFullscreenDisplayMode must be NULL for windowed mode
		&pDevice);

	if (FAILED(res)) {
		SpoutLogFatal("spoutDX9::CreateDX9device - CreateDeviceEx returned error %u (0x%.X)", LOWORD(res), LOWORD(res));
		return NULL;
	}

	SpoutLogNotice("    device (0x%.7X)", PtrToUint(pDevice));

	return pDevice;

} // end CreateDX9device

// Function: GetDX9object
// Return the class DirectX9 object.
IDirect3D9Ex* spoutDX9::GetDX9object()
{
	return m_pD3D;
}

// Function: GetDX9device
// Return the class DirectX9 device.
IDirect3DDevice9Ex* spoutDX9::GetDX9device()
{
	return m_pDevice;
}

// Function: SetDX9device
// Set the DirectX9 device to be used subsequently.
// The class DX9 object and device are not created or used
// if the device is set externally.
void spoutDX9::SetDX9device(IDirect3DDevice9Ex* pDevice)
{
	// The class DX9 object is not used if the device is set externally
	if (m_pD3D) {
		m_pD3D->Release();
		// The Spout DX9 device can be released here because
		// it will not be released again if m_pD3D is NULL.
		// If set externally, the device must also released externally
		if (m_pDevice)
			m_pDevice->Release();
		m_pD3D = nullptr;
		m_pDevice = nullptr;
	}

	SpoutLogNotice("spoutDX9::SetDX9device (0x%.7X)", PtrToUint(pDevice));

	// Already initialized ?
	if (pDevice && m_pDevice == pDevice) {
		SpoutLogWarning("spoutDX9::SetDX9device -(0x%.7X) already initialized", PtrToUint(pDevice));
	}

	// Set the Spout DX9 device to the application device
	m_pDevice = pDevice;

}

//
// Group: Sender
//


//---------------------------------------------------------
// Function: SetSenderName
// Set name for sender creation.
//   If no name is specified, the executable name is used. 
bool spoutDX9::SetSenderName(const char* sendername)
{
	if (!sendername) {
		// Executable name default
		// (GetExeName - see SpoutUtils)
		strcpy_s(m_SenderName, 256, GetExeName().c_str());
	}
	else {
		strcpy_s(m_SenderName, 256, sendername);
	}

	// If a sender with this name is already registered, create an incremented name
	int i = 1;
	char name[256]{};
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

//---------------------------------------------------------
// Function: SendDX9surface
// Send DirectX9 surface using the surface format.
//
//   bUpdate - update the sender size
//     - If true (default), size changes are handled by SendDX9surface.
//     - If false, the sender is created but not updated.
//       This allows a fixed sender size because DirectX 9 uses "StretchRect"
//       to copy the surface to the sender shared texture.
//
bool spoutDX9::SendDX9surface(IDirect3DSurface9* pSurface, bool bUpdate)
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
	if (bUpdate || !m_bSpoutInitialized) {
		if (!CheckDX9sender(desc.Width, desc.Height, (DWORD)desc.Format)) {
			return false;
		}
	}

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


//---------------------------------------------------------
// Function: ReleaseSender
// Close receiver and release resources.
// A new sender is created or updated by all sending functions.
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
// Function: IsInitialized
// Initialization status.
bool spoutDX9::IsInitialized()
{
	return m_bSpoutInitialized;
}

//---------------------------------------------------------
// Function: GetName
// Sender name.
const char * spoutDX9::GetName()
{
	return m_SenderName;
}

//---------------------------------------------------------
// Function: GetWidth
// Sender width.
unsigned int spoutDX9::GetWidth()
{
	return m_Width;
}

//---------------------------------------------------------
// Function: GetHeight
// Sender height.
unsigned int spoutDX9::GetHeight()
{
	return m_Height;
}

//---------------------------------------------------------
// Function: GetFps
// Sender frame rate.
double spoutDX9::GetFps()
{
	return (frame.GetSenderFps());
}

//---------------------------------------------------------
// Function: GetFrame
// Sender frame number.
long spoutDX9::GetFrame()
{
	return (frame.GetSenderFrame());
}



//
// Receiver
//


//
// Group: Receiver
//

//---------------------------------------------------------
// Function: SetReceiverName
// Specify sender for connection
//
//   The application will not connect to any other unless the user selects one
//   If that sender closes, the application will wait for the nominated sender to open 
//   If no name is specified, the receiver will connect to the active sender
void spoutDX9::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
	}
}

//---------------------------------------------------------
// Function: ReceiveDX9Texture
//  Copy from the sender shared texture
//
//	- Connect to a sender
//	- Set class variables for sender name, width and height
//  - If the sender has changed size, set a flag for the application to 
//	  re-allocate the receiving texture if IsUpdated() returns true.
//	  The receiving texture must be the same format.
//  - Copy the sender shared texture to the user texture or image.
//
bool spoutDX9::ReceiveDX9Texture(LPDIRECT3DTEXTURE9 &pTexture)
{
	// Try to receive texture details from a sender
	if (ReceiveSenderData()) {

		// Was the shared texture handle retrieved ?
		if (!m_dxShareHandle)
			return false;

		// The sender name, width, height, format, shared texture handle and pointer have been retrieved.
		// update the receiving texture
		ReadDX9texture(m_pDevice, pTexture);

		// Found a sender
		m_bConnected = true;

	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// Let the application know.
		m_bConnected = false;
	}

	// ReceiveTexture fails if there is no sender or the connected sender closed.
	return m_bConnected;

}


//---------------------------------------------------------
// Function: ReleaseReceiver
// Close receiver and release resources ready to connect to another sender.
void spoutDX9::ReleaseReceiver()
{
	if (!m_bSpoutInitialized)
		return;

	SpoutLogNotice("ReleaseReceiver(%s)", m_SenderName);

	// Restore the sender name if one was specified by SetReceiverName
	if (m_SenderNameSetup[0])
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);
	else
		m_SenderName[0] = 0;

	// Wait 4 frames in case the same sender opens again
	Sleep(67);

	if (m_pSharedTexture)
		m_pSharedTexture->Release();
	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	// Close the named access mutex and frame counting semaphore.
	frame.CloseAccessMutex();
	frame.CleanupFrameCount();

	// Zero width and height so that they are reset when a sender is found
	m_Width = 0;
	m_Height = 0;

	// Initialize again when a sender is found
	m_bSpoutInitialized = false;
	m_bUpdated = false;

}

//---------------------------------------------------------
// Function: SelectSender
// Open sender selection dialog.
bool spoutDX9::SelectSender(HWND hwnd)
{
	//
	// Use SpoutPanel if available
	//
	// SpoutPanel opens either centred on the cursor position 
	// or on the application window if the handle is passed in.

	// For a valid window handle, convert hwnd to chars
	// for the SpoutPanel command line
	char* msg = nullptr;
	if (hwnd) {
		// Window handle is an 32 bit unsigned value
		// Cast to long of 8 characters without new line
		msg = new char[256];
		sprintf_s(msg, 256, "%8.8ld\n", HandleToLong(hwnd));
	}

	if (!SelectSenderPanel(msg)) {

		// If SpoutPanel is not available use a SpoutMessageBox for sender selection.
		// Note that SpoutMessageBox is modal and will interrupt the host program.

		// create a local sender list
		std::vector<std::string> senderlist = GetSenderList();

		// Get the active sender index "selected".
		// The index is passed in to SpoutMessageBox and used as the current combobox item.
		int selected = 0;
		char sendername[256]{};
		if (GetActiveSender(sendername))
			selected = GetSenderIndex(sendername);

		// SpoutMessageBox opens either centred on the cursor position 
		// or on the application window if the handle is passed in.
		if (!hwnd) {
			POINT pt={};
			GetCursorPos(&pt);
			SpoutMessageBoxPosition(pt);
		}

		// Show the SpoutMessageBox even if the list is empty.
		// This makes it clear to the user that no senders are running.
		if (SpoutMessageBox(hwnd, NULL, "Select sender", MB_OKCANCEL, senderlist, selected) == IDOK && !senderlist.empty()) {
			// Release the receiver and set the selected sender as active for the next receive
			ReleaseReceiver();
			SetActiveSender(senderlist[selected].c_str());
			// Set the opened flag in the same way as for SelectSenderPanel
			// to indicate that the user has selected a sender.
			// This is tested in CheckSpoutPanel.
			m_bSpoutPanelOpened = true;
		}
	}

	if (msg) delete[] msg;

	return true;
}

//---------------------------------------------------------
// Function: IsUpdated
// Query whether the sender has changed.
//
//   Must be checked at every cycle before receiving data. 
//   If this is not done, the receiving functions fail.
bool spoutDX9::IsUpdated()
{
	bool bRet = m_bUpdated;
	m_bUpdated = false; // Reset the update flag
	return bRet;
}


//---------------------------------------------------------
// Function: IsConnected
// Query sender connection.
//
//   If the sender closes, receiving functions return false,  
//   but connection can be tested at any time.
bool spoutDX9::IsConnected()
{
	return m_bConnected;
}

//---------------------------------------------------------
// Function: IsFrameNew
// Query received frame status
//
//   The receiving texture or pixel buffer is refreshed if the sender has produced a new frame  
//   This can be queried to process texture data only for new frames.
bool spoutDX9::IsFrameNew()
{
	return m_bNewFrame;
}


//---------------------------------------------------------
// Function: GetSenderHandle
// Received sender share handle.
HANDLE spoutDX9::GetSenderHandle()
{
	return m_dxShareHandle;
}

//---------------------------------------------------------
// Function: GetSenderFormat
// Get sender DirectX texture format.
DWORD spoutDX9::GetSenderFormat()
{
	return m_dwFormat;
}


//---------------------------------------------------------
// Function: GetSenderName
// Get sender name.
const char * spoutDX9::GetSenderName()
{
	return m_SenderName;
}

//---------------------------------------------------------
// Function: GetSenderWidth
// Get sender width.
unsigned int spoutDX9::GetSenderWidth()
{
	return m_Width;
}

//---------------------------------------------------------
// Function: GetSenderHeight
// Get sender height.
unsigned int spoutDX9::GetSenderHeight()
{
	return m_Height;

}

//---------------------------------------------------------
// Function: GetSenderFps
// Get sender frame rate.
double spoutDX9::GetSenderFps()
{
	return frame.GetSenderFps();
}

//---------------------------------------------------------
// Function: GetSenderFrame
// Get sender frame number.
long spoutDX9::GetSenderFrame()
{
	return frame.GetSenderFrame();
}

//
// Group: Sender names
//

//---------------------------------------------------------
// Function: GetSenderCount
// Number of senders
int spoutDX9::GetSenderCount()
{
	return sendernames.GetSenderCount();
}

//---------------------------------------------------------
// Function: GetSender
// Sender item name in the sender names set
bool spoutDX9::GetSender(int index, char* sendername, int sendernameMaxSize)
{
	return sendernames.GetSender(index, sendername, sendernameMaxSize);
}

//---------------------------------------------------------
// Function: GetSenderList
// Return a list of current senders
std::vector<std::string> spoutDX9::GetSenderList()
{
	std::vector<std::string> list;
	int nSenders = GetSenderCount();
	if (nSenders > 0) {
		char sendername[256]{};
		for (int i=0; i<nSenders; i++) {
			if (GetSender(i, sendername))
				list.push_back(sendername);
		}
	}
	return list;
}

//---------------------------------------------------------
// Function: GetSenderIndex
// Sender index into the set of names
int spoutDX9::GetSenderIndex(const char* sendername)
{
	return sendernames.GetSenderIndex(sendername);
}

//---------------------------------------------------------
// Function: GetSenderInfo
// Sender information
bool spoutDX9::GetSenderInfo(const char* sendername, unsigned int& width, unsigned int& height, HANDLE& dxShareHandle, DWORD& dwFormat)
{
	return sendernames.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

//---------------------------------------------------------
// Function: GetActiveSender
// Current active sender name
bool spoutDX9::GetActiveSender(char* Sendername)
{
	return sendernames.GetActiveSender(Sendername);
}

//---------------------------------------------------------
// Function: SetActiveSender
// Set sender as active
bool spoutDX9::SetActiveSender(const char* Sendername)
{
	return sendernames.SetActiveSender(Sendername);
}

//---------------------------------------------------------
// Function: GetMaxSenders
// Get user Maximum senders allowed
int spoutDX9::GetMaxSenders()
{
	return(sendernames.GetMaxSenders());
}

//---------------------------------------------------------
// Function: SetMaxSenders
// Set user Maximum senders allowed
void spoutDX9::SetMaxSenders(int maxSenders)
{
	sendernames.SetMaxSenders(maxSenders);
}


//---------------------------------------------------------
// Function: HoldFps
// Frame rate control with desired frames per second.
void spoutDX9::HoldFps(int fps)
{
	frame.HoldFps(fps);
}


//
// Group: Utility
//

//---------------------------------------------------------
// Function: CreateSharedDX9Texture
// Create a shared DirectX9 texture
// by giving it a sharehandle variable - dxShareHandle.
// - for a SENDER   : the sharehandle is NULL and a new texture is created
// - for a RECEIVER : the sharehandle is valid and a handle to the existing shared texture is created
bool spoutDX9::CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle)
{
	if (!pDevice) {
		SpoutLogError("spoutDX9::CreateSharedDX9Texture - NULL DX9 device");
		return false;
	}
	// printf("spoutDX9::CreateSharedDX9Texture(0x%7X, %d, %d, %d, 0x%.7X, 0x%.7X)\n", PtrToUint(pDevice), width, height, format, PtrToUint(dxTexture), LOWORD(dxShareHandle) );

	if (dxTexture) dxTexture->Release();

	LPDIRECT3DTEXTURE9 pTexture = nullptr;
	HANDLE dxHandle = dxShareHandle;

	HRESULT res = pDevice->CreateTexture(width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		format, // default is D3DFMT_A8R8G8B8 - may be set externally
		D3DPOOL_DEFAULT, // Required by interop spec
		&pTexture, // local texture
		&dxHandle);	// local share handle 

	// USAGE may also be D3DUSAGE_DYNAMIC and pay attention to format and resolution!!!
	// USAGE, format and size for sender and receiver must all match
	if (FAILED(res)) {
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDX9::CreateSharedDX9Texture(0x%.7X, %d, %d, %d, 0x%.7X, 0x%.7X)\n", PtrToUint(pDevice), width, height, format, PtrToUint(pTexture), LOWORD(dxHandle));
		strcat_s(tmp, 256, "Error");
		// sprintf_s(tmp, 256, "spoutDX9::CreateSharedDX9Texture error %d (0x%.X) - ", LOWORD(res), LOWORD(res) );
		switch (res) {
		case ERROR_INVALID_PARAMETER:
			strcat_s(tmp, 256, "    ERROR_INVALID_PARAMETER");
			break;
		case D3DERR_INVALIDCALL:
			strcat_s(tmp, 256, "    D3DERR_INVALIDCALL");
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			strcat_s(tmp, 256, "    D3DERR_OUTOFVIDEOMEMORY");
			break;
		case E_OUTOFMEMORY:
			strcat_s(tmp, 256, "    E_OUTOFMEMORY");
			break;
		default:
			strcat_s(tmp, 256, "    Unknown error");
			break;
		}
		SpoutLogFatal("%s", tmp);

		return false;
	}

	dxTexture = pTexture;
	dxShareHandle = dxHandle;

	SpoutLogNotice("spoutDX9::CreateSharedDX9Texture %dx%d - format %d - handle 0x%.7X", width, height, (int)format, LOWORD(dxShareHandle));

	return true;

} // end CreateSharedDX9Texture



//
// Private functions
//


//---------------------------------------------------------
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
		char exepath[256]{};
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
		CreateSharedDX9Texture(m_pDevice, width, height, (D3DFORMAT)m_dwFormat, m_pSharedTexture, m_dxShareHandle);
		// Update the sender and class variables
		sendernames.UpdateSender(m_SenderName, width, height, m_dxShareHandle, dwFormat);
		m_Width = width;
		m_Height = height;
		m_dwFormat = dwFormat;

	} // endif initialization or size checks

	return true;

}

//---------------------------------------------------------
// Write to a DirectX9 system memory surface
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


//---------------------------------------------------------
// COPY FROM A GPU DX9 SURFACE TO THE SHARED DX9 TEXTURE
// The source surface must have been created using the same device as the texture
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
//	o Connect to a sender and inform the application to update texture dimensions
//	o Check for user sender selection
//  o Receive texture details from the sender for write to the user texture
//  o Retrieve width, height, format, share handle and texture pointer
bool spoutDX9::ReceiveSenderData()
{
	m_bUpdated = false;

	// Make sure DirectX is initialized
	if (!OpenDirectX9())
		return false;

	// Initialization is recorded in this class for sender or receiver
	// m_Width or m_Height are established when the receiver connects to a sender
	char sendername[256];
	strcpy_s(sendername, 256, m_SenderName);

	// Check the entered Sender name to see if it exists
	if (sendername[0] == 0) {
		// Passed name was null, so find the active sender
		if (!sendernames.GetActiveSender(sendername))
			return false; // No sender
	}

	// If SpoutPanel has been opened, the active sender name could be different
	if (CheckSpoutPanel(sendername, 256)) {
		// Disable the setup name
		m_SenderNameSetup[0] = 0;
	}

	// Now we have either an existing sender name or the active sender name

	// Save current sender name and dimensions to test for change
	unsigned int width = m_Width;
	unsigned int height = m_Height;
	DWORD dwFormat = m_dwFormat;
	HANDLE dxShareHandle = m_dxShareHandle;

	// Try to get the sender information
	// Retreive width, height, sharehandle and format.
	SharedTextureInfo info;
	if (sendernames.getSharedInfo(sendername, &info)) {

		// Memoryshare not supported (no texture share handle)
		if (info.shareHandle == 0) {
			ReleaseReceiver();
			return false;
		}

		width = info.width;
		height = info.height;
		// For 64 bit
		dxShareHandle = (HANDLE)(LongToHandle((long)info.shareHandle));
		dwFormat = info.format;
		// printf("Found sender (%dx%d) format = %d, handle = 0x%.7X\n", width, height, dwFormat, PtrToUint(dxShareHandle) );

		// Only two sender texture formats can be used.
		//     21 =	D3DFMT_A8R8G8B8
		//     22 = D3DFMT_X8R8G8B8
		// For a D3D11 sender, use the default D3DFMT_A8R8G8B8.
		if (!(dwFormat == D3DFMT_A8R8G8B8 || dwFormat == D3DFMT_X8R8G8B8))
			dwFormat = D3DFMT_A8R8G8B8;

		// Access the sender shared texture

		// The shared texture handle will be different
		//   o for a new sender
		//   o for texture size or format change
		if (dxShareHandle != m_dxShareHandle) {
			// Release everything and start again
			ReleaseReceiver();
			// Update the sender share handle
			m_dxShareHandle = dxShareHandle;
			m_Width = width;
			m_Height = height;
			m_dwFormat = dwFormat;
			// Initialize again with the newly connected sender values
			CreateReceiver(sendername, width, height, dwFormat);
			m_bUpdated = true; // Return to update the receiving texture or image
		}

		// Connected and intialized
		// Sender name, width, height, format, texture pointer and share handle have been retrieved

		// The application can now create a copy of the sender texture using m_dxShareHandle
		return true;

	} // end find sender

	// There is no sender or the connected sender closed
	m_dxShareHandle = nullptr;
	m_Width = 0;
	m_Height = 0;
	m_dwFormat = 0;

	return false;

}


//
// COPY FROM A SENDER'S SHARED TEXTURE TO A DX9 TEXTURE
//
bool spoutDX9::ReadDX9texture(IDirect3DDevice9Ex* pDevice, LPDIRECT3DTEXTURE9 &dxTexture)
{
	if (!pDevice || m_Width == 0 || m_Height == 0 || !m_dxShareHandle)
		return false;


	bool bRet = false;

	// Access the sender shared texture
	if (frame.CheckTextureAccess()) {
		m_bNewFrame = false; // For query of new frame
		// Check if the sender has produced a new frame.
		if (frame.GetNewFrame()) {
			// printf("New frame : sender (%dx%d) format = %d, handle = 0x%.7X\n", m_Width, m_Height, m_dwFormat, PtrToUint(m_dxShareHandle));
			// Create a new texture from the sender's shared texture handle
			if (dxTexture) dxTexture->Release();

			bRet = CreateSharedDX9Texture(pDevice,
				m_Width, m_Height,
				(D3DFORMAT)m_dwFormat,
				dxTexture,
				m_dxShareHandle);
			m_bNewFrame = true; // The application can query IsNewFrame()

			// printf("bRet = %d\n", bRet);

		}
	}
	// Allow access to the shared texture
	frame.AllowTextureAccess();

	return bRet;
}

// Create receiver resources for a new sender
void spoutDX9::CreateReceiver(const char * SenderName, unsigned int width, unsigned int height, DWORD dwFormat)
{
	SpoutLogNotice("CreateReceiver(%s, %d x %d)", SenderName, width, height);

	if (m_bSpoutInitialized)
		ReleaseReceiver();

	// Create a named sender mutex for access to the sender's shared texture
	frame.CreateAccessMutex(SenderName);

	// Enable frame counting to get the sender frame number and fps
	frame.EnableFrameCount(SenderName);

	// Set class globals
	strcpy_s(m_SenderName, 256, SenderName);
	m_Width = width;
	m_Height = height;
	m_dwFormat = dwFormat;

	m_bSpoutInitialized = true;

}


//
// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.
//

//
// Open dialog for the user to select a sender
// Optional message argument
//
bool spoutDX9::SelectSenderPanel(const char* message)
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char path[MAX_PATH]={};
	char drive[MAX_PATH]={};
	char dir[MAX_PATH]={};
	char fname[MAX_PATH]={};
	char UserMessage[512]={};

	if (message && *message) {
		strcpy_s(UserMessage, 512, message); // could be an arg or a user message
	}

	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	path[0] = 0;
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
	}

	if (path[0]) {
		// Does SpoutPanel.exe exist in this path ?
		if(_access(path, 0) == -1) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (_access(path, 0) == -1) {
					return false;
				}
			}
		}
	}
	

	// Check whether the panel is already running
	// Try to open the application mutex.
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
	if (!hMutex1) {
		// No mutex, so not running, so can open it
		// Use ShellExecuteEx so we can test its return value later
		ZeroMemory(&m_ShExecInfo, sizeof(m_ShExecInfo));
		m_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		m_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		m_ShExecInfo.hwnd = NULL;
		m_ShExecInfo.lpVerb = NULL;
		m_ShExecInfo.lpFile = (LPCSTR)path;
		m_ShExecInfo.lpParameters = UserMessage;
		m_ShExecInfo.lpDirectory = NULL;
		m_ShExecInfo.nShow = SW_SHOW;
		m_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&m_ShExecInfo);

		//
		// The flag "m_bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		m_bSpoutPanelOpened = true;

	}
	else {
		// The mutex exists, so another instance is already running.
		// Find the SpoutPanel window and bring it to the top.
		// SpoutPanel is opened as topmost anyway but pop it to
		// the front in case anything else has stolen topmost.
		HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		if (hWnd && IsWindow(hWnd)) {
			SetForegroundWindow(hWnd);
			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		}
		else if (path[0]) {
			// If the window was not found but the mutex exists
			// and SpoutPanel is installed, it has crashed.
			// Terminate the process and the mutex or the mutex will remain
			// and SpoutPanel will not be started again.
			PROCESSENTRY32 pEntry{};
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				SpoutLogError("spoutDX::OpenSpoutPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("spoutDX::OpenSpoutPanel - Process32First error");
					CloseHandle(hProcessSnap);
				}
				else {
					// Look through all processes to find SpoutPanel
					while (hRes && !done) {
#ifdef UNICODE
						_wcsicmp(pEntry.szExeFile, L"SpoutPanel.exe");
#else
						_tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
#endif
						HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, pEntry.th32ProcessID);
						if (hProcess != NULL) {
							// Terminate SpoutPanel and it's mutex if it opened
							TerminateProcess(hProcess, 9);
							CloseHandle(hProcess);
							done = true;
						}

						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = 0; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return true;

} // end SelectSenderPanel

//
// Check whether SpoutPanel opened and return the new sender name
//
bool spoutDX9::CheckSpoutPanel(char *sendername, int maxchars)
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if (m_bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if (!m_bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if (hMutex) m_bSpoutPanelActive = true;
		}
		else if (!hMutex) { // It has now closed
			m_bSpoutPanelOpened = false; // Don't do this part again
			m_bSpoutPanelActive = false;
			// call GetExitCodeProcess() with the hProcess member of
			// global SHELLEXECUTEINFO to get the exit code from SpoutPanel
			if (m_ShExecInfo.hProcess) {
				GetExitCodeProcess(m_ShExecInfo.hProcess, &dwExitCode);
				// Only act if exit code = 0 (OK)
				if (dwExitCode == 0) {
					// SpoutPanel has been activated and OK clicked
					// Test the active sender which should have been set by SpoutPanel
					newname[0] = 0;
					if (!sendernames.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (sendernames.getSharedInfo(newname, &TextureInfo)) {
									// If not already registered
									if (!sendernames.FindSenderName(newname)) {
										// Register in the list of senders and make it the active sender
										sendernames.RegisterSenderName(newname);
										sendernames.SetActiveSender(newname);
									}
								}
							}
						}
					}
					// Now do we have a valid sender name ?
					if (newname[0] != 0) {
						// Pass back the new name
						strcpy_s(sendername, maxchars, newname);
						bRet = true;
					} // endif valid sender name
				} // endif SpoutPanel OK
			} // got the exit code
		} // endif no mutex so SpoutPanel has closed
		// If we opened the mutex, close it now or it is never released
		if (hMutex) CloseHandle(hMutex);
		return bRet;
	} // SpoutPanel has not been opened

	return false;

}
