//
//	SpoutLibraryLoader.h
//
//	Dynamic load of SpoutLibrary.dll
//
//	Use this if SpoutLibrary.lib is not compatible with your compiler,
//  or to allow user update of SpoutLibrary.dll if the disributed library
//	build has been updated with SDK updates but the functions have not changed.
//

/*
		Copyright (c) 2021, Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once

#ifndef __SpoutLibraryLoader__
#define __SpoutLibraryLoader__

#include "SpoutLibrary.h"
#include <io.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib") // for path functions

struct SpoutLibraryLoader
{

public:
	
	SpoutLibraryLoader() {};
	~SpoutLibraryLoader() {};

	//
	// Load SpoutLibrary.dll dynamically
	//
	// The dll file must be in the same folder as the executable.
	// The dll type (Win32 or x64) must match the executable.
	//
	SPOUTLIBRARY * LoadSpoutLibrary()
	{
		//
		// To avoid anti-virus false positive detections, SpoutLibrary.dll is
		// distributed compiled as "Multi-threaded DLL (/MD)" and requires the 
		// Visual Studio runtime dlls to be installed on the system.
		// Visual C++ 2015, 2017 and 2019 all share the same redistributable files.
		//
		// https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0
		//
		// If you build the library yourself, you can use "Multi-threaded (/MT)"
		// to embed the runtime in the dll. Then you can remove the following check.
		//

		// Check for VS2017 runtime.
		// For 64-bit VC++ Redistributable
		//   HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64
		// For 32 - bit VC++ Redistributable
		//   HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86
		HKEY hRegKey = NULL;
		LONG regres = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes", NULL, KEY_READ, &hRegKey);
		if (regres != ERROR_SUCCESS) {
			MessageBoxA(NULL, "Could not find VS2017 runtimes\nInstall from the VS2015-2019 folder\nof the Spout distribution.", "Error", MB_OK);
			return nullptr;
		}

		//
		// Look in the executable folder for the dll file.
		//

		// Get the executable path.
		char filepath[MAX_PATH];
		if (GetModuleFileNameA(NULL, filepath, sizeof(filepath)) == 0)
			return nullptr;

		// Remove executable file name and replace with the dll file name
		PathRemoveFileSpecA(filepath);
		strcat_s(filepath, MAX_PATH, "\\SpoutLibrary.dll");

		// Now we have the full path of the dll file.
		// Does the file exist ?
		if (_access(filepath, 0) == -1)
			return nullptr;

		// Get a handle to the dll module
		hSpoutLib = LoadLibraryA(filepath);
		if (hSpoutLib == NULL)
			return nullptr;

		// Try to get the address of the function that creates an instance of SpoutLibrary.
		// Credit to Newtek NDI (https://www.ndi.tv/) dynamic load example for this code.
		SPOUTLIBRARY * (*GetSpout)(void) = nullptr;
		*((FARPROC*)&GetSpout) = GetProcAddress(hSpoutLib, "GetSpout");
		if (GetSpout == NULL)
			return nullptr;

		// Now we can create an instance of the library and access all the functions in it.
		// The dll LoadLibrary handle (hSpoutLib) must be freed on exit using FreeSpoutLibrary()

		return GetSpout();
	}

	void FreeSpoutLibrary()
	{
		if (hSpoutLib)
			FreeLibrary(hSpoutLib);
	}

private:

	HINSTANCE hSpoutLib = nullptr;

};

#endif
