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
//

#include <windows.h>
#include <vector>
#include <fstream>
#include <Shlwapi.h> // for path functions
#include "resource.h"

#include "../../../../SpoutSDK/SpoutSenderNames.h"

#pragma comment(lib, "Shlwapi") // for Path functions

char SpoutSenderName[256]; // global Sender name to retrieve from the list dialog
char UserMessage[512]; // User message for the text dialog

bool bDX9compatible = false; // Only list DX9 compatible senders - modified by /DX9 arg
bool bArgFound = false;
bool bFileOpen = false;
bool GetSenderDialog(HINSTANCE hInst);
bool OpenFile(char *filename, const char *extension, int maxchars);
int ParseCommandline();
bool ReadPathFromRegistry(const char *filepath, const char *subkey, const char *valuename);
bool WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename);
char **argv = NULL;
spoutSenderNames sendernames; // Names class functions
HINSTANCE g_hInst = NULL;
static HBRUSH hbrBkgnd = NULL;

// The sender list dialog
INT_PTR CALLBACK SenderListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// The MessageBox dialog
INT_PTR CALLBACK TextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd = NULL;
	int i, argc;
	HMODULE module;
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];
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
	printf("SpoutPanel 2.05\n");
	*/

	// Find the current active window to restore to the top when SpoutPanel quits
	hWnd = GetForegroundWindow();

	// Remove any temporary file
	module = GetModuleHandle(NULL);
	GetModuleFileNameA(module, path, MAX_PATH);
	_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
	_makepath_s(path, MAX_PATH, drive, dir, "spoutpanel", ".txt");
	remove(path);
	// printf("Text file path is [%s]\n");

	// Check for arguments
	UserMessage[0] = 0;
	argc = ParseCommandline();
	if( argc > 1) { // 0 = "SpoutPanel"
		bFileOpen = false;
		bArgFound = false;
		hWnd = GetActiveWindow();
		EnableWindow(hWnd, FALSE);
		for( i=1; i <argc; i++ ) {
			// printf("Arg[%d] = [%s]\n", i, argv[i]);
			// Argument /DX9
			if(strcmp(argv[i], "/DX9") == 0) {
				// printf("DX9 mode\n");
				bArgFound = true;
				bDX9compatible = true;
			}
			// Argument /DX11 or default
			else if ( strcmp(argv[i], "/DX11") == 0) {
				// printf("DX11 mode\n");
				bArgFound = true;
				bDX9compatible = false;
			}
			// "/FILEOPEN" to activate a modal file selection dialog
			else if ( strcmp(argv[i], "/FILEOPEN") == 0) {
				// printf("FileOpen found\n");
				bArgFound = true;
				bFileOpen = true;
			}
			else {
				// printf("No known arg found\n");
				bArgFound = false;
				bFileOpen = false;
			}
		}

		if(!bArgFound && lpCmdLine) {
			// Know listed args, but a command line so send a user message
			strcpy_s(UserMessage, 512, lpCmdLine); // Message to be shown instead of sender list
			// printf("text arg [%s]\n", lpCmdLine);
		}
		else {
			// printf("No text arg\n");
			UserMessage[0] = 0; // make sure this is not an un-initialized string
		}

		EnableWindow(hWnd, TRUE);
	}

	// Try to open the application mutex.
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

	if (!hMutex) {
		hMutex = CreateMutexA(0, 0, "SpoutPanel");
		// FileOpen common dialog
		if(bFileOpen) {
			// Get the current file path from the registry
			ReadPathFromRegistry(filename, "Software\\Leading Edge\\SpoutPanel", "Filepath");
			// printf("SpoutPanel read from registry\n[%s]\n", filename);

			// What file extension did the user last choose ?
			char *extension = NULL;
			extension = PathFindExtensionA(filename);
			// if(extension) printf("extension is [%s]\n", extension);
			bRet = OpenFile(filename, extension, MAX_PATH); // returns a file path if successful
			// 04-01-15 - WritePathToRegistry
			if(bRet) {
				// printf("SpoutPanel write to registry\n[%s]\n", filename);
				WritePathToRegistry(filename, "Software\\Leading Edge\\SpoutPanel", "Filepath");
			}
			// Give time before the app tries to access the file or the registry
			// 07-01-15 - Now depends on registry flush
			// Sleep(125);
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
			ReleaseMutex(hMutex);
		}
	}
	else {
		// Restore to top is handled in SpoutSDK.cpp 
		// We opened it so close it, otherwise it is never released
		CloseHandle(hMutex);
	}

	// Restore the host window
	if(IsWindow(hWnd)) {
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
	string namestring;
		
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
	std::set<string> Senders;
	std::set<string>::iterator iter;
	string namestring;
	SharedTextureInfo info;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {

		// Make Version infor light grey
		case WM_CTLCOLORSTATIC:
        {
			HDC hdcStatic = (HDC)wParam;
			// lParam is the handle to the control
			if(GetDlgItem(hDlg, IDC_VERS) == (HWND)lParam) {
				SetTextColor(hdcStatic, RGB(140, 140, 140));
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
				for(iter = Senders.begin(); iter != Senders.end(); iter++) {
					namestring = *iter; // the Sender name string
					strcpy_s(name, namestring.c_str());
					// we have the name already, so look for it's info
					if(!sendernames.getSharedInfo(name, &info)) {
						// Sender does not exist any more
						sendernames.ReleaseSenderName(name); // release from the shared memory list
					}
				}
			}

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
					SetDlgItemTextA(hDlg, IDC_ACTIVE, (LPCSTR)activename);
					sendernames.getSharedInfo(activename, &info);

					bool bUnKnown = false;

					switch(info.format) {
						// DX9
						case 0 : // default unknown
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
						default:
							sprintf_s(temp, 512, "%dx%d :", info.width, info.height);
							bUnKnown = true;
							break;
					}

					if(bDX9compatible && !bUnKnown) { // Specify DX9 compatible senders
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
							printf("Sharehandle      [%x]\n", info.shareHandle);
							printf("Width            [%d]\n", info.width);
							printf("Height           [%d]\n", info.height);
							printf("Format           [%d] (%x)\n", info.format, info.format);
							printf("Usage            [%d]\n", info.usage);
						}
					}

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
						sprintf_s(itemstring, "%s : (%dx%d)", name, info.width, info.height);
						pos = (int)SendMessageA(hwndList, LB_ADDSTRING, 0, (LPARAM)itemstring); 
						SendMessageA(hwndList, LB_SETITEMDATA, pos, (LPARAM)item);
						// Was it the active Sender
						// Listbox cannot be sorted to do this
						if(strcmp(name, activename) == 0) {
							SenderItem = item;
						}
						item++;
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

				case IDOK: // 1

					// printf("\nIDOK\n");

					// Get contents of edit field into the global char string
					GetDlgItemTextA(hDlg, IDC_ACTIVE, (LPSTR)SpoutSenderName, 256);
					if(strlen(SpoutSenderName) > 0) {

						// printf("    Sender  [%s]\n", SpoutSenderName);

						// Does it have any shared info
						if(sendernames.getSharedInfo(SpoutSenderName, &info)) {

							// printf("    Width   %d\n", info.width);
							// printf("    Height  %d\n", info.height);
							// printf("    Format  %d\n", info.format);
							// printf("    Handle  %u (%x)\n", info.shareHandle, info.shareHandle);


							// Check the DirectX texture format and quit if not
							// compatible if in compatibility mode
							if(bDX9compatible) { // Specify DX9 compatible senders
								// If it is 0 or 87 the sender is DX9 compatible
								if(! ( info.format == 0  // default directX 9
									|| info.format == 87 // compatible DX11
									|| info.format == 22 // DX9 XRGB
									|| info.format == 21 // DX9 ARGB
									)) {

									// printf("Specified DX9 and the format (%d) is not compatible\n", info.format);

									EndDialog(hDlg, LOWORD(wParam));
									return TRUE;
								}
							}

							// Is it registered ?
							if(!sendernames.FindSenderName(SpoutSenderName)) {

								// printf("    Not registerd\n");
								

								// Allow for a Sender which is not registered - e.g. VVVV
								// Registering the sender here will only work if another sender
								// is running or "SpoutTray" is present and has been activated 
								// to show the sender list after this sender has been registered, 
								// because this instance of spoutSenderNames for spoutpanel
								// will close and erase the active name map and the sender map
								// and any map handle in this app will be closed

								// Failsafe method - open a text file and write to it,
								// then delete it with the app when it is read or when
								// SpoutPanel opens again. The path in calling app has to be 
								// the same as for SpoutPanel.exe

								ofstream myfile;
								HMODULE module;
								char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];
								module = GetModuleHandle(NULL);
								GetModuleFileNameA(module, path, MAX_PATH);
								_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
								_makepath_s(path, MAX_PATH, drive, dir, "spoutpanel", ".txt");
								myfile.open (path, ios::out | ios::app);
								if (myfile.is_open()) {
									myfile.write(SpoutSenderName, strlen(SpoutSenderName));
									myfile.close();
									// Register with the calling app
								}
							} // endif was not registered
							// drop through
						}
						else {
							// Serious enough for a messagebox
							sprintf_s(name, "Sender [%s] does not exist", SpoutSenderName);
							MessageBoxA(hDlg, name, "SpoutPanel", 0);
							EndDialog(hDlg, LOWORD(wParam));
							return TRUE;
						}

						// Set the selected name as the active Sender
						// Any receiver can then query the active Sender name
						// printf("    Setting as the active sender\n");
						sendernames.SetActiveSender(SpoutSenderName);

						// LJ DEBUG to halt for debugging
						// sprintf_s(name, "Sender [%s] OK", SpoutSenderName);
						// MessageBoxA(hDlg, name, "SpoutPanel", 0);
					}
				
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
										default:
											sprintf_s(temp, 512, "%dx%d : incompatible format [%d]", info.width, info.height, info.format);
											break;
									}
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

