//
//		SpoutDXmode
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
//		16.09.15 - Recompile with revised memoryshare class 
//				 - Sorted out compatibilty tests and memoryshare mode
//		05-10-15 - included NVIDIA nvapi functions to set NVIDIA as global for Optimus systems.
//				 - TODO : test on Optimus systems.
//		12-11-15 - remove profiling and compile for 2.005
//		15.12.15 - Change to a combo-box for user selection of NVIDIA mode
//				   Detect current settings from NVIDIA base profile rather than the registry
//		16.12.15 - build for 2.005 release as SpoutDirectX.exe
//		24.02.16 - updated SpoutDirectX icon file to organize layers properly
//		26.03.16 - Changed caption to SpoutDXmode
//		02.06.16 - Rebuild for 2.005 release VS2012 /MT - SpoutDXmode.exe
//		14.01.17 - Rebuild for 2.006
//				 - Change options to radio buttons
//		21.01.17 - Rebuild /MD to avoid virus false positive - still postive for Qihoo-360
//
#include "stdafx.h"
#include "Spout.h"
#include "SpoutOptimus.h" // NVIDIA profile settings
#include "resource.h"

Spout spout; // Spout object
bool bDX9mode = false;  // Use DirectX 9 instead of default DirectX 11
bool bCPUmode = false;  // Use CPU processing instead of default Graphics
bool bMemorymode = false;  // Use MemoryShare instead of default DirectX
bool bBuffermode = false; // OpenGL PBO buffering

// NVAPI object to force NVIDIA
nVidia g_NvApi;
int NvidiaMode = 0; // Optimus graphics mode

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
static HWND ParamWnd = NULL;
static HBRUSH hbrBkgnd = NULL;
static bool bdx9 = false;
static bool bcpu = false;
static bool bmemory = false;
static bool bbuffer = false;
static int nvidiamode = 0;
static int comboindex = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutDXmode\n");
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
		// If DX9 check for DirectX 9.0c
		if(bdx9 && !CheckForDirectX9c()) {
			MessageBoxA(NULL, "DirectX 9.0c not available", "Warning", MB_OK | MB_ICONEXCLAMATION);
		}
		else {
			if(bdx9) bDX9mode = true;
			else     bDX9mode = false;
			// Set DX9 mode
			spout.SetDX9((DWORD)bDX9mode);
			// Write the DX9 mode to the registry. This is global for all apps
			spout.interop.spoutdx.WriteDwordToRegistry((DWORD)bDX9mode, "Software\\Leading Edge\\Spout", "DX9");
		}
	}

	// printf("bcpu = %d, bmemory = %d, bbuffer = %d\n", bcpu, bmemory, bbuffer);

	//
	// CPU texture processing
	//
	if(bcpu) bCPUmode = true;
	else     bCPUmode = false;
	spout.interop.spoutdx.WriteDwordToRegistry((DWORD)bCPUmode, "Software\\Leading Edge\\Spout", "CPU");

	//
	// Memoryshare
	//
	if(bmemory) bMemorymode = true;
	else        bMemorymode = false;
	spout.interop.spoutdx.WriteDwordToRegistry((DWORD)bMemorymode, "Software\\Leading Edge\\Spout", "MemoryShare");

	//
	// OpenGL PBO buffering
	//
	if(bbuffer) bBuffermode = true;
	else        bBuffermode = false;
	spout.interop.spoutdx.WriteDwordToRegistry((DWORD)bBuffermode, "Software\\Leading Edge\\Spout", "Buffering");


	//
	// NVIDIA Optimus
	//
	if(nvidiamode != NvidiaMode) { // user has changed
		NvidiaMode = nvidiamode;
		g_NvApi.ActivateNVIDIA(NvidiaMode); // will just fail for unsupported hardware
	}

	// MessageBoxA(NULL, "Finished", "SpoutDXmode", MB_OK);

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
  wc.lpszClassName = L"SpoutDXmodeWClass";

  return (RegisterClass(&wc));   
}

