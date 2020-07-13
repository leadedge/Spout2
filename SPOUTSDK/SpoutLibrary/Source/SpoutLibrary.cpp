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
//				   Re-build 32/64 bit - VS2017 / MT
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

#include <stdio.h>
#include "SpoutLibrary.h"
#include "..\..\SpoutSDK\spout.h"

using namespace spoututils;

class SpoutFunctions {

public:

	SpoutFunctions();
	~SpoutFunctions();

	SpoutSender spoutsender;
	SpoutReceiver spoutreceiver;
	Spout spout; // common functions

};

SpoutFunctions::SpoutFunctions() {

};

SpoutFunctions::~SpoutFunctions() {
	spoutsender.ReleaseSender();
	spoutreceiver.ReleaseReceiver();
};


////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the SPOUT interface.
//

class SPOUTImpl : public SPOUTLIBRARY
{
	public :

		SpoutFunctions * spoutSDK; // Spout SDK functions object for this class

		// LJ DEBUG
		SPOUTImpl();
		~SPOUTImpl();

	private : // Spout SDK functions

		//
		// 2.007
		//

		// Sender
		void SetSenderName(const char* sendername = nullptr);
		void ReleaseSender(DWORD dwMsec = 0);
		bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
		bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
		bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
		const char * GetName();
		unsigned int GetWidth();
		unsigned int GetHeight();
		long GetFrame();
		double GetFps();
		void HoldFps(int fps);

		// Receiver
		void SetReceiverName(const char * SenderName);
		void ReleaseReceiver();
		bool ReceiveTexture(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0);
		bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0);
		bool IsUpdated();
		bool IsConnected();
		void SelectSender();
		const char * GetSenderName();
		unsigned int GetSenderWidth();
		unsigned int GetSenderHeight();
		DWORD GetSenderFormat();
		double GetSenderFps();
		long GetSenderFrame();
		bool IsFrameNew();

		// Common
		void DisableFrameCount();
		bool IsFrameCountEnabled();

		// DX9 application support
		bool SetDX9device(IDirect3DDevice9Ex* pDevice);
		bool WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface);

		// Log utilities
		void OpenSpoutConsole();
		void CloseSpoutConsole(bool bWarning = false);
		void EnableSpoutLog();
		void EnableSpoutLogFile(const char *filename, bool append = false);
		std::string GetSpoutLog();
		void ShowSpoutLogs();
		void DisableSpoutLog();
		void SetSpoutLogLevel(LogLevel level);
		void SpoutLog(const char* format, ...);
		void SpoutLogVerbose(const char* format, ...);
		void SpoutLogNotice(const char* format, ...);
		void SpoutLogWarning(const char* format, ...);
		void SpoutLogError(const char* format, ...);
		void SpoutLogFatal(const char* format, ...);
		int SpoutMessageBox(const char * message, DWORD dwMilliseconds = 0);
		int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0);

		// Registry utilities
		bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue);
		bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue);
		bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath);
		bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath);
		bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename);
		bool RemoveSubKey(HKEY hKey, const char *subkey);
		bool FindSubKey(HKEY hKey, const char *subkey);

		//
		// 2.006  compatibility
		//

		// Sender
		bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
		bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height);

		// Receiver
		bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false);
		bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);

		bool IsInitialized();
		bool BindSharedTexture();
		bool UnBindSharedTexture();
		GLuint GetSharedTextureID();

		int  GetSenderCount();
		bool GetSender(int index, char* sendername, int MaxSize = 256);
		bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
		bool GetActiveSender(char* Sendername);
		bool SetActiveSender(const char* Sendername);
	
		// Utilities
		bool SetDX9(bool bDX9 = true); // User request to use DirectX 9 (default is DirectX 11)
		bool GetDX9(); // Return the flag that has been set
		bool SetMemoryShareMode(bool bMem = true);
		bool GetMemoryShareMode();
		void SetBufferMode(bool bActive); // Set the pbo availability on or off
		bool GetBufferMode();

		int  GetMaxSenders(); // Get maximum senders allowed
		void SetMaxSenders(int maxSenders); // Set maximum senders allowed
		bool GetHostPath(const char *sendername, char *hostpath, int maxchars); // The path of the host that produced the sender
		int  GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);

		// Adapter functions
		int  GetNumAdapters(); // Get the number of graphics adapters in the system
		bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
		bool SetAdapter(int index = 0); // Set required graphics adapter for output
		int  GetAdapter(); // Get the SpoutDirectX global adapter index

		// OpenGL
		bool CreateOpenGL();
		bool CloseOpenGL();
		bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
			GLuint DestID, GLuint DestTarget,
			unsigned int width, unsigned int height,
			bool bInvert = false, GLuint HostFBO = 0);

		//
		// Release the class instance
		//
		void Release();

};

