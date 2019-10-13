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
//				   Re-build for 32 bit and 64 bit - VS2017 /MT
//
/*
		Copyright (c) 2016-2019, Lynn Jarvis. All rights reserved.

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

#include "SpoutLibrary.h"
#include <stdio.h>
#include "SpoutFunctions.h"

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the SPOUT interface.
//

class SPOUTImpl : public SPOUTLIBRARY
{
	public :

		SpoutFunctions * spoutSDK; // Spout SDK functions object for this class

	private : // Spout SDK functions

		//
		// New for 2.007
		//

		// Sender
		bool SetupSender(const char* SenderName, unsigned int width, unsigned int height, bool bInvert = true, DWORD dwFormat = 0);
		bool IsInitialized();
		bool SendTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo = 0);
		bool SendFboData(GLuint FboID);
		bool SendImageData(const unsigned char* pixels, GLenum glFormat = GL_RGBA, GLuint HostFbo = 0);
		unsigned int GetWidth();
		unsigned int GetHeight();
		long GetFrame();
		double GetFps();
		void HoldFps(int fps);

		// Receiver
		void SetupReceiver(unsigned int width, unsigned int height, bool bInvert = false);
		void SetReceiverName(const char * SenderName);
		bool ReceiveTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo = 0);
		bool ReceiveImageData(unsigned char *pixels, GLenum glFormat = GL_RGBA, GLuint HostFbo = 0);
		bool IsUpdated();
		bool IsConnected();
		void SelectSender();
		const char * GetSenderName();
		unsigned int GetSenderWidth();
		unsigned int GetSenderHeight();
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
		// 2.006 and earlier
		//

		// Sender
		bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
		bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height);
		bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
		bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert=false);
		void ReleaseSender(DWORD dwMsec = 0);

		// Receiver
		bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive = false);
		void ReleaseReceiver();
		bool ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFBO = 0);
		bool SelectSenderPanel(const char* message = NULL);
		bool ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);
		bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);

		bool BindSharedTexture();
		bool UnBindSharedTexture();
	
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
		int  GetShareMode();
		bool SetShareMode(int mode);
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

		//
		// Release the class instance
		//
		void Release();

};


// ---------------------------------------------------------------
// 2.007
//

//
// Sender
//

bool SPOUTImpl::SetupSender(const char* sendername, unsigned int width, unsigned int height, bool bInvert, DWORD dwFormat)
{
	return spoutSDK->SetupSender(sendername, width, height, bInvert, dwFormat);
}

bool SPOUTImpl::IsInitialized()
{
	return spoutSDK->IsInitialized();
}

bool SPOUTImpl::SendTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo)
{
	return spoutSDK->SendTextureData(TextureID, TextureTarget, HostFbo);
}

bool SPOUTImpl::SendFboData(GLuint FboID)
{
	return spoutSDK->SendFboData(FboID);
}

bool SPOUTImpl::SendImageData(const unsigned char* pixels, GLenum glFormat, GLuint HostFbo)
{
	return spoutSDK->SendImageData(pixels, glFormat, HostFbo);
}

unsigned int SPOUTImpl::GetWidth()
{
	return spoutSDK->GetWidth();
}

unsigned int SPOUTImpl::GetHeight()
{
	return spoutSDK->GetHeight();
}

long  SPOUTImpl::GetFrame()
{
	return spoutSDK->GetFrame();
}

double SPOUTImpl::GetFps()
{
	return spoutSDK->GetFps();
}

void SPOUTImpl::HoldFps(int fps)
{
	return spoutSDK->HoldFps(fps);
}

//
// Receiver
//

void SPOUTImpl::SetupReceiver(unsigned int width, unsigned int height, bool bInvert)
{
	spoutSDK->SetupReceiver(width, height, bInvert);
}

void SPOUTImpl::SetReceiverName(const char* SenderName)
{
	spoutSDK->SetReceiverName(SenderName);
}

bool SPOUTImpl::ReceiveTextureData(GLuint TextureID, GLuint TextureTarget, GLuint HostFbo)
{
	return spoutSDK->ReceiveTextureData(TextureID, TextureTarget, HostFbo);
}

bool SPOUTImpl::ReceiveImageData(unsigned char *pixels, GLenum glFormat, GLuint HostFbo)
{
	return spoutSDK->ReceiveImageData(pixels, glFormat, HostFbo);
}

bool SPOUTImpl::IsUpdated()
{
	return spoutSDK->IsUpdated();
}

bool SPOUTImpl::IsConnected()
{
	return spoutSDK->IsConnected();
}

void SPOUTImpl::SelectSender()
{
	spoutSDK->SelectSender();
}

const char * SPOUTImpl::GetSenderName()
{
	return spoutSDK->GetSenderName();
}

unsigned int SPOUTImpl::GetSenderWidth()
{
	return spoutSDK->GetSenderWidth();
}

unsigned int SPOUTImpl::GetSenderHeight()
{
	return spoutSDK->GetSenderHeight();
}

long SPOUTImpl::GetSenderFrame()
{
	return spoutSDK->GetSenderFrame();
}

double SPOUTImpl::GetSenderFps()
{
	return spoutSDK->GetSenderFps();
}

bool SPOUTImpl::IsFrameNew()
{
	return spoutSDK->IsFrameNew();
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
	return spoutSDK->CreateSender(Sendername, width, height, dwFormat);
}

bool SPOUTImpl::UpdateSender(const char* Sendername, unsigned int width, unsigned int height)
{
	return spoutSDK->UpdateSender(Sendername, width, height);
}

bool SPOUTImpl::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

bool SPOUTImpl::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	return spoutSDK->SendImage(pixels, width, height, glFormat, bInvert);
}

void SPOUTImpl::ReleaseSender(DWORD dwMsec)
{
	spoutSDK->ReleaseSender(dwMsec);
}

// 
// Receiver
//
bool SPOUTImpl::CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spoutSDK->CreateReceiver(Sendername, width, height, bUseActive);
}

void SPOUTImpl::ReleaseReceiver()
{
	spoutSDK->ReleaseReceiver();
}

bool SPOUTImpl::ReceiveTexture(char* Sendername, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->ReceiveTexture(Sendername, width, height, TextureID, TextureTarget, bInvert, HostFBO);
}


bool SPOUTImpl::SelectSenderPanel(const char* message)
{
	return spoutSDK->SelectSenderPanel(message);
}

bool SPOUTImpl::ReceiveImage(char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->ReceiveImage(Sendername, width, height, pixels, glFormat, bInvert, HostFBO);
}

bool SPOUTImpl::CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spoutSDK->CheckReceiver(Sendername, width, height, bConnected);
}

bool SPOUTImpl::BindSharedTexture()
{
	return spoutSDK->BindSharedTexture();
}

bool SPOUTImpl::UnBindSharedTexture()
{
	return spoutSDK->UnBindSharedTexture();
}

int  SPOUTImpl::GetSenderCount()
{
	return spoutSDK->GetSenderCount();
}

bool SPOUTImpl::GetSender(int index, char* sendername, int MaxSize)
{
	return spoutSDK->GetSender(index, sendername, MaxSize);
}

bool SPOUTImpl::GetSenderInfo  (const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spoutSDK->GetSenderInfo (sendername, width, height, dxShareHandle, dwFormat);
}

bool SPOUTImpl::GetActiveSender(char* Sendername)
{
	return spoutSDK->GetActiveSender(Sendername);
}

bool SPOUTImpl::SetActiveSender(const char* Sendername)
{
	return spoutSDK->SetActiveSender(Sendername);
}

// Utilities
bool SPOUTImpl::SetDX9(bool bDX9)
{
	return spoutSDK->SetDX9(bDX9);
}

bool SPOUTImpl::GetDX9()
{
	return spoutSDK->GetDX9();
}

bool SPOUTImpl::SetMemoryShareMode(bool bMem)
{
	return spoutSDK->SetMemoryShareMode(bMem);
}

bool SPOUTImpl::GetMemoryShareMode()
{
	return spoutSDK->GetMemoryShareMode();
}

int SPOUTImpl::GetShareMode()
{
	return spoutSDK->GetShareMode();
}

bool SPOUTImpl::SetShareMode(int mode)
{
	return spoutSDK->SetShareMode(mode);
}

void SPOUTImpl::SetBufferMode(bool bActive)
{
	spoutSDK->SetBufferMode(bActive);
}

bool SPOUTImpl::GetBufferMode()
{
	return spoutSDK->GetBufferMode();
}


int  SPOUTImpl::GetMaxSenders()
{
	return spoutSDK->GetMaxSenders();
}

void SPOUTImpl::SetMaxSenders(int maxSenders)
{
	spoutSDK->SetMaxSenders(maxSenders);
}

bool SPOUTImpl::GetHostPath(const char *sendername, char *hostpath, int maxchars)
{
	return spoutSDK->GetHostPath(sendername, hostpath, maxchars);
}

int  SPOUTImpl::GetVerticalSync()
{
	return spoutSDK->GetVerticalSync();
}

bool SPOUTImpl::SetVerticalSync(bool bSync)
{
	return spoutSDK->SetVerticalSync(bSync);
}

// Adapter functions
int  SPOUTImpl::GetNumAdapters()
{
	return spoutSDK->GetNumAdapters();
}

bool SPOUTImpl::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spoutSDK->GetAdapterName(index, adaptername, maxchars);
}

bool SPOUTImpl::SetAdapter(int index)
{
	return spoutSDK->SetAdapter(index);
}

int  SPOUTImpl::GetAdapter()
{
	return spoutSDK->GetAdapter();
}

bool SPOUTImpl::CreateOpenGL()
{
	return spoutSDK->spout.interop.CreateOpenGL();
}

bool SPOUTImpl::CloseOpenGL()
{
	return spoutSDK->spout.interop.CloseOpenGL();
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
	// pSpout->spoutSDK = new Spout;
	pSpout->spoutSDK = new SpoutFunctions;
	
	return pSpout;
}

////////////////////////////////////////////////////////////////////////////////