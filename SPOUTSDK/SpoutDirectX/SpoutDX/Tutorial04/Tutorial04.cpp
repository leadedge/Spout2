//--------------------------------------------------------------------------------------
// File: Tutorial04.cpp
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adapted for SPOUT output (http://spout.zeal.co/)
// from : https://github.com/walbourn/directx-sdk-samples/tree/master/Direct3D11Tutorials
// Search on "SPOUT" for additions.
// Version to send using 2.007 methods
//
// This is a sender using the "SpoutDX" support class using SendTexture
//
// bool spoutDX::SendTexture(ID3D11Texture2D* pTexture)
//
// Compare with a stand-alone version using methods directly from 
// the Spout SDK classes and with Windows examples using SendImage.
//
// Compare also with the "Tutorial04_Lib" project, a version using 
// SpoutDX as a dynamic or static libary. Application source files
// are the same apart from a simple code addition.
//
//
// Window resizing has been added to the original sample to demonstrate
// requirements for a Spout sender to handle size changes.
//
// 24.04.23 - remove redundant OpenDirectX11 from ResetDevice
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This application displays a 3D cube using Direct3D 11
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729721.aspx
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
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "Resource.h"

// SPOUT
#include "../../SpoutDX/SpoutDX.h"

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           g_pVertexBuffer = nullptr;
ID3D11Buffer*           g_pIndexBuffer = nullptr;
ID3D11Buffer*           g_pConstantBuffer = nullptr;
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

// SPOUT
spoutDX sender;

// Functions for selecting graphics adapter
void ResetDevice();
void SelectAdapter();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdapterProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static std::string adaptername[10];
static int adaptercount = 0;
static int currentadapter = 0;
static int selectedadapter = 0;
static int senderadapter = 0;

// Option to create a SpoutDX class device (see InitDevice)
bool bClassdevice = false; 


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	//
	// SPOUT
	//

	// Optionally enable logging to catch Spout warnings and errors
	// OpenSpoutConsole(); // Console only for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile(); // Log to file
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors

	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

	//
	// SPOUT
	//

	// See InitDevice for creating a Spout class device
    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	//
	// Menu option for change of graphics adapter
	//
	// Both sender and receiver must use the same graphics adapter.
	// See more information in the Tutorial07 receiver code.
	//
	// Graphics adapter selection requires the device to be created
	// in the SpoutDX class (see InitDevice). If a class device was
	// not created, remove the menu option.
	//
	if (!sender.IsClassDevice()) {
		HMENU hPopup = GetSubMenu(GetMenu(g_hWnd), 0);
		RemoveMenu(hPopup, IDM_ADAPTER, MF_BYCOMMAND);
	}
	
	// Option : give the sender a name
	// If none is specified, the executable name is used
	// sender.SetSenderName("Tutorial04sender");
	
	// Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
		if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
	    {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		else {
			Render();
		}
    }

	// SPOUT
	sender.ReleaseSender();

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
	WNDCLASSEX wcex={};
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
    // AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU, TRUE); // Resize for the menu
    g_hWnd = CreateWindow(	L"TutorialWindowClass",
							L"Direct3D 11 Tutorial 4: - Spout sender",
							// SPOUT - enable resize and maximize to demonstrate sender resizing
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT,
							rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if( !g_hWnd )
        return E_FAIL;

	//
	// SPOUT
	//
	// Centre the window on the desktop
	GetWindowRect(g_hWnd, &rc);
	MoveWindow(g_hWnd,
		(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
		(rc.right - rc.left),
		(rc.bottom - rc.top), false);

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

	// ===============================================================
	//
	// SPOUT
	//
	// Option: Create a device within the SpoutDX class.
	//
	// InitDevice() will return the hr result after swap chain creation
	// whether a class device is ceated or an application device is created as usual. 
	//
	// Use the current graphics adapter index (currentadapter). This can then
	// be selected by the user to change the adapter (see SelectAdapter).
	//
	// Graphics adapter selection requires a class device.
	// Both sender and receiver must be using the same graphics adapter
	//
	// The other metohod is to use OpenDirectX11() after an application device
	// has been created. See creation of an application device further below.
	//
	if (bClassdevice) {
		if (sender.OpenDirectX11()) {
			// Set the application device and context to those created in the SpoutDX class
			g_pd3dDevice = sender.GetDX11Device();
			g_pImmediateContext = sender.GetDX11Context();
		}
		else {
			return 0;
		}
	}
	else {

		// Usual creation of an application device

		// SPOUT note
		// GL/DX interop Spec
		// ID3D11Device can only be used on WDDM operating systems : Must be multithreaded

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
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
			hr = D3D11CreateDevice(nullptr,
				g_driverType,
				nullptr,
				createDeviceFlags,
				featureLevels,
				numFeatureLevels,
				D3D11_SDK_VERSION,
				&g_pd3dDevice,
				&g_featureLevel,
				&g_pImmediateContext);

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
				hr = D3D11CreateDevice(nullptr,
					g_driverType,
					nullptr,
					createDeviceFlags,
					&featureLevels[1],
					numFeatureLevels - 1,
					D3D11_SDK_VERSION,
					&g_pd3dDevice,
					&g_featureLevel,
					&g_pImmediateContext);

			}
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		//
		// Initialize the Spout DirectX device using the application device.
		//
		// If an application DirectX 11.0 device was created above,
		// the device pointer must be passed in to the SpoutDX class.
		// The function does nothing if a class device was already created.
		if (!sender.OpenDirectX11(g_pd3dDevice))
			return S_FALSE;

	}
	// ===============================================================

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
		// SPOUT
		// For compatibility with DirectX9 shared textures
		// the format should be BGRA instead of RGBA
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	// DXGI_FORMAT_R8G8B8A8_UNORM;
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
		// SPOUT
		// For compatibility with DirectX9 shared textures
		// the format should be BGRA instead of RGBA
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //  DXGI_FORMAT_R8G8B8A8_UNORM;
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

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, nullptr );

    // Setup the viewport
	D3D11_VIEWPORT vp={};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile( L"Tutorial04.fx", "VS", "vs_4_0", &pVSBlob );
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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
    hr = CompileShaderFromFile( L"Tutorial04.fx", "PS", "ps_4_0", &pPSBlob );
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
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  1.0f,  1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f,  1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f, -1.0f,  1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f,  1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 8;
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
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36; // 36 vertices needed for 12 triangles in a triangle list
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

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pConstantBuffer );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrix
	g_World = XMMatrixIdentity();

    // Initialize the view matrix
	/*
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At  = XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f );
	XMVECTOR Up  = XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f );
	*/
	// SPOUT - Move it up a bit
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At  = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	// g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f );
	// SPOUT - Make it a bit bigger
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, (FLOAT)width / (FLOAT)height, 0.01f, 100.0f);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();

	// SPOUT
	// If the device and context have been created in SpoutDX
	// do not release them here
	if (!sender.IsClassDevice()) {
		if (g_pImmediateContext) g_pImmediateContext->Release();
		if (g_pd3dDevice) g_pd3dDevice->Release();
	}

}

