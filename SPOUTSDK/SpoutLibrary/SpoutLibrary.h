// SpoutMessageBox(struct HWND__ *,char const *,char const *,unsigned int,
// SpoutMessageBox(HWND *,char const *,char const *,unsigned int,
//
//	SpoutLibrary.dll
//
//	Spout SDK dll compatible with any C++ compiler
//
/*
		Copyright (c) 2016-2025, Lynn Jarvis. All rights reserved.

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

#ifdef _MSC_VER
#pragma warning(disable : 26433) // Function should be marked with 'override'
#endif

// for definitions
#include <windows.h>
#include <string>
#include <vector>
#include <dxgiformat.h> // for DXGI_FORMAT enum
#include <d3d11.h> // For ID3D11Texture2D

// Define here to avoid include of GL.h 
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;

#ifndef GL_RGBA
#define GL_RGBA 0x1908
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

#define SPOUTLIBRARY_EXPORTS // defined for this DLL. The application imports rather than exports

#ifdef SPOUTLIBRARY_EXPORTS
#define SPOUTAPI __declspec(dllexport)
#else
#define SPOUTAPI __declspec(dllimport)
#endif

// Local log level definitions for SetSpoutLogLevel.
// "SpoutLogLevel" enum conflicts with previous definition
// in SpoutUtils if it is used in the application.
// Change enum name to "SpoutLibLogLevel".
//
// Option : disable warning C26812 (unscoped enums) for Visual Studio.
//
// Use of C++11 scoped (class) enums are not compatible with early compilers (< VS2012 and others).
// For Visual Studio, this warning is designated "Prefer" and "C" standard unscoped enums are
// therefore retained for compatibility. The warning can be enabled or disabled here.
//
#ifdef _MSC_VER
#pragma warning(disable:26812)
#endif

//
enum SpoutLibLogLevel {
	SPOUT_LOG_SILENT,
	SPOUT_LOG_VERBOSE,
	SPOUT_LOG_NOTICE,
	SPOUT_LOG_WARNING,
	SPOUT_LOG_ERROR,
	SPOUT_LOG_FATAL,
	SPOUT_LOG_NONE
};

////////////////////////////////////////////////////////////////////////////////
//
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
//

struct SPOUTLIBRARY
{

	//
	// Sender
	//

	virtual void SetSenderName(const char* sendername = nullptr) = 0;
	// Set the sender DX11 shared texture format
	virtual void SetSenderFormat(DWORD dwFormat) = 0;
	// Close sender and free resources
	//   A sender is created or updated by all sending functions
	virtual void ReleaseSender(DWORD dwMsec = 0) = 0;
	// Send texture attached to fbo
	//   The fbo must be currently bound
	//   The sending texture can be larger than the size that the sender is set up for
	//   For example, if the application is using only a portion of the allocated texture space,  
	//   such as for Freeframe plugins. (The 2.006 equivalent is DrawToSharedTexture)
	//   To send the OpenGL default framebuffer, specify "0" for the fbo ID, width and height.
	virtual bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true) = 0;
	// Send OpenGL texture
	virtual bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0) = 0;
	// Send image pixels
	virtual bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false) = 0;
	// Sender status
	virtual bool IsInitialized() = 0;
	// Sender name
	virtual const char* GetName() = 0;
	// Sender width
	virtual unsigned int GetWidth() = 0;
	// Sender height
	virtual unsigned int GetHeight() = 0;
	// Sender frame rate
	virtual double GetFps() = 0;
	// Sender frame number
	virtual long GetFrame() = 0;
	// Sender share handle
	virtual HANDLE GetHandle() = 0;
	// Sender sharing method
	virtual bool GetCPU() = 0;
	// Sender GL/DX hardware compatibility
	virtual bool GetGLDX() = 0;

	//
	// Receiver
	//

	// Specify sender for connection
	//   If a name is specified, the receiver will not connect to any other unless the user selects one
	//   If that sender closes, the receiver will wait for the nominated sender to open 
	//   If no name is specified, the receiver will connect to the active sender
	virtual void SetReceiverName(const char* SenderName = nullptr) = 0;
	// Get sender for connection
	virtual bool GetReceiverName(char* SenderName, int maxchars = 256) = 0;
	// Close receiver and release resources ready to connect to another sender
	virtual void ReleaseReceiver() = 0;
	// Receive texture
	//   If no arguments, connect to a sender and retrieve texture details ready for access
	//	 (see BindSharedTexture and UnBindSharedTexture)
	// 	 Connect to a sender and inform the application to update
	//   the receiving texture if it has changed dimensions
	//   For no change, copy the sender shared texture to the application texture
	virtual bool ReceiveTexture(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0) = 0;
	// Receive image pixels
	//   Connect to a sender and inform the application to update
	//   the receiving buffer if it has changed dimensions
	//   For no change, copy the sender shared texture to the pixel buffer
	virtual bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0) = 0;
	// Query whether the sender has changed
	//   Checked at every cycle before receiving data
	virtual bool IsUpdated() = 0;
	// Query sender connection
	//   If the sender closes, receiving functions return false,  
	virtual bool IsConnected() = 0;
	// Query received frame status
	//   The receiving texture or pixel buffer is only refreshed if the sender has produced a new frame  
	//   This can be queried to process texture data only for new frames
	virtual bool IsFrameNew() = 0;
	// Get sender name
	virtual const char* GetSenderName() = 0;
	// Get sender width
	virtual unsigned int GetSenderWidth() = 0;
	// Get sender height
	virtual unsigned int GetSenderHeight() = 0;
	// Get sender DirectX texture format
	virtual DWORD GetSenderFormat() = 0;
	// Get sender frame rate
	virtual double GetSenderFps() = 0;
	// Get sender frame number
	virtual long GetSenderFrame() = 0;
	// Received sender share handle
	virtual HANDLE GetSenderHandle() = 0;
	// Received sender texture
	virtual ID3D11Texture2D* GetSenderTexture() = 0;
	// Received sender sharing mode
	virtual bool GetSenderCPU() = 0;
	// Received sender GL/DX compatibility
	virtual bool GetSenderGLDX() = 0;
	// The path of the host that produced the sender
	virtual bool GetHostPath(const char* sendername, char* hostpath, int maxchars) = 0;
	// Return a list of current senders
	virtual std::vector<std::string> GetSenderList() = 0;
	// Open sender selection dialog
	virtual void SelectSender(HWND hwnd = NULL) = 0;
	// Open sender selection dialog with optional message - 2.006 compatibility
	virtual void SelectSenderPanel(const char* message) = 0;

	//
	// Frame count
	//

	// Enable or disable frame counting globally
	virtual void SetFrameCount(bool bEnable) = 0;
	// Disable frame counting specifically for this application
	virtual void DisableFrameCount() = 0;
	// Return frame count status
	virtual bool IsFrameCountEnabled() = 0;
	// Sender frame rate control
	virtual void HoldFps(int fps) = 0;
	// Get system refresh rate
	virtual double GetRefreshRate() = 0;
	// Signal sync event 
	virtual void SetFrameSync(const char* SenderName) = 0;
	// Wait or test for a sync event
	virtual bool WaitFrameSync(const char *SenderName, DWORD dwTimeout = 0) = 0;
	// Enable / disable frame sync
	virtual void EnableFrameSync(bool bSync = true) = 0;
	// Close frame sync
	virtual void CloseFrameSync() = 0;
	// Check for frame sync option
	virtual bool IsFrameSyncEnabled() = 0;
	// Vertical sync status
	virtual int GetVerticalSync() = 0;
	// Lock to monitor vertical sync
	virtual bool SetVerticalSync(bool bSync = true) = 0;

	//
	// Data sharing
	//

	// Write data
	virtual bool WriteMemoryBuffer(const char *sendername, const char* data, int length) = 0;
	// Read data
	virtual int  ReadMemoryBuffer(const char* sendername, char* data, int maxlength) = 0;
	// Create a shared memory buffer
	virtual bool CreateMemoryBuffer(const char *name, int length) = 0;
	// Delete a shared memory buffer
	virtual bool DeleteMemoryBuffer() = 0;
	// Get the number of bytes available for data transfer
	virtual int GetMemoryBufferSize(const char *name) = 0;

	//
	// Log utilities
	//

	// Open console for debugging
	virtual void OpenSpoutConsole() = 0;
	// Close console
	virtual void CloseSpoutConsole(bool bWarning = false) = 0;
	// Enable spout log to the console
	virtual void EnableSpoutLog() = 0;
	// Enable spout log to a file with optional append
	virtual void EnableSpoutLogFile(const char* filename, bool bappend = false) = 0;
	// Disable logging to file
	virtual void DisableSpoutLogFile() = 0;
	// Remove a log file
	virtual void RemoveSpoutLogFile(const char* filename = nullptr) = 0;
	// Disable logging to console and file
	virtual void DisableSpoutLog() = 0;
	// Disable logging temporarily
	virtual void DisableLogs() = 0;
	// Enable logging again
	virtual void EnableLogs() = 0;
	// Are console logs enabled
	virtual bool LogsEnabled() = 0;
	// Is file logging enabled
	virtual bool LogFileEnabled() = 0;
	// Return the full log file path
	virtual std::string GetSpoutLogPath() = 0;
	// Return the log file as a string
	virtual std::string GetSpoutLog() = 0;
	// Show the log file folder in Windows Explorer
	virtual void ShowSpoutLogs() = 0;

	// Set the current log level
	// SPOUT_LOG_SILENT  - Disable all messages
	// SPOUT_LOG_VERBOSE - Show all messages
	// SPOUT_LOG_NOTICE  - Show information messages - default
	// SPOUT_LOG_WARNING - Something might go wrong
	// SPOUT_LOG_ERROR   - Something did go wrong
	// SPOUT_LOG_FATAL   - Something bad happened
	virtual void SetSpoutLogLevel(SpoutLibLogLevel level) = 0;
	// General purpose log
	virtual void SpoutLog(const char* format, ...) = 0;
	// Verbose - show log for SPOUT_LOG_VERBOSE or above
	virtual void SpoutLogVerbose(const char* format, ...) = 0;
	// Notice - show log for SPOUT_LOG_NOTICE or above
	virtual void SpoutLogNotice(const char* format, ...) = 0;
	// Warning - show log for SPOUT_LOG_WARNING or above
	virtual void SpoutLogWarning(const char* format, ...) = 0;
	// Error - show log for SPOUT_LOG_ERROR or above
	virtual void SpoutLogError(const char* format, ...) = 0;
	// Fatal - always show log
	virtual void SpoutLogFatal(const char* format, ...) = 0;

	//
	// MessageBox dialog
	//

	// MessageBox dialog with optional timeout
	//   Used where a Windows MessageBox would interfere with the application GUI
	//   The dialog closes itself if a timeout is specified
	virtual int SpoutMessageBox(const char* message, DWORD dwMilliseconds = 0) = 0;
	// MessageBox dialog with variable arguments
	virtual int SpoutMessageBox(const char* caption, const char* format, ...) = 0;
	// MessageBox with variable arguments and icon, buttons
	virtual int SpoutMessageBox(const char* caption, UINT uType, const char* format, ...) = 0;
	// MessageBox dialog with standard arguments
	//   Replaces an existing MessageBox call
	virtual int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0) = 0;
	// MessageBox dialog with standard arguments
	//   including taskdialog main instruction large text
	virtual int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption,  UINT uType, const char* instruction, DWORD dwMilliseconds = 0) = 0;
	// MessageBox dialog with an edit control for text input
	virtual int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::string& text) = 0;
	// MessageBox dialog with a combobox control for item selection
	virtual	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::vector<std::string> items, int& selected) = 0;

	// Custom icon for SpoutMessageBox from resources
	virtual void SpoutMessageBoxIcon(HICON hIcon) = 0;
	// Custom icon for SpoutMessageBox from file
	virtual bool SpoutMessageBoxIcon(std::string iconfile) = 0;
	// Custom button for SpoutMessageBox
	virtual void SpoutMessageBoxButton(int ID, std::wstring title) = 0;
	// Activate modeless mode using SpoutPanel.exe
	virtual void SpoutMessageBoxModeless(bool bMode = true) = 0;
	// Window handle for SpoutMessageBox where not specified
	virtual void SpoutMessageBoxWindow(HWND hWnd) = 0;
	// Position to point SpoutMessageBox
	virtual void SpoutMessageBoxPosition(POINT pt) = 0;
	// Copy text to the clipboard
	virtual bool CopyToClipBoard(HWND hwnd, const char* caps) = 0;
	// Open logs folder
	virtual bool OpenSpoutLogs() = 0;

	//
	// Registry utilities
	//
	// Read subkey DWORD value
	virtual bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue) = 0;
	// Write subkey DWORD value
	virtual bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue) = 0;
	// Read subkey character string
	virtual bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath) = 0;
	// Write subkey character string
	virtual bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath) = 0;
	// Write subkey binary hex data string
	virtual bool WriteBinaryToRegistry(HKEY hKey, const char* subkey, const char* valuename, const unsigned char* hexdata, DWORD nchars) = 0;
	// Remove subkey value name
	virtual bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename) = 0;
	// Delete a subkey and its values.
	//   It must be a subkey of the key that hKey identifies, but it cannot have subkeys.  
	//   Note that key names are not case sensitive.  
	virtual bool RemoveSubKey(HKEY hKey, const char *subkey) = 0;
	// Find subkey
	virtual bool FindSubKey(HKEY hKey, const char *subkey) = 0;

	//
	// Information
	//
	// Get SDK version number string e.g. "2.007.000"
	// Optional - return as a single number
	virtual std::string GetSDKversion(int* pNumber = nullptr) = 0;
	// Computer type
	virtual bool IsLaptop() = 0;
	// Get the module handle of an executable or dll
	virtual HMODULE GetCurrentModule() = 0;
	// Get executable or dll version
	virtual std::string GetExeVersion(const char* path) = 0;
	// Get executable or dll path
	virtual std::string GetExePath(bool bFull = false) = 0;
	// Get executable or dll name
	virtual std::string GetExeName() = 0;
	// Remove file name and return the path
	virtual std::string GetPath(std::string fullpath) = 0;
	// Remove path and return the file name
	virtual std::string GetName(std::string fullpath) = 0;

	//
	// Timing utilities
	//
	// Start timing period
	virtual void StartTiming() = 0;

	// Stop timing and return milliseconds or microseconds elapsed.
	// (microseconds default).
	virtual double EndTiming(bool microseconds = false, bool bPrint = false) = 0;

	// -----------------------------------------
	// Bind OpenGL shared texture
	virtual bool BindSharedTexture() = 0;
	// Un-bind OpenGL shared texture
	virtual bool UnBindSharedTexture() = 0;
	// OpenGL shared texture ID
	virtual GLuint GetSharedTextureID() = 0;

	//
	// Sender names
	//

	// Number of senders
	virtual int GetSenderCount() = 0;
	// Sender item name
	virtual bool GetSender(int index, char* sendername, int MaxSize = 256) = 0;
	// Find a sender in the list
	virtual bool FindSenderName(const char* sendername) = 0;
	// Sender information
	virtual bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat) = 0;
	// Current active sender
	virtual bool GetActiveSender(char* Sendername) = 0;
	// Set sender as active
	virtual bool SetActiveSender(const char* Sendername) = 0;
	
	//
	// Get user registry settings recorded by "SpoutSettings"
	// Set them either to the registry or for the application only
	//

	// Get user buffering mode
	virtual bool GetBufferMode() = 0;
	// Set application buffering mode
	virtual void SetBufferMode(bool bActive = true) = 0;
	// Get user number of pixel buffers
	virtual int GetBuffers() = 0;
	// Set application number of pixel buffers
	virtual void SetBuffers(int nBuffers) = 0;
	// Get user Maximum senders allowed
	virtual int GetMaxSenders() = 0;
	// Set user Maximum senders allowed
	virtual void SetMaxSenders(int maxSenders) = 0;

	//
	// 2.006 compatibility
	//

	// Create a sender
	virtual bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0) = 0;
	// Update a sender
	virtual bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height) = 0;
	// Create receiver connection
	virtual bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height) = 0;
	// Check receiver connection
	virtual bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected) = 0;
	// Get user DX9 mode
	virtual bool GetDX9() = 0;
	// Set user DX9 mode
	virtual bool SetDX9(bool bDX9 = true) = 0;
	// Get user memory share mode
	virtual bool GetMemoryShareMode() = 0;
	// Set user memory share mode
	virtual bool SetMemoryShareMode(bool bMem = true) = 0;
	// Get user CPU mode
	virtual bool GetCPUmode() = 0;
	// Set user CPU mode
	virtual bool SetCPUmode(bool bCPU) = 0;
	// Get user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	virtual int GetShareMode() = 0;
	// Set user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	virtual void SetShareMode(int mode) = 0;

	//
	// Graphics compatibility
	//

	// Get auto GPU/CPU share depending on compatibility
	virtual bool GetAutoShare() = 0;
	// Set auto GPU/CPU share depending on compatibility
	virtual void SetAutoShare(bool bAuto = true) = 0;
	// OpenGL texture share compatibility
	virtual bool IsGLDXready() = 0;

	//
	// Adapter functions
	//

	// The number of graphics adapters in the system
	virtual int GetNumAdapters() = 0;
	// Adapter item name
	virtual bool GetAdapterName(int index, char *adaptername, int maxchars) = 0;
	// Return current adapter name
	virtual char* AdapterName() = 0;
	// Get adapter index 
	virtual int GetAdapter() = 0;
	// Get the description and output display name of the current adapter
	virtual bool GetAdapterInfo(char* description, char* output, int maxchars = 0) = 0;
	// Get the description and output display name for a given adapter
	virtual bool GetAdapterInfo(int index, char* description, char* output, int maxchars = 0) = 0;

	//
	// Graphics preference
	//
	// Windows 10+ SDK required
	//
#ifdef NTDDI_WIN10_RS4

	// Get the Windows graphics preference for an application
	//	-1 - Not registered
	//	 0 - Let Windows decide  DXGI_GPU_PREFERENCE_UNSPECIFIED
	//	 1 - Power saving        DXGI_GPU_PREFERENCE_MINIMUM_POWER
	//	 2 - High performance    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
	virtual int GetPerformancePreference(const char* path) = 0;
	// Set the Windows graphics preference for an application
	virtual bool SetPerformancePreference(int preference, const char* path) = 0;
	// Get the graphics adapter name for a Windows preference
	virtual bool GetPreferredAdapterName(int preference, char* adaptername, int maxchars) = 0;
	// Set graphics adapter index for a Windows preference
	virtual bool SetPreferredAdapter(int preference) = 0;
	// Availability of Windows graphics preference
	virtual bool IsPreferenceAvailable() = 0;
	// Is the path a valid application
	virtual bool IsApplicationPath(const char* path) = 0;
#endif


	//
	// OpenGL utilities
	//

	// Create an OpenGL window and context for situations where there is none.
	//   Not used if applications already have an OpenGL context.
	//   Always call CloseOpenGL afterwards.
	virtual bool CreateOpenGL() = 0;
	// Close OpenGL window
	virtual bool CloseOpenGL() = 0;
	// Copy OpenGL texture with optional invert
	//   Textures can be different sizes
	virtual bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0) = 0;
	// Copy OpenGL texture data to a pixel buffer
	virtual bool ReadTextureData(GLuint SourceID, GLuint SourceTarget,
		void* data, unsigned int width, unsigned int height, unsigned int rowpitch,
		GLenum dataformat, GLenum datatype, bool bInvert = false, GLuint HostFBO = false) = 0;

	//
	// Formats
	//

	// Get sender DX11 shared texture format
	virtual DXGI_FORMAT GetDX11format() = 0;
	// Set sender DX11 shared texture format
	virtual void SetDX11format(DXGI_FORMAT textureformat) = 0;
	// Return OpenGL compatible DX11 format
	virtual DXGI_FORMAT DX11format(GLint glformat) = 0;
	// Return DX11 compatible OpenGL format
	virtual GLint GLDXformat(DXGI_FORMAT textureformat = DXGI_FORMAT_UNKNOWN) = 0;
	// Return OpenGL texture internal format
	virtual int GLformat(GLuint TextureID, GLuint TextureTarget) = 0;
	// Return OpenGL texture format description
	virtual std::string GLformatName(GLint glformat = 0) = 0;

	//
	// DirectX utilities
	//

	virtual bool OpenDirectX() = 0;
	virtual void CloseDirectX() = 0;

	// Initialize and prepare DirectX 11
	virtual bool OpenDirectX11(void * pDevice = nullptr) = 0;
	virtual void CloseDirectX11() = 0;

	// Return the class device
	virtual void* GetDX11Device() = 0;

	// Return the class context
	virtual void* GetDX11Context() = 0;
	
	// Library release function
    virtual void Release() = 0;

};

// Handle type. In C++ language the interface type is used.
typedef SPOUTLIBRARY* SPOUTHANDLE;

// Factory function that creates an instance of the SPOUT object.
extern "C" SPOUTAPI SPOUTHANDLE WINAPI GetSpout(VOID);

////////////////////////////////////////////////////////////////////////////////
