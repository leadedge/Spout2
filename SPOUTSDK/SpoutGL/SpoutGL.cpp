﻿//
//		SpoutGL
//
//		Base class for OpenGL texture sharing using the NVIDIA GL/DX intoerop extensions
//
//		See also - spoutDirectX, spoutSenderNames
//
// ====================================================================================
//		Revisions :
//
//		07.10.20	- Started class based on previous work with SpoutGLDXinterop.cpp
//					  for 2.006 and 2.007 beta : 15-07-14 - 03-09-20
//					  with reference to the SpoutDX class for consolidation of global variables.
//					  Compatibility with NVIDIA GL/DX interop is tested with fall-back to CPU share
//					  using DirectX11 staging textures for failure.
//					  MemoryShare is supported for receive only.
//					  DX9 support is removed.
//		09.12.20	- Correct ReadDX11texture for staging texture pitch
//		27.12.20	- Functions allocated to SpoutSDK class where appropriate
//
// ====================================================================================
/*
	Copyright (c) 2021, Lynn Jarvis. All rights reserved.

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

#include "SpoutGL.h"

spoutGL::spoutGL()
{
	m_SenderName[0] = 0;
	m_SenderNameSetup[0] = 0;
	m_Width = 0;
	m_Height = 0;
	m_dwFormat = (DWORD)DXGI_FORMAT_B8G8R8A8_UNORM; // default sender format

	m_bAuto = true;
	m_bUseGLDX = true;
	m_bSenderCPUmode = false;
	
	m_bConnected = false;
	m_bNewFrame = false;
	m_bInitialized = false;
	m_bSpoutPanelOpened = false;;
	m_bSpoutPanelActive = false;;
	m_bUpdated = false;
	m_bMirror = false;
	m_bSwapRB = false;

	m_glTexture = 0;
	m_TexID = 0;
	m_TexWidth = 0;
	m_TexHeight = 0;
	m_TexFormat = GL_RGBA;
	m_fbo = 0;

	m_dxShareHandle = nullptr; // Shared texture handle
	m_pSharedTexture = nullptr; // DX11 shared texture
	m_DX11format = DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9
	m_pStaging[0] = nullptr; // DX11 staging textures
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;

	m_hInteropDevice = nullptr;
	m_hInteropObject = nullptr;
	m_hWnd = nullptr;

	// For CreateOpenGL and CloseOpenGL
	m_hdc = nullptr;
	m_hwndButton = nullptr;
	m_hRc = nullptr;

	// OpenGL extensions
	m_caps = 0; // nothing loaded yet
	m_bExtensionsLoaded = false;
	m_bBGRAavailable = false;
	m_bFBOavailable  = false;
	m_bBLITavailable = false;
	m_bSWAPavailable = false;
	m_bGLDXavailable = false;
	m_bCOPYavailable = false;
	m_bPBOavailable  = true; // Assume true until tested by LoadGLextensions
	m_bCONTEXTavailable = false;

	// PBO support
	PboIndex = 0;
	NextPboIndex = 0;
	m_nBuffers = 2; // number of buffers used
	m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;

	// Memoryshare variables
	memoryshare.senderMem = nullptr;
	memoryshare.m_Width = 0;
	memoryshare.m_Height = 0;

	// Check the user selected Auto share mode
	DWORD dwValue = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Auto", &dwValue))
		m_bAuto = (dwValue == 1);

	// Check the user selected buffering mode
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffering", &dwValue))
		m_bPBOavailable = (dwValue == 1);

	// Number of PBO buffers user selected
	m_nBuffers = 2;
	if(ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffers", &dwValue))
		m_nBuffers = (int)dwValue;

	// Find version number from the registry if Spout is installed (2005, 2006, etc.)
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Version", &dwValue))
		m_SpoutVersion = (int)dwValue; // 0 for earlier than 2.005
	else
		m_SpoutVersion = -1; // Spout not installed
	   
}

spoutGL::~spoutGL()
{
	if (m_bInitialized) {
		sendernames.ReleaseSenderName(m_SenderName);
		frame.CleanupFrameCount();
		frame.CloseAccessMutex();
	}

	if (m_fbo > 0) {
		// Delete the fbo before the texture
		// so that any texture attachment is released
		glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;
	}

	if (m_glTexture > 0)
		glDeleteTextures(1, &m_glTexture);

	if (m_TexID > 0)
		glDeleteTextures(1, &m_TexID);

	if (m_pbo[0] > 0) {
		glDeleteBuffersEXT(m_nBuffers, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
	}

	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	CleanupInterop();
	CloseDirectX();

	if (memoryshare.senderMem) 
		delete memoryshare.senderMem;
	memoryshare.senderMem = nullptr;
	memoryshare.m_Width = 0;
	memoryshare.m_Height = 0;
	
}

//
// OpenGL shared texture access
//

//---------------------------------------------------------
bool spoutGL::BindSharedTexture()
{
	// Only for GL/DX interop mode
	if (!m_hInteropDevice || !m_hInteropObject)
		return false;

	bool bRet = false;
	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// lock dx object
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			// Leave interop and mutex both locked for success
			bRet = true;
		}
		else {
			// Release interop lock and allow texture access for fail
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			bRet = false;
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;

} // end BindSharedTexture

//---------------------------------------------------------
bool spoutGL::UnBindSharedTexture()
{
	// Only for GL/DX interop mode
	if (!m_hInteropDevice || !m_hInteropObject)
		return false;

	// Unbind our shared OpenGL texture
	glBindTexture(GL_TEXTURE_2D, 0);
	// unlock dx object
	UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
	// Release mutex and allow access to the texture
	frame.AllowTextureAccess(m_pSharedTexture);

	return true;

} // end UnBindSharedTexture

//---------------------------------------------------------
GLuint spoutGL::GetSharedTextureID()
{
	return m_glTexture;
}

//
// Graphics compatibility
//

//---------------------------------------------------------
bool spoutGL::GetAutoShare()
{
	return m_bAuto;
}

//---------------------------------------------------------
void spoutGL::SetAutoShare(bool bAuto)
{
	m_bAuto = bAuto;
}

//---------------------------------------------------------
bool spoutGL::IsGLDXready()
{
	return m_bUseGLDX;
}

//
// For direct access if necessary
//

//---------------------------------------------------------
bool spoutGL::OpenSpout(bool bRetest)
{
	// Return if already initialized and not re-testing compatibility
	if (m_hWnd > 0 && !bRetest)
		return true;

	printf("\n"); // This is the start, so make a new line in the log
#ifdef _M_X64
	SpoutLogNotice("spoutGL::OpenSpout - 64bit 2.007 - this 0x%.7X", PtrToUint(this));
#else
	SpoutLogNotice("spoutGL::OpenSpout - 32bit 2.007 - this 0x%.7X", PtrToUint(this));
#endif

	m_bUseGLDX = false;

	if (!OpenDirectX()) {
		SpoutLogFatal("spoutGL::OpenSpout - Could not initialize DirectX 11");
		return false;
	}

	// DirectX is OK
	// OpenGL device context is needed to go on
	HDC hdc = wglGetCurrentDC();
	if (!hdc) {
		SpoutLogFatal("spoutGL::OpenSpout - Cannot get GL device context");
		return false;
	}

	// Get a window handle
	m_hWnd = WindowFromDC(hdc);

	// Load extensions
	if (!LoadGLextensions()) {
		SpoutLogFatal("spoutGL::OpenSpout - Could not load GL extensions");
		return false;
	}

	// If DirectX and OpenGL are both OK - test GLDX compatibility

	// For a re-test, create a new interop device in GLDXReady()
	if (bRetest)
		CleanupInterop();

	if(!GLDXready()) {
		// Not GL/DX compatible.
		SpoutLogWarning("spoutGL::OpenSpout - system is not compatible with GL/DX interop");
		// Use CPU backup if Auto share enabled
		if (m_bAuto)
			SpoutLogWarning("   Using CPU share mode");
		else
			SpoutLogWarning("   Cannot share textures");
	}
	else {
		// GL/DX compatible. Use GL/DX interop.
		SpoutLogNotice("    Using texture share mode");
	}

	return true;

}

//---------------------------------------------------------
bool spoutGL::OpenDirectX()
{
	SpoutLogNotice("spoutGL::OpenDirectX");
	return spoutdx.OpenDirectX11();
}

//---------------------------------------------------------
void spoutGL::SetDX11format(DXGI_FORMAT textureformat)
{
	m_DX11format = textureformat;
}

//---------------------------------------------------------
void spoutGL::CloseDirectX()
{
	SpoutLogNotice("spoutGL::CloseDirectX()");

	if (m_pSharedTexture)
		m_pSharedTexture->Release();
	m_pSharedTexture = nullptr;
	spoutdx.CloseDirectX11();

	// Re-set shared texture handle
	m_dxShareHandle = nullptr;

}

//---------------------------------------------------------
bool spoutGL::CreateOpenGL()
{
	m_hdc = nullptr;
	m_hwndButton = nullptr;
	m_hRc = nullptr;

	SpoutLogNotice("spoutGL::CreateOpenGL()");

	HGLRC glContext = wglGetCurrentContext();

	if (!glContext) {

		// We only need an OpenGL context with no render window because we don't draw to it
		// so create an invisible dummy button window. This is then independent from the host
		// program window (GetForegroundWindow). If SetPixelFormat has been called on the
		// host window it cannot be called again. This caused a problem in Mapio.
		// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
		//
		// CS_OWNDC allocates a unique device context for each window in the class. 
		//
		if (!m_hwndButton || !IsWindow(m_hwndButton)) {
			m_hwndButton = CreateWindowA("BUTTON",
				"SpoutOpenGL",
				WS_OVERLAPPEDWINDOW | CS_OWNDC,
				0, 0, 32, 32,
				NULL, NULL, NULL, NULL);
		}

		if (!m_hwndButton) {
			SpoutLogError("spoutGL::CreateOpenGL - no hwnd");
			return false;
		}

		m_hdc = GetDC(m_hwndButton);
		if (!m_hdc) {
			SpoutLogError("spoutGL::CreateOpenGL - no hdc");
			CloseOpenGL();
			return false;
		}

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int iFormat = ChoosePixelFormat(m_hdc, &pfd);
		if (!iFormat) {
			SpoutLogError("spoutGL::CreateOpenGL - pixel format error");
			CloseOpenGL();
			return false;
		}

		if (!SetPixelFormat(m_hdc, iFormat, &pfd)) {
			DWORD dwError = GetLastError();
			// 2000 (0x7D0) The pixel format is invalid.
			// Caused by repeated call of the SetPixelFormat function
			char temp[128];
			sprintf_s(temp, "spoutGL::CreateOpenGL - SetPixelFormat Error %lu (0x%4.4lX)", dwError, dwError);
			SpoutLogError("%s", temp);
			CloseOpenGL();
			return false;
		}

		m_hRc = wglCreateContext(m_hdc);
		if (!m_hRc) {
			SpoutLogError("spoutGL::CreateOpenGL - could not create OpenGL context");
			CloseOpenGL();
			return false;
		}

		wglMakeCurrent(m_hdc, m_hRc);
		if (!wglGetCurrentContext()) {
			SpoutLogError("spoutGL::CreateOpenGL - no OpenGL context");
			CloseOpenGL();
			return false;
		}
		SpoutLogNotice("    OpenGL window created OK");
	}
	else {
		SpoutLogNotice("    OpenGL context exists");
	}

	return true;
}

//---------------------------------------------------------
bool spoutGL::CloseOpenGL()
{

	SpoutLogNotice("spoutGL::CloseOpenGL()");

	// Properly kill the OpenGL window
	if (m_hRc) {
		if (!wglMakeCurrent(NULL, NULL)) { // Are We Able To Release The DC And RC Contexts?
			SpoutLogError("spoutGL::CloseOpenGL - release of DC and RC failed");
			return false;
		}
		if (!wglDeleteContext(m_hRc)) { // Are We Able To Delete The RC?
			SpoutLogError("spoutGL::CloseOpenGL - release rendering context failed");
			return false;
		}
		m_hRc = NULL;
	}

	if (m_hdc && !ReleaseDC(m_hwndButton, m_hdc)) { // Are We Able To Release The DC
		SpoutLogError("spoutGL::CloseOpenGL - release device context Failed");
		m_hdc = NULL;
		return false;
	}

	if (m_hwndButton && !DestroyWindow(m_hwndButton)) { // Are We Able To Destroy The Window?
		SpoutLogError("spoutGL::CloseOpenGL - could not release hWnd");
		m_hwndButton = NULL;
		return false;
	}

	SpoutLogNotice("    closed the OpenGL window OK");

	return true;
}

//---------------------------------------------------------
bool spoutGL::IsSpoutInitialized()
{
	return m_bInitialized;
}

//
// GLDXready
//
// Hardware compatibility test
//
//  o Check that extensions for GL/DX interop are available
//
//  o GLDXready
//      Checks operation of GL/DX interop functions
//		and creates an interop device for success
//
//	o m_bUseGLDX - true for GL/DX interop availability
//
bool spoutGL::GLDXready()
{
	// === Simulate failure for debugging ===
	// SpoutLogNotice("spoutGL::GLDXready - simulated compatibility failure");
	// m_bUseGLDX = false;
	// return false;

	// Test whether the NVIDIA OpenGL/DirectX interop extensions function correctly. 
	// Creates dummy textures and uses the interop functions.
	// Creates an interop device on success.
	// Must be called after OpenDirectX.
	// Success means the GLDX interop functions can be used.
	// Other errors should not happen if OpenDirectX succeeded

	HANDLE dxShareHandle = nullptr; // Shared texture handle for a sender texture
	ID3D11Texture2D* pSharedTexture = nullptr; // the shared DX11 texture
	HANDLE hInteropObject = nullptr; // handle to the DX/GL interop object
	GLuint glTexture = 0; // the OpenGL texture linked to the shared DX texture

	SpoutLogNotice("spoutGL::GLDXready - testing for GL/DX interop compatibility");

	// Assume not GLDX interop compatible until all tests pass
	m_bUseGLDX = false;

	if (!spoutdx.GetDX11Device()) {
		SpoutLogError("spoutGL::GLDXready - No D3D11 device");
		return false;
	}

	// DirectX is OK but check for availabilty of the GL/DX extensions.
	if (!m_bGLDXavailable) {
		// The extensions required for texture access are not available.
		SpoutLogError("spoutGL::GLDXready - GL/DX interop extensions not available");
		return false;
	}

	SpoutLogNotice("    GL/DX interop extensions available");

	// Create an opengl texture for the test
	glGenTextures(1, &glTexture);
	if (glTexture == 0) {
		SpoutLogError("spoutGL::GLDXready - glGenTextures failed");
		return false;
	}
	glDeleteTextures(1, &glTexture);

	//
	// Create a directX texture and link using the NVIDIA GLDX interop functions
	//

	SpoutLogNotice("spoutGL::GLDXready - testing GL/DX interop functions");

	// Create a new shared DirectX resource
	if (!spoutdx.CreateSharedDX11Texture(spoutdx.GetDX11Device(),
		256, 256, DXGI_FORMAT_B8G8R8A8_UNORM, // default
		&pSharedTexture, dxShareHandle)) {
		glDeleteTextures(1, &glTexture);
		SpoutLogError("spoutGL::GLDXready - CreateSharedDX11Texture failed");
		return false;
	}

	SpoutLogNotice("    Linking test - OpenGL texture (0x%.7X) DX11 texture (0x%.7X)", glTexture, PtrToUint(pSharedTexture));

	// Link the shared DirectX texture to the OpenGL texture
	// If sucessful, LinkGLDXtextures initializes a class handle
	// to a GL/DirectX interop device - m_hInteropDevice
	hInteropObject = LinkGLDXtextures(spoutdx.GetDX11Device(), pSharedTexture, dxShareHandle, glTexture);
	if (!hInteropObject) {
		spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), pSharedTexture);
		glDeleteTextures(1, &glTexture);
		// It is possible that extensions for the GL/DX interop load OK
		// but that the GL/DX interop functions fail.
		// This has been noted on dual graphics machines with the NVIDIA Optimus driver.
		SpoutLogWarning("spoutGL::GLDXready - GL/DX interop functions failed");
	}
	else {
		// All passes, so unregister and release textures
		// m_hInteropDevice remains and does not need to be created again
		if (m_hInteropDevice)
			wglDXUnregisterObjectNV(m_hInteropDevice, hInteropObject);

		spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), pSharedTexture);
		glDeleteTextures(1, &glTexture);

		// Set compatibility flag
		m_bUseGLDX = true;

		SpoutLogNotice("    Test OpenGL and DX11 textures created and linked OK");

	}

	// Now GLDXready() has set m_bUseGLDX is set to use the GL/DX interop or not.
	
	// Use of texture sharing or CPU backup is assessed from
	// user settings (retrieved with GetAutoShare)
	// and actual GL/DX compatibility (retrieved with GetGLDXready)

	// Texture sharing is used if GL/DX compatible (m_bUseGLDX = true)
	// CPU backup is used if :
	//   1) Graphics is incompatible (m_bUseGLDX = false)
	//   2) The user has selected "Auto" share in SpoutSettings (m_bAuto = false)
	// Otherwise no sharing is performed.

	// If not GLDX compatible, LinkGLDXtexture will not be called (see CreateDX11interop)
	// ReadDX11Texture and WriteDX11Texture using staging textures will be used instead

	return m_bUseGLDX;

}


//
// Protected functions
//

// Create shared DirectX texture and OpenGL texture and link with GL/DX interop
bool spoutGL::CreateInterop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	SpoutLogNotice("spoutGL::CreateInterop");

	// printf("spoutGL::CreateInterop %dx%d - format = %d\n", width, height, dwFormat);

	// Create or use a shared DirectX texture that will be linked
	// to the OpenGL texture and get it's share handle for sharing textures
	if (bReceive) {
		// A receiver uses a texture already created from the sender share handle
		if (!m_pSharedTexture || !m_dxShareHandle) {
			SpoutLogError("spoutGL::CreateInterop - no receiver texture : device = 0x%.7X, sharehandle = 0x%.7X", PtrToUint(spoutdx.GetDX11Device()), LOWORD(m_dxShareHandle));
			return false;
		}
	}
	else {

		// A sender creates a new texture with a new share handle
		m_dxShareHandle = nullptr;

		//
		// Texture format tests
		//
		// DX9 compatible formats
		// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
		// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
		//
		// Other formats that work with DX11 but not with DX9
		// DXGI_FORMAT_R16G16B16A16_FLOAT
		// DXGI_FORMAT_R16G16B16A16_SNORM
		// DXGI_FORMAT_R10G10B10A2_UNORM
		//
		// A directX 11 receiver accepts DX9 formats
		DWORD format = (DWORD)DXGI_FORMAT_B8G8R8A8_UNORM; // (87) default compatible with DX9
		if (dwFormat > 0) {
			format = dwFormat;
			SetDX11format((DXGI_FORMAT)format); // Set the global texture format
		}
		else {
			format = m_DX11format;
		}

		// Create or re-create the linked DX11 texture
		if (!spoutdx.CreateSharedDX11Texture(spoutdx.GetDX11Device(),
			width, height, (DXGI_FORMAT)format, // default is DXGI_FORMAT_B8G8R8A8_UNORM
			&m_pSharedTexture, m_dxShareHandle)) {
			SpoutLogError("spoutGL::CreateInterop - CreateSharedDX11Texture failed");
			return false;
		}
	}

	SpoutLogNotice("spoutGL::CreateInterop - m_pSharedTexture = 0x%.7X : m_dxShareHandle = 0x%.7X", PtrToUint(m_pSharedTexture), LOWORD(m_dxShareHandle));

	// Link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	// When a sender size changes, the new texture has to be re-registered
	if (m_hInteropDevice &&  m_hInteropObject) {
		SpoutLogNotice("    Unregistering interop");
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = nullptr;
	}

	// Create or re-create the class OpenGL texture
	InitTexture(m_glTexture, GL_RGBA, width, height);
	m_Width = width;
	m_Height = height;

	// Link the texture using the GL/DX interop
	m_hInteropObject = LinkGLDXtextures((void *)spoutdx.GetDX11Device(), m_pSharedTexture, m_dxShareHandle, m_glTexture);
	if (!m_hInteropObject) {
		SpoutLogError("spoutGL::CreateInterop - LinkGLDXtextures failed");
		return false;
	}
	SpoutLogNotice("    m_hInteropObject = 0x%.7X", LOWORD(m_hInteropObject));

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

	// Important to reset PBO index
	PboIndex = 0;
	NextPboIndex = 0;

	// Also reset staging texture index
	m_Index = 0;
	m_NextIndex = 0;

	return true;

}

//
//	Link a shared DirectX texture to an OpenGL texture
//	and create a GLDX interop object handle
//
//	IN	pSharedTexture  Pointer to shared the DirectX texture
//	IN	dxShareHandle   Handle of the DirectX texture to be shared
//	IN	glTextureID     ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returns             Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGL::LinkGLDXtextures(void* pDXdevice,
	void* pSharedTexture, HANDLE dxShareHandle, GLuint glTexture)
{

	HANDLE hInteropObject = nullptr;
	BOOL bResult = 0;
	DWORD dwError = 0;
	char tmp[128];

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if (!m_hInteropDevice) {
		try {
			m_hInteropDevice = wglDXOpenDeviceNV(pDXdevice);
		}
		catch (...) {
			SpoutLogError("spoutGL::LinkGLDXtextures - wglDXOpenDeviceNV exception");
			return NULL;
		}
	}

	if (!m_hInteropDevice) {
		// wglDXOpenDeviceNV failed to open the Direct3D device
		dwError = GetLastError();
		sprintf_s(tmp, 128, "spoutGL::LinkGLDXtextures : wglDXOpenDeviceNV(0x%.7X) - error %lu (0x%.X)\n",
			LOWORD(pDXdevice), dwError, LOWORD(dwError));
		// Other errors reported
		// 1008, 0x3F0 - ERROR_NO_TOKEN
		switch (LOWORD(dwError)) {
		case ERROR_OPEN_FAILED:
			strcat_s(tmp, 128, "    Could not open the Direct3D device.");
			break;
		case ERROR_NOT_SUPPORTED:
			// This can be caused either by passing in a device from an unsupported DirectX
			// version, or by passing in a device referencing a display adapter that is
			// not accessible to the GL.
			strcat_s(tmp, 128, "    The dxDevice is not supported.");
			break;
		default:
			strcat_s(tmp, 128, "    Unknown error.");
			break;
		}
		SpoutLogError("%s", tmp);
		return NULL;
	}

	// prepare shared resource
	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	// Error when dxShareHandle is NULL
	try {
		bResult = wglDXSetResourceShareHandleNV(pSharedTexture, dxShareHandle);
	}
	catch (...) {
		SpoutLogError("spoutGL::LinkGLDXtextures - wglDXSetResourceShareHandleNV exception");
		return NULL;
	}

	if (!bResult) {
		SpoutLogError("spoutGL::LinkGLDXtextures - wglDXSetResourceShareHandleNV error");
		return NULL;
	}

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	// Returns a handle that can be used for sharing functions
	try {
		hInteropObject = wglDXRegisterObjectNV(m_hInteropDevice,
			pSharedTexture,	// DX texture
			glTexture,		// OpenGL texture
			GL_TEXTURE_2D,	// Must be TEXTURE_2D - multisampling not supported
			WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read
	}
	catch (...) {
		SpoutLogError("spoutGL::LinkGLDXtextures - wglDXRegisterObjectNV exception");
		return NULL;
	}

	if (!hInteropObject) {
		// Noted C007006E returned on failure.
		// Error codes are 32-bit values, but expected results are in the low word.
		// 006E is ERROR_OPEN_FAILED (110L)
		dwError = GetLastError();
		sprintf_s(tmp, 128, "spoutGL::LinkGLDXtextures - wglDXRegisterObjectNV :error %u, (0x%.X)\n",
			LOWORD(dwError), LOWORD(dwError));
		switch (LOWORD(dwError)) {
		case ERROR_INVALID_HANDLE:
			strcat_s(tmp, 128, "    No GL context is current.");
			break;
		case ERROR_INVALID_DATA:
			strcat_s(tmp, 128, "    Incorrect GL name, type or access parameters.");
			break;
		case ERROR_OPEN_FAILED:
			strcat_s(tmp, 128, "    Failed to open the Direct3D resource.");
			break;
		default:
			strcat_s(tmp, 128, "    Unknown error.");
			break;
		}
		SpoutLogError("%s", tmp);

		if (m_hInteropDevice) {
			wglDXCloseDeviceNV(m_hInteropDevice);
			m_hInteropDevice = nullptr;
		}

	}

	return hInteropObject;

}

HANDLE spoutGL::GetInteropDevice()
{
	return m_hInteropDevice; // Handle to the GL/DX interop device
}

//
//	GL/DX Interop lock
//
//	A return value of S_OK indicates that all objects were
//    successfully locked.  Other return values indicate an
//    error. If the function returns false, none of the objects will be locked.
//
//	Attempting to access an interop object via GL when the object is
//    not locked, or attempting to access the DirectX resource through
//    the DirectX API when it is locked by GL, will result in undefined
//    behavior and may result in data corruption or program
//    termination. Likewise, passing invalid interop device or object
//    handles to this function has undefined results, including program
//    termination.
//
//	Note that only one GL context may hold the lock on the
//    resource at any given time --- concurrent access from multiple GL
//    contexts is not currently supported.
//
//	DISCUSSION: The Lock/Unlock calls serve as synchronization points
//    between OpenGL and DirectX. They ensure that any rendering
//    operations that affect the resource on one driver are complete
//    before the other driver takes ownership of it.
//
//	This function assumes only one object
//
//	Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGL::LockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if (!hDevice || !hObject || !*hObject) {
		return E_HANDLE;
	}

	// lock dx object
	if (wglDXLockObjectsNV(hDevice, 1, hObject)) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (LOWORD(dwError)) {
		case ERROR_BUSY:			// One or more of the objects in <hObjects> was already locked.
			hr = E_ACCESSDENIED;	// General access denied error
			SpoutLogError("spoutGL::LockInteropObject - ERROR_BUSY");
			break;
		case ERROR_INVALID_DATA:	// One or more of the objects in <hObjects>
									// does not belong to the interop device
									// specified by <hDevice>.
			hr = E_ABORT;			// Operation aborted
			SpoutLogError("spoutGL::LockInteropObject - ERROR_INVALID_DATA");
			break;
		case ERROR_LOCK_FAILED:	// One or more of the objects in <hObjects> failed to 
			hr = E_ABORT;			// Operation aborted
			SpoutLogError("spoutGL::LockInteropObject - ERROR_LOCK_FAILED");
			break;
		default:
			hr = E_FAIL;			// unspecified error
			SpoutLogError("spoutGL::LockInteropObject - UNKNOWN_ERROR");
			break;
		} // end switch
	} // end false

	return hr;

} // LockInteropObject


//
// Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGL::UnlockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if (!hDevice || !hObject || !*hObject) {
		return E_HANDLE;
	}

	if (wglDXUnlockObjectsNV(hDevice, 1, hObject)) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (LOWORD(dwError)) {
		case ERROR_NOT_LOCKED:
			hr = E_ACCESSDENIED;
			SpoutLogError("spoutGL::UnLockInteropObject - ERROR_NOT_LOCKED");
			break;
		case ERROR_INVALID_DATA:
			SpoutLogError("spoutGL::UnLockInteropObject - ERROR_INVALID_DATA");
			hr = E_ABORT;
			break;
		case ERROR_LOCK_FAILED:
			hr = E_ABORT;
			SpoutLogError("spoutGL::UnLockInteropObject - ERROR_LOCK_FAILED");
			break;
		default:
			hr = E_FAIL;
			SpoutLogError("spoutGL::UnLockInteropObject - UNKNOWN_ERROR");
			break;
		} // end switch
	} // end fail

	return hr;

} // end UnlockInteropObject


// Clean up the gldx interop
void spoutGL::CleanupInterop()
{
	// Release OpenGL objects etc. even if DirectX has been released
	if (!m_hInteropDevice && !m_hInteropObject)
		return;

	// These things need an opengl context so check
	if (wglGetCurrentContext()) {
		SpoutLogNotice("spoutGL::CleanupInterop");
		if (m_hInteropDevice && m_hInteropObject) {
			if (!wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject)) {
				SpoutLogWarning("spoutGL::CleanupInterop - could not un-register interop");
			}
			m_hInteropObject = nullptr;
		}

		if (m_hInteropDevice) {
			if (!wglDXCloseDeviceNV(m_hInteropDevice)) {
				SpoutLogWarning("spoutGL::CleanupInterop - could not close interop");
			}
			m_hInteropDevice = nullptr;
		}
	}
}

//---------------------------------------------------------
void spoutGL::CleanupGL()
{
	if (m_fbo > 0) {
		// Delete the fbo before the texture so that any texture attachment 
		// is released even though it should have been
		glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;
	}

	if (m_glTexture > 0)
		glDeleteTextures(1, &m_glTexture);

	if (m_TexID > 0)
		glDeleteTextures(1, &m_TexID);

	if (m_pbo[0] > 0) {
		glDeleteBuffersEXT(m_nBuffers, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
	}

	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	m_pStaging[0] = nullptr;
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;
	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	m_bInitialized = false;

	CleanupInterop();

	// do not close DirectX

}

// If a class OpenGL texture has not been created or it is a different size, create a new one
// Typically used for texture copy and invert
void spoutGL::CheckOpenGLTexture(GLuint &texID, GLenum GLformat, unsigned int width,  unsigned int height)
{
	if (texID == 0 || texID != m_TexID || GLformat != m_TexFormat || width != m_TexWidth || height != m_TexHeight) {
		InitTexture(texID, GLformat, width, height);
		m_TexID = texID;
		m_TexWidth  = width;
		m_TexHeight = height;
		m_TexFormat = (DWORD)GLformat;
	}
}

// Initialize OpenGL texture
void spoutGL::InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height)
{
	if (texID != 0) glDeleteTextures(1, &texID);
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GLformat, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

}


//
// COPY AN OPENGL TEXTURE TO THE SHARED OPENGL TEXTURE
//
// Allows for a texture attached to the host fbo
// Where the input texture can be larger than the shared texture
// and Width and height are the used portion. Only the used part is copied.
// For example Freeframe textures.
//
bool spoutGL::WriteGLDXtexture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// Only for GL/DX interop mode
	if (!m_hInteropDevice || !m_hInteropObject)
		return false;

	// Specify greater here because the width/height passed can be smaller
	if (width > m_Width || height > m_Height)
		return false;

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// lock dx interop object
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Write to the shared texture
			if (SetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO)) {
				// Increment the sender frame counter for successful write
				frame.SetNewFrame();
			}
			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;

} // end WriteGLDXTexture


bool spoutGL::ReadGLDXtexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// No interop, no copy
	if (!m_hInteropDevice || !m_hInteropObject) {
		return false;
	}

	// width and height must be the same as the shared texture
	// m_TextureInfo is established in CreateDX11interop
	if (width != m_Width || height != m_Height) {
		return false;
	}

	bool bRet = true; // Error only if texture read fails

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {

		// Read the shared texture if the sender has produced a new frame
		// GetNewFrame updates sender frame count and fps
		if (frame.GetNewFrame()) {
			// No texture read for zero texture - allowed for by ReceiveTexture
			if (TextureID > 0 && TextureTarget > 0) {
				if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
					bRet = GetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO);
					UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
				}
			}
		}

		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;

} // end ReadGLDXTexture


bool spoutGL::SetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	GLenum status = 0;
	bool bRet = false;

	// "TextureID" can be NULL if it is attached to the host fbo
	// m_fbo is a local FBO
	// "m_glTexture" is destination texture
	// width/height are the dimensions of the destination texture
	// Because two fbos are used, the input texture can be larger than the shared texture
	// Width and height are the used portion and only the used part is copied

	if (TextureID == 0 && HostFBO > 0 && glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {

		// Enter with the input texture attached to
		// attachment point 0 of the currently bound fbo
		// and set for read or read/write

		// Bind our local fbo for draw
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
		// Draw to the first attachment point
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		// Attach the texture we write into (the shared texture)
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
		// Check fbo for completeness
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
			if (m_bBLITavailable) {
				if (bInvert)
					// copy one texture buffer to the other while flipping upside down 
					glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				else
					// Do not flip during blit
					glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
			else {
				// No fbo blit extension
				// Copy from the host fbo (input texture attached)
				// to the shared texture
				glBindTexture(GL_TEXTURE_2D, m_glTexture);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			bRet = true;
		}
		else {
			PrintFBOstatus(status);
			bRet = false;
		}
		// restore the host fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
	}
	else if (TextureID > 0) {
		// There is a valid texture passed in.
		// Copy the input texture to the destination texture.
		// Both textures must be the same size.
		bRet = CopyTexture(TextureID, TextureTarget, m_glTexture, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
	}

	return bRet;

}

// Copy shared texture via fbo blit
bool spoutGL::GetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	GLenum status = 0;
	bool bRet = false;

	// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	// Attach the Input texture (the shared texture) to the color buffer in our frame buffer - note texturetarget 
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Attach target texture (the one we write into and return) to second attachment point
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, TextureTarget, TextureID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
		if (m_bBLITavailable) {
			// Flip if the user wants that
			if (bInvert) {
				// copy one texture buffer to the other while flipping upside down
				glBlitFramebufferEXT(0, 0,		// srcX0, srcY0, 
					width, height, // srcX1, srcY1
					0, height,	// dstX0, dstY0,
					width, 0,		// dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
			else {
				// Do not flip during blit
				glBlitFramebufferEXT(0, 0,			// srcX0, srcY0, 
					width, height,	// srcX1, srcY1
					0, 0,			// dstX0, dstY0,
					width, height,	// dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}
		else {
			// No fbo blit extension available
			// Copy from the fbo (shared texture attached) to the dest texture
			glBindTexture(TextureTarget, TextureID);
			glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
			glBindTexture(TextureTarget, 0);
		}
		bRet = true;
	}
	else {
		PrintFBOstatus(status);
		bRet = false;
	}

	// restore the previous fbo - default is 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); // 04.01.16
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return bRet;

}


//
// COPY IMAGE PIXELS TO THE OPENGL SHARED TEXTURE
//
bool spoutGL::WriteGLDXpixels(const unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	if (width != m_Width || height != m_Height || !pixels)
		return false;

	// Use a GL texture so that WriteTexture can be used
	GLenum glformat = glFormat;

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, glFormat, width, height);

	// Transfer the pixels to the local texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // In case of RGB pixel data
	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glformat, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// Write the local texture to the shared texture and invert if necessary
	WriteGLDXtexture(m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);

	return true;

} // end WriteGLDXpixels


//
// COPY OPENGL SHARED TEXTURE TO IMAGE PIXELS
//
bool spoutGL::ReadGLDXpixels(unsigned char* pixels,
	unsigned int width, unsigned int height,
	GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	if (!m_hInteropDevice || !m_hInteropObject)
		return false;

	if (width != m_Width || height != m_Height)
		return false;

	// GLenum glformat = glFormat;
	bool bRet = true; // Error only if pixel read fails

	// retrieve opengl texture data directly to image pixels

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// read texture for a new frame
		if (frame.GetNewFrame()) {
			// lock gl/dx interop object
			if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
				// Set single pixel alignment in case of rgb source
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				// Always allow for invert here - only consumes 0.1 msec
				// Create or resize a local OpenGL texture
				CheckOpenGLTexture(m_TexID, glFormat, width, height);
				// Copy the shared texture to the local texture, inverting if necessary
				CopyTexture(m_glTexture, GL_TEXTURE_2D, m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
				// Extract the pixels from the local texture - changing to the user passed format
				// Use PBO method for maximum speed, otherwise use DirectX staging texture method
				// ReadTextureData using glReadPixels is half the speed of using DX11 texture directly
				if (m_bPBOavailable)
					bRet = UnloadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, 0, pixels, glFormat, false, HostFBO);
				else
					bRet = ReadTextureData(m_TexID, GL_TEXTURE_2D, width, height, 0, pixels, glFormat, false, HostFBO);
				// default alignment
				glPixelStorei(GL_PACK_ALIGNMENT, 4);
			} // interop lock failed
			// Unlock interop object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		} // no new frame
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	} // mutex access failed

	return bRet;

} // end ReadGLDXpixels 


//
// Asynchronous Read-back from an OpenGL texture
//
// Adapted from : http://www.songho.ca/opengl/gl_pbo.html
//
bool spoutGL::UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, unsigned int rowpitch,
	unsigned char* data, GLenum glFormat,
	bool bInvert, GLuint HostFBO)
{
	void *pboMemory = nullptr;
	int channels = 4; // RGBA or RGB

	if (!data) {
		return false;
	}

	if (glFormat == GL_RGB || glFormat == GL_BGR_EXT) {
		channels = 3;
	}

	unsigned int pitch = rowpitch; // row pitch passed in
	if (rowpitch == 0)
		pitch = width * channels; // RGB or RGBA
	
	if (m_fbo == 0) {
		SpoutLogNotice("spoutGL::UnloadTexturePixels - creating FBO");
		glGenFramebuffersEXT(1, &m_fbo);
	}

	// Create pbos if not already
	if (m_pbo[0] == 0) {
		SpoutLogNotice("spoutGL::UnloadTexturePixels - creating PBO");
		glGenBuffersEXT(m_nBuffers, m_pbo);
		PboIndex = 0;
		NextPboIndex = 0;
	}

	PboIndex = (PboIndex + 1) % m_nBuffers;
	NextPboIndex = (PboIndex + 1) % m_nBuffers;
	
	// If Texture ID is zero, the texture is already attached to the Host Fbo
	// and we do nothing. If not we need to create an fbo and attach the user texture
	if (TextureID > 0) {
		// Attach the texture to point 0
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
		// Set the target framebuffer to read
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	}
	else if (HostFBO == 0) {
		// If no texture ID, a Host FBO must be provided
		// testing only - error log will repeat
		return false;
	}

	// Bind the PBO
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Check it's size
	GLint size = 0;
	glGetBufferParameterivEXT(GL_PIXEL_PACK_BUFFER, GL_BUFFER_SIZE_EXT, &size);
	if (size > 0 && size != (int)(pitch * height) ) {
		// All PBOs must be re-created
		glBindBufferEXT(GL_PIXEL_PACK_BUFFER, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		glDeleteBuffersEXT(m_nBuffers, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
		return false;
	}

	// Null existing PBO data to avoid a stall
	// This allocates memory for the PBO pitch*height wide
	glBufferDataEXT(GL_PIXEL_PACK_BUFFER, pitch*height, 0, GL_STREAM_READ);
	
	// Read pixels from framebuffer to PBO - glReadPixels() should return immediately.
	glPixelStorei(GL_PACK_ROW_LENGTH, pitch/channels); // row length in pixels
	glReadPixels(0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, (GLvoid *)0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	// If there is data in the next pbo from the previous call, read it back
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);

	// Map the PBO to process its data by CPU
	pboMemory = glMapBufferEXT(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	// glMapBuffer can return NULL when called the first time
	// when the next pbo has not been filled with data yet
	glGetError(); // remove the last error

	if (pboMemory && data) {
		// Update data directly from the mapped buffer (TODO: RGB)
		spoutcopy.CopyPixels((const unsigned char*)pboMemory, (unsigned char*)data, pitch/channels, height, glFormat, bInvert);
		glUnmapBufferEXT(GL_PIXEL_PACK_BUFFER);
	}
	// skip the copy rather than return false.

	// Back to conventional pixel operation
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, 0);

	// Restore the previous fbo binding
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

}


//
// Copy OpenGL to DirectX 11 texture via CPU where the GL/DX interop is not available
//
// GPU read is from OpenGL.
// Use multiple PBOs instead of glReadPixels for best speed.
//
bool spoutGL::WriteDX11texture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	bool bRet = false;

	// Only for DX11 mode
	if (!spoutdx.GetDX11Context()) {
		return false;
	}

	// If a staging texture has not been created or a different size create a new one
	// Only one staging texture is required. Buffering read from GPU is done by OpenGL PBO.
	if (!CheckStagingTextures(width, height, 1))
		return false;
	
	// Map the DX11 staging texture and write the sender OpenGL texture pixels to it
	if (SUCCEEDED(spoutdx.GetDX11Context()->Map(m_pStaging[0], 0, D3D11_MAP_WRITE, 0, &mappedSubResource))) {
		
		// Staging texture width is multiples of 16 and pitch can be greater that width*4
		// Copy OpenGL texture pixelsto the staging texture taking account of the destination row pitch
		if (m_bPBOavailable) {
			bRet = UnloadTexturePixels(TextureID, TextureTarget,
				width, height,
				mappedSubResource.RowPitch,
				(unsigned char *)mappedSubResource.pData,
				GL_BGRA_EXT, bInvert, HostFBO);
		}
		else {
			bRet = ReadTextureData(TextureID, TextureTarget, // OpenGL source texture
				width, height, // width and height of OpenGL texture
				mappedSubResource.RowPitch, // bytes per line of staging texture
				(unsigned char *)mappedSubResource.pData, // staging texture pixels
				GL_BGRA_EXT, bInvert, HostFBO);
		}

		spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);

		// The staging texture is updated with the OpenGL texture data
		// Write it to the sender's shared texture
		WriteTexture(&m_pStaging[0]);

		return true;

	}

	return false;

} // end WriteDX11texture

//
// Copy from the shared DX11 texture to an OpenGL texture via CPU staging texture
// GPU write is to OpenGL
//
bool spoutGL::ReadDX11texture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;

	// Quit for zero texture
	if (TextureID == 0 || TextureTarget == 0) {
		return false;
	}

	// Only for DX11 mode
	if (!spoutdx.GetDX11Context())
		return false;

	// Only one staging texture is required because GPU write is to OpenGL
	if (!CheckStagingTextures(width, height, 1)) {
		return false;
	}

	// Read from from the sender shared texture to a staging texture
	if (!ReadTexture(&m_pStaging[0])) {
		return false;
	}

	// Update the application receiving OpenGL texture from the DX11 staging texture
	// Default format is BGRA. Change if the sender is RGBA.
	GLenum glFormat = GL_BGRA_EXT;
	if (m_dwFormat == 28) glFormat = GL_RGBA;

	// Make sure the GPU is ready to access the staging texture 
	spoutdx.GetDX11Context()->Flush();

	// Map the staging texture to access the sender pixels
	if (SUCCEEDED(spoutdx.GetDX11Context()->Map(m_pStaging[0], 0, D3D11_MAP_READ, 0, &mappedSubResource))) {

		// TODO : format testing for invert if m_TexID exists
		if (bInvert) {
			// Create or resize a local OpenGL texture
			CheckOpenGLTexture(m_TexID, glFormat, width, height);
			// Copy the DX11 pixels to it
			glBindTexture(GL_TEXTURE_2D, m_TexID);
		}
		else {
			// Copy the DX11 pixels to the user texture
			glBindTexture(TextureTarget, TextureID);
		}

		// Allow for the staging texture for row pitch
		glPixelStorei(GL_UNPACK_ROW_LENGTH, mappedSubResource.RowPitch / 4); // row length in pixels

		// Get the pixels from the staging texture
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height,
			glFormat, GL_UNSIGNED_BYTE, (const GLvoid *)mappedSubResource.pData);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		// Copy the local texture to the user texture and invert as necessary
		if(bInvert)	
			CopyTexture(m_TexID, GL_TEXTURE_2D, TextureID, TextureTarget, width, height, bInvert, HostFBO);

		spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);

		return true;
	}

	return false;

} // end ReadDX11texture


// Copy OpenGL texture data to a pixel buffer via fbo
bool spoutGL::ReadTextureData(GLuint SourceID, GLuint SourceTarget,
	unsigned int width, unsigned int height, unsigned int pitch,
	unsigned char* dest, GLenum GLformat, bool bInvert, GLuint HostFBO)
{
	GLenum status;

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, GL_RGBA, width, height);

	// Create a local fbo if not already
	if (m_fbo == 0)	glGenFramebuffersEXT(1, &m_fbo);

	// If texture ID is zero, assume the source texture is attached
	// to the host fbo which is bound for read and write
	if (SourceID == 0 && HostFBO > 0) {
		// Bind our local fbo for draw only
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
		// Source texture is already attached to point 0 for read
	}
	else {
		// bind the local fbo for read and write
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// Read from attachment point 0
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		// Attach the Source texture to point 0 for read
		glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
	}

	// Draw to attachment point 1
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	// Attach the texture we write into (the local texture) to attachment point 1
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_TexID, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {

		if (bInvert && m_bBLITavailable) {
			// copy the source texture (0) to the local texture (1) while flipping upside down 
			glBlitFramebufferEXT(0,	0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// Bind local fbo for read
			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo);
			// Read from attachment point 1
			glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
			
			// Read pixels from it
			glPixelStorei(GL_PACK_ROW_LENGTH, pitch / 4); // row length in pixels
			glReadPixels(0, 0, width, height, GLformat, GL_UNSIGNED_BYTE, (GLvoid *)dest);
			glPixelStorei(GL_PACK_ROW_LENGTH, 0);

		}
		else {
			// No invert or no fbo blit extension
			// Read from the source texture attachment point 0
			// This will be the local fbo if a texture ID was passed in

			// Pitch is destination line length in bytes. Divide by 4 to get the width in rgba pixels.
			glPixelStorei(GL_PACK_ROW_LENGTH, pitch/4); // row length in pixels
			glReadPixels(0, 0, width, height, GLformat, GL_UNSIGNED_BYTE, (GLvoid *)dest);
			glPixelStorei(GL_PACK_ROW_LENGTH, 0);

		}
	}
	else {
		PrintFBOstatus(status);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

} // end ReadTextureData

//
// COPY IMAGE PIXELS TO THE SHARED DX11 TEXTURE VIA STAGING TEXTURES
// RGBA/RGB/BGRA/BGR supported
//
// GPU write is to DX11
// Use staging texture to support RGBA/RGB
//
bool spoutGL::WriteDX11pixels(const unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (width != m_Width || height != m_Height || !pixels)
		return false;

	// if(!CheckStagingTextures(width, height, 1))
	if (!CheckStagingTextures(width, height, 2))
		return false;

	// 1) pixels (RGBA or RGB) -> staging texture (RGBA) - CPU
	// 2) staging texture -> DX11 sender texture (RGBA) CopyResource - GPU
	//
	// RGBA :
	//   6.2 msec @ 3840 x 2160
	//   1.6 msec @ 1920 x 1080
	// RGBA Using UpdateSubresource (pixels > texture) instead of pixels > staging > texture
	//   5.8 msec @ 3840 x 2160
	//   1.5 msec @ 1920 x 1080
	// RGB :
	//   10 msec @ 3840 x 2160
	//   2.7 msec @ 1920 x 1080
	//
	// Access the sender shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Map the staging texture and write pixels to it (CPU)
		WritePixelData(pixels, m_pStaging[0], width, height, glFormat, bInvert);
		// Copy from the staging texture to the sender shared texture (GPU)
		spoutdx.GetDX11Context()->CopyResource(m_pSharedTexture, m_pStaging[0]);
		spoutdx.GetDX11Context()->Flush();
		frame.SetNewFrame();
		frame.AllowTextureAccess(m_pSharedTexture);
		return true;
	}
	return false;

} // end WriteDX11pixels


// Receive from a sender via DX11 staging textures to an rgba or rgb buffer of variable size
// A new shared texture pointer (m_pSharedTexture) is retrieved if the sender changed
bool spoutGL::ReadDX11pixels(unsigned char * pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!CheckStagingTextures(width, height, 2))
		return false;

	// Access the sender shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Check if the sender has produced a new frame.
		if (frame.GetNewFrame()) {
			// Read from the sender GPU texture to CPU pixels via two staging textures
			m_Index = (m_Index + 1) % 2;
			m_NextIndex = (m_Index + 1) % 2;
			// Copy from the sender's shared texture to the first staging texture
			spoutdx.GetDX11Context()->CopyResource(m_pStaging[m_Index], m_pSharedTexture);
			// Map and read from the second while the first is occupied
			ReadPixelData(m_pStaging[m_NextIndex], pixels, m_Width, m_Height, glFormat, bInvert);
		}
		// Allow access to the shared texture
		frame.AllowTextureAccess(m_pSharedTexture);
		return true;
	}

	return false;

}


// RGBA/RGB/BGRA/BGR supported
bool spoutGL::WritePixelData(const unsigned char* pixels, ID3D11Texture2D* pStagingTexture,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!spoutdx.GetDX11Context() || !pStagingTexture || !pixels)
		return false;

	// glFormat = GL_RGB      0x1907
	// glFormat = GL_RGBA     0x1908
	// glFormat = GL_BGR_EXT  0x80E0
	// glFormat = GL_BGRA_EXT 0x80E1
	//
	// m_dwFormat = 28 RGBA staging textures
	// m_dwFormat = 87 BGRA staging textures

	// Map the resource so we can access the pixels
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	// Make sure all commands are done before mapping the staging texture
	spoutdx.GetDX11Context()->Flush();
	// Map waits for GPU access
	HRESULT hr = spoutdx.GetDX11Context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {
		//
		// Copy from the pixel buffer to the staging texture
		//
		// The shared texture format is BGRA or RGBA and the staging textures are the same format.
		// If the texture format is BGRA and the receiving pixel buffer is RGBA/RGB or vice-versa,
		// the data has to be converted from BGRA to RGBA/RGB or RGBA to BGRA/BGR during the pixel copy.
		//
		if (glFormat == GL_RGBA) { // RGBA pixel buffer
			if (m_dwFormat == 28) // RGBA staging textures
				spoutcopy.rgba2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgra((const void *)pixels, mappedSubResource.pData, 
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGRA_EXT) { // BGRA pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgra((const void *)pixels, mappedSubResource.pData,
					width, height, width * 4, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_RGB) { // RGB pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgb2rgba((const void *)pixels, mappedSubResource.pData, 
					width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgb2bgra((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGR_EXT) { // BGR pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.bgr2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgb2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
		}
		spoutdx.GetDX11Context()->Unmap(pStagingTexture, 0);

		return true;

	} // endif DX11 map OK

	return false;

} // end WritePixelData


//
// COPY FROM A DX11 STAGING TEXTURE TO A USER RGBA/RGB/BGR PIXEL BUFFER
//
bool spoutGL::ReadPixelData(ID3D11Texture2D* pStagingTexture, unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!spoutdx.GetDX11Context() || !pStagingTexture || !pixels)
		return false;

	// glFormat = GL_RGB      0x1907
	// glFormat = GL_RGBA     0x1908
	// glFormat = GL_BGR_EXT  0x80E0
	// glFormat = GL_BGRA_EXT 0x80E1
	//
	// m_dwFormat = 28 RGBA staging textures
	// m_dwFormat = 87 BGRA staging textures

	// Map the resource so we can access the pixels
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	// Make sure all commands are done before mapping the staging texture
	spoutdx.GetDX11Context()->Flush();
	// Map waits for GPU access
	HRESULT hr = spoutdx.GetDX11Context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {
		//
		// Copy from staging texture to the pixel buffer
		//
		// The shared texture format is BGRA or RGBA and the staging textures are the same format.
		// If the texture format is BGRA and the receiving pixel buffer is RGBA/RGB or vice-versa,
		// the data has to be converted from BGRA to RGBA/RGB or RGBA to BGRA/BGR during the pixel copy.
		//
		if (glFormat == GL_RGBA) { // RGBA pixel buffer
			if (m_dwFormat == 28) // RGBA staging textures
				spoutcopy.rgba2rgba(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgra(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGRA_EXT) { // BGRA pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgra(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgba(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_RGB) { // RGB pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2rgb(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgr(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGR_EXT) { // BGR pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgr(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgb(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
		}

		spoutdx.GetDX11Context()->Unmap(pStagingTexture, 0);

		return true;
	} // endif DX11 map OK

	return false;

} // end ReadPixelData


// Create class staging textures for changed size or if they do not exist yet
// Two are available but only one can be allocated to save memory
// Format is the same as the shared texture - m_dwFormat
bool spoutGL::CheckStagingTextures(unsigned int width, unsigned int height, int nTextures)
{
	if (!spoutdx.GetDX11Device()) {
		return false;
	}

	D3D11_TEXTURE2D_DESC desc = { 0 };

	if (m_pStaging[0]) {

		// Get the size to test for change
		m_pStaging[0]->GetDesc(&desc);
		if (desc.Width != width || desc.Height != height) {
			// Staging textures must not be mapped before release
			if (m_pStaging[0]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[0]);
			if (m_pStaging[1]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[1]);
			m_pStaging[0] = nullptr;
			m_pStaging[1] = nullptr;
			// Drop through to create new textures
		}
		else {
			return true;
		}
	}

	if (!spoutdx.CreateDX11StagingTexture(spoutdx.GetDX11Device(), width, height, (DXGI_FORMAT)m_dwFormat, &m_pStaging[0]))
		return false;

	if (nTextures > 1) {
		if (!spoutdx.CreateDX11StagingTexture(spoutdx.GetDX11Device(), width, height, (DXGI_FORMAT)m_dwFormat, &m_pStaging[1]))
			return false;
	}

	// Update class width and height
	m_Width = width;
	m_Height = height;

	// Reset staging texture index
	m_Index = 0;
	m_NextIndex = 0;

	// Also reset PBO index
	PboIndex = 0;
	NextPboIndex = 0;

	// Did something go wrong somehow
	if (!m_pStaging[0])
		return false;
	if (nTextures > 1 && !m_pStaging[1])
		return false;

	return true;

} // end CheckStagingTextures


//
// Memoryshare functions - receive only
//

//
// Read rgba shared memory to texture pixel data
//
bool spoutGL::ReadMemory(const char* sendername, GLuint TexID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// Open the sender memory map and close it afterwards for a read,
	// so that the receiver does not retain a handle to the shared memory
	if (!memoryshare.OpenSenderMemory(sendername))
		return false;

	unsigned char* pBuffer = memoryshare.LockSenderMemory();

	if (!pBuffer) {
		memoryshare.CloseSenderMemory();
		return false;
	}

	bool bRet = true; // Error only if pixel read fails

	// Query a new frame and read pixels while the buffer is locked
	if (frame.GetNewFrame()) {
		if (bInvert) {
			// Create or resize a local OpenGL texture
			CheckOpenGLTexture(m_TexID, GL_RGBA, width, height);
			// Read the memory pixels into it
			glBindTexture(GL_TEXTURE_2D, m_TexID);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
			glBindTexture(GL_TEXTURE_2D, 0);
			// Copy to the user texture, inverting at the same time
			bRet = CopyTexture(m_TexID, GL_TEXTURE_2D, TexID, TextureTarget, width, height, true, HostFBO);
		}
		else {
			// No invert - copy memory pixels directly to the user texture
			glBindTexture(TextureTarget, TexID);
			glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
			glBindTexture(TextureTarget, 0);
		}
	} // No new frame

	memoryshare.UnlockSenderMemory();

	// Close the memory map handle so the sender can finally close the map
	memoryshare.CloseSenderMemory();

	return bRet;

}

//
// Read rgba shared memory to image pixels
//
bool spoutGL::ReadMemoryPixels(const char* sendername, unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!pixels)
		return false;

	// Open the sender memory map and close it afterwards for a read,
	// so that the receiver does not retain a handle to the shared memory.
	if (!memoryshare.OpenSenderMemory(sendername)) {
		SpoutLogWarning("spoutGLDXinterop::ReadMemoryPixels - no buffer");
		return false;
	}

	unsigned char* pBuffer = memoryshare.LockSenderMemory();
	if (!pBuffer) {
		memoryshare.CloseSenderMemory();
		SpoutLogWarning("spoutGLDXinterop::ReadMemoryPixels - no buffer lock");
		return false;
	}

	// Query a new frame and read pixels while the buffer is locked
	if (frame.GetNewFrame()) {
		// Read pixels from shared memory
		if (glFormat == GL_RGBA) {
			spoutcopy.CopyPixels(pBuffer, pixels, width, height, GL_RGBA, bInvert);
		}
	}
	memoryshare.UnlockSenderMemory();
	// Close the memory map handle so the sender can close the map
	memoryshare.CloseSenderMemory();

	return true;

}

//---------------------------------------------------------
bool spoutGL::SetHostPath(const char *sendername)
{
	SharedTextureInfo info;
	if (!sendernames.getSharedInfo(sendername, &info)) {
		SpoutLogWarning("spoutGL::SetHostPath(%s) - could not get sender info", sendername);
		return false;
	}
	char exepath[256];
	GetModuleFileNameA(NULL, exepath, sizeof(exepath));
	// Description is defined as wide chars, but the path is stored as byte chars
	strcpy_s((char*)info.description, 256, exepath);
	if (!sendernames.setSharedInfo(sendername, &info)) {
		SpoutLogWarning("spoutGL::SetHostPath(%s) - could not set sender info", sendername);
	}
	return true;

}


//---------------------------------------------------------
// CPU mode is "not GL/DX compatible"
// 2.006 senders will typically not have this bit set
// so GL/DX compatibility is assumed
bool spoutGL::SetSenderCPUmode(const char *sendername, bool bCPU)
{
	SharedTextureInfo info;

	if (sendernames.getSharedInfo(sendername, &info)) {
		// CPU mode - set top bit of 32 bit partner ID field
		// 1000 0000 0000 0000 0000 0000 0000 0000
		if (bCPU) {
			info.partnerId = info.partnerId | 0x80000000; // Set bit
			m_bSenderCPUmode = true;
		}
		else {
			info.partnerId = info.partnerId & ~0x80000000; // Clear bit default
			m_bSenderCPUmode = false;
		}

		// Save the info for this sender in the sender shared memory map
		sendernames.setSharedInfo(sendername, &info);
		return true;
	}
	return false;

}

//---------------------------------------------------------
bool spoutGL::OpenDirectX11()
{
	SpoutLogNotice("spoutGL::OpenDirectX11");
	return spoutdx.OpenDirectX11();
}

//---------------------------------------------------------
void spoutGL::CleanupDirectX()
{
	// DirectX 9 not supported >= 2.007
	CleanupDX11();
}

//---------------------------------------------------------
void spoutGL::CleanupDX11()
{
	if (spoutdx.GetDX11Device()) {

		SpoutLogNotice("spoutGL::CleanupDX11()");

		// Reference count warnings are in the SpoutDirectX class

		if (m_pSharedTexture) {
			SpoutLogNotice("    Releasing shared texture");
			// Release interop link before releasing the texture
			if (m_hInteropDevice && m_hInteropObject)
				wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
			spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pSharedTexture);
		}

		// Important to set pointer to NULL or it will crash if released again
		m_pSharedTexture = nullptr;

		// Re-set shared texture handle
		m_dxShareHandle = nullptr;

		// Release staging texture if they have been used
		if (m_pStaging[0]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[0]);
		if (m_pStaging[1]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[1]);
		m_pStaging[0] = nullptr;
		m_pStaging[1] = nullptr;
		m_Index = 0;
		m_NextIndex = 0;

		// 12.11.18 - To avoid memory leak with dynamic objects
		//            must always be freed, not only on exit.
		//            Device recreated for a new sender.
		// Releases immediate context and device in the SpoutDirectX class
		// spoutdx.GetDX11Context() and spoutdx.GetDX11Device() are copies of these
		spoutdx.CloseDirectX11();
	}

}

//
// Extensions and availability
//

//---------------------------------------------------------
bool spoutGL::LoadGLextensions()
{
	// Return silently if already loaded
	if (m_caps > 0)
		return true;

	m_caps = loadGLextensions(); // in spoutGLextensions

	if (m_caps == 0) {
		SpoutLogError("spoutGL::LoadGLextensions failed");
		return false;
	}

	m_bFBOavailable = false;
	if (m_caps & GLEXT_SUPPORT_FBO) m_bFBOavailable = true;
	// FBO not available is terminal
	if (!m_bFBOavailable) {
		SpoutLogError("spoutGL::LoadGLextensions - no FBO extensions available");
		return false;
	}

	m_bGLDXavailable = false;
	m_bBLITavailable = false;
	m_bSWAPavailable = false;
	m_bBGRAavailable = false;
	m_bCOPYavailable = false;
	m_bCONTEXTavailable = false;

	if (m_caps & GLEXT_SUPPORT_NVINTEROP) m_bGLDXavailable = true; // Interop needed for texture sharing
	if (m_caps & GLEXT_SUPPORT_FBO_BLIT)  m_bBLITavailable = true;
	if (m_caps & GLEXT_SUPPORT_SWAP)      m_bSWAPavailable = true;
	if (m_caps & GLEXT_SUPPORT_BGRA)      m_bBGRAavailable = true;
	if (m_caps & GLEXT_SUPPORT_COPY)      m_bCOPYavailable = true;
	if (m_caps & GLEXT_SUPPORT_CONTEXT)   m_bCONTEXTavailable = true;

	// Test PBO availability unless user has selected buffering off
	// m_bPBOavailable also set by SetBufferMode()
	if (m_bPBOavailable)
		if (!(m_caps && GLEXT_SUPPORT_PBO)) m_bPBOavailable = false;

	if (!m_bGLDXavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - interop extensions not available");
	if (!m_bBLITavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - fbo blit extension not available");
	if (!m_bSWAPavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - sync control extensions not available");
	if (!m_bBGRAavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - bgra extension not available");
	if (!m_bCOPYavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - copy extensions not available");
	if (!m_bPBOavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - pbo extensions not available");
	if (!m_bCONTEXTavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - context extension not available");
	m_bExtensionsLoaded = true;

	return true;
}

//---------------------------------------------------------
bool spoutGL::IsGLDXavailable()
{
	return m_bGLDXavailable;
}

//---------------------------------------------------------
bool spoutGL::IsBLITavailable()
{
	return m_bBLITavailable;
}

//---------------------------------------------------------
bool spoutGL::IsSWAPavailable()
{
	return m_bSWAPavailable;
}

//---------------------------------------------------------
bool spoutGL::IsBGRAavailable()
{
	return m_bBGRAavailable;
}

//---------------------------------------------------------
bool spoutGL::IsCOPYavailable()
{
	return m_bCOPYavailable;
}

//---------------------------------------------------------
bool spoutGL::IsPBOavailable()
{
	return m_bPBOavailable;
}

//---------------------------------------------------------
bool spoutGL::IsCONTEXTavailable()
{
	return m_bCONTEXTavailable;
}

// 
// Legacy OpenGL functions
//

#ifdef legacyOpenGL

//---------------------------------------------------------
void spoutGL::SaveOpenGLstate(unsigned int width, unsigned int height, bool bFitWindow)
{
	float dim[4];
	float vpScaleX, vpScaleY, vpWidth, vpHeight;
	int vpx, vpy;

	// save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_TRANSFORM_BIT);

	// find the current viewport dimensions in order to scale to the aspect ratio required
	glGetFloatv(GL_VIEWPORT, dim);

	// Fit to window
	if (bFitWindow) {
		// Scale both width and height to the current viewport size
		vpScaleX = dim[2] / (float)width;
		vpScaleY = dim[3] / (float)height;
		vpWidth = (float)width  * vpScaleX;
		vpHeight = (float)height * vpScaleY;
		vpx = vpy = 0;
	}
	else {
		// Preserve aspect ratio of the sender
		// and fit to the width or the height
		vpWidth = dim[2];
		vpHeight = ((float)height / (float)width)*vpWidth;
		if (vpHeight > dim[3]) {
			vpHeight = dim[3];
			vpWidth = ((float)width / (float)height)*vpHeight;
		}
		vpx = (int)(dim[2] - vpWidth) / 2;;
		vpy = (int)(dim[3] - vpHeight) / 2;
	}

	glViewport((int)vpx, (int)vpy, (int)vpWidth, (int)vpHeight);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); // reset the current matrix back to its default state
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}


void spoutGL::RestoreOpenGLstate()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopClientAttrib();
	glPopAttrib();

}

#endif

//
// Utility
//

//---------------------------------------------------------
// Given a DeviceKey string from a DisplayDevice
// read all the information about the adapter.
// Only used by this class.
bool spoutGL::OpenDeviceKey(const char* key, int maxsize, char* description, char* version)
{
	if (!key)
		return false;

	// Extract the subkey from the DeviceKey string
	HKEY hRegKey = nullptr;
	DWORD dwSize = 0;
	DWORD dwKey = 0;

	char output[256];
	strcpy_s(output, 256, key);
	char *found = strstr(output, "System");
	if (!found)
		return false;
	std::string SubKey = found;

	// Convert all slash to double slash using a C++ string function
	// to get subkey string required to extract registry information
	for (unsigned int i = 0; i < SubKey.length(); i++) {
		if (SubKey[i] == '\\') {
			SubKey.insert(i, 1, '\\');
			++i; // Skip inserted char
		}
	}

	// Open the key to find the adapter details
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey.c_str(), NULL, KEY_READ, &hRegKey) == 0) {
		dwSize = MAX_PATH;
		// Adapter name
		if (RegQueryValueExA(hRegKey, "DriverDesc", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			strcpy_s(description, (rsize_t)maxsize, output);
		}
		if (RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			// Find the last 6 characters of the version string then
			// convert to a float and multiply to get decimal in the right place
			sprintf_s(output, 256, "%5.2f", atof(output + strlen(output) - 6)*100.0);
			strcpy_s(version, (rsize_t)maxsize, output);
		} // endif DriverVersion
		RegCloseKey(hRegKey);
	} // endif RegOpenKey

	return true;
}

//---------------------------------------------------------
void spoutGL::trim(char* s) {
	char* p = s;
	int l = (int)strlen(p);

	while (isspace(p[l - 1])) p[--l] = 0;
	while (*p && isspace(*p)) ++p, --l;

	memmove(s, p, (size_t)(l + 1));
}

//
// Errors
//

void spoutGL::PrintFBOstatus(GLenum status)
{
	char tmp[256];
	sprintf_s(tmp, 256, "FBO status error %u (0x%.7X) - ", status, status);
	if (status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_UNSUPPORTED_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT - width-height problems?");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
	// else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
	// 	strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT\n");
	else
		strcat_s(tmp, 256, "Unknown Code");
	SpoutLogError("%s", tmp);
	GLerror();
}

bool spoutGL::GLerror() {
	GLenum err = GL_NO_ERROR;
	bool bError = false;
	while ((err = glGetError()) != GL_NO_ERROR) {
		SpoutLogError("    GLerror - OpenGL error = %u (0x%.7X)", err, err);
		bError = true;
		// gluErrorString needs glu32.lib
		// printf("GL error = %d (0x%.7X) %s\n", err, err, gluErrorString(err));
	}
	return bError;
}

//
// User settings recorded in the registry and retrieved in constructor
// Set for the application (except max senders which must be global)
//

//---------------------------------------------------------
bool spoutGL::GetBufferMode()
{
	return m_bPBOavailable;
}

//---------------------------------------------------------
void spoutGL::SetBufferMode(bool bActive)
{
	if (m_bExtensionsLoaded) {
		if (bActive) {
			if (m_caps & GLEXT_SUPPORT_PBO) {
				m_bPBOavailable = true;
			}
		}
		else {
			m_bPBOavailable = false;
		}
	}
	else {
		m_bPBOavailable = false;
	}
}

//---------------------------------------------------------
int spoutGL::GetBuffers()
{
	return m_nBuffers;
}

//---------------------------------------------------------
void spoutGL::SetBuffers(int nBuffers)
{
	m_nBuffers = nBuffers;
}

//---------------------------------------------------------
int spoutGL::GetMaxSenders()
{
	return sendernames.GetMaxSenders();
}

//---------------------------------------------------------
void spoutGL::SetMaxSenders(int maxSenders)
{
	// Setting must be global for all applications
	sendernames.SetMaxSenders(maxSenders);
}

//
// For 2.006 compatibility
//

//---------------------------------------------------------
bool spoutGL::GetDX9()
{
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);
	return (dwDX9 == 1);
}

//---------------------------------------------------------
bool spoutGL::SetDX9(bool bDX9)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", (DWORD)bDX9);
}

//---------------------------------------------------------
bool spoutGL::GetMemoryShareMode()
{
	DWORD dwMem = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem);
	return (dwMem == 1);
}

//---------------------------------------------------------
bool spoutGL::SetMemoryShareMode(bool bMem)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", (DWORD)bMem);
}

//---------------------------------------------------------
bool spoutGL::GetCPUmode()
{
	DWORD dwCpu = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwCpu);
	return (dwCpu == 1);
}

//---------------------------------------------------------
bool spoutGL::SetCPUmode(bool bCPU)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", (DWORD)bCPU);
}

//
// Return user set sharing mode
//
// 0 - texture : default
// 1 - memory  : not used
// 2 - CPU     : compatibility mode
//
int spoutGL::GetShareMode()
{
	DWORD dwMem = 0;
	DWORD dwCPU = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem);
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwCPU);

	if (dwCPU > 0) {
		return 2;
	}
	if (dwMem > 0) {
		return 1;
	}

	// 0 : Texture share default
	return 0;

}

//---------------------------------------------------------
// Set user sharing mode
// 0 - texture : 1 - memory : 2 - CPU
void spoutGL::SetShareMode(int mode)
{
	switch (mode) {

	case 1: // Memory
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 1);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 0);
		break;
	case 2: // CPU
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 0);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 1);
		break;
	default: // 0 - Texture
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 0);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 0);
		break;
	}
}

//
// Information
//

//---------------------------------------------------------
// Get the path of the host that produced the sender
// from the description string in the sender info memory map
// Could be used for other things in future
bool spoutGL::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	SharedTextureInfo info;
	int n;

	if (!sendernames.getSharedInfo(sendername, &info)) {
		// Just quit if the key does not exist
		SpoutLogWarning("spoutGL::GetHostPath - could not get sender info [%s]", sendername);
		return false;
	}

	n = maxchars;
	if (n > 256) n = 256; // maximum field width in shared memory

	strcpy_s(hostpath, n, (char*)info.description);

	return true;
}

//---------------------------------------------------------
int spoutGL::GetVerticalSync()
{
	// Needs OpenGL context
	if (wglGetCurrentContext()) {
		// needed for both sender and receiver
		if (m_bSWAPavailable) {
			return(wglGetSwapIntervalEXT());
		}
	}
	return 0;
}

//---------------------------------------------------------
bool spoutGL::SetVerticalSync(bool bSync)
{
	// wglSwapIntervalEXT specifies the minimum number
	// of video frame periods per buffer swap
	if (wglGetCurrentContext()) {
		if (m_bSWAPavailable) {
			if (bSync) {
				wglSwapIntervalEXT(1); // lock to monitor vsync
			}
			else {
				// buffer swaps are not synchronized to a video frame.
				wglSwapIntervalEXT(0); // unlock from monitor vsync
			}
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------
int spoutGL::GetSpoutVersion()
{
	// Version number is retrieved from the registry at class initialization
	// Integer number 2005, 2006, 2007 etc.
	// 0 for earlier than 2.005
	// Set by the Spout installer for 2.005/2.006, or by SpoutSettings
	return m_SpoutVersion;
}

//
// Utility
//

//---------------------------------------------------------
bool spoutGL::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget, unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	GLenum status;

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

	// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	// Attach the Source texture to the color buffer in our frame buffer
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Attach destination texture (the texture we write into) to second attachment point
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, DestTarget, DestID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {

		if (m_bBLITavailable) {
			if (bInvert) {
				// Blit method with checks - 0.75 - 0.85 msec
				// copy one texture buffer to the other while flipping upside down 
				// (OpenGL and DirectX have different texture origins)
				glBlitFramebufferEXT(0, 0, // srcX0, srcY0, 
					width, height,         // srcX1, srcY1
					0, height,             // dstX0, dstY0,
					width, 0,              // dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
			else {
				// Do not flip during blit
				glBlitFramebufferEXT(0, 0, // srcX0, srcY0, 
					width, height,         // srcX1, srcY1
					0, 0,                  // dstX0, dstY0,
					width, height,         // dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
		}
		else {
			// No fbo blit extension
			// Copy from the fbo (source texture attached) to the dest texture
			glBindTexture(DestTarget, DestID);
			glCopyTexSubImage2D(DestTarget, 0, 0, 0, 0, 0, width, height);
			glBindTexture(DestTarget, 0);
		}
	}
	else {
		PrintFBOstatus(status);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

} // end CopyTexture

//---------------------------------------------------------
void spoutGL::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	spoutcopy.RemovePadding(source, dest, width, height, stride, glFormat);
}


//
// DX11 versions - https://github.com/DashW/Spout2
//

//---------------------------------------------------------
bool spoutGL::ReadTexture(ID3D11Texture2D** texture)
{
	// Only for DX11 mode
	if (!texture || !*texture || !spoutdx.GetDX11Context())
		return false;

	D3D11_TEXTURE2D_DESC desc = { 0 };
	(*texture)->GetDesc(&desc);
	if (desc.Width != (unsigned int)m_Width || desc.Height != (unsigned int)m_Height) {
		return false;
	}
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Copy the shared texture if the sender has produced a new frame
		if (frame.GetNewFrame()) {
			spoutdx.GetDX11Context()->CopyResource(*texture, m_pSharedTexture);
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;

} // end ReadTexture

//---------------------------------------------------------
bool spoutGL::WriteTexture(ID3D11Texture2D** texture)
{
	// Only for DX11 mode
	if (!texture || !spoutdx.GetDX11Context()) {
		SpoutLogWarning("spoutGL::WriteTexture(ID3D11Texture2D** texture) failed");
		if (!texture)
			SpoutLogWarning("    ID3D11Texture2D** NULL");
		if (!spoutdx.GetDX11Context())
			SpoutLogVerbose("    pImmediateContext NULL");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc = { 0 };

	(*texture)->GetDesc(&desc);
	if (desc.Width != m_Width || desc.Height != m_Height) {
		SpoutLogWarning("spoutGL::WriteTexture(ID3D11Texture2D** texture) sizes do not match");
		SpoutLogWarning("    texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_Width, m_Height);
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		spoutdx.GetDX11Context()->CopyResource(m_pSharedTexture, *texture);
		// Flush after update of the shared texture on this device
		spoutdx.GetDX11Context()->Flush();
		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
		bRet = true;
	}

	return bRet;
}

//---------------------------------------------------------
bool spoutGL::WriteTextureReadback(ID3D11Texture2D** texture,
	GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	// Only for DX11 mode
	if (!texture || !spoutdx.GetDX11Context()) {
		SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) failed");
		if (!texture)
			SpoutLogWarning("    ID3D11Texture2D** NULL");
		if (!spoutdx.GetDX11Context())
			SpoutLogVerbose("    pImmediateContext NULL");
		return false;
	}

	if (!m_hInteropDevice || !m_hInteropObject) {
		SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) no interop device");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc = { 0 };

	(*texture)->GetDesc(&desc);
	if (desc.Width != m_Width || desc.Height != m_Height) {
		SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) sizes do not match");
		SpoutLogWarning("    texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_Width, m_Height);
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		bRet = true;
		// Copy the DirectX texture to the shared texture
		spoutdx.GetDX11Context()->CopyResource(m_pSharedTexture, *texture);
		// Flush after update of the shared texture on this device
		spoutdx.GetDX11Context()->Flush();
		// Copy the linked OpenGL texture back to the user texture
		if (width != m_Width || height != m_Height) {
			SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) sizes do not match");
			SpoutLogWarning("    OpenGL texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_Width, m_Height);
			bRet = false;
		}
		else if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			bRet = GetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			if (!bRet)
				SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) readback failed");
		}

		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;
}