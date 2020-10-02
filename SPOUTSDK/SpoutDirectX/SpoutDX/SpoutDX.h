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
#include "..\..\SpoutSDK\SpoutCommon.h" // for dll build and utilities
#include "..\..\SpoutSDK\SpoutSenderNames.h" // for sender creation and update
#include "..\..\SpoutSDK\SpoutDirectX.h" // for creating DX11 textures
#include "..\..\SpoutSDK\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\SpoutSDK\SpoutCopy.h" // for pixel copy
#include "..\..\SpoutSDK\SpoutUtils.h" // Registry utiities

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
	bool OpenDirectX11(ID3D11Device* pDevice = nullptr);
	ID3D11Device* GetDevice();
	void CleanupDX11();

	//
	// SENDER
	//
	// Set the sender name
	bool SetSenderName(const char* sendername = nullptr);
	// Set the sender texture format
	void SetSenderFormat(DXGI_FORMAT format);
	// Close sender and free resources
	void ReleaseSender();
	// Send a texture
	bool SendTexture(ID3D11Texture2D* pTexture);
	// Send an image
	bool SendImage(unsigned char * pData, unsigned int width, unsigned int height);
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
	// Close receiver and free resources
	void ReleaseReceiver();
	// Receive a texture from a sender
	bool ReceiveTexture(ID3D11Texture2D** ppTexture = nullptr);
	// Receive an image
	bool ReceiveImage(unsigned char * pixels, bool bInvert = false);
	// Receive an rgb image
	bool ReceiveRGBimage(unsigned char * pixels, unsigned int width, unsigned int height, bool bInvert = false);
	// Open sender selection dialog
	void SelectSender();
	// Sender has changed
	bool IsUpdated();
	// Connected to a sender
	bool IsConnected();
	// Received frame is new
	bool IsFrameNew();
	// Received sender share handle
	HANDLE GetSenderHandle();
	// Received sender texture format
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
	
	//
	// COMMON
	//
	void HoldFps(int fps); // Hold frame rate
	void DisableFrameCount(); // Disable frame counting for this application
	bool IsFrameCountEnabled(); // Return frame count status
								
	// Sender names
	int  GetSenderCount();
	bool GetSender(int index, char* sendername, int MaxSize = 256);
	bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	bool GetActiveSender(char* sendername);
	bool SetActiveSender(const char* sendername);
	int  GetMaxSenders(); // Get maximum senders allowed
	void SetMaxSenders(int maxSenders); // Set maximum senders allowed

	// Adapter functions
	int  GetNumAdapters(); // Get the number of graphics adapters in the system
	bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
	int  GetAdapter(); // Get the current adapter index
	bool SetAdapter(int index = 0); // Set required graphics adapter for output

	// Sharing modes not supported
	bool GetDX9();
	bool GetMemoryShareMode();

	// Utility
	bool CreateDX11texture(ID3D11Device* pd3dDevice,
		unsigned int width, unsigned int height,
		DXGI_FORMAT format, ID3D11Texture2D** ppTexture);

	spoutSenderNames spoutsender;
	spoutFrameCount frame;
	spoutDirectX spoutdx;
	spoutCopy spoutcopy;

protected :

	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pImmediateContext;
	ID3D11Texture2D* m_pSharedTexture;
	ID3D11Texture2D* m_pStaging[2];
	int m_Index;
	int m_NextIndex;

	HANDLE m_dxShareHandle;
	DWORD m_dwFormat;
	SharedTextureInfo m_SenderInfo;
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
	bool m_bClassDevice;
	SHELLEXECUTEINFOA m_ShExecInfo;

	bool ReceiveSenderData();
	void CreateReceiver(const char * sendername, unsigned int width, unsigned int height, DWORD dwFormat);
	
	// Read pixels via staging texture
	bool ReadRGBApixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReadRGBpixels(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReadRGBAimage(unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool ReadRGBimage(unsigned char* pixels, unsigned int width, unsigned int height, bool bInvert);
	bool CheckStagingTextures(unsigned int width, unsigned int height, DWORD dwFormat = DXGI_FORMAT_B8G8R8A8_UNORM);
	bool CreateDX11StagingTexture(unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** pStagingTexture);

	void SelectSenderPanel();
	bool CheckSpoutPanel(char *sendername, int maxchars = 256);


};

#endif
