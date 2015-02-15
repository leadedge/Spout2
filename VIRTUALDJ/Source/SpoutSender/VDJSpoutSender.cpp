//
// VDJSpoutSender.cpp : Defines the exported functions for the DLL application.
//
//		10.02.15	Inital testing
//		14.02.15	added Optimus enablement export
//					Changed to /MT compile
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

#include "stdafx.h"
#include "VDJSpoutSender.h"

// to get the dll hModule since there is no main
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

// This allows the Optimus global 3d setting to be "adapt" instead of "high performance"
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// Syphon comment : TODO: Is this good?
	if(memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 
    if(memcmp(&riid, &IID_IVdjPluginVideoFx8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 

	*ppObject = new SpoutSenderPlugin(); 

    return NO_ERROR; 
}


SpoutSenderPlugin::SpoutSenderPlugin()
{

	// DirectX9
	d3d_device = NULL;
	source_surface = NULL;
	texture_surface = NULL;

	// SPOUT variables and functions
	m_Width = 0;
	m_Height = 0;
	SenderName[0] = 0;
	bInitialized = false;
	bSpoutOut = false; // toggle for plugin start and stop
	bOpenGL = true; // Glut initialization test flag - assume it will work to start
	
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL; // rendering context
	m_hSharedRC = NULL; // shared context

	/*
	// Debug console window so printf works
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutSenderPlugin()\n");
	*/


}

SpoutSenderPlugin::~SpoutSenderPlugin()
{
	
}

HRESULT __stdcall SpoutSenderPlugin::OnLoad()
{
    return NO_ERROR;
}

HRESULT __stdcall SpoutSenderPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";
    infos->PluginName = (char *)"VDJSpoutSender";
    infos->Description = (char *)"Sends frames to a Spout Receiver\nSpout : http://Spout.zeal.co/";
	infos->Version = (char *)"v1.0";
    infos->Bitmap = NULL;

	// A sender is an effect
	// A receiver is a source
	infos->Flags = VDJFLAG_PROCESSLAST; // to ensure that all other effects are processed first

    return NO_ERROR;
}


HRESULT __stdcall SpoutSenderPlugin::OnStart()
{
	printf("OnStart()\n");
	StartOpenGL(); // Initialize openGL if not already
	bSpoutOut = true;

	return NO_ERROR;
}

HRESULT __stdcall SpoutSenderPlugin::OnStop()
{
	printf("OnStop()\n");

	StartOpenGL(); // return to the main context
	bSpoutOut = false;
 
	return NO_ERROR;
}

// When DirectX/OpenGL is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutSenderPlugin::OnDeviceInit() 
{
	printf("OnDeviceInit()\n");
	return S_OK;
}

HRESULT __stdcall SpoutSenderPlugin::OnDeviceClose() 
{
	printf("OnDeviceClose()\n");
	// Cleanup
	if(m_hRC && wglMakeCurrent(m_hdc, m_hRC)) {
		if(bInitialized) spoutsender.ReleaseSender();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hSharedRC);
		wglDeleteContext(m_hRC);
	}
	bOpenGL = false;
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL;
	m_hSharedRC = NULL;

	return S_OK;
}


// use this function to render the GL surface on the device, using any modification you want
// return S_OK if you actually draw the texture on the device, or S_FALSE to let VirtualDJ do it
// if using VDJPLUGINFLAG_VIDEOINPLACE, texture and vertices will be NULL
HRESULT __stdcall SpoutSenderPlugin::OnDraw()
{
	TVertex *vertices;

	// Quit if OpenGL initialization failed
	if(!bOpenGL) { DrawDeck(); return S_OK; }

	// Activate the shared context for draw
	if(!wglMakeCurrent(m_hdc, m_hSharedRC)) {
		printf("wglMakeCurrent 1 fail\n");
		bOpenGL = false;
		// It will start again if the start button is toggled
		DrawDeck();
		return S_OK;
	}

	// In order to draw the original image, you can either just call DrawDeck()
	// if you don't need to modify the image (for overlay plugins for examples),
	// or call GetTexture to get low-level access to the texture and its vertices.

	// Get the DX9 device
	GetDevice(VdjVideoEngineDirectX9, (void **)&d3d_device);
	if(d3d_device) {

		// Get the Virtual DJ texture and description
		GetTexture(VdjVideoEngineDirectX9, (void **)&dxTexture, &vertices);
		dxTexture->GetLevelDesc(0, &desc);
		if(!dxTexture) {
			DrawDeck();
		    return S_OK; // Let VirtualDJ do the drawing
		}

		// Activate the shared context for draw
		if(!wglMakeCurrent(m_hdc, m_hSharedRC)) {
			printf("wglMakeCurrent 2 fail\n");
			bOpenGL = false;
			// It will start again if the start button is toggled
			DrawDeck();
			return S_OK;
		}

		// Is Spout initialized yet ?
		if(!bInitialized) {
			m_Width = desc.Width;
			m_Height = desc.Height;

			// This is a sender so create one
			sprintf_s(SenderName, 256, "VirtualDJ Spout Sender");

			// To use DirectX 9 we need to specify that first
			spoutsender.SetDX9(true);

			// And we also have to set the shared texture format as D3DFMT_X8R8G8B8 so that receivers know it
			// because the default format argument is zero and that assumes D3DFMT_A8R8G8B8
			if(spoutsender.CreateSender(SenderName, m_Width, m_Height, (DWORD)D3DFMT_X8R8G8B8)) {
				bInitialized = true;
			}


		}
		else if(m_Width != desc.Width || m_Height != desc.Height) {

			// Initialized but has the texture changed size ?
			m_Width = desc.Width;
			m_Height = desc.Height;

			// Update the sender	
			spoutsender.UpdateSender(SenderName, m_Width, m_Height);

		}
		else if(bSpoutOut) { // Initialized and plugin has started

			// Copy from video memory to system memory
			hr = d3d_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &source_surface, NULL);
			if(SUCCEEDED(hr)) {
				
				// Get the Virtual DJ texture Surface
				hr = dxTexture->GetSurfaceLevel(0, &texture_surface);

				if(SUCCEEDED(hr)) {

					// Copy Surface to Surface
					hr = d3d_device->GetRenderTargetData(texture_surface, source_surface);	

					if(SUCCEEDED(hr)) {
						// Lock the source surface using some flags for optimization
						hr = source_surface->LockRect(&d3dlr, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);
						if(SUCCEEDED(hr)) {
							source_surface->GetDesc(&desc);
							// Pass the pixels to spout
							// Disable invert of texture because this is a DirectX source
							// 4-byte alignment might need checking
							if(desc.Format == D3DFMT_X8R8G8B8) { // We have initialized the sender for this format
								spoutsender.SendImage((unsigned char *)d3dlr.pBits, desc.Width, desc.Height, GL_BGRA_EXT, true, false);
							}
							source_surface->UnlockRect();
						}
					}
				}
			}

			if(texture_surface) texture_surface->Release();
			if(source_surface) source_surface->Release();
			texture_surface = NULL;
			source_surface = NULL;

		}
	}

	DrawDeck(); // Draw the image coming in (necessary ?)

	return S_OK;

}


// OpenGL setup function - tests for current context first
bool SpoutSenderPlugin::StartOpenGL()
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
bool SpoutSenderPlugin::InitOpenGL()
{
	char windowtitle[512];

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

