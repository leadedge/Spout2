/*

					SpoutCopy.h

		Functions to manage pixel buffer copying

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
#ifndef __spoutCopy__ // standard way as well
#define __spoutCopy__

#include "SpoutCommon.h"
#include <windows.h>
#include <stdio.h> // for debug printf
#include <gl/gl.h> // For OpenGL definitions
#include <intrin.h> // for cpuid to test for SSE2
#include <emmintrin.h> // for SSE2
#include <tmmintrin.h> // for SSSE3


class SPOUT_DLLEXP spoutCopy {

	public:

		spoutCopy();
		~spoutCopy();

		void CopyPixels(const unsigned char *src, unsigned char *dst,
						unsigned int width, unsigned int height, 
						GLenum glFormat = GL_RGBA, bool bInvert = false) const;

		void FlipBuffer(const unsigned char *src, unsigned char *dst,
						unsigned int width, unsigned int height,
						GLenum glFormat = GL_RGBA) const;

		void memcpy_sse2(void* dst, const void* src, size_t size) const;

		void rgba2rgba(const void* source, void* dest, unsigned int width, unsigned int height,
			unsigned int sourcePitch, bool bInvert = false) const;

		void rgba2rgba(const void* source, void* dest, unsigned int width, unsigned int height,
			unsigned int sourcePitch, unsigned int destPitch, bool bInvert) const;
		void rgba2rgbaResample(const void* source, void* dest,
			unsigned int sourceWidth, unsigned int sourceHeight, unsigned int sourcePitch,
			unsigned int destWidth, unsigned int destHeight, bool bInvert = false) const;

		void rgba2bgra(const void* rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void bgra2rgba(const void* bgra_source, void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false) const;

		void RemovePadding(const unsigned char *source, unsigned char *dest,
			unsigned int width, unsigned int height,
			unsigned int source_stride, GLenum glFormat) const;

		// TODO : avoid redundancy
		// TODO : add source pitch
		void rgb2rgba (const void* rgb_source, void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void bgr2rgba (const void* bgr_source, void *rgba_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void rgb2bgra (const void* rgb_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void bgr2bgra (const void* bgr_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void rgba2bgr (const void* rgba_source, void *bgr_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void bgra2rgb (const void* bgra_source, void *rgb_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void bgra2bgr (const void* bgra_source, void *bgr_dest, unsigned int width, unsigned int height, bool bInvert = false) const;

		void rgba2rgb (const void* rgba_source, void *rgb_dest,	unsigned int width, unsigned int height,
			unsigned int sourcePitch, bool bInvert = false) const;
		void rgba2rgbResample(const void* source,void* dest,
			unsigned int sourceWidth, unsigned int sourceHeight, unsigned int sourcePitch,
			unsigned int destWidth, unsigned int destHeight, bool bInvert = false) const;

		void rgba2bgr(const void* rgba_source, void *rgb_dest, unsigned int width, unsigned int height,
			unsigned int sourcePitch, bool bInvert = false) const;
		void rgba2bgrResample(const void* source, void* dest,
			unsigned int sourceWidth, unsigned int sourceHeight, unsigned int sourcePitch,
			unsigned int destWidth, unsigned int destHeight, bool bInvert = false) const;

	protected :

		void CheckSSE();
		bool m_bSSE2;
		bool m_bSSE3;
		bool m_bSSSE3;

		void rgba_bgra(const void *rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void rgba_bgra_sse2(const void *rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;
		void rgba_bgra_sse3(const void *rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false) const;

};

#endif
