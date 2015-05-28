/*

			spoutGLDXinterop.h

		LJ - leadedge@adam.com.au

		Functions to manage texture sharing using the NVIDIA GL/DX opengl extensions

		https://www.opengl.org/registry/specs/NV/DX_interop.txt


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
		bool CreateInterop(HWND hWnd, char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		bool CheckInterop(HWND hWnd); // Check for successful open of the interop
		void CleanupInterop(bool bExit = false); // Cleanup with flag to avoid unknown crash bug

		void setSharedMemoryName(char* sharedMemoryName, bool bReceive = true); 
		bool getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		bool setSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		
		bool ReadTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, GLuint HostFBO=0);
		bool WriteTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bAlignment = true);

		bool ReadTexture (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false, GLuint HostFBO=0);
		bool WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false, GLuint HostFBO=0);
		#ifdef USE_PBO_EXTENSIONS
		bool LoadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, unsigned char *data);
		#endif

		bool BindSharedTexture();
		bool UnBindSharedTexture();

		bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true);
		bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);

		// DX9
		bool bUseDX9; // Use DX11 (default) or DX9
		bool UseDX9(bool bDX9);
		bool isDX9();

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
		void CleanupDX11();

		// Common
		bool OpenDirectX(HWND hWnd, bool bDX9);
		HANDLE LinkGLDXtextures(void* pDXdevice, void* pSharedTexture, HANDLE dxShareHandle, GLuint glTextureID);
		void CleanupDirectX();

		// LJ DEBUG - not working
		bool LinkGLtexture(GLuint glTexture) ;

		// Utilities
		bool GLDXcompatible();
		int GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);
		bool GetAdapterInfo(char *renderadapter, 
						    char *renderdescription, char *renderversion,
							char *displaydescription, char *displayversion,
							int maxsize, bool &bUseDX9);

		spoutMemoryShare MemoryShare;	// Shared memory method
		spoutSenderNames senders;	// Sender management
		spoutDirectX spoutdx;	// DirectX class

		// Locks for gl/dx interop functions
		HRESULT LockInteropObject(HANDLE hDevice, HANDLE *hObject);
		HRESULT UnlockInteropObject(HANDLE hDevice, HANDLE *hObject);

		GLuint m_glTexture;		// the OpenGL texture linked to the shared DX texture
		GLuint m_fbo;

		// ====================
		// LJ DEBUG
		// public for debugging
		IDirect3DDevice9Ex* m_pDevice;   // DX9 device
		LPDIRECT3DTEXTURE9  m_dxTexture; // the shared DX9 texture
		void GLerror();
		// ====================


protected:

		bool m_bInitialized;    // this instance initialized flag
		bool bExtensionsLoaded; // extensions have been loaded
		bool bFBOavailable;     // fbo extensions available
		bool bBLITavailable;    // fbo blit extensions available
		bool bPBOavailable;     // pbo extensions available
		bool bSWAPavailable;    // swap extensions available

		HWND              m_hWnd;          // parent window
		HANDLE            m_hSharedMemory; // handle to the texture info shared memory
		SharedTextureInfo m_TextureInfo;   // local texture info structure

		// DX11
		ID3D11Device*        g_pd3dDevice;
		ID3D11DeviceContext* g_pImmediateContext;
		D3D_DRIVER_TYPE      g_driverType;
		D3D_FEATURE_LEVEL    g_featureLevel;
		ID3D11Texture2D*     g_pSharedTexture; // The shared DX11 texture

		// DX9
		IDirect3D9Ex*       m_pD3D;      // DX9 object
		// IDirect3DDevice9Ex* m_pDevice;   // DX9 device
		// LPDIRECT3DTEXTURE9  m_dxTexture; // the shared DX9 texture
		
		HANDLE m_hInteropDevice; // handle to the DX/GL interop device
		HANDLE m_hInteropObject; // handle to the DX/GL interop object (the shared texture)
		HANDLE m_dxShareHandle;  // shared DX texture handle
		HANDLE m_hAccessMutex;   // Texture access mutex lock handle

		bool getSharedTextureInfo(char* sharedMemoryName);
		bool setSharedTextureInfo(char* sharedMemoryName);

		bool OpenDeviceKey(const char* key, int maxsize, char *description, char *version);
		void trim(char * s);


		// LJ DEBUG
		// Timing calcs
		// __int64 CounterStart;
		// double PCFreq;
		// void StartCounter();
		// double GetCounter();

};

#endif
