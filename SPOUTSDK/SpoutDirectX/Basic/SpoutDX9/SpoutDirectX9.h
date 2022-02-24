/*

	spoutDirectX9.h

	Functions to manage DirectX 9 texture sharing

	Copyright (c) 2014 - 2022, Lynn Jarvis. All rights reserved.

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
#ifndef __spoutDirectX9__ 
#define __spoutDirectX9__

#include "..\..\..\SpoutGL\SpoutCommon.h"
#include <d3d9.h>
#pragma comment (lib, "d3d9.lib")

using namespace spoututils;

class SPOUT_DLLEXP spoutDirectX9 {

	public:

		spoutDirectX9();
		~spoutDirectX9();

		// Initialize and prepare DirectX 9
		bool OpenDirectX9(HWND hWnd);
		// Release DirectX9 class object and device
		void CloseDirectX9();
		// Create a DirectX9 object
		IDirect3D9Ex* CreateDX9object();
		// Create a DirectX9 device
		IDirect3DDevice9Ex* CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd, unsigned int AdapterIndex = 0);
		// Create a DirectX9 shared texture
		bool CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle);
		// Create a DirectX9 object
		IDirect3D9Ex* GetDX9object();
		// Create a DirectX9 device
		IDirect3DDevice9Ex* GetDX9device();
		// Set a DirectX9 device
		void SetDX9device(IDirect3DDevice9Ex* pDevice);
		// Write to a DirectX9 system memory surface
		bool WriteDX9memory (IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture);
		// Copy from a GPU DX9 surface to the DX9 shared texture
		bool WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture);

	protected:

		IDirect3D9Ex* m_pD3D; // DX9 object
		IDirect3DDevice9Ex* m_pDevice; // DX9 device

};

#endif
