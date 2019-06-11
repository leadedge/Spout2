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
//
// ====================================================================================
/*
	Copyright (c) 2014-2019, Lynn Jarvis. All rights reserved.

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
	m_pSenderTexture = nullptr;
	m_pSharedTexture = nullptr;
	m_dxShareHandle = NULL;
	m_SenderNameSetup[0] = 0;
	m_SenderName[0] = 0;
	m_dwFormat = 0;
	m_Width = 0;
	m_Height = 0;
	bSpoutUpdated = false;
	bSpoutInitialized = false;
	bSpoutConnected = false;
	bSpoutPanelOpened = false;
	bSpoutPanelActive = false;

}

spoutDX::~spoutDX()
{
	CloseSender();
	CloseReceiver();
}


//---------------------------------------------------------
//
// SENDER
//

//
// Compatible formats
//
// DX9 compatible formats
// DXGI_FORMAT_R8G8B8A8_UNORM; // default DX11 format - compatible with DX9 (28)
// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
//
// Other formats that work with DX11 but not with DX9
// DXGI_FORMAT_R16G16B16A16_FLOAT
// DXGI_FORMAT_R16G16B16A16_SNORM
// DXGI_FORMAT_R10G10B10A2_UNORM
//
bool spoutDX::SendTexture(const char* sendername, ID3D11Device* pDevice, ID3D11Texture2D* pTexture)
{
	D3D11_TEXTURE2D_DESC desc;
	ID3D11DeviceContext* pImmediateContext = nullptr;

	if (!sendername[0] || !pDevice || !pTexture)
		return false;

	// Get the texture details
	ZeroMemory(&desc, sizeof(desc));
	pTexture->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0)
		return false;

	// If a sender has not been initialized yet, create one
	if (!bSpoutInitialized) {

		// Save width and height to test for sender size changes
		m_Width = desc.Width;
		m_Height = desc.Height;

		// Create a local shared texture the same size and format as the texture
		spoutdx.CreateSharedDX11Texture(pDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);

		// Create a sender, specifying the same texture format
		strcpy_s(m_SenderName, 256, sendername);
		bSpoutInitialized = spoutsender.CreateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, (DWORD)desc.Format);

		// Create a sender mutex for access to the shared texture
		frame.CreateAccessMutex(m_SenderName);

		// Enable frame counting so the receiver gets frame number and fps
		frame.EnableFrameCount(m_SenderName);

	}
	// Otherwise check for change of render size
	else if (m_Width != desc.Width || m_Height != desc.Height) {
		// Initialized but has the source texture changed size ?
		m_Width = desc.Width;
		m_Height = desc.Height;
		// Release and re-create the local shared texture to match
		m_pSharedTexture->Release();
		spoutdx.CreateSharedDX11Texture(pDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		// Update the sender	
		spoutsender.UpdateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle);
	}

	// Access the sender shared texture
	if (frame.CheckAccess()) {
		// Copy the texture to the sender's shared texture
		pDevice->GetImmediateContext(&pImmediateContext);
		pImmediateContext->CopyResource(m_pSharedTexture, pTexture);
		// Flush and wait until CopyResource is finished
		// so that the receiver can read this frame.
		spoutdx.FlushWait(pDevice, pImmediateContext);
		// Signal a new frame while the mutex is still locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowAccess();
	}

	return true;
}

void spoutDX::CloseSender()
{
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (bSpoutInitialized)
		spoutsender.ReleaseSenderName(m_SenderName);

	m_pSharedTexture = nullptr;
	m_dxShareHandle = NULL;
	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	bSpoutInitialized = false;

}

void spoutDX::HoldFps(int fps)
{
	frame.HoldFps(fps);
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

bool spoutDX::ReceiveTexture(ID3D11Device* pd3dDevice)
{
	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		CloseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			CloseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);
			// Create a local texture of the same size on the application device
			// for copy from the shared texture.
			// The two textures must have compatible DXGI formats.
			// Default shared texture format is DXGI_FORMAT_B8G8R8A8_UNORM.
			// https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
			if (m_pSenderTexture) m_pSenderTexture->Release();
			m_pSenderTexture = nullptr;
			CreateDX11Texture(pd3dDevice, m_Width, m_Height, (DXGI_FORMAT)m_dwFormat, &m_pSenderTexture);
			bSpoutInitialized = true;
			// printf("Updated [%s] %dx%d\n", m_SenderName, m_Width, m_Height);
			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			bSpoutUpdated = true;
		}

		// Access the sender shared texture
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame.
			// This function must be called within a sender mutex lock so the sender does not
			// write a frame and increment the frame count while a receiver is reading it.
			if (frame.GetNewFrame()) {
				if (spoutdx.OpenDX11shareHandle(pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
					// Use the shared texture pointer for copy to the local texture.
					// Copy during the mutex lock for sole access to the shared texture.
					if (m_pSenderTexture) {
						ID3D11DeviceContext* pImmediateContext = nullptr;
						pd3dDevice->GetImmediateContext(&pImmediateContext);
						if(pImmediateContext)
							pImmediateContext->CopyResource(m_pSenderTexture, m_pSharedTexture);
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}
		bSpoutConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		CloseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		bSpoutConnected = false;
	}

	return bSpoutConnected;

}

// Return the sender texture copy
ID3D11Texture2D* spoutDX::GetSenderTexture()
{
	return m_pSenderTexture;
}

DXGI_FORMAT spoutDX::GetSenderTextureFormat()
{
	return (DXGI_FORMAT)m_dwFormat;
}

void spoutDX::CloseReceiver()
{
	// Release the local texture
	if (m_pSenderTexture) m_pSenderTexture->Release();
	m_pSenderTexture = nullptr;

	// Close the named access mutex and frame counting semaphore.
	frame.CloseAccessMutex();
	frame.CleanupFrameCount();

	// Restore the sender name if one was specified
	// by the application in SetReceiverName
	if(m_SenderNameSetup[0])
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);

	// Initialize again when a sender is found
	bSpoutInitialized = false;
	bSpoutUpdated = false;

}

void spoutDX::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
	}
}

bool spoutDX::IsUpdated()
{
	bool bRet = bSpoutUpdated;
	// Reset the update flag
	bSpoutUpdated = false;
	return bRet;
}

bool spoutDX::IsConnected()
{
	return bSpoutConnected;
}

//---------------------------------------------------------
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
// COMMON
//

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

bool spoutDX::GetMemoryShareMode()
{
	bool bRet = false;
	DWORD dwMem = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem)) {
		bRet = (dwMem == 1);
		SpoutLogWarning("spoutDX::GetMemoryShareMode() - not supported for DirectX");
	}
	return bRet;
}

bool spoutDX::SetMemoryShareMode(bool bMem)
{
	if (WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", (DWORD)bMem)) {
		SpoutLogWarning("spoutDX::SetMemoryShareMode() - not supported for DirectX");
		return true;
	}
	return false;
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
}

int spoutDX::GetMaxSenders()
{
	return(spoutsender.GetMaxSenders());
}

void spoutDX::SetMaxSenders(int maxSenders)
{
	spoutsender.SetMaxSenders(maxSenders);
}



//
// PRIVATE
//

// Create a texture that is not shared
bool spoutDX::CreateDX11Texture(ID3D11Device* pd3dDevice,
	unsigned int width, unsigned int height,
	DXGI_FORMAT format, ID3D11Texture2D** ppTexture)
{
	ID3D11Texture2D* pTexture = nullptr;
	DXGI_FORMAT texformat = DXGI_FORMAT_B8G8R8A8_UNORM;

	if (format != 0)
		texformat = format;

	if (pd3dDevice == NULL) {
		SpoutLogFatal("spoutDX::CreateDX11Texture NULL device");
		return false;
	}

	SpoutLogNotice("spoutDX::CreateDX11Texture");
	SpoutLogNotice("    pDevice = 0x%Ix, width = %d, height = %d, format = %d", (intptr_t)pd3dDevice, width, height, format);

	// Create a new DX11 texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0; // This texture will not be shared
	desc.Format = texformat; // Default DXGI_FORMAT_B8G8R8A8_UNORM
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (res != S_OK) {
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDX::CreateDX11Texture ERROR - [0x%x] : ", res);
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
		default:
			strcat_s(tmp, 256, "Unlisted error");
			break;
		}
		SpoutLogFatal("%s", tmp);
		return false;
	}

	// Return the DX11 texture pointer
	*ppTexture = pTexture;

	SpoutLogNotice("    pTexture = 0x%Ix", pTexture);

	return true;

}

// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.

//
// Check whether SpoutPanel opened and return the new sender name
//
bool spoutDX::CheckSpoutPanel(char *sendername, int maxchars)
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if (bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if (!bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if (hMutex) bSpoutPanelActive = true;
		}
		else if (!hMutex) { // It has now closed
			bSpoutPanelOpened = false; // Don't do this part again
			bSpoutPanelActive = false;
			// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
			// to get the exit code from SpoutPanel
			if (g_ShExecInfo.hProcess) {
				GetExitCodeProcess(g_ShExecInfo.hProcess, &dwExitCode);
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
		ZeroMemory(&g_ShExecInfo, sizeof(g_ShExecInfo));
		g_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		g_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		g_ShExecInfo.hwnd = NULL;
		g_ShExecInfo.lpVerb = NULL;
		g_ShExecInfo.lpFile = (LPCSTR)path;
		g_ShExecInfo.lpDirectory = NULL;
		g_ShExecInfo.nShow = SW_SHOW;
		g_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&g_ShExecInfo);
		Sleep(125); // allow time for SpoutPanel to open nominally 0.125s
		//
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		bSpoutPanelOpened = true;
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






