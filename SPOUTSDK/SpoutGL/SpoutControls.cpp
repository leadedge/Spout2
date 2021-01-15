//
//		SpoutControls
//
//		Allows an application to control a Spout sender
//
// ====================================================================================
//		Revisions :
//
//		21.06.15	- project start
//		08.07.15	- project Version 1
//		16.07.15	- changed CheckControls to return the control index that was changed
//					  int SpoutControls::CheckControls(vector<control> &controls)
//		17.07.15	- introduced two versions of CheckControls
//					  introduced Lesser GPL licence
//		22.07.15	- included CreateControl
//					- removed  clear control file path from the registry
//					  so that it remains for controllers to find the last sender started
//		27.07.15	- Added "OpenSpoutController"
//		18.08.15	- Cleanup for 1.002 release
//		24.09.15	- re-ordering in cleanup
//		24.12.15	- added executable path to registry "ControlPath"
//		30.12.15	- restored CopyControlFile
//					- Moved GetControls public
//		13.01.16	- Changed OpenSpoutController to look for executable host path
//					  as well as SpoutControls installation path.
//		15.01.16	- Added fixed "NoClose" argument for opening SpoutController from an application
//					  so that the Close button is hidden.
//		17.01.16	- Added commandline argument to the OpenController function
//					  "NoClose" is now a key word
//					- CloseControls - look for the SpoutController window and close it
//		18.01.16	- added getcwd to OpenSpoutController
//					- cleanup on exit confirmed for Processing
//					- Instead of closecontrols, close the controller in CleanUp if it has been opened
//		26.01.16	- added default empty string for OpenSpoutController - then needs SpoutControls installation
//		27.01.16	- brought SpoutController.exe topmost before close to prevent freeze if hidden
//		19.02.16	- fixed command line for OpenSpoutController
//		08.05.16	- fixed OpenSpoutController function to detect SpoutController.exe
//		11.05.16	- 1.007
//		02.09.16	- safety checks for mailslot handle. Code cleanup.
//		17.01.17	- 1.008 release for Spout 2.006 - no code changes
//		27.01.17	- Changes to OpenSpoutController to find a SpoutControls installation
//		02.06.17	- Registry functions called from SpoutUtils
//		13.12.17	- OpenControls - returns false if no controls - no registry write
//		28.12.17	- Added GetFileControls
//		04.12.18	- corrected return value for GetFileControls
//		03.01.19	- Changed to revised registry functions in SpoutUtils
//		28.04.19	- Rebuild VS2017 - 2.007 - /MT
//		27.12.20	- Remove from Spout SDK and use for Processing library
//					  Rebuild Win32/x64 - /MT : VS2017 - 2.007
//		
//
// ====================================================================================
//
//		Copyright (C) 2015-2021. Lynn Jarvis, Leading Edge. Pty. Ltd.
//
//		This program is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You will receive a copy of the GNU Lesser General Public License along 
//		with this program.  If not, see http://www.gnu.org/licenses/.
//
#include "SpoutControls.h"

SpoutControls::SpoutControls()
{
	m_sharedMemoryName = ""; // Memory map name sring
	m_hSharedMemory = NULL; // Memory map handle
	m_pBuffer = NULL; // Shared memory pointer
	m_hAccessMutex = NULL; // Map access mutex
	m_hSlot = NULL; // Handle to the mailslot
	m_dwSize = 0; // Size of the memory map

}


//---------------------------------------------------------
SpoutControls::~SpoutControls()
{
	Cleanup();
}


// ======================================================================
//								Public
// ======================================================================


// ===================================================================== //
//                             CONTROLLER                                //
// ===================================================================== //


//---------------------------------------------------------
// Find a control map name in the registry
// This will only exist if the sender has created it and is running
// It is erased when the sender closes.
bool SpoutControls::FindControls(std::string &mapname)
{
	char path[MAX_PATH];

	// Find the if controller map name exists the registry
	if(ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlMap", path) ) {
		if(path[0] > 0) {
			mapname = path;
			return true;
		}
	}

	return false;

}


