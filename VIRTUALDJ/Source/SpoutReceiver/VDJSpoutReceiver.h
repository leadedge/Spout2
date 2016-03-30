#ifndef __spoutReceiverPlugin__
#define __spoutReceiverPlugin__

#include "stdafx.h"
#include "vdjVideo8.h"
#include "vdjPlugin8.h"

//
// Libraries needed :
//	Winmm.lib
//	d3d11.lib
//	D3dx9.lib
//	DirectX headers and libs from DirectX SDK 2010
//

#define DIRECT3D_VERSION 0x9000
#include "..\..\SpoutSDK\Spout.h"
#include <d3d9.h>
#include <d3d10.h>
#include <dxgi.h>

// #include <d3dx9.h> // Needed for D3DXLoadSurfaceFromSurface() 
// #pragma comment(lib, "d3dx9.lib") 

// VDJ VideoFx plugin class
class SpoutReceiverPlugin : public IVdjPluginVideoFx8
{

public:

    SpoutReceiverPlugin();
    ~SpoutReceiverPlugin();
    HRESULT __stdcall OnLoad();
    HRESULT __stdcall OnGetPluginInfo(TVdjPluginInfo8 *infos);
	HRESULT __stdcall OnStart();
	HRESULT __stdcall OnStop();
	HRESULT __stdcall OnDraw();

	// When DirectX is initialized or closed, these functions will be called
	HRESULT __stdcall OnDeviceInit();
	HRESULT __stdcall OnDeviceClose();
	ULONG   __stdcall Release(); // added 11.12.15 - not really necessary
	HRESULT __stdcall OnParameter(int id);

private:

	// Select sender parameter
	int SelectButton;

  	// DirectX9
	HRESULT hr;
	IDirect3DDevice9* m_VDJ_device; // VirtualDJ DirectX device
	LPDIRECT3DTEXTURE9 m_VDJ_texture; // The Virtual DJ texture

	// SPOUT variables and functions
	SpoutReceiver spoutreceiver;

	unsigned int m_Width; // global width and height of the VirtualDJ texture
	unsigned int m_Height;

	unsigned int m_SenderWidth; // Width and height of the sender detected
	unsigned int m_SenderHeight;

	bool bInitialized; // did Spout initialization work ?
	bool bSpoutOut; // Spout output on or off when plugin is started and stopped
	bool bUseActive; // Use the active sender
	bool bIsClosing;

	char SenderName[256]; // The sender name
	char ReceivedName[256];
	char activesender[256]; // The name of any Spout sender being received

	// DirectX9Ex
	IDirect3D9Ex *m_pD3D;
	IDirect3DDevice9Ex *m_pDevice;
	LPDIRECT3DTEXTURE9 m_dxTexture; // Pointer to DirectX 9 texture to shadow the VirtualDJ directX texture
	LPDIRECT3DTEXTURE9 m_dxSpoutTexture; // Pointer to DirectX 9 texture to shadow the Spout shared directX texture
	
	// Utility functions
	bool InitD3DEx(HWND hWnd);
	bool CreateDX9exTexture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle);

	bool CreateReceiver(char* sendername, unsigned int &width, unsigned int &height, bool bActive);
	bool OpenReceiver (char* theName, unsigned int& theWidth, unsigned int& theHeight);
	// bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO=0);
	bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height);

};

#endif
