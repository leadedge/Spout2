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
//		15.10.15	Remove copy/paste error in draw
//					Changed to /MT compile
//					Recompile for 2.005
//					Version 1.05
//		19.10.15	Complete re-write - DirectX version
//		07.11.15	Allowed format change to allow receive of X8R8G8B8 textures from Milkdrop
//					VDJ <> Milkdrop is DX9 only, so must use DX9 for all apps
//		04.12.15	Cleanup
//		06.12.15	Version 1.06
//		07.12.15	Release offscreen surface before equating to resolved surface
//					Gave problems with Windows 7 32bit but not 64bit and only for AOV mode
//				    Cleanup of surface naming and unused variables
//					Version 1.07
//		08.12.15	info flag VDJFLAG_VIDEO_VISUALISATION only
//					Version 1.07b
//		11.12.15	Added release function as per examples - checked that it was previously called
//					Version 1.07c
//		14.12.15	Used UpdateSurface instead of  D3DXLoadSurfaceFromSurface
//					Removed dependency on D3dx9.lib
//		15.12.15	Rebuild for Spout 2.005 release
//					Version 1.08
//		18.02.16	Added an optional define "SPOUTEFFECT" to compile as an effect plugin
//					Should be copied to the "Plugins\VideoEffect" folder as "VDJSpoutEffect"
//					Version 1.09
//		23.06.16 - rebuild for Spout 2.005 release Version 1.10
//				   VS2012 /MT
//		23.01.17 - Rebuild for 2.006 VS2012 /MD - Version 1.11
//		08.02.17 - Reset closing flag on InitDevice - Version 1.12
//
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2017. Lynn Jarvis, Leading Edge. Pty. Ltd.
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
// #define SPOUTEFFECT

#include "stdafx.h"
#include "VDJSpoutReceiver.h"

VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// VDJ 8 only
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
	printf("VDJSpoutReceiver - 1.12\n");
	*/

	// DirectX9
	m_VDJ_device = NULL;
	m_VDJ_texture = NULL;

	// SPOUT variables and functions
	m_Width  = 0; // global width and height of the VirtualDJ texture
	m_Height = 0; // and also the size of the local shadowing OpenGL texture
	m_SenderWidth = 0; // Width and height of the sender detected
	m_SenderHeight = 0;
	SenderName[0] = 0;
	bInitialized = false;
	bSpoutOut = false; // toggle for plugin start and stop
	bUseActive = true; // use active sender
	bIsClosing = false; // is not closing

	m_pD3D = NULL;
	m_pDevice = NULL;
	m_dxSpoutTexture = NULL;
	m_dxTexture = NULL;

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

#ifdef SPOUTEFFECT
	infos->PluginName = (char *)"VDJSpoutEffect";
	infos->Description = (char *)"Receives frames from a Spout Sender\nas an effect plugin\nSpout : http://Spout.zeal.co/";
#else
	infos->PluginName = (char *)"VDJSpoutReceiver";
	infos->Description = (char *)"Receives frames from a Spout Sender\nas a visualisation plugin\nSpout : http://Spout.zeal.co/";
#endif

	infos->Version = (char *)"v1.12n";
    infos->Bitmap = NULL;

#ifndef SPOUTEFFECT
	infos->Flags = VDJFLAG_VIDEO_VISUALISATION;
#endif
    return NO_ERROR;
}


