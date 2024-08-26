/*
=========================================================================

                       ReceiverWindows.cpp

					Basic OpenGL Spout receiver

   A Windows Desktop Application project generated by Visual Studio
   and adapted for Spout input (http://spout.zeal.co/).

   This is an example using SpoutGL ReceiveImage

   bool ReceiveImage(unsigned char* pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)

   Receives to a pixel buffer and displays the output using Windows bitmap functions.

   Search on "SPOUT" for additions.

   Compare with the Openframeworks "Receiver > Graphics" example
   Compare also with the basic Windows OpenGL sender example.

	---------------------------------------------------------------------------

                 Copyright(C) 2024 Lynn Jarvis.

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
========================================================================

*/
#include "framework.h"
#include "ReceiverWindows.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                       // current instance
WCHAR szTitle[MAX_LOADSTRING]{};       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]{}; // the main window class name

// SPOUT
Spout receiver;                        // Receiver object
HWND g_hWnd = NULL;                    // Window handle
unsigned char *pixelBuffer = nullptr;  // Receiving pixel buffer
unsigned char g_SenderName[256]{};     // Received sender name
unsigned int g_SenderWidth = 0;        // Received sender width
unsigned int g_SenderHeight = 0;       // Received sender height
DWORD g_SenderFormat = 0;              // Received sender format

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

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSPOUTGL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	// SPOUT
	// Initialize OpenGL
	if (!receiver.CreateOpenGL())
		return FALSE;

	// Option - enable Spout logging
	// EnableSpoutLog(); // Log to console
	// Or open an empty console for debugging
	// OpenSpoutConsole();

	// Option - set the name of the sender to receive from
	// The receiver will only connect to that sender.
	// The user can over-ride this by selecting another.
	// receiver.SetReceiverName("Spout DX11 Sender");
	
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
	if (pixelBuffer) delete[] pixelBuffer;

	// Release the receiver
	receiver.ReleaseReceiver();

	// Release OpenGL resources
	receiver.CloseOpenGL();

    return (int) msg.wParam;
}

