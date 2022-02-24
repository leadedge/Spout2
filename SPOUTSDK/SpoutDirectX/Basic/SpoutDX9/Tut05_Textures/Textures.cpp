//-----------------------------------------------------------------------------
//
// File: Textures.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT input (https://spout.zeal.co/)
// from C++ Direct3D samples provided with the Microsoft DirectX SDK (June 2010)
//
// https://www.microsoft.com/en-au/download/details.aspx?id=6812
//
// Search on "SPOUT" for additions.
//
// This is a receiver using methods directly from the Spout 2.007 SDK.
// Compare with a version using D3D11 and the SpoutDX support class.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Desc: Better than just lights and materials, 3D objects look much more
//       convincing when texture-mapped. Textures can be thought of as a sort
//       of wallpaper, that is shrinkwrapped to fit a texture. Textures are
//       typically loaded from image files, and D3DX provides a utility to
//       function to do this for us. Like a vertex buffer, textures have
//       Lock() and Unlock() functions to access (read or write) the image
//       data. Textures have a width, height, miplevel, and pixel format. The
//       miplevel is for "mipmapped" textures, an advanced performance-
//       enhancing feature which uses lower resolutions of the texture for
//       objects in the distance where detail is less noticeable. The pixel
//       format determines how the colors are stored in a texel. The most
//       common formats are the 16-bit R5G6B5 format (5 bits of red, 6-bits of
//       green and 5 bits of blue) and the 32-bit A8R8G8B8 format (8 bits each
//       of alpha, red, green, and blue).
//
//       Textures are associated with geometry through texture coordinates.
//       Each vertex has one or more sets of texture coordinates, which are
//       named tu and tv and range from 0.0 to 1.0. Texture coordinates can be
//       supplied by the geometry, or can be automatically generated using
//       Direct3D texture coordinate generation (which is an advanced feature).
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

// SPOUT
// Change paths as required
#include "..\SpoutDirectX9.h" // for D3D9ex and creating textures
#include "..\..\..\SpoutGL\SpoutSenderNames.h" // for sender creation and update
#include "..\..\..\SpoutGL\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\..\SpoutGL\SpoutUtils.h" // for logging utilites
#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

// SPOUT
spoutSenderNames spoutsender;
spoutDirectX9 spoutdx9;
spoutFrameCount frame;

LPDIRECT3DTEXTURE9 g_pSenderTexture = nullptr; // Copy of the sender's shared texture
HANDLE g_dxShareHandle = NULL; // Share handle of the sender's shared texture
char g_SenderName[256]; // Sender name
char g_SenderNameSetup[256]; // Sender name to connect to
unsigned int g_Width = 0; // Sender width
unsigned int g_Height = 0; // sender height
long g_senderframe = 0; // Sender frame number
double g_senderfps = 0.0; // Sender frame rate
bool bSpoutInitialized = false; // Initialized for the connected sender
bool bSpoutPanelOpened = false; // User opened sender selection panel
bool bSpoutPanelActive = false; // Selection panel is still open
SHELLEXECUTEINFOA g_ShExecInfo; // Global info so the exit code can be tested

// SPOUT utility functions
bool OpenSpoutPanel();
bool CheckSpoutPanel(char *sendername, int maxchars = 256);


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
// SPOUT - must used D3D9ex to share textures
IDirect3D9Ex*             g_pD3D = NULL; // Used to create the D3DDevice
IDirect3DDevice9Ex*       g_pd3dDevice = NULL; // Our rendering device
// LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
// LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // Our texture

// A structure for our custom vertex type. We added texture coordinates
struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; // The position
    D3DCOLOR color;    // The color
#ifndef SHOW_HOW_TO_USE_TCI
    FLOAT tu, tv;   // The texture coordinates
#endif
};

