//
//		SpoutTray
//
//		A system tray application to show and choose Spout Senders
//
//		Based on : http://www.codeproject.com/Articles/4768/Basic-use-of-Shell_NotifyIcon-in-Win32
//
//		30-12-13 -	Glew support included in spxConnector
//		22.01.14 -	Noted a crash on click of OK with Windows 8 - try not releasing wglDXOpenDeviceNV
//				 -	temp declared twice - removed
//		31.01.14 - used ID_ACPS dialog instead of main dialog, trying o strace a crash with Windows 8
//				 - maybe a string length problem
//				 - changed all sprintf to sprintf_s
//		04.02.14 - used local char array instead of global for SetDlgItemText in IDC_CAPS dialog
//                 and filled data locally every time
//		28.02.14 - removed caps dialog after successful Windows 8 debug
//				 - cleanup
//				 - Version 1.01
//		01.03.14 - revised spxConnector including FBO blit and test for FBO blit capability
//				 - Version 1.02
//		15.04.14 - used revised spxConnector
//				 - changed all Sender to Sender
//		03.08.14 - converted to Spout SDK - Version 2.00
//		30.08.14 - recompiled using MB sendernames revision
//		03.09.14 - update after SDK testing
//				 - Version 2.01
//		29.09.14 - update with with SDK revision
//		30.09.14 - Additional diagnostics
//				 - Version 2.02
//		12.10.14 - recompiled for release
//				 - Version 2.03
//		20.10.14 - Recompile for update V 2.001 beta with detection of DirectX 11 compatibility
//				 - Version 2.04
//		02.01.15 - Recompile for SDK changes - removed dependence on Glew
//				 - Version 2.05
//
#define GLEW_STATIC // to use glew32s.lib instead of glew32.lib otherwise there is a redefinition error

#include <glew.h>
#include <wglew.h> // wglew.h and glxew.h, which define the available WGL and GLX extensions
#include <glut.h>
#include <gl/gl.h>

#include "stdafx.h"
#include "..\Spout.h"
#include "resource.h"

#define TRAYICONID	1//				ID number for the Notify Icon
#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window

#define SWM_SHOW	WM_APP + 10//	show the window
#define SWM_HIDE	WM_APP + 12//	hide the window
#define SWM_EXIT	WM_APP + 13//	close the window

// Global Variables:
HINSTANCE		hInst;	// current instance
NOTIFYICONDATA	niData;	// notify icon data
HWND hWndMain;

// Spout sender variables
Spout spout;
SharedTextureInfo info;
std::set<string> Senders;
std::set<string>::iterator iter;
string namestring;
char name[512];
char temp[1024];
char gldxcaps[1024]; // capability info

// Forward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE, int);
BOOL				OnInitDialog(HWND hWnd);
void				ShowContextMenu(HWND hWnd);
ULONGLONG			GetDllVersion(LPCTSTR lpszDllName);

INT_PTR CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_STEALTHDIALOG);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)||
			!IsDialogMessage(msg.hwnd,&msg) ) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

