//
//		SpoutDX
//
//		Send a DirectX11 shared texture
//		Receive from a Spout sender DirectX11 shared texture
//		DirectX9 not supported
//
// ====================================================================================
//		Revisions :
//
//		07.06.19	- started DirectX helper class
//		17.10.19	- For revison of SpoutCam from OpenGL to DirectX
//					  Added DirectX initialise and release functions
//					  Added support for pixel read via staging texture
//					  Added pixel copy functions
//					  Added memoryshare support
//		30.01.20	- Simplify names for Get received texture methods
//					  Revise CreateDX11Texture for user flags
//					  Add SetTextureFlags
//		07.01.20	- Correct immediate context in SendTexture
//		13.04.20	- Revise receiver methods
//		10.05.20	- Corrected format for ReceivedTexture and GetSenderFormat
//					  Cleanup Tutorial07 example code
//		05.06.20	- Get memoryshare mode from registry in constructor
//		06.06.20	- Working memoryshare
//		07.06.20	- Use application device exclusively
//					  Strip back to basic texture share only for simplicity
//					  Memoryshare use unlikely for DirectX applications
//					  Can be restored on request
//
// ====================================================================================
/*
	Copyright (c) 2014-2020, Lynn Jarvis. All rights reserved.

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
#include "spoutDX.h"

spoutDX::spoutDX()
{
	// Initialize variables
	m_pd3dDevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pReceivedTexture = nullptr;
	m_pSharedTexture = nullptr;
	m_dxShareHandle = NULL;
	m_SenderNameSetup[0] = 0;
	m_SenderName[0] = 0;
	m_dwFormat = DXGI_FORMAT_B8G8R8A8_UNORM; // default;
	m_Width = 0;
	m_Height = 0;
	m_bUpdated = false;
	m_bConnected = false;
	m_bNewFrame = false;
	m_bSpoutInitialized = false;
	m_bSpoutPanelOpened = false;
	m_bSpoutPanelActive = false;

}

spoutDX::~spoutDX()
{
	if (m_pReceivedTexture)
		m_pReceivedTexture->Release();
	ReleaseSender();
	ReleaseReceiver();
}


//---------------------------------------------------------
//
// SENDER
//

bool spoutDX::SetSenderName(const char* sendername)
{
	if (!sendername[0])
		return false;
	strcpy_s(m_SenderName, 256, sendername);
	return true;
}

//
// Send a texture
//
// DX9 compatible
// DXGI_FORMAT_R8G8B8A8_UNORM; // default DX11 format - compatible with DX9 (28)
// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
//
// Other formats that work with DX11 but not with DX9
// DXGI_FORMAT_R16G16B16A16_FLOAT
// DXGI_FORMAT_R16G16B16A16_SNORM
// DXGI_FORMAT_R10G10B10A2_UNORM
//
bool spoutDX::SendTexture(ID3D11Device* pDevice, ID3D11Texture2D* pTexture)
{
	if (!m_SenderName[0] || !pDevice || !pTexture)
		return false;

	// Get the texture details
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pTexture->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0)
		return false;

	if (!m_bSpoutInitialized) {

		// Set globals for the application device
		m_pd3dDevice = pDevice;
		pDevice->GetImmediateContext(&m_pImmediateContext);
		if (!m_pImmediateContext)
				return false;

		// Save width and height to test for sender size changes
		m_Width = desc.Width;
		m_Height = desc.Height;
		m_dwFormat = (DWORD)desc.Format;

		// Create a shared texture for the sender
		spoutdx.CreateSharedDX11Texture(m_pd3dDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		// Create a sender, specifying the same texture format
		m_bSpoutInitialized = spoutsender.CreateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, (DWORD)desc.Format);
		// Create a sender mutex for access to the shared texture
		frame.CreateAccessMutex(m_SenderName);
		// Enable frame counting so the receiver gets frame number and fps
		frame.EnableFrameCount(m_SenderName);

	}
	// Initialized but has the source texture changed size ?
	else if (m_Width != desc.Width || m_Height != desc.Height) {
		m_Width = desc.Width;
		m_Height = desc.Height;
		// Re-create the class shared texture to the new size
		if (m_pSharedTexture) m_pSharedTexture->Release();
		spoutdx.CreateSharedDX11Texture(m_pd3dDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		// Update the sender	
		spoutsender.UpdateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle);
	} // endif initialization or size checks

	// Check the sender mutex for access the shared texture
	if (frame.CheckTextureAccess()) {
		// Copy the texture to the sender's shared texture
		m_pImmediateContext->CopyResource(m_pSharedTexture, pTexture);
		// CopyResource is asynchronous.
		// Here we can wait for it to complete so the
		// receiver can read the new data straight away.
		// Test before use. See comments in the FlushWait function.
		// spoutdx.FlushWait(m_pd3dDevice, m_pImmediateContext);
		// Signal a new frame while the mutex is locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowTextureAccess();
	}

	return true;
}

void spoutDX::ReleaseSender()
{
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (m_bSpoutInitialized)
		spoutsender.ReleaseSenderName(m_SenderName);

	m_pSharedTexture = nullptr;
	m_dxShareHandle = NULL;
	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	m_bSpoutInitialized = false;

}

unsigned int spoutDX::GetWidth()
{
	return m_Width;
}

unsigned int spoutDX::GetHeight()
{
	return m_Height;
}

double spoutDX::GetFps()
{
	return (frame.GetSenderFps());
}

long spoutDX::GetFrame()
{
	return (frame.GetSenderFrame());
}

//---------------------------------------------------------
//
// RECEIVER
//


// Set the sender name to connect to
void spoutDX::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
	}
}


// Receive a texture
bool spoutDX::ReceiveTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D** ppTexture)
{
	ID3D11Texture2D* pTexture = nullptr; // The receiving texture pointer

	// Set the initial width and height to current globals.
	// width and height are returned from the sender.
	unsigned int width = m_Width;
	unsigned int height = m_Height;
	DWORD dwFormat = 0;
	HANDLE dxShareHandle = NULL;

	// If SpoutPanel has been opened, the sender name will be different.
	CheckSpoutPanel(m_SenderName, 256);

	// Find if the sender exists.
	// For an empty name string, the active sender is returned.
	if (spoutsender.FindSender(m_SenderName, width, height, dxShareHandle, dwFormat)) {

		// Check for texture size changes
		if (m_Width != width || m_Height != height) {

			// printf("Changed sender size %dx%d to %dx%d\n", m_Width, m_Height, width, height);

			// If no texture pointer is passed in
			// create or re-create a receiving texture
			if (!ppTexture) {
				if(!CreateDX11texture(pd3dDevice,
					width, height,
					(DXGI_FORMAT)dwFormat, // Format is the same as the sender (default bgra)
					&m_pReceivedTexture)) {
					return false;
				}
				// m_pReceivedTexture is finally released in the class destructor
			}
			else {
				// If an application texture is used,
				// GetTexture will return a null pointer
				m_pReceivedTexture = nullptr;
			}

			// Set the texture format
			m_dwFormat = dwFormat;

			// Set the sender share handle
			m_dxShareHandle = dxShareHandle;

			// Update global variables for subsequent size checks
			m_Width = width;
			m_Height = height;

			// Inform the receiving application that the texture has changed
			m_bUpdated = true;

			// If a pointer was passed in, return to update the receiving texture
			if(ppTexture)
				return true;

			// If not, continue to copy the sender's shared texture.
			// The application can still detect change with IsUpdated()
			// to use the new received texture

		}

		if (!ppTexture)
			pTexture = m_pReceivedTexture; // Use the class receiving texture
		else
			pTexture = *ppTexture; // Use the texture passed in

		// Initialize after finding a sender
		if (!m_bSpoutInitialized) {
			// Open a named mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);
			m_bSpoutInitialized = true;
		}

		// Retrieve the sender's shared texture using the share handle
		CopySenderTexture(pd3dDevice, pTexture, dxShareHandle);

		// Found the sender
		m_bConnected = true;
		return true;

	} // sender exists
	else {

		// Sender not found or the connected sender closed
		if (m_bSpoutInitialized) {
			// Close the named access mutex and frame counting
			frame.CloseAccessMutex();
			frame.CleanupFrameCount();
			// Zero width and height so that they are reset when a sender is found
			m_Width = 0;
			m_Height = 0;
		}
		m_bConnected = false;
		// Initialize again when a sender is found
		m_bSpoutInitialized = false;
	}

	return false;
}

void spoutDX::ReleaseReceiver()
{
	// Close the named access mutex and frame counting semaphore.
	frame.CloseAccessMutex();
	frame.CleanupFrameCount();

	// Restore the sender name if one was specified
	// by the application in SetReceiverName
	if (m_SenderNameSetup[0])
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);
	else
		m_SenderName[0] = 0;

	// Initialize again when a sender is found
	m_bSpoutInitialized = false;
	m_bUpdated = false;

}

//---------------------------------------------------------
// Check for sender change
//  If updated, the application must update the receiving texture
//  before the next call to ReceiveTexture
bool spoutDX::IsUpdated()
{
	bool bRet = m_bUpdated;
	m_bUpdated = false; // Reset the update flag
	return bRet;
}

bool spoutDX::IsConnected()
{
	return m_bConnected;
}

bool spoutDX::IsFrameNew()
{
	return m_bNewFrame;
}

ID3D11Texture2D* spoutDX::GetTexture()
{
	return m_pReceivedTexture;
}

HANDLE spoutDX::GetSenderHandle()
{
	return m_dxShareHandle;
}

DXGI_FORMAT spoutDX::GetSenderFormat()
{
	return (DXGI_FORMAT)m_dwFormat;
}

const char * spoutDX::GetSenderName()
{
	return m_SenderName;
}

unsigned int spoutDX::GetSenderWidth()
{
	return m_Width;
}

unsigned int spoutDX::GetSenderHeight()
{
	return m_Height;

}

double spoutDX::GetSenderFps()
{
	return frame.GetSenderFps();
}

long spoutDX::GetSenderFrame()
{
	return frame.GetSenderFrame();
}

int spoutDX::GetSenderCount()
{
	std::set<std::string> SenderNameSet;
	if (spoutsender.GetSenderNames(&SenderNameSet)) {
		return((int)SenderNameSet.size());
	}
	return 0;
}

// Get a sender name given an index into the sender names set
bool spoutDX::GetSender(int index, char* sendername, int sendernameMaxSize)
{
	std::set<std::string> SenderNameSet;
	std::set<std::string>::iterator iter;
	std::string namestring;
	char name[256];
	int i;

	if (spoutsender.GetSenderNames(&SenderNameSet)) {
		if (SenderNameSet.size() < (unsigned int)index) {
			return false;
		}
		i = 0;
		for (iter = SenderNameSet.begin(); iter != SenderNameSet.end(); iter++) {
			namestring = *iter; // the name string
			strcpy_s(name, 256, namestring.c_str()); // the 256 byte name char array
			if (i == index) {
				strcpy_s(sendername, sendernameMaxSize, name); // the passed name char array
				break;
			}
			i++;
		}
		return true;
	}
	return false;
}

bool spoutDX::GetActiveSender(char* Sendername)
{
	return spoutsender.GetActiveSender(Sendername);
}

bool spoutDX::SetActiveSender(const char* Sendername)
{
	return spoutsender.SetActiveSender(Sendername);
}

//---------------------------------------------------------
//
// Receiver utilities
//

// Copy from a sender shared texture (DX9 sender supported)
bool spoutDX::CopySenderTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D* pTexture, HANDLE shareHandle)
{
	// No shared texture handle for memoryshare mode
	if (!pTexture || !shareHandle)
		return false;

	m_bNewFrame = false; // For query of new frame

	// Get a pointer to the sender's shared texture via the share handle
	ID3D11Texture2D* pSharedTexture = nullptr;
	if (spoutdx.OpenDX11shareHandle(pd3dDevice, &pSharedTexture, shareHandle)) {
		// Check for access to it. Keyed mutex texture supported.
		if (frame.CheckTextureAccess(pSharedTexture)) {
			// Copy if the sender has produced a new frame
			if (frame.GetNewFrame()) {
				// Use the sender's shared texture pointer for copy to the class texture
				ID3D11DeviceContext* pImmediateContext = nullptr;
				pd3dDevice->GetImmediateContext(&pImmediateContext);
				if (pImmediateContext) {
					pImmediateContext->CopyResource(pTexture, pSharedTexture);
					pImmediateContext->Release();
				}
				m_bNewFrame = true;
				frame.AllowTextureAccess(pSharedTexture);
				return true;
			}
			frame.AllowTextureAccess(pSharedTexture);
		}
	}
	return false;
}


//---------------------------------------------------------
//
// COMMON
//

void spoutDX::HoldFps(int fps)
{
	frame.HoldFps(fps);
}

void spoutDX::DisableFrameCount()
{
	frame.DisableFrameCount();
}

bool spoutDX::IsFrameCountEnabled()
{
	return frame.IsFrameCountEnabled();
}

bool spoutDX::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spoutsender.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

bool spoutDX::GetDX9()
{
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);
	return (dwDX9 == 1);
}

bool spoutDX::GetMemoryShareMode()
{
	bool bRet = false;
	DWORD dwMem = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem)) {
		bRet = (dwMem == 1);
	}
	return bRet;
}

int spoutDX::GetNumAdapters()
{
	return spoutdx.GetNumAdapters();
}

bool spoutDX::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spoutdx.GetAdapterName(index, adaptername, maxchars);
}

int spoutDX::GetAdapter()
{
	return spoutdx.GetAdapter();
}

bool spoutDX::SetAdapter(int index)
{
	if (spoutdx.SetAdapter(index)) {
		return true;
	}
	SpoutLogError("spoutDX::SetAdapter(%d) failed", index);
	spoutdx.SetAdapter(-1); // make sure globals are reset to default
	return false;
}

int spoutDX::GetMaxSenders()
{
	return(spoutsender.GetMaxSenders());
}

void spoutDX::SetMaxSenders(int maxSenders)
{
	spoutsender.SetMaxSenders(maxSenders);
}

bool spoutDX::CreateDX11texture(ID3D11Device* pDevice,
	unsigned int width, unsigned int height,
	DXGI_FORMAT format,
	ID3D11Texture2D** ppTexture,
	HANDLE *shareHandle)
{
	HANDLE sh = NULL;
	// If a sharehandle has been specified, pass it back
	if(shareHandle)
		spoutdx.CreateSharedDX11Texture(pDevice, width, height, format, ppTexture, *shareHandle);
	else
		spoutdx.CreateSharedDX11Texture(pDevice, width, height, format, ppTexture, sh);
	return *ppTexture;
}

//
// PRIVATE
//

//
// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.
//

//
// Check whether SpoutPanel opened and return the new sender name
//
bool spoutDX::CheckSpoutPanel(char *sendername, int maxchars)
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
					if (!spoutsender.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (spoutsender.getSharedInfo(newname, &TextureInfo)) {
									// Register in the list of senders and make it the active sender
									spoutsender.RegisterSenderName(newname);
									spoutsender.SetActiveSender(newname);
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

//
// Pop up SpoutPanel to allow the user to select a sender
// activated by RH click in this application
//
void spoutDX::SelectSender()
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
		// Does SpoutPanel.exe exist in this path ?
		if (!PathFileExistsA(path)) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (!PathFileExistsA(path)) {
					SpoutLogWarning("spoutDX::SelectSender - SpoutPanel path not found");
					return;
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
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
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
					// Look through all processes
					while (hRes && !done) {
						int value = _tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
						if (value == 0) {
							HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
							if (hProcess != NULL) {
								// Terminate SpoutPanel and it's mutex
								TerminateProcess(hProcess, 9);
								CloseHandle(hProcess);
								done = true;
							}
						}
						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = NULL; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return;

} // end SelectSender






