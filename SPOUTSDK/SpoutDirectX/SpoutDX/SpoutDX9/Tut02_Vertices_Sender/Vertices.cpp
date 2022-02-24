//-----------------------------------------------------------------------------
//
// File: Vertices.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT input (https://spout.zeal.co/)
// from C++ Direct3D samples provided with the Microsoft DirectX SDK (June 2010)
// C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D\Tutorials
//
// This is a triangle vertices example modified for a quad
// so that a received texture can be displayed.
// Search on "SPOUT" for additions to your own application.
//
// This is a Spout sender using the SpoutDX9 support class :
//
//		bool spoutDX::SendDX9surface(IDirect3DSurface9* pSurface)
//
// See also the corresponding receiver example.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Desc: In this tutorial, we are rendering some vertices. This introduces the
//       concept of the vertex buffer, a Direct3D object used to store
//       vertices. Vertices can be defined any way we want by defining a
//       custom structure and a custom FVF (flexible vertex format). In this
//       tutorial, we are using vertices that are transformed (meaning they
//       are already in 2D window coordinates) and lit (meaning we are not
//       using Direct3D lighting, but are supplying our own colors).
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <d3d9.h>
#include <d3dx9.h>

// SPOUT
// Disable warnings for d3dx9 repeat definitions
#pragma warning( disable : 4005 )

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

// SPOUT
#include "..\SpoutDX9.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// SPOUT - must used D3D9ex to share textures
IDirect3D9Ex*             g_pD3D = NULL; // Used to create the D3DDevice
IDirect3DDevice9Ex*       g_pd3dDevice = NULL; // Our rendering device
// The original D3D9 versions are here for reference but should not be used
// LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
// LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
int g_Width = 0;
int g_Height = 0;

// SPOUT
spoutDX9 sender; // Sender object
LPDIRECT3DTEXTURE9 g_pTexture = nullptr; // Texture for display

// A structure for our vertex with texture coordinates
struct TexturedVertex {
	float x, y, z, rhw;  // The transformed(screen space) position for the vertex.
	float tu, tv;        // Texture coordinates
};

// Transformed vertex with 1 set of texture coordinates
#define D3DFVF_TRIVERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)
int g_ListCount = 0;

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{

	// ================================================================================

	//
	// SPOUT
	//

	// Spout logging options
	// OpenSpoutConsole(); // Console only for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile("Vertices DX9 Sender.log"); // Log to file
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors

	// Sender name option
	// Set the name of the sender here.
	// If not set, the executable name will be used.
	sender.SetSenderName("Vertices DX9 sender");

	// This application is D3D9 which does not support shared textures
	// Create the device with D3D9ex within the SpoutDirectX9 class.
	if (!sender.OpenDirectX9(hWnd))
		return E_FAIL;

	// Use the SpoutDX9 class object and device
	g_pD3D = sender.GetDX9object();
	g_pd3dDevice = sender.GetDX9device();

	// Load a texture for display
	D3DXCreateTextureFromFileA(g_pd3dDevice, "koala-on-tree.jpg", &g_pTexture);

	// ================================================================================


    // Device state

	// Turn off culling
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	// Turn off D3D lighting
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	// Turn off the zbuffer for the quad draw
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitVB()
// Desc: Creates a vertex buffer and fills it with our vertices. The vertex
//       buffer is basically just a chuck of memory that holds vertices. After
//       creating it, we must Lock()/Unlock() it to fill it. For indices, D3D
//       also uses index buffers. The special thing about vertex and index
//       buffers is that they can be created in device memory, allowing some
//       cards to process them in hardware, resulting in a dramatic
//       performance gain.
//-----------------------------------------------------------------------------
HRESULT InitVB()
{

	// x, y, z, rhw;  // The transformed(screen space) position for the vertex.
	// tu, tv;        // Texture coordinates
	float w = (float)g_Width;
	float h = (float)g_Height;
	TexturedVertex data[] = {
		{0, h, 1, 1, 0, 1}, {0, 0, 1, 1, 0, 0}, {w, 0, 1, 1, 1, 0},
		{0, h, 1, 1, 0, 1}, {w, 0, 1, 1, 1, 0}, {w, h, 1, 1, 1, 1}
	};

	int vert_count = sizeof(data) / sizeof(TexturedVertex);
	int byte_count = vert_count * sizeof(TexturedVertex);
	void *vb_vertices;
	HRESULT hr;

	g_ListCount = vert_count / 3;

	hr = g_pd3dDevice->CreateVertexBuffer(byte_count, // Length
		D3DUSAGE_WRITEONLY, // Usage
		D3DFVF_TRIVERTEX,   // FVF
		D3DPOOL_DEFAULT,    // Pool 
		&g_pVB,             // ppVertexBuffer
		NULL);              // Handle
	if (FAILED(hr)) {
		printf("Error Creating vertex buffer %d\n", hr);
		return hr;
	}

	hr = g_pVB->Lock(0, // Offset
		0,              // SizeToLock
		&vb_vertices,   // Vertices
		0);             // Flags
	if (FAILED(hr)) {
		printf("Error Locking vertex buffer %d\n", hr);
		return hr;
	}

	memcpy(vb_vertices, data, byte_count);

	g_pVB->Unlock();

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pVB != NULL )
        g_pVB->Release();

	//
	// SPOUT
	//

	// Release sender and resources
	sender.ReleaseDX9sender();

	// Release the display texture
	if (g_pTexture)
		g_pTexture->Release();

	// Close the SpoutDX9 class device
	sender.CloseDirectX9();
	g_pd3dDevice = NULL;
	g_pD3D = NULL;

}