HRESULT __stdcall SpoutReceiverPlugin::OnStart()
{
	bSpoutOut = true;
	return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnStop()
{
	bSpoutOut = false;
	return NO_ERROR;
}

// When DirectX/OpenGL is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutReceiverPlugin::OnDeviceInit() 
{
	bIsClosing = false; // is not closing
	return S_OK;
}

HRESULT __stdcall SpoutReceiverPlugin::OnDeviceClose() 
{
	bIsClosing = true;
	if(m_dxTexture != NULL) m_dxTexture->Release();
	if(m_dxSpoutTexture != NULL) m_dxSpoutTexture->Release();
	if (m_pDevice != NULL) m_pDevice->Release();
	if (m_pD3D != NULL) m_pD3D->Release();
	
	m_pD3D = NULL;
	m_pDevice = NULL;
	m_dxTexture = NULL;
	m_dxSpoutTexture = NULL;

	bInitialized = false;

	return S_OK;
}

// added 11.12.15 
// Confirmed that it is called in vdjPlugin8.h
ULONG __stdcall SpoutReceiverPlugin::Release()
{
	delete this; 
	return S_OK;
}


HRESULT __stdcall SpoutReceiverPlugin::OnParameter(int ParamID) 
{
	// Activate SpoutPanel to select a sender
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

	IDirect3DSurface9 * VDJTextureSurface = NULL; // Surface derived from the texture
	IDirect3DSurface9 * OffscreenSurface = NULL; // System memory surface for copying
	IDirect3DSurface9 * SharedTextureSurface = NULL;
	IDirect3DSurface9 * SourceSurface = NULL;
	IDirect3DSurface9 * ResolvedSurface = NULL;
	IDirect3DSurface9 * ShadowTextureSurface = NULL;

	D3DLOCKED_RECT d3dlr; // LockRect for data transfer
	D3DLOCKED_RECT d3dlr2; // LockRect for data transfer
	D3DSURFACE_DESC desc; // Texture description

	if(bIsClosing) {
		return S_FALSE;
	}

	// Local D3D9ex device for receiving a shared texture and copying
	if(m_pDevice == NULL) {
		if(!InitD3DEx(NULL)) {
			return S_FALSE;
		}
	}

	GetDevice(VdjVideoEngineDirectX9, (void **)&m_VDJ_device);
	if(m_VDJ_device != NULL) {
		
		// Get the Virtual DJ texture and description
		GetTexture(VdjVideoEngineDirectX9, (void **)&m_VDJ_texture, &vertices);
		m_VDJ_texture->GetLevelDesc(0, &desc);

		if(bSpoutOut) { // The plugin has started	

			// Check for VirtualDJ texture size change compared to the local shadow DirectX texture
			if(m_Width != desc.Width || m_Height != desc.Height || m_dxTexture == NULL) {
				hShareHandle = NULL; // a new texture rather than a copy of a shared one
				CreateDX9exTexture(m_pDevice, desc.Width, desc.Height, D3DFMT_A8R8G8B8, m_dxTexture, hShareHandle);
				m_Width  = desc.Width;
				m_Height = desc.Height;
				return S_FALSE;
			}

			// Find a sender and connect when it is found
			if(bInitialized == false) {
				if(spoutreceiver.GetActiveSender(activesender)) {
					// Find the information about the sender
					spoutreceiver.GetSenderInfo(activesender, m_SenderWidth, m_SenderHeight, hShareHandle, dwFormat);
					// Create a receiver using the active sender
					strcpy_s(SenderName, 256, activesender);
					bInitialized = true;
				} // found active sender
				return S_FALSE; // do no more for this frame
			} // was not initialized
		
			// Receive the sender shared texture
			width  = m_SenderWidth;
			height = m_SenderHeight;

			// Check sender presence and create a local shared texture from the sender's sharehandle
			if(!ReceiveTexture(SenderName, width, height)) {
				return S_FALSE;
			}

			// Now we have a local D3D9ex copy of the Spout shared texture (m_dxSpoutTexture)
			// and a local D3D9ex texture (m_dxTexture) the same size as the VirtualDJ texture
		
			// The local spout shared texture surface (D3D9ex m_pDevice)
			hr = m_dxSpoutTexture->GetSurfaceLevel(0, &SharedTextureSurface);
			if(SUCCEEDED(hr)) {
				// The local empty shadow VDJ texture surface (D3D9ex m_pDevice)
				hr = m_dxTexture->GetSurfaceLevel(0, &ShadowTextureSurface);
				if(SUCCEEDED(hr)) {
					// Copy from the larger spout shared texture surface to the smaller texture shadowing the VDJ texture
					hr = m_pDevice->StretchRect( SharedTextureSurface, NULL, ShadowTextureSurface, NULL, D3DTEXF_NONE );
					if(SUCCEEDED(hr)) {
						// The below seems necessary or getrendertargetdata fails
						// Usually used for resolving multi-sampled (anitaliased)
						ShadowTextureSurface->GetDesc( &desc );
						hr = m_pDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &ResolvedSurface, NULL);
						if(SUCCEEDED(hr)) {
							hr = m_pDevice->StretchRect(ShadowTextureSurface, NULL, ResolvedSurface, NULL, D3DTEXF_NONE );
							if(SUCCEEDED(hr)) {

								// ShadowTextureSurface surface needs to be released first or it is not
								// finally released and affects video memory availability for texture creation.
								if(ShadowTextureSurface != NULL) ShadowTextureSurface->Release();
								ShadowTextureSurface = ResolvedSurface;

								// Then use GetRenderTargetData to copy texture data from device memory to system memory
								hr = m_pDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &SourceSurface, NULL);
								if(SUCCEEDED(hr)) {
									// Copy the render-target data from device memory to system memory (SourceSurface).
									hr = m_pDevice->GetRenderTargetData(ShadowTextureSurface, SourceSurface);
									if(SUCCEEDED(hr)) {
										// Create a plain offscreen surface using the VDJ D3D device
										// and copy between the surfaces created on the different devices
										m_VDJ_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &OffscreenSurface, NULL);
										hr = OffscreenSurface->LockRect(&d3dlr2, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_DISCARD);
										if(SUCCEEDED(hr)) {
											hr = SourceSurface->LockRect(&d3dlr, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_DISCARD); 
											if(SUCCEEDED(hr)) {
												memcpy((void *)d3dlr2.pBits, (void *)d3dlr.pBits, desc.Width*desc.Height*4);
												SourceSurface->UnlockRect();
												OffscreenSurface->UnlockRect();
												// Now copy to the VDJtexture surface
												hr = m_VDJ_texture->GetSurfaceLevel(0, &VDJTextureSurface);
												// https://msdn.microsoft.com/en-us/library/windows/desktop/bb172904%28v=vs.85%29.aspx 
												// If D3DXLoadSurfaceFromSurface is called and the surface was not already dirty 
												// (this is unlikely under normal usage scenarios), the application needs to
												// explicitly call AddDirtyRect on the surface.
												// D3DXLoadSurfaceFromSurface(VDJTextureSurface, NULL, NULL, OffscreenSurface, NULL, NULL, D3DTEXF_NONE, 0);
												// UpdateSurface
												// https://msdn.microsoft.com/en-us/library/windows/desktop/bb205857%28v=vs.85%29.aspx
												//    The source surface must have been created with D3DPOOL_SYSTEMMEM.
												//    The destination surface must have been created with D3DPOOL_DEFAULT.
												//    Neither surface can be locked or holding an outstanding device context.
												m_VDJ_device->UpdateSurface(OffscreenSurface, NULL, VDJTextureSurface, NULL);
											}
											else {
												OffscreenSurface->UnlockRect();
											}
										} // OffscreenSurface->LockRect
									} // m_pDevice->GetRenderTargetData
								} // m_pDevice->CreateOffscreenPlainSurface
							} // m_pDevice->StretchRect
						} // m_pDevice->CreateRenderTarget
					} // m_pDevice->StretchRect
				} // m_dxTexture->GetSurfaceLevel
			} // m_dxSpoutTexture->GetSurfaceLevel
		} // plugin has not started
	} // endif device

	if(VDJTextureSurface != NULL) VDJTextureSurface->Release();
	if(OffscreenSurface != NULL) OffscreenSurface->Release();
	if(SourceSurface != NULL) SourceSurface->Release();
	if(ResolvedSurface != NULL) ResolvedSurface->Release();
	if(ShadowTextureSurface != NULL) ShadowTextureSurface->Release(); // already released by ResolvedSurface?
	if(SharedTextureSurface != NULL) SharedTextureSurface->Release();
	VDJTextureSurface = NULL;
	OffscreenSurface = NULL;
	SourceSurface = NULL;
	ResolvedSurface = NULL;
	ShadowTextureSurface = NULL;
	SharedTextureSurface = NULL;

	return S_FALSE;

}


