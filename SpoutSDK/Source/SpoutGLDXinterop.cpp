/**

	spoutGLDXinterop.cpp

	See also - spoutDirectX, spoutSenderNames

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
		========================

		15-07-14	- ReadTexturePixels - allowed for variable OpenGL format instead of RGB only.
					- Needs testing. 
					- TODO - variable gl format for WriteTexturePixels
		21.07.14	- removed local fbo and replaced with temporary fbo within
					  texture functions due to problems with Max / Jitter
		22-07-14	- added option for DX9 or DX11
		23-07-14	- cleanup of DX9 / DX11 functions
		29-07-14	- pass format 0 for DX9 sender
		31-07-14	- Corrected DrawTexture aspect argument
		13-08-14	- OpenGL texture retained on cleanup
		14-08-14	- Corrected texture delete without context
		16-08-14	- created DrawToSharedTexture
		18-08-14	- debugging with WriteTexture method
		 -- names class revision additions --
		19-08-14	- activated event locks
		01.09.14	- removed temp fbo for texture transfers and returned to use of a common fbo
					- delete texture and fbo on cleanup
					- set texture and fbo to zero on cleanup, otherwise errors in Jitter
					- changed to vertex array draw for DrawToSharedTexture
					- Removed PAINT message from OpenDirectX9 due to crash of sender in Magic
		03.09.14	- Replaced with UpdateWindow and limited to Resolume only.
					- Cleanup
		15.09.14	- corrected access lock for DrawToSharedTexture and ReadTexturePixels


*/

#include "spoutGLDXinterop.h"

spoutGLDXinterop::spoutGLDXinterop() {

	m_hWnd				= NULL;
	m_glTexture			= 0;
	m_fbo               = 0;
	m_hInteropObject	= NULL;
	m_hSharedMemory		= NULL;
	m_hInteropDevice	= NULL;
	
	m_hReadEvent		= NULL;
	m_hWriteEvent		= NULL;

	// DX9
	bUseDX9				= false; // Use DX9 or DX11
	m_pD3D				= NULL;
	m_pDevice			= NULL;
	m_dxTexture			= NULL;
	
	// DX11
	g_pd3dDevice		= NULL;
	g_pSharedTexture	= NULL;
	g_pImmediateContext = NULL;
	g_driverType		= D3D_DRIVER_TYPE_NULL;
	g_featureLevel		= D3D_FEATURE_LEVEL_11_0;
	DX11format			= DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9

	m_bInitialized		= false;
	bExtensionsLoaded	= false;
	bFBOavailable		= false;
	bBLITavailable		= false;
	bPBOavailable		= false;
	bSWAPavailable		= false;

	// LJ DEBUG
	// FPS calcs
	timeNow = timeThen = elapsedTime = frameTime = lastFrameTime = PCFreq = waitMillis = 0.0;
	fps = 60.0; //give a realistic starting value - win32 issues
	frameRate = 60.0;

}

spoutGLDXinterop::~spoutGLDXinterop() {
	
	m_bInitialized = false;
	// Because cleanup is not here it has to be specifically called
	// This is because it can crash on exit - see cleanup for details
}


// For external access so that the local global variables are used
bool spoutGLDXinterop::OpenDirectX(HWND hWnd, bool bDX9)
{
	if(bDX9) {
		bUseDX9 = true;
		return (OpenDirectX9(hWnd));
	}
	else {
		bUseDX9 = false;
		return (OpenDirectX11());
	}
}


// this function initializes and prepares Direct3D
bool spoutGLDXinterop::OpenDirectX9(HWND hWnd)
{
	HWND fgWnd = NULL;
	char fgwndName[256];

	// Already initialized ?
	if(m_pD3D != NULL) {
		return true;
	}

	// Create a IDirect3D9Ex object if not already created
	if(!m_pD3D) {
		m_pD3D = spoutdx.CreateDX9object(); 
	}

	if(m_pD3D == NULL) {
		return false;
	}

	// Create a DX9 device
	if(!m_pDevice) {
		m_pDevice = spoutdx.CreateDX9device(m_pD3D, hWnd); 
	}

	if(m_pDevice == NULL) {
		return false;
	}

	// Problem for FFGL plugins - might be a problem for other FFGL hosts or applications.
	// DirectX 9 device initialization creates black areas and the host window has to be redrawn.
	// But this causes a crash for a sender in Magic when the render window size is changed.
	// Not a problem for DirectX 11.
	// Not needed in Isadora.
	// Needed for Resolume.
	// For now, limit this to Resolume only.
	fgWnd = GetForegroundWindow();
	GetWindowTextA(fgWnd, fgwndName, 256);
	if(strstr(fgwndName, "Resolume") != NULL // Is resolume in the window title ?
	&& strstr(fgwndName, "magic") == NULL) { // Make sure it is not a user named magic project.
		 UpdateWindow(fgWnd);
	}

	return true;
}