//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer to black
    // LJ DEBUG 
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );
	// D3DFMT_A8R8G8B8
	// g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0xff), 1.0f, 0);

	// Use the sender's texture for rendering
	if (g_pTexture) {

		// Begin the scene
		if (SUCCEEDED(g_pd3dDevice->BeginScene()))
		{
			// Setup our texture.
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);   //Ignored

			g_pd3dDevice->SetTexture(0, g_pTexture);

			g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(TexturedVertex));
			g_pd3dDevice->SetFVF(D3DFVF_TRIVERTEX);
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, g_ListCount);

			// End the scene
			g_pd3dDevice->EndScene();
		}

	}

	//
	// SPOUT
	//

	// Get the swap chain's backbuffer surface
	IDirect3DSurface9* pBackBuffer = nullptr;
	HRESULT hr = g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	if (SUCCEEDED(hr)) {
		//
		// Send a texture surface
		//
		// SendDX9surface handles sender creation and resizing
		//
		// Sender details can be retrieved with :
		//		const char * GetName();
		//		unsigned int GetWidth();
		//		unsigned int GetHeight();
		//		DWORD GetFormat();
		//		HANDLE GetHandle();
		//		long GetFrame();
		//		double GetFps();
		//
		sender.SendDX9surface(pBackBuffer);

		pBackBuffer->Release();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
		case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


//-----------------------------------------------------------------------------
// Name: wWinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
	// This prevents the hourglass from getting stuck
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx( &wc );

    // Create the application's window
    // SPOUT - modified for 640x360 starting client size
	g_Width = 640;
	g_Height = 360;
	RECT rc = { 0, 0, g_Width, g_Height }; // Desired client size
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWnd = CreateWindow( L"D3D Tutorial",
							L"D3D Tutorial 02: Vertices - Spout sender",
							WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX,
   							CW_USEDEFAULT, CW_USEDEFAULT,
							rc.right - rc.left, rc.bottom - rc.top,
							nullptr, nullptr, wc.hInstance, nullptr);

	// Centre the window on the desktop work area
	GetWindowRect(hWnd, &rc);
	RECT WorkArea;
	int WindowPosLeft = 0;
	int WindowPosTop = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
	WindowPosLeft += ((WorkArea.right - WorkArea.left) - (rc.right - rc.left)) / 2;
	WindowPosTop += ((WorkArea.bottom - WorkArea.top) - (rc.bottom - rc.top)) / 2;
	MoveWindow(hWnd, WindowPosLeft, WindowPosTop, (rc.right - rc.left), rc.bottom - rc.top, false);

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the vertex buffer
        if( SUCCEEDED( InitVB() ) )
        {
            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof( msg ) );
            while( msg.message != WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                    Render();
            }
        }
    }
    UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}

// That's all ...