//	Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HDC GLhdc;
	int argc = 1;
	char *argv = (char*)"SpoutSenders";
	char **vptr = &argv;

	// prepare for XP style controls
	InitCommonControls();

	 // store instance handle and create dialog
	hInst = hInstance;
	HWND hWnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DLG_DIALOG), NULL, (DLGPROC)DlgProc );
	if (!hWnd) return FALSE;

	// Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon

	// zero the structure - note:	Some Windows funtions require this but
	//								I can't be bothered which ones do and
	//								which ones don't.
	ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

	// get Shell32 version number and set the size of the structure
	//		note:	the MSDN documentation about this is a little
	//				dubious and I'm not at all sure if the method
	//				bellow is correct
	ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));

	if(ullVersion >= MAKEDLLVERULL(5, 0,0,0))
		niData.cbSize = sizeof(NOTIFYICONDATA);
	else 
		niData.cbSize = NOTIFYICONDATA_V2_SIZE;

	// the ID number can be anything you choose
	niData.uID = TRAYICONID;

	// state which structure members are valid
	// niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_STATE; // LJ DEBUG
	niData.dwState = 0x00000000; // not hidden
	niData.dwStateMask = NIS_HIDDEN; // alow only the hidden state to be modified - we have no notifications

	// load the icon
	niData.hIcon = (HICON)LoadImage(hInstance,
									MAKEINTRESOURCE(IDI_STEALTHDLG),
									IMAGE_ICON, 
									GetSystemMetrics(SM_CXSMICON),
									GetSystemMetrics(SM_CYSMICON),
									LR_DEFAULTCOLOR);

	// the window to send messages to and the message to send
	//		note:	the message value should be in the
	//				range of WM_APP through 0xBFFF
	niData.hWnd = hWnd;
    niData.uCallbackMessage = SWM_TRAYMSG;

	wchar_t  ws[512];
	wsprintf(ws, L"SpoutTray");
	lstrcpyn(niData.szTip, ws, sizeof(niData.szTip)/sizeof(TCHAR));
	Shell_NotifyIcon(NIM_ADD, &niData);

	// free icon handle
	if(niData.hIcon && DestroyIcon(niData.hIcon))
		niData.hIcon = NULL;

	// Fill capability info
	// Only need to do this once
	GLhdc = wglGetCurrentDC();
	if(!GLhdc) {

		// ======= Hardware compatibility test =======
		// Get the Windows version.
		DWORD dwVersion = 0; 
		DWORD dwMajorVersion = 0;
		DWORD dwMinorVersion = 0; 
		DWORD dwBuild = 0;
		DWORD dwPlatformId;
		WORD wServicePackMajor = 0;
	    WORD wServicePackMinor = 0;
		WORD wSuiteMask = 0;
		BYTE wProductType = 0;

		bool bOsviX = true;
		bool bCanDetect = true;
		char output[256];
		size_t charsConverted = 0;
	
		OSVERSIONINFOEX osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	    if(!GetVersionEx((OSVERSIONINFO *)&osvi)) {
			bOsviX = false;
			// If that fails, try using the OSVERSIONINFO structure.
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if(!GetVersionEx((OSVERSIONINFO *)&osvi))
				bCanDetect = false;

		}

		if(bCanDetect) {

			dwBuild = osvi.dwBuildNumber;
			dwMajorVersion = osvi.dwMajorVersion;
			dwMinorVersion = osvi.dwMinorVersion;
			dwPlatformId = osvi.dwPlatformId;
			// printf("Version is %d.%d Build (%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);
			if(dwMajorVersion < 6) {
				strcat_s(gldxcaps, 1024, "Windows XP - ");
			}
			else {
				switch (dwMinorVersion) {
					case 0 : // vista
						strcat_s(gldxcaps, 1024, "Windows Vista - ");
						break;
					case 1 : // 7
						strcat_s(gldxcaps, 1024, "Windows 7 - ");
						break;
					case 2 : // 8
						strcat_s(gldxcaps, 1024, "Windows 8 - ");
						break;
					case 3 : // 8.1
						strcat_s(gldxcaps, 1024, "Windows 8.1 - ");
						break;
					default : // Later than October 2013
						strcat_s(gldxcaps, 1024, "Windows - ");
						break;
				}
			}
		}

		/*
		if(bOsviX) {
			// Here we can get additional service pack and other information
			wServicePackMajor = osvi.wServicePackMajor;
			wServicePackMinor = osvi.wServicePackMinor;
			sprintf_s(output, 256, "SP %d.%d - ", wServicePackMajor, wServicePackMinor);
			strcat_s(gldxcaps, 1024, output);
		}
		*/

		// DirectX 11 only available for Windows 7 (6.1) and higher
		#ifdef is64bit
				strcat_s(gldxcaps, 1024, "64bit\r\n");
		#else
				strcat_s(gldxcaps, 1024, "32bit\r\n");
		#endif

		// Additional info
		DISPLAY_DEVICE DisplayDevice;
		DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
		if(EnumDisplayDevices(NULL, 0, &DisplayDevice, 0)) {
			wcstombs_s(&charsConverted, output, 129, DisplayDevice.DeviceKey, 128);
	
			HKEY hRegKey;
			DWORD dwSize, dwKey;  
			// Extract the subkey from the DeviceKey string
			string SubKey = strstr(output, "System");

			// Convert all slash to double slash using a C++ string function
			// to get subkey string required to extract registry information
			for (unsigned int i=0; i<SubKey.length(); i++) {
				if (SubKey[i] == '\\') {
					SubKey.insert(i, 1, '\\');
					++i; // Skip inserted char
				}
			}

			// Open the key to find the adapter details
			if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey.c_str(), NULL, KEY_READ, &hRegKey) == 0) { 
				dwSize = MAX_PATH;
				// Adapter name
				if(RegQueryValueExA(hRegKey, "DriverDesc", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
					strcat_s(gldxcaps, 1024, output);
					strcat_s(gldxcaps, 1024, "\r\n");
				}
				if(RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
					strcat_s(gldxcaps, 1024, "Driver : ");
					// Find the last 6 characters of the version string then
					// convert to a float and multiply to get decimal in the right place
					sprintf_s(output, 256, "%5.2f", atof(output + strlen(output)-6)*100.0);
					strcat_s(gldxcaps, 1024, output);
				} // endif DriverVersion
				RegCloseKey(hRegKey);
			} // endif RegOpenKey
		} // endif EnumDisplayDevices

		// Bits per pixel
		HDC hDC=GetDC(hWnd);
		if (hDC) {
			int bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL); //to get current system's color depth
			sprintf_s(output, 256, " (%d bpp)", bitsPerPixel);
			strcat_s(gldxcaps, 1024, output);
		}
		strcat_s(gldxcaps, 1024, "\r\n");

		// Now we can call an initial hardware compatibilty check
		// This checks for the NV_DX_interop extensions and will fail
		// if the graphics deliver does not support them, or fbos

		// Initialize glew so that extensions can be found - only needed for this dialog
		// You need to create a rendering context BEFORE calling glewInit()
		// First you need to create a valid OpenGL rendering context and call glewInit() 
		// to initialize the extension entry points, so create a window here but it will not show.
		glutInit(&argc, vptr);
		glutCreateWindow("SpoutSendersGL");
		HGLRC glContext = wglGetCurrentContext(); // should check if opengl context creation succeed
		if(glContext) {
			// ======================================================
			if(wglGetProcAddress("wglDXOpenDeviceNV")) { // extensions loaded OK
				// It is possible that the extensions load OK, but that initialization will still fail
				// This occurs when wglDXOpenDeviceNV fails - noted on dual graphics machines with NVIDIA Optimus
				// Directx initialization seems OK with null hwnd, but in any case we will not use it.
				// printf("GetMemoryShareMode\n");
				bool bMem = spout.GetMemoryShareMode();
				if (!bMem) { // Test for memoryshare initialization
					if(wglGetProcAddress("glBlitFramebufferEXT"))
						strcat_s(gldxcaps, 1024, "Compatible hardware\r\nNV_DX_interop extensions supported\r\nInterop load successful\r\nTexture sharing mode available\r\nFBO blit available");
					else
						strcat_s(gldxcaps, 1024, "Compatible hardware\r\nNV_DX_interop extensions supported\r\nInterop load successful\r\nTexture sharing mode available\r\nFBO blit not available");
				}
				else {
					strcat_s(gldxcaps, 1024,  "Compatible hardware\r\nNV_DX_interop extensions supported\r\nbut wglDXOpenDeviceNV failed to load\r\nLimited to memory share mode");
				}
			}
			else {
				// Determine whether fbo support is the reason or interop
				if(!wglGetProcAddress("glGenFramebuffersEXT"))
					strcat_s(gldxcaps, 1024,  "Hardware does not support EXT_framebuffer_object extensions\r\nTexture sharing not available\r\nLimited to memory share mode");
				else
					strcat_s(gldxcaps, 1024,  "Hardware does not support NV_DX_interop extensions\r\nTexture sharing not available\r\nLimited to memory share mode");
			}
		}
		else {
			strcat_s(gldxcaps, 1024,  "No GL context");
		}
	}

	SetDlgItemTextA(hWnd, IDC_INFOTEXT, (LPCSTR)gldxcaps);

	// call ShowWindow here to make the dialog initially visible
	// ShowWindow(hWnd, SW_SHOWNORMAL);

	return TRUE;
}