// this function initializes and prepares Direct3D
bool spoutGLDXinterop::OpenDirectX11()
{
	// Quit if already initialized
	if(g_pd3dDevice != NULL)	return true;

	// Create a DirectX 11 device
	if(!g_pd3dDevice) g_pd3dDevice = spoutdx.CreateDX11device();
	if(g_pd3dDevice == NULL) return false;

	return true;
}


//
bool spoutGLDXinterop::CreateInterop(HWND hWnd, char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	bool bRet = true;
	DWORD format;
	D3DFORMAT DX9format = D3DFMT_A8R8G8B8; // fixed format for DX9 (21)

	// Needs an openGL context to work
	if(!wglGetCurrentContext()) {
		// MessageBoxA(NULL, "CreateInterop - no GL context", "Warning", MB_OK);
		return false;
	}

	if(bUseDX9) {
		// printf("CreateInterop - DX9 mode - format D3DFMT_A8R8G8B8 (%d) \n", DX9format);
		// DirectX 9
		format = (DWORD)DX9format;
	}
	else {
		// DirectX 11
		// Is this a DX11 texture or a DX9 sender texture?
		if(dwFormat > 0) {
			// printf("CreateInterop - DX11 mode - format %d \n", dwFormat);
			format = (DXGI_FORMAT)dwFormat;
		}
		else {
			// printf("CreateInterop - DX11 mode - default compatible format %d \n", DX11format);
			format = (DWORD)DX11format; // DXGI_FORMAT_B8G8R8A8_UNORM default compatible with DX9
		}
	}

	// Formats
	// DXGI_FORMAT_R8G8B8A8_UNORM; // default DX11 format - not compatible with DX9 (28)
	// DXGI_FORMAT_B8G8R8A8_UNORM; // compatoble DX11 format - works with DX9 (87)

	// Quit now if the receiver can't access the shared memory info of the sender
	// Otherwise m_dxShareHandle is set by getSharedTextureInfo and is the
	// shared texture handle of the Sender texture
	if (bReceive && !getSharedTextureInfo(sendername)) {
		return false;
	}

	// Check the sender format for a DX9 receiver
	// It can only be from a DX9 sender (format 0)
	// or from a compatible DX11 sender (format 87)
	if(bReceive && bUseDX9) {
		if(!(m_TextureInfo.format == 0 || m_TextureInfo.format == 87)) {
			// printf("Incompatible format %d \n", m_TextureInfo.format);
			return false;
		}
	}

	// Make sure DirectX has been initialized
	// Creates a global pointer to the DirectX device (DX11 g_pd3dDevice or DX9 m_pDevice)
	if(!OpenDirectX(hWnd, bUseDX9)) {
		return false;
	}

	// Allow for sender updates
	// TOD - check. Redundant - done in likGLDXtextures
	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}

	// Create a local opengl texture that will be linked to a shared DirectX texture
	if(m_glTexture) {
		glDeleteTextures(1, &m_glTexture);
		m_glTexture = 0;
	}
	glGenTextures(1, &m_glTexture);

	// Create an fbo for copying textures
	if(m_fbo) {
		glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;
	}
	glGenFramebuffersEXT(1, &m_fbo); 


	// Create textures and GLDX interop objects
	if(bUseDX9)	{
		bRet = CreateDX9interop(width, height, format, bReceive);
	}
	else {
		bRet = CreateDX11interop(width, height, format, bReceive);
	}

	if(!bRet) {
		return false;
	}

	// Now the global shared texture handle - m_dxShareHandle - has been set so a sender can be created
	// this creates the sender shared memory map and registers the sender
	if (!bReceive) {
		// Quit if sender creation failed - i.e. trying to create the same sender
		// TODO - modify SpoutPanel to detect format 21 ? How to know it is DX11 ?
		if(!senders.RegisterSenderName(sendername))
			return false;

		// Work out a better way of sending default format here
		if(bUseDX9)
			bRet = senders.UpdateSender(sendername, width, height, m_dxShareHandle);
		else
			bRet = senders.UpdateSender(sendername, width, height, m_dxShareHandle, format);
		
		if(!bRet) { // Did the update work ?
			return false;
		}
	}

	// Set up local values for this instance
	// Needed for texture read and write size checks
	m_TextureInfo.width			= (unsigned __int32)width;
	m_TextureInfo.height		= (unsigned __int32)height;
	m_TextureInfo.shareHandle	= (unsigned __int32)m_dxShareHandle;
	m_TextureInfo.format		= format;

	// Initialize texture transfer sync events - either sender or receiver can do this
	senders.InitEvents(sendername, m_hReadEvent, m_hWriteEvent);

	// Now it has initialized OK
	m_bInitialized = true;

	//
	// Now we have globals for this instance
	//
	// m_TextureInfo.width			- width of the texture
	// m_TextureInfo.height			- height of the texture
	// m_TextureInfo.shareHandle	- handle of the shared texture
	// m_TextureInfo.format			- format of the texture
	// m_glTexture					- a linked opengl texture
	// m_dxTexture					- a linked, shared DirectX texture created here
	// m_hInteropDevice				- handle to interop device created by wglDXOpenDeviceNV by init
	// m_hInteropObject				- handle to the connected texture created by wglDXRegisterObjectNV
	// m_hReadEvent					- event for reads
	// m_hWriteEvent				- event for writes
	// m_bInitialized				- whether it initialized OK

	// true means the init was OK, not the connection
	return true; 

}


