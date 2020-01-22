/*

			SpoutDX.h

			Sender and receiver for DirectX applications

	Copyright (c) 2014-2020, Lynn Jarvis. All rights reserved.

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

#ifndef __spoutDX__
#define __spoutDX__

#include "..\..\..\SpoutSDK\SpoutCommon.h" // for dll build and utilities
#include "..\..\..\SpoutSDK\SpoutSenderNames.h" // for sender creation and update
#include "..\..\..\SpoutSDK\SpoutDirectX.h" // for creating DX11 textures
#include "..\..\..\SpoutSDK\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\..\SpoutSDK\SpoutCopy.h" // for pixel copy
#include "..\..\..\SpoutSDK\SpoutMemoryShare.h" // for memoryshare fallback support
#include "..\..\..\SpoutSDK\SpoutUtils.h" // Registry utiities

#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

class SPOUT_DLLEXP spoutDX {

	public:

	spoutDX();
    ~spoutDX();

	bool OpenDirectX11();
	void CleanupDX11();

	//
	// SENDER
	//

	// Send a DX11 texture (creates and updates a sender)
	bool SendTexture(const char* sendername, ID3D11Device* pDevice, ID3D11Texture2D* pTexture);
	// Close sender and free resources
	void ReleaseSender();
	// Hold frame rate
	void HoldFps(int fps);
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

	// Set the sender name to connect to
	void SetReceiverName(const char * SenderName);
	// Receive a DX11 texture from a sender
	bool ReceiveTexture(ID3D11Device* pd3dDevice);
	// Receive pixels from a sender via DX11 staging texture
	bool ReceiveImage(unsigned char * pData);
	// Receive pixels from a sender via DX11 staging texture to an rgb buffer of variable size
	bool ReceiveRGBimage(unsigned char * pData, unsigned int sourceWidth, unsigned int sourceHeight, bool bInvert = false);
	// Receive pixels from sender shared memory (Spout memory mode must be selected)
	bool ReceiveMemory(const char* sendername, unsigned char* pixels,
		unsigned int width, unsigned int height, GLenum glFormat, bool bInvert);
	// Close receiver and free resources
	void ReleaseReceiver();
	// Open the user sender selection dialog
	void SelectSender();
	// Return whether connected to a sender
	bool IsConnected();
	// Return whether the connected sender has changed
	bool IsUpdated();
	// Return shared texture copy
	ID3D11Texture2D* GetSenderTexture();
	// Return texture format
	DXGI_FORMAT GetSenderTextureFormat();
	// Return sender name
	const char * GetSenderName();
	// Return sender width
	unsigned int GetSenderWidth();
	// Return sender height
	unsigned int GetSenderHeight();
	// Return sender frame rate
	double GetSenderFps();
	// Return sender frame number
	long GetSenderFrame();

	// Disable frame counting for this application
	void DisableFrameCount();
	// Return frame count status
	bool IsFrameCountEnabled();

	// Sender names
	int  GetSenderCount();
	bool GetSender(int index, char* Sendername, int MaxSize = 256);
	bool GetSenderInfo(const char* Sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	bool GetActiveSender(char* Sendername);
	bool SetActiveSender(const char* Sendername);
	// Adapter functions
	int  GetNumAdapters(); // Get the number of graphics adapters in the system
	bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
	int  GetAdapter(); // Get the current adapter index
	bool SetAdapter(int index = 0); // Set required graphics adapter for output
	// Utilities
	bool GetDX9();
	bool GetMemoryShareMode();
	int  GetMaxSenders(); // Get maximum senders allowed
	void SetMaxSenders(int maxSenders); // Set maximum senders allowed

protected :

	void SetupReceiver(unsigned int width, unsigned int height);
	bool CreateDX11Texture(ID3D11Device* pd3dDevice, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** ppTexture);
	bool CreateDX11StagingTexture(ID3D11Device* pDevice, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** pStagingTexture);
	bool CheckStagingTexture(unsigned int width, unsigned int height);
	bool ReadRGBApixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReadRGBpixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool CheckSpoutPanel(char *sendername, int maxchars);

	spoutSenderNames spoutsender;
	spoutDirectX spoutdx;
	spoutFrameCount frame;
	spoutCopy spoutcopy;
	spoutMemoryShare memoryshare;

	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pImmediateContext; // DX11
	ID3D11Texture2D* m_pSenderTexture;
	ID3D11Texture2D* m_pStagingTexture;
	ID3D11Texture2D* m_pSharedTexture;
	HANDLE m_dxShareHandle;
	DWORD m_dwFormat;
	char m_SenderNameSetup[256];
	char m_SenderName[256];
	unsigned int m_Width;
	unsigned int m_Height;
	bool bSpoutInitialized;
	bool m_bConnected;
	bool m_bUpdate;
	bool m_bInvert;
	bool m_bUseActive;
	bool bSpoutPanelOpened;
	bool bSpoutPanelActive;
	SHELLEXECUTEINFOA g_ShExecInfo;

};

#endif
