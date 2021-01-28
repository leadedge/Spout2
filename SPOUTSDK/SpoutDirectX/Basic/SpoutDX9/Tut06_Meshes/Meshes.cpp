//-----------------------------------------------------------------------------
//
// File: Meshes.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT input (https://spout.zeal.co/)
// from C++ Direct3D samples provided with the Microsoft DirectX SDK (June 2010)
//
// https://www.microsoft.com/en-au/download/details.aspx?id=6812
//
// Search on "SPOUT" for additions.
//
// This is a sender using methods directly from the Spout 2.007 SDK.
// Compare with a version using D3D11 and the SpoutDX support class.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Desc: For advanced geometry, most apps will prefer to load pre-authored
//       meshes from a file. Fortunately, when using meshes, D3DX does most of
//       the work for this, parsing a geometry file and creating vertx buffers
//       (and index buffers) for us. This tutorial shows how to use a D3DXMESH
//       object, including loading it from a file and rendering it. One thing
//       D3DX does not handle for us is the materials and textures for a mesh,
//       so note that we have to handle those manually.
//
//       Note: one advanced (but nice) feature that we don't show here is that
//       when cloning a mesh we can specify the FVF. So, regardless of how the
//       mesh was authored, we can add/remove normals, add more texture
//       coordinate sets (for multi-texturing), etc.
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

// SPOUT
spoutSenderNames sender;
spoutDirectX9 spoutdx9;
spoutFrameCount frame;

char g_SenderName[256];
unsigned int g_Width = 0;
unsigned int g_Height = 0;
LPDIRECT3DTEXTURE9 g_pSharedTexture = nullptr; // Texture to be shared
HANDLE g_dxShareHandle = NULL; // Share handle for the sender
bool bSpoutInitialized = false;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
// SPOUT - must used D3D9ex to share textures
IDirect3D9Ex*             g_pD3D = NULL; // Used to create the D3DDevice
IDirect3DDevice9Ex*       g_pd3dDevice = NULL; // Our rendering device
// LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
// LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device