BOOL OnInitDialog(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	/// ??? hMenu does not exist yet
	/*
	if (hMenu) {
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
	}
	*/

	HICON hIcon = (HICON)LoadImage(	hInst,
									MAKEINTRESOURCE(IDI_STEALTHDLG),
									IMAGE_ICON, 
									0, 0, 
									LR_SHARED|LR_DEFAULTSIZE);

	SendMessage(hWnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);
	SendMessage(hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);

	return TRUE;
}


// Name says it all
void ShowContextMenu(HWND hWnd)
{
	int item;
	POINT pt;
	char activename[512];
	char itemstring[512];

	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();

	if(hMenu) {

		// Insert all the Sender names as menu items here
		if(spout.interop.senders.GetSenderNames(&Senders)) {
			
			// Now we have a local set of names "Senders"
			// 27.12.13 - noted that if a Processing sketch is stopped by closing the window
			// all is OK and either the "stop" or "dispose" overrides work, but if STOP is used, 
			// or the sketch is closed, neither the exit or dispose functions are called and
			// the sketch does not release the sender.
			// So here we run through again and check whether the sender exists and if it does not
			// release the sender from the local sender list
			if(Senders.size() > 0) {
				for(iter = Senders.begin(); iter != Senders.end(); iter++) {
					namestring = *iter; // the Sender name string
					strcpy_s(name, namestring.c_str());
					// we have the name already, so look for it's info
					if(!spout.interop.senders.getSharedInfo(name, &info)) {
						// Sender does not exist any more
						spout.interop.senders.ReleaseSenderName(name); // release from the shared memory list
					}
				}
			}

			// Now we have cleaned up the list in shared memory, so get it again
			Senders.clear();
			spout.interop.senders.GetSenderNames(&Senders);

            // Add all the Sender names as items to the dialog list.
			if(Senders.size() > 0) {
				// Get the active Sender name
				spout.interop.senders.GetActiveSender(activename);
				item = 0;
				for(iter = Senders.begin(); iter != Senders.end(); iter++) {
					namestring = *iter; // the string to copy
					strcpy_s(name, namestring.c_str());
					// Find it's width and height
					spout.interop.senders.getSharedInfo(name, &info);
					sprintf_s(itemstring, "%s : (%d x %d)", name, info.width, info.height);
					InsertMenuA(hMenu, -1, MF_BYPOSITION, WM_APP+item, itemstring);
					// Was it the active Sender
					if(strcmp(name, activename) == 0)
						CheckMenuItem (hMenu, WM_APP+item, MF_BYCOMMAND | MF_CHECKED);
					item++;
				} // end menu item loop
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			} // endif Senders size > 0
		} // end go the Sender names OK

		// Dialog
		if( IsWindowVisible(hWnd) )
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, _T("Hide info")); // ????
		else
			InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, _T("Show info"));

		AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));

		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		SetForegroundWindow(hWnd);

		TrackPopupMenu(	hMenu, 
						TPM_BOTTOMALIGN,
						pt.x, pt.y, 
						0, hWnd, NULL );
		
		DestroyMenu(hMenu);

	}
}

