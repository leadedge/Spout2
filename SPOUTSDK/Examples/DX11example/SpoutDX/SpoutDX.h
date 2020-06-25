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

// Change the path as required
#include "..\..\..\SpoutSDK\SpoutCommon.h" // for dll build and utilities
#include "..\..\..\SpoutSDK\SpoutSenderNames.h" // for sender creation and update
#include "..\..\..\SpoutSDK\SpoutDirectX.h" // for creating DX11 textures
#include "..\..\..\SpoutSDK\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\..\SpoutSDK\SpoutCopy.h" // for pixel copy
#include "..\..\..\SpoutSDK\SpoutUtils.h" // Registry utiities

#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

class SPOUT_DLLEXP spoutDX {

	public:

	spoutDX();
    ~spoutDX();

	//
	// DIRECTX
	//
	ID3D11Device* OpenDirectX11();
	ID3D11Device* GetDevice();
	void CleanupDX11();

	//
	// SENDER
	//
	// Set the sender name
	bool SetSenderName(const char* sendername);
	// Close sender and free resources
	void ReleaseSender();
	// Send a texture
	bool SendTexture(ID3D11Device* pDevice, ID3D11Texture2D* pTexture);
	// Send an image
	bool SendImage(ID3D11Device* pDevice, unsigned char * pData, unsigned int width, unsigned int height);
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
	void SetReceiverName(const char * SenderName);
	// Set up receiver for a new sender
	void CreateReceiver(const char * SenderName, unsigned int width, unsigned int height);
	// Close receiver and free resources
	void ReleaseReceiver();
	// Receive a DX11 texture from a sender
	bool ReceiveTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D** ppTexture = nullptr);
	// Receive an image
	bool ReceiveImage(ID3D11Device* pd3dDevice, unsigned char * pData, unsigned int width, unsigned int height, bool bInvert = false);
	// Receive an rgb image
	bool ReceiveRGBimage(ID3D11Device* pd3dDevice, unsigned char * pData, unsigned int width, unsigned int height, bool bInvert = false);
	// Open sender selection dialog
	void SelectSender();
	// Sender has changed
	bool IsUpdated();
	// Connected to a sender
	bool IsConnected();
	// Received frame is new
	bool IsFrameNew();
	// Received texture pointer
	ID3D11Texture2D* GetTexture();
	// Received sender share handle
	HANDLE GetSenderHandle();
	// Received sender format
	DXGI_FORMAT GetSenderFormat();
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
	// Receiver utility
	bool CopySenderTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D* pTexture, HANDLE sharehandle);
	
	//
	// COMMON
	//
	void HoldFps(int fps); // Hold frame rate
	void DisableFrameCount(); // Disable frame counting for this application
	bool IsFrameCountEnabled(); // Return frame count status
								
	// Sender names
	int  GetSenderCount();
	bool GetSender(int index, char* Sendername, int MaxSize = 256);
	bool GetSenderInfo(const char* Sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	bool GetActiveSender(char* Sendername);
	bool SetActiveSender(const char* Sendername);
	int  GetMaxSenders(); // Get maximum senders allowed
	void SetMaxSenders(int maxSenders); // Set maximum senders allowed

	// Adapter functions
	int  GetNumAdapters(); // Get the number of graphics adapters in the system
	bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
	int  GetAdapter(); // Get the current adapter index
	bool SetAdapter(int index = 0); // Set required graphics adapter for output

	// Utilities
	bool GetDX9();
	bool GetMemoryShareMode();
	bool CreateDX11texture(ID3D11Device* pDevice,
		unsigned int width, unsigned int height,
		DXGI_FORMAT format,
		ID3D11Texture2D** ppTexture,
		HANDLE *shareHandle = nullptr);

	spoutFrameCount frame;
	spoutDirectX spoutdx;

protected :

	spoutSenderNames spoutsender;
	spoutCopy spoutcopy;

	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pImmediateContext;
	ID3D11Texture2D* m_pReceivedTexture;
	ID3D11Texture2D* m_pSharedTexture;
	HANDLE m_dxShareHandle;
	DWORD m_dwFormat;
	char m_SenderNameSetup[256];
	char m_SenderName[256];
	unsigned int m_Width;
	unsigned int m_Height;
	bool m_bUpdated;
	bool m_bConnected;
	bool m_bNewFrame;
	bool m_bSpoutInitialized;
	bool m_bSpoutPanelOpened;
	bool m_bSpoutPanelActive;
	SHELLEXECUTEINFOA m_ShExecInfo;
	ID3D11Texture2D* m_pStagingTexture;

	bool ReadRGBpixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReadRGBApixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReceiveSenderData();
	bool CreateDX11StagingTexture(ID3D11Device* pDevice,
		unsigned int width, unsigned int height,
		DXGI_FORMAT format,
		ID3D11Texture2D** pStagingTexture);
	bool CheckStagingTexture(ID3D11Device* pDevice, unsigned int width, unsigned int height);
	void SelectSenderPanel();
	bool CheckSpoutPanel(char *sendername, int maxchars = 256);

};

#endif
