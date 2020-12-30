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
//				   Re-build 32/64 bit - VS2017 / MT
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

class SPOUTImpl : public SPOUTLIBRARY
{

public:

	Spout * spout; // Spout SDK functions object for this class

private: // Spout SDK functions

	//
	// Sender
	//

	void SetSenderName(const char* sendername = nullptr);
	void SetSenderFormat(DWORD dwFormat);
	void ReleaseSender(DWORD dwMsec = 0);
	bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true);
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0);
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false);
	const char * GetName();
	unsigned int GetWidth();
	unsigned int GetHeight();
	double GetFps();
	long GetFrame();
	HANDLE GetHandle();

	//
	// Receiver
	//

	void SetReceiverName(const char * SenderName);
	void ReleaseReceiver();
	bool ReceiveTexture(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0);
	bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0);
	bool IsUpdated();
	bool IsConnected();
	bool IsFrameNew();
	const char * GetSenderName();
	unsigned int GetSenderWidth();
	unsigned int GetSenderHeight();
	DWORD GetSenderFormat();
	double GetSenderFps();
	long GetSenderFrame();
	HANDLE GetSenderHandle();
	bool GetSenderCPUmode();
	void SelectSender();

	//
	// Frame counting
	//

	void SetFrameCount(bool bEnable);
	void DisableFrameCount();
	bool IsFrameCountEnabled();
	void HoldFps(int fps);

	//
	// Log utilities
	//

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

	//
	// Registry utilities
	//

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

	//
	// User registry settings recorded by "SpoutSettings"
	//

	bool GetBufferMode();
	void SetBufferMode(bool bActive = true);
	int GetBuffers();
	void SetBuffers(int nBuffers);
	int GetMaxSenders();
	void SetMaxSenders(int maxSenders);
	
	//
	// 2.006 compatibility
	//

	bool GetDX9();
	bool SetDX9(bool bDX9 = true);
	bool GetMemoryShareMode();
	bool SetMemoryShareMode(bool bMem = true);
	bool GetCPUmode();
	bool SetCPUmode(bool bCPU);
	int GetShareMode();
	void SetShareMode(int mode);

	//
	// Information
	//

	bool GetHostPath(const char *sendername, char *hostpath, int maxchars);
	int  GetVerticalSync();
	bool SetVerticalSync(bool bSync = true);
	int GetSpoutVersion();

	//
	// Graphics compatibility
	//

	// Get auto GPU/CPU share depending on compatibility
	bool GetAutoShare();
	// Set auto GPU/CPU share depending on compatibility
	void SetAutoShare(bool bAuto = true);
	// OpenGL texture share compatibility
	bool IsGLDXready();

	//
	// Adapter functions
	//

	int GetNumAdapters();
	bool GetAdapterName(int index, char *adaptername, int maxchars);
	int GetAdapter();
	bool SetAdapter(int index = 0);
	int Adapter();
	char * AdapterName();

	//
	// OpenGL utilities
	//

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
