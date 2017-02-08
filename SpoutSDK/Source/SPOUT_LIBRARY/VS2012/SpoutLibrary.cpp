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
//
//
/*
		Copyright (c) 2016-2017, Lynn Jarvis. All rights reserved.

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
#include "..\..\SpoutSDK3\Spout.h"

////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the SPOUT interface.
//

class SPOUTImpl : public SPOUTLIBRARY
{
	public :

		Spout * spoutSDK; // Spout SDK object for this class

	private : // Spout SDK functions

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
		bool ReceiveImage   (char* Sendername, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFBO=0);
		bool CheckReceiver	(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected);
		bool GetImageSize   (char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode);	

		bool BindSharedTexture();
		bool UnBindSharedTexture();
	
		bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true, GLuint HostFBO = 0);
		bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);

		int  GetSenderCount ();
		bool GetSenderName  (int index, char* sendername, int MaxSize = 256);
		bool GetSenderInfo  (const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
		bool GetActiveSender(char* Sendername);
		bool SetActiveSender(const char* Sendername);
	
		// Utilities
		bool SetDX9(bool bDX9 = true); // User request to use DirectX 9 (default is DirectX 11)
		bool GetDX9(); // Return the flag that has been set
		bool SetMemoryShareMode(bool bMem = true);
		bool GetMemoryShareMode();
		bool SetCPUmode(bool bCPU = true);
		bool GetCPUmode();
		int  GetShareMode();
		bool SetShareMode(int mode);
		void SetBufferMode(bool bActive); // Set the pbo availability on or off
		bool GetBufferMode();

		int  GetMaxSenders(); // Get maximum senders allowed
		void SetMaxSenders(int maxSenders); // Set maximum senders allowed
		bool GetHostPath(const char *sendername, char *hostpath, int maxchars); // The path of the host that produced the sender
		int  GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);

		// Access to globals
		bool GetSpoutSenderName(char * sendername, int maxchars); // get the global sender name
		bool IsSpoutInitialized(); // has the class been initialized
		
		// Adapter functions
		int  GetNumAdapters(); // Get the number of graphics adapters in the system
		bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
		bool SetAdapter(int index = 0); // Set required graphics adapter for output
		int  GetAdapter(); // Get the SpoutDirectX global adapter index

		//
		// Release the class instance
		//
		void Release();

};

//
// Sender
//
bool SPOUTImpl::CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	return spoutSDK->CreateSender(Sendername, width, height, dwFormat);
}

bool SPOUTImpl::UpdateSender(const char* Sendername, unsigned int width, unsigned int height)
{
	return spoutSDK->CreateSender(Sendername, width, height);
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

bool SPOUTImpl::GetImageSize(char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode)	
{
	return spoutSDK->GetImageSize(sendername, width, height, bMemoryMode);
}

bool SPOUTImpl::BindSharedTexture()
{
	return spoutSDK->BindSharedTexture();
}

bool SPOUTImpl::UnBindSharedTexture()
{
	return spoutSDK->UnBindSharedTexture();
}

bool SPOUTImpl::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->DrawSharedTexture(max_x, max_y, aspect, bInvert, HostFBO);
}

bool SPOUTImpl::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	return spoutSDK->DrawToSharedTexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO);
}

int  SPOUTImpl::GetSenderCount()
{
	return spoutSDK->GetSenderCount();
}

bool SPOUTImpl::GetSenderName(int index, char* sendername, int MaxSize)
{
	return spoutSDK->GetSenderName(index, sendername, MaxSize);
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


bool SPOUTImpl::SetCPUmode(bool bCPU)
{
	return spoutSDK->SetCPUmode(bCPU);
}

bool SPOUTImpl::GetCPUmode()
{
	return spoutSDK->GetCPUmode();
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
	return spoutSDK->SetBufferMode(bActive);
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
	return spoutSDK->SetMaxSenders(maxSenders);
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

// Access to globals
bool SPOUTImpl::GetSpoutSenderName(char * sendername, int maxchars)
{
	return spoutSDK->GetSpoutSenderName(sendername, maxchars);
}

bool SPOUTImpl::IsSpoutInitialized()
{
	return spoutSDK->IsSpoutInitialized();
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
	pSpout->spoutSDK = new Spout;
	
	return pSpout;
}

////////////////////////////////////////////////////////////////////////////////