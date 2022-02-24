//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT output (http://spout.zeal.co/)
// from : https://github.com/walbourn/directx-sdk-samples/tree/master/Direct3D11Tutorials
// Search on "SPOUT" for additions.
//
// This is a receiver using the SpoutDX support class and ReceiveTexture :
//
// bool spoutDX::ReceiveTexture(ID3D11Texture2D** ppTexture)
//
// Compare with a version using methods directly from the Spout 2.007 SDK.
// Compare also with a Windows example using ReceiveImage.
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
#include "..\..\SpoutDX\SpoutDX.h"

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

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

// SPOUT
spoutDX receiver; // Spout DirectX11 receiver
ID3D11Texture2D* g_pReceivedTexture = nullptr; // Texture received from a sender
ID3D11ShaderResourceView* g_pSpoutTextureRV = nullptr; // Shader resource view of the texture

// Functions for selecting graphics adapter
void ResetDevice();
void SelectAdapter();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdapterProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
bool g_bAutoAdapt = false; // Auto switch to the same adapter as the sender (menu option)

// Statics for dialog box
static char sendername[256];
static std::string adaptername[10];
static int adaptercount = 0;
static int currentadapter = 0;
static int selectedadapter = 0;
static int senderadapter = 0;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	//
	// SPOUT Options
	//

	// OpenSpoutConsole(); // Console only for debugging
	EnableSpoutLog(); // Enable Spout logging to console
	// EnableSpoutLogFile("Tutorial07.log"); // Log to file
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors

	// Set the name of the sender to receive from.
	// The receiver will only connect to that sender.
	// The user can over-ride this by selecting another (receiver.SelectSender())
	// It can be cleared with a null sender name (receiver.SetReceiverName())
	// receiver.SetReceiverName("Spout Demo Sender");
    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

	// SPOUT
	// See InitDevice for creating a Spout class device
	if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	//
	// SPOUT
	//

	// Initialize DirectX.
	// The device pointer must be passed in if a DirectX 11.0 device is available.
	// Otherwise a device is created in the SpoutDX class.
	// The function does nothing if a class device was already created.
	// See InitDevice for creating a class device
	//
	if (!receiver.OpenDirectX11(g_pd3dDevice))
		return FALSE;

	//
	// Menu options for change of graphics adapter
	//

	// The "Select graphics adapter" menu option allows selection
	// and change to the same adapter as used by a particluar sender.
	//
	// The "Auto switch adapter" menu option enables auto switch to
	// the same adapter as the sender.
	// Search for : IDM_AUTO_ADAPTER for more detail on the "SetAdapterAuto()" 
	// function which enables or disables this option..
	//
	// Both sender and receiver must use the same graphics adapter.
	// If the "Auto switch adapter" menu option is enabled, selection of a different
	// graphics adapter will result in change back to the sender graphics adapter.
	//
	// The D3D11 device must have been be created within the SpoutDX class
	// If a class device was not created, remove both menu options.
	//
	if (!receiver.IsClassDevice()) {
		HMENU hPopup = GetSubMenu(GetMenu(g_hWnd), 0);
		RemoveMenu(hPopup, IDM_ADAPTER, MF_BYCOMMAND);
		RemoveMenu(hPopup, IDM_AUTO_ADAPTER, MF_BYCOMMAND);
	}

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
	receiver.ReleaseReceiver();
	receiver.CloseDirectX11();
	if (g_pSpoutTextureRV)
		g_pSpoutTextureRV->Release();
	if (g_pReceivedTexture)
		g_pReceivedTexture->Release();

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
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TUTORIAL1); // add a menu
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640, 360 };
	AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU, TRUE); // Resize for the menu
    g_hWnd = CreateWindow( L"TutorialWindowClass", 
						   L"Direct3D 11 Tutorial 7 - Spout receiver",
						   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, 
						   rc.right - rc.left, rc.bottom - rc.top,
						   nullptr, nullptr, hInstance, nullptr );

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
    UINT width  = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	//
	// SPOUT
	//
	// Option: Create a device within the SpoutDX class.
	// See below for device reaction code that has been commented out.
	// IsClassDevice() will return the hr result whether this has been done or not.
	// See also the option to use OpenDirectX11() after an application device has been created.
	// ===============================================================
	if (receiver.OpenDirectX11()) {
		g_pd3dDevice = receiver.GetDX11Device();
		g_pImmediateContext = receiver.GetDX11Context();
	}
	else {
		return 0;
	}
	// ===============================================================

	/*
	// ===============================================================
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
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// SPOUT
	// If an application DirectX 11.0 device was created above,
	// the device pointer must be passed in to the SpoutDX class.
	// The function does nothing if a class device was already created.
	//
	if (!receiver.OpenDirectX11(g_pd3dDevice))
		return FALSE;
	// ===============================================================
	*/

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
        { XMFLOAT3(  1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, 1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, 1.0f ),  XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ),  XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ),  XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, 1.0f ),  XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( 1.0f, -1.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f,  1.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
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
	if (g_pd3dDevice1) g_pd3dDevice1->Release();

	// SPOUT
	// If the device and context have been created in SpoutDX
	// do not release them here
	if (!receiver.IsClassDevice()) {
		if (g_pImmediateContext) g_pImmediateContext->Release();
		if (g_pd3dDevice) g_pd3dDevice->Release();
	}


}