//---------------------------------------------------------
// Create a controls memory map, existence mutex and access mutex
// and fill the memory map with the control vector
bool SpoutControls::CreateControls(std::string mapname, std::vector<control> controls)
{
	std::string	mutexName;
	std::string sharedMemoryName;

	// Create a mutex to control the write / read
	mutexName = mapname;
	mutexName += "ControlsAccess";
	mutexName += "_mutex";
	m_hAccessMutex = CreateMutexA(NULL, true, mutexName.c_str()); // initial ownership
	if (!m_hAccessMutex) {
		printf("CreateControls : Mutex creation failed\n");
		return false;
	}

	// Create a memory map that will contain the updated control information
	m_sharedMemoryName = mapname; // Global shared memory name used throughout

	sharedMemoryName = mapname;
	sharedMemoryName += "Controls";
	sharedMemoryName += "_map";	

	// Calculate the size required for the memory map
	// First 4 bytes on the first line are the size of the control array
	// Next the information for each control
	// Type DWORD (4 bytes) Name (16 bytes) Data (256 bytes) - total 276 bytes per control
	// Total : 276 + size *(276)
	m_dwSize = 276 + (DWORD)(controls.size()*276);

	// Create or open the shared memory map
	m_hSharedMemory = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_dwSize, (LPCSTR)sharedMemoryName.c_str());
	if (m_hSharedMemory == NULL || m_hSharedMemory == INVALID_HANDLE_VALUE) { 
		printf("CreateControls : error occured while creating file mapping object : %d\n", GetLastError() );
		CloseHandle(m_hAccessMutex);
		return false;
	}


	// Map a view to get a pointer to write to
	m_pBuffer = (LPTSTR)MapViewOfFile(m_hSharedMemory, FILE_MAP_ALL_ACCESS, 0, 0, m_dwSize);
	if (m_pBuffer == NULL) { 
		printf("CreateControls : error occured while mapping view of the file : %d\n", GetLastError() );
		CloseHandle(m_hSharedMemory);
		CloseHandle(m_hAccessMutex);
		return false;
	}

	WriteControls(m_pBuffer, controls);
		
	UnmapViewOfFile(m_pBuffer);

	// End of creation so unlock the access mutex
	// If the reader tries to access the memory there will be nothing in it
	ReleaseMutex(m_hAccessMutex);

	return true;
}


//---------------------------------------------------------
// Set the changed controls to the memory map and inform the sender
bool SpoutControls::SetControls(std::vector<control> controls)
{

	if(UpdateControls(controls)) {
		// Inform the sender that new control data is ready
		WriteMail(m_sharedMemoryName, "Ready");
		return true;
	}

	return false;
}



//---------------------------------------------------------
// Find a control map and update it with the controls
bool SpoutControls::UpdateControls(std::vector<control> controls)
{
	std::string mutexName;
	std::string memoryMapName; // local name of the shared memory
	HANDLE hMemory = NULL; // local handle to shared memory
	LPTSTR pBuf = NULL; // local shared memory pointer
	HANDLE hAccessMutex = NULL;
	DWORD dwWaitResult;
	DWORD dwMapSize = 0;
	char *buf = NULL;
	char temp[256];

	//
	// Controller writes to the memory map to update control data
	//

	// Check the access mutex
	mutexName = m_sharedMemoryName; // mapname;
	mutexName += "ControlsAccess";
	mutexName += "_mutex";
	hAccessMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, mutexName.c_str());
	if(!hAccessMutex) {
		printf("UpdateControls - access mutex does not exist\n");
		CloseHandle(hAccessMutex);
		return false;
	}

	dwWaitResult = WaitForSingleObject(m_hAccessMutex, 67);
	if (dwWaitResult != WAIT_OBJECT_0) { // reader is accessing it
		CloseHandle(hAccessMutex);
		return false;
	}

	// The mutex is now locked so that the reader does not read while writing

	// The memory map name is defined by the global mapname
	memoryMapName = m_sharedMemoryName; // local name;
	memoryMapName += "Controls";
	memoryMapName += "_map";	

	// ====================================
	// At this stage the map size is not known, but it's size 
	// is the first 4 bytes of the map, so read that first to get the size
	hMemory = CreateFileMappingA ( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, (LPCSTR)memoryMapName.c_str());
	if (hMemory == NULL || hMemory == INVALID_HANDLE_VALUE) { 
		printf("UpdateControls : Error occured while opening file mapping object : %d\n", GetLastError() );
		CloseHandle(hAccessMutex);
		return false;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 4); // only 4 bytes to read
	if (pBuf == NULL || pBuf[0] == 0) { 
		printf("UpdateControls : Error 1 occured while mapping view of the file : %d\n", GetLastError() );
		if(pBuf) UnmapViewOfFile(pBuf);
		CloseHandle(hMemory);
		CloseHandle(hAccessMutex);
		return false;
	}

	// Retrieve the map size - the first 4 bytes
	buf = (char *)pBuf; // moveable pointer
	for(int i = 0; i<4; i++)
		temp[i] = *buf++;
	temp[4] = 0;
	dwMapSize = (DWORD)atoi(temp);

	// Now close the map and re-open it with the known size
	UnmapViewOfFile(pBuf);
	CloseHandle(hMemory);

	// First check that the map size is correct for the control vector passed
	// First 4 bytes on the first line are the size of the control array
	// Next the information for each control
	// Type DWORD (4 bytes) Name (16 bytes) Data (256 bytes) - total 276 bytes per control
	// Total : 276 + size *(276)
	if(dwMapSize != (276 + (DWORD)(controls.size()*276)) ) {
		CloseHandle(hAccessMutex);
		return false;
	}

	// Now open the full map and write the control data to it
	hMemory = CreateFileMappingA ( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwMapSize, (LPCSTR)memoryMapName.c_str());
	if (hMemory == NULL || hMemory == INVALID_HANDLE_VALUE) { 
		CloseHandle(hAccessMutex);
		return false;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, dwMapSize);
	if (pBuf == NULL) { 
		printf("UpdateControls : Error 2 occured while mapping view of the file : %d\n", GetLastError() );
		CloseHandle(hMemory);
		CloseHandle(hAccessMutex);
		return false;
	}

	//
	// Write the controls to the memory map
	//
	WriteControls(pBuf, controls);

	UnmapViewOfFile(pBuf); // Finished with the buffer
	CloseHandle(hMemory); // Closes the memory map
	ReleaseMutex(hAccessMutex); // Unlock the access mutex so that the reader can read from the memory map
	CloseHandle(hAccessMutex);// Close the local handle

	return true;
}


