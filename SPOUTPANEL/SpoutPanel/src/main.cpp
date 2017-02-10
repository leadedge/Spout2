//
//	SpoutPanel
//
//	Displays a dialog for the user to select a Sender name
//	from the list of Senders available. This dialog is activated by spout.dll.
//	The selected Sender is made "Active" so that clients can
//	Get the active Sender as the last one selected by the user.
//
//	30.11.13 - Version 1
//	09.12.13 - changed to dll instead of constructor class
//			   depending on Spout.dll for GetSenderDialog
//	12.12.13 - changed back to a class spxConnector and local dialog routine
//  27.12.13 - changed spxconnector after work with JSpout
//	31.12.13 - changed to spxConnector using Glew instead of dynamic loading of extensions
//	05.01.14 - changed to revised ofxConnector after work with JSpout
//	11.01.14 - disabled sort style of listbox to avboid active sender item highlight error
//	28.01.14 - show window at cursor position
//	01.02.14 - return an exit code
//	10.04.14 - changed to spxConnector for 64bit compatibility
//	12.04.14 - added 64bit detection
//	15.04.14 - changed to updated spxconnector
//			 - changed all Server to Sender
//	08-05-14 - recompiled and tested with Avira due to possible virus problem
//	27-06-14 - added arguments via SelectSenderPanel(char* message);
//			 - compile /MT and embedded manifest
//	28-06-14 - added args for messagebox and /DX9 flag
//	01.08.14 - Version 2
//	01.08.14 - converted to Spout SDK - used only the sendernames class
//			 - uploaded to GitHub
//			 - cleanup
//	03.08.14 - work on unregistered sender
//	04.08.14 - text file for unregistered sender
//			 - refine text file method for unregistered sender - needs path work
//  13.08.14 - Updated for Isadora testing
//  16.08.14 - Tested with Isadora and Version 1 SpoutSender dll OK
//	20.08.14 - recompile and copied to GitHub for Isadora testing
//	29.08.14 - added user message dialog
//			 - removed existence bring to top - handled by Spout SDK
//			 - restore foreground window on exit
//	03.09.14 - GitHub update
//	21.09.14 - Changed default compatibility for /DX9 arg
//	09.10.14 - included SWP_ASYNCWINDOWPOS for topmost
//	12.10.14 - Ensure messagestring is null if no commandline
//	21.10.14 - Change to format detection - SpoutPanel 2
//	17.12.14 - added file open common dialog "/FILEOPEN"
//	29.12.14 - cleanup and commit Version 2.1
//	02-01-15 - changed version numbering to Version 2.01
//  04-01-15 - changed from text file to registry to save the path obtained from OpenFile
//           - TODO - change in Spout programs
//           - Version 2.02
//  05-01-15 - added delay after file or registry write
//  07-01-15 - replaced delay with registry flush on write
//	10-01-15 - null for string passed to OpenFileName to empty initial entry
//           - Version 2.03
//	18-01-15 - added support for "glsl and "frag" extensions in OpenFile
//           - Version 2.04
//	30-01-15 - added version static text and caption icon
//           - Version 2.05
//	07.02.15 - Fixed bug where VVVV sender was not registered
//           - Version 2.06
//	14.02.15 - Recomplied with latest SDK
//           - Version 2.07
//	06.04.15 - added .fs format to known extension list
//  21.04.15 - added more known compatible textures to info
//			 - removed quit if texture incompatible with DX9
//			 - darker version number text
//           - Version 2.08
//	29.04.15 - added texture info button in place of info text
//			 - TODO : cleanup duplicate code
//			 - Version 2.09
//	24.05.15 - Registry write of sender name
//	28.05.15 - Reduced sleep after select sender due to registry flush delay
//			 - Version 2.10
//	13.06.15 - Added Font (ttf)to known extension types
//			 - Version 2.11
//	14.06.15 - Added Font selection dialog and "\FONT" option
//	15.06.15 - Added font properties to file name search via font handle
//			 - Version 2.12
//	19.06.15 - fixed case sensitive extensions for Openfile
//	08.07.15 - remove unknown texture from DX9 incompatibility report
//			 - recompile for 2.004 release
//			 - Version 2.14 /MT
//	08.09.15 - Detection of Memoryshare sender (shared texture pointer is NULL)
//	12.09.15 - Read of memoryshare mode from registry
//	12.10.15 - Removed NULL share handle indicator for memoryshare
//			 - Version 2.15
//	24.11.15 - Reintroduced 250msec sleep after sender selection using the OK button
//			 - re-arranged sender registration and active sender setting to happen before registry write
//	15.12.15 - Rebuild for Spout 2.005 release
//	25.02.16 - Added max senders to texture information
//			 - Version 2.16
//	02.03.16 - change default extension for openfile to *.*
//			 - Version 2.17
//	30.03.16 - Rebuild for 2.005
//	04.04.16 - increased message buffer to 1024
//	13.05.16 - Rebuild for 2.005 installation
//	31.08.16 - re-arrange flags in main entry
//			 - Version 2.18
//	12.01.17 - Update for Spout 2.006
//			 - Add bCPUmode for texture info display
//			 - Version 2.19
//	16.01.17 - Rebuild VS2012
//	21.01.17 - include zero char in command line checks
//	22.01.17 - rebuild VS2012 /MD to avoid virus false postive
//	10.02.17 - Fixed command line for Message Box
//			 - Version 2.20
//
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <Shlwapi.h> // for path functions
#include <io.h> // for file existence check
#include <memory> // for shared_ptr VS2012
// #include <tchar.h> // for _tWinMain - no effect on Virus false positive
#include "resource.h"

#include "../../../../SpoutSDK/SpoutSenderNames.h"

#pragma comment(lib, "Shlwapi") // for Path functions

