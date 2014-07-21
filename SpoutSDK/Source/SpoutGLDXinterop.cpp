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
*/

#include "spoutGLDXinterop.h"

spoutGLDXinterop::spoutGLDXinterop() {

	m_hWnd				= NULL;
	m_glTexture			= 0;
	m_hInteropObject	= NULL;
	m_hSharedMemory		= NULL;
	m_hInteropDevice	= NULL;
	
	m_hReadEvent		= NULL;
	m_hWriteEvent		= NULL;
	
	// DX11
	g_pd3dDevice		= NULL;
	g_pSharedTexture	= NULL;
	g_pImmediateContext = NULL;
	g_driverType		= D3D_DRIVER_TYPE_NULL;
	g_featureLevel		= D3D_FEATURE_LEVEL_11_0;
	DX11format			= DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9

	m_bInitialized		= false;
	bFBOavailable		= false;
	bBLITavailable		= false;
	bPBOavailable		= false;
	bSWAPavailable		= false;

}

spoutGLDXinterop::~spoutGLDXinterop() {
	m_bInitialized = false;
	// Because cleanup is not here it has to be specifically called
	// This is becasue it can crash on exit - see cleanup for details
}

// For external access so that the local global variables are used
bool spoutGLDXinterop::OpenDirectX(HWND hWnd)
{
	return (OpenDirectX11(hWnd, g_pd3dDevice, m_hInteropDevice));
}


// this function initializes and prepares Direct3D
// It can be called more than once - requires an openGL context for wglDXOpenDeviceNV
// Returns a pointer to the DirectX device and a handle to the GLDX interop device
bool spoutGLDXinterop::OpenDirectX11(HWND hWnd, ID3D11Device* &pd3dDevice, HANDLE &hInteropDevice)
{
	// Create a DirectX 11 device
	if(!pd3dDevice) pd3dDevice = spoutdx.CreateDX11device(hWnd);
	if(pd3dDevice == NULL) return false;

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if(!hInteropDevice) hInteropDevice = wglDXOpenDeviceNV(g_pd3dDevice);
	if (hInteropDevice == NULL) return false;

	return true;
}

//
bool spoutGLDXinterop::CreateInterop(HWND hWnd, char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	DXGI_FORMAT format = DX11format; // DXGI_FORMAT_B8G8R8A8_UNORM;

	// printf("spoutGLDXinterop::CreateInterop (%s, %d, %d, %d, %d)\n", sendername, width, height, dwFormat, bReceive);

	// Needs an openGL context to work
	if(!wglGetCurrentContext()) {
		// MessageBoxA(NULL, "CreateInterop - no GL context", "Warning", MB_OK);
		return false;
	}

	if(dwFormat > 0) format = (DXGI_FORMAT)dwFormat;

	// printf("spoutGLDXinterop::CreateInterop - format passed = %d, format used = %d\n", dwFormat, format);

	// LJ DEBUG - testing
	// format = DXGI_FORMAT_R8G8B8A8_UNORM; // not compatible with DX9
	// format = DXGI_FORMAT_B8G8R8A8_UNORM; // works with DX9
	// format = DXGI_FORMAT_B8G8R8X8_UNORM; // crash

	// Quit now if the receiver can't access the shared memory info of the sender
	// Otherwise m_dxShareHandle is set by getSharedTextureInfo and is the
	// shared texture handle of the Sender texture
	if (bReceive && !getSharedTextureInfo(sendername)) {
		return false;
	}

	// Make sure DirectX has been initialized
	// Also opens the GLDX interop (wglDXOpenDeviceNV)
	// Returns a pointer to the DirectX11 device (g_pd3dDevice)
	// Returns a handle to a GL/DirectX interop device ( m_hInteropDevice)
	if(!OpenDirectX11(hWnd, g_pd3dDevice, m_hInteropDevice)) {
		return false;
	}

	// Allow for sender updates
	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}

	// Create a local opengl texture that will be linked to a shared DirectX texture
	if(m_glTexture) glDeleteTextures(1, &m_glTexture);
	glGenTextures(1, &m_glTexture);

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
		if(!spoutdx.CreateSharedDX11Texture(g_pd3dDevice, width, height, format, &g_pSharedTexture, m_dxShareHandle)) {
			return false;
		}
	}

	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	m_hInteropObject = LinkGLDXtextures(m_hInteropDevice, g_pSharedTexture, m_dxShareHandle, m_glTexture); 

	if(m_hInteropObject == NULL) {
		return false;
	}
	
	// Now the global shared texture handle - m_dxShareHandle - has been set so a sender can be created
	// this creates the sender shared memory map and registers the sender
	if (!bReceive) senders.CreateSender(sendername, width, height, m_dxShareHandle, format);

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
//	Link a shared DirectX texture to an OpenGL texture
//	and create a GLDX interop object handle
//
//	IN	pSharedTexture	Pointer to shared the DirectX texture
//	IN	dxShareHandle	Handle of the DirectX texture to be shared
//	IN	glTextureID		ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returnse			Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGLDXinterop::LinkGLDXtextures (	HANDLE hInteropDevice,
											ID3D11Texture2D* pSharedTexture,
											HANDLE dxShareHandle,
											GLuint glTexture) 
{

	HANDLE hInteropObject;

	// prepare shared resource
	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	if (!wglDXSetResourceShareHandleNV(pSharedTexture, dxShareHandle))
		return NULL;

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	hInteropObject = wglDXRegisterObjectNV( hInteropDevice,
											pSharedTexture,	// DX11 texture
											glTexture,		// OpenGL texture
											GL_TEXTURE_2D,	// Must be TEXTURE_2D
											WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read

	if(!hInteropObject) {
		// DWORD dwError = GetLastError();
		// printf("    wglDXRegisterObjectNV error %d [%x]\n", dwError, dwError);
		return NULL;
	}

	return hInteropObject;

}


void spoutGLDXinterop::CleanupDirectX()
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
// but a double chack for safety.
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
		}
		if (m_glTexture > 0) glDeleteTextures(1, &m_glTexture);
		m_glTexture = 0;
	} // endif there is an opengl contex

	m_hInteropDevice = NULL;
	m_hInteropObject = NULL;
	m_glTexture = 0;

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