bool OpenFile(char *filepath, const char *extension, int maxchars)
{
	OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // TODO hwnd;

	// Set defaults
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0"; // TODO different file types as args
    ofn.lpstrDefExt = "txt";

	// What extension did the user last use?
	if(extension) {
		// Known file types
		// .glsl, .frag
		if(strcmp(extension, ".glsl") == 0) {
			ofn.lpstrFilter = "Shader Files (*.glsl)\0*.glsl\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "glsl";
		}
		else if(strcmp(extension, ".frag") == 0) {
			ofn.lpstrFilter = "Shader Files (*.frag)\0*.frag\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "frag";
		}
	}

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    // ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;

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
	argw = CommandLineToArgvW( wcCommandLine, &argc);

	// Create the first dimension of the double array
	argv = (char **)GlobalAlloc( LPTR, argc + 1 );
	
	// convert each line of wcCommandeLine to MultiByte and place them
	// to the argv[] array
	for( i=0; i < argc; i++) {
		BuffSize = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, argw[i], -1, NULL, 0, NULL, NULL );
		argv[i] = (char *)GlobalAlloc( LPTR, BuffSize );		
		WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, argw[i], BuffSize * sizeof( WCHAR ) ,argv[i], BuffSize, NULL, NULL );
	}
	
	// return the number of arguments
	return argc;

} // ParseCommandline()

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
		printf("SpoutPanel - creating a new registry key (%s)\n", mySubKey);
		// Create a new key
		regres = RegCreateKeyExA(HKEY_CURRENT_USER, mySubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
	}

	if(regres == ERROR_SUCCESS && hRegKey != NULL) {
		printf("SpoutPanel writing the path to the registry\n[%s]\n", filepath);
		// Write the path
		regres = RegSetValueExA(hRegKey, valuename, 0, REG_SZ, (BYTE*)filepath, ((DWORD)strlen(filepath) + 1)*sizeof(unsigned char));
		// For immediate read after write - necessary here becasue the app that opeded SpoutPanel
		// will read the registry straight away and it might not be available yet
		// The key must have been opened with the KEY_QUERY_VALUE access right (included in KEY_ALL_ACCESS)
		RegFlushKey(hRegKey); // needs an open key
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

