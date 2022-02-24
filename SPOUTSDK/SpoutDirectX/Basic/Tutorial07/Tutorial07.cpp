//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT input (http://spout.zeal.co/)
// from : https://github.com/walbourn/directx-sdk-samples/tree/master/Direct3D11Tutorials
// Search on "SPOUT" for additions.
//
// This is a receiver using methods directly from the Spout 2.007 SDK.
// Compare with a version using the SpoutDX support class
// which could be suitable for your application.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This application demonstrates texturing
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729724.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"

// SPOUT
// Change paths as required
#include "..\..\..\SpoutGL\SpoutSenderNames.h" // for sender creation and update
#include "..\..\..\SpoutGL\SpoutDirectX.h" // for creating a shared texture
#include "..\..\..\SpoutGL\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\..\SpoutGL\SpoutUtils.h" // for logging utilites
#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct CBNeverChanges
{
    XMMATRIX mView;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = nullptr;
HWND                                g_hWnd = nullptr;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = nullptr;
ID3D11Device1*                      g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*                g_pImmediateContext = nullptr;
ID3D11DeviceContext1*               g_pImmediateContext1 = nullptr;
IDXGISwapChain*                     g_pSwapChain = nullptr;
IDXGISwapChain1*                    g_pSwapChain1 = nullptr;
ID3D11RenderTargetView*             g_pRenderTargetView = nullptr;
ID3D11Texture2D*                    g_pDepthStencil = nullptr;
ID3D11DepthStencilView*             g_pDepthStencilView = nullptr;
ID3D11VertexShader*                 g_pVertexShader = nullptr;
ID3D11PixelShader*                  g_pPixelShader = nullptr;
ID3D11InputLayout*                  g_pVertexLayout = nullptr;
ID3D11Buffer*                       g_pVertexBuffer = nullptr;
ID3D11Buffer*                       g_pIndexBuffer = nullptr;
ID3D11Buffer*                       g_pCBNeverChanges = nullptr;
ID3D11Buffer*                       g_pCBChangeOnResize = nullptr;
ID3D11Buffer*                       g_pCBChangesEveryFrame = nullptr;
ID3D11ShaderResourceView*           g_pTextureRV = nullptr;
ID3D11SamplerState*                 g_pSamplerLinear = nullptr;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
// XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );
// SPOUT
XMFLOAT4                            g_vMeshColor(1.0f, 1.0f, 1.0f, 1.0f);

// SPOUT
spoutSenderNames spoutsender;
spoutDirectX spoutdx;
spoutFrameCount frame;

ID3D11Texture2D* g_pReceivedTexture = nullptr; // Receiving texture for this example
ID3D11Texture2D* g_pSenderTexture = nullptr; // Pointer to the sender's shared texture
HANDLE g_dxShareHandle = NULL; // Share handle of the sender's shared texture
ID3D11ShaderResourceView* g_pSpoutTextureRV = nullptr; // Shader resource view of the texture
char g_SenderName[256]; // Sender name
char g_SenderNameSetup[256]; // Sender name to connect to
unsigned int g_Width = 0; // Sender width
unsigned int g_Height = 0; // sender height
long g_senderframe = 0; // Sender frame number
double g_senderfps = 0.0; // Sender frame rate
bool bNewFrame = false; // The received frame is new
bool bSpoutInitialized = false; // Initialized for the connected sender
bool bSpoutPanelOpened = false; // User opened sender selection panel
bool bSpoutPanelActive = false; // Selection panel is still open
SHELLEXECUTEINFOA g_ShExecInfo; // Global info so the exit code can be tested

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

// SPOUT utility functions
bool OpenSpoutPanel();
bool CheckSpoutPanel(char *sendername, int maxchars = 256);


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	// SPOUT

	// Optionally enable Spout logging
	OpenSpoutConsole(); // Console only without logs for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile("Tutorial07.log"); // Log to file
	// Default log file path is "C:\Users\username\AppData\Roaming\Spout\"
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // Show only warnings and errors

	// Initialize Spout variables
	g_pReceivedTexture = nullptr;
	g_pSenderTexture = nullptr;
	g_dxShareHandle = NULL;
	g_pSpoutTextureRV = nullptr;
	g_SenderName[0] = 0;
	g_SenderNameSetup[0] = 0;
	g_Width = 0;
	g_Height = 0;
	g_senderframe = 0;
	g_senderfps = 0.0;
	bNewFrame = false;
	bSpoutInitialized = false;
	
    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	// Optionally set the name of the sender to receive from
	// The receiver will only connect to that sender.
	// The user can over-ride this by selecting another.
	// See also for reset of name in render
	// strcpy_s(g_SenderNameSetup, 256, "Spout DX11 Sender"); // Set the starting name
	// strcpy_s(g_SenderName, 256, "Spout DX11 Sender"); // Set the general name as well

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
	{
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
		{
            Render();
        }
    }

	// SPOUT

	// Close the named texture access mutex
	frame.CloseAccessMutex();
	
	// Close frame counting
	frame.CleanupFrameCount();

	// Release application resources

	// Shader resource view of the sender's texture
	if (g_pSpoutTextureRV)
		g_pSpoutTextureRV->Release();

	// Receiving texture
	if (g_pReceivedTexture)
		g_pReceivedTexture->Release();

	// Pointer to the sender's texture created with OpenSharedResource
	if (g_pSenderTexture)
		g_pSenderTexture->Release();

	g_pSpoutTextureRV = nullptr;
	g_pReceivedTexture = nullptr;
	g_pSenderTexture = nullptr;
	g_dxShareHandle = NULL;
	
	CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640, 360 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", 
						   L"Direct3D 11 Tutorial 7 - Spout receiver",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

	//
	// SPOUT
	//
	// Centre the window on the desktop work area
	GetWindowRect(g_hWnd, &rc);
	RECT WorkArea;
	int WindowPosLeft = 0;
	int WindowPosTop = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
	WindowPosLeft += ((WorkArea.right - WorkArea.left) - (rc.right - rc.left)) / 2;
	WindowPosTop += ((WorkArea.bottom - WorkArea.top) - (rc.bottom - rc.top)) / 2;
	MoveWindow(g_hWnd, WindowPosLeft, WindowPosTop,
		(rc.right - rc.left), rc.bottom - rc.top, false);
	
	ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// SPOUT note
	// GL/DX interop Spec
	// ID3D11Device can only be used on WDDM operating systems : Must be multithreaded

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        if ( hr == E_INVALIDARG )
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        }

        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if ( dxgiFactory2 )
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1) );
        if (SUCCEEDED(hr))
        {
            (void) g_pImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain) );
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, nullptr, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Create index buffer
    // Create vertex buffer
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Load the Texture
    hr = CreateDDSTextureFromFile( g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV );
    if( FAILED( hr ) )
        return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    /*
	XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	*/
	// SPOUT - Move it up a bit
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear ) g_pSamplerLinear->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pCBNeverChanges ) g_pCBNeverChanges->Release();
    if( g_pCBChangeOnResize ) g_pCBChangeOnResize->Release();
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice1 ) g_pd3dDevice1->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();

}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

	// SPOUT - RH click to open SpoutPanel
	case WM_RBUTTONDOWN:
		OpenSpoutPanel();
		break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    // Update our time
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static ULONGLONG timeStart = 0;
        ULONGLONG timeCur = GetTickCount64();
        if( timeStart == 0 )
            timeStart = timeCur;
        t = ( timeCur - timeStart ) / 1000.0f;
		t /= 2.0f; // SPOUT - slow the cube rotation down a bit
    }

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
				bNewFrame = false;
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

			// Create or re-create the receiving texture.
			// spoutdx.CreateSharedDX11Texture will release it, but do it here anyway
			if (g_pReceivedTexture)	g_pReceivedTexture->Release();
			g_pReceivedTexture = nullptr;
			HANDLE textureHandle = NULL; // dummy handle for the Spout function
			spoutdx.CreateSharedDX11Texture(g_pd3dDevice,
				g_Width, g_Height, (DXGI_FORMAT)dwFormat, // Format is the same as the sender
				&g_pReceivedTexture, textureHandle);

			// Release any resource previously created with OpenSharedResource
			if (g_pSenderTexture) g_pSenderTexture->Release();
			g_pSenderTexture = nullptr;

			// Retrieve the new sender's pointer using the share handle retrieved by FindSender above
			spoutdx.OpenDX11shareHandle(g_pd3dDevice, &g_pSenderTexture, g_dxShareHandle);

		}

		// If we have a pointer to the sender texture it can be used directly.
		// For this example, copy from it to a receiving texture
		if (g_pSenderTexture && g_pReceivedTexture) {

			// Access the sender shared texture
			// (See comments in the CheckTextureAccess function)
			if (frame.CheckTextureAccess(g_pSenderTexture)) {

				// Optionally check whether the sender has produced a new frame.
				// This is not required, but will avoid un-necessary processing for every frame.

				// This must be done within a sender mutex lock so that the sender will not write
				// to the texture and increment the count while a receiver is reading it.
				if (frame.GetNewFrame()) {
					// Here is where the sender's shared texture can be safely accessed.
					// In this example we will copy it to a local texture.
					g_pImmediateContext->CopyResource(g_pReceivedTexture, g_pSenderTexture);
					// Set an update flag for use below because the 
					// new frame query has to be done in a mutex lock
					bNewFrame = true;
				} // New frame from the sender
				else {
					bNewFrame = false;
				}

				// Allow texture access even if it was not a new frame
				frame.AllowTextureAccess(g_pSenderTexture);

			} // Accessed sender's shared texture

		} // Retrieved sender shared texture pointer

		// The received texture has been updated.
		// Any action required by the receiver can be done here.

		// In this example, a shader resource view of the received texture
		// is created after receiving a new frame. 
		if (bNewFrame && g_pReceivedTexture) {

			if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
			g_pSpoutTextureRV = nullptr;

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
			// Matching format for the shader resource view is important.
			shaderResourceViewDesc.Format = (DXGI_FORMAT)dwFormat;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
			g_pd3dDevice->CreateShaderResourceView(g_pReceivedTexture, &shaderResourceViewDesc, &g_pSpoutTextureRV);

			bNewFrame = false; // reset the new frame flag
		}

		// The receiver can also query the sender frame number and rate
		g_senderframe = frame.GetSenderFrame();
		g_senderfps = frame.GetSenderFps();

	} // sender exists
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

			// Clear the spout texture resource view so render uses the default
			if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
			g_pSpoutTextureRV = nullptr;

			// Initialize them again when a sender is found
			bSpoutInitialized = false;
		}

	}

	// Rotate cube around the origin
	// g_World = XMMatrixRotationY(t);
	// SPOUT - pitch and yaw
	g_World = XMMatrixRotationRollPitchYaw(0.0f, t, t);

    // Modify the color
	// SPOUT - remove
    // g_vMeshColor.x = ( sinf( t * 1.0f ) + 1.0f ) * 0.5f;
    // g_vMeshColor.y = ( cosf( t * 3.0f ) + 1.0f ) * 0.5f;
    // g_vMeshColor.z = ( sinf( t * 5.0f ) + 1.0f ) * 0.5f;

    //
    // Clear the back buffer
    //
    // g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );
	// SPOUT
	// Change backgound colour for better contrast with a blue sender
	FLOAT backgnd[4] = { 0.39f, 0.025f, 0.0f }; // red/brown
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, backgnd);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose( g_World );
    cb.vMeshColor = g_vMeshColor;
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0 );

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader( g_pVertexShader, nullptr, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->VSSetConstantBuffers( 1, 1, &g_pCBChangeOnResize );
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetShader( g_pPixelShader, nullptr, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
	
	// SPOUT
	// Use the sender's texture shader resource view
	if(g_pSpoutTextureRV)
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pSpoutTextureRV);
	else
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    g_pImmediateContext->DrawIndexed( 36, 0, 0 );

    //
    // Present our back buffer to our front buffer
    //
    // g_pSwapChain->Present( 0, 0 );
	g_pSwapChain->Present(1, 0); // Cap at monitor refresh rate

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
	if(hMutex1) CloseHandle(hMutex1);

	

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