bool spoutGLDXinterop::getSharedTextureInfo(char* sharedMemoryName) {

	HANDLE hMap;
	char* pBuf;
	// HANDLE hLock;

	// Lock the shared memory map with it's mutex - wait 4 frames for access
	// if(!(hLock = senders.LockMap(sharedMemoryName))) return false;

	// Open the named memory map for the active sender and return a pointer to the memory
	pBuf = senders.OpenMap(sharedMemoryName, sizeof(SharedTextureInfo), hMap);
	if(hMap == NULL || pBuf == NULL) return false;

	memcpy((void *)&m_TextureInfo, (void *)pBuf, sizeof(SharedTextureInfo) );

	// Set local share handle from the info
	m_dxShareHandle = (HANDLE)m_TextureInfo.shareHandle;

	senders.CloseMap(pBuf, hMap);
	// senders.UnlockMap(hLock);

	return true;

}


// Set texture info to shared memory for the sender init
// width and height must have been set first
bool spoutGLDXinterop::setSharedTextureInfo(char* sharedMemoryName) {

	HANDLE hMap;
	char* pBuf;
	// HANDLE hLock;

	// Create or open shared memory - allocate enough for the texture info
	m_hSharedMemory = senders.CreateMap(sharedMemoryName, sizeof(SharedTextureInfo));
	if(m_hSharedMemory == NULL) return false;

	// Lock the shared memory map with it's mutex - wait 4 frames for access
	// if(!(hLock = senders.LockMap(sharedMemoryName))) return false;

	// Open the named map to access it
	pBuf = senders.OpenMap(sharedMemoryName, sizeof(SharedTextureInfo), hMap);
	if(hMap == NULL || pBuf == NULL) return false;

	// convert handle (either 32bit or 64bit) to unsigned __int32 for sharing between 32bit and 64bit
	m_TextureInfo.shareHandle = (unsigned __int32)m_dxShareHandle;

	memcpy( (void *)pBuf, &m_TextureInfo, sizeof(SharedTextureInfo) ); // write the info structure to shared memory

	senders.CloseMap(pBuf, hMap);
	// senders.UnlockMap(hLock);

	return true;

}

