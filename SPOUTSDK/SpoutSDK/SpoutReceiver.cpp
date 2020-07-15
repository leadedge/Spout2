//
//		SpoutReceiver
//
//		Wrapper class so that a receiver object can be created independent of a sender
//
// ====================================================================================
//		Revisions :
//
//		27-07-14	- CreateReceiver - bUseActive flag instead of null name
//		03.09.14	- Cleanup
//		23.09.14	- return DirectX 11 capability in SetDX9
//		28.09.14	- Added Host FBO for ReceiveTexture
//		12.10.14	- changed SelectSenderPanel arg to const char
//		23.12.14	- added host fbo arg to ReceiveImage
//		08.02.15	- Changed default texture format for ReceiveImage in header to GL_RGBA
//		29.05.15	- Included SetAdapter for multiple adapters - Franz Hildgen.
//		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
//		24.08.15	- Added GetHostPath to retrieve the path of the host that produced the sender
//		15.09.15	- Removed SetMemoryShareMode for 2.005 - now done globally by SpoutDirectX.exe
//		10.10.15	- Added transition flag to set invert true for 2.004 rather than default false for 2.005
//					- currently not used - see SpoutSDK.cpp CreateSender
//		14.11.15	- changed functions to "const char *" where required
//		18.11.15	- added CheckReceiver so that DrawSharedTexture can be used by a receiver
//		18.06.16	- Add invert to ReceiveImage
//		17.09.16	- removed CheckSpout2004() from constructor
//		13.01.17	- Add SetCPUmode, GetCPUmode, SetBufferMode, GetBufferMode
//					- Add HostFBO arg to DrawSharedTexture
//		15.01.17	- Add GetShareMode, SetShareMode
//		06.06.17	- Add OpenSpout
//		05.11.18	- Add IsSpoutInitialized
//		11.11.18	- Add 2.007 high level application functions
//		13.11.18	- Remove SetCPUmode, GetCPUmode
//		24.11.18	- Remove redundant GetImageSize
//		28.11.18	- Add IsFrameNew
//		11.12.18	- Add utility functions
//		05.01.19	- Make names for 2.007 functions compatible with SpoutLibrary
//		16.01.19	- Initialize class variables
//		16.03.19	- Add IsFrameCountEnabled
//		19.03.19	- Change IsInitialized to IsConnected
//		05.04.19	- Change GetSenderName(index, ..) to GetSender
//					  Create const char * GetSenderName for receiver class
//		18.09.19	- Remove UseDX9 from GetDX9 to avoid registry change
//					- Remove reset of m_SenderNameSetup from SetupReceiver
//					- Add connected test to IsUpdated
//					- Remove redundant CloseReceiver
//		28.11.19	- Remove SetupReceiver
//					  Add invert option to ReceiveTextureData and ReceiveImageData
//		13.01.20	- Add null texture option for ReceiveTextureData
//					  Add ReceiveTextureData option with no args and GetSenderTextureID()
//					  Updated receiver example
//		18.01.20	- Add CopyTexture. Update receiver example
//		20.01.20	- Changed GetSenderTextureID() to GetSharedTextureID
//		25.01.20	- Remove GetDX9compatible and SetDX9compatible
//		25.01.20	- Change ReceiveTextureData and ReceiveImageData to overloads
//		26.04.20	- Reset the update flag in IsUpdated
//		30.04.20	- Add ReceiveTexture()
//		17.06.20	- Add GetSenderFormat()
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
#include "SpoutReceiver.h"

SpoutReceiver::SpoutReceiver()
{
	// CreateReceiver will use the active sender unless the user 
	// has specified a sender to connect to using SetReceiverName
	m_SenderNameSetup[0] = 0;
	m_SenderName[0] = 0;
	m_bUseActive = true;
	m_TextureID = 0;
	m_TextureTarget = 0;
	m_Width = 0;
	m_Height = 0;
	m_bUpdate = false;
	m_bConnected = false;
}

//---------------------------------------------------------
SpoutReceiver::~SpoutReceiver()
{
	ReleaseReceiver();
}

//
// ================= 2.007 functions ======================
//


//---------------------------------------------------------
// Specify the sender for a receiver to connect to
void SpoutReceiver::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
		m_bUseActive = false; // the user has specified a sender to connect to (default true)
	}
}

