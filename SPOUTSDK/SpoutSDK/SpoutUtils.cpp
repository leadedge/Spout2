/*

			SpoutUtils

			Utility functions

	CREDIT - logging based on Openframeworks ofLog
	https://github.com/openframeworks/openFrameworks/tree/master/libs/openFrameworks/utils

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
	========================

		31.05.15 - started
		01.01.18 - added check for subkey
		17.03.18 - Document SetLogLevel function in header
		16.10.18 - Add SpoutLogNotice, Warning, Error, Fatal
		28.10.18 - Checks for lastlog and fatal messagebox moved from SpoutLog to _doLog
		15.11.18 - Removed delay after SpoutPanel open
		10.12.18 - Add Timeout option to SpoutMessageBox
		12.12.18 - Add SpoutLogFile
		13.12.18 - Add GetSpoutLog and ShowSpoutLogs
		14.12.18 - Clean up
		02.10.19 - Change registry functions including hKey
				   to allow HKLM and changed argument order
				 - Add RemoveSubKey and FindSubKey
		26.01.19 - Corrected Verbose log to show verbose and not notice
		07.04.19 - Add SpoutLog for logging without specifying level
		28.04.19 - Change OpenSpoutConsole() to check for existing console
		19.05.19 - Cleanup
		16.06.19 - Include calling process file name in SpoutMessageBox
		13.10.19 - Corrected EnableSpoutLogFile for a filename without an extension
				   Changed default extension from "txt" to "log"
		27.11.19 - Prevent multiple logs for warnings and errors
		22.12.19 - add pragma in header for registry function lbraries
				   Remove calling process name from SpoutMessageBox

*/
#include "spoutUtils.h"

namespace spoututils {

	// Local variables
	bool bEnableLog = false;
	SpoutLogLevel CurrentLogLevel = SPOUT_LOG_NOTICE;
	FILE* pCout = NULL; // for log to console
	std::ofstream logFile; // for log to file
	std::string logPath = ""; // path for the logfile
	std::string logFileName = ""; // file name for the logfile
	std::string LastSpoutLog = "";
	bool bConsole = false;

	// Local functions
	std::string _getLogPath();
	std::string _levelName(SpoutLogLevel level);
	void _logtofile(bool append = false);

	//
	// Console management
	//

	void OpenSpoutConsole()
	{
		// AllocConsole fails if the process already has a console
		// Is a console associated with the calling process?
		if (GetConsoleWindow()) {
			bConsole = true;
		}
		else {
			if (AllocConsole()) {
				errno_t err = freopen_s(&pCout, "CONOUT$", "w", stdout);
				if (err == 0) {
					SetConsoleTitleA("Spout Log");
					bConsole = true;
				}
				else {
					pCout = NULL;
					bConsole = false;
				}
			}
		}

	}
	
	void CloseSpoutConsole(bool bWarning)
	{
		if(bWarning) {
			if(MessageBoxA(NULL, "Console close - are you sure?", "Spout", MB_YESNO) == IDNO)
				return;
		}

		if (pCout) {
			fclose(pCout);
			FreeConsole();
			pCout = NULL;
			bConsole = false;
		}
	}
		
	//
	// Logs
	//

	// Enable spout logging
	void EnableSpoutLog()
	{
		bEnableLog = true;

		// Console output
		if(!bConsole)
			OpenSpoutConsole();

	}

