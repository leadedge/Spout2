//
// VDJSpoutReceiver.cpp : Defines the exported functions for the DLL application.
//
//		09.02.15	corrected texture invert by adding invert flag to DrawSharedTexture
//		14.02.15	added Optimus enablement export
//					Changed to /MT compile
//					Inital release
//					Version 1.0
//		21.02.15	Fixed bug for fbo released too soon - only showed up with Optimus graphics
//					Verson 1.01
//		22.02.15	Debug SpoutPanel.exe to resolve freeze and crash due to "WideCharToMultiByte" with null command line
//					Set DX9 compatible because incompatible DX11 textures will not be received
//					Version 1.02
//		26.05.15	Recompile for revised SpoutPanel registry write of sender name
//					Version 1.03
//		07.07.15	Recompile on new drive Windows 7 32bit VS2010 - VDJ vers v8.0.2345
//		08.07.15	Create an invisible dummy button window for OpenGL due to SetPixelFormat problems noted with Mapio
//		01.08.15	Recompile for 2.004 release /MT
//					Version 1.04
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015. Lynn Jarvis, Leading Edge. Pty. Ltd.
//
//		This program is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You will receive a copy of the GNU Lesser General Public License along 
//		with this program.  If not, see http://www.gnu.org/licenses/.
//		--------------------------------------------------------------
//

#include "stdafx.h"
#include "VDJSpoutReceiver.h"


VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// Syphon comment : TODO: Is this good?
	// LJ - this limits it to VDJ 8

	// This is the standard DLL loader for COM object.
	// The memcmp(a, b, sizeof(XXXX)) function returns 0 if a and b match.
	// if the plugins returns CLASS_E_CLASSNOTAVAILABLE, it means the plugin
	// is not compatible with the current VirtualDJ version

	if(memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 
    if(memcmp(&riid, &IID_IVdjPluginVideoFx8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 

	*ppObject = new SpoutReceiverPlugin(); 

    return NO_ERROR; 
}

SpoutReceiverPlugin::SpoutReceiverPlugin()
{

	/*
	// Debug console window so printf works
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("VDJSpoutReceiver - 1.04\n");
	*/


	// DirectX9
	d3d_device = NULL;
	SourceSurface = NULL;
	TextureSurface = NULL;

	// SPOUT variables and functions
	m_Width = 0;
	m_Height = 0;
	SenderName[0] = 0;
	bInitialized = false;
	bSpoutOut = false; // toggle for plugin start and stop
	bOpenGL = false; // OpenGL initialization flag
	
	m_GLtextureVJ = 0; // OpenGL texture the same size as the VirtualDJ DirectX9 one
	m_fbo = 0; // FBO for texture transfers
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL; // rendering context
	m_hSharedRC = NULL; // shared context

	SelectButton = 0;

}

SpoutReceiverPlugin::~SpoutReceiverPlugin()
{
	
}

HRESULT __stdcall SpoutReceiverPlugin::OnLoad()
{
	DeclareParameterButton(&SelectButton, 1, "Sender", "Sender");
    return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";
    infos->PluginName = (char *)"VDJSpoutReceiver";
	infos->Description = (char *)"Receives frames from a Spout Sender\nSpout : http://Spout.zeal.co/";
	infos->Version = (char *)"v1.04";
    infos->Bitmap = NULL;

	// A receiver is a source
	infos->Flags = VDJFLAG_VIDEO_VISUALISATION | VDJFLAG_PROCESSFIRST; // to ensure that all other effects are processed afterwards

    return NO_ERROR;
}


HRESULT __stdcall SpoutReceiverPlugin::OnStart()
{
	// printf("OnStart\n");
	StartOpenGL(); // Initialize openGL if not already
	// To use DirectX 9 we need to specify that first
	// spoutreceiver.SetDX9(true); 
	bSpoutOut = true;

	return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnStop()
{
	// printf("OnStop\n");
	StartOpenGL(); // return to the main context
	return NO_ERROR;
}

// When DirectX/OpenGL is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutReceiverPlugin::OnDeviceInit() 
{
	// printf("OnDeviceInit\n");
	return S_OK;
}

HRESULT __stdcall SpoutReceiverPlugin::OnDeviceClose() 
{
	// printf("OnDeviceClose\n");
	if(m_hRC && wglMakeCurrent(m_hdc, m_hRC)) {
		if(bInitialized) spoutreceiver.ReleaseReceiver(); 
		if(m_fbo != 0) glDeleteFramebuffersEXT(1, &m_fbo);
		if(m_GLtextureVJ != 0) glDeleteTextures(1, &m_GLtextureVJ);	
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hSharedRC);
		wglDeleteContext(m_hRC);
	}

	// 08.07.15
	// Destroy dummy window used for OpenGL context creation
	if(m_hwnd) DestroyWindow(m_hwnd);

	bInitialized = false;
	bOpenGL = false;
	m_GLtextureVJ = 0;
	m_fbo = 0;
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL;
	m_hSharedRC = NULL;

	return S_OK;
}

HRESULT __stdcall SpoutReceiverPlugin::OnParameter(int ParamID) 
{
	// printf("OnParameter\n");
	// Activate SpoutPanel to select a sender
	// 22.02.15 - set DX9 compatible because incompatible DX11 textures will not be received
	if(spoutreceiver.GetDX9())
		spoutreceiver.SelectSenderPanel("/DX9");
	else
		spoutreceiver.SelectSenderPanel();

	return S_OK;
}


// Receive a sender texture and transfer to the VirtualDJ DirectX texture
HRESULT __stdcall SpoutReceiverPlugin::OnDraw()
{
	HRESULT hr;
	TVertex *vertices;
	DWORD dwFormat;
	HANDLE hShareHandle;
	unsigned int width, height;

	GetDevice(VdjVideoEngineDirectX9, (void **)&d3d_device);
	if(d3d_device) {

		// Get the Virtual DJ texture and description
		GetTexture(VdjVideoEngineDirectX9, (void **)&dxTexture, &vertices);
		dxTexture->GetLevelDesc(0, &desc);

		// Quit if OpenGL initialization failed
		if(!bOpenGL) return S_OK; // safety

		// Activate the shared context for draw
		if(!wglMakeCurrent(m_hdc, m_hSharedRC)) {
			// printf("wglMakeCurrent 1 fail\n");
			bOpenGL = false;
			StartOpenGL(); // It will start again if the start button is toggled
			return S_OK;
		}

		// Check for VirtualDJ texture size changed
		if(m_Width != desc.Width || m_Height != desc.Height || m_GLtextureVJ == 0) {
			m_Width = desc.Width;
			m_Height = desc.Height;
			// printf("Creating texture %dx%d\n", m_Width, m_Height);
			InitGLtexture(m_GLtextureVJ, m_Width, m_Height);
			// Set the viewport to the same size for texture drawing
			// We are the only GL context so no need to save it
			glViewport(0, 0, m_Width, m_Height);
			return S_OK; // do no more for this frame
		}

		if(bSpoutOut) { // The plugin has started		
		
			// Find a sender and connect when it is found
			if(!bInitialized) {
				if(spoutreceiver.GetActiveSender(activesender)) {
					// Find the information about the sender
					spoutreceiver.GetSenderInfo(activesender, m_SenderWidth, m_SenderHeight, hShareHandle, dwFormat);
					// Create a receiver using the active sender
					strcpy_s(SenderName, 256, activesender);
					spoutreceiver.CreateReceiver(SenderName, m_SenderWidth, m_SenderHeight, true);
					// printf("Created receiver [%s]\n", SenderName);
					bInitialized = true;
					return S_OK; // do no more for this frame
				} // found active sender
			} // was not initialized

			// Receive the sender shared texture
			width = m_SenderWidth;
			height = m_SenderHeight;

			if(spoutreceiver.ReceiveTexture(ReceivedName, width, height)) {

				// The texture was received OK, but check to see if the sender has changed
				if(strcmp(ReceivedName, SenderName) != 0
					||  m_SenderWidth  != width
					||	m_SenderHeight != height) {
						// printf("Sender has changed\n");
						strcpy_s(SenderName, 256, ReceivedName); // In case the sender name has changed
						m_SenderWidth = width;
						m_SenderHeight = height;
						spoutreceiver.ReleaseReceiver(); 
						spoutreceiver.CreateReceiver(SenderName, m_SenderWidth, m_SenderHeight, true);
						return(S_OK); // do no more for this frame
				} // sender changed size

				// ReceiveTexture was OK and the size has not changed so draw the
				// shared texture into the local texture via an fbo
				if(m_fbo == 0) glGenFramebuffersEXT(1, &m_fbo);

				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_GLtextureVJ, 0);
				glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
				GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
				if(status == GL_FRAMEBUFFER_COMPLETE_EXT) {

					spoutreceiver.DrawSharedTexture(1.0, 1.0, 1.0, false); // draw the sender's shared texture into the fbo

					// Transfer the OpenGL texture to system memory pixels
					hr = d3d_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &SourceSurface, NULL);
					if(SUCCEEDED(hr)) {
						hr = SourceSurface->LockRect(&d3dlr, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_DISCARD);
						if(SUCCEEDED(hr)) {

							// Read the local texture into the matching data buffer
							glEnable(GL_TEXTURE_2D);
							glBindTexture(GL_TEXTURE_2D, m_GLtextureVJ);
							glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid *)d3dlr.pBits);
							glBindTexture(GL_TEXTURE_2D, 0);
							glDisable(GL_TEXTURE_2D);

							// Copy to the Virtual DJ texture surface now that the surfaces are the same size
							hr = dxTexture->GetSurfaceLevel(0, &TextureSurface);
							if(SUCCEEDED(hr))
								hr = D3DXLoadSurfaceFromSurface(TextureSurface, NULL, NULL, SourceSurface, NULL, NULL, D3DTEXF_NONE, 0);

							if(TextureSurface) TextureSurface->Release();
							SourceSurface->UnlockRect();

							// Copy to the Virtual DJ texture surface
							hr = dxTexture->GetSurfaceLevel(0, &TextureSurface);
							if(SUCCEEDED(hr)) 
								D3DXLoadSurfaceFromSurface(TextureSurface, NULL, NULL, SourceSurface, NULL, NULL, D3DTEXF_NONE, 0);
							if(TextureSurface) TextureSurface->Release();
						} // lockrect failed
					} // create plain surface failed
					if(SourceSurface) SourceSurface->Release();
					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
				} // FBO not complete
			} // Receivetexture failed
		} // plugin has not started
	} // endif device

	return S_FALSE; // no device

}

