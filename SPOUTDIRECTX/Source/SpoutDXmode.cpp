//
//		SpoutDirectX
//
//		A dialog application to configure Spout DirectX mode
//
//			o Test for DirectX 9 availablility and provide buttons to link with Microsoft downloads
//			o Select DirectX 9 mode instead of default DirectX 11
//
//		03.06.15 - Started based on SpoutTray Version 2.08
//		07.06.15 - First working version
//		08.06.15 - Clean version 1.00 for DirectX 9 selection
//		01.07.15 - Ran into "Compatibility Assistant" trouble due to the app name
//					http://pete.akeo.ie/2011/10/avoiding-program-compatibility.html
//					https://technet.microsoft.com/en-us/library/cc709628.aspx					
//				 - renamed from SpoutSetup to SpoutDXmode
//		07.07.15 - Converted VS2012 project to VS2010
//
#include "stdafx.h"
#include "Spout.h"
#include "resource.h"

Spout spout; // Spout object
bool bDX9mode = false;  // Use DirectX 9 instead of DirectX 11 (default)

// Global Variables used
HWND g_hWndMain;
HINSTANCE g_hInst;
bool g_bDX9available = true; // Assume true. User cannot select DX9 mode if not available

// Forward declarations of functions included in this code module:
BOOL InitApplication(HANDLE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DirectXDialog(HWND, UINT, WPARAM, LPARAM);
bool CheckForDirectX9c();

// Static variables for the dialog
// Static variables for the dialog
static HWND ParamWnd = NULL;
static HBRUSH hbrBkgnd = NULL;
static bool bdx9 = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutDirectX\n");
	*/

	// Perform application initialization:
	if(!InitApplication(hInstance)) return (FALSE);
	if (!InitInstance(hInstance, nCmdShow)) return FALSE;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if(!IsWindow(ParamWnd) || !IsDialogMessage(ParamWnd, &msg)) {
			if (!IsDialogMessage(msg.hwnd,&msg) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	// Set global vars according to statics changed by the dialog
	if(bdx9 != bDX9mode) { // changed flag
		// MessageBoxA(NULL, "Changed", "Info", MB_OK);
		// If DX9 check for DirectX 9.0c
		if(bdx9 && !CheckForDirectX9c()) {
			MessageBoxA(NULL, "DirectX 9.0c not available", "Warning", MB_OK | MB_ICONEXCLAMATION);
		}
		else {
			// printf("IDM_DX9 - changing DX9 mode from %d to %d\n", bDX9mode, bdx9);
			if(bdx9) bDX9mode = true;
			else     bDX9mode = false;
			// Set DX9 mpde
			spout.SetDX9((DWORD)bDX9mode);
			// Write the DX9 mode to the registry. This is global for all apps
			spout.interop.WriteDwordToRegistry((DWORD)bDX9mode, "Software\\Leading Edge\\Spout", "DX9");
		}
	}

	return (int) msg.wParam;
}



/****************************************************************************
    FUNCTION: InitApplication(HANDLE)
    PURPOSE: Initializes window data and registers window class
****************************************************************************/
BOOL InitApplication(HANDLE hInstance)
{
  WNDCLASS  wc;

  wc.style         = 0;    
  wc.lpfnWndProc   = MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = (HINSTANCE)hInstance;
  wc.hIcon         = NULL; // LoadIcon((HINSTANCE)hInstance, L"SPOUTDIRECTXICON");
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = 0;  
  wc.lpszClassName = L"SpoutDirectXWClass";

  return (RegisterClass(&wc));   
}

//	Initialize the window and DirectX dialog
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HDC hdc = NULL;
	// HWND hwnd = NULL;
	// HWND hWnd = NULL;
	HGLRC hRc = NULL;
	char windowtitle[512];
	char graphicsCard[256];
	char installcaps[256];

	g_hInst = hInstance;

	// Create an invisible main window
	g_hWndMain = CreateWindowA("SpoutDirectXWClass",
                          "Spout DirectX",
                          WS_OVERLAPPEDWINDOW,
                          0,
                          0,
                          128,
                          128,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

	if(!g_hWndMain) return (FALSE);

	// Create openGL - we only need an OpenGL context with no window
	hdc = GetDC(g_hWndMain);
	if(!hdc) { 
		printf("InitOpenGL error 2\n"); 
		MessageBoxA(NULL, "Error 2\n", "InitOpenGL", MB_OK); 
		return FALSE; 
	}

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int iFormat = ChoosePixelFormat(hdc, &pfd);
	if(!iFormat) { 
		printf("InitOpenGL error 3\n"); 
		MessageBoxA(NULL, "Error 3\n", "InitOpenGL", MB_OK);
		return FALSE; 
	}

	if(!SetPixelFormat(hdc, iFormat, &pfd)) { 
		printf("InitOpenGL error 4\n"); 
		MessageBoxA(NULL, "Error 4\n", "InitOpenGL", MB_OK); 
		return FALSE; 
	}

	hRc = wglCreateContext(hdc);
	if(!hRc) { 
		printf("InitOpenGL error 5\n"); 
		MessageBoxA(NULL, "Error 5\n", "InitOpenGL", MB_OK); 
		return FALSE; 
	}
	
	wglMakeCurrent(hdc, hRc);
	if(wglGetCurrentContext() == NULL) {
		printf("InitOpenGL error 6\n");
		MessageBoxA(NULL, "Error 6\n", "InitOpenGL", MB_OK);
		return FALSE; 
	}

	// should check if opengl context creation succeed
	if(!hRc) {
		printf("InitOpenGL error 7\n");
		MessageBoxA(NULL, "Error 7\n", "InitOpenGL", MB_OK);
		return FALSE;
	}

	//
	// ======= DX9 and Adapter =======
	//
	// Get the DX9 flag
	// Spout will have read it from the registry when the sender object was created
	bDX9mode = spout.GetDX9();

	// Set static checkbox vars for dialog
	if(bDX9mode) bdx9 = true;
	else         bdx9 = false;

	// Check for DX9 availability
	if(CheckForDirectX9c())	
	 	g_bDX9available = true;
	 else 
		g_bDX9available = false;

	// LJ DEBUG for testing
	// g_bDX9available = false;

	// Create DirectX dialog and wait for OK / CANCEL
	// Modeless dialog
	if(!ParamWnd) ParamWnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DIRECTXBOX), NULL, (DLGPROC)DirectXDialog );

	return TRUE;
}

