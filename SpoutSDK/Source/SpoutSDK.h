/*

	SpoutSDK.h
 
	The main SDK include file


		Copyright (c) 2014>, Lynn Jarvis. All rights reserved.

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
#ifndef __SpoutSDK__
#define __SpoutSDK__

#include <windows.h>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <Mmsystem.h> // for timegettime
#include <direct.h> // for _getcwd
#include <shlwapi.h> // for path functions
#include "Shellapi.h" // for shellexecute

#pragma comment(lib, "shlwapi.lib") // for path functions
#pragma comment(lib, "Shell32.lib") // for shellexecute
#pragma comment(lib, "Advapi32.lib") // for registry functions
#pragma comment(lib, "Version.lib") // for VersionInfo API


#include "SpoutCommon.h"
#include "spoutMemoryShare.h"
#include "SpoutSenderNames.h"
#include "spoutGLDXinterop.h"

#if defined(__x86_64__) || defined(_M_X64)
	#define is64bit
// #elif defined(__i386) || defined(_M_IX86)
//	x86 32-bit
#endif

class SPOUT_DLLEXP Spout {

	public:

	Spout();
    ~Spout();

	// ================== //
	//	PUBLIC FUNCTIONS  //
	// ================== //

	// Sender
	bool CreateSender(char *name, unsigned int width, unsigned int height, DWORD dwFormat = 0);
	bool UpdateSender(char* Sendername, unsigned int width, unsigned int height);
	void ReleaseSender(DWORD dwMsec = 0);
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true, GLuint HostFBO=0);
	bool SendImage(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bAlignment = true, bool bInvert=true);

	// Receiver
	bool CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive = false);
	void ReleaseReceiver(); 

	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO=0);
	bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, GLuint HostFBO=0);
	
	bool GetImageSize (char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode);	

	bool BindSharedTexture();
	bool UnBindSharedTexture();
	
	bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true);
	bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);

	int  GetSenderCount();
	bool GetSenderName(int index, char* sendername, int MaxSize = 256);
	bool GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	bool GetActiveSender(char* Sendername);
	bool SetActiveSender(char* Sendername);
	
	// Utilities
	bool SetDX9(bool bDX9 = true); // set to use DirectX 9 (default is DirectX 11)
	bool GetDX9();

	bool GetMemoryShareMode();
	bool SetMemoryShareMode(bool bMemory = true);
	int  GetVerticalSync();
	bool SetVerticalSync(bool bSync = true);
	bool SelectSenderPanel(const char* message = NULL);

	bool CheckSpoutPanel(); // Public for debugging
	bool OpenSpout(); // Public for debugging
	
	spoutGLDXinterop interop; // Opengl/directx interop texture sharing

	// For debugging only - to disable/enable texture access locks in SpoutDirectX.cpp
	void UseAccessLocks(bool bUseLocks);
	void SpoutCleanUp(bool bExit = false);


/*
//
//
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb172558%28v=vs.85%29.aspx
//
// Compatible DX11/DX9 format for Texture2D
// http://msdn.microsoft.com/en-us/library/windows/desktop/ff471324%28v=vs.85%29.aspx
//
// DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM
// are compatible with DX9 - D3DFMT_A8B8G8R8
//
// Noted that DX11 -> DX9 only works if the DX11 format is set to DXGI_FORMAT_B8G8R8A8_UNORM
// if the DX9 format is set to D3DFMT_A8B8G8R8

	DXGI_FORMAT_R8G8B8A8_TYPELESS			= 27,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,

*/

	protected :

	// ================================= //
	//  PRIVATE VARIABLES AND FUNCTIONS  //
	// ================================= //
	char g_SharedMemoryName[256];
	char UserSenderName[256]; // used for the sender selection dialog
	unsigned int g_Width;
	unsigned int g_Height;
	HANDLE g_ShareHandle;
	DWORD g_Format;
	GLuint g_TexID;
	HWND g_hWnd;
	bool bMemory; // force memoryshare flag
	bool bGLDXcompatible;
	bool bMemoryShareInitOK;
	bool bDxInitOK;
	bool bInitialized;
	bool bChangeRequested;
	bool bSpoutPanelOpened;
	bool bUseActive; // Use the active sender for CreateReceiver
	SHELLEXECUTEINFOA ShExecInfo;

	bool GLDXcompatible();
	bool OpenReceiver(char *name, unsigned int& width, unsigned int& height);
	bool InitReceiver(HWND hwnd, char* sendername, unsigned int width, unsigned int height, bool bMemoryMode);
	bool InitSender(HWND hwnd, char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bMemoryMode);
	bool InitMemoryShare(bool bReceiver);
	bool ReleaseMemoryShare();
	// void SpoutCleanUp(bool bExit = false);
	bool FlipVertical(unsigned char *src, unsigned int width, unsigned int height, GLenum glFormat = GL_RGB);

	// FPS calcs - TODO cleanup
	double timeNow, timeThen, elapsedTime, frameTime, lastFrameTime, frameRate, fps, PCFreq, waitMillis, millisForFrame;
	__int64 CounterStart;

	// Registry read/write
	bool WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename);
	bool ReadPathFromRegistry(const char *filepath, const char *subkey, const char *valuename);

	// Find a file version
	bool FindFileVersion(const char *filepath, DWORD &versMS, DWORD &versLS);

	// TODO - used ? cleanup
	void StartCounter();
	double GetCounter();

};

#endif


