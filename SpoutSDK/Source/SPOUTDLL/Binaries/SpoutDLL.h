//
//					SpoutDLL.h - see SpoutDLL.cpp
// 
/*
		Copyright (c) 2014-2015. Lynn Jarvis. All rights reserved.

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
#ifndef __SpoutDLL__
#define __SpoutDLL__

#include <Windows.h>
#include <GL/GL.h>

namespace Spout2 {
	
	// exported functions

	//
	// Sender
	//

	extern "C" _declspec(dllexport)
	bool CreateSender(char *name, unsigned int width, unsigned int height, DWORD dwFormat = 0);

	extern "C" _declspec(dllexport)
	bool UpdateSender(char* Sendername, unsigned int width, unsigned int height);

	extern "C" _declspec(dllexport)
	void ReleaseSender(DWORD dwMsec = 0);

	extern "C" _declspec(dllexport)
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true, GLuint HostFBO=0);

	extern "C" _declspec(dllexport)
	bool SendImage(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bAlignment = true, bool bInvert=true);

	extern "C" _declspec(dllexport)
	bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);

	//
	// Receiver
	//

	extern "C" _declspec(dllexport)
	bool CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive = false);

	extern "C" _declspec(dllexport)
	void ReleaseReceiver(); 

	extern "C" _declspec(dllexport)
	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO = 0);

	extern "C" _declspec(dllexport)
	bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char * pixels, GLenum glFormat = GL_RGBA, GLuint HostFBO = 0);
	
	extern "C" _declspec(dllexport)
	bool GetImageSize (char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode);	

	extern "C" _declspec(dllexport)
	bool BindSharedTexture();

	extern "C" _declspec(dllexport)
	bool UnBindSharedTexture();
	
	extern "C" _declspec(dllexport)
	bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0);

	extern "C" _declspec(dllexport)
	int  GetSenderCount();

	extern "C" _declspec(dllexport)
	bool GetSenderName(int index, char* sendername, int MaxSize = 256);

	extern "C" _declspec(dllexport)
	bool GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	
	extern "C" _declspec(dllexport)
	bool GetActiveSender(char* Sendername);
	
	extern "C" _declspec(dllexport)
	bool SetActiveSender(char* Sendername);

	//
	// Utilities
	//

	extern "C" _declspec(dllexport)
	bool SetDX9(bool bDX9 = true); // set to use DirectX 9 (default is DirectX 11)
	
	extern "C" _declspec(dllexport)
	bool GetDX9();

	extern "C" _declspec(dllexport)
	bool GetMemoryShareMode();
	
	extern "C" _declspec(dllexport)
	bool SetMemoryShareMode(bool bMemory = true);
	
	extern "C" _declspec(dllexport)
	int  GetVerticalSync();
	
	extern "C" _declspec(dllexport)
	bool SetVerticalSync(bool bSync = true);
	
	extern "C" _declspec(dllexport)
	bool SelectSenderPanel(const char* message = NULL);
	
	//
	// OpenGL context for situations where there is none
	//

	extern "C" _declspec(dllexport)
	bool InitOpenGL();

	extern "C" _declspec(dllexport)
	bool CloseOpenGL();

}

#endif
