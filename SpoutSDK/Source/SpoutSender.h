/*

					SpoutSender.h
 
					TODO: SendImage - undocumented. Work in progress.

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
#pragma once

#ifndef __SpoutSender__
#define __SpoutSender__

#include "spoutSDK.h"

class SpoutSender {

	public:

	SpoutSender();
    ~SpoutSender();

	bool CreateSender(char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0);
	bool UpdateSender(char *Sendername, unsigned int width, unsigned int height);
	void ReleaseSender(DWORD dwMsec = 0);
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true);
	bool SendImage(unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert=true);

	bool SetMemoryShareMode(bool bMemoryMode = true);
	bool GetMemoryShareMode();

	void SetDX9compatible(bool bCompatible = true);
	bool GetDX9compatible();

	bool SenderDebug(char *Sendername, int size);


protected :

	Spout spout;

};

#endif
