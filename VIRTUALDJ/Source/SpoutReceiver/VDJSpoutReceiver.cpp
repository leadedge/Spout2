//
// VDJSpoutReceiver.cpp : Defines the exported functions for the DLL application.
//
//		09.02.15	corrected texture invert by adding invert flag to DrawSharedTexture
//		10.02.15	Inital release
//					Version 1.0
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
	printf("VDJSpoutReceiver\n");
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
	infos->Author = NULL;
    infos->PluginName = (char *)"Spout Receiver";
    infos->Description = (char *)"http://Spout.zeal.co/";
	infos->Version = (char *)"v1.0";
    infos->Bitmap = NULL;

	// A receiver is a source
	infos->Flags = VDJFLAG_VIDEO_VISUALISATION | VDJFLAG_PROCESSFIRST; // to ensure that all other effects are processed afterwards

    return NO_ERROR;
}


HRESULT __stdcall SpoutReceiverPlugin::OnStart()
{
	StartOpenGL(); // Initialize openGL if not already
	spoutreceiver.SetDX9(true); // To use DirectX 9 we need to specify that first
	bSpoutOut = true;

	return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnStop()
{
	StartOpenGL(); // return to the main context
	bSpoutOut = false;
	return NO_ERROR;
}

// When DirectX/OpenGL is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutReceiverPlugin::OnDeviceInit() 
{
	return S_OK;
}

HRESULT __stdcall SpoutReceiverPlugin::OnDeviceClose() 
{
	if(m_hRC && wglMakeCurrent(m_hdc, m_hRC)) {
		if(bInitialized) {
			spoutreceiver.ReleaseReceiver(); 
		}
		if(m_fbo != 0) glDeleteFramebuffersEXT(1, &m_fbo);
		if(m_GLtextureVJ != 0) glDeleteTextures(1, &m_GLtextureVJ);	
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hSharedRC);
		wglDeleteContext(m_hRC);
	}
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
			bOpenGL = false;
			// It will start again if the start button is toggled
			return S_OK;
		}

		// Check for VirtualDJ texture size changed
		if(m_Width != desc.Width || m_Height != desc.Height || m_GLtextureVJ == 0) {
			m_Width = desc.Width;
			m_Height = desc.Height;
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
					bInitialized = true;
					return S_OK; // do no more for this frame
				} // found active sender
			} // was not initialized

			// Receive the sender shared texture
			width = m_SenderWidth;
			height = m_SenderHeight;

			// This is a tricky thing
			// Because no texture handle is passed, there is no actual read into a texture
			// but all the checks for user selection and size change are done by ReceiveTexture
			// and a new name, width and height are returned if so.
			if(spoutreceiver.ReceiveTexture(ReceivedName, width, height)) {

				// The texture was received OK, but check to see if the sender has changed
				if(strcmp(ReceivedName, SenderName) != 0
					||  m_SenderWidth  != width
					||	m_SenderHeight != height) {
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

					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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
							SourceSurface->UnlockRect();

							// Copy to the Virtual DJ texture surface now that the surfaces are the same size
							hr = dxTexture->GetSurfaceLevel(0, &TextureSurface);
							if(SUCCEEDED(hr)) {
								hr = D3DXLoadSurfaceFromSurface(TextureSurface, NULL, NULL, SourceSurface, NULL, NULL, D3DTEXF_NONE, 0);
							}
							if(TextureSurface) TextureSurface->Release();
						} // lockrect failed
					} // create plain surface failed
					if(SourceSurface) SourceSurface->Release();

				} // fbo attach failed
				// else {
					// GLfboError(status); // debug to report the error
				// }
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

				// This can be done to preserve memory in case the OpenGL context is lost and created again
				// if(m_fbo != 0) glDeleteFramebuffersEXT(1, &m_fbo);

			} // Received OK
			else { 
				// start again
				if(bInitialized)  spoutreceiver.ReleaseReceiver();
				bInitialized = false;		
			} // Sender is gone
		} // endif plugin activated

	} // endif device

	return S_FALSE;

}

// OpenGL setup function - tests for current context first
bool SpoutReceiverPlugin::StartOpenGL()
{
	HGLRC hrc = NULL;

	// Check to see if a context has already been created
	if(bOpenGL && m_hdc && m_hRC) {
		// Switch back to the primary context to check it
		if(!wglMakeCurrent(m_hdc, m_hRC)) {
			// Not current so start again
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_hSharedRC);
			wglDeleteContext(m_hRC);
			m_hdc = NULL;
			m_hRC = NULL;
			m_hSharedRC = NULL;
			// restart opengl
			bOpenGL = InitOpenGL();
		}
	}
	else {
		bOpenGL = InitOpenGL();
	}

	return bOpenGL;
}


// Spout OpenGL initialization function
bool SpoutReceiverPlugin::InitOpenGL()
{
	// We only need an OpenGL context with no window
	m_hwnd = GetForegroundWindow(); // Any window will do - we don't render to it
	if(!m_hwnd) { printf("InitOpenGL error 1\n"); MessageBoxA(NULL, "Error 1\n", "InitOpenGL", MB_OK); return false; }
	m_hdc = GetDC(m_hwnd);
	if(!m_hdc) { printf("InitOpenGL error 2\n"); MessageBoxA(NULL, "Error 2\n", "InitOpenGL", MB_OK); return false; }
	GetWindowTextA(m_hwnd, windowtitle, 256); // debug

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
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
	// printf("InitOpenGL : hwnd = %x (%s), hdc = %x, context = %x\n", m_hwnd, windowtitle, m_hdc, m_hRC);

	// int nCurAvailMemoryInKB = 0;
	// glGetIntegerv(0x9049, &nCurAvailMemoryInKB);
	// printf("Memory available [%i]\n", nCurAvailMemoryInKB);

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