//---------------------------------------------------------
// Get controls from an existing map
bool SpoutControls::GetControls(std::vector<control> &controls)
{
	std::string mutexName;
	std::string memoryMapName; // local name of the shared memory
	HANDLE hMemory = NULL; // local handle to shared memory
	LPTSTR pBuf = NULL; // local shared memory pointer
	HANDLE hAccessMutex = NULL;
	DWORD dwWaitResult;
	DWORD dwMapSize = 0;
	// int nControls = 0;
	// int ControlType = 0;
	// float ControlValue = 0; // Float value of a control
	std::string ControlText; // Text data of a control
	char *buf = NULL;
	char temp[256];

	//
	// Reader reads the memory map to retrieve control data
	//

	// Check the access mutex
	mutexName = m_sharedMemoryName; // mapname;
	mutexName += "ControlsAccess";
	mutexName += "_mutex";
	hAccessMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, mutexName.c_str());
	if(!hAccessMutex) {
		printf("GetControls : No access mutex\n");
		CloseHandle(hAccessMutex);
		return false;
	}

	dwWaitResult = WaitForSingleObject(hAccessMutex, 67);
	if (dwWaitResult != WAIT_OBJECT_0) { // writer is accessing it
		CloseHandle(hAccessMutex);
		return false;
	}
	
	// The mutex is now locked so that the writer does not write while reading

	// The memory map name is defined by the global mapname
	memoryMapName = m_sharedMemoryName;
	memoryMapName += "Controls";
	memoryMapName += "_map";

	// ====================================
	// At this stage the map size is not known, but it's size is the
	// first 4 bytes of the map so read that first to get the size
	hMemory = CreateFileMappingA ( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, (LPCSTR)memoryMapName.c_str());
	if (hMemory == NULL || hMemory == INVALID_HANDLE_VALUE) { 
		printf("GetControls - Error occured opening file mapping object : %d\n", GetLastError() );
		ReleaseMutex(hAccessMutex);
		CloseHandle(hAccessMutex);
		return false;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 4); // only 4 bytes to read
	// Did the mapping fail or is there nothing in the map
	if (pBuf == NULL || pBuf[0] == 0) { 
		printf("GetControls - Error 1 occured while mapping view of the file : %d\n", GetLastError() );
		if(pBuf) UnmapViewOfFile(pBuf);
		CloseHandle(hMemory);
		ReleaseMutex(hAccessMutex);
		CloseHandle(hAccessMutex);
		return false;
	}

	// Retrieve the map size - the first 4 bytes
	buf = (char *)pBuf; // moveable pointer
	for(int i = 0; i<4; i++)
		temp[i] = *buf++;
	temp[4] = 0;
	dwMapSize = (DWORD)atoi(temp);

	// Now close the map and re-open it with the known size
	UnmapViewOfFile(pBuf);
	CloseHandle(hMemory);

	hMemory = CreateFileMappingA ( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwMapSize, (LPCSTR)memoryMapName.c_str());
	if (hMemory == NULL || hMemory == INVALID_HANDLE_VALUE) { 
		printf("GetControls - CreateFileMapping failed\n");
		ReleaseMutex(hAccessMutex);
		CloseHandle(hAccessMutex);
		return false;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, dwMapSize);
	if (pBuf == NULL) { 
		printf("GetControls - Error 2 occured while mapping view of the file : %d\n", GetLastError() );
		CloseHandle(hMemory);
		ReleaseMutex(hAccessMutex);
		CloseHandle(hAccessMutex);
		return false;
	}

	//
	// Get the controls from the memory map
	//
	ReadControls(pBuf, controls);

	// Finished with the buffer
	UnmapViewOfFile(pBuf);

	// Reader closes the memory map
	CloseHandle(hMemory);

	// Unlock the access mutex so that the writer can write to the memory map
	ReleaseMutex(hAccessMutex);
	
	// Close the local mutex handle
	// All handles must be closed before the writer finally closes with the creation handle
	CloseHandle(hAccessMutex);
	
	return true;
}