//
// =================== DX9 ===============================
//
//		CreateDX9interop()
//	
//		bReceive		when receiving a texture from a DX application this must be set to true (default)
//						when sending a texture from GL to the DX application, set to false
//
bool spoutGLDXinterop::CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive) 
{

	// The shared texture handle of the Sender texture "m_dxShareHandle" 
	// is already set by getSharedTextureInfo, but should be NULL for a sender
	if (!bReceive) {
		// Create a new shared DirectX resource m_dxTexture 
		// with new local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture
	}
	
	// Create a shared DirectX9 texture - m_dxTexture
	// by giving it a sharehandle variable - m_dxShareHandle
	// For a SENDER : the sharehandle is NULL and a new texture is created
	// For a RECEIVER : the sharehandle is valid and becomes a handle to the existing shared texture
	// USAGE is D3DUSAGE_RENDERTARGET
	if(!spoutdx.CreateSharedDX9Texture(m_pDevice,
									   width,
									   height,
									   (D3DFORMAT)dwFormat,  // default is D3DFMT_A8R8G8B8
									   m_dxTexture,
									   m_dxShareHandle)) return false;


	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	m_hInteropObject = LinkGLDXtextures(m_pDevice, m_dxTexture, m_dxShareHandle, m_glTexture); 
	if(!m_hInteropObject) return false;


	return true;
}



//
// =================== DX11 ==============================
//
bool spoutGLDXinterop::CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive ) 
{
	// Create or use a shared DirectX texture that will be linked to the OpenGL texture
	// and get it's share handle for sharing textures
	if (bReceive) {
		// Retrieve the shared texture pointer via the sharehandle
		if(!spoutdx.OpenDX11shareHandle(g_pd3dDevice, &g_pSharedTexture, m_dxShareHandle)) {
			return false;
		}
	} else {
		// otherwise create a new shared DirectX resource g_pSharedTexture 
		// with local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture with a new share handle
		if(!spoutdx.CreateSharedDX11Texture(g_pd3dDevice, width, height, (DXGI_FORMAT)dwFormat, &g_pSharedTexture, m_dxShareHandle)) {
			return false;
		}
	}

	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	m_hInteropObject = LinkGLDXtextures(g_pd3dDevice, g_pSharedTexture, m_dxShareHandle, m_glTexture); 
	if(!m_hInteropObject) return false;

	return true;

}