bool SpoutReceiverPlugin::ReceiveTexture(char* name, unsigned int &width, unsigned int &height) // , GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	char activename[256];
	unsigned int newWidth, newHeight;
	DWORD dwFormat;
	HANDLE hShareHandle = NULL;

	// Has the user used SpoutPanel ?
	if(spoutreceiver.spout.CheckSpoutPanel()) {
		// Get the active sender
		spoutreceiver.GetActiveSender(activename);
		spoutreceiver.GetSenderInfo(activename, newWidth, newHeight, hShareHandle, dwFormat);
		if(CreateDX9exTexture(m_pDevice, newWidth, newHeight, D3DFMT_A8R8G8B8, m_dxSpoutTexture, hShareHandle)) {
			// Pass back the new current name and size
			strcpy_s(name, 256, activename);
			m_SenderWidth  = newWidth;
			m_SenderHeight = newHeight;
			return true;
		}
	}

	// The name passed is the name to try to connect to unless the bUseActive flag is set
	// or the name is not initialized in which case it will try to find the active sender
	// Width and height are passed back as well
	char newname[256];
	if(name[0] != 0) strcpy_s(newname, 256, name);
	else newname[0] = 0;

	// Test to see whether the current sender is still there
	if(spoutreceiver.spout.interop.senders.GetSenderInfo(newname, newWidth, newHeight, hShareHandle, dwFormat)) {
		// The current sender still exists, but has the width, height, texture format changed
		if(newWidth > 0 && newHeight > 0) {
			if(newWidth  != m_SenderWidth 
			|| newHeight != m_SenderHeight
			|| dwFormat  != dwFormat
			|| strcmp(name, SenderName) != 0 ) {
				// Update the local dx texture
				 if(CreateDX9exTexture(m_pDevice, width, height, D3DFMT_A8R8G8B8, m_dxSpoutTexture, hShareHandle)) {
					// Pass back the new current name and size
					strcpy_s(name, 256, newname);
					m_SenderWidth  = newWidth;
					m_SenderHeight = newHeight;
					return true;
				}
			} // width, height, format or name have changed
		} // width and height are zero
	} // endif GetSenderInfo found the sender
	else {
		return false;
	} // Could not find the sender - probably closed

	// Sender exists and everything matched.
	// Globals are now all current, so pass back the current name and size
	// so that there is no change found by the host.
	strcpy_s(name, 256, SenderName);
	width  = m_SenderWidth;
	height = m_SenderHeight;

	// The shared texture will have been updated by the sender
	// Copy the Spout shared DirectX texture to the local spout shadow texture
	return (CreateDX9exTexture(m_pDevice, width, height, D3DFMT_A8R8G8B8, m_dxSpoutTexture, hShareHandle));

} // end ReceiveTexture