char SpoutSenderName[256]; // global Sender name to retrieve from the list dialog
char UserMessage[1024]; // User message for the text dialog
bool bDX9compatible = false; // Only list DX9 compatible senders - modified by /DX9 arg
bool bMemoryMode = false; // List memoryshare senders (sharehandle is NULL)
bool bCPUmode = false;
bool bArgFound = false;
bool bFileOpen = false;
bool bFontOpen = false;
char **argv = NULL;

std::vector<char> argdata;

spoutSenderNames sendernames; // Names class functions
HINSTANCE g_hInst = NULL;
static HBRUSH hbrBkgnd = NULL;

//
// Font dialog support
//
// http://stackoverflow.com/questions/16769758/get-a-font-filename-based-on-the-font-handle-hfont
//
#define FONT_FINGERPRINT_SIZE    256
struct FontListItem {
	std::string FileName;
	int FingerPrintOffset;
	char FingerPrint[FONT_FINGERPRINT_SIZE];
};
std::multimap< size_t, std::shared_ptr<FontListItem> > FontList;

bool GetSenderDialog(HINSTANCE hInst);
bool OpenFile(char *filename, const char *extension, int maxchars);
bool OpenFont(char *filepath);
int ParseCommandline();
void wtrim(WCHAR * s);
bool ReadPathFromRegistry(const char *filepath, const char *subkey, const char *valuename);
bool WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename);
bool ReadDwordFromRegistry(DWORD *pValue, const char *subkey, const char *valuename);
void AddFontToList(const std::string& fontFileName);

// The sender list dialog
INT_PTR CALLBACK SenderListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// The MessageBox dialog
INT_PTR CALLBACK TextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// int APIENTRY _tWinMain(	HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow)
{
	HWND hWnd = NULL;
	int i, argc;
	char filename[MAX_PATH];
	bool bRet = false;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	g_hInst = hInstance;

	
	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutPanel 2.20\n");
	*/


	// Find the current active window to restore to the top when SpoutPanel quits
	hWnd = GetForegroundWindow();

	// Check for arguments
	// printf("CmdLine [%s]\n", lpCmdLine);

	UserMessage[0] = 0;
	bFileOpen = false;
	bFontOpen = false;
	bArgFound = false;
	argc = 0;

	argc = ParseCommandline();
	if( argc > 1) { // 0 = "SpoutPanel"

		// printf("Argc = %d\n", argc);

		hWnd = GetActiveWindow();
		EnableWindow(hWnd, FALSE);
		for( i=1; i <argc; i++ ) {
			
			// printf("Arg[%d] = [%s]\n", i, argv[i]);

			// Argument /DX9 or default
			if(strcmp(argv[i], "/DX9") == 0) {
				// printf("DX9 mode\n");
				bArgFound = true;
				bDX9compatible = true;
				break;
			}
			// Argument /DX11
			else if ( strcmp(argv[i], "/DX11") == 0) {
				// printf("DX11 mode\n");
				bArgFound = true;
				bDX9compatible = false;
				break;
			}
			// "/FILEOPEN" to activate a modal file selection dialog
			else if ( strcmp(argv[i], "/FILEOPEN") == 0) {
				// printf("FileOpen found\n");
				bArgFound = true;
				bFileOpen = true;
				break;
			}
			// "/FONT" to activate a font selection dialog
			else if ( strcmp(argv[i], "/FONT") == 0) {
				// printf("FONT found\n");
				bArgFound = true;
				bFontOpen = true;
				break;
			}
			else {
				// printf("No known arg found\n");
				bArgFound = false;
				bFileOpen = false;
				bFontOpen = false;
				bMemoryMode = false;
				if(lpCmdLine && lpCmdLine[0]) {
					// No listed args, but a command line so send a user message
					// printf("text arg [%s]\n", lpCmdLine);
					strcpy_s(UserMessage, 512, (char *)lpCmdLine); // Message to be shown instead of sender list
				}
				else {
					// printf("No text arg\n");
					UserMessage[0] = 0; // make sure this is not an un-initialized string
				}
			}
		}
		EnableWindow(hWnd, TRUE);
	}
	else {
		printf("Argc is zero\n");
		if(lpCmdLine && lpCmdLine[0]) {
			// No listed args, but a command line so send a user message
			// printf("text arg [%s]\n", lpCmdLine);
			strcpy_s(UserMessage, 512, (char *)lpCmdLine); // Message to be shown instead of sender list
		}
		else {
			// printf("No text arg\n");
			UserMessage[0] = 0; // make sure this is not an un-initialized string
		}
	}

	// Look for memoryshare mode in the registry
	DWORD dwMode = 0;
	if(ReadDwordFromRegistry(&dwMode, "Software\\Leading Edge\\Spout", "MemoryShare")) {
		if(dwMode == 1) {
			bMemoryMode = true;
		}
	}

	dwMode = 0;
	if(ReadDwordFromRegistry(&dwMode, "Software\\Leading Edge\\Spout", "CPU")) {
		if(dwMode == 1) {
			bCPUmode = true;
		}
	}

	// LJ DEBUG
	// bFileOpen = true;
	// bFontOpen = true;

	// Try to open the application mutex.
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

	if (!hMutex) {
		hMutex = CreateMutexA(0, 0, "SpoutPanel");
		// printf("SpoutPanel : created Mutex [%x]\n", hMutex);

		// FileOpen common dialog
		if(bFileOpen || bFontOpen) {
			// Get the current file path from the registry
			ReadPathFromRegistry(filename, "Software\\Leading Edge\\SpoutPanel", "Filepath");
			// What file extension did the user last choose ?
			char *extension = NULL;
			extension = PathFindExtensionA(filename);
			if(bFileOpen)
				bRet = OpenFile(filename, extension, MAX_PATH); // returns a file path if successful
			else // Only one other choice
				bRet = OpenFont(filename); // returns a file path if successful
			// WritePathToRegistry
			if(bRet) {
				// printf("SpoutPanel OPEN write to registry\n[%s]\n", filename);
				WritePathToRegistry(filename, "Software\\Leading Edge\\SpoutPanel", "Filepath");
			}
			// Time before the app tries to access the file or the registry depends on registry flush
		}
		else if(UserMessage[0] != 0) {
			// Pop out a message dialog instead of a sender list
			DialogBoxParamA(hInstance, MAKEINTRESOURCEA(IDD_DIALOG2), 0, TextDlgProc, 0);
			bRet = false; // take no action
		}
		else {
			bRet = (bool)GetSenderDialog(hInstance); // activate the dialog to select the active Sender
		}
		
		// This is modal so the mutex remains until it closes
		if(hMutex) {
			// printf("SpoutPanel : releasing Mutex\n");
			ReleaseMutex(hMutex);
		}

	}
	else {
		// Restore to top is handled in SpoutSDK.cpp 
		// We opened it so close it, otherwise it is never released
		// printf("SpoutPanel : Mutex creation failed\n");
		CloseHandle(hMutex);
	}

	// Restore the host window
	if(IsWindow(hWnd)) {
		// printf("SpoutPanel : restoring host window\n");
		SetForegroundWindow(hWnd); 
	}

	// MessageBoxA(NULL, "Spoutpanel end", "Message", MB_OK);

	// quit when selected or cancelled - the dll does it all
	// return an exit code for the opening app to detect
	if(bRet)
		return 0; // OK
	else
		return 1; // Cancel

}