// Return current sharing handle, width and height of a Sender
// Note - use the map directly - we must not use getSharedTextureInfo
// which resets the local info structure from shared memory !!!
// A receiver checks this all the time so it has to be compact
bool spoutGLDXinterop::getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info) 
{

	HANDLE hMap; // handle to the memory map
	char *pBuf; // pointer to the memory map
	// HANDLE hLock;

	// Lock the shared memory map with it's mutex - wait 4 frames for access
	// if(!(hLock = senders.LockMap(sharedMemoryName))) return false;

	// Open the named memory map for the active sender and return a pointer to the memory
	pBuf = senders.OpenMap(sharedMemoryName, sizeof(SharedTextureInfo), hMap);

	if(hMap == NULL || pBuf == NULL) {
		return false;
	}

	memcpy((void *)info, (void *)pBuf, sizeof(SharedTextureInfo) );

	senders.CloseMap(pBuf, hMap);
	// senders.UnlockMap(hLock);

	return true;
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
	if(LoadGLextensions()) {
		// all OK and not debug memoryshare
		// try to set up directx and open the GL/DX interop
		if(OpenDirectX11(hWnd, g_pd3dDevice, m_hInteropDevice)) { // if it passes here all is well
			return true;
		}
	} // end hardware compatibility test
	return false;

} // end GLDXcompatible



// ----------------------------------------------------------
//		Access to texture using DX/GL interop functions
// ----------------------------------------------------------

// DRAW THE SHARED TEXTURE
bool spoutGLDXinterop::DrawSharedTexture(float max_x, float max_y)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	// Wait for writer to stop writing
	// if(senders.CheckAccess(m_hReadEvent)) {

		// go ahead and access the shared texture to draw it
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_glTexture); // bind shared texture
			glColor4f(1.f, 1.f, 1.f, 1.f);

			// Note origin of input texture at top left
			// FFGL examples have origin at bottom left
			// but needs to be this way or it comes out inverted
			glBegin(GL_QUADS);
			glTexCoord2f(0.0,	max_y);	glVertex2f(-1.0,-1.0); // lower left
			glTexCoord2f(0.0,	0.0);	glVertex2f(-1.0, 1.0); // upper left
			glTexCoord2f(max_x, 0.0);	glVertex2f( 1.0, 1.0); // upper right
			glTexCoord2f(max_x, max_y);	glVertex2f( 1.0,-1.0); // lower right
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0); // unbind shared texture
			glDisable(GL_TEXTURE_2D);

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			
			// drop through to manage events and return true;
		} // if lock failed just keep going
	// }
    // else {
		// printf("DrawSharedTexture - lock failed\n");
		// return false;
    // }

	// Allow readers and writers access
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end DrawSharedTexture



#ifdef USE_PBO_EXTENSIONS
// TOD - this is not used - to be texted
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
	GLuint tempFBO;

	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	if(width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height) {
		return false;
	}

	// Use a local fbo due to problems with Max Jitter
	glGenFramebuffersEXT(1, &tempFBO);

	/* 
	// Basic code for debugging
	wglDXLockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO);
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	wglDXUnlockObjectsNV(m_hInteropDevice, 1, &m_hInteropObject);
	glDeleteFramebuffersEXT(1, &tempFBO);
	tempFBO = 0;
	return true;
	*/

	// Wait for reader to stop reading
	// if(senders.CheckAccess(m_hReadEvent)) { // go ahead and write to the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// fbo is a  local FBO and width/height are the dimensions of the texture.
			// "TextureID" is the source texture, and "m_glTexture" is destination texture
			// which should have been already created
			// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
			// glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO);

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
					// copy from framebuffer (m_fbo here) to the bound texture
					// Not useful - Line by line method is very slow !!
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
		// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

		glDeleteFramebuffersEXT(1, &tempFBO);
		tempFBO = 0;


		return true;
	// }

	// There is no reader
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	//return false;


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
	// if(senders.CheckAccess(m_hReadEvent)) {
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			glBindTexture(GL_TEXTURE_2D, m_glTexture); // The  shared GL texture
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		} // if lock failed just keep going
	// }
    // else {
	//	return false;
    // }

	// Allow readers and writers access
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end WriteTexturePixels


