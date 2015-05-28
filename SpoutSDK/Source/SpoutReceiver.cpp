//
//		SpoutReceiver
//
//		Wrapper class so that a receiver object can be created independent of a sender
//
// ====================================================================================
//		Revisions :
//
//		27-07-14	- CreateReceiver - bUseActive flag instead of null name
//		03.09.14	- Cleanup
//		23.09.14	- return DirectX 11 capability in SetDX9
//		28.09.14	- Added Host FBO for ReceiveTexture
//		12.10.14	- changed SelectSenderPanel arg to const char
//		23.12.14	- added host fbo arg to ReceiveImage
//		08.02.15	- Changed default texture format for ReceiveImage in header to GL_RGBA
//
// ====================================================================================
/*
		Copyright (c) 2014>, Lynn Jarvis. All rights reserved.

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
#include "SpoutReceiver.h"

SpoutReceiver::SpoutReceiver()
{

}


//---------------------------------------------------------
SpoutReceiver::~SpoutReceiver()
{

}


//---------------------------------------------------------
bool SpoutReceiver::ReceiveTexture(char* name, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	return spout.ReceiveTexture(name, width, height, TextureID, TextureTarget, bInvert, HostFBO);
}


//---------------------------------------------------------
bool SpoutReceiver::ReceiveImage(char* name, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat, GLuint HostFBO)
{
	return spout.ReceiveImage(name, width, height, pixels, glFormat, HostFBO);
}


//---------------------------------------------------------
bool SpoutReceiver::GetImageSize(char* name, unsigned int &width, unsigned int &height, bool &bMemoryMode)
{
	return spout.GetImageSize(name, width, height, bMemoryMode);
}


//---------------------------------------------------------
bool SpoutReceiver::CreateReceiver(char* name, unsigned int &width, unsigned int &height, bool bUseActive)
{
	return spout.CreateReceiver(name, width, height, bUseActive);
}

//---------------------------------------------------------
void SpoutReceiver::ReleaseReceiver()
{
	spout.ReleaseReceiver();
}


//---------------------------------------------------------
bool SpoutReceiver::BindSharedTexture()
{
	return spout.BindSharedTexture();
}


//---------------------------------------------------------
bool SpoutReceiver::UnBindSharedTexture()
{
	return spout.UnBindSharedTexture();
}


//---------------------------------------------------------
int  SpoutReceiver::GetSenderCount()
{
	return spout.GetSenderCount();
}

//---------------------------------------------------------
bool SpoutReceiver::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert)
{
	return spout.DrawSharedTexture(max_x, max_y, aspect, bInvert);
}


//---------------------------------------------------------
bool SpoutReceiver::GetSenderName(int index, char* sendername, int MaxNameSize)
{
	return spout.GetSenderName(index, sendername, MaxNameSize);
}

//---------------------------------------------------------
bool SpoutReceiver::GetActiveSender(char* Sendername)
{
	return spout.GetActiveSender(Sendername);
}


//---------------------------------------------------------
bool SpoutReceiver::SetActiveSender(char* Sendername)
{
	return spout.SetActiveSender(Sendername);
}


//---------------------------------------------------------
bool SpoutReceiver::GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spout.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}


//---------------------------------------------------------
bool SpoutReceiver::SelectSenderPanel(const char* message)
{
	return spout.SelectSenderPanel(message);
}

//---------------------------------------------------------
bool SpoutReceiver::GetMemoryShareMode()
{
	return spout.GetMemoryShareMode();
}


//---------------------------------------------------------
bool SpoutReceiver::SetMemoryShareMode(bool bMemory)
{
	return spout.SetMemoryShareMode(bMemory);
}

//---------------------------------------------------------
bool SpoutReceiver::SetDX9(bool bDX9)
{
	return spout.interop.UseDX9(bDX9);
}


//---------------------------------------------------------
bool SpoutReceiver::GetDX9()
{
	return spout.interop.isDX9();
}


//---------------------------------------------------------
void SpoutReceiver::SetDX9compatible(bool bCompatible)
{
	if(bCompatible) {
		// DX11 -> DX9 only works if the DX11 format is set to DXGI_FORMAT_B8G8R8A8_UNORM
		spout.interop.SetDX11format(DXGI_FORMAT_B8G8R8A8_UNORM);
	}
	else {
		// DX11 -> DX11 only
		spout.interop.SetDX11format(DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}


//---------------------------------------------------------
bool SpoutReceiver::GetDX9compatible()
{
	if(spout.interop.DX11format == DXGI_FORMAT_B8G8R8A8_UNORM)
		return true;
	else
		return false;
}


//---------------------------------------------------------
bool SpoutReceiver::SetVerticalSync(bool bSync)
{
	return spout.interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
int SpoutReceiver::GetVerticalSync()
{
	return spout.interop.GetVerticalSync();
}