//---------------------------------------------------------
// Release receiver and resources
// ready to connect to another sender
void SpoutReceiver::ReleaseReceiver()
{
	// Reset class variables for 2.007 functions
	m_Width = 0;
	m_Height = 0;
	m_bUpdate = false;
	m_bConnected = false;
	m_TextureID = 0;
	m_TextureTarget = 0;

	// Restore the starting sender name if the user specified one in SetReceiverName
	if (m_SenderNameSetup[0]) {
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);
		m_bUseActive = false;
	}
	else {
		m_SenderName[0] = 0;
		m_bUseActive = true;
	}

	// Release resources
	spout.ReleaseReceiver();
}

//---------------------------------------------------------
//	Receive sender shared texture
//
//  The texture can then be accessed using :
//		BindSharedTexture();
//		UnBindSharedTexture();
//		GetSharedTextureID();
//
bool SpoutReceiver::ReceiveTexture()
{
	return ReceiveTexture(0, 0);
}

//---------------------------------------------------------
// Receive OpenGL texture
//
//	o Connect to a sender
//	o Set class variables for sender name, width and height
//  o If the sender has changed size, set a flag for the application to update receiving texture
//    The texture must be RGBA of dimension (width * height)
//    and can be re-allocated is IsUpdated() returns true
//  o Copy the sender shared texture to the user texture
//
bool SpoutReceiver::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
{
	// Return if flagged for update
	// The update flag is reset when the receiving application calls IsUpdated()
	if (m_bUpdate)
		return true;

	// Initialization is recorded in the spout class for sender or receiver
	// m_Width or m_Height are established when the receiver connects to a sender
	// m_SenderName is established if no connecting name has been specified
	if (!IsConnected()) {
		if (CreateReceiver(m_SenderName, m_Width, m_Height, m_bUseActive)) {
			// Signal the application to update the receiving texture size
			// Retrieved with a call to the IsUpdated function
			m_bUpdate = true;
			m_bConnected = true;
			return true;
		}
	}
	else {
		// Save sender name and dimensions to test for change
		char name[256];
		strcpy_s(name, 256, m_SenderName);
		unsigned int width = m_Width;
		unsigned int height = m_Height;
		// Receive a shared texture but don't read it into the user texture yet
		if (ReceiveTexture(name, width, height)) {
			// Test for sender name or size change
			if (width != m_Width || height != m_Height || strcmp(name, m_SenderName) != 0) {
				// Update name
				strcpy_s(m_SenderName, 256, name);
				// Update class dimensions
				m_Width = width;
				m_Height = height;
				// Signal the application to update the receiving texture
				m_bUpdate = true;
			}
			else {
				// For a valid texture ID and target, read the shared texture
				// to the user texture and invert as necessary
				if (TextureID > 0 && TextureTarget > 0)
					spout.interop.ReadTexture(m_SenderName, TextureID, TextureTarget, m_Width, m_Height, bInvert, HostFbo);
			}
			return true;
		}
		else {
			// receiving failed
			ReleaseReceiver();
			return false;
		}
	}

	// No connection
	return false;

}

//---------------------------------------------------------
// Receive image pixels
//
//	o Connect to a sender
//	o Set class variables for sender name, width and height
//  o If the sender has changed size, inform the application to update the receiving buffer
//    The buffer must be RGBA of dimension (width * height * 4)
//    and can be re-allocated is IsUpdated() returns true
//  o Receive pixel data from a sender and write to the user buffer
//  o Set class variables for sender name, width and height
bool SpoutReceiver::ReceiveImage(unsigned char *pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)
{
	// Return if flagged for update
	if (m_bUpdate)
		return true;

	if (!IsConnected()) {
		// m_SenderName, m_Width and m_Height are established when the reciever connects to a sender
		if (CreateReceiver(m_SenderName, m_Width, m_Height, m_bUseActive)) {
			m_bUpdate = true;
			m_bConnected = true;
			return true;
		}
	}
	else {
		// Save current sender details
		char sendername[256];
		strcpy_s(sendername, 256, m_SenderName);
		unsigned int width = m_Width;
		unsigned int height = m_Height;
		// Receive a shared image but don't read it into the user pixels yet
		// All tests for sender existence and user selection are made
		if (spout.ReceiveImage(sendername, width, height, NULL, glFormat, bInvert)) {
			// Test for sender name or size change
			if (width != m_Width
				|| height != m_Height
				|| strcmp(m_SenderName, sendername) != 0) {
				// Update the connected sender name
				strcpy_s(m_SenderName, 256, sendername);
				// Update class dimensions
				m_Width = width;
				m_Height = height;
				// Signal the application to update the receiving pixels
				m_bUpdate = true;
				return true;
			}
			else {
				// Read the shared texture or memory directly into the pixel buffer and invert as necessary
				// Copy functions handle the formats supported
				return spout.interop.ReadTexturePixels(m_SenderName,
					pixels, m_Width, m_Height, glFormat, bInvert, HostFbo);
			}
		}
		else {
			// receiving failed
			ReleaseReceiver();
			return false;
		}
	}
	// No connection
	return false;
}

