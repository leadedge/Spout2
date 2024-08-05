/*
=========================================================================

                        WinSpoutGLsender.cpp

						Basic OpenGL sender

    A Windows Desktop Application project generated by Visual Studio
    and adapted for Spout sender output (https://spout.zeal.co/).

	This is an example using SpoutGL and SendImage

	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert, GLuint HostFBO)

    Captures the screen to a pixel buffer using Windows bitmap functions.
	The screen can be resized to demonstrate sender update.

	Search on "SPOUT" for additions.

    Compare with the DirectX 11 Windows sender example using SpoutDX.

                 Copyright(C) 2020-2024 Lynn Jarvis.

    This program is free software : you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.If not, see < http://www.gnu.org/licenses/>.

===============================================================================
*/


#include "framework.h"
#include "WinSpoutGLsender.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                      // current instance
WCHAR szTitle[MAX_LOADSTRING];        // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name
int WindowPosLeft = 0;
int WindowPosTop = 0;
int window_width = 0;
int window_height = 0;

// SPOUT
Spout sender;                           // OpenGL sender object
HWND g_hWnd = NULL;                     // Window handle
HBITMAP g_hBitmap = NULL;               // Image bitmap for sending
unsigned int g_BitmapWidth = 0;         // Image bitmap width
unsigned int g_BitmapHeight = 0;        // Image bitmap height
unsigned char *g_pixelBuffer = nullptr; // Sending pixel buffer
unsigned char g_SenderName[256]{};      // Sender name
unsigned int g_SenderWidth = 0;         // Sender width
unsigned int g_SenderHeight = 0;        // Sender height

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// SPOUT
void Render();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// SPOUT
	// Optionally enable Spout console logging
	// EnableSpoutLog();
	// Or open an empty console for debugging
	// OpenSpoutConsole();

	// Load bitmap from resources
	g_hBitmap = (HBITMAP)LoadImageA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDB_WINSPOUTBMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	if (!g_hBitmap) {
		MessageBoxA(NULL, "Failed to load image", "WinSpoutGL", MB_OK);
		return FALSE;
	}

	// Get the size of the bitmap
	BITMAP bmpImage ={};
	GetObjectA(g_hBitmap, sizeof(BITMAP), &bmpImage);
	g_BitmapWidth = bmpImage.bmWidth;
	g_BitmapHeight = bmpImage.bmHeight;

	// Create a sending buffer of that inital size
	// It is resized as necessary - see WM_PAINT
	g_SenderWidth  = g_BitmapWidth;
	g_SenderHeight = g_BitmapHeight;
	unsigned int buffersize = g_SenderWidth * g_SenderHeight * 4;
	g_pixelBuffer = new unsigned char[buffersize];

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINSPOUTGL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// SPOUT
	// Initialize OpenGL
	if (!sender.CreateOpenGL()) {
		SpoutMessageBox(NULL, "CreateOpenGL failed", "Windows OpenGL sender", MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Optionally give the sender a name
	// If none is specified, the executable name is used
	sender.SetSenderName("Windows OpenGL sender");

	// Adjust the caption in case of multiple senders of the same name
	SetWindowTextA(g_hWnd, sender.GetName());

    // Main message loop:
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Render();
	}

	// SPOUT
	if(g_hBitmap) 
		DeleteObject(g_hBitmap);

	if (g_pixelBuffer) 
		delete[] g_pixelBuffer;

	// Release the sender
	sender.ReleaseSender();

	// Release OpenGL resources
	sender.CloseOpenGL();

    return (int) msg.wParam;
}