// OpenGL setup function - tests for current context first
bool SpoutReceiverPlugin::StartOpenGL()
{
	HGLRC hrc = NULL;

	// printf("StartOpenGL()\n");

	// Check to see if a context has already been created
	if(bOpenGL && m_hdc && m_hRC) {
		// printf("    Switching back\n");
		// Switch back to the primary context to check it
		if(!wglMakeCurrent(m_hdc, m_hRC)) {
			// printf("    Not current - starting again\n");
			// Not current so start again
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_hSharedRC);
			wglDeleteContext(m_hRC);
			m_hdc = NULL;
			m_hRC = NULL;
			m_hSharedRC = NULL;
			bOpenGL = false;
			m_GLtextureVJ = 0;
			m_fbo = 0; // restart opengl
			bInitialized = false; // start again
			bOpenGL = InitOpenGL();
		}
		// do nothing
	}
	else {
		// printf("    Starting new\n");
		bInitialized = false; // start again
		bOpenGL = InitOpenGL();
	}

	return bOpenGL;
}


// Spout OpenGL initialization function
bool SpoutReceiverPlugin::InitOpenGL()
{
	// m_hwnd = GetForegroundWindow(); // causes problems with SetPixelFormat - noted with Mapio
	//
	// 08.07.15
	// We only need an OpenGL context with no render window because we don't draw to it
	// so create an invisible dummy button window. This is then independent from the host
	// program window (GetForegroundWindow). If SetPixelFormat has been called on the
	// host window it cannot be called again. This caused a problem in Mapio and could be
	// a problem with VirtualDJ.
	//
	// Microsoft :
	//
	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
	//
	// If hdc references a window, calling the SetPixelFormat function also changes the pixel
	// format of the window. Setting the pixel format of a window more than once can lead to
	// significant complications for the Window Manager and for multithread applications,
	// so it is not allowed. An application can only set the pixel format of a window one time.
	// Once a window's pixel format is set, it cannot be changed.
	//
	if(!m_hwnd || !IsWindow(m_hwnd)) {
		m_hwnd = CreateWindowA("BUTTON",
			            "VDJ Receiver",
				        WS_OVERLAPPEDWINDOW,
					    0, 0, 32, 32,
						NULL, NULL, NULL, NULL);
	}

	if(!m_hwnd) { printf("InitOpenGL error 1\n"); MessageBoxA(NULL, "Error 1\n", "InitOpenGL", MB_OK); return false; }
	m_hdc = GetDC(m_hwnd);
	if(!m_hdc) { printf("InitOpenGL error 2\n"); MessageBoxA(NULL, "Error 2\n", "InitOpenGL", MB_OK); return false; }
	
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24; // LJ DEBUG - was 16;
	pfd.cStencilBits = 8; // LJ DEBUG -added
	pfd.iLayerType = PFD_MAIN_PLANE;

	int iFormat = ChoosePixelFormat(m_hdc, &pfd);
	if(!iFormat) { printf("InitOpenGL error 3\n"); MessageBoxA(NULL, "Error 3\n", "InitOpenGL", MB_OK); return false; }

	if(!SetPixelFormat(m_hdc, iFormat, &pfd)) { printf("InitOpenGL error 4\n"); MessageBoxA(NULL, "Error 4\n", "InitOpenGL", MB_OK); return false; }

	m_hRC = wglCreateContext(m_hdc);
	if(!m_hRC) { printf("InitOpenGL error 5\n"); MessageBoxA(NULL, "Error 5\n", "InitOpenGL", MB_OK); return false; }

	wglMakeCurrent(m_hdc, m_hRC);
	if(wglGetCurrentContext() == NULL) { printf("InitOpenGL error 6\n"); MessageBoxA(NULL, "Error 6\n", "InitOpenGL", MB_OK); return false; }

	// Set up a shared context
	if(!m_hSharedRC) m_hSharedRC = wglCreateContext(m_hdc);
	if(!m_hSharedRC) { printf("InitOpenGL shared context not created\n"); }
	if(!wglShareLists(m_hSharedRC, m_hRC)) { printf("wglShareLists failed\n"); }

	// Drop through to return true
	
	/*
	// GetWindowTextA(m_hwnd, windowtitle, 256); // debug
	SendMessageTimeoutA(m_hwnd, WM_GETTEXT, 256, (LPARAM)windowtitle, SMTO_ABORTIFHUNG, 128, NULL);
	printf("InitOpenGL : hwnd = %x (%s), hdc = %x, context = %x\n", m_hwnd, windowtitle, m_hdc, m_hRC);

	int nTotalAvailMemoryInKB = 0;
	int nCurAvailMemoryInKB = 0;
	// GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
	// GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
	glGetIntegerv(0x9048, &nTotalAvailMemoryInKB);
	glGetIntegerv(0x9049, &nCurAvailMemoryInKB);
	printf("Memory : Total [%i], Available [%i]\n", nTotalAvailMemoryInKB, nCurAvailMemoryInKB);
	*/

	return true;

}