// SPOUT
// Re-initialize for size changes
void ResetDevice()
{
	// Close the receiver
	receiver.ReleaseReceiver();
	// Close the spoutdx class device
	receiver.CloseDirectX11();
	// Release everything
	CleanupDevice();
	// Start again
	InitDevice();
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

	case WM_COMMAND:

		// Parse the menu selections:
		switch (LOWORD(wParam))
		{
			// Select sender
			case IDM_SENDER:
				// Sender selection dialog box 
				sendername[0] = 0; // Clear static name for dialog
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SENDERBOX), g_hWnd, (DLGPROC)SenderProc);
				break;
			// Select graphics adapter
			case IDM_ADAPTER:
				SelectAdapter();
				break;
			// Auto switch to sender adapter
			case IDM_AUTO_ADAPTER:
				{
					g_bAutoAdapt = !g_bAutoAdapt;
					if (g_bAutoAdapt) {
						receiver.SetAdapterAuto(true);
						// Switch to sender adapter if connected.
						// See details in SelectAdapter()
						if (receiver.IsConnected()) {
							selectedadapter = receiver.GetSenderAdapter(receiver.GetSenderName());
							if (receiver.SetAdapter(selectedadapter)) {
								currentadapter = selectedadapter;
								ResetDevice();
							}
						}
						CheckMenuItem(GetMenu(g_hWnd), IDM_AUTO_ADAPTER, MF_CHECKED);
					}
					else {
						receiver.SetAdapterAuto(false);
						CheckMenuItem(GetMenu(g_hWnd), IDM_AUTO_ADAPTER, MF_UNCHECKED);
					}
				}
				break;
			case IDM_ABOUT:
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				break;
		}
		break;

    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

	// SPOUT - RH click to open SpoutPanel
	case WM_RBUTTONDOWN:
		receiver.SelectSender();
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

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// SPOUT
	//
	// A texture can be received to a local texture
	// or to a texture withing the SpoutDX class.
	//
	// Change the receiving option here for testing
	//   1 - receive to a local texture (g_pReceivedTexture)
	//   2 - receive to a SpoutDX class texture
	//
	int option = 1;

	if (option == 1) {
		//
		// Option 1 : Receive from a sender to a local texture
		//
		if (receiver.ReceiveTexture(&g_pReceivedTexture)) {
			// Create or re-create the receiving texture 
			// for a new sender or if the sender size changed
			if (receiver.IsUpdated()) {

				// The D3D11 device within the SpoutDX class could have changed
				// if it has switched to use a different sender graphics adapter.
				// Re-intialize to refresh the application global device pointer.
				if (receiver.GetAdapterAuto()) {
					if (g_pd3dDevice != receiver.GetDX11Device()) {
						ResetDevice();
						// No more this round because the receiver has been released
						// and there is no width or height to create a texture.
						return;
					}
				}

				// Update the receiving texture
				if (g_pReceivedTexture)	g_pReceivedTexture->Release();
				g_pReceivedTexture = nullptr;
				receiver.CreateDX11texture(g_pd3dDevice,
					receiver.GetSenderWidth(),
					receiver.GetSenderHeight(),
					receiver.GetSenderFormat(),
					&g_pReceivedTexture);

				// Any other action required by the receiver can be done here
				// In this example, clear the shader resource view
				if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
				g_pSpoutTextureRV = nullptr;
			}

			// A texture has been received

			// In this example, a shader resource view is created if the frame is new
			if (receiver.IsFrameNew()) {
				if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
				g_pSpoutTextureRV = nullptr;
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
				ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
				// Matching format with the sender is important
				shaderResourceViewDesc.Format = receiver.GetSenderFormat();
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
				g_pd3dDevice->CreateShaderResourceView(g_pReceivedTexture, &shaderResourceViewDesc, &g_pSpoutTextureRV);
			}
		}
		else {
			// A sender was not found or the connected sender closed
			// Release the receiving texture
			if (g_pReceivedTexture)	g_pReceivedTexture->Release();
			g_pReceivedTexture = nullptr;
			// Release the texture resource view so render uses the default
			if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
			g_pSpoutTextureRV = nullptr;
		}

	}
	else if (option == 2) {
		//
		// Option 2 : Receive from a sender to a SpoutDX class texture
		//
		if (receiver.ReceiveTexture()) {
			//
			// Sender details can be retrieved with :
			//		const char * GetSenderName();
			//		unsigned int GetSenderWidth();
			//		unsigned int GetSenderHeight();
			//		DXGI_FORMAT GetSenderFormat();
			//		HANDLE GetSenderHandle;
			//		long GetSenderFrame();
			//		double GetSenderFps();
			//

			// The D3D11 device within the SpoutDX class could have changed
			// if it has switched to use a different sender graphics adapter.
			// Re-intialize to refresh the application global device pointer.
			if (receiver.GetAdapterAuto()) {
				if (g_pd3dDevice != receiver.GetDX11Device()) {
					ResetDevice();
					// No more this round because the receiver has been released
					// and there is no width or height to create a texture.
					return;
				}
			}

			// A class texture has been received
			// The received texture can be retrieved with
			//     ID3D11Texture2D* GetSenderTexture();

			// In this example, a shader resource view is created if the frame is new
			if (receiver.IsFrameNew()) {
				if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
				g_pSpoutTextureRV = nullptr;
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
				ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
				// Matching format with the sender is important
				shaderResourceViewDesc.Format = receiver.GetSenderFormat();
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
				g_pd3dDevice->CreateShaderResourceView(receiver.GetSenderTexture(), &shaderResourceViewDesc, &g_pSpoutTextureRV);
			}
		}
		else {
			// A sender was not found or the connected sender closed
			// Release the texture resource view so render uses the default
			if (g_pSpoutTextureRV) g_pSpoutTextureRV->Release();
			g_pSpoutTextureRV = nullptr;
		}
	}
	// End receive texture
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


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
	// Change backgound colour for better contrast with the blue demo sender
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::DarkRed);

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
	// Use the sender's texture shader resource view if received
	if(g_pSpoutTextureRV)	
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pSpoutTextureRV);
	else
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    g_pImmediateContext->DrawIndexed( 36, 0, 0 );

    //
    // Present our back buffer to our front buffer
    //
	//
	// SPOUT - fps control
	//
	// Here the frame rate can be extremely high.
	// To avoid exessive processing, hold a target frame rate
	// using sync interval for the Present method to synchronize
	// with vertical blank, typically 60 fps.
	// g_pSwapChain->Present( 0, 0 );
	g_pSwapChain->Present(1, 0);


}