// Get dll version number
ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
    ULONGLONG ullVersion = 0;
	HINSTANCE hinstDll;
    hinstDll = LoadLibrary(lpszDllName);

    if(hinstDll) {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        if(pDllGetVersion) {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if(SUCCEEDED(hr))
				ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion,0,0);
        }
        FreeLibrary(hinstDll);
    }
    return ullVersion;
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, ID;
	HMENU hMenu;

	hMenu = GetMenu(hWnd);

	switch (message) {
	
	case SWM_TRAYMSG:

		switch(lParam) {
		
			case WM_LBUTTONDBLCLK:
				ShowWindow(hWnd, SW_RESTORE);
				break;
		
			case WM_RBUTTONDOWN:
		
			case WM_CONTEXTMENU:
				ShowContextMenu(hWnd);
		
		}
		break;

	case WM_SYSCOMMAND:
		if((wParam & 0xFFF0) == SC_MINIMIZE) {
			ShowWindow(hWnd, SW_HIDE);
			return 1;
		}
		break;

	case WM_COMMAND:
		
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam); 
		ID		= LOWORD(wParam) - WM_APP;

		// ID has the item number of the selected Sender - make it acive
		if(Senders.size() > 0 && ID < 10) {
			if(ID < (int)Senders.size()) {
				iter = std::next(Senders.begin(), ID);
				namestring = *iter; // the selected Sender in the list
				strcpy_s(name, namestring.c_str());
				spout.interop.senders.SetActiveSender(name);
			}
		}

		switch (wmId) {

			case SWM_SHOW:
				ShowWindow(hWnd, SW_RESTORE);
				break;

			case SWM_HIDE:
			case IDOK:
				ShowWindow(hWnd, SW_HIDE);
				break;

			case IDC_COPY :

				if(OpenClipboard(NULL)) {
					HGLOBAL clipbuffer;
					char* buffer;
			        EmptyClipboard();
					clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(gldxcaps)+1);
					buffer = (char*)GlobalLock(clipbuffer);
					strcpy_s(buffer, strlen(gldxcaps)+1, LPCSTR(gldxcaps));
					GlobalUnlock(clipbuffer);
			        SetClipboardData(CF_TEXT, clipbuffer);
					GlobalFree(clipbuffer);
					CloseClipboard();
					MessageBoxA(hWnd, "Diagnostics copied to the clipboard.", "Spout Demo", MB_OK);
				}
				else {
					MessageBoxA(hWnd, "Unknown clipboard open error.", "Spout Demo", MB_OK);
				}
				return TRUE;

			case SWM_EXIT:
				DestroyWindow(hWnd);
				break;
		
			case IDM_ABOUT:
				DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				break;

		}

		return 1;

	case WM_INITDIALOG:
		return OnInitDialog(hWnd);

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		niData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &niData);
		PostQuitMessage(0);
		break;
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		
		case WM_INITDIALOG:
			// Changeable text here
			sprintf_s(temp,  1024, "SpoutTray - Version 2.05\n\nSelect a sender for sharing frames with receivers\nThe sender selected becomes the 'active' sender\nwhich can be detected when a receiver starts.\n\nhttp://spout.zeal.co");
			SetDlgItemTextA(hDlg, IDC_ABOUTBOXTEXT, (LPCSTR)temp);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
	return FALSE;
}