	// Log to a user file with optional append
	void EnableSpoutLogFile(const char* filename, bool append)
	{
		bEnableLog = true;

		if (!logPath.empty()) {
			if (logFile.is_open())
				logFile.close();
			logPath = "";
		}

		// Set the log file name or path
		if (filename[0]) {

			char fname[MAX_PATH];
			strcpy_s(fname, MAX_PATH, filename);
			PathRemoveBackslashA(fname);
			logPath.clear();

			if (PathIsDirectoryA(fname)) {
				// Path without a filename
				logPath = fname;
				logPath += "\\SpoutLog.log";
			}
			else if (PathIsFileSpecA(fname)) {
				// Filename without a path
				// Add an extension if none supplied
				if (!PathFindExtensionA(fname)[0])
					strcat_s(fname, MAX_PATH, ".log");
				logPath = _getLogPath();
				logPath += "\\";
				logPath += fname;
			}
			else if (PathFindFileNameA(fname)) {
				// Full path with a filename
				// Add an extension if none supplied
				if (!PathFindExtensionA(fname)[0])
					strcat_s(fname, MAX_PATH, ".log");
				logPath = fname;
			}
			// logPath is empty if all options fail
		}
		_logtofile(append);
	}

	// Disable spout logging
	void DisableSpoutLog()
	{
		CloseSpoutConsole();
		if (!logPath.empty()) {
			if (logFile.is_open())
				logFile.close();
			logPath = "";
		}
		bEnableLog = false;
	}
	
	// Return the Spout log file as a string
	std::string GetSpoutLog()
	{
		std::string logString = "";

		if (!logPath.empty()) {
			logFile.open(logPath);
			if (logFile.is_open()) {
				if (_access(logPath.c_str(), 0) != -1) { // does the file exist
					// Open the log file
					std::ifstream logstream(logPath);
					// Source file loaded OK ?
					if (logstream.is_open()) {
						// Get the file text as a single string
						logString.assign((std::istreambuf_iterator< char >(logstream)), std::istreambuf_iterator< char >());
						logString += ""; // ensure a NULL terminator
						logstream.close();
					}
				}
			}
		}

		return logString;
	}
	
	// Show the Spout log file folder in Windows Explorer
	void ShowSpoutLogs()
	{
		SHELLEXECUTEINFOA ShExecInfo;
		char directory[MAX_PATH];

		if (logPath.empty() || _access(logPath.c_str(), 0) == -1) {
			std::string logfilefolder = _getLogPath();
			strcpy_s(directory, MAX_PATH, logfilefolder.c_str());
		}
		else {
			strcpy_s(directory, MAX_PATH, logPath.c_str());
			PathRemoveFileSpecA(directory); // Current log file path
		}

		memset(&ShExecInfo, 0, sizeof(ShExecInfo));
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.lpFile = (LPCSTR)directory;
		ShExecInfo.nShow = SW_SHOW;
		ShellExecuteExA(&ShExecInfo);

	}
	
	// Set the current log level
	void SetSpoutLogLevel(SpoutLogLevel level)
	{
		CurrentLogLevel = level;
	}


