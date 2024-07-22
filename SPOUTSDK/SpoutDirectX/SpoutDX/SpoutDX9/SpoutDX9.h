/*

	spoutDX9.h

	Functions to manage DirectX 9 texture sharing

	Copyright (c) 2020-2024, Lynn Jarvis. All rights reserved.

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
#ifndef __spoutDX9__ 
#define __spoutDX9__

#pragma warning( disable : 4005 ) // Disable macro re-definition warnings

//
// Include file path
//
// 1) If the include files are in the same folder there is no prefix.
//    This applies for a build using SpoutDX9 dll or static library.
//
// 2) If the Spout source is built as a dll or static library,
//    or an application is built using the repository folder structure
//    the path prefix for include files is "..\..\..\SpoutGL\"
//
// 3) If the include files are in a different folder, change the prefix as required.
//

#if __has_include("SpoutCommon.h")
#include "SpoutCommon.h" // include files in the same folder
#include "SpoutSenderNames.h"
#include "SpoutFrameCount.h"
#include "SpoutUtils.h"
#else
#include "..\..\..\SpoutGL\SpoutCommon.h" // repository folder structure
#include "..\..\..\SpoutGL\SpoutSenderNames.h"
#include "..\..\..\SpoutGL\SpoutFrameCount.h"
#include "..\..\..\SpoutGL\SpoutUtils.h"
#endif

#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp
#include <d3d9.h>
#pragma comment (lib, "d3d9.lib")

using namespace spoututils;

class SPOUT_DLLEXP spoutDX9 {

	public:

		spoutDX9();
		~spoutDX9();

		//
		// DIRECTX9
		//

		// Initialize and prepare DirectX 9
		bool OpenDirectX9(HWND hWnd = nullptr);
		// Release DirectX9 class object and device
		void CloseDirectX9();

		// Create a DirectX9 object
		IDirect3D9Ex* CreateDX9object();
		// Create a DirectX9 device
		IDirect3DDevice9Ex* CreateDX9device(IDirect3D9Ex* pD3D, HWND hWnd, unsigned int AdapterIndex = 0);

		// Get DirectX9 object
		IDirect3D9Ex* GetDX9object();
		// Get DirectX9 device
		IDirect3DDevice9Ex* GetDX9device();
		// Set a DirectX9 device
		void SetDX9device(IDirect3DDevice9Ex* pDevice);

		//
		// SENDER
		//

		// Set the sender name
		bool SetSenderName(const char* sendername = nullptr);
		// Send a DirectX9 surface
		bool SendDX9surface(IDirect3DSurface9* pSurface, bool bUpdate = true);
		// Close sender and free resources
		void ReleaseDX9sender();
		// Sender status
		bool IsInitialized();
		// Sender name
		const char * GetName();
		// Get width
		unsigned int GetWidth();
		// Get height
		unsigned int GetHeight();
		// Get frame rate
		double GetFps();
		// Get frame number
		long GetFrame();

		//
		// RECEIVER
		//

		// Set the sender to connect to
		void SetReceiverName(const char * sendername);
		// Receive a DirectX 9 texture from a sender
		bool ReceiveDX9Texture(LPDIRECT3DTEXTURE9 &pTexture);
		// Close receiver and free resources
		void ReleaseReceiver();
		// Open sender selection dialog
		bool SelectSender(HWND hwnd = nullptr);
		// Sender has changed
		bool IsUpdated();
		// Connected to a sender
		bool IsConnected();
		// Received frame is new
		bool IsFrameNew();
		// Received sender share handle
		HANDLE GetSenderHandle();
		// Received sender texture format (DX11)
		DWORD GetSenderFormat();
		// Received sender name
		const char * GetSenderName();
		// Received sender width
		unsigned int GetSenderWidth();
		// Received sender height
		unsigned int GetSenderHeight();
		// Received sender frame rate
		double GetSenderFps();
		// Received sender frame number
		long GetSenderFrame();

		//
		// Sender names
		//

		// Get number of senders
		int GetSenderCount();
		// Get sender name for a given index
		bool GetSender(int index, char* sendername, int MaxSize = 256);
		// Return a list of current senders
		std::vector<std::string> GetSenderList();
		// Sender index into the set of names
		int GetSenderIndex(const char* sendername);
		// Get sender details
		bool GetSenderInfo(const char* sendername, unsigned int& width, unsigned int& height, HANDLE& dxShareHandle, DWORD& dwFormat);
		// Get active sender name
		bool GetActiveSender(char* sendername);
		// set active sender name
		bool SetActiveSender(const char* sendername);
		// Get maximum senders allowed
		int  GetMaxSenders();
		// Set maximum senders allowed
		void SetMaxSenders(int maxSenders);


		//
		// COMMON
		//

		// Hold frame rate
		void HoldFps(int fps);
		// Create a DirectX9 shared texture
		bool CreateSharedDX9Texture(IDirect3DDevice9Ex* pDevice, unsigned int width, unsigned int height, D3DFORMAT format, LPDIRECT3DTEXTURE9 &dxTexture, HANDLE &dxShareHandle);


	protected:

		spoutFrameCount frame;
		spoutSenderNames sendernames;

		IDirect3D9Ex* m_pD3D; // DX9 object
		IDirect3DDevice9Ex* m_pDevice; // DX9 device
		bool m_bSpoutInitialized;
		HANDLE m_dxShareHandle;
		LPDIRECT3DTEXTURE9 m_pSharedTexture; // Texture to be shared
		DWORD m_dwFormat;
		SharedTextureInfo m_SenderInfo;
		char m_SenderNameSetup[256];
		char m_SenderName[256];
		unsigned int m_Width;
		unsigned int m_Height;
		bool m_bUpdated;
		bool m_bConnected;
		bool m_bNewFrame;
		bool m_bSpoutPanelOpened;
		bool m_bSpoutPanelActive;
		bool m_bClassDevice;
		SHELLEXECUTEINFOA m_ShExecInfo;

		// Check that a sender is up to date
		bool CheckDX9sender(unsigned int width, unsigned int height, DWORD dwFormat);
		// Write to a DirectX9 system memory surface
		bool WriteDX9memory(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture);
		// Copy from a GPU DX9 surface to the DX9 shared texture
		bool WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface, LPDIRECT3DTEXTURE9 dxTexture);

		// Connect to a sender
		bool ReceiveSenderData();
		// Copy from a sender shared texture to a DX9 texture
		bool ReadDX9texture(IDirect3DDevice9Ex* pDevice, LPDIRECT3DTEXTURE9 &dxTexture);
		// Create receiver resources
		void CreateReceiver(const char * SenderName, unsigned int width, unsigned int height, DWORD dwFormat);
		// Pop up SpoutPanel to allow the user to select a sender
		bool SelectSenderPanel(const char* message);
		// Check whether SpoutPanel opened and return the new sender name
		bool CheckSpoutPanel(char *sendername, int maxchars = 256);

};

#endif