// LJ DEBUG
SPOUTImpl::SPOUTImpl() {
	MessageBoxA(NULL, "SPOUTImpl", "Message", MB_OK);
};

SPOUTImpl::~SPOUTImpl() {
	MessageBoxA(NULL, "~SPOUTImpl", "Message", MB_OK);
};


// ---------------------------------------------------------------
// 2.007
//

//
// Sender
//

void SPOUTImpl::SetSenderName(const char* sendername)
{
	spoutSDK->spoutsender.SetSenderName(sendername);
}

void SPOUTImpl::ReleaseSender(DWORD dwMsec)
{
	dwMsec = 0; // Not used for 2.007
	spoutSDK->spoutsender.ReleaseSender();
}

bool SPOUTImpl::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->spoutsender.SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

bool SPOUTImpl::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	return spoutSDK->spoutsender.SendFbo(FboID, width, height, bInvert);
}

bool SPOUTImpl::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	return spoutSDK->spoutsender.SendImage(pixels, width, height, glFormat, bInvert);
}

const char * SPOUTImpl::GetName()
{
	return spoutSDK->spoutsender.GetName();
}

unsigned int SPOUTImpl::GetWidth()
{
	return spoutSDK->spoutsender.GetWidth();
}

unsigned int SPOUTImpl::GetHeight()
{
	return spoutSDK->spoutsender.GetHeight();
}

long  SPOUTImpl::GetFrame()
{
	return spoutSDK->spoutsender.GetFrame();
}

double SPOUTImpl::GetFps()
{
	return spoutSDK->spoutsender.GetFps();
}

void SPOUTImpl::HoldFps(int fps)
{
	return spoutSDK->spoutsender.HoldFps(fps);
}

//
// Receiver
//

void SPOUTImpl::SetReceiverName(const char* SenderName)
{
	spoutSDK->spoutreceiver.SetReceiverName(SenderName);
}

void SPOUTImpl::ReleaseReceiver()
{
	spoutSDK->spoutreceiver.ReleaseReceiver();
}

bool SPOUTImpl::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
{
	return spoutSDK->spoutreceiver.ReceiveTexture(TextureID, TextureTarget, bInvert, HostFbo);
}

bool SPOUTImpl::ReceiveImage(unsigned char *pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)
{
	return spoutSDK->spoutreceiver.ReceiveImage(pixels, glFormat, bInvert, HostFbo);
}

bool SPOUTImpl::IsUpdated()
{
	return spoutSDK->spoutreceiver.IsUpdated();
}

bool SPOUTImpl::IsConnected()
{
	return spoutSDK->spoutreceiver.IsConnected();
}

void SPOUTImpl::SelectSender()
{
	spoutSDK->spoutreceiver.SelectSender();
}

const char * SPOUTImpl::GetSenderName()
{
	return spoutSDK->spoutreceiver.GetSenderName();
}

unsigned int SPOUTImpl::GetSenderWidth()
{
	return spoutSDK->spoutreceiver.GetSenderWidth();
}

unsigned int SPOUTImpl::GetSenderHeight()
{
	return spoutSDK->spoutreceiver.GetSenderHeight();
}

DWORD SPOUTImpl::GetSenderFormat()
{
	return spoutSDK->spoutreceiver.GetSenderFormat();
}

long SPOUTImpl::GetSenderFrame()
{
	return spoutSDK->spoutreceiver.GetSenderFrame();
}

double SPOUTImpl::GetSenderFps()
{
	return spoutSDK->spoutreceiver.GetSenderFps();
}

bool SPOUTImpl::IsFrameNew()
{
	return spoutSDK->spoutreceiver.IsFrameNew();
}

// Common

void SPOUTImpl::DisableFrameCount()
{
	spoutSDK->spout.interop.frame.DisableFrameCount();
}

bool SPOUTImpl::IsFrameCountEnabled()
{
	return spoutSDK->spout.interop.frame.IsFrameCountEnabled();
}

// Support for DirectX9 applications

bool SPOUTImpl::SetDX9device(IDirect3DDevice9Ex* pDevice)
{
	return  spoutSDK->spout.interop.SetDX9device(pDevice);
}

bool SPOUTImpl::WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface)
{
	return spoutSDK->spout.interop.WriteDX9surface(pDevice, surface);
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


//
// ======================= 2.006 and earlier =======================
//

//
// Sender
//
bool SPOUTImpl::CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	return spoutSDK->spoutsender.CreateSender(Sendername, width, height, dwFormat);
}

bool SPOUTImpl::UpdateSender(const char* Sendername, unsigned int width, unsigned int height)
{
	return spoutSDK->spoutsender.UpdateSender(Sendername, width, height);
}