// SPOUT
// Re-initialize for size changes
void ResetDevice()
{
	// Release everything
	sender.ReleaseSender();
	sender.CloseDirectX11();
	CleanupDevice();
	// Create device and objects
	InitDevice();
	// SpoutDX will now use the new device
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )  {

		case WM_COMMAND :
			// Parse the menu selections:
			switch (LOWORD(wParam))	{
				case IDM_ADAPTER:
					SelectAdapter();
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

		// =====================================================================
		// SPOUT
		// SYNC OPTION
		// Option only - not required for typical function
		// Enable these lines for the sync option
		/*
		// Space bar - disable or enable sync with the receiver
		case WM_KEYUP:
			if (wParam == 32) {
				if (sender.frame.IsFrameSyncEnabled()) {
					sender.frame.EnableFrameSync(false);
					SpoutLog("Sync disabled");
				}
				else {
					sender.frame.EnableFrameSync(true);
					SpoutLog("Sync enabled");
				}
			}
			break;
		// =====================================================================
		*/

	    case WM_PAINT:
		    hdc = BeginPaint( hWnd, &ps );
			EndPaint( hWnd, &ps );
			break;

		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;

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
	if (sender.IsInitialized()) {
		// Has the window changed size?
		if (!IsIconic(g_hWnd)) {
			RECT rc;
			GetClientRect(g_hWnd, &rc);
			if (sender.GetWidth() != (unsigned int)(rc.right - rc.left)
				|| sender.GetHeight() != (unsigned int)(rc.bottom - rc.top)) {
				// Reset everything
				ResetDevice();
				return;
			}
		}
	}

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
		t /= 2.0f; // slow it down a bit
    }

    //
    // Animate the cube
    //
	// g_World = XMMatrixRotationY( t );
	// SPOUT - pitch and yaw
	g_World = XMMatrixRotationRollPitchYaw(0.0f, t, t);

    //
    // Clear the back buffer
    //
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );

    //
    // Update variables
    //
	ConstantBuffer cb={};
	cb.mWorld = XMMatrixTranspose( g_World );
	cb.mView = XMMatrixTranspose( g_View );
	cb.mProjection = XMMatrixTranspose( g_Projection );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, nullptr, &cb, 0, 0 );

    //
    // Renders a triangle
    //
	g_pImmediateContext->VSSetShader( g_pVertexShader, nullptr, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader, nullptr, 0 );
	g_pImmediateContext->DrawIndexed( 36, 0, 0 ); // 36 vertices needed for 12 triangles in a triangle list

	//
	// SPOUT
	//

	// =====================================================================
	// OPTION - sync with the receiver
	// Option only - not required for typical function
	//
	// Wait until the receiver is ready to read a frame
	// The receiver must send a "ready to receive" SendFrameSync messsage
	// See the Tutorial07 example
	//
	// Enable this line for the sync option
	// sender.WaitFrameSync(sender.GetName(), 67);
	//
	// =====================================================================

	// Option 1 
	// Send the swap chain's back buffer.
	// A single render target view is assumed.
	// Sending functions handle sender creation and resizing.
	sender.SendBackBuffer();

	/*
	// Option 2
	// Send a texture.
	// In this example, we get the back buffer but the texture can also be independent.
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (SUCCEEDED(hr)) {
		sender.SendTexture(pBackBuffer);
	}
	*/

    //
    // Present our back buffer to our front buffer
	//
	// Here the frame rate can be extremely high if "vertical sync"
	// is disabled for the graphics driver.
	// To avoid exessive processing, a sync interval for the Present
	// method will synchronize with vertical blank, typically 60 fps.
	g_pSwapChain->Present(1, 0);

	//
	// SPOUT - fps control
	//
	// The frame rate can be controlled further if necessary
	// using thread sleep until the required frame time elapses.
	// Note code comments for the HoldFps function.
	//
	// In this example, the cube rotation speed will not change
	// because it is calculated independently, but the render rate
	// will be controlled.
	// 
	// For example :
	// sender.HoldFps(30);

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

		sprintf_s(about, 256, "                Tutorial04");
		strcat_s(about, 1024, "\n\n\n");
		strcat_s(about, 1024, "Spout DirectX texture sender example\n");
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