// Function to return a selected Sender name from a dialog showing the list in shared memory
bool GetSenderDialog(HINSTANCE hInst)
{
	INT_PTR nRet;
	std::string namestring;
		
	// This is a modal dialog so will stop this application at this point
	nRet = DialogBoxParamA(hInst, MAKEINTRESOURCEA(IDD_DIALOG1), 0, SenderListDlgProc, 0);
	if(nRet == 1) { // OK and not CANCEL
		if(strlen(SpoutSenderName) > 0 && strcmp(SpoutSenderName, "SpoutSenderName") != 0) {
			return true;
		}
	}

	return false;
}


//
// The SpoutPanel dialog
//
// http://msdn.microsoft.com/en-us/library/windows/desktop/hh298365%28v=vs.85%29.aspx
//
INT_PTR CALLBACK SenderListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char name[512];
	char temp[512];
	char activename[512];
	char itemstring[512];
	HWND hwndList = NULL;
	int pos, lbItem, item;
	int SenderItem = -1;
	std::set<std::string> Senders;
	std::set<std::string>::iterator iter;
	std::string namestring;
	SharedTextureInfo info;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {

		// Make Version info light grey
		case WM_CTLCOLORSTATIC:
        {
			HDC hdcStatic = (HDC)wParam;
			// lParam is the handle to the control
			if(GetDlgItem(hDlg, IDC_VERS) == (HWND)lParam) {
				// SetTextColor(hdcStatic, RGB(140, 140, 140));
				SetTextColor(hdcStatic, RGB(96, 96, 96));
				SetBkColor(hdcStatic, RGB(240, 240, 240));
				if (hbrBkgnd == NULL) {
					hbrBkgnd = CreateSolidBrush(RGB(240, 240, 240));
				}
				return (INT_PTR)hbrBkgnd;
			}
        }
		break;

		
		case WM_INITDIALOG:
			{
				HICON hIcon;
				hIcon = (HICON)LoadImageA(	g_hInst,
									MAKEINTRESOURCEA(IDI_ICON1),
									IMAGE_ICON,
									GetSystemMetrics(SM_CXSMICON),
									GetSystemMetrics(SM_CYSMICON),
									0);
				if(hIcon) {
					SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				}
			}

			// get the sender name list in shared memory into a local list
			sendernames.GetSenderNames(&Senders);

			// Now we have a local set of names "Senders"

			// 27.12.13 - noted that if a Processing sketch is stopped by closing the window
			// all is OK and either the "stop" or "dispose" overrides work, but if STOP is used, 
			// or the sketch is closed, neither the exit or dispose functions are called and
			// the sketch does not release the sender.
			// So here we run through again and check whether the sender exists and if it does not
			// release the sender from the local sender list
			if(Senders.size() > 0) {
				// printf("%d senders\n", Senders.size());
				for(iter = Senders.begin(); iter != Senders.end(); iter++) {
					namestring = *iter; // the Sender name string
					strcpy_s(name, namestring.c_str());
					// printf("    %s\n", name);
					// we have the name already, so look for it's info
					if(!sendernames.getSharedInfo(name, &info)) {
						// Sender does not exist any more
						// printf("    %s does not exist any more\n", name);
						sendernames.ReleaseSenderName(name); // release from the shared memory list
					}
				}
			}
			// else {
				// printf("No senders\n");
			// }

			// Now we have cleaned up the list in shared memory, so get it again
			Senders.clear();
			sendernames.GetSenderNames(&Senders);

			// Get the active Sender name
			// and set the name into the editbox
			if(sendernames.GetActiveSender(activename)) {
				// Is it registered e.g. VVVV which has been accessed ?
				if(!sendernames.FindSenderName(activename)) {
					if(sendernames.getSharedInfo(activename, &info)) {
						// printf("Registering unlisted active sender (%s)\n", activename);
						sendernames.RegisterSenderName(activename);
					}
				}

				// Now it should be in the name set
				if(sendernames.FindSenderName(activename)) {
					sendernames.getSharedInfo(activename, &info);
					// LJ DEBUG if((bMemoryMode && !info.shareHandle) || (!bMemoryMode && info.shareHandle) ) {
						SetDlgItemTextA(hDlg, IDC_ACTIVE, (LPCSTR)activename);
					// }

					bool bUnKnown = false;

							/*
							printf("Sharehandle      [%x]\n", info.shareHandle);
							printf("Width            [%d]\n", info.width);
							printf("Height           [%d]\n", info.height);
							printf("Format           [%d] (%x)\n", info.format, info.format);
							printf("Usage            [%d]\n", info.usage);
							*/

					switch(info.format) {
						// DX9
						case 0 : // default unknown
							// if(info.shareHandle == NULL)
								// sprintf_s(temp, 512, "Memoryshare : %dx%d", info.width, info.height);
							// else
								sprintf_s(temp, 512, "DirectX : %dx%d", info.width, info.height);
							break;
						case 21 : // D3DFMT_A8R8G8B8
							sprintf_s(temp, 512, "DirectX 9 : %dx%d [ARGB]", info.width, info.height);
							break;
						case 22 : // D3DFMT_X8R8G8B8
							sprintf_s(temp, 512, "DirectX 9 : %dx%d [XRGB]", info.width, info.height);
							break;
						// DX11
						case 28 : // DXGI_FORMAT_R8G8B8A8_UNORM
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [RGBA]", info.width, info.height);
							break;
						case 87 : // DXGI_FORMAT_B8G8R8A8_UNORM
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [BGRA]", info.width, info.height);
							break;
						case 88 : // DXGI_FORMAT_B8G8R8AX_UNORM (untested)
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [BGRX]", info.width, info.height);
							break;
						case 2 : // DXGI_FORMAT_R32G32B32A32_FLOAT = 2
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [32bit float RGBA]", info.width, info.height);
							break;
						case 10 : // DXGI_FORMAT_R16G16B16A16_FLOAT = 10
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [16bit float RGBA]", info.width, info.height);
							break;
						case 13 : // DXGI_FORMAT_R16G16B16A16_SNORM = 13
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [16bit RGBA]", info.width, info.height);
							break;
						case 24 : // DXGI_FORMAT_R10G10B10A2_UNORM = 24
							sprintf_s(temp, 512, "DirectX 11 : %dx%d [10bit RGBA]", info.width, info.height);
							break;
						default:
							sprintf_s(temp, 512, "Format(%d) %dx%d :", info.format, info.width, info.height);
							bUnKnown = true;
							break;
					}

					if(bDX9compatible && info.shareHandle != NULL) { // Specify DX9 compatible senders

						// Is the sender DX9 compatible
						if(! ( info.format == 0  // default directX 9
							|| info.format == 21 // DX9 ARGB
							|| info.format == 22 // DX9 XRGB
							|| info.format == 87 // compatible DX11
							)) {
							strcat_s(temp, 512, " not DX9 compatible");

							// Info
							// unsigned __int32 shareHandle;
							// unsigned __int32 width;
							// unsigned __int32 height;
							// DWORD format; // Texture pixel format
							// DWORD usage; // not used
							// wchar_t description[128]; // Wyhon compatible description (not used)
							// unsigned __int32 partnerId; // Wyphon id of partner that shared it with us (not unused)
							// 
							// printf("not DX9 compatible\n");
							// printf("Sharehandle      [%x]\n", info.shareHandle);
							// printf("Width            [%d]\n", info.width);
							// printf("Height           [%d]\n", info.height);
							// printf("Format           [%d] (%x)\n", info.format, info.format);
							// printf("Usage            [%d]\n", info.usage);

						}
					}

					// LJ DEBUG if(bMemoryMode && info.shareHandle == NULL) { // Specify Memoryshare senders
					// if(bMemoryMode)	sprintf_s(temp, 512, "Memoryshare : %dx%d", info.width, info.height);

					SetDlgItemTextA(hDlg, IDC_INFO, (LPCSTR)temp);
				}
			}

			// Add all the Sender names as items to the dialog list.
			if(Senders.size() > 0) {
				hwndList = GetDlgItem(hDlg, IDC_LIST1);  
				item = 0;
				for(iter = Senders.begin(); iter != Senders.end(); iter++) {
					namestring = *iter; // the string to copy
					strcpy_s(name, namestring.c_str());
					if(sendernames.getSharedInfo(name, &info)) {
						
						// LJ DEBUG
						// if((bMemoryMode && !info.shareHandle)
						// || (!bMemoryMode && info.shareHandle) ) {

							sprintf_s(itemstring, "%s : (%dx%d)", name, info.width, info.height);
							pos = (int)SendMessageA(hwndList, LB_ADDSTRING, 0, (LPARAM)itemstring); 
							SendMessageA(hwndList, LB_SETITEMDATA, pos, (LPARAM)item);
							// Was it the active Sender
							// Listbox cannot be sorted to do this
							if(strcmp(name, activename) == 0) {
								SenderItem = item;
							}
							item++;
						// }
					}
				}
			}
			
            //  Select all text in the edit field
            SendDlgItemMessage (hDlg, IDC_ACTIVE, EM_SETSEL, 0, 0x7FFF0000L);

			// Set input focus to the list box.
			SetFocus(hwndList); 

			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			int x, y, w, h;
			POINT p;
			RECT rect;
			GetWindowRect(hDlg, &rect);
			if (GetCursorPos(&p)) {
				//cursor position now in p.x and p.y
				x = p.x;
				y = p.y;
			}
			else {
				x = rect.left;
				y = rect.top;
			}
			w = rect.right - rect.left; h = rect.bottom - rect.top;

			// SWP_ASYNCWINDOWPOS
			// If the calling thread and the thread that owns the window are attached to different input queues,
			// the system posts the request to the thread that owns the window. 
			// This prevents the calling thread from blocking its execution while other threads
			// process the request. 
			SetWindowPos(hDlg, HWND_TOPMOST, x, y, w, h, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW);

			// for match with active Sender otherwise make the user choose
			if(SenderItem >= 0) {
				SendMessageA(hwndList, LB_SETCURSEL, SenderItem, 0);
				return FALSE;
			}

			return TRUE; // return TRUE  unless you set the focus to a control

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDC_INFO :
					GetDlgItemTextA(hDlg, IDC_ACTIVE, (LPSTR)SpoutSenderName, 256);
					if(strlen(SpoutSenderName) > 0) {
						// printf("    Sender  [%s]\n", SpoutSenderName);
						if(sendernames.getSharedInfo(SpoutSenderName, &info)) {
							// printf("(%s) - %dx%d [%d]\n", name, info.width, info.height, info.format);
							// printf("\n    Sender  [%s]\n", SpoutSenderName);
							// printf("    Width   %d\n", info.width);
							// printf("    Height  %d\n", info.height);
							// printf("    Format  %d\n", info.format);
							// printf("    Handle  %u (%x)\n", info.shareHandle, info.shareHandle);
								switch(info.format) {
									// DX9
									case 0 : // default unknown
										if(info.shareHandle == NULL)
											sprintf_s(temp, 512, "Memoryshare : %dx%d", info.width, info.height);
										else
											sprintf_s(temp, 512, "DirectX : %dx%d\nDefault format", info.width, info.height);
										break;
									case 21 : // D3DFMT_A8R8G8B8
										sprintf_s(temp, 512, "DirectX 9 : %dx%d\nD3DFMT_A8R8G8B8 (21)", info.width, info.height);
										break;
									case 22 : // D3DFMT_X8R8G8B8
										sprintf_s(temp, 512, "DirectX 9 : %dx%d\nD3DFMT_X8R8G8B8 (22)", info.width, info.height);
										break;
									// DX11
									// DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
									case 2 :
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_R32G32B32A32_FLOAT (2)", info.width, info.height);
										break;
									//    DXGI_FORMAT_R32G32B32A32_UINT           = 3,
									//    DXGI_FORMAT_R32G32B32A32_SINT           = 4,
									//    DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
									//    DXGI_FORMAT_R32G32B32_FLOAT             = 6,
									//    DXGI_FORMAT_R32G32B32_UINT              = 7,
									//    DXGI_FORMAT_R32G32B32_SINT              = 8,
									//    DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
									case 10 :
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_R16G16B16A16_FLOAT (10)", info.width, info.height);
										break;
									//        DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
									//        DXGI_FORMAT_R16G16B16A16_UINT           = 12,
									case 13 :
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_R16G16B16A16_SNORM (13)", info.width, info.height);
										break;
									//        DXGI_FORMAT_R16G16B16A16_SINT           = 14,
									//        DXGI_FORMAT_R32G32_TYPELESS             = 15,
									//        DXGI_FORMAT_R32G32_FLOAT                = 16,
									//        DXGI_FORMAT_R32G32_UINT                 = 17,
									//        DXGI_FORMAT_R32G32_SINT                 = 18,
									//        DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
									//        DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
									//        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
									//        DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
									//        DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
									case 24 :
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_R10G10B10A2_UNORM (24)", info.width, info.height);
										break;
									//        DXGI_FORMAT_R10G10B10A2_UINT            = 25,
									//        DXGI_FORMAT_R11G11B10_FLOAT             = 26,
									//        DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,								
									case 28 : // DXGI_FORMAT_R8G8B8A8_UNORM
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_R8G8B8A8_UNORM (28)", info.width, info.height);
										break;
									//        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
									//        DXGI_FORMAT_R8G8B8A8_UINT               = 30,
									//        DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
									//        DXGI_FORMAT_R8G8B8A8_SINT               = 32,
									case 87 : // DXGI_FORMAT_B8G8R8A8_UNORM
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_B8G8R8A8_UNORM (87)", info.width, info.height);
										break;
									case 88 : // DXGI_FORMAT_B8G8R8AX_UNORM (untested)
										sprintf_s(temp, 512, "DirectX 11 : %dx%d\nDXGI_FORMAT_B8G8R8AX_UNORM (88)", info.width, info.height);
										break;
									default:
										sprintf_s(temp, 512, "DirectX : %dx%d\nUnlisted format [%d]", info.width, info.height, info.format);
										break;
								} // end switch(info.format)

							if(bMemoryMode)	
								strcat_s(temp, 512, "\nMemoryshare");
							else if(bCPUmode)
								strcat_s(temp, 512, "\nCPU texture share");
							else 
								strcat_s(temp, 512, "\nTexture share");
							
							// ====================================================
							// 25.02.16 - get the maximum number of senders
							// Only works if SpoutDXmode has set the registry entry
							DWORD dwSenders = 0;
							if(ReadDwordFromRegistry(&dwSenders, "Software\\Leading Edge\\Spout", "MaxSenders")) {
								char tmp[32];
								// sprintf(tmp, "\nMaximum number of senders : %d", dwSenders);
								strcat_s(temp, 512, tmp);
							}
							// ====================================================

							MessageBoxA(hDlg, temp, "Texture info", MB_OK);
						} // endif sendernames.getSharedInfo(SpoutSenderName, &info)
					} // endif strlen(SpoutSenderName) > 0
					return TRUE;

				case IDOK: // 1

					// printf("\nIDOK\n");

					// Get contents of edit field into the global char string
					GetDlgItemTextA(hDlg, IDC_ACTIVE, (LPSTR)SpoutSenderName, 256);
					if(strlen(SpoutSenderName) > 0) {
						// Does it have any shared info
						if(sendernames.getSharedInfo(SpoutSenderName, &info)) {

							// 24.11.15 - moved from below
							// Register the sender name
							sendernames.RegisterSenderName(SpoutSenderName);							// Set the selected name as the active Sender
							// Any receiver can then query the active Sender name
							sendernames.SetActiveSender(SpoutSenderName);

							// It is an OK sender so write it's name to the registry
							WritePathToRegistry(SpoutSenderName, "Software\\Leading Edge\\SpoutPanel", "Sendername");
							// For a Sender which is not registered - e.g. VVVV
							// Registering the sender here will only work if another sender
							// is running or "SpoutTray" is present and has been activated 
							// to show the sender list after this sender has been registered, 
							// because this instance of spoutSenderNames for spoutpanel
							// will close and erase the active name map and the sender map
							// and any map handle in this app will be closed
							// Therefore we write the name to the registry and register it again later if not registered
						}
						else {
							// Serious enough for a messagebox
							sprintf_s(name, "Sender [%s] does not exist", SpoutSenderName);
							MessageBoxA(hDlg, name, "SpoutPanel", 0);
							EndDialog(hDlg, LOWORD(wParam));
							return TRUE;
						}

						// LJ DEBUG to halt for debugging
						// sprintf_s(name, "Sender [%s] OK", SpoutSenderName);
						// MessageBoxA(hDlg, name, "SpoutPanel", 0);

						Sleep(125); // This seems necessary or the texture is not received

					} // endif strlen(SpoutSenderName) > 0
				
				case IDCANCEL: // 2
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;


				case IDC_LIST1:
					
					switch (HIWORD(wParam))  { 
						case LBN_SELCHANGE:
							HWND hwndList = GetDlgItem(hDlg, IDC_LIST1); 
							// Get the Sender list again
							sendernames.GetSenderNames(&Senders);
							// Get selected list item
	                        lbItem = (int)SendMessageA(hwndList, LB_GETCURSEL, 0, 0); 
							if(lbItem != LB_ERR) {
								// Get the saved index
								SendMessageA(hwndList, LB_GETTEXT, lbItem, (LPARAM)SpoutSenderName);
		                        pos = (int)SendMessageA(hwndList, LB_GETITEMDATA, lbItem, 0);
								// Get the details of the Sender
								if(Senders.size() > 0) {
									iter = std::next(Senders.begin(), pos);
									namestring = *iter; // the selected Sender in the list
									strcpy_s(name, namestring.c_str());
									SetDlgItemTextA(hDlg, IDC_ACTIVE, (LPCSTR)name);
									sendernames.getSharedInfo(name, &info);

									// printf("(%s) - %dx%d [%d]\n", name, info.width, info.height, info.format);
									// printf("\n    Sender  [%s]\n", SpoutSenderName);
									// printf("    Width   %d\n", info.width);
									// printf("    Height  %d\n", info.height);
									// printf("    Format  %d\n", info.format);
									// printf("    Handle  %u (%x)\n", info.shareHandle, info.shareHandle);

									switch(info.format) {
										// DX9
										case 0 : // default unknown
											if(info.shareHandle == NULL)
												sprintf_s(temp, 512, "Memoryshare : %dx%d", info.width, info.height);
											else
												sprintf_s(temp, 512, "DirectX : %dx%d", info.width, info.height);
											break;
										case 21 : // D3DFMT_A8R8G8B8
											sprintf_s(temp, 512, "DirectX 9 : %dx%d [ARGB]", info.width, info.height);
											break;
										case 22 : // D3DFMT_X8R8G8B8
											sprintf_s(temp, 512, "DirectX 9 : %dx%d [XRGB]", info.width, info.height);
											break;
										// DX11
										case 28 : // DXGI_FORMAT_R8G8B8A8_UNORM
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [RGBA]", info.width, info.height);
											break;
										case 87 : // DXGI_FORMAT_B8G8R8A8_UNORM
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [BGRA]", info.width, info.height);
											break;
										case 88 : // DXGI_FORMAT_B8G8R8AX_UNORM (untested)
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [BGRX]", info.width, info.height);
											break;
										// 21.04.15 - added more formats
										// DXGI_FORMAT_R32G32B32A32_FLOAT = 2
										// DXGI_FORMAT_R16G16B16A16_FLOAT = 10
										// DXGI_FORMAT_R16G16B16A16_SNORM = 13
										// DXGI_FORMAT_R10G10B10A2_UNORM = 24
										case 2 :
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [32bit float RGBA]", info.width, info.height);
											break;

										case 10 :
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [16bit float RGBA]", info.width, info.height);
											break;

										case 13 :
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [16bit RGBA]", info.width, info.height);
											break;

										case 24 :
											sprintf_s(temp, 512, "DirectX 11 : %dx%d [10bit RGBA]", info.width, info.height);
											break;
					
										default:
											sprintf_s(temp, 512, "%dx%d : incompatible format [%d]", info.width, info.height, info.format);
											break;
									}
									
									// if(bMemoryMode)	sprintf_s(temp, 512, "Memoryshare : %dx%d", info.width, info.height);
					
									SetDlgItemTextA(hDlg, IDC_INFO, (LPCSTR)temp);
								}
							}
			                return TRUE; 
					} // end switch (HIWORD(wParam))
					// end case IDC_LIST1
			} // end switch (LOWORD(wParam))
		// end case case WM_COMMAND:
		return TRUE;
	} // end switch (message)

	return FALSE;

} // end SenderListDlgProc


