//
//	SpoutFunctions.cpp
//
//	A Spout functions class specifically for SpoutLibrary.
//  Includes both sender and receiver functions
//	but a separate instance of SpoutLibrary is required
//	for individual senders or receivers.
//
/*
		Copyright (c) 2016-2020, Lynn Jarvis. All rights reserved.

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

#include "SpoutFunctions.h"

/*
// Log level definitions
enum SpoutLogLevel {
	SPOUT_LOG_SILENT,
	SPOUT_LOG_VERBOSE,
	SPOUT_LOG_NOTICE,
	SPOUT_LOG_WARNING,
	SPOUT_LOG_ERROR,
	SPOUT_LOG_FATAL
};
*/

SpoutFunctions::SpoutFunctions() {

	bIsSending = false;

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

};

SpoutFunctions::~SpoutFunctions() {
	if (bIsSending)
		ReleaseSender();
	else
		ReleaseReceiver();
};

// ---------------------------------------------------------------
// 2.007

//
// Sender
//

//---------------------------------------------------------
const char * SpoutFunctions::GetName()
{
	char name[256];
	if (spout.GetSpoutSenderName(name, 256))
		strcpy_s(m_SenderName, 256, name);
	return m_SenderName;
}

//---------------------------------------------------------
unsigned int SpoutFunctions::GetWidth()
{
	return m_Width;
}

//---------------------------------------------------------
unsigned int SpoutFunctions::GetHeight()
{
	return m_Height;
}

//---------------------------------------------------------
long SpoutFunctions::GetFrame()
{
	return (spout.interop.frame.GetSenderFrame());
}

//---------------------------------------------------------
double SpoutFunctions::GetFps()
{
	return (spout.interop.frame.GetSenderFps());
}

//---------------------------------------------------------
void SpoutFunctions::HoldFps(int fps)
{
	return (spout.interop.frame.HoldFps(fps));
}


//
// Receiver
//

//---------------------------------------------------------
void SpoutFunctions::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
		m_bUseActive = false; // the user has specified a sender to connect to
	}
}