	void SpoutLog(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_NONE, format, args);
		va_end(args);
	}

	void SpoutLogVerbose(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_VERBOSE, format, args);
		va_end(args);
	}

	void SpoutLogNotice(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_NOTICE, format, args);
		va_end(args);
	}

	void SpoutLogWarning(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_WARNING, format, args);
		va_end(args);
	}

	void SpoutLogError(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_ERROR, format, args);
		va_end(args);
	}

	void SpoutLogFatal(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		_doLog(SPOUT_LOG_FATAL, format, args);
		va_end(args);
	}
	
	//
	// MessageBox
	//

	// SpoutPanel Messagebox with optional timeout
	int SpoutMessageBox(const char * message, DWORD dwMilliseconds)
	{
		return SpoutMessageBox(NULL, message, "spout", MB_OK, dwMilliseconds);
	}

	// SpoutPanel Messagebox with standard arguments and optional timeout
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds)
	{
		int iRet = 0;
		std::string spoutmessage;
		SHELLEXECUTEINFOA ShExecInfo;
		char UserMessage[512];
		char path[MAX_PATH];

		spoutmessage = message;

		// Find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
		if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {

			// If a timeout has been specified, add the timeout option
			if (dwMilliseconds > 0) {
				spoutmessage += " /TIMEOUT-";
				spoutmessage += std::to_string((unsigned long long)dwMilliseconds);
			}
			// Open SpoutPanel text message
			// SpoutPanel handles the timeout delay
			if (!spoutmessage.empty()) {
				strcpy_s(UserMessage, 512, spoutmessage.c_str());
			}
			else {
				UserMessage[0] = 0; // make sure SpoutPanel does not see an un-initialized string
			}

			ZeroMemory(&ShExecInfo, sizeof(ShExecInfo));
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			ShExecInfo.hwnd = NULL;
			ShExecInfo.lpVerb = NULL;
			ShExecInfo.lpFile = (LPCSTR)path;
			ShExecInfo.lpParameters = UserMessage;
			ShExecInfo.lpDirectory = NULL;
			ShExecInfo.nShow = SW_SHOW;
			ShExecInfo.hInstApp = NULL;
			ShellExecuteExA(&ShExecInfo);
			// Returns straight away here but multiple instances of SpoutPanel
			// are prevented in it's WinMain procedure by the mutex.
		}
		else {
			// Use a standard untimed topmost messagebox
			// iRet = MessageBoxA(hwnd, message, caption, (uType | MB_TOPMOST));
			iRet = MessageBoxA(hwnd, spoutmessage.c_str(), caption, (uType | MB_TOPMOST));
		}

		return iRet;
	}

	//
	// Registry utilities
	//

	//
	// New registry functions for 2.007 including hKey and changed argument order
	//
	bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue)
	{
		HKEY  hRegKey = NULL;
		LONG  regres = 0;
		DWORD dwKey = 0;
		DWORD dwSize = MAX_PATH;

		// 01.01.18
		if (!subkey[0])	return false;

		// Does the key exist
		regres = RegOpenKeyExA(hKey, subkey, NULL, KEY_READ, &hRegKey);
		if (regres == ERROR_SUCCESS) {
			// Read the key DWORD value
			regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)pValue, &dwSize);
			RegCloseKey(hRegKey);
			if (regres == ERROR_SUCCESS)
				return true;
		}

		// Just quit if the key does not exist
		return false;

	}

	bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue)
	{
		HKEY  hRegKey = NULL;
		LONG  regres = 0;
		char  mySubKey[512];

		if (!subkey[0])
			return false;

		// The required key
		strcpy_s(mySubKey, 512, subkey);

		// Does the key already exist ?
		regres = RegOpenKeyExA(hKey, mySubKey, NULL, KEY_ALL_ACCESS, &hRegKey);
		if (regres != ERROR_SUCCESS) {
			// Create a new key
			regres = RegCreateKeyExA(hKey, mySubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
		}

		if (regres == ERROR_SUCCESS && hRegKey != NULL) {
			// Write the DWORD value
			regres = RegSetValueExA(hRegKey, valuename, 0, REG_DWORD, (BYTE*)&dwValue, 4);
			// For immediate read after write - necessary here because the app might set the values 
			// and read the registry straight away and it might not be available yet
			// The key must have been opened with the KEY_QUERY_VALUE access right 
			// (included in KEY_ALL_ACCESS)
			RegFlushKey(hRegKey); // needs an open key
			RegCloseKey(hRegKey); // Done with the key
		}

		if (regres != ERROR_SUCCESS) {
			SpoutLogWarning("WriteDwordToRegistry - could not write to registry");
			return false;
		}

		return true;

	}

	bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath)
	{
		HKEY  hRegKey = NULL;
		LONG  regres = 0;
		DWORD dwKey = 0;
		DWORD dwSize = MAX_PATH;

		if (!subkey[0])
			return false;

		// Does the key exist
		regres = RegOpenKeyExA(hKey, subkey, NULL, KEY_READ, &hRegKey);
		if (regres == ERROR_SUCCESS) {
			// Read the key Filepath value
			regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)filepath, &dwSize);
			RegCloseKey(hRegKey);
			if (regres == ERROR_SUCCESS)
				return true;
		}
		// Quit if the key does not exist
		SpoutLogWarning("ReadPathFromRegistry - could not open subkey [%s] Error (%ld)", subkey, regres);

		return false;
	}


	bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath)
	{
		HKEY  hRegKey = NULL;
		LONG  regres = 0;
		char  mySubKey[512];

		if (!subkey[0]) {
			SpoutLogWarning("WritePathToRegistry - no subkey specified");
			return false;
		}

		// The required key
		strcpy_s(mySubKey, 512, subkey);

		// Does the key already exist ?
		regres = RegOpenKeyExA(hKey, mySubKey, NULL, KEY_ALL_ACCESS, &hRegKey);
		if (regres != ERROR_SUCCESS) {
			// Create a new key
			regres = RegCreateKeyExA(hKey, mySubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
		}

		if (regres == ERROR_SUCCESS && hRegKey != NULL) {
			// Write the path
			regres = RegSetValueExA(hRegKey, valuename, 0, REG_SZ, (BYTE*)filepath, ((DWORD)strlen(filepath) + 1) * sizeof(unsigned char));
			RegCloseKey(hRegKey);
		}

		if (regres != ERROR_SUCCESS) {
			SpoutLogWarning("WritePathToRegistry - could not write to registry");
			return false;
		}

		return true;

	}

	bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename)
	{
		HKEY  hRegKey = NULL;
		LONG  regres = 0;

		// 01.01.18
		if (!subkey[0]) {
			SpoutLogWarning("RemovePathFromRegistry - no subkey specified");
			return false;
		}

		regres = RegOpenKeyExA(hKey, subkey, NULL, KEY_ALL_ACCESS, &hRegKey);
		if (regres == ERROR_SUCCESS) {
			regres = RegDeleteValueA(hRegKey, valuename);
			RegCloseKey(hRegKey);
			return true;
		}

		// Quit if the key does not exist
		SpoutLogWarning("RemovePathFromRegistry - could not open key [%s]", subkey);
		return false;
	}

	// Delete a subkey and its values.
	// Note that key names are not case sensitive.
	// It must be a subkey of the key that hKey identifies, but it cannot have subkeys.
	bool RemoveSubKey(HKEY hKey, const char *subkey)
	{
		LONG lStatus;

		lStatus = RegDeleteKeyA(hKey, subkey);
		if (lStatus == ERROR_SUCCESS)
			return true;

		SpoutLogWarning("RemoveSubkey - error #%ld", lStatus);
		return false;
	}

	bool FindSubKey(HKEY hKey, const char *subkey)
	{
		HKEY hRegKey;
		LONG lStatus = RegOpenKeyExA(hKey, subkey, NULL, KEY_READ, &hRegKey);
		if(lStatus == ERROR_SUCCESS) {
			RegCloseKey(hRegKey);
			return true;
		}

		SpoutLogWarning("FindSubkey - error #%ld", lStatus);
		return false;

	}

	//
	// Local functions
	//

	// Perform the log
	void _doLog(SpoutLogLevel level, const char* format, va_list args)
	{
		char currentLog[128];
		std::string logString;

		if (bEnableLog
			&& level != SPOUT_LOG_SILENT
			&& CurrentLogLevel != SPOUT_LOG_SILENT
			&& level >= CurrentLogLevel
			&& format != nullptr) {

			// Construct the current log
			vsprintf_s(currentLog, 128, format, args);
			logString = currentLog;

			if (bConsole) {
				// For console output, allow multiple warnings
				FILE* out = stdout;
				if (level != SPOUT_LOG_NONE && level != SPOUT_LOG_VERBOSE) {
					fprintf(out, "[%s] ", _levelName(level).c_str());
				}
				vfprintf(out, format, args);
				fprintf(out, "\n");
			}

			// Save the last log for warning or fatal
			if (level >= SPOUT_LOG_WARNING) {
				// Prevent multiple logs by comparing with the last
				if (logString == LastSpoutLog)
					return;
				// Fatal error will quit - we want to know why
				if (level == SPOUT_LOG_FATAL) {
					SpoutMessageBox(NULL, LastSpoutLog.c_str(), "Spout", MB_OK);
				}
				LastSpoutLog = logString; // update the last log
			}

			if (!logPath.empty()) {
				// Log file output - append the current log
				logFile.open(logPath, logFile.app);
				if (logFile.is_open()) {
					char name[128];
					name[0] = 0;
					if (level != SPOUT_LOG_NONE && level != SPOUT_LOG_VERBOSE) {
						sprintf_s(name, 128, "[%s] ", _levelName(level).c_str());
					}
					logFile << name << currentLog << std::endl;
					logFile.close();
				}
			}
		}
	}


	// Get the default log file path
	std::string _getLogPath()
	{
		wchar_t wpath[MAX_PATH];
		LPWSTR appdatapath = NULL;
		char logpath[MAX_PATH];
		logpath[0] = 0;

		// Try the User "AppData > Roaming > Spout" folder first
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT_PATH, NULL, &appdatapath);
		if (SUCCEEDED(hr) && appdatapath) {
			wcscpy_s(wpath, MAX_PATH, appdatapath);
			wcscat_s(wpath, MAX_PATH, L"\\Spout");
			wcstombs_s(NULL, logpath, wpath, MAX_PATH);
			if (_access(logpath, 0) == -1) {
				if (!CreateDirectoryA(logpath, NULL)) {
					hr = S_FALSE;
				}
			}
		}

		if (hr == S_FALSE) {
			// CreateDirectory failed
			// Find the path of the executable
			GetModuleFileNameA(NULL, (LPSTR)logpath, sizeof(logpath));
			PathRemoveFileSpecA((LPSTR)logpath);
		}

		return logpath;

	}

	// Get the name for the current log level
	std::string _levelName(SpoutLogLevel level) {
		switch (level) {
		case SPOUT_LOG_SILENT:
			return "silent";
		case SPOUT_LOG_VERBOSE:
			return "verbose";
		case SPOUT_LOG_NOTICE:
			return "notice";
		case SPOUT_LOG_WARNING:
			return "warning";
		case SPOUT_LOG_ERROR:
			return "error";
		case SPOUT_LOG_FATAL:
			return "fatal";
		default:
			return "";
		}
	}


	// Log to file with optional append 
	void _logtofile(bool append)
	{
		bool bNewFile = true;

		// Set default log file if not specified
		// C:\Users\username\AppData\Roaming\Spout\SpoutLog.txt
		if (logPath.empty()) {
			logPath = _getLogPath();
			logPath += "\\SpoutLog.txt";
		}

		// Check for existence before file open
		if (_access(logPath.c_str(), 0) != -1) {
			bNewFile = false; // File exists already
		}

		// File is created if it does not exist
		if (append) {
			logFile.open(logPath, logFile.app);
		}
		else {
			logFile.open(logPath);
		}

		if (logFile.is_open()) {
			// Date and time to identify the log
			char tmp[128];
			time_t datime;
			struct tm tmbuff;
			time(&datime);
			localtime_s(&tmbuff, &datime);
			int year = tmbuff.tm_year + 1900;
			int month = tmbuff.tm_mon + 1;
			int day = tmbuff.tm_mday;
			int hour = tmbuff.tm_hour;
			int min = tmbuff.tm_min;
			sprintf_s(tmp, 128, "%4d-%2.2d-%2.2d %2.2d:%2.2d", year, month, day, hour, min);

			if (append && !bNewFile) {
				logFile << "   " << tmp << std::endl;
			}
			else {
				logFile << "========================" << std::endl;
				logFile << "    Spout log file" << std::endl;
				logFile << "========================" << std::endl;
				logFile << "   " << tmp << std::endl;
			}
			logFile.close();
		}
		else {
			// disable file writes and use a console instead
			logPath = "";
		}
	}


}
