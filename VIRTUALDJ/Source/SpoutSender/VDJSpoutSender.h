#ifndef __spoutSenderPlugin__
#define __spoutSenderPlugin__

#include "stdafx.h"
#include "vdjVideo8.h"

//
// Include the entire SDK in the project because we initialize
// OpenGL in this app and use the OpenGL functions of Spout
// An OpenGL device is not returned by the GetDevice functions
// because the Windows version of VDJ is DirectX 9
//
// Libraries needed :
//
//	opengl32.lib
//	glu32.lib
//	Winmm.lib
//	d3d11.lib
//	D3dx9.lib
//
//	DirectX headers and libs from DirectX SDK 2010
//
#include "..\..\SpoutSDK3\Spout.h"

class SpoutSenderPlugin : public IVdjPluginVideoFx8
{

public:

    SpoutSenderPlugin();
    ~SpoutSenderPlugin();
    
    HRESULT __stdcall OnLoad();
    HRESULT __stdcall OnGetPluginInfo(TVdjPluginInfo8 *infos);
	HRESULT __stdcall OnStart();
	HRESULT __stdcall OnStop();
	HRESULT __stdcall OnDraw();
	HRESULT __stdcall OnDeviceInit();
	HRESULT __stdcall OnDeviceClose();
	ULONG   __stdcall Release();

private:

  	// DirectX9
	HRESULT hr;
	IDirect3DDevice9* d3d_device;
	LPDIRECT3DTEXTURE9 dxTexture;
	IDirect3DSurface9* texture_surface;
	LPDIRECT3DSURFACE9 source_surface;
	D3DLOCKED_RECT d3dlr;
	D3DSURFACE_DESC desc;

	// SPOUT variables and functions
	unsigned int m_Width; // global width and height to test for change
	unsigned int m_Height;
	bool bInitialized; // did Spout initialization work ?
	bool bOpenGL; // OpenGL is available
	bool bSpoutOut; // Spout output on or off when plugin is started and stopped
	SpoutSender spoutsender; // VDJ Spout plugin is a sender
	char SenderName[256]; // The sender name

	// OpenGL globals
	HWND m_hwnd;
	HDC m_hdc;
	HGLRC m_hRC; // primary rendering context
	HGLRC m_hSharedRC; // draw shared context

	// OpenGL functions
	bool StartOpenGL();
	bool InitOpenGL();

};

#endif