//	Link a shared DirectX texture to an OpenGL texture
//	and create a GLDX interop object handle
//
//	IN	pSharedTexture  Pointer to shared the DirectX texture
//	IN	dxShareHandle   Handle of the DirectX texture to be shared
//	IN	glTextureID     ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returns             Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGLDXinterop::LinkGLDXtextures (	void* pDXdevice,
											void* pSharedTexture,
											HANDLE dxShareHandle,
											GLuint glTexture) 
{

	HANDLE hInteropObject;

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if(!m_hInteropDevice) m_hInteropDevice = wglDXOpenDeviceNV(pDXdevice);
	if (m_hInteropDevice == NULL) return false;

	// prepare shared resource
	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	if (!wglDXSetResourceShareHandleNV(pSharedTexture, dxShareHandle))
		return NULL;

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	hInteropObject = wglDXRegisterObjectNV( m_hInteropDevice,
											pSharedTexture,	// DX texture
											glTexture,		// OpenGL texture
											GL_TEXTURE_2D,	// Must be TEXTURE_2D
											WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read

	return hInteropObject;

}

// LJ DEBUG - TODO - not working
// Re-link a gl texture to the shared directX texture
bool spoutGLDXinterop::LinkGLtexture(GLuint glTexture) 
{
	// printf("LinkGLtexture(%d)\n", glTexture);
	if(g_pd3dDevice == NULL || g_pSharedTexture == NULL || m_dxShareHandle == NULL) {
		// printf("    null handles\n");
		return false;
	}

	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		// printf("    unregister\n");
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}
	// printf("    unregister OK\n");

	if (m_hInteropDevice != NULL) {
		wglDXCloseDeviceNV(m_hInteropDevice);
	}

	// printf("    close device OK\n");
	m_hInteropDevice = NULL;
	m_hInteropObject = NULL;


	m_hInteropDevice = wglDXOpenDeviceNV(g_pd3dDevice);
	if (m_hInteropDevice == NULL) {
		// printf("    open device fail\n");
		return false;
	}
	// printf("    open device OK\n");

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	m_hInteropObject = wglDXRegisterObjectNV(g_pd3dDevice, 
											 g_pSharedTexture,		// DX texture
											 glTexture,				// OpenGL texture
											 GL_TEXTURE_2D,			// Must be TEXTURE_2D
											 WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read

	if(!m_hInteropObject) {
		// printf("    null InteropObject\n");
		return false;
	}

	// printf("    InteropObject OK\n");

	return true;

}

void spoutGLDXinterop::CleanupDirectX()
{
	if(bUseDX9)
		CleanupDX9();
	else
		CleanupDX11();
}


void spoutGLDXinterop::CleanupDX9()
{
	if(m_pD3D != NULL) m_pD3D->Release();
	if(m_pDevice != NULL) m_pDevice->Release();

	// 01.09.14 - texture release was missing for a receiver - caused a VRAM leak
	// If an existing texture exists, CreateTexture can fail with and "unknown error"
	// so delete any existing texture object
	if (m_dxTexture) m_dxTexture->Release();
	m_dxTexture = NULL;

	m_pD3D = NULL;
	m_pDevice = NULL;

}

void spoutGLDXinterop::CleanupDX11()
{
	if(g_pSharedTexture != NULL) g_pSharedTexture->Release();
	if(g_pd3dDevice != NULL) g_pd3dDevice->Release();
	g_pSharedTexture = NULL;
	g_pd3dDevice = NULL;
	
	spoutdx.CloseDX11();

}


// this is the function that cleans up Direct3D and the gldx interop
// The exit flag is a fix - trouble is with wglDXUnregisterObjectNV
// which crashes on exit to the program but not if called
// while the program is running. Likely due to no GL context on exit
void spoutGLDXinterop::CleanupInterop(bool bExit)
{
	// Some of these things need an opengl context so check
	if(wglGetCurrentContext() != NULL) {
		// Problem here on exit, but not on change of resolution while the program is running !?
		// On exit there may be no openGL context but while the program is running there is
		if(!bExit && m_hInteropDevice != NULL && m_hInteropObject != NULL) {
			wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
			m_hInteropObject = NULL;
		}
		if (m_hInteropDevice != NULL) {
			wglDXCloseDeviceNV(m_hInteropDevice);
			m_hInteropDevice = NULL;
		}

		if(m_glTexture)	{
			glDeleteTextures(1, &m_glTexture);
			m_glTexture = 0;
		}

		if(m_fbo) {
			glDeleteFramebuffersEXT(1, &m_fbo);
			m_fbo = 0;
		}

	} // endif there is an opengl context

	CleanupDirectX();

	senders.CloseEvents(m_hReadEvent, m_hWriteEvent);

	m_bInitialized = false;

}

//
//	Load the Nvidia gl/dx extensions
//
bool spoutGLDXinterop::LoadGLextensions() 
{
	unsigned int caps = 0;

	caps = loadGLextensions();

	char buffer [33];
	_itoa_s(caps, buffer, 2);

	if(caps == 0) return false;

	if(caps & GLEXT_SUPPORT_FBO) bFBOavailable = true;
	if(caps & GLEXT_SUPPORT_FBO_BLIT) bBLITavailable = true;
	if(caps & GLEXT_SUPPORT_PBO) bPBOavailable = true;
	if(caps & GLEXT_SUPPORT_SWAP) bSWAPavailable = true;

	return true;
}

// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedTextureInfo(char* sharedMemoryName) {

	unsigned int w, h;
	HANDLE handle;
	DWORD format;
	if (!senders.FindSender(sharedMemoryName, w, h, handle, format))
	{
		return false;
	}

	m_dxShareHandle = (HANDLE)handle;
	m_TextureInfo.width = w;
	m_TextureInfo.height = h;
	m_TextureInfo.shareHandle = (__int32)handle;
	m_TextureInfo.format = format;
	// senders.UnlockMap(hLock);

	return true;

}