// Message handler for user text dialog
INT_PTR CALLBACK TextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	switch (message) {
		
		case WM_INITDIALOG:
			SetDlgItemTextA(hDlg, IDC_USERTEXT, (LPCSTR)UserMessage);
			// prevent other windows from hiding the dialog
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOMOVE);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK :
				case IDCANCEL :
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				default:
					return FALSE;
			}
			break;
	}

	return FALSE;
}


bool OpenFont(char *filepath)
{
	static DWORD rgbCurrent;  // current text color
	CHOOSEFONTA cf;            // common dialog box structure
	static LOGFONTA lf;        // logical font structure
	char WinFontDir[MAX_PATH];

	// Find the Windows font folder
	if(GetEnvironmentVariableA((LPCSTR)"windir", (LPSTR)WinFontDir, MAX_PATH) == 0)	return false;
	strcat_s(WinFontDir, "\\fonts");
	// printf("WinFontDir [%s]\n", WinFontDir);

	// create a lookup table (FontList) of all installed/known fonts
	// with the Windows font files in WinFontDir
	HANDLE hFind = NULL;
	WIN32_FIND_DATAA fd;
	char FilePath[MAX_PATH];
	std::string FontFileName;

	sprintf_s(FilePath, MAX_PATH, "%s\\*.ttf", WinFontDir);
	hFind = FindFirstFileA(FilePath, &fd); 
	if(hFind != INVALID_HANDLE_VALUE) {
		strcpy_s(FilePath, WinFontDir);
		strcat_s(FilePath, "\\");
		int i = 0;
		do  { 
			// read all ttf files in current folder
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
				FontFileName = FilePath;
				FontFileName += fd.cFileName;
				AddFontToList(FontFileName);
			}
			i++;
		} while(FindNextFileA(hFind, &fd) != 0); 
		FindClose(hFind); 
	}

	// Initialize CHOOSEFONT for the dialog
	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof (cf);
	cf.hwndOwner = NULL; // hwnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = rgbCurrent;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS;

	// Open the dialog
	if (ChooseFontA(&cf)==TRUE) {

		/*
		printf("Face Name        = [%s]\n", cf.lpLogFont->lfFaceName);
		printf("lfEscapement     = [%d]\n", cf.lpLogFont->lfEscapement);
		printf("lfOrientation    = [%d]\n", cf.lpLogFont->lfOrientation);
		printf("lfWeight         = [%d]\n", cf.lpLogFont->lfWeight);
		printf("lfItalic         = [%d]\n", cf.lpLogFont->lfItalic);
		printf("lfUnderline      = [%d]\n", cf.lpLogFont->lfUnderline);
		printf("lfStrikeOut      = [%d]\n", cf.lpLogFont->lfStrikeOut);
		printf("lfCharSet        = [%d]\n", cf.lpLogFont->lfCharSet);
		printf("lfPitchAndFamily = [%d]\n", cf.lpLogFont->lfPitchAndFamily);
		*/
		// Create a font based on the information retrieved
		// in order to get the font handle
		HFONT fontHandle = CreateFontA(	cf.lpLogFont->lfHeight,
								cf.lpLogFont->lfWidth,
								cf.lpLogFont->lfEscapement,
								cf.lpLogFont->lfOrientation,
								cf.lpLogFont->lfWeight,
								cf.lpLogFont->lfItalic,
								cf.lpLogFont->lfUnderline,
								cf.lpLogFont->lfStrikeOut,
								cf.lpLogFont->lfCharSet,
								cf.lpLogFont->lfOutPrecision,
								cf.lpLogFont->lfClipPrecision,
								cf.lpLogFont->lfQuality,
								cf.lpLogFont->lfPitchAndFamily,
								cf.lpLogFont->lfFaceName);
		if(fontHandle) {
			printf("fontHandle = %x (%s)\n", fontHandle, cf.lpLogFont->lfFaceName);
			// Retrieve the font data given its handle
			std::vector<char> data;
			bool result = false;
			HDC hdc = CreateCompatibleDC(NULL);
			if (hdc != NULL) {
				HGDIOBJ prevObj = SelectObject(hdc, fontHandle);
				if(prevObj != NULL && prevObj != HGDI_ERROR) {
					DWORD size = GetFontData(hdc, 0, 0, NULL, 0);
					if(size != GDI_ERROR) {
						char* buffer = new char[size];
						if (GetFontData(hdc, 0, 0, buffer, size) == size) {
							data.resize(size); // LJ - this is necessary
							memcpy(&data[0], buffer, size);
							result = true;
						}
						delete[] buffer;
					}
					else {
						printf("GetFontData failed (hdc = %x, size = %d\n", hdc, (int)size);
					}
				}
				else {
					printf("SelectObject failed\n");
				}
				DeleteDC(hdc);
			}
			else {
				printf("CreateCompatibleDC failed\n");
			}
			DeleteFont(fontHandle);

			// Compare the data with the font list
			if(result) {
				for (auto i = FontList.lower_bound(data.size()); i != FontList.upper_bound(data.size()); ++i) {
					if (memcmp(&data[i->second->FingerPrintOffset], i->second->FingerPrint, FONT_FINGERPRINT_SIZE) == 0) {
						FontFileName = i->second->FileName;
					}
				}
				sprintf_s(filepath, MAX_PATH, "%s", FontFileName.c_str());
				// Does it exist ?
				if(_access(filepath, 0) != -1) { 
					return true;
				}
			}
		}
	}
	else {
		// printf("ChooseFont not OK\n");
	}

	return false;

}

