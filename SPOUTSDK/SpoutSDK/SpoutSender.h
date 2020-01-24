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

#include "spoutSDK.h"

class SPOUT_DLLEXP SpoutSender {

	public:

	SpoutSender();
    ~SpoutSender();

	//
	// 2.007
	//

	// Return sender width
	unsigned int GetWidth();
	// Return sender height
	unsigned int GetHeight();
	// Return sender frame number
	long GetFrame();
	// Return sender frame rate
	double GetFps();
	// Sender frame rate control
	void HoldFps(int fps);
	// Return frame count status
	bool IsFrameCountEnabled();
	// Disable frame counting for this application
	void DisableFrameCount();

	//
	// 2.006 and earlier
	//

	bool OpenSpout();
	bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
	bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height);
	void ReleaseSender(DWORD dwMsec = 0);
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
#ifdef legacyOpenGL
	bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);
#endif
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	void RemovePadding(const unsigned char *source, unsigned char *dest, unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat = GL_RGBA);
	
	bool IsInitialized();
	bool BindSharedTexture();
	bool UnBindSharedTexture();
	GLuint GetSharedTextureID();

	bool GetDX9();
	bool SetDX9(bool bDX9 = true); // set to use DirectX 9 (default is DirectX 11)
	bool GetMemoryShareMode();
	bool SetMemoryShareMode(bool bMem = true);
	int  GetShareMode();
	bool SetShareMode(int mode);
	bool GetBufferMode();
	void SetBufferMode(bool bActive); // Set the pbo availability on or off

	bool GetDX9compatible();
	void SetDX9compatible(bool bCompatible = true); // DirectX 11 format compatible with DirectX 9

	int  GetNumAdapters(); // Get the number of graphics adapters in the system
	bool GetAdapterName(int index, char* adaptername, int maxchars); // Get an adapter name
	int  GetAdapter(); // Get the current adapter index
	bool SetAdapter(int index = 0); // Set required graphics adapter for output

	int  GetMaxSenders(); // Get maximum senders allowed
	void SetMaxSenders(int maxSenders); // Set maximum senders allowed
	bool GetHostPath(const char* sendername, char* hostpath, int maxchars); // The path of the host that produced the sender
	int  GetVerticalSync();
	bool SetVerticalSync(bool bSync = true);

	// OpenGL utility
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0);

	bool SenderDebug(char* Sendername, int size);

	Spout spout; // For access to all functions

protected :

	char m_SenderName[256];
	GLuint m_TextureID;
	GLuint m_TextureTarget;
	DWORD m_dwFormat;
	unsigned int m_Width;
	unsigned int m_Height;

};

#endif
