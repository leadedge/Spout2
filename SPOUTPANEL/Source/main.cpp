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
//	01.08.14 - converted to Spout SDK - used only the sendernames class
//
#include <windows.h>
#include "resource.h"
// #include "spxConnector.h"

#include "../../../../SpoutSDK/SpoutSenderNames.h"

#include <vector>

#define MaxSenders 10 // Max for list of Sender names

#if defined(__x86_64__) || defined(_M_X64)
	#define is64bit
#elif defined(__i386) || defined(_M_IX86)
	// x86 32-bit
#endif
// Probably don't need this, but in case we ever need common controls in the Sender dialog.
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

char SpoutSenderName[256];				// global Sender name to retrieve from the dialog
bool bDX9compatible = false;			// Only list DX9 compatible senders - needs /DX9 arg passed
// spxConnector dxConnector;
bool GetSenderDialog(HINSTANCE hInst);
int ParseCommandline();
char **argv = NULL;

spoutSenderNames sendernames;


INT_PTR CALLBACK SenderListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	RECT rect;
	char windowname[512];
	int w, h;
	int i, argc;
	bool bRet;

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutPanel\n");
	*/


	// Argument /DX11
	// Check for arguments
	argc = ParseCommandline();
	if( argc > 1) { // 0 = "SpoutPanel"
		hWnd = GetActiveWindow();
		EnableWindow(hWnd, FALSE);
		for( i=1; i <argc; i++ ) {
			// printf("Arg[%d] (%s)\n", i, argv[i]);
			// Arg 1 - 
			if ( strcmp(argv[i], "/DX11") == 0) {
				// MessageBoxA(NULL, "DirectX 11", "Spout", MB_OK);
				bDX9compatible = false;
			}
			else if(strcmp(argv[i], "/DX9") == 0) {
				// MessageBoxA(NULL, "DirectX 9", "Spout", MB_OK);
				bDX9compatible = true;
			}
			else {
				// MessageBoxA(NULL, lpCmdLine, "Spout", MB_OK); // show the whole command line
				EnableWindow(hWnd, TRUE);
				return 1; // Cancel so no action is taken			
			}
		}
		EnableWindow(hWnd, TRUE);
	}

	// printf("SpoutPanel bDX9compatible = %d\n", bDX9compatible);


	// Try to open the application mutex.
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

	if (!hMutex) {
		hMutex = CreateMutexA(0, 0, "SpoutPanel");
		bRet = (bool)GetSenderDialog(hInstance); // activate the dialog to select the active Sender
	
		// char temp[512];
	    // sprintf(temp, "SpoutPanel exit = %d", bRet);
	    // MessageBoxA(NULL, temp, "Info", MB_OK);

		// This is modal so the mutex remains until it closes
		if(hMutex) {
			ReleaseMutex(hMutex);
		}
	}
	else {
		// The mutex exists, so another instance is already running
		// Find the dialog window and bring it to the top
		// the spout dll dialog is topmost anyway but pop it to
		// the front in case anything else has stolen topmost
		hWnd = FindWindowA(NULL, (LPCSTR)"Spout Senders");
		GetWindowTextA(hWnd, windowname, 256);
		if(IsWindow(hWnd)) {
			SetForegroundWindow(hWnd); 
			GetWindowRect(hWnd, &rect);
			w = rect.right - rect.left; h = rect.bottom - rect.top;
			SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, w, h, SWP_SHOWWINDOW);
		}
		CloseHandle(hMutex);
	}

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
		
	// create a modal dialog box
	// This is a modal dialog so will stop this application at this point
	nRet = DialogBoxParamA(hInst, MAKEINTRESOURCEA(IDD_DIALOG1), 0, SenderListDlgProc, 0);

		// char temp[512];
	    // sprintf(temp, "nRet = %d, name = %s", nRet, SpoutSenderName);
	    // MessageBoxA(NULL, temp, "Info", MB_OK);

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
	HWND hwndList;
	RECT rect;
	int w, h, pos,	lbItem, item;
	int SenderItem = -1;
	std::set<string> Senders;
	std::set<string>::iterator iter;
	string namestring;
	SharedTextureInfo info;

	switch (message) {
		
		case WM_INITDIALOG:

			int x, y;
			POINT p;
			// prevent other windows from hiding the dialog
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
			SetWindowPos(hDlg, HWND_TOPMOST, x, y, w, h, SWP_SHOWWINDOW);

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
				if(sendernames.FindSenderName(activename)) {
					// printf("SpoutPanel: active sender (%s)\n", activename);
					SetDlgItemTextA(hDlg, IDC_ACTIVE, (LPCSTR)activename);

					sendernames.getSharedInfo(activename, &info);

					if(info.format == 0) {
						sprintf_s(temp, 512, "DirectX 9 : %dx%d", info.width, info.height);
					}
					else {
						sprintf_s(temp, 512, "DirectX 11 : %dx%d : format [%d]", info.width, info.height, info.format);
						if(bDX9compatible) {
							if(info.format != 87) {
								sprintf_s(temp, 512, "DirectX 11 : %dx%d : incompatible format [%d]", info.width, info.height, info.format);
							}
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

						// printf("Adding (%s) item %d, pos %d\n", name, item, pos);

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

			// for match with active Sender otherwise make the user choose
			if(SenderItem >= 0) {
				SendMessageA(hwndList, LB_SETCURSEL, SenderItem, 0);
				return FALSE;
			}

			return TRUE; // return TRUE  unless you set the focus to a control

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDOK:		// 1

					// printf("SpoutPanel Dialog OK\n");

					// Get contents of edit field into the global char string
					GetDlgItemTextA(hDlg, IDC_ACTIVE, (LPSTR)SpoutSenderName, 256);

					if(strlen(SpoutSenderName) > 0) {

						// printf("IDC_ACTIVE (%s)\n", SpoutSenderName);

						// ==========================================================
						// Does it have any shared info
						if(sendernames.getSharedInfo(SpoutSenderName, &info)) {
							// Check the DirectX texture format
							// If it is 0 or 87 the sender is DX9 compatible
							if(bDX9compatible) { // Specify DX9 compatible senders
								// Is it dx9 compatible
								if(info.format != 0 && info.format != 87) {
									EndDialog(hDlg, LOWORD(wParam));
									return TRUE;
								}
							}
							
							// Allow for a Sender which is not registered - e.g. VVVV
							// LJ DEBUG this will not work because the instance 
							// of spoutSenderNames for spoutpanel will erase the sender map
							sendernames.RegisterSenderName(SpoutSenderName);

						}
						else {
							// Serious enough for a messagebox
							// printf("Sender [%s] does not exist\n", SpoutSenderName);
							sprintf_s(name, "Sender [%s] does not exist", SpoutSenderName);
							MessageBoxA(hDlg, name, "Spout", 0);
							EndDialog(hDlg, LOWORD(wParam));
							return TRUE;
						}
						// =============================================================

						// Set the selected name as the active Sender
						// Any client can then query the active Sender name
						// printf("Setting active sender to (%s)\n", SpoutSenderName);

						// sprintf(temp, "setting active to (%s)", SpoutSenderName);
						// MessageBoxA(NULL, temp, "Info", MB_OK);
						sendernames.SetActiveSender(SpoutSenderName);

					}
				
				case IDCANCEL:	// 2

					// sprintf(temp, "SpoutPanel Dialog exit = %d", LOWORD(wParam));
					// MessageBoxA(NULL, temp, "Info", MB_OK);
					// printf("SpoutPanel Dialog exit = %d\n", LOWORD(wParam));

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
							// printf("Selected Item = %d\n", lbItem);
							if(lbItem != LB_ERR) {
								// Get the saved index
								SendMessageA(hwndList, LB_GETTEXT, lbItem, (LPARAM)SpoutSenderName);
		                        pos = (int)SendMessageA(hwndList, LB_GETITEMDATA, lbItem, 0);
								// printf("Selected Item pos = %d\n", pos);
								// Get the details of the Sender
								if(Senders.size() > 0) {
									iter = std::next(Senders.begin(), pos);
									namestring = *iter; // the selected Sender in the list
									strcpy_s(name, namestring.c_str());
									SetDlgItemTextA(hDlg, IDC_ACTIVE, (LPCSTR)name);

									// ===============================================
									sendernames.getSharedInfo(name, &info);
									if(info.format == 0) {
										sprintf_s(temp, "DirectX 9 : %dx%d", info.width, info.height);
									}
									else {
										sprintf_s(temp, "DirectX 11 : %dx%d : format [%d]", info.width, info.height, info.format);
										if(bDX9compatible) {
											if(info.format != 87) {
												sprintf_s(temp, "DirectX 11 : %dx%d : incompatible format [%d]", info.width, info.height, info.format);
											}
										}
									}
									SetDlgItemTextA(hDlg, IDC_INFO, (LPCSTR)temp);
									// ===============================================

								}
							}
			                return TRUE; 
					} // end switch (HIWORD(wParam))
				// end case IDC_LIST1
			} // end switch (LOWORD(wParam))
		// end case case WM_COMMAND:
		return TRUE;
	} // end switch (message)

	// printf("SpoutPanel Dialog End\n");

	return FALSE;

} // end SenderListDlgProc


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
	
	// return the number of argument
	return argc;

} // ParseCommandline()