void AddFontToList(const std::string& fontFileName)
{
	// printf("AddFontToList(%s)\n", fontFileName.c_str());

    std::ifstream file(fontFileName, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    size_t fileSize = (size_t)file.tellg();
    if (fileSize < FONT_FINGERPRINT_SIZE) {
		file.close();
        return;
	}

    std::shared_ptr<FontListItem> fontListItem(new FontListItem());
    fontListItem->FileName = fontFileName;
    fontListItem->FingerPrintOffset = rand() % (fileSize - FONT_FINGERPRINT_SIZE);
    file.seekg(fontListItem->FingerPrintOffset);
    file.read(fontListItem->FingerPrint, FONT_FINGERPRINT_SIZE);
    FontList.insert(std::pair<size_t, std::shared_ptr<FontListItem> >(fileSize, fontListItem));
	file.close();

}


bool OpenFile(char *filepath, const char *extension, int maxchars)
{
	OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = "";
	
	ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // TODO hwnd;

	// Set defaults
    // ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0"; // TODO different file types as args
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0"; // TODO different file types as args
    ofn.lpstrDefExt = "txt";

	// What extension did the user last use?
	if(extension) {

		printf("extension [%s]\n", extension);

		// Known file types
		if(strcmp(extension, ".txt") == 0 || strcmp(extension, ".TTF") == 0) {
			ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0Shader Files (*.glsl)\0*.glsl\0Font Files (*.ttf)\0*.ttf\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "ttf";
		}
		// .ttf (font files)
		else if(strcmp(extension, ".ttf") == 0 || strcmp(extension, ".TTF") == 0) {
			ofn.lpstrFilter = "Font Files (*.ttf)\0*.ttf\0Shader Files (*.glsl)\0*.glsl\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "ttf";
		}
		// .glsl, .frag. .fs
		else if(strcmp(extension, ".glsl") == 0 || strcmp(extension, ".GLSL") == 0) {
			ofn.lpstrFilter = "Shader Files (*.glsl)\0*.glsl\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "glsl";
		}
		else if(strcmp(extension, ".frag") == 0 || strcmp(extension, ".FRAG") == 0) {
			ofn.lpstrFilter = "Shader Files (*.frag)\0*.frag\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "frag";
		}
		else if(strcmp(extension, ".fs") == 0 || strcmp(extension, ".FS") == 0) {
			ofn.lpstrFilter = "ISF Shader Files (*.fs)\0*.fs\0Shader Files (*.frag)\0*.frag\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "fs";
		}

		printf("default extension [%s]\n", ofn.lpstrDefExt);
	}

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    // ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	// ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn.Flags = OFN_EXPLORER;
	// OFN_FORCESHOWHIDDEN : Forces the showing of system and hidden files, thus overriding
	// the user setting to show or not show hidden files.
	// However, a file that is marked both system and hidden is not shown.
	// ofn.Flags = OFN_FORCESHOWHIDDEN | OFN_EXPLORER;

    if(GetOpenFileNameA(&ofn)) {
		strcpy_s(filepath, maxchars, szFile);
        // Do something useful with the filename stored in szFile 
		return true;
    }

	return false;

}


//
// http://www.codeguru.com/cpp/w-p/win32/article.php/c1427/A-Simple-Win32-CommandLine-Parser.htm
//
int ParseCommandline()
{
	int    argc, BuffSize, i;
	WCHAR  *wcCommandLine;
	LPWSTR *argw;

	// Get a WCHAR version of the parsed commande line
	wcCommandLine = GetCommandLineW();
	if (*wcCommandLine == NULL) {
		// printf("NULL command line\n");
		return 0;
	}


	// When a program is launched directory with no arguments, GetCommandLineW() can return 
	// an unquoted path with spaces in it. In this case, CommandLineToArgvW() will not handle 
	// the string properly. Ran into this issue on Vista 64 with a 32-bit app, not sure how widespread it is.
	wtrim(wcCommandLine);
	argw = CommandLineToArgvW( wcCommandLine, &argc);
	if(*argw == NULL || argc <= 1)
		return 0;


	argv = (char **)GlobalAlloc( LPTR, argc + 1 ); 	// Create the first dimension of the double array
	
	// convert each line of wcCommandeLine to MultiByte and place them
	// to the argv[] array
	for( i=0; i < argc; i++) {
		BuffSize = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, argw[i], -1, NULL, 0, NULL, NULL );
		argv[i] = (char *)GlobalAlloc( LPTR, BuffSize );		
		WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, argw[i], BuffSize * sizeof( WCHAR ) ,argv[i], BuffSize, NULL, NULL );
	}
	
	// LJ DEBUG - argv is not released - memory leak

	// return the number of arguments
	return argc;

} // ParseCommandline()