void SelectAdapter()
{
	currentadapter = sender.GetAdapter(); // Get the current adapter index
	selectedadapter = currentadapter; // The index to be selected in the dialog
	// Create an adapter name list for the dialog
	adaptercount = sender.GetNumAdapters();
	adaptername->clear();
	char name[64];
	for (int i = 0; i < adaptercount; i++) {
		sender.GetAdapterName(i, name, 64);
		adaptername[i] = name;
	}

	// Show the dialog box 
	int retvalue = (int)DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ADAPTERBOX), g_hWnd, (DLGPROC)AdapterProc);

	if (retvalue != 0) {
		// OK - adapter index (selectedadapter) has been selected
		// Set the selected adapter if different
		if (selectedadapter != currentadapter) {

			SpoutLogNotice("Tutorial04 : selectedadapter = %d, currentadapter = %d", selectedadapter, currentadapter);

			// A new sender using the selected adapter will be created
			// on the first SendTexture call (Requires 2.007)
			sender.ReleaseSender();
			if (!sender.SetAdapter(selectedadapter)) {
				// SetAdapter returns to the primary adapter for failure
				// Refer to error logs for diagnostics
				MessageBoxA(NULL, "Could not select graphics adapter", "Tutorial04", MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);
				// Set the adapter back to what it was (the compatibility test is repeated)
				sender.SetAdapter(currentadapter);
			}
			else {
				// Change the application current adapter index to the one selected
				// This will take effect when the sender is re-created
				currentadapter = selectedadapter;
			}

			// Release everything
			sender.ReleaseSender();
			sender.CloseDirectX11();
			CleanupDevice();
			// Create device and objects
			InitDevice();
			// SpoutDX will now use the new device
			sender.OpenDirectX11(g_pd3dDevice);
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

// That's all ..
