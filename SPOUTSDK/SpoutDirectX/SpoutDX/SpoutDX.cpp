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
//		09.06.20	- Updated 2.007 Spout SDK files
//					  Restored ReceiveRGBIMage and staging texture functions
//					  for DirectX version of SpoutCam
//		21.06.20	- Create basic windows example using SpoutDX class
//		22.06.20	- Add ReceiveImage and ReadRGBApixels - see also SpoutCopy
//		23.06.20	- SetSenderName revision and testing
//					  Clean up
//		25.06.20	- Include texture format in update checks
//		26.06.20	- Revise ReceiveTexture
//					  Remove local receiving texture
//					  Remove CreateDX11Texture
//				      More SetSenderName revision and testing
//					  General update throughout
//		26.06.20	- Restore revised CreateDX11texture
//		28.06.17	- Remove ReadRGBApixels
//					  Change to allow class or application device
//		30.06.20	- Due to hesitions with SpoutCam, move flush for staging texture map
//					  from ReceiveRGBimage to ReadRGBpixels and added to ReceiveImage.
//		03.07.20	- Change OpenDirectX11 so it can be repeatedly called (see ReceiveImage)
//		04.07.20	- OpenDirectX11 check in all sending functions
//		09.07.20	- Correct ReadRGBpixels for RGBA senders
//		02.08.20	- Remove console print from ReadRGBApixels
//		02.09.20	- Revise ReceiveRGBimage, ReadRGBpixels for SpoutCam 
//					  Only use OpenDX11shareHandle if the sender texture changes size
//					  to avoid a memory leak (https://github.com/leadedge/SpoutCam/issues/2)
//					  Release m_pSharedTexture in all close functions
//		08.09.20	- Revise ReceiveTexture and ReceiveImage to avoid repeated OpenDX11shareHandle
//					  Revise ReceiveImage due to problems if the staging texture remains mapped
//					  SendImage do not release immediate context
//					  Made all class objects public
//		18.09.20	- Add GetSenderAdapter and SetSenderAdapter
//		21.09.20	- Add sender IsInitialized and GetName
//					  ReceiveTexture, ReceiveImage and ReceiveRGBimage -
//					  retrieve a new shared texture pointer in ReceiveSenderData() rather than afterwards
//					  SendTexture - set the current adapter index after creating sender
//					  Prevent sharing if the sender texture was created on a different adapter
//					  Some protections in GetSenderAdapter
//		23.09.20	- GetSenderAdapter : return -1 on failure. Remove logs due to repeats.
//					  ReceiveSenderData() : catch exception for OpenDX11shareHandle failure
//					  Initialize m_SenderInfo and m_ShExecInfo in constructor
//		24.09.20	- Move try/catch to OpenDX11shareHandle in SpoutDirectX class
//		25.09.20	- Clean up ReceiveSenderData to allow for failure of OpenDX11shareHandle
//					  Compare share handle to detect a new sender instead of the name
//		02.10.20	- Async readback from GPU using two staging textures
//		04.10.20	- Correct CheckStagingTextures for size change
//					- Correct ReceiveSenderData to release texture before OpenSharedResource
//					- Replace ReceiveRGBimage with ReceiveImage and rgb flag
//					- Replace ReadRGBAimage, ReadRGBimage, ReadRGBApixels, ReadRGBpixels
//					  with ReadPixelData and rgb flag
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
	m_pStaging[0] = nullptr;
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;
	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;
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
	m_bClassDevice = false;
	ZeroMemory(&m_SenderInfo, sizeof(SharedTextureInfo));
	ZeroMemory(&m_ShExecInfo, sizeof(m_ShExecInfo));

}

spoutDX::~spoutDX()
{
	ReleaseSender();
	ReleaseReceiver();
	CleanupDX11();
}

//---------------------------------------------------------
// DIRECTX
//

// Initialize and prepare Directx 11
bool spoutDX::OpenDirectX11(ID3D11Device* pDevice)
{
	if (!m_pd3dDevice) {
		SpoutLogNotice("spoutDX::OpenDirectX11()");
		if (pDevice) {
			m_pd3dDevice = pDevice;
			m_bClassDevice = false; // An existing device pointer was used
		}
		else {
			// Create a DirectX 11 device if not already
			m_pd3dDevice = spoutdx.CreateDX11device();
			if (!m_pd3dDevice)
				return false;
			m_bClassDevice = true; // A new class device pointer was created
		}
		// Retrieve the class context pointer
		m_pd3dDevice->GetImmediateContext(&m_pImmediateContext);
	}

	return true;

}