// Our custom FVF, which describes our custom vertex structure
#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
#else
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{

	//
	// SPOUT
	//

	// This application is D3D9 which does not support shared textures
	// Create the device with D3D9ex within the SpoutDirectX9 class.

	// Initialize DirectX9ex
	if (!spoutdx9.OpenDirectX9(hWnd)) {
		return E_FAIL;
	}
	// Use the class object and device
	g_pD3D = spoutdx9.GetDX9object();
	g_pd3dDevice = spoutdx9.CreateDX9device(g_pD3D, hWnd);

    /*
	// Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }
	*/

    // Turn off culling
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Create the textures and vertex buffers
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    // Use D3DX to create a texture from a file based image
    if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"banana.bmp", &g_pTexture ) ) )
    {
        // If texture is not in current folder, try parent folder
        if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"..\\banana.bmp", &g_pTexture ) ) )
        {
            MessageBox( NULL, L"Could not find banana.bmp", L"Textures.exe", MB_OK );
            return E_FAIL;
        }
    }

    // Create the vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 50 * 2 * sizeof( CUSTOMVERTEX ),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    CUSTOMVERTEX* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, ( void** )&pVertices, 0 ) ) )
        return E_FAIL;
    for( DWORD i = 0; i < 50; i++ )
    {
        FLOAT theta = ( 2 * D3DX_PI * i ) / ( 50 - 1 );

        pVertices[2 * i + 0].position = D3DXVECTOR3( sinf( theta ), -1.0f, cosf( theta ) );
        pVertices[2 * i + 0].color = 0xffffffff;
#ifndef SHOW_HOW_TO_USE_TCI
        pVertices[2 * i + 0].tu = ( ( FLOAT )i ) / ( 50 - 1 );
        pVertices[2 * i + 0].tv = 1.0f;
#endif

        pVertices[2 * i + 1].position = D3DXVECTOR3( sinf( theta ), 1.0f, cosf( theta ) );
        pVertices[2 * i + 1].color = 0xff808080;
#ifndef SHOW_HOW_TO_USE_TCI
        pVertices[2 * i + 1].tu = ( ( FLOAT )i ) / ( 50 - 1 );
        pVertices[2 * i + 1].tv = 0.0f;
#endif
    }
    g_pVB->Unlock();

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pTexture != NULL )
        g_pTexture->Release();

    if( g_pVB != NULL )
        g_pVB->Release();

	//
	// SPOUT
	//

	// Close the named texture access mutex
	frame.CloseAccessMutex();

	// Close frame counting
	frame.CleanupFrameCount();

	// Release application resources

	// Copy of the sender's shared texture
	if (g_pSenderTexture)
		g_pSenderTexture->Release();

	g_pSenderTexture = nullptr;
	g_dxShareHandle = NULL;

	// Close the class device
	spoutdx9.CloseDirectX9();
	g_pd3dDevice = NULL;
	g_pD3D = NULL;
	
	/*
	// Original code should not be used
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
	*/

}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // Set up world matrix
    D3DXMATRIXA16 matWorld;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixRotationX( &matWorld, timeGetTime() / 1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        // D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );
						// SPOUT
						// Make background different so that we can clearly see
						// the sender rendered on the rotating cylinder.
						D3DCOLOR_XRGB(128, 32, 0), 1.0f, 0);

	//
	// SPOUT
	//

	// Set the initial width and height to current globals.
	// New width and height are returned from the sender.
	unsigned int width = g_Width;
	unsigned int height = g_Height;

	// If SpoutPanel has been opened, the sender name could be different
	char sendername[256];
	if (CheckSpoutPanel(sendername)) {
		// Reset everything for a new sender name
		if (strcmp(sendername, g_SenderName) != 0) {
			if (bSpoutInitialized) {
				frame.CloseAccessMutex();
				frame.CleanupFrameCount();
				g_Width = 0;
				g_Height = 0;
				bSpoutInitialized = false;
			}
			strcpy_s(g_SenderName, 256, sendername);
		}
	}

	//
	// Find if the sender exists.
	//

	// For an empty name string, the active sender is returned if that exists.
	// Return the sender name, width, height, sharehandle and format.
	DWORD dwFormat = 0;
	if (spoutsender.FindSender(g_SenderName, width, height, g_dxShareHandle, dwFormat)) {

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Open a named mutex to control access to the sender's shared texture
			frame.CreateAccessMutex(g_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(g_SenderName);
			bSpoutInitialized = true;
		}

		// Check for size change
		if (g_Width != width || g_Height != height) {

			// Update globals for subsequent size checks
			g_Width = width;
			g_Height = height;

			// Access the sender shared texture
			// (See comments in the CheckTextureAccess function)
			if (frame.CheckTextureAccess()) {

				// Optionally check whether the sender has produced a new frame.
				// This is not required, but will avoid un-necessary processing.

				// This must be done within a sender mutex lock so that the sender will not write
				// to the texture and increment the count while a receiver is reading it.
				if (frame.GetNewFrame()) {

					// Here is where the sender's shared texture can be safely accessed.
					// In this example we will create a texture from the share handle.

					// Create a new texture using the share handle retrieved by FindSender above
					// This does not have to be shared but it doesn't matter if it is.
					if (g_pSenderTexture) g_pSenderTexture->Release();
					g_pSenderTexture = NULL;

					// Formats must match if receiving from a DX9 sender but only two are allowed
					// D3DFMT_A8R8G8B8 = 21
					// D3DFMT_X8R8G8B8 = 22;
					D3DFORMAT format = (D3DFORMAT)dwFormat;
					if (!(dwFormat == (DWORD)D3DFMT_A8R8G8B8 || dwFormat == (DWORD)D3DFMT_X8R8G8B8) )
						format = D3DFMT_A8R8G8B8;

					spoutdx9.CreateSharedDX9Texture(g_pd3dDevice,
						g_Width, g_Height,
						format, // Format compatible with DX11 and OpenGL senders
						g_pSenderTexture,
						g_dxShareHandle);

				} // New frame from the sender

				// Allow texture access even if it was not a new frame
				frame.AllowTextureAccess();

			} // Accessed sender's shared texture
		} // Retrieved sender shared texture pointer

		// A copy of the sender shared texture has been updated.
		// In this example, we use that texture for rendering below.

		// The receiver can also query the sender frame number and rate
		g_senderframe = frame.GetSenderFrame();
		g_senderfps = frame.GetSenderFps();

	} // Sender exists
	else {

		// A sender was not found or the connected sender closed

		// The receiving texture and the pointer to the sender's shared texture
		// could be released now but they are updated when connected to another sender.
		// They should be released when the program closes
		if (bSpoutInitialized) {

			// If a connecting name has been specified, reset the global name to it.
			// Otherwise zero the name to look for the active sender next time
			// (See "set the name of the sender" at the beginning)
			if (g_SenderNameSetup[0])
				strcpy_s(g_SenderName, 256, g_SenderNameSetup);
			else
				g_SenderName[0] = 0;

			// Zero width and height to ensure they are reset
			g_Width = 0;
			g_Height = 0;

			// Close the named access mutex and frame counting
			frame.CloseAccessMutex();
			frame.CleanupFrameCount();

			// Clear the sender texture copy so render uses the default
			if (g_pSenderTexture) g_pSenderTexture->Release();
			g_pSenderTexture = nullptr;
			g_dxShareHandle = NULL;

			// Initialize them again when a sender is found
			bSpoutInitialized = false;
		}
	}

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Setup the world, view, and projection matrices
        SetupMatrices();

        // Setup our texture. Using textures introduces the texture stage states,
        // which govern how textures get blended together (in the case of multiple
        // textures) and lighting information. In this case, we are modulating
        // (blending) our texture with the diffuse color of the vertices.

		//
		// SPOUT
		//

		// Use the sender's texture for rendering if it has been received
		if(g_pSenderTexture)
			g_pd3dDevice->SetTexture(0, g_pSenderTexture);
		else
			g_pd3dDevice->SetTexture( 0, g_pTexture );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