void wtrim(WCHAR * s) {

    WCHAR * p = s;
    int l = (int)wcslen(p);

	if(!s[0]) return;

    while(iswspace(p[l - 1])) p[--l] = 0;
    while(* p && iswspace(* p)) ++p, --l;

    memmove(s, p, l*2+1);

}


bool ReadPathFromRegistry(const char *filepath, const char *subkey, const char *valuename)
{
	HKEY  hRegKey;
	LONG  regres;
	DWORD  dwSize, dwKey;  

	dwSize = MAX_PATH;

	// Does the key exist
	regres = RegOpenKeyExA(HKEY_CURRENT_USER, subkey, NULL, KEY_READ, &hRegKey);
	if(regres == ERROR_SUCCESS) {
		// Read the key Filepath value
		regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)filepath, &dwSize);
		RegCloseKey(hRegKey);
		if(regres == ERROR_SUCCESS)
			return true;
	}

	// Just quit if the key does not exist
	return false;

}

bool WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename)
{
	HKEY  hRegKey;
	LONG  regres;
	char  mySubKey[512];

	// The required key
	strcpy_s(mySubKey, 512, subkey);

	// Does the key already exist ?
	regres = RegOpenKeyExA(HKEY_CURRENT_USER, mySubKey, NULL, KEY_ALL_ACCESS, &hRegKey);
	if(regres != ERROR_SUCCESS) {
		// Create a new key
		regres = RegCreateKeyExA(HKEY_CURRENT_USER, mySubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
	}

	if(regres == ERROR_SUCCESS && hRegKey != NULL) {
		// Write the path
		regres = RegSetValueExA(hRegKey, valuename, 0, REG_SZ, (BYTE*)filepath, ((DWORD)strlen(filepath) + 1)*sizeof(unsigned char));
		// For immediate read after write - necessary here becasue the app that opeded SpoutPanel
		// will read the registry straight away and it might not be available yet
		// The key must have been opened with the KEY_QUERY_VALUE access right (included in KEY_ALL_ACCESS)
		LONG lr = RegFlushKey(hRegKey); // needs an open key
		if(lr != ERROR_SUCCESS)
			MessageBoxA(NULL, "Registry flush failed", "SpoutPanel", MB_OK);

		RegCloseKey(hRegKey); // Done with the key

    }
	else {
		// printf("RegCreateKeyEx failed (%d)*%x)\n", regres);
	}

	if(regres == ERROR_SUCCESS)
		return true;
	else
		return false;

}

bool ReadDwordFromRegistry(DWORD *pValue, const char *subkey, const char *valuename)
{
	HKEY  hRegKey;
	LONG  regres;
	DWORD  dwSize, dwKey;  

	dwSize = MAX_PATH;

	// Does the key exist
	regres = RegOpenKeyExA(HKEY_CURRENT_USER, subkey, NULL, KEY_READ, &hRegKey);
	if(regres == ERROR_SUCCESS) {
		// Read the key DWORD value
		regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)pValue, &dwSize);
		RegCloseKey(hRegKey);
		if(regres == ERROR_SUCCESS)
			return true;
	}

	// Just quit if the key does not exist
	return false;

}


