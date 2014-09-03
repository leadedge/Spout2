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
//
#include <windows.h>
#include <vector>
#include <fstream>
#include "resource.h"

#include "../../../../SpoutSDK/SpoutSenderNames.h"

char SpoutSenderName[256]; // global Sender name to retrieve from the list dialog
char UserMessage[512]; // User message for the text dialog

bool bDX9compatible = false; // Only list DX9 compatible senders - modified by /DX9 arg
bool bArgFound = false;
bool GetSenderDialog(HINSTANCE hInst);
int ParseCommandline();
char **argv = NULL;
spoutSenderNames sendernames; // Names class functions

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
	bool bRet = false;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutPanel\n");
	*/

	// Find the current active window to restore to the top when SpoutPanel quits
	hWnd = GetForegroundWindow();

	// Remove any temporary file
	module = GetModuleHandle(NULL);
	GetModuleFileNameA(module, path, MAX_PATH);
	_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
	_makepath_s(path, MAX_PATH, drive, dir, "spoutpanel", ".txt");
	remove(path);

	// Check for arguments
	UserMessage[0] = 0;
	argc = ParseCommandline();
	if( argc > 1) { // 0 = "SpoutPanel"
		hWnd = GetActiveWindow();
		EnableWindow(hWnd, FALSE);
		for( i=1; i <argc; i++ ) {
			// Argument /DX11
			if ( strcmp(argv[i], "/DX11") == 0) {
				// printf("DX11 mode\n");
				bArgFound = true;
				bDX9compatible = false;
			}
			// Argument /DX9
			else if(strcmp(argv[i], "/DX9") == 0) {
				// printf("DX9 mode\n");
				bArgFound = true;
				bDX9compatible = true;
			}
			else {
				bArgFound = false;
			}
		}

		if(!bArgFound && lpCmdLine) {
			// Know listed args, so send a user message
			strcpy_s(UserMessage, 512, lpCmdLine); // Message to be shown instead of sender list
		}

		EnableWindow(hWnd, TRUE);
	}

	// Try to open the application mutex.
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

	if (!hMutex) {
		hMutex = CreateMutexA(0, 0, "SpoutPanel");
		if(UserMessage[0]) {
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
		/*
		// printf("Mutex exists\n");
		// The mutex exists, so another instance is already running
		// Find the dialog window and bring it to the top
		// the spout dll dialog is topmost anyway but pop it to
		// the front in case anything else has stolen topmost
		hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		GetWindowTextA(hWnd, windowname, 256);
		if(IsWindow(hWnd)) {
			// printf("SpoutPanel exists\n");
			SetForegroundWindow(hWnd); 
			GetWindowRect(hWnd, &rect);
			w = rect.right - rect.left; h = rect.bottom - rect.top;
			SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, w, h, SWP_SHOWWINDOW);
		}
		*/
		CloseHandle(hMutex);
	}

	// Restore the host window
	if(IsWindow(hWnd)) {
		SetForegroundWindow(hWnd); 
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
	RECT rect;
	int w, h, pos,	lbItem, item;
	int SenderItem = -1;
	std::set<string> Senders;
	std::set<string>::iterator iter;
	string namestring;
	SharedTextureInfo info;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		
		case WM_INITDIALOG:

			int x, y;
			POINT p;

			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
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
				// Is it registered e.g. VVVV which has been accessed ?
				if(!sendernames.FindSenderName(activename)) {
					if(sendernames.getSharedInfo(activename, &info)) {
						// printf("Registering unlisted active sender\n");
						sendernames.RegisterSenderName(activename);
					}
				}
				// Now it should be in the name set
				if(sendernames.FindSenderName(activename)) {
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

				case IDOK: // 1

					// Get contents of edit field into the global char string
					GetDlgItemTextA(hDlg, IDC_ACTIVE, (LPSTR)SpoutSenderName, 256);
					if(strlen(SpoutSenderName) > 0) {
						// Does it have any shared info
						if(sendernames.getSharedInfo(SpoutSenderName, &info)) {
							// Check the DirectX texture format and quit if not
							// compatible if in Dr=irectX 11 and compatibility mode
							if(bDX9compatible) { // Specify DX9 compatible senders
								// If it is 0 or 87 the sender is DX9 compatible
								if(info.format != 0 && info.format != 87) {
									EndDialog(hDlg, LOWORD(wParam));
									return TRUE;
								}
							}

							// Is it registered ?
							if(!sendernames.FindSenderName(SpoutSenderName)) {
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
						sendernames.SetActiveSender(SpoutSenderName);
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
									// printf("(%s) - %dx%d [%x]\n", name, info.width, info.height, info.format);
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
			int x, y, w, h;
			POINT p;
			RECT rect;

			SetDlgItemTextA(hDlg, IDC_USERTEXT, (LPCSTR)UserMessage);

			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
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
			w = rect.right - rect.left; 
			h = rect.bottom - rect.top;
			SetWindowPos(hDlg, HWND_TOPMOST, x, y, w, h, SWP_SHOWWINDOW);

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