//---------------------------------------------------------
// OpenControls
// A sender creates the controls and then calls OpenControls with a control name
// so that the controller can set up a memory map ans share data with the sender as it changes the controls
// The sender provides the map name, creates the mailslot and writes the map name to the registry
bool SpoutControls::OpenControls(std::string mapname)
{
	char Path[MAX_PATH];
	HRESULT hr;
	Path[0] = 0;

	// 13.12.17 - return if no controls
	// TODO remove size check below
	// printf("SpoutControls::OpenControls(%s) - controls size = %d\n", mapname.c_str(), filecontrols.size());
	if (filecontrols.size() == 0) {
		// printf("No controls\n");
		return false;
	}

	// Set the global map name
	m_sharedMemoryName = mapname;

	// Create the mailslot
	// m_hSlot is intitialized to NULL in constructor
	CreateMail(m_sharedMemoryName, m_hSlot);
	// printf("Created mailslot handle [%x]\n", m_hSlot);

	// Write the executable path to the registry
	GetModuleFileNameA(NULL, Path, sizeof(Path));
	// printf("ControlPath(%s)\n", Path);
	if (WritePathToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlPath", Path)) {
		// printf("registry write OK\n");
	}

	// Write the name path to the registry to identify the memory map
	strcpy_s(Path, MAX_PATH, m_sharedMemoryName.c_str());
	PathStripPathA(Path);
	PathRemoveExtensionA(Path);
	// printf("ControlMap(%s)\n", Path);
	if (WritePathToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlMap", Path)) {
		// printf("registry write 2 OK\n");
	}

	// If the user has created controls, find the programdata or executable path and write the control file
	m_ControlFilePath[0] = 0;
	if(filecontrols.size() > 0) {
		// Find or create the Spout programdata folder
		// Look for "ProgramData" if it exists
		hr = SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, Path);
		if (SUCCEEDED(hr)) {
			sprintf_s(m_ControlFilePath, MAX_PATH, "%s\\Spout", Path);
			CreateDirectoryA((LPCSTR)m_ControlFilePath, NULL); // Will create if it does not exist
			// RemoveDirectoryA((LPCSTR)SpoutPath);
			strcat_s(m_ControlFilePath, MAX_PATH, "\\");
			strcat_s(m_ControlFilePath, mapname.c_str());
			strcat_s(m_ControlFilePath, ".txt"); // The control file name
		}

		// If that failed use the executable path
		if(!m_ControlFilePath[0]) {
			GetModuleFileNameA(NULL, Path, sizeof(Path));
			PathRemoveFileSpecA(Path);
			strcat_s(Path, MAX_PATH, "\\");
			strcat_s(Path, mapname.c_str());
			strcat_s(Path, ".txt"); // The control file name
			strcpy_s(m_ControlFilePath, MAX_PATH, Path);
		}

		CreateControlFile(m_ControlFilePath);
		// Write the control file path to the registry
		// printf("writing control file [%s]\n", m_ControlFilePath);
		WritePathToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlFile", m_ControlFilePath);

		return true;

	}

	return false;
}




//---------------------------------------------------------
bool SpoutControls::CloseControls()
{
	// Release all handles etc
	Cleanup();
	return true;
}


//---------------------------------------------------------
// Open the SpoutController executable to allow controls to be changed
// Requires SpoutController.exe to be in the path passed
// or in the folder of a host executable
// or a SpoutControls installation
bool SpoutControls::OpenSpoutController(std::string CommandLine)
{
	char Path[MAX_PATH];
	char InstallPath[MAX_PATH];
	// HRESULT hr;
	Path[0] = 0;
	InstallPath[0] = 0;
	
	// Look for the SpoutControls installation path in the registry
	// This contains the full path including "SpoutController.exe"
	if(ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutControls", "InstallPath", InstallPath)) {
		strcpy_s(Path, MAX_PATH, InstallPath);
	}
	else if(!CommandLine.empty()) {
		// A path been passed - add the executable name
		strcpy_s(Path, MAX_PATH, CommandLine.c_str());
		strcat_s(Path, MAX_PATH, "\\SpoutController.exe");
	}
	else {
		// No path can be found
		return false;
	}

	if(PathFileExistsA(Path) ) {
		// Launch SpoutController
		SHELLEXECUTEINFOA ShExecInfo;
		ZeroMemory(&ShExecInfo, sizeof(ShExecInfo));
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = (LPCSTR)Path;
		// If opening the controller from an application, give it an argument
		// "NoClose" to remove the Close button 
		// WinSpoutController is now changed to show/hide or exit on Escape key
		if(!CommandLine.empty() && CommandLine.c_str()[0] != 0) {
			if(InstallPath[0] == 0) {
				// No SpoutContols installation, so use the path passed
				ShExecInfo.lpParameters = (LPCSTR)CommandLine.c_str(); // "";
			}
			else {
				// SpoutContols installation as well as a command line, so use "NOCLOSE"
				strcpy_s(InstallPath, MAX_PATH, "NOCLOSE");
				ShExecInfo.lpParameters = InstallPath;
			}
		}
		else {
			ShExecInfo.lpParameters = "";
		}
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;	
		ShellExecuteExA(&ShExecInfo);
		Sleep(125); // alow time to open
		return true;
	}

	return false;
}