// Set texture info to shared memory for the sender init
// width and height must have been set first
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::setSharedTextureInfo(char* sharedMemoryName) {

	return senders.UpdateSender(sharedMemoryName, 
							m_TextureInfo.width,
							m_TextureInfo.height,
							m_dxShareHandle,
							m_TextureInfo.format);


}

// Return current sharing handle, width and height of a Sender
// Note - use the map directly - we must not use getSharedTextureInfo
// which resets the local info structure from shared memory !!!
// A receiver checks this all the time so it has to be compact
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info) 
{
	return senders.getSharedInfo(sharedMemoryName, info);
} // end getSharedInfo


// Sets the given info structure to shared memory with the given name
// IMPORTANT: this modifies the local structure
// Used to change the texture dimensions before init
bool spoutGLDXinterop::setSharedInfo(char* sharedMemoryName, SharedTextureInfo* info)
{
	m_TextureInfo.width			= info->width;
	m_TextureInfo.height		= info->height;
	m_dxShareHandle				= (HANDLE)info->shareHandle; 
	// the local info structure handle "m_TextureInfo.shareHandle" gets converted 
	// into (unsigned __int32) from "m_dxShareHandle" by setSharedTextureInfo
	if(setSharedTextureInfo(sharedMemoryName)) {
		return true;
	}
	else {
		return false;
	}
}

// Utilities
//
// GLDXcompatible
//
bool spoutGLDXinterop::GLDXcompatible()
{
	//
	// ======= Hardware compatibility test =======
	//
	// Call LoadGLextensions for an initial hardware compatibilty check and
	// also load Glew for the Nvidia OpenGL/Directx interop extensions.
	// This will fail straight away for non Nvidia hardware but it needs
	// an additional check. It is possible that the extensions load OK, 
	// but that initialization will still fail. This occurs when wglDXOpenDeviceNV
	// fails. This has been noted on dual graphics machines with the NVIDIA Optimus driver.
	// "GLDXcompatible" tests for this by initializing directx and calling wglDXOpenDeviceNV
	// If OK and the debug flag has not been set all the parameters are available
	// Otherwise it is limited to memory share
	//
	HDC hdc = wglGetCurrentDC(); // OpenGl device context is needed
	if(!hdc) {
		// MessageBoxA(NULL, "Spout compatibility test\nCannot get GL device context", "OpenSpout", MB_OK);
		return false;
	}
	HWND hWnd = WindowFromDC(hdc); // can be null though
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();
	if(bExtensionsLoaded) {
		// all OK and not debug memoryshare
		// try to set up directx and open the GL/DX interop
		if(OpenDirectX(hWnd, bUseDX9)) {
			// if it passes here all is well
			return true;
		}
	} // end hardware compatibility test
	return false;

} // end GLDXcompatible



// ----------------------------------------------------------
//		Access to texture using DX/GL interop functions
// ----------------------------------------------------------

// DRAW A TEXTURE INTO THE THE SHARED TEXTURE VIA AN FBO
bool spoutGLDXinterop::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}
	
	if(width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height) {
		return false;
	}

	if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

		// Draw the input texture into the shared texture via an fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
		glBindTexture(GL_TEXTURE_2D, m_glTexture);

		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnable(TextureTarget);
		glBindTexture(TextureTarget, TextureID);

		GLfloat tc[4][2] = {0};
		if(bInvert) {
			tc[0][0] = 0.0;   tc[0][1] = max_y;
			tc[1][0] = 0.0;   tc[1][1] = 0.0;
			tc[2][0] = max_x; tc[2][1] = 0.0;
			tc[3][0] = max_x; tc[3][1] = max_y;
		}
		else {
			tc[0][0] = 0.0;   tc[0][1] = 0.0;
			tc[1][0] = 0.0;   tc[1][1] = max_y;
			tc[2][0] = max_x; tc[2][1] = max_y;
			tc[3][0] = max_x; tc[3][1] = 0.0;
		}

		GLfloat verts[] =  {
						-aspect, -1.0,   // bottom left
						-aspect,  1.0,   // top left
						 aspect,  1.0,   // top right
						 aspect, -1.0 }; // bottom right

		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, 0, tc );
		glEnableClientState(GL_VERTEX_ARRAY);		
		glVertexPointer(2, GL_FLOAT, 0, verts );
		glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindTexture(TextureTarget, 0);
		glDisable(TextureTarget);

		glBindTexture(GL_TEXTURE_2D, 0);	
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
	}
	return true;
}


