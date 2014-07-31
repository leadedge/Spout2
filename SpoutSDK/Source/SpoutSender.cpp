//
//		SpoutSender
//
//		Wrapper class so that a sender object can be created independent of a receiver
//
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
#include "SpoutSender.h"

SpoutSender::SpoutSender()
{

}


//---------------------------------------------------------
SpoutSender::~SpoutSender()
{

}



//---------------------------------------------------------
bool SpoutSender::CreateSender(char *name, unsigned int width, unsigned int height, DWORD dwFormat)
{
	return spout.CreateSender(name, width, height, dwFormat);
}


//---------------------------------------------------------
bool SpoutSender::UpdateSender(char *name, unsigned int width, unsigned int height)
{
	return spout.UpdateSender(name, width, height);
}


//---------------------------------------------------------
void SpoutSender::ReleaseSender(DWORD dwMsec)
{
	spout.ReleaseSender(dwMsec);
}


//---------------------------------------------------------
bool SpoutSender::SendTexture(GLuint TextureID, GLuint TextureTarget,  unsigned int width, unsigned int height, bool bInvert) //DWORD dwFormat, bool bInvert)
{
	return spout.SendTexture(TextureID, TextureTarget, width, height, bInvert);
}


//---------------------------------------------------------
bool SpoutSender::SendImage(unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert)
{
	return spout.SendImage(pixels, width, height, bInvert);
}


//---------------------------------------------------------
bool SpoutSender::GetMemoryShareMode()
{
	return spout.GetMemoryShareMode();
}


//---------------------------------------------------------
bool SpoutSender::SetMemoryShareMode(bool bMemoryMode)
{
	return spout.SetMemoryShareMode(bMemoryMode);
}


//---------------------------------------------------------
void SpoutSender::SetDX9(bool bDX9)
{
	spout.SetDX9(bDX9);
}


//---------------------------------------------------------
bool SpoutSender::GetDX9()
{
	return spout.interop.isDX9();
}

//---------------------------------------------------------
void SpoutSender::SetDX9compatible(bool bCompatible)
{
	if(bCompatible) {
		// printf("Setting : DXGI_FORMAT_B8G8R8A8_UNORM\n");
		// DX11 -> DX9 only works if the DX11 format is set to DXGI_FORMAT_B8G8R8A8_UNORM
		spout.interop.SetDX11format(DXGI_FORMAT_B8G8R8A8_UNORM);
	}
	else {
		// DX11 -> DX11 only
		// printf("Setting : DXGI_FORMAT_R8G8B8A8_UNORM\n");
		spout.interop.SetDX11format(DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}


//---------------------------------------------------------
bool SpoutSender::GetDX9compatible()
{
	if(spout.interop.DX11format == DXGI_FORMAT_B8G8R8A8_UNORM)
		return true;
	else
		return false;
	
}

//---------------------------------------------------------
bool SpoutSender::SetVerticalSync(bool bSync)
{
	return spout.interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
bool SpoutSender::GetVerticalSync()
{
	return spout.interop.GetVerticalSync();
}





bool SpoutSender::SenderDebug(char *Sendername, int size)
{
	return spout.interop.senders.SenderDebug(Sendername, size);

}