//---------------------------------------------------------
// Check the mailslot and return all controls
bool SpoutControls::CheckControls(std::vector<control> &controls)
{

	// if intialized already, check for a message from the controller
	if(CheckMail(m_sharedMemoryName, m_hSlot)) {
		// Get the new controls
		GetControls(controls);
		return true;
	}

	return false;

}



// ======================================================================
//				Sender functions to create a control file
// ======================================================================
bool SpoutControls::CreateControl(std::string name, std::string type)
{
	return CreateFileControl(name, type, 0.0f, 1.0f, 1.0, "");
}

bool SpoutControls::CreateControl(std::string name, std::string type, float value)
{
	return CreateFileControl(name, type, 0.0f, value, value, ""); // Min. max, default
}

bool SpoutControls::CreateControl(std::string name, std::string type, std::string text)
{
	return CreateFileControl(name, type, 0.0f, 1.0f, 1.0, text);
}

bool SpoutControls::CreateControl(std::string name, std::string type, float minimum, float maximum, float value)
{
	return CreateFileControl(name, type, minimum, maximum, value, "");
}


//---------------------------------------------------------
// Used by the controller to find the control file path
// and find the control map name in the registry
bool SpoutControls::FindControlFile(std::string &filepath)
{
	char path[MAX_PATH];

	// Find if the controller map file name exists the registry
	if(ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlFile", path) ) {
		if(path[0] > 0) {
			 
			// 24.08.15 - add existence check
			if(_access(path, 0) == -1) // Mode 0 - existence check
				return false;
			
			// File found
			filepath = path;
			return true;

		}
	}

	return false;

}

//---------------------------------------------------------
// Used by a multiple controlled senders to copy the control
// file from ProgramData to the local dll directory
bool SpoutControls::CopyControlFile (std::string &filepath, std::string &destpath)
{
	// copy from source to destination
	if(CopyFileA((LPCSTR)filepath.c_str(), (LPCSTR)destpath.c_str(), false)) {
		return true;
	}
	return false;
}

bool SpoutControls::GetFileControls(std::vector<filecontrol> &controls)
{
	if (filecontrols.size() > 0) {
		controls.clear();
		for (unsigned int i = 0; i < filecontrols.size(); i++) {
			controls.push_back(filecontrols.at(i));
		}
		return true;
	}
	return false;
}


// ======================================================================
//								Private
// ======================================================================


//---------------------------------------------------------
// Create a file control and add to the sender's filecontrol vector
bool SpoutControls::CreateFileControl(std::string name, std::string type, float minimum, float maximum, float value, std::string text)
{
	filecontrol fc;

	if(name.empty() || type.empty()) {
		return false;
	}

	fc.name = name;
	fc.desc = type;
	fc.min = minimum;
	fc.max = maximum;
	fc.def = value;
	fc.value = value;
	fc.text = text.c_str();

	if(type == "bool")
		fc.fftype = 0; // checkbox
	if(type == "event")
		fc.fftype = 1; // button
	if(type == "float")
		fc.fftype = 10; // float slider
	if(type == "text")
		fc.fftype = 100; // text

	filecontrols.push_back(fc);

	return true;
}