// Invisible main window
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
 
	switch (uMsg) {

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			return (DefWindowProc(hwnd, uMsg, wParam, lParam));

	}
	return (0L);
}



// Registry method only works for DirectX 9 and lower but that is OK
bool CheckForDirectX9c()
{

	// HKLM\Software\Microsoft\DirectX\Version should be 4.09.00.0904
	// handy information : http://en.wikipedia.org/wiki/DirectX
	HKEY  hRegKey;
	LONG  regres;
	DWORD  dwSize, major, minor, revision, notused;
	char value[256];
	dwSize = 256;

	// Does the key exist
	regres = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DirectX", NULL, KEY_READ, &hRegKey);
	if(regres == ERROR_SUCCESS) {
		// Read the key
		regres = RegQueryValueExA(hRegKey, "Version", 0, NULL, (LPBYTE)value, &dwSize);
		// Decode the string : 4.09.00.0904
		sscanf_s(value, "%d.%d.%d.%d", &major, &minor, &notused, &revision);
		// printf("DirectX registry : [%s] (%d.%d.%d.%d)\n", value, major, minor, notused, revision);
		RegCloseKey(hRegKey);
		if(major == 4 && minor == 9 && revision == 904)
			return true;
	}
	// else {
		// printf("RegOpenKey failed\n");
	// }

	return false;

    /*
	// Needs dsetup.h, dsetup.lib and dsetup.dll
	// dsetup.dll remains as a dependency
	DWORD dwVersion;
    DWORD dwRevision;
	DirectXSetupGetVersion( &dwVersion, &dwRevision );
    // Use HIWORD(dwVersion); to get the DirectX major version
    // Use LOWORD(dwVersion); to get the DirectX minor version
    // For example: for DirectX 5 dwVersion == 0x00040005
	// printf("Version 0x%08lX\nRevision %ld\n", dwVersion, dwRevision );
	// printf("Major %d, Minor %d\n", HIWORD(dwVersion), LOWORD(dwVersion));

	// Version
	// DirectX 9.0	0x00040009
	// Revision 904 - 9.0c
	// if(dwVersion == 0x00040009 && dwRevision == 904)
		// return true;
	*/

	return false;

}