//---------------------------------------------------------
// Check for sender change
//  If true, the application must update the receiving texture
//  before the next call to ReceiveTexture or ReceiveImage
bool SpoutReceiver::IsUpdated()
{
	bool bRet = m_bUpdate;
	// Reset the update flag
	m_bUpdate = false;
	return bRet;
}

//---------------------------------------------------------
bool SpoutReceiver::IsConnected()
{
	return m_bConnected;
}

//---------------------------------------------------------
// Equivalent to SelectSenderPanel with no message option
void SpoutReceiver::SelectSender()
{
	SelectSenderPanel();
}

//---------------------------------------------------------
const char * SpoutReceiver::GetSenderName()
{
	return m_SenderName;
}

//---------------------------------------------------------
unsigned int SpoutReceiver::GetSenderWidth()
{
	return m_Width;
}

//---------------------------------------------------------
unsigned int SpoutReceiver::GetSenderHeight()
{
	return m_Height;
}

//---------------------------------------------------------
DWORD SpoutReceiver::GetSenderFormat()
{
	if (m_SenderName[0] == 0 || m_Width == 0 || m_Height == 0)
		return 0;

	HANDLE dxShareHandle = NULL;
	unsigned int width, height;
	DWORD dwFormat = 0;
	if (GetSenderInfo(m_SenderName, width, height, dxShareHandle, dwFormat)) {
		return dwFormat;
	}
	return 0;
}
//---------------------------------------------------------
double SpoutReceiver::GetSenderFps()
{
	return spout.interop.frame.GetSenderFps();
}

//---------------------------------------------------------
long SpoutReceiver::GetSenderFrame()
{
	return spout.interop.frame.GetSenderFrame();
}

//---------------------------------------------------------
bool SpoutReceiver::IsFrameNew()
{
	return spout.interop.frame.IsFrameNew();
}

//---------------------------------------------------------
void SpoutReceiver::DisableFrameCount()
{
	spout.interop.frame.DisableFrameCount();
}

//---------------------------------------------------------
bool SpoutReceiver::IsFrameCountEnabled()
{
	return spout.interop.frame.IsFrameCountEnabled();
}

//
// ================= end 2.007 functions ===================
//


//---------------------------------------------------------
bool SpoutReceiver::OpenSpout()
{
	return spout.OpenSpout();
}

//---------------------------------------------------------
bool SpoutReceiver::CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spout.CreateReceiver(name, width, height, bUseActive);
}

//---------------------------------------------------------
bool SpoutReceiver::ReceiveTexture(char* name, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	return spout.ReceiveTexture(name, width, height, TextureID, TextureTarget, bInvert, HostFBO);
}

#ifdef legacyOpenGL
//---------------------------------------------------------
bool SpoutReceiver::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	return spout.DrawSharedTexture(max_x, max_y, aspect, bInvert, HostFBO);
}
#endif

//---------------------------------------------------------
bool SpoutReceiver::ReceiveImage(char* Sendername, 
								 unsigned int &width, 
								 unsigned int &height, 
								 unsigned char* pixels, 
								 GLenum glFormat, 
								 bool bInvert,
								 GLuint HostFBO)
{
	return spout.ReceiveImage(Sendername, width, height, pixels, glFormat, bInvert, HostFBO);
}

//---------------------------------------------------------
void SpoutReceiver::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	return spout.RemovePadding(source, dest, width, height, stride, glFormat);
}

//---------------------------------------------------------
bool SpoutReceiver::CheckReceiver(char* name, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spout.CheckReceiver(name, width, height, bConnected);
}

