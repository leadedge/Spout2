#ifndef __spoutReceiverPlugin__
#define __spoutReceiverPlugin__

#include "stdafx.h"
#include "vdjVideo8.h"
#include "vdjPlugin8.h"

//
// Include the entire SDK in the project because we initialize
// OpenGL in this app and use the OpenGL functions of Spout
// The OpenGL device is not returned by the GetDevice functions
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

#define DIRECT3D_VERSION 0x9000
#include <d3dx9.h> // Needed for D3DXLoadSurfaceFromSurface() 
#include "..\..\SpoutSDK\Spout.h"

#pragma comment(lib, "d3dx9.lib") 

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
	HRESULT __stdcall  OnDeviceInit();

	HRESULT __stdcall  OnDeviceClose();

	HRESULT __stdcall OnParameter(int id);


private:

	// Select sender parameter
	int SelectButton;

  	// DirectX9
	HRESULT hr;
	IDirect3DDevice9* d3d_device; // VirtualDJ DirectX device
	LPDIRECT3DTEXTURE9 dxTexture; // The Virtual DJ texture
	LPDIRECT3DSURFACE9 TextureSurface; // Surface derived from the texture
	LPDIRECT3DSURFACE9 SourceSurface; // System memory surface for copying
	D3DLOCKED_RECT d3dlr; // LockRect for data transfer
	D3DSURFACE_DESC desc; // Texture description

	// SPOUT variables and functions
	SpoutReceiver spoutreceiver;

	unsigned int m_Width; // global width and height of the VirtualDJ texture
	unsigned int m_Height;

	unsigned int m_SenderWidth; // Width and height of the sender detected
	unsigned int m_SenderHeight;

	bool bInitialized; // did Spout initialization work ?
	bool bSpoutOut; // Spout output on or off when plugin is started and stopped
	bool bOpenGL; // OpenGL is initialized OK
	char SenderName[256]; // The sender name
	char ReceivedName[256];
	char activesender[256]; // The name of any Spout sender being received

	// OpenGL globals
	HWND m_hwnd;
	HDC m_hdc;
	char windowtitle[256];
	HGLRC m_hRC; // primary rendering context
	HGLRC m_hSharedRC; // draw shared context
	GLuint m_fbo; // FBO for texture transfers
	GLuint m_GLtextureVJ; // OpenGL texture to shadow the VirtualDJ directX texture

	// OpenGL functions
	bool StartOpenGL();
	bool InitOpenGL();
	bool InitGLtexture(GLuint &texID, unsigned int width, unsigned int height);
	void GLerror(char *intext);
	void GLfboError(GLenum status);

};

#endif
