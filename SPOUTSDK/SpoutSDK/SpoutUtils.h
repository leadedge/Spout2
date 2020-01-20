/*

						SpoutUtils.h

					General utility functions

		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		Copyright (c) 2017-2020, Lynn Jarvis. All rights reserved.

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
#ifndef __spoutUtils__ // standard way as well
#define __spoutUtils__

#include "SpoutCommon.h"
#include <windows.h>
#include <stdio.h> // for console
#include <iostream> // std::cout, std::end
#include <fstream> // for log file
#include <time.h> // for time and date
#include <io.h> // for _access
#include <vector>
#include <string>
#include <Shellapi.h> // for shellexecute
#include <shlwapi.h> // for path functions
#include <shlobj.h> // to find the AppData folder

#pragma comment(lib, "Shell32.lib") // for shellexecute
#pragma comment(lib, "shlwapi.lib") // for path functions
#pragma comment(lib, "Advapi32.lib") // for registry functions

namespace spoututils {

	// Log level definitions
	enum SpoutLogLevel {
		SPOUT_LOG_SILENT,
		SPOUT_LOG_VERBOSE,
		SPOUT_LOG_NOTICE,
		SPOUT_LOG_WARNING,
		SPOUT_LOG_ERROR,
		SPOUT_LOG_FATAL,
		SPOUT_LOG_NONE
	};

	// Console management
	void OpenSpoutConsole();
	void CloseSpoutConsole(bool bWarning = false);

	// Enable logging to the console
	void EnableSpoutLog();

	// Enable logging to a file with optional append
	void EnableSpoutLogFile(const char* filename, bool append = false);

	// Disable logging
	void DisableSpoutLog();

	// Return the log file as a string
	std::string GetSpoutLog();
	
	// Show the log file folder in Windows Explorer
	void ShowSpoutLogs();

	// Set the current log level
	// SPOUT_LOG_SILENT  - Disable all messages
	// SPOUT_LOG_VERBOSE - Show all messages
	// SPOUT_LOG_NOTICE  - Show information messages - default
	// SPOUT_LOG_WARNING - Something might go wrong
	// SPOUT_LOG_ERROR   - Something did go wrong
	// SPOUT_LOG_FATAL   - Something bad happened
	// SPOUT_LOG_NONE    - Ignore log levels
	void SetSpoutLogLevel(SpoutLogLevel level);

	// Logs
	void SpoutLog(const char* format, ...);
	void SpoutLogVerbose(const char* format, ...);
	void SpoutLogNotice(const char* format, ...);
	void SpoutLogWarning(const char* format, ...);
	void SpoutLogError(const char* format, ...);
	void SpoutLogFatal(const char* format, ...);
	void _doLog(SpoutLogLevel level, const char* format, va_list args);

	// SpoutPanel Messagebox with optional timeout
	int SpoutMessageBox(const char * message, DWORD dwMilliseconds = 0);

	// SpoutPanel Messagebox with standard arguments
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0);

	// Registry utility functions
	bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue);
	bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue);
	bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath);
	bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath);
	bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename);
	bool RemoveSubKey(HKEY hKey, const char *subkey);
	bool FindSubKey(HKEY hKey, const char *subkey);

}

#endif
