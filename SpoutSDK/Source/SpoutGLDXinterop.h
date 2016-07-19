/*

			spoutGLDXinterop.h

		Functions to manage texture sharing using the NVIDIA GL/DX opengl extensions

		https://www.opengl.org/registry/specs/NV/DX_interop.txt


		Copyright (c) 2014-2016, Lynn Jarvis. All rights reserved.

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
#include "spoutGLextensions.h"
#include "spoutDirectX.h"
#include "spoutSenderNames.h"
#include "SpoutMemoryShare.h"

#include <windowsx.h>
#include <d3d9.h>	// DX9
#include <d3d11.h>	// DX11
#include <gl/gl.h>
#include <gl/glu.h> // For glerror
#include <shlwapi.h> // for path functions


class SPOUT_DLLEXP spoutGLDXinterop {

	public:

		spoutGLDXinterop();
		~spoutGLDXinterop();

		// Initialization functions
		bool LoadGLextensions(); // Load required opengl extensions
		bool CreateInterop(HWND hWnd, const char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		// bool CheckInterop(HWND hWnd); // Check for successful open of the interop
		void CleanupInterop(bool bExit = false); // Cleanup with flag to avoid unknown crash bug

		// void setSharedMemoryName(char* sharedMemoryName, bool bReceive = true); 
		bool getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		bool setSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		
		bool WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true,  GLuint HostFBO=0);
		bool ReadTexture (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false, GLuint HostFBO=0);

		bool WriteTexture(ID3D11Texture2D** texture);
		bool ReadTexture (ID3D11Texture2D** texture);

		bool WriteDX9surface(LPDIRECT3DSURFACE9 source_surface);

		bool WriteTexturePixels(const unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO = 0);
		bool ReadTexturePixels (unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);

		// PBO functions for external access
		bool UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
								 unsigned int width, unsigned int height,
								 unsigned char *data, GLenum glFormat = GL_RGBA, GLuint HostFBO = 0);

		bool LoadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
							   unsigned int width, unsigned int height,
							   const unsigned char *data, GLenum glFormat = GL_RGBA);

		bool InitOpenGL();
		bool CloseOpenGL();

		bool BindSharedTexture();
		bool UnBindSharedTexture();

		bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true);
		bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);

		// DX9
		bool m_bUseDX9; // Use DX11 (default) or DX9
		bool UseDX9(bool bDX9);
		bool isDX9();

		// Set and get flags only
		void SetDX9(bool bDX9);
		bool GetDX9();

		bool m_bUseMemory; // Use memoryshare
		bool GetMemoryShareMode();
		bool SetMemoryShareMode(bool bMem = true);
		bool IsBGRAavailable(); // are the bgra extensions available
		bool IsPBOavailable(); // Are pbo extensions supported
		void SetBufferMode(bool bActive); // Set the pbo availability on or off

		D3DFORMAT DX9format; // the DX9 texture format to be used
		void SetDX9format(D3DFORMAT textureformat);

		int GetNumAdapters(); // Get the number of graphics adapters in the system
		bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
		bool SetAdapter(int index); // Set required graphics adapter for output
		int GetAdapter(); // Get the SpoutDirectX global adapter index

		bool GetHostPath(const char *sendername, char *hostpath, int maxchars); // The path of the host that produced the sender

		bool CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		bool OpenDirectX9(HWND hWnd); // Initialize and prepare DirectX9
		void CleanupDX9(bool bExit = false);

		// DX11
		DXGI_FORMAT	DX11format; // the DX11 texture format to be used
		void SetDX11format(DXGI_FORMAT textureformat); // set format by user

		bool CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive);
		bool OpenDirectX11(); // Initialize and prepare DirectX11
		bool DX11available(); // Test for DX11 by attempting to open a device
		void CleanupDX11(bool bExit = false);

		// Common
		bool OpenDirectX(HWND hWnd, bool bDX9);
		void CleanupDirectX(bool bExit = false);
		HANDLE LinkGLDXtextures(void* pDXdevice, void* pSharedTexture, HANDLE dxShareHandle, GLuint glTextureID);

		// Utilities
		bool GLDXcompatible();
		bool isOptimus();
		int GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);
		bool GetAdapterInfo(char *renderadapter, 
						    char *renderdescription, char *renderversion,
							char *displaydescription, char *displayversion,
							int maxsize, bool &bUseDX9);
		bool CheckSpout2004(); // LJ DEBUG - 2.005 transition utility TODO : remove
		
		// Spout objects
		spoutMemoryShare memoryshare; // Memory sharing
		spoutSenderNames senders; // Sender management
		spoutDirectX spoutdx; // DirectX class
		
		// Locks for gl/dx interop functions
		HRESULT LockInteropObject(HANDLE hDevice, HANDLE *hObject);
		HRESULT UnlockInteropObject(HANDLE hDevice, HANDLE *hObject);
		
		// Memoryshare functions
		bool WriteMemory (GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false,  GLuint HostFBO=0);
		bool ReadMemory  (GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false,  GLuint HostFBO=0);
		
		bool WriteMemoryPixels (const unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
		bool ReadMemoryPixels  (unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
		
		bool DrawSharedMemory  (float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false);
		bool DrawToSharedMemory(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);

		// Memory buffer utilities
		bool FlipBuffer(const unsigned char *src, unsigned char *dst, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA); 
		
		void rgba2rgb (void* rgba_source, void *rgb_dest,  unsigned int width, unsigned int height, bool bInvert = false);
		void rgba2bgr (void* rgba_source, void *rgb_dest,  unsigned int width, unsigned int height, bool bInvert = false);
		void rgba2bgra(void* rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false);
		void bgra2rgba(void* bgra_source, void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false);
		void rgb2rgba (void* rgb_source,  void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false);
		void bgr2rgba (void* bgr_source,  void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false);

		// OpenGL utilities
		bool CopyTexture(GLuint SourceID, GLuint SourceTarget, GLuint DestID, GLuint DestTarget,
						 unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO);
			
		GLuint GetGLtextureID(); // Get OpenGL shared texture ID

		// Global texture and fbo used for texture sharing
		GLuint m_glTexture; // the OpenGL texture linked to the shared DX texture
		GLuint m_fbo; // General fbo used for texture transfers

		// public for external access
		IDirect3DDevice9Ex* m_pDevice;     // DX9 device
		LPDIRECT3DTEXTURE9  m_dxTexture;   // the shared DX9 texture
		HANDLE m_dxShareHandle;            // the shared DX texture handle

		ID3D11Device* g_pd3dDevice;        // DX11 device
		ID3D11Texture2D* g_pSharedTexture; // The shared DX11 texture

		// For debugging
		void GLerror();
		void PrintFBOstatus(GLenum status);

protected:

		bool m_bInitialized;	  // this instance initialized flag
		bool m_bExtensionsLoaded; // extensions have been loaded
		unsigned int m_caps;      // extension capabilities
		bool m_bFBOavailable;     // fbo extensions available
		bool m_bBLITavailable;    // fbo blit extensions available
		bool m_bPBOavailable;     // pbo extensions available
		bool m_bSWAPavailable;    // swap extensions available
		bool m_bBGRAavailable;    // BGRA extensions are supported
		bool m_bGLDXavailable;    // GL/DX interop extensions are supported

		HWND              m_hWnd;          // parent window
		HANDLE            m_hSharedMemory; // handle to the texture info shared memory
		SharedTextureInfo m_TextureInfo;   // local texture info structure
		GLuint            m_TexID;         // Local texture used for memoryshare functions
		unsigned int      m_TexWidth;      // width and height of local texture
		unsigned int      m_TexHeight;     // for testing changes of memoryshare sender size

		// PBO support
		GLuint m_pbo[2];
		int PboIndex;
		int NextPboIndex;

		// DX11
		ID3D11DeviceContext* g_pImmediateContext;
		D3D_DRIVER_TYPE      g_driverType;
		D3D_FEATURE_LEVEL    g_featureLevel;

		// DX9
		IDirect3D9Ex* m_pD3D; // DX9 object
	
		HANDLE m_hInteropDevice; // handle to the DX/GL interop device
		HANDLE m_hInteropObject; // handle to the DX/GL interop object (the shared texture)
		HANDLE m_hAccessMutex;   // Texture access mutex lock handle

		bool getSharedTextureInfo(const char* sharedMemoryName);
		bool setSharedTextureInfo(const char* sharedMemoryName);

		bool OpenDeviceKey(const char* key, int maxsize, char *description, char *version);
		void trim(char * s);
		bool InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height);


};

#endif