//---------------------------------------------------------
// Create a JSON control file from the filecontrols vector
bool SpoutControls::CreateControlFile(const char *filepath)
{
	std::string linestring;
	char path[MAX_PATH];
	char temp[256];

	if(!filepath[0] || filecontrols.empty())
		return false;

	strcpy_s(path, MAX_PATH, filepath);

	//
	// Write the contents of the parameter list
	//
	// Example
	//

	/*{
		"CREDIT": "by Lynn Jarvis - spout.zeal.co",
		"DESCRIPTION": "SpoutControls",
		"CATEGORIES": [
			"Parameter adjustment"
		],
			"INPUTS": [
			{
				"NAME": "User text",
				"TYPE": "text",
				"DEFAULT": 0
			},
			{
				"NAME": "Rotate",
				"TYPE": "bool",
				"DEFAULT": 1
			},
			{
				"NAME": "Speed",
				"TYPE": "float",
				"MIN": 0.0,
				"MAX": 4.0,
				"DEFAULT": 0.5
			},
		]
	}*/

	// Create the file
	std::ofstream sourceFile(path);

	// Source file created OK ?
	if(sourceFile.is_open()) {
		
		// Create the JSON header
		linestring = "/*{\n";
		sourceFile.write(linestring.c_str(), linestring.length());

		sprintf_s(temp, 256, "	\"CREDIT\": \"SpoutControls - spout.zeal.co\",\n");
		sourceFile.write(temp, strlen(temp));

		PathStripPathA(path);
		PathRemoveExtensionA(path);
		sprintf_s(temp, 256, "	\"DESCRIPTION\": \"%s\",\n", path);
		sourceFile.write(temp, strlen(temp));

		sprintf_s(temp, 256, "	\"CATEGORIES\": [\n");
		sourceFile.write(temp, strlen(temp));
		sprintf_s(temp, 256, "	\"Parameter adjustment\"\n");
		sourceFile.write(temp, strlen(temp));
		sprintf_s(temp, 256, "	],\n");
		sourceFile.write(temp, strlen(temp));
		sprintf_s(temp, 256, "	\"INPUTS\": [\n");
		sourceFile.write(temp, strlen(temp));

		// Write the control data here
		// Example
		// {
		// 	"NAME": "Speed",
		// 	"TYPE": "float",
		// 	"MIN": 0.0,
		// 	"MAX": 4.0,
		//	"DEFAULT": 0.5
		// },
		for(unsigned int i = 0; i < filecontrols.size(); i++) {
			sprintf_s(temp, 256, "		{\n");
			sourceFile.write(temp, strlen(temp));
			sprintf_s(temp, 256, "			\"NAME\": \"%s\",\n", filecontrols.at(i).name.c_str());
			sourceFile.write(temp, strlen(temp));
			sprintf_s(temp, 256, "			\"TYPE\": \"%s\",\n", filecontrols.at(i).desc.c_str());
			sourceFile.write(temp, strlen(temp));
			if(filecontrols.at(i).desc != "text") {
				sprintf_s(temp, 256, "			\"MIN\": %.2f,\n", filecontrols.at(i).min);
				sourceFile.write(temp, strlen(temp));
				sprintf_s(temp, 256, "			\"MAX\": %.2f,\n", filecontrols.at(i).max);
				sourceFile.write(temp, strlen(temp));
				sprintf_s(temp, 256, "			\"DEFAULT\": %.2f\n", filecontrols.at(i).def);
				sourceFile.write(temp, strlen(temp));
			}
			else {
				sprintf_s(temp, 256, "			\"TEXT\": \"%s\"\n", filecontrols.at(i).text.c_str());
				sourceFile.write(temp, strlen(temp));
			}
			sprintf_s(temp, 256, "		},\n");
			sourceFile.write(temp, strlen(temp));
		}

		sprintf_s(temp, 256, "	],\n");
		sourceFile.write(temp, strlen(temp));
		sprintf_s(temp, 256, "}*/\n");
		sourceFile.write(temp, strlen(temp));

		sourceFile.close();

		// printf("Control file created\n[%s]\n", filepath);
	}
	else {
		sprintf_s(temp, 256, "Control file not created\n[%s]", path);
		MessageBoxA(NULL, temp, "Info", MB_OK);
		// MessageBoxA(NULL, "File not created", "Info", MB_OK);
	}

	return true;
}


//---------------------------------------------------------
// Write control map with updated values
bool SpoutControls::WriteControls(void *pBuffer, std::vector<control> controls)
{
	//
	//		Write the control data to shared memory
	//		The writer knows the memory map size to open it (m_dwSize)
	//
	float fValue = 0;
	char desc[256];
	char *buffer = NULL; // the buffer to store in shared memory
	char *buf = NULL; // pointer within the buffer
	int i, j;

	buffer = (char *)malloc(m_dwSize*sizeof(unsigned char));
	if(!buffer) return false;

	// Clear the buffer to zero so that there is a null for each data line
	ZeroMemory(buffer, m_dwSize*sizeof(unsigned char));
	buf = buffer; // pointer within the buffer

	// The first 4 bytes of the first line is the memory map size so the reader knows how big it is
	// printf("Writing the memory map size (%d)\n", m_dwSize);
	sprintf_s(desc, 256, "%4d", m_dwSize);
	for(i = 0 ; i< 4; i++) 
		*buf++ = desc[i];

	// The next 4 bytes contains the number of controls
	ZeroMemory(desc, 256);
	sprintf_s(desc, 256, "%4d", (DWORD)controls.size());
	for(i = 0 ; i< 4; i++) 
		*buf++ = desc[i];

	// Move on 268 bytes to the start of the controls (each memory map line is 276 bytes)
	buf += 268;

	// Next the controls and their data
	// Name (16 bytes) Type (4 bytes) Data (256 bytes) - total 276 bytes per control
	for(i = 0; i < (int)controls.size(); i++) {

		// Control name - 16 bytes
		ZeroMemory(desc, 256);
		if(controls.at(i).name.c_str()[0])
			sprintf_s(desc, 256, "%s", controls.at(i).name.c_str());
		for(j = 0 ; j<16; j++)
			*buf++ = desc[j];

		// Control type - 4 bytes
		ZeroMemory(desc, 256);
		sprintf_s(desc, 256, "%4d", (DWORD)controls.at(i).type);
		for(j = 0 ; j<4; j++)
			*buf++ = desc[j];

		// Control data - 256 bytes
		ZeroMemory(desc, 256);
		if(controls.at(i).type == 100) { // Text data
			if(controls.at(i).text[0]) {
				sprintf_s(desc, 256, "%s", controls.at(i).text.c_str());
			}
		}
		else {
			fValue = controls.at(i).value;
			sprintf_s(desc, 256, "%f", fValue); // float data
		}


		// copy it with 256 length
		for(j = 0 ; j<256; j++)	
			*buf++ = desc[j];
	
	} // end all controls

	// Now transfer to shared memory which will be the same size
	memcpy( (void *)pBuffer, (void *)buffer, m_dwSize );

	free((void *)buffer);

	return true;
}