// COPY THE SHARED TEXTURE TO AN OUTPUT TEXTURE
bool spoutGLDXinterop::ReadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height)
{

	// Local fbo
	GLuint tempFBO;

	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		// printf("spoutGLDXinterop::ReadTexture- error 1\n");
		return false;
	}

	if(width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		// printf("spoutGLDXinterop::ReadTexture- error 2\n");
		return false;
	}

	glGenFramebuffersEXT(1, &tempFBO);

	// printf("spoutGLDXinterop::ReadTexture [%d][%x] %dx%d\n", TextureID, TextureTarget, width, height);

	// Wait for writer to signal ready to read
	// if(senders.CheckAccess(m_hWriteEvent)) { // Read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			// Bind our local fbo
			// glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo); 
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO); 
			// Attach the shared texture to the color buffer in our frame buffer
			// needs GL_TEXTURE_2D as a target for our shared texture
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			// LJ DEBUG
			if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
				// bind output texture (destination)
				glBindTexture(TextureTarget, TextureID);
				// copy from framebuffer (fbo) to the bound texture
				glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
				// unbind the texture
				glBindTexture(TextureTarget, 0);
			}
			// else {
				// printf("spoutGLDXinterop::ReadTexture - fbo not complete\n");
			// }
			// Unbind our fbo
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 

			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		// else {
			// printf("spoutGLDXinterop::ReadTexture - lock error2\n");
		// }
		// Allow readers and writers access
		// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
		// return true;
	// }

	glDeleteFramebuffersEXT(1, &tempFBO);
	tempFBO = 0;

	return true;

} // end ReadTexture


//
// COPY THE SHARED TEXTURE TO IMAGE PIXELS - 15-07-14 allowed for variable format instead of RGB only
// #define GL_RGB   0x1907
// #define GL_RGBA  0x1908
bool spoutGLDXinterop::ReadTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, int glFormat)
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL) return false;
	if(width != m_TextureInfo.width || height != m_TextureInfo.height) return false;

	// printf("ReadTexturePixels - format = [%x]\n", glFormat);

	// retrieve opengl texture data directly to image pixels rather than via an fbo and texture

	// Wait for writer to stop writing
	// if(senders.CheckAccess(m_hReadEvent)) {
		// go ahead and read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			// glGetTexImage(GL_TEXTURE_2D, 0,  GL_RGB,  GL_UNSIGNED_BYTE, pixels);
			glGetTexImage(GL_TEXTURE_2D, 0,  glFormat,  GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			// drop through to manage events and return true;
		} // if lock failed just keep going
	// }
    // else {
		// return false;
    // }

	// Allow readers and writers access
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);

	return true;

} // end ReadTexturePixels 


// BIND THE SHARED TEXTURE
// for use in an application - this locks the interop object and binds the shared texture
// Afterwards a call to UnbindSharedTxeture MUST be called
bool spoutGLDXinterop::BindSharedTexture()
{
	if(m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	// Wait for writer to signal ready to read
	// if(senders.CheckAccess(m_hWriteEvent)) { // Read the shared texture
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
		}
		return true;
	// }
	// no access - return false
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	// return false;

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
	// senders.AllowAccess(m_hReadEvent, m_hWriteEvent);
	
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

	This lock triggers the GPU to perform the necessary flushing and stalling
	to guarantee that the surface has finished being written to before reading from it. 

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
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_BUSY\n");
				break;
			case ERROR_INVALID_DATA :	// One or more of the objects in <hObjects>
										// does not belong to the interop device
										// specified by <hDevice>.
				hr = E_ABORT;			// Operation aborted
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_INVALID_DATA\n");
				break;
			case ERROR_LOCK_FAILED :	// One or more of the objects in <hObjects> failed to 
				hr = E_ABORT;			// Operation aborted
				// printf("	spoutGLDXinterop::LockInteropObject ERROR_LOCK_FAILED\n");
				break;
			default:
				hr = E_FAIL;			// unspecified error
				// printf("	spoutGLDXinterop::LockInteropObject UNKNOWN_ERROR\n");
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


void spoutGLDXinterop::SetDX11format(DXGI_FORMAT textureformat)
{
	DX11format = textureformat;
	// printf("spoutGLDXinterop::SetDX11format(%d)\n", DX11format);
}


bool spoutGLDXinterop::GetVerticalSync()
{
	// needed for both sender and receiver
	if(bSWAPavailable) {
		if(wglGetSwapIntervalEXT()) {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}


bool spoutGLDXinterop::SetVerticalSync(bool bSync)
{
	if(bSWAPavailable) {

		if(bSync)
			wglSwapIntervalEXT(1); // lock to monitor vsync
		else
			wglSwapIntervalEXT(0); // unlock from monitor vsync

		return true;
	}

	return false;

}