ID3D11Device* spoutDX::GetDevice()
{
	return m_pd3dDevice;
}

void spoutDX::CleanupDX11()
{
	SpoutLogNotice("spoutDX::CleanupDX11()");


	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (m_pStaging[0]) m_pStaging[0]->Release();
	if (m_pStaging[1]) m_pStaging[1]->Release();
	m_pStaging[0] = nullptr;
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;

	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	if (m_pImmediateContext)
		m_pImmediateContext->Release();
	m_pImmediateContext = nullptr;

	// A device pointer exists and a class device was created
	if (m_pd3dDevice && m_bClassDevice) {
		unsigned long refcount = spoutdx.ReleaseDX11Device(m_pd3dDevice);
		if (refcount > 0)
			SpoutLogWarning("CleanupDX11:ReleaseDX11Device - refcount = %d", refcount);
	}
	m_pd3dDevice = nullptr;

}

//---------------------------------------------------------
// SENDER
//

bool spoutDX::SetSenderName(const char* sendername)
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
	return true;
}


void spoutDX::SetSenderFormat(DXGI_FORMAT format)
{
	m_dwFormat = format;
}

void spoutDX::ReleaseSender()
{
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (m_pStaging[0]) m_pStaging[0]->Release();
	if (m_pStaging[1]) m_pStaging[1]->Release();

	if (m_bSpoutInitialized)
		spoutsender.ReleaseSenderName(m_SenderName);

	m_pStaging[0] = nullptr;
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;
	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	m_bSpoutInitialized = false;

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
bool spoutDX::SendTexture(ID3D11Texture2D* pTexture)
{
	// Quit if no data
	if (!pTexture)
		return false;

	// Make sure DirectX is initialized
	if (!OpenDirectX11())
		return false;

	// The sender needs a name
	if (!m_SenderName[0])
		SetSenderName();

	// Get the texture details
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pTexture->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0)
		return false;

	if (!m_bSpoutInitialized) {

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
		m_pSharedTexture = nullptr;
		m_dxShareHandle = nullptr;
		spoutdx.CreateSharedDX11Texture(m_pd3dDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		// Update the sender	
		spoutsender.UpdateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, desc.Format);
	} // endif initialization or size checks

	// Check the sender mutex for access the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Copy the texture to the sender's shared texture
		m_pImmediateContext->CopyResource(m_pSharedTexture, pTexture);
		// Flush the command queue now because the shared texture has been updated on this device
		m_pImmediateContext->Flush();
		// Signal a new frame while the mutex is locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;
}

bool spoutDX::SendImage(unsigned char * pData, unsigned int width, unsigned int height)
{
	// Quit if no data
	if (!pData)
		return false;

	// Make sure DirectX is initialized
	if(!OpenDirectX11())
		return false;

	// The sender needs a name
	if (!m_SenderName[0])
		SetSenderName();

	if (!m_bSpoutInitialized) {
		// Create a class shared texture
		// Default texture format is DXGI_FORMAT_B8G8R8A8_UNORM or set by SenderFormat
		if (spoutdx.CreateSharedDX11Texture(m_pd3dDevice, width, height, (DXGI_FORMAT)m_dwFormat, &m_pSharedTexture, m_dxShareHandle)) {
			// Create a sender with the same format
			if (spoutsender.CreateSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {
				// Save width and height to test for sender size changes
				m_Width = width;
				m_Height = height;
				// Create a sender mutex for access to the shared texture
				frame.CreateAccessMutex(m_SenderName);
				// Enable frame counting so the receiver gets frame number and fps
				frame.EnableFrameCount(m_SenderName);
				m_bSpoutInitialized = true;
			}
		}
		if (!m_bSpoutInitialized) {
			if (m_pSharedTexture) {
				m_pSharedTexture->Release();
				m_pSharedTexture = nullptr;
				m_dxShareHandle = nullptr;
			}
			return false;
		}
	}
	// Initialized but has the sender changed size ?
	else if (m_Width != width || m_Height != height) {
		m_Width = width;
		m_Height = height;
		// Re-create the shared texture with the new size
		if (m_pSharedTexture) m_pSharedTexture->Release();
		m_pSharedTexture = nullptr;
		m_dxShareHandle = nullptr;
		spoutdx.CreateSharedDX11Texture(m_pd3dDevice, m_Width, m_Height, (DXGI_FORMAT)m_dwFormat, &m_pSharedTexture, m_dxShareHandle);
		// Update the sender	
		spoutsender.UpdateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, m_dwFormat);
	} // endif initialization or size checks

	// Check the sender mutex for access the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Update the shared texture resource with the pixel buffer
		m_pImmediateContext->UpdateSubresource(m_pSharedTexture, 0, NULL, pData, m_Width * 4, 0);
		// Flush the command queue because the shared texture has been updated on this device
		m_pImmediateContext->Flush();
		// Signal a new frame while the mutex is locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;
}

