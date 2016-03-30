//
// VDJSpoutSender.cpp : Defines the exported functions for the DLL application.
//
//		10.02.15	Inital testing
//		14.02.15	added Optimus enablement export
//					Changed to /MT compile
//					Version 1.0
//		21.02.15	Removed OptimusEnablement export - does not work for a dll
//					Version 1.01
//		26.05.15	Recompile for revised SpoutPanel registry write of sender name
//					Version 1.02
//		08.07.15	Create an invisible dummy button window for OpenGL due to SetPixelFormat problems noted with Mapio
//		01.08.15	Recompile for 2.004
//					Version 1.03
//		17.12.15	Clean up and rebuild for 2.005 release VS2012
//					Version 1.04
//
// Example : http://www.virtualdj.com/wiki/Plugins_SDKv8_Example.html
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2016. Lynn Jarvis, Leading Edge. Pty. Ltd.
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
//		30.03.16 - rebuild for Spout 2.005 release Version 1.05
//				   VS2012 /MT
//

#include "stdafx.h"
#include "VDJSpoutSender.h"


VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// VDJ 8
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
	printf("SpoutSenderPlugin() - testing\n");
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
	infos->Version = (char *)"v1.05";
    infos->Bitmap = NULL;

	// A sender is an effect - process last so all other effects are shown
	infos->Flags = VDJFLAG_PROCESSLAST;

    return NO_ERROR;
}


HRESULT __stdcall SpoutSenderPlugin::OnStart()
{
	
	StartOpenGL(); // Initialize openGL if not already
	bSpoutOut = true;

	return NO_ERROR;
}

HRESULT __stdcall SpoutSenderPlugin::OnStop()
{
	// Cleanup and start again on start
	if(m_hRC && wglMakeCurrent(m_hdc, m_hRC)) {
		if(bInitialized) spoutsender.ReleaseSender();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hSharedRC);
		wglDeleteContext(m_hRC);
	}

	// Destroy OpenGL window
	if(m_hwnd) DestroyWindow(m_hwnd);

	bInitialized = false;
	bOpenGL = false;
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL;
	m_hSharedRC = NULL;
	bSpoutOut = false;
 
	return NO_ERROR;
}

// When DirectX/OpenGL is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutSenderPlugin::OnDeviceInit() 
{
	return S_OK;
}

HRESULT __stdcall SpoutSenderPlugin::OnDeviceClose() 
{
	// Cleanup
	if(m_hRC && wglMakeCurrent(m_hdc, m_hRC)) {
		if(bInitialized) spoutsender.ReleaseSender();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hSharedRC);
		wglDeleteContext(m_hRC);
	}

	// Destroy OpenGL window
	if(m_hwnd) DestroyWindow(m_hwnd);

	bInitialized = false;
	bOpenGL = false;
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hRC = NULL;
	m_hSharedRC = NULL;

	return S_OK;
}

ULONG __stdcall SpoutSenderPlugin::Release()
{
	delete this; 
	return S_OK;
}


HRESULT __stdcall SpoutSenderPlugin::OnDraw()
{
	TVertex *vertices;

	// Quit if OpenGL initialization failed
	if(!bOpenGL) { 
		DrawDeck(); 
		return S_OK; 
	}

	// Activate the shared context for draw
	// This can fail if the video window is closed and re-opened
	// Possibly because the dc that was orginally used is gone
	// It will start again if the start button is toggled
	// but calling StartOpenGL here seems to work OK.
	if(!wglMakeCurrent(m_hdc, m_hSharedRC)) {
		bOpenGL = false;
		StartOpenGL(); // Initialize openGL again
		return S_OK;
	}

	// Get the DX9 device
	GetDevice(VdjVideoEngineDirectX9, (void **)&d3d_device);
	if(d3d_device) {

		// Get the Virtual DJ texture and description
		GetTexture(VdjVideoEngineDirectX9, (void **)&dxTexture, &vertices);
		if(!dxTexture) {
			DrawDeck(); // Let VirtualDJ do the drawing
		    return S_OK;
		}
		dxTexture->GetLevelDesc(0, &desc);

		// Is Spout initialized yet ?
		if(!bInitialized) {
			m_Width = desc.Width;
			m_Height = desc.Height;
			// This is a sender so create one
			sprintf_s(SenderName, 256, "VirtualDJ Spout Sender");
			// The default format argument is zero and that assumes D3DFMT_A8R8G8B8
			if(spoutsender.CreateSender(SenderName, m_Width, m_Height)) {
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
					// Get the rendertarget data into system memory
					hr = d3d_device->GetRenderTargetData(texture_surface, source_surface);	
					if(SUCCEEDED(hr)) {
						// Lock the source surface using some flags for optimization
						hr = source_surface->LockRect(&d3dlr, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);
						if(SUCCEEDED(hr)) {
							// Pass the pixels to spout
							spoutsender.SendImage((unsigned char *)d3dlr.pBits, desc.Width, desc.Height, GL_BGRA_EXT);
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

	DrawDeck(); // Draw the image coming in

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
	// We only need an OpenGL context with no rendering window
	if(!m_hwnd || !IsWindow(m_hwnd)) {
		m_hwnd = CreateWindowA("BUTTON",
			            "VDJ Sender",
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


	return true;

}