bool SpoutReceiverPlugin::InitGLtexture(GLuint &texID, unsigned int width, unsigned int height)
{
	if(texID != 0) glDeleteTextures(1, &texID);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLerror("Texture");

	return true;
}

// For debugging only - needs glu32.lib
void SpoutReceiverPlugin::GLerror(char *intext) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("GL error (%s) = %d (0x%x) %s\n", intext, err, err, gluErrorString(err));
	}
}	

void SpoutReceiverPlugin::GLfboError(GLenum status)
{
	switch(status) {
		case GL_FRAMEBUFFER_UNDEFINED_EXT :
			printf("FBO error 1\n");
			// is returned if target is the default framebuffer, but the default framebuffer does not exist.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT :
			printf("FBO error 2\n");
			// is returned if any of the framebuffer attachment points are framebuffer incomplete.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT :
			printf("FBO error 3\n");
			// is returned if the framebuffer does not have at least one image attached to it.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT :
			printf("FBO error 4\n");
			// is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE
			// for any color attachment point(s) named by GL_DRAWBUFFERi.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT :
			printf("FBO error 5\n");
			// is returned if GL_READ_BUFFER is not GL_NONE and the value of
			// GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment 
			// point named by GL_READ_BUFFER
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT :
			printf("FBO error 6\n");
			// is returned if the combination of internal formats of the attached images
			// violates an implementation-dependent set of restrictions.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT :
			printf("FBO error 7\n");
			// is returned if the value of GL_RENDERBUFFER_SAMPLES
			// is not the same for all attached renderbuffers; 
			// if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures;
			// or, if the attached images are a mix of renderbuffers and textures,
			// the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES
			//
			// is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS
			// is not the same for all attached textures; 
			// or, if the attached images are a mix of renderbuffers and textures, the value of
			// GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT :
			printf("FBO error 8\n");
			// is returned if any framebuffer attachment is layered, and any populated attachment
			// is not layered, or if all populated color attachments are not from textures of the same target.
			break;
		case 0:
			GLerror("FBO error 9");
			// Additionally, if an error occurs, zero is returned. 
			// resize the local texture
			break;
	}
}