// DRAW THE SHARED TEXTURE
bool spoutGLDXinterop::DrawSharedTexture(float max_x, float max_y, float aspect)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	// Wait for writer to stop writing
	if(senders.CheckAccess(m_hWriteEvent)) {

		// go ahead and access the shared texture to draw it
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_glTexture); // bind shared texture
			glColor4f(1.f, 1.f, 1.f, 1.f);

			// Note origin of input texture at top left
			// FFGL examples have origin at bottom left
			// but needs to be this way or it comes out inverted
			// TODO - convert to vertex array
			glBegin(GL_QUADS);
			glTexCoord2f(0.0,	max_y);	glVertex2f(-aspect,-1.0); // lower left
			glTexCoord2f(0.0,	0.0);	glVertex2f(-aspect, 1.0); // upper left
			glTexCoord2f(max_x, 0.0);	glVertex2f( aspect, 1.0); // upper right
			glTexCoord2f(max_x, max_y);	glVertex2f( aspect,-1.0); // lower right
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0); // unbind shared texture
			glDisable(GL_TEXTURE_2D);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			
			// drop through to manage events and return true;
		} // if lock failed just keep going
	}

	// Allow readers and writers access
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end DrawSharedTexture



#ifdef USE_PBO_EXTENSIONS
// TODO - this is not used - to be texted
// Load a texture into pixels via PBO - NOTE RGB only
bool spoutGLDXinterop::LoadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, unsigned char *data)
{
	GLuint pixelBufferID;

	// PBO method - load into PBO - then texture
	// Note that PBO functions have been loaded explicitly
	// Presumably lets the GPU continue after the data has been transferred to the PBO
	// but it has an additional memcpy. 

	// Really needs two PBOs
	
	// "CopyMemory" seems a little faster than memcpy although no detailed timing tests done
	// Seems to have a small advantage, perhaps 3 or 4 fps at 1920x1080. 

	// The most time consuming part is loading from shared memory. 
	// There is a small speed advantage by putting the memoryshare read
	// inside here instead of via a local buffer (perhaps 1-2 fps) but then the changes in 
	// Sender image size are not made.
	if(bPBOavailable) {

		void *pboMemory;
		glGenBuffers(1, &pixelBufferID);

		// Bind buffer
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pixelBufferID);

		// Null existing data
		glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*3, NULL, GL_STREAM_DRAW);

		// Map buffer - returns pointer to buffer
		pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

		// Write data into pboMemory - presumably this is the time consuming bit
		// might be a very slight speed increase by using CopyMemory instead of memcpy
		CopyMemory(pboMemory, (void *)data, width*height*3);

		// Unmap buffer, indicating we are done writing data to it
		glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

		// while PBO is still bound, transfer the data from the PBO to the texture
		glEnable(TextureTarget);
		glBindTexture (TextureTarget, TextureID);
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void *)0);
		glDisable(TextureTarget);

		// Unbind buffer
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);

		if(pixelBufferID) glDeleteBuffers(1, &TextureID);

	}
	else {

		//
		// Texture method using glTexSubImage2D may be subject to "GPU pipeline stalling"
		//
		// http://blogs.msdn.com/b/shawnhar/archive/2008/04/14/stalling-the-pipeline.aspx
		//
		glEnable(TextureTarget);
		glBindTexture (TextureTarget, TextureID);
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void *)data);
		glBindTexture (TextureTarget, 0);
		glDisable(TextureTarget);

	}

	return true;

}
#endif