// SPOUT
void Render()
{
	//
	// ReceiveImage connects to and receives from a sender
	// and handles sender detection, receiver creation and update.
	//
	// For successful receive, sender details can be retrieved with
	//		const char * GetSenderName();
	//		unsigned int GetSenderWidth();
	//		unsigned int GetSenderHeight();
	//		DWORD GetSenderFormat();
	//		double GetSenderFps();
	//		long GetSenderFrame();
	//
	// If receive fails, the sender has closed
	// Connection can be tested at any time with 
	//		bool IsConnected();
	//

	// ----------------------------------------------------------------------------
	// Compare with the Openframeworks ofSpoutExample "Receiver > Graphics" example
	// for additional options :
	//
	// 1) Allocate the receiving texture with an OpenGL format
	//	  compatible with the sender DirectX shared texture format.
	//	  The Format can be GL_RGBA, GL_RGBA16, GL_RGBA16F or GL_RGBA32F.
	//			glformat = receiver.GLDXformat();
	//
	// 2) Receive a sender OpenGL shared texture to access directly
	//		The example copies from the shared texture.
	//		The texture binding may also be used directly for rendering.
	//
	// ----------------------------------------------------------------------------

	//
	// Get pixels from the sender shared texture
	//
	// Format can be GL_RGBA, GL_BGRA, GL_RGB or GL_BGR for the receving buffer.
	// Specify GL_BGRA for this example to match with Windows bitmap draw in WM_PAINT.
	// Data received from a sender is then converted to BGRA for return. 
	//
	// Because Windows bitmaps are bottom-up, the pixel buffer is flipped
	// here ready for WM_PAINT but it could also be drawn upside down.
	if (receiver.ReceiveImage(pixelBuffer, GL_BGRA, true)) {
		
		// IsUpdated() returns true if the sender has changed
		if (receiver.IsUpdated()) {

			// Update the sender name - it could be different
			strcpy_s((char *)g_SenderName, 256, receiver.GetSenderName());

			// Update globals
			g_SenderWidth = receiver.GetSenderWidth();
			g_SenderHeight = receiver.GetSenderHeight();
			g_SenderFormat = receiver.GetSenderFormat();

			// Update the receiving buffer
			if(pixelBuffer)	delete[] pixelBuffer;
			unsigned int buffersize = g_SenderWidth * g_SenderHeight * 4;
			pixelBuffer = new unsigned char[buffersize];

			// Do anything else necessary for the application here

		}
	}

	// Trigger a re-paint to draw the pixel buffer - see WM_PAINT
	InvalidateRect(g_hWnd, NULL, FALSE);
	UpdateWindow(g_hWnd); // Update immediately

	// Hold a target frame rate - e.g. 60 or 30fps.
	// Not necessary if the application already has fps control
	// but in this example, render rate can be extremely high if 
	// graphics driver "wait for vertical refresh" is disabled.
	receiver.HoldFps(60);

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
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE); // Allow for menu
   HWND hWnd = CreateWindowW(szWindowClass,
	   szTitle,
	   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
	   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
	   nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // SPOUT
   // Centre the window on the desktop work area
   GetWindowRect(hWnd, &rc);
   RECT WorkArea;
   int WindowPosLeft = 0;
   int WindowPosTop = 0;
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
					std::string about = "                  Windows OpenGL\n";
					about += "             Spout receiver example\n\n";
					about += " 'File > Select sender' or mouse right click\n";
					about += "    to choose the sender to receive from.\n\n";
					about += "               <a href=\"http://spout.zeal.co/\">http://spout.zeal.co</a>\n"; 
					// Custom icon for the SpoutMessagBox, activated by MB_USERICON
					SpoutMessageBoxIcon(LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_WINSPOUTGL)));
					// Centre on the application window
					SpoutMessageBoxWindow(hWnd);
					SpoutMessageBox(NULL, (LPSTR)about.c_str(), "ReceiverWindows", MB_USERICON | MB_OK);
				}
				break;

			case IDM_SELECT:
				//
				// Select a sender
				//
				// The change is detected withion the Spout class
				// and the receiver switches to that sender.
				// IsUpdated() returns true so that resources can be updated
				// See details in Render()
				//
				receiver.SelectSender();
				break;

			case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// SPOUT
			if (pixelBuffer) {

				//
				// Draw the received image
				//

				RECT dr = { 0 };
				GetClientRect(hWnd, &dr);

				// No sender - draw default background
				if (!receiver.IsConnected()) {
					HBRUSH backbrush = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
					FillRect(hdc, &dr, backbrush);
					DeleteObject(backbrush);
				}
				else {
					BITMAPINFO bmi;
					ZeroMemory(&bmi, sizeof(BITMAPINFO));
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biSizeImage = (LONG)(g_SenderWidth * g_SenderHeight * 4); // Pixel buffer size
					bmi.bmiHeader.biWidth = (LONG)g_SenderWidth;   // Width of buffer
					bmi.bmiHeader.biHeight = (LONG)g_SenderHeight;  // Height of buffer
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					//
					// ReceiveImage receives data of format GL_BGRA
					// which matches with StretchDIBits. If received data is RGBA,
					// the extended BITMAPV4HEADER bitmap info header can be used
					// but is not necessary here.
					//
					// StretchDIBits adapts the pixel buffer received from the sender
					// to the window size. The sender can be resized or changed.
					//
					SetStretchBltMode(hdc, COLORONCOLOR); // Fastest method
					StretchDIBits(hdc,
							0, 0, (dr.right - dr.left), (dr.bottom - dr.top), // destination rectangle 
							0, 0, g_SenderWidth, g_SenderHeight, // source rectangle 
							pixelBuffer,
							&bmi, DIB_RGB_COLORS, SRCCOPY);
				}
			}
			EndPaint(hWnd, &ps);
        }
        break;

	// SPOUT
	case WM_RBUTTONDOWN:
		// Right mouse click to select a sender
		// See also IDM_SELECT
		receiver.SelectSender();
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