//---------------------------------------------------------
// Read controls from the memory map
bool SpoutControls::ReadControls(void *pBuffer, std::vector<control> &controls)
{
	char *buf = NULL;
	char temp[256];
	int i, j, nControls;
	control control;

	//
	// Get the controls
	//
	buf = (char *)pBuffer; // moveable pointer
	buf += 4; // The first 4 bytes of the first line is the memory map size, so skip that

	// the next 4 bytes contains the number of controls
	for(i = 0; i<4; i++)
		temp[i] = *buf++;
	temp[4] = 0;
	nControls = atoi(temp);

	// Move on 268 bytes to the control data (each line is 276 bytes)
	buf += 268;

	// Clear the controls vector
	controls.clear();

	// Fill it again (use previously found size)
	for(i = 0; i<nControls; i++) {

		control.name.clear();
		control.text.clear();
		control.value = 0;
		control.type = 0;

		// First 16 bytes are the control name
		ZeroMemory(temp, 256);
		for(j = 0; j<16; j++)
			temp[j] = *buf++;
		temp[16] = 0;
		if(temp[0])
			control.name = temp;
													
		// Control type
		ZeroMemory(temp, 256);
		for(j = 0; j<4; j++)
			temp[j] = *buf++;
		temp[4] = 0;
		control.type = atoi(temp);

		// Next 256 bytes on the same line are allocated to the float or string data
		ZeroMemory(temp, 256);
		for(j = 0; j<256; j++)
			temp[j] = *buf++;

		if(control.type == 100) { // text data
			if(temp[0]) {
				control.text = temp;
			}
		}
		else { // float data
			control.value = (float)atof(temp);
		}
		
		controls.push_back(control);

	} // Done all controls

	return true;
}


//---------------------------------------------------------
// Sender creates the mailslot
bool SpoutControls::CreateMail(std::string SlotName, HANDLE &hSlot)
{
	HANDLE hslot = NULL;
	std::string slotstring;

	// printf("SpoutControls::CreateMail\n");

	// Already exists ?
	if (hSlot != NULL) {
		// printf("SpoutControls::CreateMail - hSlot exists\n");
		return true;
	}

	/*
	// LJ DEBUG - check with CreateFile
	// An error occurs if a mailslot with the specified name already exists.
	// If CreateFile is called to access a non-existent mailslot, the ERROR_FILE_NOT_FOUND error code will be set
	
	// If the function succeeds, the return value is an open handle to 
	// the specified file, device, named pipe, or mail slot.
	// If the function fails, the return value is INVALID_HANDLE_VALUE.
	// To get extended error information, call

	HANDLE hFile;
	hFile = CreateFileA(slotstring.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	// DWORD dwError = GetLastError();
	if (hFile != INVALID_HANDLE_VALUE) {
		// The system found the file specified
		printf("SpoutControls::CreateMail - CreateFileA already exists\n"); 
		CloseHandle(hFile);
		return true;
	}

	printf("SpoutControls::CreateMail - CreateFileA returned %x\n", hFile);
	CloseHandle(hFile);
	*/

	slotstring = "\\\\.\\mailslot\\";
	slotstring += SlotName;
    hslot = CreateMailslotA(slotstring.c_str(), 
							0, // no maximum message size 
							0, // Time-out for operations - set to zero - was originally 2msec
							(LPSECURITY_ATTRIBUTES) NULL); // default security

	if (hslot == INVALID_HANDLE_VALUE)  { // (-1)
		// Want no delay here
		// DWORD dwError = GetLastError();
		// printf("CreateMailslot (%s) failed with %d\n", SlotName.c_str(), dwError);
		// Now should not happen
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			// printf("CreateMailslot (%s) already exists (%x)(%x)\n", SlotName.c_str(), (unsigned int)hSlot, (unsigned int)m_hSlot);
			return true;
		}
        return false;
	}

	hSlot = hslot;
	// printf("Created mailslot handle [%x]\n", hSlot);

	return true;
}


//---------------------------------------------------------
// Controller writes to the mailslot
bool SpoutControls::WriteMail(std::string SlotName, std::string SlotMessage)
{
	std::string slotstring;
	HANDLE hFile; 
	BOOL fResult; 
	DWORD dwWritten; 

	slotstring = "\\\\.\\mailslot\\";
	slotstring += SlotName;
	
	hFile = CreateFileA(slotstring.c_str(),
						GENERIC_WRITE,
						FILE_SHARE_READ,
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						(HANDLE)NULL); 
 
	if (hFile == INVALID_HANDLE_VALUE) { 
		// The system cannot find the file specified
		// printf("CreateFile failed with %d.\n", GetLastError()); 
		return false; 
   } 

	fResult = WriteFile(hFile, 
						SlotMessage.c_str(),
						(DWORD) (strlen(SlotMessage.c_str())+1)*sizeof(TCHAR),  
						&dwWritten, 
						(LPOVERLAPPED)NULL); 
	if (!fResult) { 
		// printf("WriteFile failed with error (%d)\n", GetLastError()); 
		CloseHandle(hFile);
		return false; 
	} 
 
	CloseHandle(hFile);

	return true;
}


