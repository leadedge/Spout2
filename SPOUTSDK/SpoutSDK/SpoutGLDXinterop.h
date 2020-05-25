/*

			spoutGLDXinterop.h

	Functions to manage texture sharing using the NVIDIA GL/DX opengl extensions

	https://www.opengl.org/registry/specs/NV/DX_interop.txt


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
#ifndef __spoutGLDXinterop__ // standard way as well
#define __spoutGLDXinterop__

#include "SpoutCommon.h"
#include "SpoutDirectX.h"
#include "SpoutSenderNames.h"
#include "SpoutMemoryShare.h"
#include "SpoutCopy.h"
#include "SpoutFrameCount.h"

#include <windowsx.h>
#include <d3d9.h>	// DX9
#include <d3d11.h>	// DX11
#include <gl/gl.h>
#include <gl/glu.h> // For glerror
#include <shlwapi.h> // for path functions
#include "SpoutGLextensions.h" // include last due to redefinition problems with OpenCL

using namespace spoututils;

#define STRINGIFY(A) #A

class SPOUT_DLLEXP spoutGLDXinterop {

	public:

		spoutGLDXinterop();
		~spoutGLDXinterop();

		// Spout objects
		spoutDirectX spoutdx; // DirectX functions
		spoutSenderNames senders; // Sender management
		spoutCopy spoutcopy; // Memory copy and rgb-rgba conversion functions
		spoutMemoryShare memoryshare; // Memory sharing
		spoutFrameCount frame; // Sender frame counter and texture access mutex

		// Initialization functions
		bool LoadGLextensions(); // Load required opengl extensions
		bool CreateInterop(HWND hWnd, const char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		void CleanupInterop();

		// Direct read/write of sender shared mempry block
		bool getSharedInfo(const char* sharedMemoryName, SharedTextureInfo* info);
		bool setSharedInfo(const char* sharedMemoryName, SharedTextureInfo* info);

		// Texture read/write
		bool WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true,  GLuint HostFBO=0);
		bool ReadTexture(const char* sendername, GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO = 0);
		
		// Pixel read/write
		bool WriteTexturePixels(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
		bool ReadTexturePixels (const char* sendername, unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);
		
		// Direct shared texture access
		bool BindSharedTexture();
		bool UnBindSharedTexture();
		GLuint GetSharedTextureID();
#ifdef legacyOpenGL
		bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);
		bool DrawToSharedTexture(GLuint TexID, GLuint TexTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);
#endif

		// DX9 application support
		bool WriteDX9memory(LPDIRECT3DSURFACE9 source_surface);
		bool SetDX9device(IDirect3DDevice9Ex* pDevice);
		bool WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface);

		// DX11 shared texture write and read
		bool ReadTexture (ID3D11Texture2D** texture);
		bool WriteTexture(ID3D11Texture2D** texture);
		bool WriteTextureReadback(ID3D11Texture2D** texture,
			GLuint TextureID, GLuint TextureTarget,
			unsigned int width, unsigned int height,
			bool bInvert = false, GLuint HostFBO = 0);

		// PBO functions for external access
		bool UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
								 unsigned int width, unsigned int height,
								 unsigned char* data, GLenum glFormat = GL_RGBA,
								 bool bInvert = false, GLuint HostFBO = 0);

		bool LoadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
							   unsigned int width, unsigned int height,
							   const unsigned char* data, GLenum glFormat = GL_RGBA, 
							   bool bInvert = false);

		// DX9
		bool GetDX9();
		bool SetDX9(bool bDX9);

		bool m_bUseDX9; // Use DX11 (default) or DX9
		bool m_bUseGLDX; // Use GPU texture processing
		bool m_bUseMemory; // Use memoryshare
		bool UseDX9(bool bDX9); // Includes DX11 compatibility check
		bool isDX9(); // Test for DX11 in case it failed to initialize
		bool GLDXcompatible();
		bool GLDXready();

		bool SetMemoryShareMode(bool bMem = true);
		bool GetMemoryShareMode();
		int  GetShareMode(); // 0 - memory, 1 - cpu, 2 - texture
		bool SetShareMode(int mode);

		HWND GetRenderWindow(); // Render window handle retrieved in GLDXcompatible

		bool IsGLDXavailable(); // Are GL/DX interop extensions supported
		bool IsPBOavailable();  // Are pbo extensions supported
		bool IsBLITavailable(); // Are fbo blit extensions available
		bool IsBGRAavailable(); // Are bgra extensions available
		bool IsSWAPavailable(); // Are swap extensions available
		bool IsCOPYavailable(); // Are copy extensions available

		void SetBufferMode(bool bActive); // Set the pbo availability on or off
		bool GetBufferMode();

		int GetNumAdapters(); // Get the number of graphics adapters in the system
		bool GetAdapterName(int index, char* adaptername, int maxchars); // Get an adapter name
		bool SetAdapter(int index); // Set required graphics adapter for output
		int GetAdapter(); // Get the SpoutDirectX global adapter index
		
		bool GetHostPath(const char* sendername, char* hostpath, int maxchars); // The path of the host that produced the sender
		int GetSpoutVersion(); // Get Spout version (2.005 and greater)

		// DX9
		D3DFORMAT DX9format; // the DX9 texture format to be used
		void SetDX9format(D3DFORMAT textureformat);
		bool CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		bool OpenDirectX9(HWND hWnd); // Initialize and prepare DirectX9
		void CleanupDX9();

		// DX11
		DXGI_FORMAT	DX11format; // the DX11 texture format to be used
		void SetDX11format(DXGI_FORMAT textureformat); // set format by user
		bool CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive);
		bool OpenDirectX11(); // Initialize and prepare DirectX11
		bool DX11available(); // Test for DX11 by attempting to open a device
		void CleanupDX11();

		// Common
		bool OpenDirectX(HWND hWnd, bool bDX9);
		void CleanupDirectX();
		HANDLE LinkGLDXtextures(void* pDXdevice, void* pSharedTexture, HANDLE dxShareHandle, GLuint glTextureID);
		
		// Locks for gl/dx interop functions
		HRESULT LockInteropObject(HANDLE hDevice, HANDLE *hObject);
		HRESULT UnlockInteropObject(HANDLE hDevice, HANDLE *hObject);

		int  GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);
		bool GetAdapterInfo(char* renderadapter, 
						    char* renderdescription, char* renderversion,
							char* displaydescription, char* displayversion,
							int maxsize, bool &bUseDX9);
	
		// OpenGL utilities
		bool CreateOpenGL();
		bool CloseOpenGL();
		bool CopyTexture(GLuint SourceID, GLuint SourceTarget, 
			GLuint DestID, GLuint DestTarget,
			unsigned int width, unsigned int height,
			bool bInvert, GLuint HostFBO);
		void CheckOpenGLTexture(GLuint &texID, GLenum GLformat,
								unsigned int newWidth, unsigned int newHeight,
								unsigned int &texWidth, unsigned int &texHeight);
		void InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height);

		void SaveOpenGLstate(unsigned int width, unsigned int height, bool bFitWindow = true);
		void RestoreOpenGLstate();
		bool GLerror();
		void PrintFBOstatus(GLenum status);

		// public for external access
		IDirect3DDevice9Ex* m_pDevice;     // DX9 device
		LPDIRECT3DTEXTURE9 m_dxTexture;    // the shared DX9 texture
		HANDLE m_dxShareHandle;            // the shared DX texture handle
		ID3D11Device* m_pd3dDevice;        // DX11 device
		ID3D11Texture2D* m_pSharedTexture; // The shared DX11 texture

protected:

		int m_SpoutVersion;       // Spout version held in this class for fast access
		bool m_bInitialized;      // this instance initialized flag
		bool m_bExtensionsLoaded; // extensions have been loaded
		unsigned int m_caps;      // extension capabilities

		bool m_bFBOavailable;     // fbo extensions available
		bool m_bBLITavailable;    // fbo blit extensions available
		bool m_bPBOavailable;     // pbo extensions available
		bool m_bSWAPavailable;    // swap extensions available
		bool m_bBGRAavailable;    // BGRA extensions are supported
		bool m_bGLDXavailable;    // GL/DX interop extensions are supported
		bool m_bCOPYavailable;    // copy extensions are supported

		SharedTextureInfo m_TextureInfo; // local texture info structure
		HWND m_hWnd;              // parent window
		HANDLE m_hSharedMemory;   // handle to the texture info shared memory

		GLuint m_glTexture;       // the OpenGL texture linked to the shared DX texture
		GLuint m_fbo;             // General fbo used for texture transfers
		GLuint m_TexID;           // Local texture used for memoryshare and CPU functions
		unsigned int m_TexWidth;  // width and height of local texture
		unsigned int m_TexHeight; // height of local texture

		// PBO support
		GLuint m_pbo[4];
		int PboIndex;
		int NextPboIndex;

		// For InitOpenGL and CloseOpenGL
		HDC m_hdc;
		HWND m_hwndButton;
		HGLRC m_hRc;

		ID3D11DeviceContext* m_pImmediateContext; // DX11
		IDirect3D9Ex* m_pD3D; // DX9

		// Interop
		HANDLE m_hInteropDevice; // handle to the DX/GL interop device
		HANDLE m_hInteropObject; // handle to the DX/GL interop object (the shared texture)

		bool getSharedTextureInfo(const char* sharedMemoryName);
		bool setSharedTextureInfo(const char* sharedMemoryName);

		// GL/DX interop texture functions
		bool WriteGLDXtexture (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true,  GLuint HostFBO=0);
		bool ReadGLDXtexture  (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false, GLuint HostFBO=0);
		bool WriteGLDXpixels  (const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
		bool ReadGLDXpixels   (unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);
		bool SetSharedTextureData (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);
		bool GetSharedTextureData (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);

		// Memoryshare functions
		bool WriteMemory (GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO=0);
		bool ReadMemory(const char* sendername, GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false, GLuint HostFBO=0);
		bool WriteMemoryPixels (const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
		bool ReadMemoryPixels(const char* sendername, unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
		bool ReadTexturePixels (GLuint SourceID, GLuint SourceTarget, unsigned int width, unsigned int height, unsigned char* dest, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);

#ifdef legacyOpenGL
		// Legacy Draw and Drawto functions
		bool DrawGLDXtexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true);
		bool DrawSharedMemory(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false);
		bool DrawToGLDXtexture(GLuint TexID, GLuint TexTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);
		bool DrawToSharedMemory(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);
#endif

		// Utility
		void trim(char* s);
		bool OpenDeviceKey(const char* key, int maxsize, char *description, char *version);

};

#endif
