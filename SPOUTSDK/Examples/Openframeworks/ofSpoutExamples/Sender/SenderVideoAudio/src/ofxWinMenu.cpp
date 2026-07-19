/*

	ofxWinMenu

	Create a menu for a Microsoft Windows Openframeworks application.
	
	Copyright (C) 2016-2026 Lynn Jarvis.

	https://github.com/leadedge

	http://www.spout.zeal.co

    =========================================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    =========================================================================

	17.11.16 - fixed submenu item count in AddPopupSeparator
			 - fixed submenu item increment in SetPopupItem
	21.02.17 - changed constructor to take the window handle
	23.12.17 - Add WM_ENTERMENULOOP and WM_EXITMENULOOP
	29.11.19 - Corrected SetClassLong > SetClassLongPtrA for 64 bits
	19.09.20 - Add EnablePopupItem
	01.10.21 - Correct AddPopupSeparator to include MF_BYPOSITION
	07.05.22 - Change EnablePopupItem to use menu item number directly
	07.12.23 - used std:: thoughout instead of depending on "using namespave std"
	02.01.25 - Add GetPopupItem, Save and Load functions
	09.01.25 - Return new values to ofApp in Load function
	10.01.25 - Change Load from void to bool
	17.01.25 - Constructor - change LongPtrA functions to LongPtr
	18.01.25 - Constructor - conditional Unicode for menu name
	12.05.25 - Load/Save allow for empty file name
	12.06.26 - Load - Load item states 
			   Test for tmp returned by GetPrivateProfileString


*/
#include "ofxWinMenu.h"

static LRESULT CALLBACK ofxWinMenuWndProc(HWND, UINT, WPARAM, LPARAM); // Local window message procedure
static WNDPROC ofAppWndProc; // Openframeworks application window message procedure
static ofxWinMenu *pThis; // Pointer to access the ofxWinMenu class from the window procedure

ofxWinMenu::ofxWinMenu(ofApp *app, HWND hwnd) {

	g_hMenu = NULL; // Set by CreateMenu and returned to ofApp
	pAppMenuFunction = NULL; // Set by CreateMenuFunction to return menu state to ofApp

	// The window handle of ofApp
	g_hwnd = hwnd;

	pThis = this; // Pointer for access the ofxWinMenu class
	pApp = app; // The ofApp class pointer

	// Save the Openframeworks application window message procedure
	ofAppWndProc = (WNDPROC)GetWindowLongPtr(g_hwnd, GWLP_WNDPROC);

	// Set our own window message procedure
	SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)ofxWinMenuWndProc);

	// Set the Menu name
	#ifdef UNICODE
	SetClassLongPtr(g_hwnd, GCLP_MENUNAME, (LONG_PTR)L"ofxWinMenu");
	#else
	SetClassLongPtrA(g_hwnd, GCLP_MENUNAME, (LONG_PTR)"ofxWinMenu");
	#endif

}

ofxWinMenu::~ofxWinMenu()
{
	// Clear all vectors
	subMenus.clear();
	itemIDs.clear();
	autoCheck.clear();
	isChecked.clear();

}

//
// MENU
//

// Main menu we will create
HMENU ofxWinMenu::CreateWindowMenu()
{
	HMENU hMenu = GetMenu(g_hwnd);
	if(!hMenu) 
		g_hMenu = CreateMenu();
	return g_hMenu;
}

// Popup menu of the main menu
HMENU ofxWinMenu::AddPopupMenu(HMENU hMenu, std::string MenuName)
{
	if(hMenu) {
		HMENU hSubMenu = CreatePopupMenu();
		if(hSubMenu) {
			AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, MenuName.c_str());
			return hSubMenu;
		}
	}
	return NULL;

}

//
// Popup menu items
//
//     hSubMenu   - the popup menu to add an item to
//     ItemName   - the name of the menu item which will appear in the menu
//     bChecked   - initial state of the menu item, checked or not
//     bAutoCheck - Check the item on or off automatically on selection
//
bool ofxWinMenu::AddPopupItem(HMENU hSubMenu, std::string ItemName)
{
	return AddPopupItem(hSubMenu, ItemName, false, true);
}

bool ofxWinMenu::AddPopupItem(HMENU hSubMenu, std::string ItemName, bool bChecked)
{
	return AddPopupItem(hSubMenu, ItemName, bChecked, true);
}