// SPOUT
void Render()
{

	// Trigger a re-paint to draw the bitmap and refresh the sending pixel buffer - see WM_PAINT
	InvalidateRect(g_hWnd, NULL, FALSE);
	UpdateWindow(g_hWnd); // Update immediately

	// LoadImage and Windows screen capture produces alpha 0 for the whole image.
	// Some applications display the received image as black,
	// so alpha of all the pixels should be converted to 255. 
	// See also : http://www.winprog.org/tutorial/transparency.html
	// The following function consumes approximately 1.5 msec at 1920x1080
	sender.spoutcopy.ClearAlpha(g_pixelBuffer, g_SenderWidth, g_SenderHeight, 255);

	//
	// Send the pixels
	//
	// SendImage manages sender create and re-size
	// Windows screen capture pixels are BGRA
	sender.SendImage(g_pixelBuffer, g_SenderWidth, g_SenderHeight, GL_BGRA);

	//
	// SPOUT - fps control
	//
	// Optionally hold a target frame rate - e.g. 60 or 30fps.
	// Not necessary if the application already has fps control
	// but in this example, render rate can be extremely high if 
	// graphics driver "wait for vertical refresh" is disabled.
	sender.HoldFps(60);

}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSPOUTGL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	// SPOUT
	// wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground  = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSPOUTGL);
    wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINSPOUTGL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Create window
   RECT rc = { 0, 0, 640, 360 }; // Desired client size
   AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, TRUE);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle,
   	   // SPOUT - enable resize and maximize to demonstrate sender resizing
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, 
	   rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // SPOUT
   // Centre the window on the desktop work area
   GetWindowRect(hWnd, &rc);
   RECT WorkArea;
   SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
   WindowPosLeft += ((WorkArea.right  - WorkArea.left) - (rc.right - rc.left)) / 2;
   WindowPosTop  += ((WorkArea.bottom - WorkArea.top)  - (rc.bottom - rc.top)) / 2;
   MoveWindow(hWnd, WindowPosLeft, WindowPosTop, (rc.right - rc.left), (rc.bottom - rc.top), false);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;
  
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
				case IDM_ABOUT:
				{
					std::string about = "                    Windows OpenGL\n";
					about += "           Spout image sender example\n\n";
					about += "                    <a href=\"http://spout.zeal.co/\">http://spout.zeal.co/</a>\n";
					// Custom icon for the SpoutMessagBox, activated by MB_USERICON
					SpoutMessageBoxIcon(LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_WINSPOUTGL)));
					// Centre on the application window
					SpoutMessageBoxWindow(hWnd);
					SpoutMessageBox(NULL, (LPSTR)about.c_str(), "WinSpoutGLsender", MB_USERICON | MB_OK);
				}
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	// ESC to quit
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	}

    case WM_PAINT:
        {
			if (IsIconic(hWnd))
				break;

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(g_hWnd, &ps);

			// For this example, draw an image, capture the client area
			// and copy pixel data to the sending buffer.

			//
			// Draw the image bitmap
			//
			
			// Get the client area
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			UINT rcWidth = (rcClient.right - rcClient.left);
			UINT rcHeight = (rcClient.bottom - rcClient.top);

			// Create a compatible memory DC and select the image bitmap into it
			HDC hdcMemDC = CreateCompatibleDC(hdc);
			SelectObject(hdcMemDC, g_hBitmap);

			// Stretch blit the image to the client area
			SetStretchBltMode(hdc, COLORONCOLOR); // Fastest method
			StretchBlt(hdc,	0, 0,
						rcWidth, rcHeight, // client size
						hdcMemDC, 0, 0,
						g_BitmapWidth, g_BitmapHeight, // image size
						SRCCOPY);

			//
			// Capture the client area for sending
			//

			// The window is re-sizable - check for client area size change
			if (rcWidth != g_SenderWidth || rcHeight != g_SenderHeight) {
				// Update the sender dimensions
				g_SenderWidth  = rcWidth;
				g_SenderHeight = rcHeight;
				// Re-size the the sending buffer to match
				if(g_pixelBuffer) delete[] g_pixelBuffer;
				unsigned int buffersize = g_SenderWidth * g_SenderHeight * 4;
				g_pixelBuffer = new unsigned char[buffersize];
			}
			// The sender is now the same size as the client area

			// Create a compatible bitmap sized to the client area
			HBITMAP hBitmap = CreateCompatibleBitmap(hdcMemDC, rcWidth, rcHeight);

			// Select the bitmap into the memory DC
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hBitmap);

			// Blit the client screen into it
			BitBlt(hdcMemDC, 0, 0,
				   rcClient.right - rcClient.left,
				   rcClient.bottom - rcClient.top,
				   hdc, 0, 0, SRCCOPY);

			// Copy to our sending buffer
			GetBitmapBits(hBitmap, rcWidth*rcHeight*4, (LPVOID)g_pixelBuffer);

			// Clean up
			SelectObject(hdcMemDC, hOldBitmap);
			DeleteObject(hBitmap);
			DeleteDC(hdcMemDC);
		
			EndPaint(hWnd, &ps);

        }
        break;

	case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// That's all..
