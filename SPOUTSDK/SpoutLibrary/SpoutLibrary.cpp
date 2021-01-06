//
//	SpoutLibrary.dll
//
//	Spout SDK dll compatible with any C++ compiler
//
//	Based on the CodeProject "HowTo: Export C++ classes from a DLL" by Alex Blekhman.
//	http://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
//
//		30.03.16 - Build for 2.005 release - VS2012 /MT
//		13.05.16 - Rearrange folders - rebuild 2.005 - VS2012 /MT
//		23.06.16 - Add invert to ReceiveImage
//		23.06.16 - Rebuild for 2.005 release - VS2012 /MT
//		03.07.16 - Rebuild with VS2015
//		13.01.17 - Rebuild for Spout 2.006
//				 - Add SetCPUmode, GetCPUmode, SetBufferMode, GetBufferMode
//				 - Add HostFBO arg to DrawSharedTexture
//		17.01.17 - Add GetShareMode, SetShareMode
//		23.01.17 - Rebuild for Spout 2.006 - VS2012 /MT
//		08.01.17 - Rebuild - VS2012 /MT
//		20.01.17 - Rebuild update - VS2012 /MT
//		05-04-19 - Introduce SpoutFunctions class
//				   Add CreateOpenGL() and CloseOpenGL()
//				   Test with QT and MingW compiler
//		11-05-19 - Build for 32 bit and 64 bit - VS2017 /MT
//		24.05.19 - Add OpenSpoutConsole for debugging
//		03.06.19 - Add CloseSpoutConsole for debugging
//		04-06-19 - Re-build for 32 bit and 64 bit - VS2017 /MT
//		06-06-19 - Re-build for 256 max senders - 32 bit and 64 bit - VS2017 /MT
//		18.06.19 - Change sender Update to include sender name
//		26.09.19 - Remove redundant 2.007 functions for single class
//				 - Remove Update and use UpdateSender
//				 - Remove CloseSender/CloseReceiver and use ReleaseSender/ReleaseSender
//		29.09.19 - Change IsDX9 to GetDX9 to avoid repeated compatibility test
//		09.10.19 - Add WriteDX9surface and SetDX9device
//		22.10.19 - Add include guard in SpoutLibrary.h
//				 - Add SendFboTexture
//				 - Relocate ReleaseSender
//				   Re-build for 32 bit and 64 bit - VS2017 /MT
//		27.11.19 - Re-build for revised Spout SDK - 32 bit and 64 bit - VS2017 / MT
//		18.01.20 - Add CopyTexture, GetSenderTextureID. Update receiver example
//		19.01.20 - Remove send data functions
//				   Change SendFboTexture to SendFbo
//		20.01.20 - Change GetSenderTextureID() to GetSharedTextureID
//		30.05.20 - Change SendTextureData/SendImageData back to SendTexture/SendImage
//				   Remove 2.006 overloads. Library is now exclusively 2.007.
//				   Re-build for revised Spout SDK - 32 bit and 64 bit - VS2017 / MT
//		18.06.20 - Add GetSenderFormat()
//		12.07.20 - Simplify and use 2.007 sender / receiver classes
//				 - Remove GetShareMode / SetShareMode
//		18.07.20 - Re-build 32/64 bit - VS2017 / MT
//		28.12.20 - Update functions using the SpoutGL class for 2.007
//		29.12.20 - Re-arrange files for CMake and modify project files
//		31.12.20 - Re-build 32/64 bit - VS2017 / MT
//		06.01.20 - Re-build 32/64 bit - VS2017 / MT
//
/*
		Copyright (c) 2016-2021, Lynn Jarvis. All rights reserved.

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

#include <stdio.h>
#include "SpoutLibrary.h"
#include "..\SpoutGL\SpoutSDK.h"

using namespace spoututils;


////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the SPOUT interface.
//

// Title: SpoutLibrary
//
// <https://spout.zeal.co/>
//
//	Spout SDK dll compatible with any C++ compiler
//
//	Based on the CodeProject :
//
//	"HowTo: Export C++ classes from a DLL" by Alex Blekhman.
//
//	http://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
//
// Group: Building the library
// 
// The "VS2017" folder contains Visual Studio projects to build the dll.
//
// The Spout SDK source files should be located in a folder "SpoutGL" two levels above the project folder.
// 
// Open the SpoutLibrary solution, change to "Release" and build the project.
// SpoutLibrary.dll and SpoutLibrary.lib will be in the Win32\Release or x64\Release folder.
// 
// Also refer to "Building the libraries.pdf" for details on building the libraries using CMake.
//
// Group: Building applications with the library
//
// o Include SpoutLibrary.h in your application header file.
// o Include SpoutLibrary.lib in your project for the linker.
// o Include SpoutLibrary.dll in the application executable folder.
// 
//
// All functions are the same as described for in the Spout SDK documentation.
//
// Group: Using the library
//
// Refer to the source code of the SpoutLibrary example.
//
//
// 1) Include SpoutLibrary.h in your application header file
// 
// --- Code
// #include "SpoutLibrary.h"
// ---
// 
// 2) create a spout sender object pointer
// 
// --- Code
// SPOUTLIBRARY * sender;
// ---
// 
// 3) Create an instance of the library
// 
// --- Code
// sender = GetSpout();
// ---
// 
// 4) Use the object as usual :
// 
// --- Code
// sender->SendTexture(...) etc.
// ---
// 
// Compare with the graphics sender example using the Spout SDK source files.


class SPOUTImpl : public SPOUTLIBRARY
{

public:

	Spout * spout; // Spout SDK functions object for this class

private: // Spout SDK functions

	//
	// Group: Sender
	//
	// SendFbo, SendTexture and SendImage create or update a sender as required.
	//
	// - If a sender has not been created yet :
	//
	//    - Make sure Spout has been initialized and OpenGL context is available
	//    - Perform a compatibility check for GL/DX interop
	//    - If compatible, create interop for GL/DX transfer
	//    - If not compatible, create a shared texture for the sender
	//    - Create a sender using the DX11 shared texture handle
	//
	// - If the sender exists, test for size change :
	//
	//    - If compatible, update the shared textures and GL/DX interop
	//    - If not compatible, re-create the class shared texture to the new size
	//    - Update the sender and class variables	
	//

	// Function: SetSenderName
	// Set name for sender creation
	//
	//   If no name is specified, the executable name is used. 
	void SetSenderName(const char* sendername = nullptr);
	// Function: SetSenderFormat
	// Set the sender DX11 shared texture format
	void SetSenderFormat(DWORD dwFormat);
	// Function: ReleaseSender
	// Close receiver and release resources.
	//
	// A new sender is created or updated by all sending functions
	void ReleaseSender(DWORD dwMsec = 0);
	// Function: SendFbo
	// Send texture attached to fbo.
	//
	//   The fbo must be currently bound.  
	//   The sending texture can be larger than the size that the sender is set up for.  
	//   For example, if the application is using only a portion of the allocated texture space,  
	//   such as for Freeframe plugins. (The 2.006 equivalent is DrawToSharedTexture).
	bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
	// Function: SendTexture
	// Send OpenGL texture.
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	// Function: SendImage
	// Send pixel image
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
	// Function: GetName
	// Sender name
	const char * GetName();
	// Function: GetWidth
	// Sender width
	unsigned int GetWidth();
	// Function: GetHeight
	// Sender height
	unsigned int GetHeight();
	// Function: GetFps
	// Sender frame rate
	double GetFps();
	// Function: GetFrame
	// Sender frame number
	long GetFrame();
	// Function: GetHandle
	// Sender share handle
	HANDLE GetHandle();

	//
	// Group: Receiver
	//
	// ReceiveTexture and ReceiveImage
	//
	//	- Connect to a sender
	//	- Set class variables for sender name, width and height
	//  - If the sender has changed size, set a flag for the application to update receiving 
	//    texture or image which is re-allocated if IsUpdated() returns true.
	//  - Copy the sender shared texture to the user texture or image.
	//

	// Function: SetReceiverName
	// Specify sender for connection
	//
	//   The application will not connect to any other  unless the user selects one
	//   If that sender closes, the application will wait for the nominated sender to open 
	//   If no name is specified, the receiver will connect to the active sender
	void SetReceiverName(const char * SenderName);
	// Function: ReleaseReceiver
	// Close receiver and release resources ready to connect to another sender
	void ReleaseReceiver();
	// Function: ReceiveTexture
	//
	//   If no arguments are passed :
	//     Connect to a sender and retrieve shared texture details
	//
	//  The texture can then be accessed using :
	//
	//		- BindSharedTexture();
	//		- UnBindSharedTexture();
	//		- GetSharedTextureID();
	//
	//   For a valid OpenGL texture passed :
	//
	//  Copy the sender shared texture
	//
	//    The receiving texture can only be RGBA of dimension (width * height)
	//    and must be re-allocated if IsUpdated() returns true
	//
	bool ReceiveTexture(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0);
	// Function: ReceiveImage
	// Copy the sender texture to image pixels.
	//
	//    The receiving image can be RGBA, RGB or BGR of dimension (width * height)
	//    and must be re-allocated if IsUpdated() returns true
	bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0);
	// Function: IsUpdated
	// Query whether the sender has changed.
	//
	//   Must be checked at every cycle before receiving data. 
	//
	//   If this is not done, the receiving functions fail.
	bool IsUpdated();
	// Function: IsConnected
	// Query sender connection.
	//
	//   If the sender closes, receiving functions return false,  
	//   but connection can be tested at any time.
	bool IsConnected();
	// Function: IsFrameNew
	// Query received frame status
	//
	//   The receiving texture or pixel buffer is refreshed if the sender has produced a new frame  
	//   This can be queried to process texture data only for new frames
	bool IsFrameNew();
	// Function: GetSenderName
	// Get sender name
	const char * GetSenderName();
	// Function: GetSenderWidth
	// Get sender width
	unsigned int GetSenderWidth();
	// Function: GetSenderHeight
	// Get sender height
	unsigned int GetSenderHeight();
	// Function: GetSenderFormat
	// Get sender DirectX texture format
	DWORD GetSenderFormat();
	// Function: GetSenderFps
	// Get sender frame rate
	double GetSenderFps();
	// Function: GetSenderFrame
	// Get sender frame number
	long GetSenderFrame();
	// Function: GetSenderHandle
	// Received sender share handle
	HANDLE GetSenderHandle();
	// Function: GetSenderCPUmode
	// Received sender sharing mode
	bool GetSenderCPUmode();
	// Function: SelectSender
	// Open sender selection dialog
	void SelectSender();

	//
	// Group: Frame counting
	//

	// Function: SetFrameCount
	// Enable or disable frame counting globally
	void SetFrameCount(bool bEnable);
	// Function: DisableFrameCount
	// Disable frame counting specifically for this application
	void DisableFrameCount();
	// Function: IsFrameCountEnabled
	// Return frame count status
	bool IsFrameCountEnabled();
	// Function: HoldFps
	// Frame rate control
	void HoldFps(int fps);

	//
	// Group: Log utilities
	//

	// Function: OpenSpoutConsole
	// Open console window.
	//
	// A console window opens without logs.
	// Useful for debugging with console output.
	void OpenSpoutConsole();
	// Function: CloseSpoutConsole
	// Close console window.
	//
	// The optional warning displays a MessageBox if user notification is required.
	void CloseSpoutConsole(bool bWarning = false);
	// Function: EnableSpoutLog
	// Enable logging to the console.
	//
	// Logs are displayed in a console window.  
	// Useful for program development.
	void EnableSpoutLog();
	// Function: EnableSpoutLogFile
	// Enable logging to a file with optional append.
	//
	// You can instead, or additionally to a console window,  
	// specify output to a text file with the extension of your choice  
	// Example : EnableSpoutLogFile("Sender.log");
	// The log file is re-created every time the application starts unless you specify to append to the existing one :  
	// Example : EnableSpoutLogFile("Sender.log", true);
	// The file is saved in the %AppData% folder unless you specify the full path :  
	//    C:>Users>username>AppData>Roaming>Spout   
	// You can find and examine the log file after the application has run.
	void EnableSpoutLogFile(const char *filename, bool append = false);
	// Function: GetSpoutLog
	// Return the log file as a string
	std::string GetSpoutLog();
	// Function: ShowSpoutLogs
	// Show the log file folder in Windows Explorer
	void ShowSpoutLogs();
	// Function: DisableSpoutLog
	// Disable logging to console and file
	void DisableSpoutLog();
	// Function: SetSpoutLogLevel
	// Set the current log level
	void SetSpoutLogLevel(LogLevel level);
	// Function: SpoutLog
	// General purpose log
	void SpoutLog(const char* format, ...);
	// Function: SpoutLogVerbose
	// Verbose - show log for SPOUT_LOG_VERBOSE or above
	void SpoutLogVerbose(const char* format, ...);
	// Function: SpoutLogNotice
	// Notice - show log for SPOUT_LOG_NOTICE or above
	void SpoutLogNotice(const char* format, ...);
	// Function: SpoutLogWarning
	// Warning - show log for SPOUT_LOG_WARNING or above
	void SpoutLogWarning(const char* format, ...);
	// Function: SpoutLogError
	// Error - show log for SPOUT_LOG_ERROR or above
	void SpoutLogError(const char* format, ...);
	// Function: SpoutLogFatal
	// Fatal - always show log
	void SpoutLogFatal(const char* format, ...);
	// Function: SpoutMessageBox
	// MessageBox dialog with optional timeout.
	//
	// Used where a Windows MessageBox would interfere with the application GUI.  
	// The dialog closes itself if a timeout is specified.
	int SpoutMessageBox(const char * message, DWORD dwMilliseconds = 0);
	// Function: SpoutMessageBox
	// MessageBox dialog with standard arguments.
	// Replaces an existing MessageBox call.
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0);

	//
	// Group: Registry utilities
	//

	// Function: ReadDwordFromRegistry
	// Read subkey DWORD value
	bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue);
	// Function: WriteDwordToRegistry
	// Write subkey DWORD value
	bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue);
	// Function: ReadPathFromRegistry
	// Read subkey character string
	bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath);
	// Function: WritePathToRegistry
	// Write subkey character string
	bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath);
	// Function: RemovePathFromRegistry
	// Remove subkey value name
	bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename);
	// Function: RemoveSubKey
	// Delete a subkey and its values.
	//   It must be a subkey of the key that hKey identifies, but it cannot have subkeys.  
	//   Note that key names are not case sensitive.  
	bool RemoveSubKey(HKEY hKey, const char *subkey);
	// Function: FindSubKey
	// Find subkey
	bool FindSubKey(HKEY hKey, const char *subkey);

	//
	// Group: OpenGL shared texture
	//

	// Function: IsInitialized
	// Initialization status
	bool IsInitialized();
	// Function: BindSharedTexture
	// Bind OpenGL shared texture
	bool BindSharedTexture();
	// Function: UnBindSharedTexture
	// Un-bind OpenGL shared texture
	bool UnBindSharedTexture();
	// Function: GetSharedTextureID
	// OpenGL shared texture ID
	GLuint GetSharedTextureID();

	//
	// Group: Sender names
	//
	
	// Function: GetSenderCount
	// Number of senders
	int  GetSenderCount();
	// Function: GetSender
	// Sender item name in the sender names set
	bool GetSender(int index, char* sendername, int MaxSize = 256);
	// Function: GetSenderInfo
	// Sender information
	bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	// Function: GetActiveSender
	// Current active sender
	bool GetActiveSender(char* Sendername);
	// Function: SetActiveSender
	// Set sender as active
	bool SetActiveSender(const char* Sendername);

	//
	// Group: User registry settings
	// recorded by "SpoutSettings"
	//

	// Function: GetBufferMode
	// Get user buffering mode
	bool GetBufferMode();
	// Function: SetBufferMode
	// Set application buffering mode
	void SetBufferMode(bool bActive = true);
	// Function: GetBuffers
	// Get user number of pixel buffers
	int GetBuffers();
	// Function: SetBuffers
	// Set application number of pixel buffers
	void SetBuffers(int nBuffers);
	// Function: GetMaxSenders
	// Get user Maximum senders allowed
	int GetMaxSenders();
	// Function: SetMaxSenders
	// Set user Maximum senders allowed
	void SetMaxSenders(int maxSenders);
	
	//
	// Group: 2.006 compatibility
	//
	// These functions are not necessary for Version 2.007.
	// But are retained for compatibility with existing 2.006 code.
	//

	// Function: CreateSender
	// Create a sender
	bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
	// Function: UpdateSender
	// Update a sender
	bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height);
	// Function: CreateReceiver
	// Create receiver connection
	bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false);
	// Function: CheckReceiver
	// Check receiver connection
	bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);
	// Function: GetDX9
	// Get user DX9 mode
	bool GetDX9();
	// Function: SetDX9
	// Set user DX9 mode
	bool SetDX9(bool bDX9 = true);
	// Function: GetMemoryShareMode
	// Get user memory share mode
	bool GetMemoryShareMode();
	// Function: SetMemoryShareMode
	// Set user memory share mode
	bool SetMemoryShareMode(bool bMem = true);
	// Function: GetCPUmode
	// Get user CPU mode
	bool GetCPUmode();
	// Function: SetCPUmode
	// Set user CPU mode
	bool SetCPUmode(bool bCPU);
	// Function: GetShareMode
	// Get user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	int GetShareMode();
	// Function: SetShareMode
	// Set user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	void SetShareMode(int mode);

	//
	// Group: Information
	//

	// Function: GetHostPath
	// The path of the host that produced the sender
	//
	// Retrieved from the description string in the sender info memory map
	bool GetHostPath(const char *sendername, char *hostpath, int maxchars);
	// Function: GetVerticalSync
	// Vertical sync status
	int GetVerticalSync();
	// Function: SetVerticalSync
	// Lock to monitor vertical sync
	bool SetVerticalSync(bool bSync = true);
	// Function: GetSpoutVersion
	// Get Spout version
	int GetSpoutVersion();

	//
	// Group: Graphics compatibility
	//

	// Function: GetAutoShare
	// Get auto GPU/CPU share depending on compatibility
	bool GetAutoShare();
	// Function: SetAutoShare
	// Set auto GPU/CPU share depending on compatibility
	void SetAutoShare(bool bAuto = true);
	// Function: IsGLDXready
	// OpenGL texture share compatibility
	bool IsGLDXready();

	//
	// Group: Graphics adapter
	//
	// Note that both the Sender and Receiver must use the same graphics adapter.
	//

	// Function: GetNumAdapters
	// The number of graphics adapters in the system
	int GetNumAdapters();
	// Function: GetAdapterName
	// Get adapter item name
	bool GetAdapterName(int index, char *adaptername, int maxchars);
	// Function: GetAdapter
	// Get adapter index
	int GetAdapter();
	// Function: SetAdapter
	// Set graphics adapter for output
	bool SetAdapter(int index = 0);
	// Function: Adapter
	// Current adapter
	int Adapter();
	// Function: AdapterName
	// Current adapter name
	char * AdapterName();

	//
	// Group: OpenGL utilities
	//

	// Function: CreateOpenGL
	// Create an OpenGL window and context for situations where there is none.
	//   Not used if applications already have an OpenGL context.
	//   Always call CloseOpenGL afterwards.
	bool CreateOpenGL();
	// Function: CloseOpenGL
	// Close OpenGL window
	bool CloseOpenGL();
	// Function: CopyTexture
	// Copy OpenGL texture with optional invert
	//   Textures must be the same size
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0);

	//
	// Group: Class release
	//

	// Function: Release
	// Release the class instance
	void Release();

};


// ---------------------------------------------------------------
// 2.007
//

//
// Sender
//

void SPOUTImpl::SetSenderName(const char* sendername)
{
		spout->SetSenderName(sendername);
}

void SPOUTImpl::SetSenderFormat(DWORD dwFormat)
{
		spout->SetSenderFormat(dwFormat);
}

void SPOUTImpl::ReleaseSender(DWORD dwMsec)
{
	dwMsec = 0; // Not used for 2.007
	spout->ReleaseSender();
}

bool SPOUTImpl::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	return spout->SendFbo(FboID, width, height, bInvert);
}

bool SPOUTImpl::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spout->SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

bool SPOUTImpl::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	return spout->SendImage(pixels, width, height, glFormat, bInvert);
}

const char * SPOUTImpl::GetName()
{
	return spout->GetName();
}

unsigned int SPOUTImpl::GetWidth()
{
	return spout->GetWidth();
}

unsigned int SPOUTImpl::GetHeight()
{
	return spout->GetHeight();
}

double SPOUTImpl::GetFps()
{
	return spout->GetFps();
}

long SPOUTImpl::GetFrame()
{
	return spout->GetFrame();
}

HANDLE SPOUTImpl::GetHandle()
{
	return spout->GetHandle();
}

//
// Receiver
//

void SPOUTImpl::SetReceiverName(const char* SenderName)
{
	spout->SetReceiverName(SenderName);
}

void SPOUTImpl::ReleaseReceiver()
{
	spout->ReleaseReceiver();
}

bool SPOUTImpl::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
{
	return spout->ReceiveTexture(TextureID, TextureTarget, bInvert, HostFbo);
}

bool SPOUTImpl::ReceiveImage(unsigned char *pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)
{
	return spout->ReceiveImage(pixels, glFormat, bInvert, HostFbo);
}

bool SPOUTImpl::IsUpdated()
{
	return spout->IsUpdated();
}

bool SPOUTImpl::IsConnected()
{
	return spout->IsConnected();
}

bool SPOUTImpl::IsFrameNew()
{
	return spout->IsFrameNew();
}

const char * SPOUTImpl::GetSenderName()
{
	return spout->GetSenderName();
}

unsigned int SPOUTImpl::GetSenderWidth()
{
	return spout->GetSenderWidth();
}

unsigned int SPOUTImpl::GetSenderHeight()
{
	return spout->GetSenderHeight();
}

DWORD SPOUTImpl::GetSenderFormat()
{
	return spout->GetSenderFormat();
}

double SPOUTImpl::GetSenderFps()
{
	return spout->GetSenderFps();
}

long SPOUTImpl::GetSenderFrame()
{
	return spout->GetSenderFrame();
}

HANDLE SPOUTImpl::GetSenderHandle()
{
	return spout->GetSenderHandle();
}

bool SPOUTImpl::GetSenderCPUmode()
{
	return spout->GetSenderCPUmode();
}

void SPOUTImpl::SelectSender()
{
	spout->SelectSender();
}

//
// Frame count
//

void SPOUTImpl::SetFrameCount(bool bEnable)
{
	spout->SetFrameCount(bEnable);
}


void SPOUTImpl::DisableFrameCount()
{
	spout->DisableFrameCount();
}

bool SPOUTImpl::IsFrameCountEnabled()
{
	return spout->IsFrameCountEnabled();
}

void SPOUTImpl::HoldFps(int fps)
{
	return spout->HoldFps(fps);
}

//
// ======================= Logging and registry utilities =======================
//
// These functions use the spoututils namespace directly
//

// Logging

void SPOUTImpl::OpenSpoutConsole()
{
	spoututils::OpenSpoutConsole();
}

void SPOUTImpl::CloseSpoutConsole(bool bWarning)
{
	spoututils::CloseSpoutConsole(bWarning);
}

void SPOUTImpl::EnableSpoutLog()
{
	spoututils::EnableSpoutLog();
}

void SPOUTImpl::EnableSpoutLogFile(const char* filename, bool append)
{
	spoututils::EnableSpoutLogFile(filename, append);
}

std::string SPOUTImpl::GetSpoutLog()
{
	return spoututils::GetSpoutLog();
}

void SPOUTImpl::ShowSpoutLogs()
{
	spoututils::ShowSpoutLogs();
}

void SPOUTImpl::DisableSpoutLog()
{
	spoututils::DisableSpoutLog();
}

void SPOUTImpl::SetSpoutLogLevel(LogLevel level)
{
	spoututils::SetSpoutLogLevel((SpoutLogLevel)level);
}

void SPOUTImpl::SpoutLog(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::SpoutLog(format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogVerbose(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_VERBOSE, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogNotice(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_NOTICE, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_WARNING, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_ERROR, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogFatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_FATAL, format, args);
	va_end(args);
}

int SPOUTImpl::SpoutMessageBox(const char * message, DWORD dwMilliseconds)
{
	return spoututils::SpoutMessageBox(message, dwMilliseconds);
}

int SPOUTImpl::SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds)
{
	return spoututils::SpoutMessageBox(hwnd, message, caption, uType, dwMilliseconds);
}

// Registry utilities

bool SPOUTImpl::ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue)
{
	return spoututils::ReadDwordFromRegistry(hKey, subkey, valuename, pValue);
}

bool SPOUTImpl::WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue)
{
	return spoututils::WriteDwordToRegistry(hKey, subkey, valuename, dwValue);
}

bool SPOUTImpl::ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath)
{
	return spoututils::ReadPathFromRegistry(hKey, subkey, valuename, filepath);
}

bool SPOUTImpl::WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath)
{
	return spoututils::WritePathToRegistry(hKey, subkey, valuename, filepath);
}

bool SPOUTImpl::RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename)
{
	return spoututils::RemovePathFromRegistry(hKey, subkey, valuename);
}

bool SPOUTImpl::RemoveSubKey(HKEY hKey, const char *subkey)
{
	return spoututils::RemoveSubKey(hKey, subkey);
}

bool SPOUTImpl::FindSubKey(HKEY hKey, const char *subkey)
{
	return spoututils::FindSubKey(hKey, subkey);
}

bool SPOUTImpl::IsInitialized()
{
	return spout->IsSpoutInitialized();
}

bool SPOUTImpl::BindSharedTexture()
{
	return spout->BindSharedTexture();
}

bool SPOUTImpl::UnBindSharedTexture()
{
	return spout->UnBindSharedTexture();
}

GLuint SPOUTImpl::GetSharedTextureID()
{
	return spout->GetSharedTextureID();
}

int  SPOUTImpl::GetSenderCount()
{
	return spout->GetSenderCount();
}

bool SPOUTImpl::GetSender(int index, char* sendername, int MaxSize)
{
	return spout->GetSender(index, sendername, MaxSize);
}

bool SPOUTImpl::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spout->GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

bool SPOUTImpl::GetActiveSender(char* Sendername)
{
	return spout->GetActiveSender(Sendername);
}

bool SPOUTImpl::SetActiveSender(const char* Sendername)
{
	return spout->SetActiveSender(Sendername);
}

bool SPOUTImpl::GetBufferMode()
{
	return spout->GetBufferMode();
}

void SPOUTImpl::SetBufferMode(bool bActive)
{
	spout->SetBufferMode(bActive);
}

int SPOUTImpl::GetBuffers()
{
	return spout->GetBuffers();
}

void SPOUTImpl::SetBuffers(int nBuffers)
{
	spout->SetBuffers(nBuffers);
}

int  SPOUTImpl::GetMaxSenders()
{
	return spout->GetMaxSenders();
}

void SPOUTImpl::SetMaxSenders(int maxSenders)
{
	spout->SetMaxSenders(maxSenders);
}


//
// For 2.006 compatibility
//

//
// ======================= 2.006 and earlier =======================
//

//
// Sender
//
bool SPOUTImpl::CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	return spout->CreateSender(Sendername, width, height, dwFormat);
}

bool SPOUTImpl::UpdateSender(const char* Sendername, unsigned int width, unsigned int height)
{
	return spout->UpdateSender(Sendername, width, height);
}

// 
// Receiver
//
bool SPOUTImpl::CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spout->CreateReceiver(Sendername, width, height, bUseActive);
}

bool SPOUTImpl::CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spout->CheckReceiver(Sendername, width, height, bConnected);
}

bool SPOUTImpl::GetDX9()
{
	return spout->GetDX9();
}

bool SPOUTImpl::SetDX9(bool bDX9)
{
	return spout->SetDX9(bDX9);
}

bool SPOUTImpl::GetMemoryShareMode()
{
	return spout->GetMemoryShareMode();
}

bool SPOUTImpl::SetMemoryShareMode(bool bMem)
{
	return spout->SetMemoryShareMode(bMem);
}

bool SPOUTImpl::GetCPUmode()
{
	return spout->GetCPUmode();
}

bool SPOUTImpl::SetCPUmode(bool bCPU)
{
	return spout->SetCPUmode(bCPU);
}

int SPOUTImpl::GetShareMode()
{
	return spout->GetShareMode();
}

void SPOUTImpl::SetShareMode(int mode)
{
	spout->SetShareMode(mode);
}

//
// Information
//

bool SPOUTImpl::GetHostPath(const char *sendername, char *hostpath, int maxchars)
{
	return spout->GetHostPath(sendername, hostpath, maxchars);
}

int SPOUTImpl::GetVerticalSync()
{
	return spout->GetVerticalSync();
}

bool SPOUTImpl::SetVerticalSync(bool bSync)
{
	return spout->SetVerticalSync(bSync);
}

int SPOUTImpl::GetSpoutVersion()
{
	return spout->GetSpoutVersion();
}


//
// Graphics compatibility
//

bool SPOUTImpl::GetAutoShare()
{
	return spout->GetAutoShare();
}

void SPOUTImpl::SetAutoShare(bool bAuto)
{
	spout->SetAutoShare(bAuto);
}

bool SPOUTImpl::IsGLDXready()
{
	return spout->IsGLDXready();
}

//
// Adapter functions
//

int SPOUTImpl::GetNumAdapters()
{
	return spout->GetNumAdapters();
}

bool SPOUTImpl::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spout->GetAdapterName(index, adaptername, maxchars);
}

int SPOUTImpl::GetAdapter()
{
	return spout->GetAdapter();
}

bool SPOUTImpl::SetAdapter(int index)
{
	return spout->SetAdapter(index);
}

int SPOUTImpl::Adapter()
{
	return spout->Adapter();
}

char * SPOUTImpl::AdapterName()
{
	return spout->AdapterName();
}

//
// OpenGL utilities
//

bool SPOUTImpl::CreateOpenGL()
{
	return spout->CreateOpenGL();
}

bool SPOUTImpl::CloseOpenGL()
{
	return spout->CloseOpenGL();
}

bool SPOUTImpl::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	return spout->CopyTexture(SourceID, SourceTarget, DestID, DestTarget,
								width, height, bInvert, HostFBO);
}

//
// Class function
//

void SPOUTImpl::Release()
{
	// Delete Spout object
	delete(spout);
	spout = nullptr;

	// Delete this class instance
	delete this;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the SPOUT object.
//
// Export both decorated and undecorated names. For example :
//		 GetSpout    - Undecorated name, which can be easily used with GetProcAddress
//                     Win32 API function.
//		_GetSpout@0  - Common name decoration for __stdcall functions in C language.
//
// For more information on name decoration see here:
// "Format of a C Decorated Name"
// http://msdn.microsoft.com/en-us/library/x7kb4e2f.aspx

#if !defined(_WIN64)
// This pragma is required only for 32-bit builds.
// In a 64-bit environment, C functions are not decorated.
#pragma comment(linker, "/export:GetSpout=_GetSpout@0")
#endif  // _WIN64

extern "C" SPOUTAPI SPOUTHANDLE APIENTRY GetSpout()
{
	// The Spout class implementation
	SPOUTImpl * pSpout = new SPOUTImpl;

	// Create a new spout sender pointer for this class
	pSpout->spout = new Spout;

	return pSpout;
}

////////////////////////////////////////////////////////////////////////////////
