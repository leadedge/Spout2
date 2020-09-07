/*

					SpoutSender.h

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

#ifndef __SpoutSender__
#define __SpoutSender__

#include "SpoutSDK.h"

class SPOUT_DLLEXP SpoutSender {

	public:

	SpoutSender();
    ~SpoutSender();

	//
	// 2.007 sender
	//

	// Set name for sender creation
	//   If no name is specified, the executable name is used.  
	void SetSenderName(const char* sendername = nullptr);
	// Close sender and free resources
	void ReleaseSender();
	// Send OpenGL texture.
	//   A sender is created or updated based on the size and name
	//   that has been set (see SetSenderName and CreateSender)
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	// Send texture attached to fbo.
	//   The fbo must be currently bound.  
	//   The sending texture can be larger than the size that the sender is set up for.  
	//   For example, if the application is using only a portion of the allocated texture space,  
	//   such as for Freeframe plugins. The 2.006 equivalent is DrawToSharedTexture.
	//   A sender is created or updated based on the size and name
	//   that has been set (see SetSenderName and CreateSender)
	bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
	// Send image pixels.
	//   A sender is created or updated based on the size and name
	//   that has been set (see SetSenderName and CreateSender)
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	// Sender status
	bool IsInitialized();
	// Sender name
	const char * GetName();
	// Sender width
	unsigned int GetWidth();
	// Sender height
	unsigned int GetHeight();
	// Sender frame rate
	double GetFps();
	// Sender frame number
	long GetFrame();
	// Sender frame rate control.
	//   Can be used to control the frame rate if no other means is available.
	void HoldFps(int fps);

	//
	// 2.007 common
	//

	// Frame count status
	bool IsFrameCountEnabled();
	// Disable frame counting for this application
	void DisableFrameCount();
	// Get sender shared texture ID.
	GLuint GetSharedTextureID();
	// OpenGL texture copy (textures must be the same size)
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0);
	// Correct for image stride
	void RemovePadding(const unsigned char *source, unsigned char *dest, unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat = GL_RGBA);

	//
	// 2.006 sender compatibility
	//

	// Create a sender
	//   For valid width and height, a sender is created at this time.
	//   For no width and height, a sender of that name is created on the first sending call.
	//   If no name is specified, the executable name is used.
	bool CreateSender(const char *sendername, unsigned int width = 0, unsigned int height = 0, DWORD dwFormat = 0);
	// Update sender dimensions
	bool UpdateSender(const char* sendername, unsigned int width, unsigned int height);
#ifdef legacyOpenGL
	// Render a texture to the shared texture. 
	// See _SpoutCommon.h_ #define legacyOpenGL
	bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);
#endif

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
	// Get memoryshare mode status
	bool GetMemoryShareMode();
	// Set memoryshare mode on or off
	bool SetMemoryShareMode(bool bMem = true);
	// Get sharing mode : 0-texture, 1-CPU (disabled for 2.007) 2-memory
	int  GetShareMode();
	// Set share mode : 0-texture, 1&2-memory
	bool SetShareMode(int mode);
	// Get OpenGL pixel buffering setting
	bool GetBufferMode();
	// Enable/disable OpenGL pixel buffering (default off).
	void SetBufferMode(bool bActive);
	// Get the number of graphics adapters in the system
	int  GetNumAdapters();
	// Get an adapter name
	bool GetAdapterName(int index, char *adaptername, int maxchars);
	// Get the current adapter index
	int  GetAdapter();
	// Set required graphics adapter for output
	bool SetAdapter(int index = 0);
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

	// For access to all functions
	Spout spout;

protected :

	// Check sender name, creation and update
	bool CheckSender(unsigned int width, unsigned int height);
	char m_SenderName[256]; // Sender name
	GLuint m_TextureID; // Sender texture ID
	GLuint m_TextureTarget; // Sender texture Target
	DWORD m_dwFormat; // Sender shared texture D3D11 format
	unsigned int m_Width; // Sender width
	unsigned int m_Height; // Sender height

};

#endif