// 
// Receiver
//
bool SPOUTImpl::CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spoutSDK->spoutreceiver.CreateReceiver(Sendername, width, height, bUseActive);
}

bool SPOUTImpl::CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spoutSDK->spoutreceiver.CheckReceiver(Sendername, width, height, bConnected);
}

bool SPOUTImpl::IsInitialized()
{
	return spoutSDK->spout.IsSpoutInitialized();
}

bool SPOUTImpl::BindSharedTexture()
{
	return spoutSDK->spout.BindSharedTexture();
}

bool SPOUTImpl::UnBindSharedTexture()
{
	return spoutSDK->spout.UnBindSharedTexture();
}

GLuint SPOUTImpl::GetSharedTextureID()
{
	return spoutSDK->spoutreceiver.GetSharedTextureID();
}

int  SPOUTImpl::GetSenderCount()
{
	return spoutSDK->spoutreceiver.GetSenderCount();
}

bool SPOUTImpl::GetSender(int index, char* sendername, int MaxSize)
{
	return spoutSDK->spoutreceiver.GetSender(index, sendername, MaxSize);
}

bool SPOUTImpl::GetSenderInfo  (const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spoutSDK->spoutreceiver.GetSenderInfo (sendername, width, height, dxShareHandle, dwFormat);
}

bool SPOUTImpl::GetActiveSender(char* Sendername)
{
	return spoutSDK->spoutreceiver.GetActiveSender(Sendername);
}

bool SPOUTImpl::SetActiveSender(const char* Sendername)
{
	return spoutSDK->spoutreceiver.SetActiveSender(Sendername);
}

// Utilities
bool SPOUTImpl::SetDX9(bool bDX9)
{
	return spoutSDK->spout.SetDX9(bDX9);
}

bool SPOUTImpl::GetDX9()
{
	return spoutSDK->spout.GetDX9();
}

bool SPOUTImpl::SetMemoryShareMode(bool bMem)
{
	return spoutSDK->spout.SetMemoryShareMode(bMem);
}

bool SPOUTImpl::GetMemoryShareMode()
{
	return spoutSDK->spout.GetMemoryShareMode();
}

void SPOUTImpl::SetBufferMode(bool bActive)
{
	spoutSDK->spout.SetBufferMode(bActive);
}

bool SPOUTImpl::GetBufferMode()
{
	return spoutSDK->spout.GetBufferMode();
}


int  SPOUTImpl::GetMaxSenders()
{
	return spoutSDK->spout.GetMaxSenders();
}

void SPOUTImpl::SetMaxSenders(int maxSenders)
{
	spoutSDK->spout.SetMaxSenders(maxSenders);
}

bool SPOUTImpl::GetHostPath(const char *sendername, char *hostpath, int maxchars)
{
	return spoutSDK->spout.GetHostPath(sendername, hostpath, maxchars);
}

int  SPOUTImpl::GetVerticalSync()
{
	return spoutSDK->spout.GetVerticalSync();
}

bool SPOUTImpl::SetVerticalSync(bool bSync)
{
	return spoutSDK->spout.SetVerticalSync(bSync);
}

// Adapter functions
int  SPOUTImpl::GetNumAdapters()
{
	return spoutSDK->spout.GetNumAdapters();
}

bool SPOUTImpl::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spoutSDK->spout.GetAdapterName(index, adaptername, maxchars);
}

bool SPOUTImpl::SetAdapter(int index)
{
	return spoutSDK->spout.SetAdapter(index);
}

int  SPOUTImpl::GetAdapter()
{
	return spoutSDK->spout.GetAdapter();
}

// OpenGL utilities
bool SPOUTImpl::CreateOpenGL()
{
	return spoutSDK->spout.interop.CreateOpenGL();
}

bool SPOUTImpl::CloseOpenGL()
{
	return spoutSDK->spout.interop.CloseOpenGL();
}

bool SPOUTImpl::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	return spoutSDK->spout.interop.CopyTexture(SourceID, SourceTarget, DestID, DestTarget,
		width, height, bInvert, HostFBO);
}



// Class function
void SPOUTImpl::Release()
{
	// Delete the Spout SDK object
	if(spoutSDK) delete(spoutSDK);

	// Delete this class instance
    delete this;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the SPOUT object.
//
// Export both decorated and undecorated names.
//		GetSpout     - Undecorated name, which can be easily used with GetProcAddress
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

	SPOUTImpl * pSpout = new SPOUTImpl; // the Spout class implementation

	// Create a new spout SDK pointer for this class
	pSpout->spoutSDK = new SpoutFunctions;
	
	return pSpout;
}

////////////////////////////////////////////////////////////////////////////////