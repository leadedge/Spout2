//
//	SpoutLibrary.dll
//
//	Spout SDK dll compatible with any C++ compiler
//
/*
		Copyright (c) 2016-2020, Lynn Jarvis. All rights reserved.

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

#ifndef __SpoutLibrary__
#define __SpoutLibrary__

#include <windows.h>
#include <GL/GL.h>
#include <string>
#include <d3d9.h>


#define SPOUTLIBRARY_EXPORTS // defined for this DLL. The application imports rather than exports

#ifdef SPOUTLIBRARY_EXPORTS
#define SPOUTAPI __declspec(dllexport)
#else
#define SPOUTAPI __declspec(dllimport)
#endif

// Local log level definitions
enum LogLevel {
	SPOUT_LOG_SILENT,
	SPOUT_LOG_VERBOSE,
	SPOUT_LOG_NOTICE,
	SPOUT_LOG_WARNING,
	SPOUT_LOG_ERROR,
	SPOUT_LOG_FATAL
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

	// -----------------------------------------
	// 2.007
	//

	//
	// Sender
	//

	// Return sender width
	virtual unsigned int GetWidth() = 0;
	// Return sender height
	virtual unsigned int GetHeight() = 0;
	// Return sender frame number
	virtual long GetFrame() = 0;
	// Return sender frame rate
	virtual double GetFps() = 0;
	// Sender frame rate control
	virtual void HoldFps(int fps) = 0;

	//
	// Receiver
	//
	// Set the sender name to connect to
	virtual void SetReceiverName(const char* SenderName) = 0;
	// Receive texture data
	virtual bool ReceiveTextureData(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0) = 0;
	// Receive pixel data
	virtual bool ReceiveImageData(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0) = 0;
	// Return whether the connected sender has changed
	virtual bool IsUpdated() = 0;
	// Return whether connected to a sender
	virtual bool IsConnected() = 0;
	// Open the user sender selection dialog
	virtual void SelectSender() = 0;
	// Return the connected sender name
	virtual const char * GetSenderName() = 0;
	// Return the connected sender width
	virtual unsigned int GetSenderWidth() = 0;
	// Return the connected sender height
	virtual unsigned int GetSenderHeight() = 0;
	// Return the connected sender frame number
	virtual long GetSenderFrame() = 0;
	// Return the connected sender frame rate
	virtual double GetSenderFps() = 0;
	// Return whether the received frame is new
	virtual bool IsFrameNew() = 0;

	//
	// Common
	//

	// Disable frame counting for this application
	virtual void DisableFrameCount() = 0;
	// Return frame count status
	virtual bool IsFrameCountEnabled() = 0;

	//
	// DX9 application support
	//

	// Set the DX9ex object and device from the application
	virtual bool SetDX9device(IDirect3DDevice9Ex* pDevice) = 0;
	// Write a DX9 surface to the spout sender shared texture
	virtual bool WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface) = 0;

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
	// Return the log file as a string
	virtual std::string GetSpoutLog() = 0;
	// Show the log file folder in Windows Explorer
	virtual void ShowSpoutLogs() = 0;
	// Disable logging
	virtual void DisableSpoutLog() = 0;
	// Set the current log level
	// SPOUT_LOG_SILENT  - Disable all messages
	// SPOUT_LOG_VERBOSE - Show all messages
	// SPOUT_LOG_NOTICE  - Show information messages - default
	// SPOUT_LOG_WARNING - Something might go wrong
	// SPOUT_LOG_ERROR   - Something did go wrong
	// SPOUT_LOG_FATAL   - Something bad happened
	virtual void SetSpoutLogLevel(LogLevel level) = 0;
	// Logs
	virtual void SpoutLog(const char* format, ...) = 0;
	virtual void SpoutLogVerbose(const char* format, ...) = 0;
	virtual void SpoutLogNotice(const char* format, ...) = 0;
	virtual void SpoutLogWarning(const char* format, ...) = 0;
	virtual void SpoutLogError(const char* format, ...) = 0;
	virtual void SpoutLogFatal(const char* format, ...) = 0;
	// SpoutPanel Messagebox with optional timeout
	virtual int SpoutMessageBox(const char * message, DWORD dwMilliseconds = 0) = 0;
	// SpoutPanel Messagebox with standard arguments
	virtual int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0) = 0;

	//
	// Registry utilities
	//
	virtual bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue) = 0;
	virtual bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue) = 0;
	virtual bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath) = 0;
	virtual bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath) = 0;
	virtual bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename) = 0;
	virtual bool RemoveSubKey(HKEY hKey, const char *subkey) = 0;
	virtual bool FindSubKey(HKEY hKey, const char *subkey) = 0;

	// -----------------------------------------

	//
	// 2.006 and earlier
	//

	// Sender
	virtual bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0) = 0;
	virtual bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height) = 0;
	virtual void ReleaseSender(DWORD dwMsec = 0) = 0;
	virtual bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0) = 0;
	virtual bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true) = 0;
	virtual bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert=false) = 0;

	// Receiver
	virtual bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false) = 0;
	virtual void ReleaseReceiver() = 0;
	virtual bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO = 0) = 0;
	virtual bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0) = 0;
	virtual bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected) = 0;

	virtual bool IsInitialized() = 0;
	virtual bool BindSharedTexture() = 0;
	virtual bool UnBindSharedTexture() = 0;
	virtual GLuint GetSharedTextureID() = 0;

	virtual int  GetSenderCount() = 0;
	virtual bool GetSender(int index, char* sendername, int MaxSize = 256) = 0;
	virtual bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat) = 0;
	virtual bool GetActiveSender(char* Sendername) = 0;
	virtual bool SetActiveSender(const char* Sendername) = 0;
	
	// Utilities
	virtual bool SetDX9(bool bDX9 = true) = 0; // User request to use DirectX 9 (default is DirectX 11)
	virtual bool GetDX9() = 0; // Return the flag that has been set
	virtual bool SetMemoryShareMode(bool bMem = true) = 0;
	virtual bool GetMemoryShareMode() = 0;
	virtual int  GetShareMode() = 0;
	virtual bool SetShareMode(int mode) = 0;
	virtual void SetBufferMode(bool bActive) = 0;
	virtual bool GetBufferMode() = 0;

	virtual int  GetMaxSenders() = 0; // Get maximum senders allowed
	virtual void SetMaxSenders(int maxSenders) = 0; // Set maximum senders allowed
	virtual bool GetHostPath(const char *sendername, char *hostpath, int maxchars) = 0; // The path of the host that produced the sender
	virtual int  GetVerticalSync() = 0;
	virtual bool SetVerticalSync(bool bSync = true) = 0;
	virtual bool SelectSenderPanel(const char* message = NULL) = 0;

	// Adapter functions
	virtual int  GetNumAdapters() = 0; // Get the number of graphics adapters in the system
	virtual bool GetAdapterName(int index, char *adaptername, int maxchars) = 0; // Get an adapter name
	virtual bool SetAdapter(int index = 0) = 0; // Set required graphics adapter for output
	virtual int  GetAdapter() = 0; // Get the SpoutDirectX global adapter index

	// OpenGL
	virtual bool CreateOpenGL() = 0;
	virtual bool CloseOpenGL() = 0;
	virtual bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0) = 0;

	
	// Library release function
    virtual void Release() = 0;

};


// Handle type. In C++ language the interface type is used.
typedef SPOUTLIBRARY* SPOUTHANDLE;

// Factory function that creates instances of the SPOUT object.
extern "C" SPOUTAPI SPOUTHANDLE WINAPI GetSpout(VOID);

#endif
////////////////////////////////////////////////////////////////////////////////