//	Initialize the window and DirectX dialog
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	UNREFERENCED_PARAMETER(nCmdShow);

	HDC hdc = NULL;
	HGLRC hRc = NULL;
	char windowtitle[512];
	char graphicsCard[256];
	char installcaps[256];

	g_hInst = hInstance;

	// Create an invisible main window
	g_hWndMain = CreateWindowA("SpoutDXmodeWClass",
                          "SpoutDXmode",
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

	// Get the current CPU texture processing mode
	bCPUmode = spout.GetCPUmode();

	// Get the current Memoryshare mode
	// Performs a compatibilty test and sets memorymode to the registry if not compatible
	// The compatibility check is repeated after user selection.
	bMemorymode = spout.GetMemoryShareMode();

	// Get the current PBO buffering mode
	bBuffermode = spout.interop.GetBufferMode();

	// Get the current Optimus NVIDIA setting from the NVIDIA base profile
	// will just fail for unsupported hardware and return 0
	NvidiaMode = g_NvApi.GetNVIDIA(); // 0 - nvidia, 1 - integrated or 2 - auto-select

	// Set static checkbox vars for dialog
	if(bDX9mode) bdx9 = true;
	else         bdx9 = false;

	if(bCPUmode) bcpu = true;
	else         bcpu = false;

	if(bMemorymode) bmemory = true;
	else            bmemory = false;

	if(bBuffermode) bbuffer = true;
	else            bbuffer = false;

	// printf("bcpu = %d, bmemory = %d, bbuffer = %d\n", bcpu, bmemory, bbuffer);

	nvidiamode = comboindex = NvidiaMode; // to check for changes

	// Check for DX9 availability
	if(CheckForDirectX9c())	
	 	g_bDX9available = true;
	 else 
		g_bDX9available = false;

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
	char prefs[3][128] =  { "High performance", "Integrated", "Auto select" };

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
					SetDlgItemTextA(hDlg, IDC_DX9TEXT2, "( DirectX 9 functions )");
					SetDlgItemTextA(hDlg, IDC_TEXTEXT,  "( GPU texture processing )");
					SetDlgItemTextA(hDlg, IDC_CPUTEXT,  "( CPU texture processing )");
					SetDlgItemTextA(hDlg, IDC_MEMTEXT,  "( Shared memory )");
					SetDlgItemTextA(hDlg, IDC_BUFTEXT,  "( OpenGL pixel buffering )");

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
			// http://blog.barthe.ph/2009/07/17/wmseticon/
			HICON hIcon1;
			hIcon1 = (HICON)LoadImage(g_hInst,
									MAKEINTRESOURCE(IDI_SPOUTICON),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXICON), // SM_CXSMICON),
									GetSystemMetrics(SM_CXICON), // SM_CYSMICON),
									// GetSystemMetrics(SM_CXSMICON),
									// GetSystemMetrics(SM_CYSMICON),
									0);
			if(hIcon1)
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon1); // ICON_SMALL
					SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon1); // ICON_SMALL

			HICON hIcon2;
			hIcon2 = (HICON)LoadImage(g_hInst,
									MAKEINTRESOURCE(IDI_SPOUTICON),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXSMICON),
									GetSystemMetrics(SM_CYSMICON),
									0);
			if(hIcon2)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon2);

			// Set checkboxes
			if(bdx9) CheckDlgButton(hDlg, IDC_DX9, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_DX9, BST_UNCHECKED);

			if(bbuffer) CheckDlgButton(hDlg, IDC_BUF, BST_CHECKED);
			else        CheckDlgButton(hDlg, IDC_BUF, BST_UNCHECKED);

			// Set radio buttons
			if(bcpu)
				CheckRadioButton(hDlg, IDC_TEX, IDC_MEM, IDC_CPU);
			else if(bmemory) 
				CheckRadioButton(hDlg, IDC_TEX, IDC_MEM, IDC_MEM);
			else       
				CheckRadioButton(hDlg, IDC_TEX, IDC_MEM, IDC_TEX);

			hwndList = GetDlgItem(hDlg, IDC_NVIDIA);
			for (int k = 0; k < 3; k ++) {
				strcpy_s(name, sizeof(name),  prefs[k]);
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}
			
			// Display an initial item in the selection field  
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)comboindex, (LPARAM)0);


			return TRUE; // return TRUE  unless you set the focus to a control

		case WM_COMMAND:

			// Combo box selection
			if(HIWORD(wParam) == CBN_SELCHANGE)
				comboindex = SendMessage((HWND)lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				
			switch(LOWORD(wParam)) {

				case IDC_DX9_INSTALL :
					//
					// Web installer
					//
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

				/*
				case IDC_CPU :
					if(IsDlgButtonChecked(hDlg, IDC_CPU) == BST_CHECKED)
						EnableWindow(GetDlgItem(hDlg, IDC_MEM), FALSE);
					else
						EnableWindow(GetDlgItem(hDlg, IDC_MEM), TRUE);
					break;

				case IDC_MEM :
					if(IsDlgButtonChecked(hDlg, IDC_MEM) == BST_CHECKED)
						EnableWindow(GetDlgItem(hDlg, IDC_CPU), FALSE);
					else
						EnableWindow(GetDlgItem(hDlg, IDC_CPU), TRUE);
					break;
				*/


				case IDOK :
					// Set static flags according to checkboxes
					if(IsDlgButtonChecked(hDlg, IDC_DX9) == BST_CHECKED)
						bdx9 = true;
					else
						bdx9 = false;

					if(IsDlgButtonChecked(hDlg, IDC_CPU) == BST_CHECKED)
						bcpu = true;
					else
						bcpu = false;

					if(IsDlgButtonChecked(hDlg, IDC_MEM) == BST_CHECKED)
						bmemory = true;
					else
						bmemory = false;

					if(IsDlgButtonChecked(hDlg, IDC_TEX) == BST_CHECKED) {
						printf("Texture share\n");
						bcpu = false;
						bmemory = false;
					}


					if(IsDlgButtonChecked(hDlg, IDC_BUF) == BST_CHECKED)
						bbuffer = true;
					else
						bbuffer = false;

					nvidiamode = comboindex; // set global NVIDIA preference flag

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