//
// COPY A TEXTURE TO THE SHARED TEXTURE
//
bool spoutGLDXinterop::WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height) {
		return false;
	}

	/*
	// glFlush(); // 0.80 - 0.90
	// glFinish(); // 1.4 ms / frame - adds approx 0.5 - 0.7 ms / frame
	// timeNow = (double)timeGetTime();
	// timeThen = timeNow;
	// Basic code for debugging 0.85 - 0.90 msec
	wglDXLockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	wglDXUnlockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);

	// glFlush(); // 0.80 - 0.90
	// glFinish();
	timeNow = (double)timeGetTime();
	elapsedTime = (timeNow-timeThen); // +0.5; // to allow for integer truncation ?
	frameTime = elapsedTime/1000.0; // in seconds
	if( frameTime  > 0.000001) {
		fps	= 1.0 / frameTime;
		printf("frame time = %f, fps = %f\n", frameTime*1000.0, fps);
	}
	timeThen = timeNow;
	return true;
	*/

	// Original blit method with checks - 0.75 - 0.85 msec
	// Wait for reader to stop reading
	if(senders.CheckAccess(m_hReadEvent)) { // go ahead and write to the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// fbo is a  local FBO and width/height are the dimensions of the texture.
			// "TextureID" is the source texture, and "m_glTexture" is destination texture
			// which should have been already created
			// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

			// Attach the Input texture to the color buffer in our frame buffer - note texturetarget 
			#ifdef USE_GLEW
			glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
			#else
			glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
			#endif
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

			// Default invert flag is false but then it comes out inverted
			// so do the flip to get it the right way up if the user wants that
			if(bInvert) {
				if (bBLITavailable) { // fbo blitting supported - flip the texture while copying
					// attach target texture to second attachment point
					#ifdef USE_GLEW
					glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_glTexture, 0);
					#else
					glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_glTexture, 0);
					#endif
					glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

					// copy one texture buffer to the other while flipping upside down (OpenGL and DirectX have different texture origins)
					glBlitFramebufferEXT(0, 0,			// srcX0, srcY0, 
										 width, height, // srcX1, srcY1
										 0, height,		// dstX0, dstY0,
										 width, 0,		// dstX1, dstY1,
										 GL_COLOR_BUFFER_BIT, GL_NEAREST); // GLbitfield mask, GLenum filter
				} 
				else { // no blitting supported - directly copy to texture line by line to invert
					// bind dx texture (destination)
					glBindTexture(GL_TEXTURE_2D, m_glTexture);
					// copy from fbo to the bound texture
					// Not useful - Line by line method is very slow !! ~ 12msec/frame 1920 hd
					for(unsigned int i = 0; i <height; i++) glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, 0, height-i-1, width, 1);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
			else { 
				// no invert flag means leave it alone and it comes out inverted (bInvert false)
				glBindTexture(GL_TEXTURE_2D, m_glTexture);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
				glBindTexture(GL_TEXTURE_2D, 0);		
			}

			// unbind our frame buffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			// unbind the shared texture
			glBindTexture(GL_TEXTURE_2D, 0);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
	
		// Allow readers and writers access
		senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

		return true;
	}

	// There is no reader
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	return false;


} // end WriteTexture


// COPY IMAGE PIXELS TO THE SHARED TEXTURE  - note RGB only
bool spoutGLDXinterop::WriteTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		return false;
	}

	// Wait for reader to stop reading
	if(senders.CheckAccess(m_hReadEvent)) {
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			glBindTexture(GL_TEXTURE_2D, m_glTexture); // The  shared GL texture
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		} // if lock failed just keep going
	}

	// Allow readers and writers access
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end WriteTexturePixels


// COPY THE SHARED TEXTURE TO AN OUTPUT TEXTURE
bool spoutGLDXinterop::ReadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height)
{

	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		return false;
	}

	/*
	// Basic code for debugging
	if(wglDXLockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject)) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); 
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
		glBindTexture(TextureTarget, TextureID);
		glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
		glBindTexture(TextureTarget, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 
		wglDXUnlockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
		return true;
	}
	return false;
	*/

	// Wait for writer to signal ready to read
	if(senders.CheckAccess(m_hWriteEvent)) { // Read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our local fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); 
			// glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO); 
			// Attach the shared texture to the color buffer in our frame buffer
			// needs GL_TEXTURE_2D as a target for our shared texture
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
				// bind output texture (destination)
				glBindTexture(TextureTarget, TextureID);
				// copy from framebuffer (fbo) to the bound texture
				glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
				// unbind the texture
				glBindTexture(TextureTarget, 0);
			}
			// Unbind our fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
	}
	
	// Allow readers and writers access
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end ReadTexture


//
// COPY THE SHARED TEXTURE TO IMAGE PIXELS - 15-07-14 allowed for variable format instead of RGB only
bool spoutGLDXinterop::ReadTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, int glFormat)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) return false;
	if(width != m_TextureInfo.width || height != m_TextureInfo.height) return false;

	// retrieve opengl texture data directly to image pixels rather than via an fbo and texture

	// Wait for writer to stop writing
	if(senders.CheckAccess(m_hWriteEvent)) {
		// go ahead and read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			glGetTexImage(GL_TEXTURE_2D, 0,  glFormat,  GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			// drop through to manage events and return true;
		} // if lock failed just keep going
	}

	// Allow readers and writers access
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end ReadTexturePixels 