// Message handler for about box.
// SPOUT : adapted for this example.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	char tmp[MAX_PATH];
	char about[1024];
	LPDRAWITEMSTRUCT lpdis;
	HWND hwnd = NULL;
	HCURSOR cursorHand = NULL;

	switch (message)
	{
	case WM_INITDIALOG:

		sprintf_s(about, 256, "                Tutorial07");
		strcat_s(about, 1024, "\n\n\n");
		strcat_s(about, 1024, "Spout DirectX texture receiver example\n");
		strcat_s(about, 1024, "using DirectX 11 and the SpoutDX class.\n");
		strcat_s(about, 1024, "Adapted from DirectX SDK examples by Chuck Walbourn.");
		SetDlgItemTextA(hDlg, IDC_ABOUT_TEXT, (LPCSTR)about);

		// Spout website hyperlink hand cursor
		cursorHand = LoadCursor(NULL, IDC_HAND);
		hwnd = GetDlgItem(hDlg, IDC_SPOUT_URL);
		SetClassLongPtrA(hwnd, GCLP_HCURSOR, (LONG_PTR)cursorHand);
		return (INT_PTR)TRUE;

	case WM_DRAWITEM:
		// The blue hyperlink
		lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->itemID == -1) break;
		SetTextColor(lpdis->hDC, RGB(6, 69, 173));
		switch (lpdis->CtlID) {
		case IDC_SPOUT_URL:
			DrawTextA(lpdis->hDC, "http://spout.zeal.co", -1, &lpdis->rcItem, DT_LEFT);
			break;
		default:
			break;
		}
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_SPOUT_URL) {
			// Open the website url
			sprintf_s(tmp, MAX_PATH, "http://spout.zeal.co");
			ShellExecuteA(hDlg, "open", tmp, NULL, NULL, SW_SHOWNORMAL);
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for selecting sender
INT_PTR  CALLBACK SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	switch (message) {

	case WM_INITDIALOG:
		// Sender combo selection
		{
			// Create an sender name list for the combo box
			HWND hwndList = GetDlgItem(hDlg, IDC_SENDERS);

			// Active sender name for initial item
			char activename[256];
			receiver.GetActiveSender(activename);
			int activeindex = 0;

			// Sender count
			int count = receiver.GetSenderCount();

			// Populate the combo box
			char name[128];
			for (int i = 0; i < count; i++) {
				receiver.GetSender(i, name, 128);
				// Active sender index for the initial combo box item
				if (strcmp(name, activename) == 0)
					activeindex = i;
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}

			// Show the active sender as the initial item
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)activeindex, (LPARAM)0);
		}
		return TRUE;

	case WM_COMMAND:

		// Combo box selection
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// Get the selected sender name
			int index = (int)SendMessageA((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			SendMessageA((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)index, (LPARAM)sendername);
		}
		// Drop through

		switch (LOWORD(wParam)) {

		case IDOK:
			// Selected sender
			if (sendername[0]) {
				// Make the sender active
				receiver.SetActiveSender(sendername);
				// Reset the receiving name
				// A new sender is detected on the first ReceiveTexture call
				// LJ DEBUG
				// receiver.SetReceiverName();
			}
			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			// User pressed cancel.
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;

		default:
			return (INT_PTR)FALSE;
		}
	}

	return (INT_PTR)FALSE;
}