//---------------------------------------------------------
bool SpoutReceiver::SelectSenderPanel(const char* message)
{
	return spout.SelectSenderPanel(message);
}

//---------------------------------------------------------
bool SpoutReceiver::BindSharedTexture()
{
	return spout.BindSharedTexture();
}

//---------------------------------------------------------
bool SpoutReceiver::UnBindSharedTexture()
{
	return spout.UnBindSharedTexture();
}

//---------------------------------------------------------
GLuint SpoutReceiver::GetSharedTextureID()
{
	return spout.interop.GetSharedTextureID();
}

//---------------------------------------------------------
int  SpoutReceiver::GetSenderCount()
{
	return spout.GetSenderCount();
}

//---------------------------------------------------------
bool SpoutReceiver::GetSender(int index, char* sendername, int MaxNameSize)
{
	return spout.GetSender(index, sendername, MaxNameSize);
}

//---------------------------------------------------------
bool SpoutReceiver::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spout.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

//---------------------------------------------------------
bool SpoutReceiver::GetActiveSender(char* Sendername)
{
	return spout.GetActiveSender(Sendername);
}

//---------------------------------------------------------
bool SpoutReceiver::SetActiveSender(const char* Sendername)
{
	return spout.SetActiveSender(Sendername);
}

//---------------------------------------------------------
bool SpoutReceiver::GetMemoryShareMode()
{
	return spout.GetMemoryShareMode();
}

//---------------------------------------------------------
bool SpoutReceiver::SetMemoryShareMode(bool bMem)
{
	return spout.SetMemoryShareMode(bMem);
}

//---------------------------------------------------------
int SpoutReceiver::GetShareMode()
{
	return spout.GetShareMode();
}

//---------------------------------------------------------
bool SpoutReceiver::SetShareMode(int mode)
{
	return (spout.SetShareMode(mode));
}

//---------------------------------------------------------
bool SpoutReceiver::GetBufferMode()
{
	return spout.GetBufferMode();
}

//---------------------------------------------------------
void SpoutReceiver::SetBufferMode(bool bActive)
{
	spout.SetBufferMode(bActive);
}

//---------------------------------------------------------
bool SpoutReceiver::GetDX9()
{
	return spout.interop.GetDX9();
}

//---------------------------------------------------------
bool SpoutReceiver::SetDX9(bool bDX9)
{
	return spout.interop.UseDX9(bDX9);
}

//---------------------------------------------------------
void SpoutReceiver::SetDX9format(D3DFORMAT textureformat)
{
	spout.SetDX9format(textureformat);
}

//---------------------------------------------------------
void SpoutReceiver::SetDX11format(DXGI_FORMAT textureformat)
{
	spout.SetDX11format(textureformat);
}
//---------------------------------------------------------
int SpoutReceiver::GetAdapter()
{
	return spout.GetAdapter();
}

//---------------------------------------------------------
bool SpoutReceiver::SetAdapter(int index)
{
	return spout.SetAdapter(index);
}

//---------------------------------------------------------
int SpoutReceiver::GetNumAdapters()
{
	return spout.GetNumAdapters();
}

//---------------------------------------------------------
// Get an adapter name
bool SpoutReceiver::GetAdapterName(int index, char* adaptername, int maxchars)
{
	return spout.GetAdapterName(index, adaptername, maxchars);
}

//---------------------------------------------------------
int SpoutReceiver::GetMaxSenders()
{
	// Get the maximum senders allowed from the sendernames class
	return(spout.interop.senders.GetMaxSenders());
}

//---------------------------------------------------------
void SpoutReceiver::SetMaxSenders(int maxSenders)
{
	// Sets the maximum senders allowed
	spout.interop.senders.SetMaxSenders(maxSenders);
}

//---------------------------------------------------------
bool SpoutReceiver::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	return spout.GetHostPath(sendername, hostpath, maxchars);
}

//---------------------------------------------------------
int SpoutReceiver::GetVerticalSync()
{
	return spout.interop.GetVerticalSync();
}

//---------------------------------------------------------
bool SpoutReceiver::SetVerticalSync(bool bSync)
{
	return spout.interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
bool SpoutReceiver::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	return spout.interop.CopyTexture(SourceID, SourceTarget, DestID, DestTarget,
		width, height, bInvert, HostFBO);
}