LPD3DXMESH          g_pMesh = NULL; // Our mesh object in sysmem
D3DMATERIAL9*       g_pMeshMaterials = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL; // Textures for our mesh
DWORD               g_dwNumMaterials = 0L;   // Number of mesh materials


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
	g_pd3dDevice = spoutdx9.GetDX9device();
	
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

    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Turn on ambient lighting 
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Load the mesh and build the material and texture arrays
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    LPD3DXBUFFER pD3DXMtrlBuffer;

    // Load the mesh from the specified file
    if( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
                                   g_pd3dDevice, NULL,
                                   &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                   &g_pMesh ) ) )
    {
        // If model is not in current folder, try parent folder
        if( FAILED( D3DXLoadMeshFromX( L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
                                       g_pd3dDevice, NULL,
                                       &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                       &g_pMesh ) ) )
        {
            MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
            return E_FAIL;
        }
    }

    // We need to extract the material properties and texture names from the 
    // pD3DXMtrlBuffer
    D3DXMATERIAL* d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();
    g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    if( g_pMeshMaterials == NULL )
        return E_OUTOFMEMORY;
    g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if( g_pMeshTextures == NULL )
        return E_OUTOFMEMORY;

    for( DWORD i = 0; i < g_dwNumMaterials; i++ )
    {
        // Copy the material
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // Set the ambient color for the material (D3DX does not do this)
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

        g_pMeshTextures[i] = NULL;
        if( d3dxMaterials[i].pTextureFilename != NULL &&
            lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
        {
            // Create the texture
            if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                    d3dxMaterials[i].pTextureFilename,
                                                    &g_pMeshTextures[i] ) ) )
            {
                // If texture is not in current folder, try parent folder
                const CHAR* strPrefix = "..\\";
                CHAR strTexture[MAX_PATH];
                strcpy_s( strTexture, MAX_PATH, strPrefix );
                strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
                // If texture is not in current folder, try parent folder
                if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                        strTexture,
                                                        &g_pMeshTextures[i] ) ) )
                {
                    MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
                }
            }
        }
    }

    // Done with the material buffer
    pD3DXMtrlBuffer->Release();

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pMeshMaterials != NULL )
        delete[] g_pMeshMaterials;

    if( g_pMeshTextures )
    {
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
        {
            if( g_pMeshTextures[i] )
                g_pMeshTextures[i]->Release();
        }
        delete[] g_pMeshTextures;
    }
    if( g_pMesh != NULL )
        g_pMesh->Release();

	//
	// SPOUT
	//

	// Close the named texture access mutex
	frame.CloseAccessMutex();

	// Close frame counting
	frame.CleanupFrameCount();

	// Release application resources

	sender.ReleaseSenderName(g_SenderName);
	if (g_pSharedTexture)
		g_pSharedTexture->Release();
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
    D3DXMatrixRotationY( &matWorld, timeGetTime() / 1000.0f );
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
    // D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	// SPOUT - Make the tiger a little bit bigger
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 5, 1.0f, 1.0f, 100.0f);
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
						// SPOUT - Tigers are in the jungle, so make the background green.
						D3DCOLOR_XRGB(33, 130, 0), 1.0f, 0);

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Setup the world, view, and projection matrices
        SetupMatrices();

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
        {
            // Set the material and texture for this subset
            g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
            g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

            // Draw the mesh subset
            g_pMesh->DrawSubset( i );
        }

        // End the scene
        g_pd3dDevice->EndScene();
    }

	//
	// SPOUT
	//
	// Get the swap chain's backbuffer to a texture
	IDirect3DSurface9* pBackBuffer = nullptr;
	HRESULT hr = g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	if (SUCCEEDED(hr)) {

		// Get the texture details
		D3DSURFACE_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		pBackBuffer->GetDesc(&desc);
		// Now that we have the backbuffer, we can create a sender
		// Formats compatible with OpenGL applications are D3DFMT_A8R8G8B8 or D3DFMT_X8R8G8B8
		if (desc.Width > 0 && desc.Height > 0
			&& (desc.Format == D3DFMT_A8R8G8B8 || desc.Format == D3DFMT_X8R8G8B8)) {
			if (!bSpoutInitialized) {
				g_Width = desc.Width;
				g_Height = desc.Height;
				// Create a new shared texture of the same size for the sender
				// The format should match that of the local texture (backbuffer in this case)
				// This should be either the same as the backbuffer
				// TODO g_dxShareHandle = NULL; // For a new texture
				spoutdx9.CreateSharedDX9Texture(g_pd3dDevice, g_Width, g_Height, desc.Format, g_pSharedTexture, g_dxShareHandle);
				// Create a sender using the shared texture share-handle
				sender.CreateSender(g_SenderName, g_Width, g_Height, g_dxShareHandle, (DWORD)desc.Format);
				// Create a sender mutex for access to the shared texture
				frame.CreateAccessMutex(g_SenderName);
				// Enable frame counting for sender frame number and fps
				frame.EnableFrameCount(g_SenderName);
				bSpoutInitialized = true;
			}
			// If the sender is already created, check for change of rendering size
			// In this example, the back buffer does not change size even if the window
			// is re-sized. But this is what would be done in a different application
			else if (g_Width != desc.Width || g_Height != desc.Height) {
				// Update the global width & height
				g_Width = desc.Width;
				g_Height = desc.Height;
				// Update the sender's shared texture
				spoutdx9.CreateSharedDX9Texture(g_pd3dDevice, g_Width, g_Height, desc.Format, g_pSharedTexture, g_dxShareHandle);
				// Update the sender's information shared memory map
				sender.UpdateSender(g_SenderName, g_Width, g_Height, g_dxShareHandle, desc.Format);
			}

			// Send the texture
			if (bSpoutInitialized) {
				// Check the sender mutex for access the shared texture in case a receiver is holding it
				if (frame.CheckTextureAccess()) {
					// Copy the backbuffer texture to the sender's shared texture
					// Both textures have the same size and format
					spoutdx9.WriteDX9surface(g_pd3dDevice, pBackBuffer, g_pSharedTexture);
					// Signal a new frame while the mutex is still locked
					frame.SetNewFrame();
					// Allow access to the shared texture
					frame.AllowTextureAccess();
				}
			}
			pBackBuffer->Release();
		}
	}

	//
	// SPOUT - fps control
	//
	// Hold a target frame rate - e.g. 60 or 30fps
	// This is not necessary if the application already has
	// fps control but in this example rendering is done
	// during idle time and render rate can be extremely high.
	// If frame count is enabled, receivers will detect the sender fps
	frame.HoldFps(60);
	
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
    HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 06: Meshes",
                              WS_OVERLAPPEDWINDOW, 0, 0, 512, 512,
                              NULL, NULL, wc.hInstance, NULL );

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
	g_dxShareHandle = NULL;
	g_SenderName[0] = 0;
	g_Width = 0;
	g_Height = 0;
	bSpoutInitialized = false;

	// Optionally enable Spout logging
	// OpenSpoutConsole(); // Console only without logs for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile("D3D9_Lights.log"); // Log to file
	// Default log file path is "C:\Users\username\AppData\Roaming\Spout\"
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // Show only warnings and errors

	// Give the sender a name
	strcpy_s(g_SenderName, 256, "MeshesDX9sender");
	
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
                else
                    Render();
            }
        }
    }

	// SPOUT
	// Cleanup() takes care of releasing resources when the window is closed
	
	UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}