void SelectAdapter()
{
	// The current adapter index
	currentadapter = receiver.GetAdapter();

	// The index to be selected in the dialog
	selectedadapter = currentadapter;

	// Create an adapter name list for the dialog
	adaptercount = receiver.GetNumAdapters();
	adaptername->clear();
	char name[64];
	for (int i = 0; i < adaptercount; i++) {
		receiver.GetAdapterName(i, name, 64);
		adaptername[i] = name;
	}

	// Show the dialog box 
	int retvalue = (int)DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ADAPTERBOX), g_hWnd, (DLGPROC)AdapterProc);

	if (retvalue != 0) {

		// OK - adapter index (selectedadapter) has been selected
		// Set the selected adapter if different
		if (selectedadapter != currentadapter) {
			SpoutLogNotice("Tutorial07:SelectAdapter() - selected = %d, current = %d", selectedadapter, currentadapter);
			if (receiver.SetAdapter(selectedadapter)) {
				// The adapter index was changed successfully. 
				currentadapter = selectedadapter;
			}
			else {
				MessageBoxA(NULL, "Could not select graphics adapter", "Tutorial07", MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
				// SetAdapter returns to the primary adapter for failure
				// so set the adapter index back to what it was
				receiver.SetAdapter(currentadapter);
			}

			// Reset everything to create a new device with the new adapter
			// A new sender using the selected adapter is detected on the first ReceiveTexture call
			ResetDevice();

		}
	}
}


// Message handler for selecting adapter
INT_PTR  CALLBACK AdapterProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	HWND hwndList = NULL;
	int i = 0;
	char name[128];

	switch (message) {

	case WM_INITDIALOG:
		// Adapter combo selection
		hwndList = GetDlgItem(hDlg, IDC_ADAPTERS);
		if (adaptercount < 10) {
			for (i = 0; i < adaptercount; i++) {
				sprintf_s(name, 128, "%d : %s", i, adaptername[i].c_str());
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}
			// Display an initial item in the selection field
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)currentadapter, (LPARAM)0);
		}
		return TRUE;

	case WM_COMMAND:

		// Combo box selection
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			selectedadapter = (int)SendMessageA((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		}
		// Drop through

		switch (LOWORD(wParam)) {

		case IDOK:
			// Return the selected adapter index
			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			// User pressed cancel.
			// Reset the selected index and take down dialog box.
			selectedadapter = currentadapter;
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;

		default:
			return (INT_PTR)FALSE;
		}
	}

	return (INT_PTR)FALSE;
}

// That's all..
