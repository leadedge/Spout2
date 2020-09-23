/*

			SpoutReceiver.h

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
#pragma once

#ifndef __SpoutReceiver__
#define __SpoutReceiver__

#include "SpoutSDK.h"

class SPOUT_DLLEXP SpoutReceiver {

	public:

	SpoutReceiver();
    ~SpoutReceiver();

	//
	// 2.007 receiver
	//

	// Specify sender for connection.
	//   The application will not connect to any other  unless the user selects one.
	//   If that sender closes, the application will wait for the nominated sender to open. 
	//   If no name is specified, the receiver will connect to the active sender.
	void SetReceiverName(const char * SenderName);
	// Release receiver.
	//   Close connection and release resources  
	//   ready to connect to another sender.
	void ReleaseReceiver();
	// Receive shared texture.
	//   Connect to a sender and retrieve shared texture details ready for access
	//	 (see BindSharedTexture and UnBindSharedTexture).
	bool ReceiveTexture();
	// Receive OpenGL texture.
	// 	 Connect to a sender and inform the application to update the
	//   receiving texture if it has changed dimensions.
	//   For no change, copy the sender shared texture to the application texture
	bool ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert = false, GLuint HostFbo = 0);
	// Receive image pixels.
	//   Connect to a sender and inform the application to update the
	//   receiving buffer if it has changed dimensions.
	//   For no change, copy the sender shared texture to the pixel buffer.
	bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0);
	// Query whether the sender has changed.
	//   It is not necessary to monitor sender size changes.  
	//   However, sender update must be checked at every cycle before receiving data.  
	//   If this is not done, the receiving functions fail.
	bool IsUpdated();
	// Query sender connection.
	//   If the sender closes, receiving functions return false,  
	//   but connection can be tested at any time.
	bool IsConnected();
	// Query received frame status.
	//   The receiving texture or pixel buffer is only refreshed
	//   if the sender has produced a new frame.  
	//   This can be queried if it is necessary to process
	//   texture data only for new frames. 
	bool IsFrameNew();
	// Get sender name
	const char * GetSenderName();
	// Get sender width
	unsigned int GetSenderWidth();
	// Get sender height
	unsigned int GetSenderHeight();
	// Get sender texture format
	DWORD GetSenderFormat();
	// Get sender frame rate
	double GetSenderFps();
	// Get sender frame number
	long GetSenderFrame();
	// Open sender selection dialog (replaces SelectSenderPanel)
	void SelectSender();

	//
	// 2.007 common
	//

	// Frame count status
	bool IsFrameCountEnabled();
	// Disable frame counting for this application
	void DisableFrameCount();
	// Get sender shared texture ID
	GLuint GetSharedTextureID();
	// Copy texture with optional invert. Textures must be the same size.
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0);
	// Correct for image stride
	void RemovePadding(const unsigned char *source, unsigned char *dest, unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat = GL_RGBA);

	//
	// 2.006 receiver compatibility
	//

	// Create receiver connection
	bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false);
	// Receive OpenGL texture
	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO = 0);
	// Receive image pixels
	bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);
	// Check receiver connection
	bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);
#ifdef legacyOpenGL
	// Render shared texture.
	//   (see SpoutCommon.h : #define legacyOpenGL)
	bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);
#endif	
	// Get the number of senders
	int  GetSenderCount();
	// Get sender item name
	bool GetSender(int index, char* Sendername, int MaxSize = 256);
	// Get sender information
	bool GetSenderInfo(const char* Sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	// Get current active sender
	bool GetActiveSender(char* Sendername);
	// Set active sender
	bool SetActiveSender(const char* Sendername);
	// Open sender selection dialog
	bool SelectSenderPanel(const char* message = NULL);

	//
	// 2.006 compatibility common
	//

	// Initialize OpenGL and DirectX and set share mode
	bool OpenSpout();
	// Bind the OpenGL shared texture for access
	bool BindSharedTexture();
	// Un-bind the OpenGL shared texture
	bool UnBindSharedTexture();
	// Get DirectX 9 mode.
	//   Default is DirectX 11 (return false)
	bool GetDX9();
	// Set to use DirectX 9
	bool SetDX9(bool bDX9 = true);
	// Set the DirectX 9 format for texture sharing
	void SetDX9format(D3DFORMAT textureformat);
	// Set the DirectX 11 format for texture sharing
	void SetDX11format(DXGI_FORMAT textureformat);

	// Sharing modes
	bool GetMemoryShareMode(); // User selected memory share mode
	bool SetMemoryShareMode(bool bMem = true); // Set memoryshare mode on or off
	int  GetShareMode(); // User selected share mode : 0 -texture, 1 - memory, 2 - auto
	bool SetShareMode(int mode); // Set sharing mode : 0-texture, 1-memory, 2-auto
	bool GetMemoryShare(); // Get memory share compatibility mode
	void SetMemoryShare(bool bMem = true); // Set memory share compatibility mode
	bool GetSenderMemoryShare(const char* sendername); // Get share mode of a sender (memory or texture)

	// Get OpenGL pixel buffering setting
	bool GetBufferMode();
	// Enable/disable OpenGL pixel buffering (default off).
	void SetBufferMode(bool bActive);
	// Get the number of graphics adapters in the system
	int  GetNumAdapters();
	// Get an adapter name
	bool GetAdapterName(int index, char* adaptername, int maxchars);
	// Get the current adapter index
	int  GetAdapter();
	// Set required graphics adapter for output
	bool SetAdapter(int index = 0);
	// Get a sender adapter index
	int  GetSenderAdapter(const char* sendername);
	// Get maximum senders allowed
	int  GetMaxSenders();
	// Set maximum senders allowed
	void SetMaxSenders(int maxSenders);
	// The path of the host that produced the sender
	bool GetHostPath(const char *sendername, char *hostpath, int maxchars);
	// Get vertical sync status
	int  GetVerticalSync();
	// Set lock to monitor vertical sync
	bool SetVerticalSync(bool bSync = true);

	// for access to all functions
	Spout spout;

protected :

	// Sender connection name.
	//   The receiver connects to the active sender unless the user  
	//   has specified a sender using SetReceiverName
	char m_SenderNameSetup[256];
	char m_SenderName[256]; // Connected sender name
	GLuint m_TextureID; // Sender texture ID
	GLuint m_TextureTarget; // Sender texture Target
	bool m_bUpdate; // Sender changed flag
	bool m_bUseActive; // Use active sender
	bool m_bConnected; // Connection flag
	unsigned int m_Width; // Sender width
	unsigned int m_Height; // Sender height

};

#endif
