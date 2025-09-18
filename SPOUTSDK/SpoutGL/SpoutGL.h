/*

	SpoutGL.h
	
	Base class for OpenGL SpoutSDK
	See also Sender and Receiver wrapper classes.

	Copyright (c) 2021-2025, Lynn Jarvis. All rights reserved.

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

#ifndef __spoutGL__
#define __spoutGL__

// Change the path as required
#include "SpoutGLextensions.h" // include first so that gl.h is not included first if Glew is used
#include "SpoutCommon.h" // for dll build and utilities
#include "SpoutSenderNames.h" // for sender creation and update
#include "SpoutDirectX.h" // for DX11 shared textures
#include "SpoutFrameCount.h" // for mutex lock and new frame signal
#include "SpoutCopy.h" // for pixel copy

#include <direct.h> // for _getcwd
#include <tlhelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

#pragma warning(disable : 26485)

// Used throughout
using namespace spoututils;


class SPOUT_DLLEXP spoutGL {

	public:

	spoutGL();
    virtual ~spoutGL();

	//
	// OpenGL shared texture access
	//

	// Bind OpenGL shared texture
	bool BindSharedTexture();
	// Un-bind OpenGL shared texture
	bool UnBindSharedTexture();
	// OpenGL shared texture ID
	GLuint GetSharedTextureID();

	//
	// Graphics compatibility
	//

	// Get user auto GPU/CPU share
	bool GetAutoShare();
	// Set application auto GPU/CPU share
	void SetAutoShare(bool bAuto = true);
	// Get user CPU share
	bool GetCPUshare();
	// Set application CPU share
	// (re-test GL/DX compatibility if set to false)
	void SetCPUshare(bool bCPU = true);
	// OpenGL texture share compatibility
	bool IsGLDXready();

	//
	// User settings recorded in the registry by "SpoutSettings"
	//
	
	// Get user buffering mode
	bool GetBufferMode();
	// Set application buffering mode
	void SetBufferMode(bool bActive = true);
	// Get user number of pixel buffers
	int GetBuffers();
	// Set application number of pixel buffers
	void SetBuffers(int nBuffers);
	// Get user Maximum senders allowed
	int GetMaxSenders();
	// Set user Maximum senders allowed
	void SetMaxSenders(int maxSenders);
	
	//
	// 2.006 compatibility
	//

	// Get user DX9 mode
	bool GetDX9();
	// Set user DX9 mode
	bool SetDX9(bool bDX9 = true);
	// Get user memory share mode
	bool GetMemoryShareMode();
	// Set user memory share mode
	bool SetMemoryShareMode(bool bMem = true);
	// Get user CPU mode
	bool GetCPUmode();
	// Set user CPU mode
	bool SetCPUmode(bool bCPU);
	// Get user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	int GetShareMode();
	// Set user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	void SetShareMode(int mode);

	//
	// Information
	//

	// The path of the host that produced the sender
	bool GetHostPath(const char* sendername, char* hostpath, int maxchars);
	// Vertical sync status
	int GetVerticalSync();
	// Lock to monitor vertical sync
	//   1 - wait for 1 cycle vertical refresh
	//   0 - buffer swaps are not synchronized to a video frame
	//  -1 - adaptive vsync
	bool SetVerticalSync(int interval = 1);
	// Get Spout version
	int GetSpoutVersion();

	//
	// Utility
	//
	
	// Copy OpenGL texture with optional invert
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget, GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO = 0);
	// Copy OpenGL texture data to a pixel buffer
	bool ReadTextureData(GLuint SourceID, GLuint SourceTarget,
		void* data, unsigned int width, unsigned int height, unsigned int rowpitch,
		GLenum dataformat, GLenum datatype, bool bInvert = false, GLuint HostFBO = 0);
	// Correct for image stride
	void RemovePadding(const unsigned char* source, unsigned char* dest,
		unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat = GL_RGBA);

	// OpenGL error reporting
	bool GLerror();

	// DX11 texture read
	//  o Copy from the shared DX11 texture to a DX11 texture
	bool ReadTexture(ID3D11Texture2D** texture);

	// DX11 texture write
	//  o Copy a DX11 texture to the shared DX11 texture
	bool WriteTexture(ID3D11Texture2D** texture);

	// DX11 texture write with readback to OpenGL
	//   o Copy a DX11 texture to the DX11 shared texture
	//   o Copy the linked OpenGL texture back to an OpenGL texture
	bool WriteTextureReadback(ID3D11Texture2D** texture, GLuint TextureID, GLuint TextureTarget,
		unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO = 0);

	// Copy a region of the DX11 texture
	bool WriteTextureReadback(ID3D11Texture2D** texture, GLuint TextureID, GLuint TextureTarget,
		unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height,
		bool bInvert, GLuint HostFBO = 0);

	// Initialize OpenGL and DX11
	//     o Load extensions and check for availability and function
	//     o Open DirectX and check for availability
	//     o Compatibility test for use or GL/DX interop
	//     o Optionally re-test compatibility even if already initialized
	bool OpenSpout(bool bRetest = false);
	// Initialize DirectX
	bool OpenDirectX();
	// Close DirectX and free resources
	void CloseDirectX();

	//
	// Formats
	//

	// Get sender DX11 shared texture format
	DXGI_FORMAT GetDX11format();
	// Set sender DX11 shared texture format
	void SetDX11format(DXGI_FORMAT textureformat);
	// Return OpenGL compatible DX11 format
	DXGI_FORMAT DX11format(GLint glformat);
	// Return DX11 compatible OpenGL format
	GLint GLDXformat(DXGI_FORMAT textureformat = DXGI_FORMAT_UNKNOWN);
	// Return OpenGL texture internal format
	GLint GLformat(GLuint TextureID, GLuint TextureTarget);
	// Return OpenGL texture format description
	std::string GLformatName(GLint glformat = 0);
	// Create an OpenGL window and context for situations where there is none.
	//   Not used if applications already have an OpenGL context.
	//   Always call CloseOpenGL afterwards.
	bool CreateOpenGL(HWND hwnd = nullptr);
	// Close OpenGL window
	bool CloseOpenGL();
	// Class initialization status
	bool IsSpoutInitialized();
	// Perform tests for GL/DX interop availability and compatibility
	bool GLDXready(bool bRetest = false);
	// Set host path to sender shared memory
	bool SetHostPath(const char* sendername);
	// Set sender PartnerID field with CPU sharing method
	// GL/DX compatibility and use of GL memory functions
	bool SetSenderID(const char* sendername, bool bCPU, bool bGLDX, bool bGLmemory);
	
	//
	// 2.006 compatibility
	//

	bool OpenDirectX11(ID3D11Device* pDevice = nullptr);
	ID3D11Device* GetDX11Device();
	ID3D11DeviceContext* GetDX11Context();
	void CleanupDirectX();
	void CleanupDX11();
	bool CleanupInterop();

	//
	// OpenGL extensions
	//

	bool LoadGLextensions();
	bool IsGLDXavailable();    // Nvidia GL/DX interop extensions supported
	bool IsBLITavailable();    // fbo blit extensions available
	bool IsSWAPavailable();    // swap extensions available
	bool IsBGRAavailable();    // bgra extensions available
	bool IsCOPYavailable();    // copy extensions available
	bool IsGLMEMavailable();   // Khronos GL memory extensions supported
	bool IsPBOavailable();     // pbo extensions supported
	bool IsCONTEXTavailable(); // Context extension supported
	float GetGLversion();      // OpenGL version - 3.0, 4.0, 4.6 etc

	//
	// Legacy OpenGL functions
	//

	// See _SpoutCommon.h_ #define legacyOpenGL
#ifdef legacyOpenGL
	void SaveOpenGLstate(unsigned int width, unsigned int height, bool bFitWindow = true);
	void RestoreOpenGLstate();
#endif

	//
	// Public for special use
	//

	bool InteropReady(); // Ready for either NVidia or GL memory GL/DX interop
	bool GetNVready();   // Using Nvidia WGL_NV_DX interop
	bool GetGLready();   // Khronos GL memory capable
	// Nvidia WGL_NV_DX usage (for testing)
	bool GetNVusage();
	void SetNVusage(bool bReady);


	// Return a handle to the the Nvidia GL/DX interop device
	HANDLE GetInteropDevice();
	// Return a handle to the the Nvidia GL/DX interop ojject
	HANDLE GetInteropObject();
	// Pointer to the shared DirectX texture
	ID3D11Texture2D* GetDXsharedTexture();
	// Create OpenGL texture
	void InitTexture(GLuint& texID, GLenum GLformat, unsigned int width, unsigned int height);
	// Copy OpenGL to shared DirectX 11 shared texture via CPU
	bool WriteDX11texture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);
	// Copy from shared DX11 texture to OpenGL shared texture via CPU
	bool ReadDX11texture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);
	// Read from an OpenGL texture to and RGBA buffer using pbo
	bool UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget,
		unsigned int width, unsigned int height, unsigned int pitch,
		unsigned char* data, GLenum glFormat = GL_RGBA,
		bool bInvert = false, GLuint HostFBO = 0);
	// Load pixels to an OpenGL texture using pbo
	bool LoadTexturePixels(GLuint TextureID, GLuint TextureTarget,
		unsigned int width, unsigned int height, 
		const unsigned char* data, int GLformat = GL_RGBA,
		bool bInvert = false);

	//
	// Data sharing
	//

	// Write data to shared memory
	bool WriteMemoryBuffer(const char* name, const char* data, int length);
	// Read data from shared memory
	int ReadMemoryBuffer(const char* name, char* data, int maxlength);
	// Create a shared memory buffer
	bool CreateMemoryBuffer(const char* name, int length);
	// Delete a shared memory buffer
	bool DeleteMemoryBuffer();
	// Get the number of bytes available for data transfer
	int GetMemoryBufferSize(const char* name);

	//
	// For external access
	//

	// DirectX 11 texture sharing
	spoutDirectX spoutdx;
	// Pixel buffer copying
	spoutCopy spoutcopy;
	// Spout sender management
	spoutSenderNames sendernames;
	// Frame counting management
	spoutFrameCount frame;

protected :
	
	// For 2.006(receive only) / WriteMemoryBuffer / ReadMemoryBuffer
	SpoutSharedMemory memoryshare;

	//
	// GL/DX functions
	//

	// Link a shared DirectX texture to an OpenGL texture
	bool CreateInterop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive);

	// Nvidia WGL_NV_DX interop
	HANDLE LinkGLDXtextures(void* pDXdevice, void* pSharedTexture, GLuint glTextureID);

	// Khronos OpenGL memory GL/DX interop
	bool LinkGLDXmemoryTextures(unsigned int width, unsigned int height, DXGI_FORMAT dxFormat,
		HANDLE texhandle,    // D3D11 texture handle
		GLuint glMemObj,     // GL memory object for D3D11 texture import
		GLuint glTextureID); // GL texture backed by the memory object

	// Lock/unlock interop resoources
	HRESULT LockInteropObject(HANDLE hDevice, HANDLE* hObject);
	HRESULT UnlockInteropObject(HANDLE hDevice, HANDLE* hObject);

	 // Free OpenGL resources
	void CleanupGL();

	// Check OpenGL texture format for BGRA swizzle
	void CheckOpenGLformat(GLuint TextureID, GLuint TextureTarget);

	// OpenGL texture create
	void CheckOpenGLTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height);

	// OpenGL texture copy to/from the shared texture
	bool WriteGLDXtexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	bool ReadGLDXtexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO = 0);
	bool SetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);
	
	// OpenGL pixel copy
	bool WriteGLDXpixels(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	bool ReadGLDXpixels(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
	
	// PBOs for OpenGL pixel copy
	int m_nBuffers = 2;

	// UnloadTexturePixels
	GLuint m_pbo[4]{};
	int PboIndex = 0;
	int NextPboIndex = 0;

	// LoadTexturePixels
	GLuint m_loadpbo[4]{};
	int PboLoadIndex = 0;
	int NextPboLoadIndex = 0;
		
	// Pixels <-> DX11
	bool WriteDX11pixels(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
	bool ReadDX11pixels(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
	bool WritePixelData(const unsigned char* pixels, ID3D11Texture2D* pStagingTexture, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert);
	bool ReadPixelData(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert);

	// Staging textures for DX11 CPU copy
	ID3D11Texture2D* m_pStaging[2]{};
	int m_Index;
	int m_NextIndex;
	bool CheckStagingTextures(unsigned int width, unsigned int height, int nTextures);

	// 2.006 shared memory
	bool ReadMemoryTexture(const char* sendername, GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO = 0);
	bool ReadMemoryPixels(const char* sendername, unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
	bool WriteMemoryPixels(const char* sendername, const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);

	// Utility
	bool OpenDeviceKey(const char* key, int maxsize, char* description, char* version);
	void trim(char* s);

	// Errors
	void PrintFBOstatus(GLenum status);

	//
	// Class globals
	//

	// Sender/Receiver
	char m_SenderName[256]{};
	char m_SenderNameSetup[256]{};
	unsigned int m_Width = 0;
	unsigned int m_Height = 0;

	// Utility
	GLuint m_fbo = 0;   // Fbo used for OpenGL functions
	GLuint m_TexID = 0; // Class texture used for invert copy
	unsigned int m_TexWidth = 0;
	unsigned int m_TexHeight = 0;
	DWORD m_TexFormat = GL_RGBA;

	// Shared texture
	// Common for Nidia or Khronos GL interop
	GLuint m_glTexture = 0; // OpenGL shared texture
	ID3D11Texture2D* m_pSharedTexture = nullptr; // DirectX shared texture
	HANDLE m_dxShareHandle = nullptr; // DirectX shared texture handle
	DXGI_FORMAT m_DX11format = DXGI_FORMAT_B8G8R8A8_UNORM; // DirectX 11 shared texture format
	DWORD m_dwFormat = DXGI_FORMAT_B8G8R8A8_UNORM; // DWORD texture format used throughout

	// GL/DX interop
	HANDLE m_hInteropDevice = nullptr; // Handle to the Nvidia interop device
	HANDLE m_hInteropObject = nullptr; // Handle to the Nvidia interop object
	GLuint m_glMemObj = 0;             // Khronos OpenGL memory object
	GLenum m_glFormat = GL_BGRA8_EXT;  // OpenGL internal format used for glTextureStorageMem2DEXT
	bool m_bInteropFailed = false;     // Interop failure flag to avoid repeats

	// General
	HWND m_hWnd = nullptr;  // OpenGL window
	int m_SpoutVersion = 0; // Spout version

	// For CreateOpenGL and CloseOpenGL
	HDC m_hdc = nullptr;
	HWND m_hwndButton = nullptr;
	HGLRC m_hRc = nullptr;

	// Status flags
	bool m_bConnected    = false; // Receiver connected to a sender
	bool m_bUpdated      = false; // Receiver update flag
	bool m_bInitialized  = false; // Receiver or sender initialization
	bool m_bSender       = true;  // Sender or receiver
	bool m_bGLDXdone     = false; // Compatibility test has been done

	// Sharing modes
	bool m_bAuto         = true;  // Auto share mode - user set
	bool m_bCPU          = false; // Global CPU mode - user set

	bool m_bGLDX         = false; // Nvidia GL/DX interop compatible
	bool m_bGLmemory     = false; // Khronos GL memory compatible
	bool m_bNThandle    = false;  // NT handle required for import
	bool m_bUseGLDX      = false; // Use Nvidia GL/DX interop
	bool m_bTextureShare = true;  // Using texture sharing methods
	bool m_bCPUshare     = false; // Using CPU sharing methods
	bool m_bMemoryShare  = false; // Using 2.006 memoryshare methods

	// Received sender sharing methods from sender info
	bool m_bSenderCPU      = false; // Sender using CPU sharing methods
	bool m_bSenderGLDX     = true;  // Sender Nvidia hardware GL/DX compatibility
	bool m_bSenderGLmemory = false; // Sender using Khronos GL memory functions for GL/DX interop

	// For SpoutPanel sender selection
	bool m_bSpoutPanelOpened = false;
	bool m_bSpoutPanelActive = false;
	SHELLEXECUTEINFOA m_ShExecInfo{};

	// OpenGL extensions
	unsigned int m_caps = 0;
	bool m_bGLDXavailable    = false;
	bool m_bGLMEMavailable   = false;
	bool m_bFBOavailable     = false;
	bool m_bBLITavailable    = false;
	bool m_bPBOavailable     = false;
	bool m_bSWAPavailable    = false;
	bool m_bBGRAavailable    = false;
	bool m_bCOPYavailable    = false;
	bool m_bCONTEXTavailable = false;
	bool m_bExtensionsLoaded = false;

	// For timing tests
	double sumtime   = 0.0;
	double numtime   = 0.0;
	double avgtime   = 0.0;
	double numframes = 0.0;

};

#endif