bool spoutDX::IsInitialized()
{
	return m_bSpoutInitialized;
}

const char * spoutDX::GetName()
{
	return m_SenderName;
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

void spoutDX::ReleaseReceiver()
{
	// Restore the sender name if one was specified by SetReceiverName
	if (m_SenderNameSetup[0])
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);
	else
		m_SenderName[0] = 0;

	if (!m_bSpoutInitialized)
		return;

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
	m_SenderName[0] = 0;
	
	// Initialize again when a sender is found
	m_bSpoutInitialized = false;
	m_bUpdated = false;

}

bool spoutDX::ReceiveTexture(ID3D11Texture2D** ppTexture)
{
	// Quit if no receiving texture pointer
	if (!ppTexture) {
		return false;
	}

	// Make sure DirectX is initialized
	if (!OpenDirectX11()) {
		return false;
	}

	// Try to receive texture details from a sender
	if (ReceiveSenderData()) {

		// Was the shared texture pointer retrieved ?
		if (!m_pSharedTexture) {
			return false;
		}

		// The sender name, width, height, format, shared texture handle and pointer have been retrieved.
		if (m_bUpdated) {
			// If the sender is new or changed, return to update the receiving texture.
			// The application detects the change with IsUpdated().
			return true;
		}

		// The application receiving texture is created on the first update above
		ID3D11Texture2D* pTexture = *ppTexture;
		if (!pTexture) {
			return false;
		}

		//
		// Found a sender
		//

		// Access the sender shared texture
		if (frame.CheckTextureAccess(m_pSharedTexture)) {
			m_bNewFrame = false; // For query of new frame
			// Check if the sender has produced a new frame.
			if (frame.GetNewFrame()) {
				// Copy from the sender's shared texture to the receiving texture.
				m_pImmediateContext->CopyResource(pTexture, m_pSharedTexture);
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
		// Let the application know.
		m_bConnected = false;
	}

	// ReceiveTexture fails if there is no sender or the connected sender closed.
	return m_bConnected;

}

// Receive from a sender via DX11 staging textures to an rgba or rgb buffer of variable size
// A new shared texture pointer (m_pSharedTexture) is retrieved if the sender changed
bool spoutDX::ReceiveImage(unsigned char * pixels, unsigned int width, unsigned int height, bool bRGB, bool bInvert)
{
	// Make sure DirectX is initialized
	if (!OpenDirectX11())
		return false;

	// Try to receive texture details from a sender
	if (ReceiveSenderData()) {
		// The sender name, width, height, format, shared texture handle and pointer have been retrieved.
		if (m_bUpdated) {
			// A new sender has been found or the one connected has changed.
			// The staging textures must be the same size and format as the sender
			// Create new staging textures if it is a different size
			CheckStagingTextures(m_Width, m_Height, m_dwFormat);
			// The application detects the change with IsUpdated()
			// and the receiving buiffer is updated to match the sender.
			return true;
		}

		// The receiving pixel buffer is created after the first update
		// So check here instead of at the beginning
		if (!pixels)
			return false;

		// No staging textures - no copy
		if (!m_pStaging[0] || !m_pStaging[1])
			return false;

		//
		// Found a sender
		//
		// Access the sender shared texture
		if (frame.CheckTextureAccess(m_pSharedTexture)) {
			// Check if the sender has produced a new frame.
			if (frame.GetNewFrame()) {
				// Read from the sender GPU texture to CPU pixels via two staging textures
				// One texture - approx 7 - 12 msec at 1920x1080
				// Two textures - approx 2.5 - 3.5 msec at 1920x1080
				m_Index = (m_Index + 1) % 2;
				m_NextIndex = (m_Index + 1) % 2;
				// Copy from the sender's shared texture to the first staging texture
				m_pImmediateContext->CopyResource(m_pStaging[m_Index], m_pSharedTexture);
				// Map and read from the second while the first is occupied
				ReadPixelData(m_pStaging[m_NextIndex], pixels, width, height, bRGB, bInvert);
			}
			// Allow access to the shared texture
			frame.AllowTextureAccess(m_pSharedTexture);
		}

		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// Let the application know.
		m_bConnected = false;
	}

	// ReceiveImage fails if there is no sender or the connected sender closed.
	return m_bConnected;

}


// Pop up SpoutPanel to allow the user to select a sender
void spoutDX::SelectSender()
{
	SelectSenderPanel();
}

// Check for sender change
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


//---------------------------------------------------------
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

//---------------------------------------------------------
// SenderNames
//
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

bool spoutDX::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spoutsender.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

bool spoutDX::GetActiveSender(char* Sendername)
{
	return spoutsender.GetActiveSender(Sendername);
}

bool spoutDX::SetActiveSender(const char* Sendername)
{
	return spoutsender.SetActiveSender(Sendername);
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
// Adapter functions
//
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


//
// Sharing modes not supported
//

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

//
// Utilities
//

bool spoutDX::CreateDX11texture(ID3D11Device* pd3dDevice,
	unsigned int width, unsigned int height,
	DXGI_FORMAT format, ID3D11Texture2D** ppTexture)
{
	HANDLE hShare = NULL;
	return spoutdx.CreateSharedDX11Texture(pd3dDevice, width, height, format, ppTexture, hShare);
}


//
// PRIVATE
//

//---------------------------------------------------------
//	o Connect to a sender and inform the application to update texture dimensions
//	o Check for user sender selection
//  o Receive texture details from the sender for write to the user texture
//  o Retrieve width, height, format, share handle and texture pointer
bool spoutDX::ReceiveSenderData()
{
	m_bUpdated = false;

	// Initialization is recorded in this class for sender or receiver
	// m_Width or m_Height are established when the receiver connects to a sender
	char sendername[256];
	strcpy_s(sendername, 256, m_SenderName);

	// If SpoutPanel has been opened, the active sender name could be different
	if (CheckSpoutPanel(sendername, 256)) {
		// Disable the setup name
		m_SenderNameSetup[0] = 0;
	}

	// Save current sender name and dimensions to test for change
	unsigned int width = m_Width;
	unsigned int height = m_Height;
	DWORD dwFormat = m_dwFormat;
	HANDLE dxShareHandle = m_dxShareHandle;

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(sendername, width, height, dxShareHandle, dwFormat)) {

		// Memoryshare not supported (no texture share handle)
		if (!dxShareHandle) {
			ReleaseReceiver();
			return false;
		}

		// The shared texture handle will be different
		//   o for a new sender
		//   o for texture size or format change
		if (dxShareHandle != m_dxShareHandle) {

			// Release everything and start again below
			ReleaseReceiver();

			// Get a new shared texture pointer
			if (!spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, dxShareHandle)) {
				//
				// Failed to get the sender's texture pointer
				// (Error logs and catch of exception are already done).
				//
				// The sender was there but the texture pointer could not be retrieved from the share handle.
				// It might be using a different adapter or some other reason.
				// Retain the share handle so we don't query it again.
				// Wait until another sender is selected or the shared texture handle is valid.
				m_dxShareHandle = dxShareHandle;
				// Return true (m_pSharedTexture is null but will not be used)
				return true;
			}

			// Use the new sender share handle and texture pointer retrieved above
			m_dxShareHandle = dxShareHandle;

			// Initialize again with the new sender values
			CreateReceiver(sendername, width, height, dwFormat);

			m_bUpdated = true; // Return to update the receiving texture or image

		}

		// Connected and intialized
		// Sender name, width, height, format, texture pointer and share handle have been retrieved

		// The application can now access and copy the sender texture
		return true;

	} // end find sender

	// There is no sender or the connected sender closed
	return false;

}

// Create receiver resources for a new sender
void spoutDX::CreateReceiver(const char * SenderName, unsigned int width, unsigned int height, DWORD dwFormat)
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
// COPY FROM A DX11 STAGING TEXTURE TO A USER RGBA/RGB/BGR PIXEL BUFFER OF GIVEN SIZE
//
// A class device and context must have been created using OpenDirectX11()
//
bool spoutDX::ReadPixelData(ID3D11Texture2D* pStagingTexture, unsigned char* pixels,
	unsigned int width, unsigned int height, bool bRGB, bool bInvert)
{
	if (!m_pImmediateContext || !pStagingTexture || !pixels)
		return false;

	// Map the resource so we can access the pixels
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	// Make sure all commands are done before mapping the staging texture
	m_pImmediateContext->Flush();
	// Map waits for GPU access
	HRESULT hr = m_pImmediateContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {

		// Copy the staging texture pixels to the user buffer
		if (!bRGB) {
			// RGBA buffer
			spoutcopy.rgba2rgba(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (m_dwFormat == 28) {
			// RGB buffer
			// If the texture format is RGBA it has to be converted to BGR by the staging texture copy
			if (width != m_Width || height != m_Height) {
				spoutcopy.rgba2bgrResample(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, width, height, bInvert);
			}
			else {
				spoutcopy.rgba2bgr(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			}
		}
		else {
			if (width != m_Width || height != m_Height) {
				spoutcopy.rgba2rgbResample(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, width, height, bInvert);
			}
			else {
				// Approx 5 msec at 1920x1080
				spoutcopy.rgba2rgb(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			}
		}

		// 0.03 msec
		m_pImmediateContext->Unmap(pStagingTexture, 0);

		return true;
	} // endif DX11 map OK

	return false;

} // end ReadPixelData

// Create new class staging textures if changed size or do not exist yet
bool spoutDX::CheckStagingTextures(unsigned int width, unsigned int height, DWORD dwFormat)
{
	if (!m_pd3dDevice)
		return false;

	D3D11_TEXTURE2D_DESC desc = { 0 };

	if (m_pStaging[0] && m_pStaging[1]) {
		// Get the size to test for change
		m_pStaging[0]->GetDesc(&desc);
		if (desc.Width != width || desc.Height != height || desc.Format != (DXGI_FORMAT)dwFormat) {

			// Staging textures must not be mapped before release
			m_pStaging[0]->Release();
			m_pStaging[1]->Release();
			m_pStaging[0] = nullptr;
			m_pStaging[1] = nullptr;
			m_Index = 0;
			m_NextIndex = 0;
			// Drop through to create new textures
		}
		else {
			return true;
		}
	}

	if (CreateDX11StagingTexture(width, height, (DXGI_FORMAT)dwFormat, &m_pStaging[0])
	 && CreateDX11StagingTexture(width, height, (DXGI_FORMAT)dwFormat, &m_pStaging[1])) {
		return true;
	}

	return false;
}


// Create a DirectX 11 staging texture for read and write
bool spoutDX::CreateDX11StagingTexture(unsigned int width, unsigned int height,	DXGI_FORMAT format,	ID3D11Texture2D** pStagingTexture)
{
	if (!m_pd3dDevice)
		return false;

	ID3D11Texture2D* pTexture = nullptr;

	pTexture = *pStagingTexture; // The texture pointer
	if (pTexture) {
		pTexture->Release();
		pTexture = nullptr;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;

	HRESULT res = m_pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (res != S_OK) {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476174%28v=vs.85%29.aspx
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDirectX::CreateDX11StagingTexture ERROR : [0x%lx] : ", res);
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
		// SpoutLogFatal("%s", tmp);
		printf("%s\n", tmp);
		return false;
	}

	*pStagingTexture = pTexture;

	return true;

}

//
// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.
//

// Pop up SpoutPanel to allow the user to select a sender
// Usually activated by RH click
void spoutDX::SelectSenderPanel()
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

} // end SelectSenderPanel

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