#ifdef SHOW_HOW_TO_USE_TCI
    // Note: to use D3D texture coordinate generation, use the stage state
	// D3DTSS_TEXCOORDINDEX, as shown below. In this example, we are using
	// the position of the vertex in camera space (D3DTSS_TCI_CAMERASPACEPOSITION)
	// to generate texture coordinates. Camera space is the vertex position
	// multiplied by the World and View matrices.  The tex coord index (TCI)  
	// parameters are passed into a texture transform, which is a 4x4 matrix  
	// which transforms the x,y,z TCI coordinates into tu, tv texture coordinates.

	// In this example, the texture matrix is setup to transform the input
	// camera space coordinates (all of R^3) to projection space (-1,+1) 
	// and finally to texture space (0,1).
	//    CameraSpace.xyzw = (input vertex position) * (WorldView)
	//    ProjSpace.xyzw = CameraSpace.xyzw * Projection           //move to -1 to 1
	//    TexSpace.xyzw = ProjSpace.xyzw * ( 0.5, -0.5, 1.0, 1.0 ) //scale to -0.5 to 0.5 (flip y)
	//    TexSpace.xyzw += ( 0.5, 0.5, 0.0, 0.0 )                  //shift to 0 to 1

	// Setting D3DTSS_TEXTURETRANSFORMFLAGS to D3DTTFF_COUNT4 | D3DTTFF_PROJECTED
	// tells D3D to divide the input texture coordinates by the 4th (w) component.
	// This divide is necessary when performing a perspective projection since
	// the TexSpace.xy coordinates prior to the homogeneous divide are not actually 
	// in the 0 to 1 range.
	D3DXMATRIXA16 mTextureTransform;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mTrans;
	D3DXMATRIXA16 mScale;

	g_pd3dDevice->GetTransform( D3DTS_PROJECTION, &mProj );
	D3DXMatrixTranslation( &mTrans, 0.5f, 0.5f, 0.0f );
	D3DXMatrixScaling( &mScale, 0.5f, -0.5f, 1.0f );
	mTextureTransform = mProj * mScale * mTrans;

	g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mTextureTransform );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4 | D3DTTFF_PROJECTED );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
    #endif

        // Render the vertex buffer contents
        g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
        g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2 );

        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {

		// SPOUT - RH click to open SpoutPanel
		case WM_RBUTTONDOWN:
			OpenSpoutPanel();
			break;

        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
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
	// SPOUT
	// This prevents the hourglass from getting stuck
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx( &wc );

		
	// Create the application's window
    HWND hWnd = CreateWindow( L"D3D Tutorial",
							  L"D3D Tutorial 05: Textures",
                              WS_OVERLAPPEDWINDOW, 0, 0, 512, 512,
                              NULL, NULL,
							  wc.hInstance, NULL );

	//
	// SPOUT
	//

	// Centre the window on the desktop work area
	RECT rc = { 0, 0, 512, 512 };
	GetWindowRect(hWnd, &rc);
	RECT WorkArea;
	int WindowPosLeft = 0;
	int WindowPosTop = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
	WindowPosLeft += ((WorkArea.right - WorkArea.left) - (rc.right - rc.left)) / 2;
	WindowPosTop += ((WorkArea.bottom - WorkArea.top) - (rc.bottom - rc.top)) / 2;
	MoveWindow(hWnd, WindowPosLeft, WindowPosTop, (rc.right - rc.left), rc.bottom - rc.top, false);

	//
	// SPOUT
	//

	// Initialize Spout variables
	g_SenderName[0] = 0;
	g_SenderNameSetup[0] = 0;
	g_pSenderTexture = nullptr;
	g_dxShareHandle = NULL;
	g_SenderName[0] = 0;
	g_SenderNameSetup[0] = 0;
	g_Width = 0;
	g_Height = 0;
	g_senderframe = 0;
	g_senderfps = 0.0;
	bSpoutInitialized = false;

	// Optionally enable Spout logging
	// OpenSpoutConsole(); // Console only without logs for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile("D3D9_Textures.log"); // Log to file
	// Default log file path is "C:\Users\username\AppData\Roaming\Spout\"
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // Show only warnings and errors

	// Optionally set the name of the sender to receive from
	// The receiver will only connect to that sender.
	// The user can over-ride this by selecting another.
	// See also for reset of name in render
	// strcpy_s(g_SenderNameSetup, 256, "Spout Demo Sender"); // Set the starting name
	// strcpy_s(g_SenderName, 256, "Spout Demo Sender"); // Set the general name as well

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the scene geometry
        if( SUCCEEDED( InitGeometry() ) )
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
				else {
					Render();
				}
            }
        }
    }

	// SPOUT
	// Cleanup() takes care of releasing resources when the window is closed

    UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}