//---------------------------------------------------------
//	o Connect to a sender and inform the application to update texture dimensions
//  o Receive texture data from the sender and write to the user texture
//
//  For no texture ID or target, the sender texture can be accessed using :
//		BindSharedTexture();
//		UnBindSharedTexture();
//		GetSharedTextureID()
//
bool SpoutFunctions::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
{
	// Return if flagged for update
	// The update flag is reset when the receiving application calls IsUpdated()
	if (m_bUpdate)
		return true;

	// Initialization is recorded in the spout class for sender or receiver
	// m_Width or m_Height are established when the receiver connects to a sender
	if (!IsConnected()) {
		if (CreateReceiver(m_SenderName, m_Width, m_Height, m_bUseActive)) {
			// Signal the application to update the receiving texture size
			// Retrieved with a call to the Updated function
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
		if (spout.ReceiveTexture(name, width, height)) {
			// Test for sender name or size change
			if (width != m_Width
				|| height != m_Height
				|| strcmp(name, m_SenderName) != 0) {
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
				if(TextureID > 0 && TextureTarget > 0)
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
//	o Connect to a sender and inform the application to update buffer dimensions
//  o Receive pixel data from the sender and write to the user buffer
bool SpoutFunctions::ReceiveImage(unsigned char *pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)
{
	// Return if flagged for update
	if (m_bUpdate)
		return true;

	if (!IsConnected()) {
		if (CreateReceiver(m_SenderName, m_Width, m_Height, m_bUseActive)) {
			m_bUpdate = true;
			m_bConnected = true;
			return true;
		}
	}
	else {
		char sendername[256];
		strcpy_s(sendername, 256, m_SenderName);
		unsigned int width = m_Width;
		unsigned int height = m_Height;
		// Receive a shared image but don't read it into the user pixels yet
		if (spout.ReceiveImage(sendername, width, height, NULL)) {
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
				return(spout.interop.ReadTexturePixels(m_SenderName, 
					pixels, width, height, glFormat, bInvert, HostFbo));
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
//  If updated, the application must update the receiving texture
//  before the next call to ReceiveTextureData or ReceiveImageData.
bool SpoutFunctions::IsUpdated()
{
	bool bRet = m_bUpdate;
	// Reset the update flag
	m_bUpdate = false;
	return bRet;
}

//---------------------------------------------------------
bool SpoutFunctions::IsConnected()
{
	return m_bConnected;
}

//---------------------------------------------------------
void SpoutFunctions::SelectSender()
{
	spout.SelectSenderPanel();
}

//---------------------------------------------------------
const char * SpoutFunctions::GetSenderName()
{
	return m_SenderName;
}

//---------------------------------------------------------
unsigned int SpoutFunctions::GetSenderWidth()
{
	return m_Width;
}

//---------------------------------------------------------
unsigned int SpoutFunctions::GetSenderHeight()
{
	return m_Height;
}

DWORD SpoutFunctions::GetSenderFormat()
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
long SpoutFunctions::GetSenderFrame()
{
	return (spout.interop.frame.GetSenderFrame());
}

//---------------------------------------------------------
double SpoutFunctions::GetSenderFps()
{
	return (spout.interop.frame.GetSenderFps());
}

//---------------------------------------------------------
bool SpoutFunctions::IsFrameNew()
{
	return (spout.interop.frame.IsFrameNew());
}


// Common

//---------------------------------------------------------
void SpoutFunctions::DisableFrameCount()
{
	spout.interop.frame.DisableFrameCount();
}

//---------------------------------------------------------
bool SpoutFunctions::IsFrameCountEnabled()
{
	return spout.interop.frame.IsFrameCountEnabled();
}

//
// 2.006  compatibility
//

//---------------------------------------------------------
bool SpoutFunctions::CreateSender(const char* name, unsigned int width, unsigned int height, DWORD dwFormat)
{
	// bIsSending is false in constructor
	if (spout.CreateSender(name, width, height, dwFormat)) {
		strcpy_s(m_SenderName, 256, name);
		m_Width = width;
		m_Height = height;
		m_dwFormat = dwFormat;
		bIsSending = true;
		return true;
	}
	else {
		m_SenderName[0] = 0;
		m_TextureID = 0;
		m_TextureTarget = 0;
		m_Width = 0;
		m_Height = 0;
		m_dwFormat = 0;
		bIsSending = false;
	}
	return false;
}

//---------------------------------------------------------
bool SpoutFunctions::UpdateSender(const char* name, unsigned int width, unsigned int height)
{
	bool bRet = false;
	// For a name change, close the sender and set up again
	if (strcmp(name, m_SenderName) != 0) {
		ReleaseSender();
		bRet = CreateSender(name, width, height, m_dwFormat);
	}
	else if (width != m_Width || height != m_Height) {
		// For sender update, only width and height are necessary
		m_Width = width;
		m_Height = height;
		bRet = spout.UpdateSender(m_SenderName, m_Width, m_Height);
	}
	return bRet;
}

//---------------------------------------------------------
void SpoutFunctions::ReleaseSender(DWORD dwMsec)
{
	if (IsInitialized())
		spout.ReleaseSender(dwMsec);
	m_SenderName[0] = 0;
	m_Width = 0;
	m_Height = 0;
}

//---------------------------------------------------------
bool SpoutFunctions::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spout.SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

//---------------------------------------------------------
bool SpoutFunctions::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	return spout.SendFbo(FboID, width, height, bInvert);
}

//---------------------------------------------------------
bool SpoutFunctions::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	return spout.SendImage(pixels, width, height, glFormat, bInvert, HostFBO);
}

//---------------------------------------------------------
void SpoutFunctions::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	spout.RemovePadding(source, dest, width, height, stride, glFormat);
}

//---------------------------------------------------------
bool SpoutFunctions::CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spout.CreateReceiver(name, width, height, bUseActive);
}

//---------------------------------------------------------
void SpoutFunctions::ReleaseReceiver()
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

/*
//---------------------------------------------------------
bool SpoutFunctions::ReceiveTexture(char* name, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	return spout.ReceiveTexture(name, width, height, TextureID, TextureTarget, bInvert, HostFBO);
}

//---------------------------------------------------------
bool SpoutFunctions::ReceiveImage(char* Sendername,
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
bool SpoutFunctions::SelectSenderPanel(const char* message)
{
	return spout.SelectSenderPanel(message);
}
*/

//---------------------------------------------------------
bool SpoutFunctions::CheckReceiver(char* name, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spout.CheckReceiver(name, width, height, bConnected);
}

//---------------------------------------------------------
bool SpoutFunctions::IsInitialized()
{
	return spout.IsSpoutInitialized();
}

//---------------------------------------------------------
bool SpoutFunctions::BindSharedTexture()
{
	return spout.BindSharedTexture();
}

//---------------------------------------------------------
bool SpoutFunctions::UnBindSharedTexture()
{
	return spout.UnBindSharedTexture();
}

//---------------------------------------------------------
GLuint SpoutFunctions::GetSharedTextureID()
{
	return spout.interop.GetSharedTextureID();
}

//---------------------------------------------------------
int  SpoutFunctions::GetSenderCount()
{
	return spout.GetSenderCount();
}

//---------------------------------------------------------
bool SpoutFunctions::GetSender(int index, char* sendername, int MaxNameSize)
{
	return spout.GetSender(index, sendername, MaxNameSize);
}

//---------------------------------------------------------
bool SpoutFunctions::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spout.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

//---------------------------------------------------------
bool SpoutFunctions::GetActiveSender(char* Sendername)
{
	return spout.GetActiveSender(Sendername);
}

//---------------------------------------------------------
bool SpoutFunctions::SetActiveSender(const char* Sendername)
{
	return spout.SetActiveSender(Sendername);
}

// Utilities

//---------------------------------------------------------
bool SpoutFunctions::GetDX9()
{
	return spout.interop.GetDX9();
}

//---------------------------------------------------------
bool SpoutFunctions::SetDX9(bool bDX9)
{
	return spout.SetDX9(bDX9);
}

//---------------------------------------------------------
bool SpoutFunctions::GetMemoryShareMode()
{
	return spout.GetMemoryShareMode();
}

//---------------------------------------------------------
bool SpoutFunctions::SetMemoryShareMode(bool bMem)
{
	return spout.SetMemoryShareMode(bMem);
}

//---------------------------------------------------------
int SpoutFunctions::GetShareMode()
{
	return (spout.GetShareMode());
}

//---------------------------------------------------------
bool SpoutFunctions::SetShareMode(int mode)
{
	return (spout.SetShareMode(mode));
}

//---------------------------------------------------------
bool SpoutFunctions::GetBufferMode()
{
	return spout.GetBufferMode();
}

//---------------------------------------------------------
void SpoutFunctions::SetBufferMode(bool bActive)
{
	spout.SetBufferMode(bActive);
}

//---------------------------------------------------------
int SpoutFunctions::GetMaxSenders()
{
	// Get the maximum senders allowed from the sendernames class
	return(spout.interop.senders.GetMaxSenders());
}

//---------------------------------------------------------
void SpoutFunctions::SetMaxSenders(int maxSenders)
{
	// Sets the maximum senders allowed
	spout.interop.senders.SetMaxSenders(maxSenders);
}

//---------------------------------------------------------
bool SpoutFunctions::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	return spout.GetHostPath(sendername, hostpath, maxchars);
}

//---------------------------------------------------------
bool SpoutFunctions::SetVerticalSync(bool bSync)
{
	return spout.interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
int SpoutFunctions::GetVerticalSync()
{
	return spout.interop.GetVerticalSync();
}

// Adapter functions

//---------------------------------------------------------
int SpoutFunctions::GetNumAdapters()
{
	return spout.GetNumAdapters();
}

//---------------------------------------------------------
bool SpoutFunctions::GetAdapterName(int index, char* adaptername, int maxchars)
{
	return spout.GetAdapterName(index, adaptername, maxchars);
}

//---------------------------------------------------------
bool SpoutFunctions::SetAdapter(int index)
{
	return spout.SetAdapter(index);
}

//---------------------------------------------------------
int SpoutFunctions::GetAdapter()
{
	return spout.GetAdapter();
}


// OpenGL

//---------------------------------------------------------
bool SpoutFunctions::CreateOpenGL()
{
	return spout.CreateOpenGL();
}

//---------------------------------------------------------
bool SpoutFunctions::CloseOpenGL()
{
	return spout.CloseOpenGL();
}

////////////////////////////////////////////////////////////////////////////////