bool ofxWinMenu::AddPopupItem(HMENU hSubMenu, std::string ItemName, bool bChecked, bool bAutoCheck)
{
	if(g_hMenu && hSubMenu) {
		int nItem = GetMenuItemCount(hSubMenu);
		int itemID = (int)itemIDs.size();
		itemIDs.push_back(nItem);
		subMenus.push_back(hSubMenu);
		itemNames.push_back(ItemName);
		isChecked.push_back(bChecked);
		autoCheck.push_back(bAutoCheck);
		if(InsertMenuA(hSubMenu, nItem, MF_BYPOSITION, itemID, ItemName.c_str())) {
			if(bAutoCheck && bChecked) 
				CheckMenuItem(hSubMenu, nItem, MF_BYPOSITION | MF_CHECKED);
			return true;
		}
	}
	return false;
}

// Item separator
bool ofxWinMenu::AddPopupSeparator(HMENU hSubMenu)
{
	int nItems = 0;
	HMENU hSubSubMenu = NULL;

	if(g_hMenu && hSubMenu) {

		int n = GetMenuItemCount(hSubMenu);

		// Include popup submenus - allow for one level deep
		for(int i = 0; i < n; i++) {
			nItems++;
			// Is the item a submenu of the popup menu ?
			hSubSubMenu = GetSubMenu(hSubMenu, i);
			if(hSubSubMenu) {
				nItems++; // Include the submenu itself
				// Add it's items to the incrementing count as we build the menu
				nItems += GetMenuItemCount(hSubMenu); 
			}
		}
		itemIDs.push_back(nItems);
		subMenus.push_back(hSubMenu);
		itemNames.push_back("");
		isChecked.push_back(false);
		autoCheck.push_back(false);

		//
		// The position indicates the menu item before which the new menu item is to be inserted
		// as determined by the uFlags parameter (MF_BYPOSITION).
		//
		return (bool)InsertMenuA(hSubMenu, nItems, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	}
	return false;
}

// Set the menu to the application
bool ofxWinMenu::SetWindowMenu()
{
	if(g_hwnd && g_hMenu)
		return (bool)SetMenu(g_hwnd, g_hMenu);
	else
		return false;
}

// Remove the menu from the application
bool ofxWinMenu::RemoveWindowMenu()
{
	if(g_hwnd)
		return (bool)SetMenu(g_hwnd, NULL);
	else
		return false;
}

// Destroy the menu - not normally used
bool ofxWinMenu::DestroyWindowMenu()
{
	if(g_hMenu)
		return (bool)DestroyMenu(g_hMenu);
	else
		return false;
}


// Check or uncheck a menu item
bool ofxWinMenu::SetPopupItem(std::string ItemName, bool bChecked)
{
	if(g_hwnd == NULL || g_hMenu == NULL || !IsMenu(g_hMenu)) return false;
	
	int nItems = (int)itemIDs.size();
	if(nItems > 0) {
		// Find the item number
		for(int i=0; i<nItems; i++) {
			if(ItemName == itemNames.at(i)) {
				// Which popup menu is the item in
				HMENU hSubMenu = subMenus.at(i);
				if(hSubMenu) {
					// How many items in the submenu
					int nPopupItems = GetMenuItemCount(hSubMenu);
					// Loop through the popup items to find a match
					if(nPopupItems > 0) {
						char itemstring[MAX_PATH];
						for(int j=0; j<nPopupItems; j++) {
							GetMenuStringA(hSubMenu, j, (LPSTR)itemstring, MAX_PATH, MF_BYPOSITION);
							if(ItemName == itemstring) {
								if(bChecked)
									CheckMenuItem(hSubMenu, j, MF_BYPOSITION | MF_CHECKED);
								else
									CheckMenuItem(hSubMenu, j, MF_BYPOSITION | MF_UNCHECKED);
								isChecked.at(i) = bChecked;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

// Enable or disable a popup item
bool ofxWinMenu::EnablePopupItem(std::string ItemName, bool bEnabled)
{
	if (g_hwnd == NULL || g_hMenu == NULL || !IsMenu(g_hMenu)) return false;

	if (itemIDs.size() > 0) {
		// Find the item number
		for (int i = 0; i < (int)itemIDs.size(); i++) {
			if (ItemName == itemNames.at(i)) {
				if (bEnabled)
					EnableMenuItem(g_hMenu, i, MF_ENABLED);
				else
					EnableMenuItem(g_hMenu, i, MF_DISABLED);
				return true;
			}
		}
	}
	return false;
}

// Get the checkmark state of a popup item
bool ofxWinMenu::GetPopupItem(std::string ItemName)
{
	if (itemIDs.size() > 0) {
		// Find the item number
		for (int i = 0; i < (int)itemIDs.size(); i++) {
			if (ItemName == itemNames.at(i)) {
				return isChecked[i];
			}
		}
	}
	return false;
}

// Save popup item states to an initialization file
void ofxWinMenu::Save(std::string filename, bool bOverWrite)
{
	char tmp[MAX_PATH]{};
	std::string inipath;

	// If no filename, create ini file from exe path
	if (filename.empty()) {
		// Find the path of the executable
		char logpath[MAX_PATH] {};
		if (GetModuleFileNameA(NULL, (LPSTR)logpath, sizeof(logpath))) {
			inipath = logpath;
			// Strip ".exe" and replace with ".ini"
			inipath = inipath.substr(0, inipath.rfind(".")) + ".ini";
		}
	}
	else if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
		// Check for full path
		inipath = filename;
	}
	else {
		// filename only - add full path - (executable directory)
		char path[MAX_PATH] {};
		GetModuleFileNameA(NULL, path, MAX_PATH);
		PathRemoveFileSpecA(path);
		inipath = path;
		inipath += "\\data\\";
		// Does the folder exist ?
		if (_access(inipath.c_str(), 0) != -1) {
			// Openframeworks application
			inipath += filename;
		}
		else {
			// Executable folder
			inipath = path;
			inipath += "\\";
			inipath += filename;
		}
	}

	// Check extension
	size_t pos = inipath.rfind(".ini");
	if (pos == std::string::npos) {
		// No extension or not ".ini"
		pos = inipath.rfind(".");
		if (pos == std::string::npos) {
			// No extension - add ".ini"
			inipath = inipath + ".ini";
		} else {
			// Extension not "ini"
			// Strip extension
			inipath = inipath.substr(0, pos);
			// Add ".ini"
			inipath = inipath + ".ini";
		}
	}

	// Check if file exists if bOverWrite is false (default)
	if (!bOverWrite && _access(inipath.c_str(), 0) != -1) {
		sprintf_s(tmp, MAX_PATH, "%s exists - overwrite?", inipath.c_str());
		if (MessageBoxA(NULL, tmp, "Warning", MB_YESNO | MB_TOPMOST) == IDNO)
			return;
	}

	if (itemIDs.size() > 0) {
		// Find the item number
		for (int i = 0; i < (int)itemIDs.size(); i++) {

			if (autoCheck[i]) {
				// For debugging
				// sprintf_s(tmp, MAX_PATH, "Save : item [%s] = %d\n", itemNames.at(i).c_str(), (bool)isChecked[i]);
				// MessageBoxA(NULL, tmp, "Save", MB_OK | MB_TOPMOST);
				if (isChecked[i])
					WritePrivateProfileStringA((LPCSTR)"Menu", (LPCSTR)itemNames.at(i).c_str(), (LPCSTR)"1", (LPCSTR)inipath.c_str());
				else
					WritePrivateProfileStringA((LPCSTR)"Menu", (LPCSTR)itemNames.at(i).c_str(), (LPCSTR)"0", (LPCSTR)inipath.c_str());
			}
		}
	}
}

// Load item states from an initialization file
bool ofxWinMenu::Load(std::string filename)
{
	char tmp[MAX_PATH]{};
	std::string inipath="";

	// If no filename, create ini file from exe path
	if (filename.empty()) {
		// Find the path of the executable
		char logpath[MAX_PATH] {};
		if (GetModuleFileNameA(NULL, (LPSTR)logpath, sizeof(logpath))) {
			inipath = logpath;
			// Strip ".exe" and replace with ".ini"
			inipath = inipath.substr(0, inipath.rfind(".")) + ".ini";
		}
	}
	else if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
		// Check for full path
		inipath = filename;
	}
	else {
		// filename only - add full path - (bin\data or executable directory)
		char path[MAX_PATH]{};
		GetModuleFileNameA(NULL, path, MAX_PATH);
		PathRemoveFileSpecA(path);
		inipath = path;
		inipath += "\\data\\";
		// Does the folder exist ?
		if (_access(inipath.c_str(), 0) != -1) {
			// Openframeworks application
			inipath += filename;
		}
		else {
			// Executable folder
			inipath = path;
			inipath += "\\";
			inipath += filename;
		}
	}

	// Check extension
	size_t pos = inipath.rfind(".ini");
	if (pos == std::string::npos) {
		// No extension or not ".ini"
		pos = inipath.rfind(".");
		if (pos == std::string::npos) {
			// No extension - add ".ini"
			inipath = inipath + ".ini";
		} else {
			// Extension not "ini"
			// Strip extension
			inipath = inipath.substr(0, pos);
			// Add ".ini"
			inipath = inipath + ".ini";
		}
	}

	// Check that the file exists in case an extension was added
	if (_access(inipath.c_str(), 0) == -1) {
		printf("ofxWinMenu::Load\nInitialization file \"%s\" not found.\n", inipath.c_str());
		return false;
	}

	// Only those saved in the ini file are changed
	if (itemIDs.size() > 0) {
		for (int i = 0; i < (int)itemIDs.size(); i++) {
			if (GetPrivateProfileStringA((LPCSTR)"Menu", (LPCSTR)(LPCSTR)itemNames.at(i).c_str(), NULL, (LPSTR)tmp, MAX_PATH, (LPCSTR)inipath.c_str()) > 0) {
				if (tmp[0]) {
					isChecked[i] = (bool)(atoi(tmp) == 1);
					SetPopupItem(itemNames.at(i), isChecked[i]);
					// Return value to ofApp
					MenuFunction(itemNames.at(i), isChecked[i]);
				}
			}
		}
	}
	return true;
}

// ofApp Function for return of memu item selection
void ofxWinMenu::CreateMenuFunction(void(ofApp::*function)(std::string title, bool bChecked))
{
	pAppMenuFunction = function; // Return function in ofApp
}

// Pass back the menu item title and state to ofApp
// by calling the function set by "CreateMenuFunction"
void ofxWinMenu::MenuFunction(std::string title, bool bChecked)
{
	(pApp->*pAppMenuFunction)(title, bChecked); 
}


//
// Our local window message callback procedure
//
LRESULT CALLBACK ofxWinMenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hSubMenu = NULL;
	char title[MAX_PATH];
	int ID = 0;

	// Menu item ID
	int wmId = (int)LOWORD(wParam);

	switch (uMsg) {	

		// Check For Windows system messages
		case WM_SYSCOMMAND:

			switch (wParam)	{         // Check System Calls
				case SC_SCREENSAVE:   // Screensaver Trying To Start?
				case SC_MONITORPOWER: // Monitor Trying To Enter Powersave?
				return 0;             // Prevent From Happening
			}
			break;

			// LJ DEBUG
			// MessageBoxA(NULL, "WM_SYSCOMMAND", "Warning", MB_ICONWARNING | MB_OK);

		case WM_ENTERSIZEMOVE:
			// Inform ofApp of window resize start
			pThis->MenuFunction("WM_ENTERSIZEMOVE", true);
			break;

		case WM_EXITSIZEMOVE:
			// Inform ofApp of window resize exit
			pThis->MenuFunction("WM_EXITSIZEMOVE", true);
			break;

		case WM_ENTERMENULOOP:
			// Inform ofApp of menu entry
			pThis->MenuFunction("WM_ENTERMENULOOP", true);
			break;

		case WM_EXITMENULOOP :
			// Inform ofApp of menu exit
			pThis->MenuFunction("WM_EXITMENULOOP", true);
			break;

		case WM_NCLBUTTONDOWN :
			// Inform ofApp of non client mouse press
			pThis->MenuFunction("WM_NCLBUTTONDOWN", true);
			break;

		case WM_COMMAND:

			// Inform ofApp which menu has been selected
			hSubMenu = pThis->subMenus.at(wmId); // Submenu that the menu item is in
			ID = pThis->itemIDs.at(wmId); // Position of the item in the submenu

			// Check the menu item if autocheck is enabled for it
			if(pThis->autoCheck.at(wmId)) {
				if(pThis->isChecked.at(wmId)) // currently checked
					CheckMenuItem (hSubMenu, ID, MF_BYPOSITION | MF_UNCHECKED); // uncheck it
				else
					CheckMenuItem (hSubMenu, ID, MF_BYPOSITION | MF_CHECKED); // otherwise check it
				pThis->isChecked.at(wmId) = !pThis->isChecked.at(wmId); // toggle the menu item state flag
			}
			
			// Get the menu item title
			GetMenuStringA(hSubMenu, wmId, (LPSTR)title, MAX_PATH, MF_BYCOMMAND);

			// Inform ofApp of the menu item title and new state
			pThis->MenuFunction(title, pThis->isChecked.at(wmId));

			break;

		case WM_CLOSE: {         // Close Message
			PostQuitMessage(0);  // Send A Quit Message
			return 0;            // Jump Back
		}
		// Openframeworks handles key and mouse and drag/drop etc.
	}

	// Pass unhandled messages on to the openframeworks application
	return(CallWindowProc(ofAppWndProc, hWnd, uMsg, wParam, lParam));

}