// BIND THE SHARED TEXTURE
// for use in an application - this locks the interop object and binds the shared texture
// Afterwards a call to UnbindSharedTxeture MUST be called
bool spoutGLDXinterop::BindSharedTexture()
{
	bool bRet = false;

	if(m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	// Wait for writer to signal ready to read
	if(senders.CheckAccess(m_hWriteEvent)) { // Read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	return bRet;

} // end BindSharedTexture


// UNBIND THE SHARED TEXTURE
// for use in an application - this unbinds the shared texture and unlocks the interop object
bool spoutGLDXinterop::UnBindSharedTexture()
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;
	
	// Unbind our shared OpenGL texture
	glBindTexture(GL_TEXTURE_2D,0);
	// unlock dx object
	UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
	// Allow readers and writers access
	senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	
	return true;

} // end BindSharedTexture


/*

	GL/DX Interop lock

	A return value of TRUE indicates that all objects were
    successfully locked.  A return value of FALSE indicates an
    error. If the function returns FALSE, none of the objects will be locked.

	Attempting to access an interop object via GL when the object is
    not locked, or attempting to access the DirectX resource through
    the DirectX API when it is locked by GL, will result in undefined
    behavior and may result in data corruption or program
    termination. Likewise, passing invalid interop device or object
    handles to this function has undefined results, including program
    termination.

	Note that only one GL context may hold the lock on the
    resource at any given time --- concurrent access from multiple GL
    contexts is not currently supported.

	http://halogenica.net/sharing-resources-between-directx-and-opengl/

	This lock triggers the GPU to perform the necessary flushing and stalling
	to guarantee that the surface has finished being written to before reading from it. 

	DISCUSSION: The Lock/Unlock calls serve as synchronization points
    between OpenGL and DirectX. They ensure that any rendering
    operations that affect the resource on one driver are complete
    before the other driver takes ownership of it.

	This function assumes only one object to 

	Must return S_OK (0) - otherwise the error can be checked.

*/
HRESULT spoutGLDXinterop::LockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	// lock dx object
	if(wglDXLockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_BUSY :			// One or more of the objects in <hObjects> was already locked.
				hr = E_ACCESSDENIED;	// General access denied error
				printf("	spoutGLDXinterop::LockInteropObject ERROR_BUSY\n");
				break;
			case ERROR_INVALID_DATA :	// One or more of the objects in <hObjects>
										// does not belong to the interop device
										// specified by <hDevice>.
				hr = E_ABORT;			// Operation aborted
				printf("	spoutGLDXinterop::LockInteropObject ERROR_INVALID_DATA\n");
				break;
			case ERROR_LOCK_FAILED :	// One or more of the objects in <hObjects> failed to 
				hr = E_ABORT;			// Operation aborted
				printf("	spoutGLDXinterop::LockInteropObject ERROR_LOCK_FAILED\n");
				break;
			default:
				hr = E_FAIL;			// unspecified error
				printf("	spoutGLDXinterop::LockInteropObject UNKNOWN_ERROR\n");
				break;
		} // end switch
	} // end false

	return hr;

} // LockInteropObject


//
// Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGLDXinterop::UnlockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	if (wglDXUnlockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_NOT_LOCKED :
				hr = E_ACCESSDENIED;
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_NOT_LOCKED\n");
				break;
			case ERROR_INVALID_DATA :
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_INVALID_DATA\n");
				hr = E_ABORT;
				break;
			case ERROR_LOCK_FAILED :
				hr = E_ABORT;
				// printf("	spoutGLDXinterop::UnLockInteropObject ERROR_LOCK_FAILED\n");
				break;
			default:
				hr = E_FAIL;
				// printf("	spoutGLDXinterop::UnLockInteropObject UNKNOWN_ERROR\n");
				break;
		} // end switch
	} // end fail

	return hr;

} // end UnlockInteropObject


void spoutGLDXinterop::UseDX9(bool bDX9)
{
	bUseDX9 = bDX9;
}

bool spoutGLDXinterop::isDX9()
{
	return bUseDX9;
}

void spoutGLDXinterop::SetDX11format(DXGI_FORMAT textureformat)
{
	DX11format = textureformat;
}


int spoutGLDXinterop::GetVerticalSync()
{
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();

	// needed for both sender and receiver
	if(bSWAPavailable) {
		return(wglGetSwapIntervalEXT());
	}
	return 0;
}


bool spoutGLDXinterop::SetVerticalSync(bool bSync)
{
	if(!bExtensionsLoaded) bExtensionsLoaded = LoadGLextensions();

	if(bSWAPavailable) {
		if(bSync) {
			wglSwapIntervalEXT(1); // lock to monitor vsync
		}
		else {
			wglSwapIntervalEXT(0); // unlock from monitor vsync
		}
		return true;
	}
	return false;
}