// Message handler for DirectX dialog
LRESULT CALLBACK DirectXDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning
	char str1[MAX_PATH];
	HWND hwndList = NULL;
	int pos, lbItem;
	char name[128];

	switch (message) {
		// Owner draw button
		case WM_CTLCOLORSTATIC:
        {
			HDC hdcStatic = (HDC)wParam;
			// lParam is the handle to the control
			if(GetDlgItem(hDlg, IDC_DX9TEXT0) == (HWND)lParam) {
				if(g_bDX9available) { // Green to go
					SetTextColor(hdcStatic, RGB(0, 128, 0));
					SetBkColor(hdcStatic, RGB(0, 128, 0));
					SetDlgItemTextA(hDlg, IDC_DX9TEXT1, "DirectX 9.0c installed");
					SetDlgItemTextA(hDlg, IDC_DX9TEXT2, "( Use DirectX 9 functions instead of DirectX 11 )");
					// Hide DirectX install buttons
					ShowWindow(GetDlgItem(hDlg, IDC_DX9_INSTALL), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_DX9_DOWNLOAD), SW_HIDE);
				}
				else { // Red warning
					SetTextColor(hdcStatic, RGB(192, 0, 0));
					SetBkColor(hdcStatic, RGB(192, 0, 0));
					SetDlgItemTextA(hDlg, IDC_DX9TEXT1, "DirectX 9.0c not installed");
					// Hide DirectX checkbox and descripton
					ShowWindow(GetDlgItem(hDlg, IDC_DX9), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_DX9TEXT2), SW_HIDE);
				}
				if (hbrBkgnd == NULL)
					hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
				return (INT_PTR)hbrBkgnd;
			}
        }
		break;

		case WM_INITDIALOG:

			// Set the window icon
			HICON hIcon;
			hIcon = (HICON)LoadImage(g_hInst,
									MAKEINTRESOURCE(IDI_SPOUTICON),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXSMICON),
									GetSystemMetrics(SM_CYSMICON),
									0);
			if(hIcon)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			// Set checkboxes
			if(bdx9) CheckDlgButton(hDlg, IDC_DX9, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_DX9, BST_UNCHECKED);

			return TRUE; // return TRUE  unless you set the focus to a control

		case WM_COMMAND:

			switch(LOWORD(wParam)) {

				case IDC_DX9_INSTALL :
					// Web installer
					sprintf(str1, "https://www.microsoft.com/en-us/download/details.aspx?id=35");
					// sprintf(str1, "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe");
					ShellExecuteA(hDlg, "open", str1, NULL, NULL, SW_SHOWNORMAL); 
					DestroyWindow(hDlg);
					break;

				case IDC_DX9_DOWNLOAD :
					// Download
					sprintf(str1, "https://www.microsoft.com/en-us/download/details.aspx?id=8109");
					// sprintf(str1, "http://download.microsoft.com/download/8/4/A/84A35BF1-DAFE-4AE8-82AF-AD2AE20B6B14/directx_Jun2010_redist.exe");
					ShellExecuteA(hDlg, "open", str1, NULL, NULL, SW_SHOWNORMAL); 
					DestroyWindow(hDlg);
					break;

				case IDOK :
					// Set static DX9 flag according to checkboxes
					if(IsDlgButtonChecked(hDlg, IDC_DX9) == BST_CHECKED)
						bdx9 = true;
					else
						bdx9 = false;
					DestroyWindow(hDlg);
                    break;

				case IDCANCEL :
                    // User pressed cancel.  Just take down dialog box.
					DestroyWindow(hDlg);
					return TRUE;

				default:
					return FALSE;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hDlg);
			break;

		case WM_DESTROY:
			SendMessage(g_hWndMain, WM_DESTROY, 0, 0L);
			PostQuitMessage(0);
			break;
	}

	// return FALSE;
	return 0;
}