bool SpoutReceiverPlugin::CreateDX9exTexture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle)
{

	if(width == 0 || height == 0) return false;
	if(m_pDevice == NULL) {
		return false; // Global D3D9ex device
	}

	if(dxTexture != NULL) {
		dxTexture->Release();
	}

	HRESULT res = pDevice->CreateTexture(width,
										 height,
										 1,
										 D3DUSAGE_RENDERTARGET, 
										 format,
										 D3DPOOL_DEFAULT,
										 &dxTexture,
										 &dxShareHandle);
	if ( res != D3D_OK ) {
		// printf("CreateDX9exTexture error : ");
		switch (res) {
			case D3DERR_INVALIDCALL:
				// The method call is invalid. For example, a method's parameter may not be a valid pointer.
				// printf("    D3DERR_INVALIDCALL \n");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				printf("    D3DERR_OUTOFVIDEOMEMORY \n");
				break;
			case E_OUTOFMEMORY:
				printf("    E_OUTOFMEMORY \n");
				break;
			default :
				printf("    Unknown error\n");
				break;
		}
		return false;
	}

	return true;

}


// In 2 parts as copied from the Spout SDK - TODO : simplify
bool SpoutReceiverPlugin::CreateReceiver(char* sendername, unsigned int &width, unsigned int &height, bool bActive)
{

	char UserName[256];
	UserName[0] = 0; // OK to do this internally

	// Use the active sender if the user wants it or the sender name is not set
	if(bActive || sendername[0] == 0) {		
		bUseActive = true;
	}
	else {
		// Try to find the sender with the name sent or over-ride with user flag
		strcpy_s(UserName, 256, sendername);
		bUseActive = false; // set global flag to use the active sender or not
	}

	// Make sure it has been initialized
	if(OpenReceiver(UserName, width, height)) {
		strcpy_s(sendername, 256, UserName); // pass back the sendername used
		return true;
	}

	return false;
}



bool SpoutReceiverPlugin::OpenReceiver (char* theName, unsigned int& theWidth, unsigned int& theHeight)
{
	char sendername[256]; // user entered Sender name
	DWORD dwFormat = 0;
	HANDLE sharehandle = NULL;
	unsigned int width;
	unsigned int height;

	// If the name begins with a null character, or the bUseActive flag has been set
	if(theName[0] != 0 && !bUseActive) { // A valid name is sent and the user does not want to use the active sender
		strcpy_s(sendername, 256, theName);
	}
	else {
		sendername[0] = 0;
	}

	// Set initial size to that passed in
	width  = theWidth;
	height = theHeight;

	// Find if the sender exists
	// Or, if a null name given, return the active sender if that exists
	if(!spoutreceiver.spout.interop.senders.FindSender(sendername, width, height, sharehandle, dwFormat)) {
	    return false;
	}

	// Set globals here
	m_Width       = width;
	m_Height      = height;
	strcpy_s(SenderName, 256, sendername);
	bInitialized = true;

	return true;

} // end OpenReceiver


// this function initializes and prepares Direct3D Ex
bool SpoutReceiverPlugin::InitD3DEx(HWND hWnd)
{
	HRESULT res;
	D3DPRESENT_PARAMETERS d3dpp;
	D3DCAPS9 d3dCaps;

	res = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D);
	if ( res != D3D_OK ) {
		printf("Direct3DCreate9Ex failed\n");
		return false;
	}

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd; // this cannot be null if m_hWnd is NULL
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// some dummy resolution - we don't render anything
    d3dpp.BackBufferWidth = 1920;
    d3dpp.BackBufferHeight = 1080;
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.BackBufferCount		 = 1;

	// Test for hardware vertex processing capability and set up as needed
	// D3DCREATE_MULTITHREADED required by interop spec
	DWORD dwBehaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED; 

	if(m_pD3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, &d3dCaps) != S_OK ) {
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else {
		if ( d3dCaps.VertexProcessingCaps != 0 )
			dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	res = m_pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
					  dwBehaviorFlags,
                      &d3dpp,
					  NULL,
                      &m_pDevice);

	if ( res != D3D_OK ) {
		printf("Failed to create DX9EX Device\n");
		return false;
	}
	
	return true;

} // InitD3DEx

