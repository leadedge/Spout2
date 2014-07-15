/*

	spoutSenderNames.h

	LJ - leadedge@adam.com.au

	Spout sender management

	= multiple Senders - a set of Sender names =
	Revisions :

	25.04.14 - started
	27.05.14 - increased MaxSenders from 10 to 20
	08.06.14 - rebuild
	11.07.14 - major re-write


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
#ifndef __spoutSenderNames__ // standard way as well
#define __spoutSenderNames__

#include <windowsx.h>
#include <d3d9.h>
#include <wingdi.h>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "spoutMemoryShare.h"

#define SPOUT_WAIT_TIMEOUT 100 // 100 msec wait for events
#define MaxSenders 10 // Max for list of Sender names

// The texture information structure that is saved to shared memory
// and used for communication between senders and receivers
// unsigned __int32 is used for compatibility between 32bit and 64bit
// See : http://msdn.microsoft.com/en-us/library/windows/desktop/aa384203%28v=vs.85%29.aspx
// This is also compatible with wyphon : 
// The structure is declared here so that this class is can be independent of opengl
struct SharedTextureInfo {
	unsigned __int32 shareHandle;
	unsigned __int32 width;
	unsigned __int32 height;
	DWORD format; // Texture pixel format
	DWORD usage; // not used
	wchar_t description[128]; // Wyhon compatible description (not used)
	unsigned __int32 partnerId; // Wyphon id of partner that shared it with us (not unused)
};

using namespace std;

class spoutSenderNames {

	public:

		spoutSenderNames();
		~spoutSenderNames();

		// public functions

		// ------------------------------------------------------------
		// Registration and find sender functions
		bool RegisterSenderName (char* Sendername);
		bool ReleaseSenderName  (char* Sendername);
		bool FindSenderName		(char* Sendername);

		// ------------------------------------------------------------
		// Functions to retrieve info about the sender set map and the senders in it
		int  GetSenderCount();
		bool GetSenderNames	   (std::set<string> *Sendernames);
		bool GetSenderNameInfo (int index, char* sendername, int sendernameMaxSize, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle);
		bool GetImageSize	   (char* sendername, unsigned int &width, unsigned int &height, bool &bMemoryMode);

		// ------------------------------------------------------------
		// Functions to read and write info to a sender memory map
		bool GetSenderInfo (char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat);
		bool SetSenderInfo (char* sendername, unsigned int width, unsigned int height, HANDLE dxShareHandle, DWORD dwFormat);

		// ------------------------------------------------------------
		// Functions to maintain the active sender
		bool SetActiveSender	 (char* Sendername);
		bool GetActiveSender	 (char* Sendername);
		bool GetActiveSenderInfo (SharedTextureInfo* info);

		// ------------------------------------------------------------
		// Functions to Create, Find, Update and Close a sender without initializing DirectX or the GL/DX interop functions
		bool CreateSender (char *sendername, unsigned int width = 0, unsigned int height = 0, HANDLE hSharehandle = NULL, DWORD dwFormat = 0);
		bool UpdateSender (char *sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat = 0);
		bool CloseSender  (char* sendername);
		
		bool FindSender		  (char *sendername, unsigned int &width, unsigned int &height, HANDLE &hSharehandle, DWORD &dwFormat);
		bool CheckSender	  (char *sendername, unsigned int &width, unsigned int &height, HANDLE &hSharehandle, DWORD &dwFormat);
		bool FindActiveSender (char *activename, unsigned int &width, unsigned int &height, HANDLE &hSharehandle, DWORD &dwFormat);
		// ------------------------------------------------------------

		// Utility functions
		bool SenderChanged(char *theSendername, int theWidth, int theHeight, DWORD dwFormat, HANDLE theShareHandle);

		// Access event locks
		bool	InitEvents	(char *eventname, HANDLE &hReadEvent, HANDLE &hWriteEvent);
		void	CloseEvents (HANDLE &hReadEvent, HANDLE &hWriteEvent);
		bool	CheckAccess (HANDLE hEvent);
		void	AllowAccess (HANDLE hReadEvent, HANDLE hWriteEvent);

		HANDLE	CreateMap		 (char* MapName, int MapSize);
		char*	OpenMap			 (char* MapName, int MapSize, HANDLE &hMap);
		void	CloseMap		 (char* MapBuffer, HANDLE hMap);

		bool SenderDebug(char *Sendername, int size);

	protected:

		spoutMemoryShare MemoryShare;	// Shared memory method

		// ------------------------------------------------------------
		// Functions to manage shared memory map creation and access
		//
		// Shared memory map management
		//
		// The way it works :
		//
		//			Creating the map
		// A map with a given name and size is created (CreateMap) and a handle to the map is returned. 
		// The creation handle is saved in a std::map keyed by the map name
		// This map is local to this instance and each instance will manage it's own list of senders
		// Within this function a matching named mutex is created (CreateMapLock) to lock and unlock access to the map
		//
		//			Using the map
		// The required, named map is locked (LockMap). Within this function the map mutex is checked  and if it
		// does not exist, the function returns NULL. If it does exist there is a wait of 4 frames for access.
		// The map is then opened (OpenMap) and a handle and pointer to the map buffer are returned for either
		// read or write to the map.
		// After map access it is closed (CloseMap), which closes it but does not release it.
		// Finally it is unlocked (UnlockMap)
		//
		//			Releasing the map
		// The map of the given name is released (ReleaseMemoryMap) by finding
		// the handle paired to it's name in the std::map of handles
		// At that time all other accesses to the memory map will have 
		// had the handle for access closed, so this will be the last one
		// and will release the memory.
		// The matching named mutex (ReleaseMapLock) is also released.
		//
		HANDLE	CreateMemoryMap	 (char *MapName, int MapSize);
		bool	ReleaseMemoryMap (char* MapName);

		// The map of map handles
		bool	GetHandleMap (char* MapName, std::map<std::string, HANDLE> &MapHandles);
		bool	SetHandleMap (char* MapName, std::map<std::string, HANDLE> MapHandles);

		// Memory map mutex locks
		bool	CreateMapLock  (char *mapname);
		void	ReleaseMapLock (char *mapname);
		bool	LockMap        (char *mapname, HANDLE &hMutex);
		void	UnlockMap      (HANDLE hMutex);

		// Sender name set management
		bool CreateSenderSet();
		bool GetSenderSet		 (std::set<string>& SenderNames);
		bool SetSenderSet		 (std::set<string>& Sendernames);
		bool RemoveSender		 (char* Sendername);
		bool setActiveSenderName (char* SenderName);
		bool getActiveSenderName (char* SenderName);
		bool getSharedInfo		 (char* SenderName, SharedTextureInfo* info);


};

#endif
