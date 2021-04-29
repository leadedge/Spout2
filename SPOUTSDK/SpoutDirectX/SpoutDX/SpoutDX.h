/*

			SpoutDX.h

			Sender and receiver for DirectX applications

	Copyright (c) 2014-2021, Lynn Jarvis. All rights reserved.

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
#include "..\..\SpoutGL\SpoutCommon.h" // for dll build
#include "..\..\SpoutGL\SpoutSenderNames.h" // for sender creation and update
#include "..\..\SpoutGL\SpoutDirectX.h" // for creating DX11 textures
#include "..\..\SpoutGL\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "..\..\SpoutGL\SpoutCopy.h" // for pixel copy
#include "..\..\SpoutGL\SpoutUtils.h" // Registry utiities

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
	ID3D11Device* GetDX11Device();
	ID3D11DeviceContext* GetDX11Context();
	void CloseDirectX11();
	bool IsClassDevice();

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
	bool ReceiveImage(unsigned char * pixels, unsigned int width, unsigned int height, bool bRGB = false, bool bInvert = false);
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

	// Frame rate control
	void HoldFps(int fps);
	// Disable frame counting for this application
	void DisableFrameCount();
	// Return frame count status
	bool IsFrameCountEnabled();
	// Signal sync event 
	void SetFrameSync(const char* SenderName);
	// Wait or test for a sync event
	bool WaitFrameSync(const char *SenderName, DWORD dwTimeout = 0);

								
	//
	// Sender names
	//

	// Get number of senders
	int  GetSenderCount();
	// Get sender name for a given index
	bool GetSender(int index, char* sendername, int MaxSize = 256);
	// Get sender details
	bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
	// Get active sender name
	bool GetActiveSender(char* sendername);
	// set active sender name
	bool SetActiveSender(const char* sendername);
	// Get maximum senders allowed
	int  GetMaxSenders();
	// Set maximum senders allowed
	void SetMaxSenders(int maxSenders);

	//
	// Adapter functions
	//

	// Get the number of graphics adapters in the system
	int GetNumAdapters();
	// Get the adapter name for a given index
	bool GetAdapterName(int index, char *adaptername, int maxchars);
	// Get the current adapter description
	bool GetAdapterInfo(char *renderdescription, char *displaydescription, int maxchars);
	// Get the current adapter index
	int  GetAdapter();
	// Set required graphics adapter for output (no args or -1 to reset)
	bool SetAdapter(int index = -1);
	// Get adapter pointer for a given adapter (-1 means current)
	IDXGIAdapter* GetAdapterPointer(int index = -1);
	// Set required graphics adapter for creating a device
	void SetAdapterPointer(IDXGIAdapter* pAdapter);
	// Get auto device switching status
	bool GetAdapterAuto();
	// Auto switch receiving device to use the same graphics adapter as the sender
	void SetAdapterAuto(bool bAuto = true);
	// Get sender adapter index and name for a given sender
	int GetSenderAdapter(const char* sendername, char* adaptername = nullptr, int maxchars = 256);

	//
	// Sharing modes (2.006 compatibility)
	//

	// Get user selected DX9 mode (2.006)
	bool GetDX9();
	bool GetMemoryShareMode();

	//
	// Utility
	//

	void CheckSenderFormat(const char * sendername);
	bool CreateDX11texture(ID3D11Device* pd3dDevice,
		unsigned int width, unsigned int height,
		DXGI_FORMAT format, ID3D11Texture2D** ppTexture);


	//
	// Memory sharing
	//

	// Write data
	bool WriteMemoryBuffer(const char *sendername, const char* data, int length);
	// Read data
	int  ReadMemoryBuffer(const char* sendername, char* data, int maxlength);

	struct {

		SpoutSharedMemory *senderMem;
		unsigned int m_Width;
		unsigned int m_Height;

		// Create a sender named shared memory map for general purpose
		// NOTE:
		//    Width and height are in bytes
		//    Height may be 1 for a luminance map
		//    Width should be a multiple of 16 for best performance
		bool CreateSenderMemory(const char *sendername, unsigned int width, unsigned int height)
		{
			std::string namestring = sendername;

			// Create a name for the map from the sender name
			namestring += "_map";

			// Close an existing map
			if (senderMem) {
				CloseSenderMemory();
				senderMem = nullptr;
			}

			// Create a new shared memory class object
			senderMem = new SpoutSharedMemory();

			// Create the sender's shared memory map.
			// This also creates a mutex to lock and unlock the map for reads.
			SpoutCreateResult result = senderMem->Create(namestring.c_str(), (int)(width*height));
			if (result == SPOUT_CREATE_FAILED) {
				delete senderMem;
				senderMem = nullptr;
				m_Width = 0;
				m_Height = 0;
				return false;
			}

			// Set the width and height for future reference
			m_Width = width;
			m_Height = height;

			return true;

		} // end CreateSenderMemory

		// Open an existing named shared memory map
		bool OpenSenderMemory(const char *sendername)
		{
			std::string namestring = sendername;
			// Create a name for the map from the sender name
			namestring += "_map";
			// Create a new shared memory class object for this receiver
			if (!senderMem)
				senderMem = new SpoutSharedMemory();
			// Open the sender's shared memory map.
			// This also creates a mutex for the receiver
			// to lock and unlock the map for reads.
			if (!senderMem->Open(namestring.c_str())) {
				// SpoutLogError("spoutGL.memoryshare::OpenSenderMemory - open shared memory failed");
				return false;
			}
			return true;
		} // end OpenSenderMemory

		// Close the sender shared memory map
		void CloseSenderMemory()
		{
			if (senderMem) {
				senderMem->Close();
				delete senderMem;
			}
			senderMem = nullptr;
			m_Width = 0;
			m_Height = 0;
		} // end CloseSenderMemory

		// Lock and unlock memory and retrieve buffer pointer - no size checks
		unsigned char * LockSenderMemory()
		{
			if (!senderMem) return nullptr;
			char *pBuf = senderMem->Lock();
			if (!pBuf) {
				// https://github.com/leadedge/Spout2/issues/15
				// senderMem->Unlock();
				return nullptr;
			}
			return reinterpret_cast<unsigned char *>(pBuf);
		}

		void UnlockSenderMemory()
		{
			if (!senderMem) return;
			senderMem->Unlock();
		}

		unsigned int GetSenderMemorySize()
		{
			return m_Width * m_Height;
		}

		const char* GetSenderMemoryName()
		{
			if (!senderMem)
				return nullptr;
			return senderMem->Name();
		}

		bool GetSenderMemory(const char *sendername)
		{
			if (sendername[0] == 0)
				return false;

			return senderMem->Open(sendername);
		}

	} memoryshare;

	//
	// Public for external access
	//

	spoutSenderNames sendernames;
	spoutFrameCount frame;
	spoutDirectX spoutdx;
	spoutCopy spoutcopy;

	//
	// Options used for SpoutCam
	//

	// Mirror image
	bool m_bMirror;
	// RGB <> BGR
	bool m_bSwapRB;

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
	bool m_bSpoutInitialized;
	bool m_bSpoutPanelOpened;
	bool m_bSpoutPanelActive;
	bool m_bClassDevice;
	bool m_bAdapt;
	SHELLEXECUTEINFOA m_ShExecInfo;

	bool CheckSender(unsigned int width, unsigned int height, DWORD dwFormat);
	ID3D11Texture2D* CheckSenderTexture(char *sendername, HANDLE dxShareHandle);

	bool ReceiveSenderData();
	void CreateReceiver(const char * sendername, unsigned int width, unsigned int height, DWORD dwFormat);
	
	// Read pixels via staging texture
	bool ReadPixelData(ID3D11Texture2D* pStagingTexture, unsigned char* pixels, unsigned int width, unsigned int height, bool bRGB, bool bInvert);
	bool CheckStagingTextures(unsigned int width, unsigned int height, DWORD dwFormat = DXGI_FORMAT_B8G8R8A8_UNORM);
	bool CreateDX11StagingTexture(unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11Texture2D** pStagingTexture);

	void SelectSenderPanel();
	bool CheckSpoutPanel(char *sendername, int maxchars = 256);
	bool CreateMemoryBuffer(const char *sendername, unsigned int length);

};

#endif