//
// SPOUT
//
//
// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.
// They allow a receiver to open a dialog for sender selection.
//

//
// Pop up SpoutPanel to allow the user to select a sender
// activated by RH click in this application
//
bool OpenSpoutPanel()
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
		// Does SpoutPanel.exe exist in this path ?
		if (!PathFileExistsA(path)) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (!PathFileExistsA(path)) {
					SpoutLogWarning("Tutorial07::OpenSpoutPanel - SpoutPanel path not found");
					return false;
				}
			}
		}
	}

	// Check whether the panel is already running
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel"); // Try to open the application mutex.
	if (!hMutex1) {
		// No mutex, so not running, so can open it
		// Use ShellExecuteEx so we can test its return value later
		ZeroMemory(&g_ShExecInfo, sizeof(g_ShExecInfo));
		g_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		g_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		g_ShExecInfo.hwnd = NULL;
		g_ShExecInfo.lpVerb = NULL;
		g_ShExecInfo.lpFile = (LPCSTR)path;
		g_ShExecInfo.lpDirectory = NULL;
		g_ShExecInfo.nShow = SW_SHOW;
		g_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&g_ShExecInfo);
		Sleep(125); // allow time for SpoutPanel to open nominally 0.125s
		//
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		bSpoutPanelOpened = true;
	}
	else {
		// The mutex exists, so another instance is already running.
		// Find the SpoutPanel window and bring it to the top.
		// SpoutPanel is opened as topmost anyway but pop it to
		// the front in case anything else has stolen topmost.
		HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		if (hWnd && IsWindow(hWnd)) {
			SetForegroundWindow(hWnd);
			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		}
		else if (path[0]) {
			// If the window was not found but the mutex exists
			// and SpoutPanel is installed, it has crashed.
			// Terminate the process and the mutex or the mutex will remain
			// and SpoutPanel will not be started again.
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				SpoutLogError("Tutorial07::OpenSpoutPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("Tutorial07::OpenSpoutPanel - Process32First error");
					CloseHandle(hProcessSnap);
				}
				else {
					// Look through all processes
					while (hRes && !done) {
						int value = _tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
						if (value == 0) {
							HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
							if (hProcess != NULL) {
								// Terminate SpoutPanel and it's mutex
								TerminateProcess(hProcess, 9);
								CloseHandle(hProcess);
								done = true;
							}
						}
						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = NULL; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return true;

} // end OpenSpoutPanel

//
// Check whether SpoutPanel was opened and return the new sender name
//
bool CheckSpoutPanel(char *sendername, int maxchars)
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if (bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if (!bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if (hMutex) bSpoutPanelActive = true;
		}
		else if (!hMutex) { // It has now closed
			bSpoutPanelOpened = false; // Don't do this part again
			bSpoutPanelActive = false;
			// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
			// to get the exit code from SpoutPanel
			if (g_ShExecInfo.hProcess) {
				GetExitCodeProcess(g_ShExecInfo.hProcess, &dwExitCode);
				// Only act if exit code = 0 (OK)
				if (dwExitCode == 0) {
					// SpoutPanel has been activated and OK clicked
					// Test the active sender which should have been set by SpoutPanel
					newname[0] = 0;
					if (!spoutsender.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (spoutsender.getSharedInfo(newname, &TextureInfo)) {
									// Register in the list of senders and make it the active sender
									spoutsender.RegisterSenderName(newname);
									spoutsender.SetActiveSender(newname);
								}
							}
						}
					}
					// Now do we have a valid sender name ?
					if (newname[0] != 0) {
						// Pass back the new name
						strcpy_s(sendername, maxchars, newname);
						bRet = true;
					} // endif valid sender name
				} // endif SpoutPanel OK
			} // got the exit code
		} // endif no mutex so SpoutPanel has closed
		// If we opened the mutex, close it now or it is never released
		if (hMutex) CloseHandle(hMutex);
		return bRet;
	} // SpoutPanel has not been opened

	return false;
}

// The end ...