//---------------------------------------------------------
// Sender checks the mailslot for messages
bool SpoutControls::CheckMail(std::string SlotName, HANDLE hSlot)
{
	if(hSlot == NULL)
		return false;

	std::string slotmessage; // Not used but could return the message

	return ReadMail(SlotName, hSlot, slotmessage);

}


//---------------------------------------------------------
// Sender reads the mailslot to determine whether
// a message is ready and clears pending messages
bool SpoutControls::ReadMail(std::string SlotName, HANDLE hSlot, std::string &SlotMessage)
{
	DWORD cbMessage, cMessage, cbRead; 
	BOOL fResult; 
	LPTSTR lpszBuffer; 
	char achID[80];
	DWORD cAllMessages; 
	HANDLE hEvent;
	OVERLAPPED ov;
 
	if(hSlot == NULL)
		return false;

	cbMessage = cMessage = cbRead = 0; 

	hEvent = CreateEvent(NULL, false, false, TEXT("SpoutControlsSlotEvent"));
    if( NULL == hEvent ) {
		return false;
	}
	
	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.hEvent = hEvent;
 
	fResult = GetMailslotInfo(	hSlot, // mailslot handle 
								(LPDWORD) NULL,               // no maximum message size 
								&cbMessage,                   // size of next message 
								&cMessage,                    // number of messages 
								(LPDWORD)NULL);               // no read time-out. TODO : 1/2 frame timeout ?
 
	if (!fResult) { 
		// printf("GetMailslotInfo failed with %d.\n", GetLastError()); 
		CloseHandle(hEvent);
        return false; 
	} 
 
	if (cbMessage == MAILSLOT_NO_MESSAGE) { 
		CloseHandle(hEvent);
		return false; 
	} 

	cAllMessages = cMessage; 

	while (cMessage != 0) { // retrieve all messages

		// Allocate memory for the message. 
		// TODO - clean up
		lpszBuffer = (LPTSTR) GlobalAlloc(GPTR, lstrlen((LPTSTR)achID)*sizeof(TCHAR) + cbMessage); 
		if( NULL == lpszBuffer ) return false;
		lpszBuffer[0] = '\0'; 
 
		fResult = ReadFile(	hSlot, 
							lpszBuffer, 
							cbMessage, 
							&cbRead, 
							&ov); 
 
		if (!fResult) { 
			// printf("ReadFile failed with %d.\n", GetLastError()); 
			GlobalFree((HGLOBAL) lpszBuffer);
			CloseHandle(hEvent);
			return false; 
		} 
 
		fResult = GetMailslotInfo(	hSlot,           // mailslot handle 
									(LPDWORD) NULL,  // no maximum message size 
									&cbMessage,      // size of next message 
									&cMessage,       // number of messages 
									(LPDWORD) NULL); // no read time-out 
 
		if (!fResult) { 
			// printf("GetMailslotInfo failed (%d)\n", GetLastError());
			GlobalFree((HGLOBAL) lpszBuffer); 
			CloseHandle(hEvent);
			return false; 
		} 

		// We only return the last message here but it could be extended to a vector of strings
		SlotMessage = (LPSTR)lpszBuffer;
 
		GlobalFree((HGLOBAL) lpszBuffer); 

	} // end all messages

	CloseHandle(hEvent);

	return true;
}


//---------------------------------------------------------
void SpoutControls::CloseMap()
{
	if(m_pBuffer) UnmapViewOfFile(m_pBuffer);
	if(m_hSharedMemory) CloseHandle(m_hSharedMemory);
	if(m_hAccessMutex) CloseHandle(m_hAccessMutex);
	m_pBuffer = NULL;
	m_hSharedMemory = NULL;
	m_hAccessMutex = NULL;

}


//---------------------------------------------------------
bool SpoutControls::Cleanup()
{
	// Cleanup for this class

	// Release the filecontrols vector created by a sender
	if(filecontrols.size() > 0) {
		filecontrols.clear();
	}

	// Release all objects
	if(m_pBuffer) UnmapViewOfFile(m_pBuffer);
	if(m_hSharedMemory) CloseHandle(m_hSharedMemory);
	if(m_hAccessMutex) CloseHandle(m_hAccessMutex);
	m_pBuffer = NULL;
	m_hSharedMemory = NULL;
	m_hAccessMutex = NULL;
	
	if(m_hSlot) {
		// printf("Closing mailslot handle\n");
		// For a sender, clear the map name from the registry
		RemovePathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "ControlMap");
		CloseHandle(m_hSlot);
		m_hSlot = NULL;
	}

	// Close the controller if it has been opened
	HWND hwnd = FindWindowA(NULL, (LPCSTR)"Spout Controller");
	if(hwnd) {
		// ShowWindow(hwnd, SW_SHOWNORMAL); // causes a freeze if the window is hidden
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		SendMessageA(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
	}

	return true;
}

