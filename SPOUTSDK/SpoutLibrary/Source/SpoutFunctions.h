//
//	SpoutFunctions.h
//
//	Spout SDK functions
//
/*
		Copyright (c) 2016-2019, Lynn Jarvis. All rights reserved.

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

#ifndef __SpoutFunctions__
#define __SpoutFunctions__

#include "..\..\SpoutSDK\spout.h"

using namespace spoututils;

class SpoutFunctions {

public:

	SpoutFunctions();
	~SpoutFunctions();

	Spout spout; // For access to all functions

	//
	// Spout SDK functions
	//

	// New for 2.007

	// Sender
	bool SetupSender(const char* SenderName, unsigned int width, unsigned int height, bool bInvert = true, DWORD dwFormat = 0);
	bool SendTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo = 0);
	bool SendFboData(GLuint FboID);
	bool SendImageData(const unsigned char* pixels, GLenum glFormat = GL_RGBA, GLuint HostFbo = 0);
	unsigned int GetWidth();
	unsigned int GetHeight();
	long GetFrame();
	double GetFps();
	void HoldFps(int fps);

	// Receiver
	void SetupReceiver(unsigned int width, unsigned int height, bool bInvert = false);
	void SetReceiverName(const char * SenderName);
	bool IsUpdated();
	bool IsConnected();
	void SelectSender();
	bool ReceiveTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo = 0);
	bool ReceiveImageData(unsigned char *pixels, GLenum glFormat = GL_RGBA, GLuint HostFbo = 0);
	const char * GetSenderName();
	unsigned int GetSenderWidth();
	unsigned int GetSenderHeight();
	double GetSenderFps();
	long GetSenderFrame();
	bool IsFrameNew();

	// Common
	void DisableFrameCount();
	bool IsFrameCountEnabled();

	//
	// 2.006 and earlier
	//

	// Sender
	bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
	bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height);
	void ReleaseSender(DWORD dwMsec = 0);
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	bool SendFboTexture(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	void RemovePadding(const unsigned char *source, unsigned char *dest, unsigned int width, unsigned int height, unsigned int source_stride, GLenum glFormat = GL_RGBA);

	// Receiver
	bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false);
	void ReleaseReceiver();
	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO = 0);
	bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	bool SelectSenderPanel(const char* message = NULL);
	bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);

	bool IsInitialized();
	bool BindSharedTexture();
	bool UnBindSharedTexture();

	int  GetSenderCount();
	bool GetSender(int index, char* sendername, int MaxSize = 256);
	bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	bool GetActiveSender(char* Sendername);
	bool SetActiveSender(const char* Sendername);

	// Utilities
	bool GetDX9(); // Return the flag that has been set
	bool SetDX9(bool bDX9 = true); // User request to use DirectX 9 (default is DirectX 11)
	bool GetMemoryShareMode();
	bool SetMemoryShareMode(bool bMem = true);
	int  GetShareMode();
	bool SetShareMode(int mode);
	bool GetBufferMode();
	void SetBufferMode(bool bActive); // Set the pbo availability on or off

	int  GetMaxSenders(); // Get maximum senders allowed
	void SetMaxSenders(int maxSenders); // Set maximum senders allowed
	bool GetHostPath(const char *sendername, char *hostpath, int maxchars); // The path of the host that produced the sender
	int  GetVerticalSync();
	bool SetVerticalSync(bool bSync = true);

	// Adapter functions
	int  GetNumAdapters(); // Get the number of graphics adapters in the system
	bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
	int  GetAdapter(); // Get the SpoutDirectX global adapter index
	bool SetAdapter(int index = 0); // Set required graphics adapter for output

	// OpenGL
	bool CreateOpenGL();
	bool CloseOpenGL();


private:

	bool bIsSending;
	char m_SenderNameSetup[256];
	char m_SenderName[256];
	GLuint m_TextureID;
	GLuint m_TextureTarget;
	bool m_bInvert;
	bool m_bUpdate;
	bool m_bUseActive;
	bool m_bConnected;
	unsigned int m_Width;
	unsigned int m_Height;


};

////////////////////////////////////////////////////////////////////////////////

#endif
