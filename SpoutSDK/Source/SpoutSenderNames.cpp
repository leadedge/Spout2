/**

	spoutSenderNames.cpp

	LJ - leadedge@adam.com.au

	Spout sender management

	25.04.14 - started class file
	27.05.14 - cleanup using memory map creation, open, close, lock
	05.06.14 - FindSenderName - allow for a null name entered
	08.06.14 - rebuild
	12.06.13 - major revision, included map handling
	23-07-14 - cleanup of DX9 / DX11 functions
			 - Changed CheckSender logic
	27.07-14 - changed mutex lock creation due to memory leak
	28-07-14 - major change
			 - remove handle management
			 - changed map creation and release
	30-07-14 - Map locks and cleanup
	31-07-14 - fixed duplicate names class object
	01-08-14 - fixed mutex handle leak / cleanup

		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Copyright (c) 2014, Lynn Jarvis. All rights reserved.

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
		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

*/
#include "spoutSenderNames.h"

spoutSenderNames::spoutSenderNames() {


	m_hSenderMutex = NULL;
	m_hActiveSenderMutex = NULL;
	m_hSenderNamesMutex = NULL;
	
	m_hSenderMap = NULL;
	m_hActiveSenderMap = NULL;
	m_hSenderNamesMap = NULL;
	
	m_pSenderMap = NULL;
	m_pActiveSenderMap = NULL;
	m_pSenderNamesMap = NULL;

}

spoutSenderNames::~spoutSenderNames() {

}

//
// =========================
// multiple Sender functions
// ========================= 
//
// Register a new Sender by adding to the list of Sender names
//
bool spoutSenderNames::RegisterSenderName(const char* Sendername) {

	std::pair<std::set<string>::iterator, bool> ret;
	std::set<string> SenderNames; // set of names

	// Create the shared memory for the sender name set if it does not exist
	if(!GetSenderSet(SenderNames)) {
		if(!CreateSenderSet()) {
			return false;
		}
		// The new memory map is empty so we don't need to get it
	}

	//
	// Add the Sender name to the set of names
	//
	ret = SenderNames.insert(Sendername);
	if(ret.second) {
		// write the new map to shared memory
		SetSenderSet(SenderNames);
		// Set as the active Sender if it is the first one registered
		// Thereafter the user can select an active Sender using SpoutPanel or SpoutSenders
		if(SenderNames.size() == 1) {
			// printf("    *** FIRST SENDER ***\n");
			// Create a memory map to set the active Sender name to shared memory
			// This is a separate small shared memory with a fixed sharing name
			// that Receivers can use to retrieve the current active Sender
			// CreateMap will either create a new map or open the map if it exists
			m_pActiveSenderMap = CreateMap("ActiveSenderName", 256, m_hActiveSenderMap);

			// Create a named mutex for map locks
			CreateMapLock("ActiveSenderName", m_hActiveSenderMutex);

			// Set the current sender name as active
			SetActiveSender(Sendername);  

		}
		else {
			// If it is not the first sender, open a view of the maps 
			// so they do not get closed by another instance
			m_pActiveSenderMap = OpenMap("ActiveSenderName", 256, m_hActiveSenderMap);
			m_pSenderNamesMap = OpenMap("SpoutSenderNames", MaxSenders*256, m_hSenderNamesMap);
		}
	}

	return ret.second;
}


// Function to release a Sender name from the list of names
// Removes the Sender name and if it was the last one, 
// closes the shared memory map for the Sender name list
// See also RemoveSender
bool spoutSenderNames::ReleaseSenderName(const char* Sendername) 
{
	string namestring;
	std::set<string> SenderNames; // set of names
	std::map<std::string, HANDLE> maphandles;
	std::set<string>::iterator iter;
	bool bRet = false;

	// printf("**** CLOSE SENDER ****\n", Sendername);

	// Get the current list to update the passed list
	if(GetSenderSet(SenderNames)) {
		if (SenderNames.size() > 0) {
			// RemoveSender removes the sender from the name set,
			// deletes it's shared memory map and removes it from the handles map
			if (RemoveSender(Sendername)) { 
				// The name existed when it was removed from the list
				// its shared memory map is now also removed
				// and if it was the active sender, that is updated if more senders exist.
				GetSenderSet(SenderNames);
				if (SenderNames.size() == 0) {
					// printf("    LAST SENDER\n");

					// If it is the last sender, the names map and mutex will be closed
					// as long as there are no open views left
					// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366537%28v=vs.85%29.aspx
					// But to be sure they can be closed here

					CloseMap(m_pSenderNamesMap, m_hSenderNamesMap);
					CloseMapLock(m_hSenderNamesMutex);
					// Important to null the pointer and handle
					m_pSenderNamesMap = NULL;
					m_hSenderNamesMap = NULL;
					m_hSenderNamesMutex = NULL;

					CloseMap(m_pActiveSenderMap, m_hActiveSenderMap);
					CloseMapLock(m_hActiveSenderMutex);
					m_pActiveSenderMap = NULL;
					m_hActiveSenderMap = NULL;
					m_hActiveSenderMutex = NULL;

				}
				bRet = true;
			}
		}
	}

	return bRet; // the Sender name did not exist

} // end ReleaseSenderName


//
// Removes a Sender from the set of Sender names
//
bool spoutSenderNames::RemoveSender(const char* Sendername) 
{
	std::set<string> SenderNames;
	std::set<string>::iterator iter;
	string namestring;
	char name[256];

	// Discovered that the project properties had been set to CLI
	// Properties -> General -> Common Language Runtime Support
	// and this caused the set "find" function not to work.
	// It also disabled intellisense.
	// printf("spoutSenderNames::ReleaseSenderName(%s)\n", Sendername);

	// Get the current map to update the list
	if(GetSenderSet(SenderNames)) {
		if(SenderNames.find(Sendername) != SenderNames.end() ) {
			// printf("Releasing sender [%s]\n", Sendername);
			SenderNames.erase(Sendername); // erase the matching Sender

			// Must be here otherwise the set is retrieved again if there was only one sender
			SetSenderSet(SenderNames); // set the map back to shared memory again

			// The sender is created by this instance
			// On application close, the memory map is released as long as there are no active views
			// But for repeated context change and sender reset, the map handle must be closed
			// or there is a leak.
			CloseMap(m_pSenderMap, m_hSenderMap);
			m_pSenderMap = NULL;
			m_hSenderMap = NULL;

			// Close the named mutex for this sender
			CloseMapLock(m_hSenderMutex);
			m_hSenderMutex = NULL;

			// Is there a set left ?
			if(SenderNames.size() > 0) {
				// This should be OK because the user selects the active sender
				// Was it the active sender ?
				if( (getActiveSenderName(name) && strcmp(name, Sendername) == 0) || SenderNames.size() == 1) { 
					// It was, so choose the first in the list
					iter = SenderNames.begin();
					namestring = *iter;
					strcpy_s(name, 256, namestring.c_str());
					// Set it as the active sender
					setActiveSenderName(name);
				}
			}
			return true;
		}
	}

	return false; // Sender name not in the set or no set in shared mempry

} // end RemoveSender



// Test to see if the Sender name exists
bool spoutSenderNames::FindSenderName(const char* Sendername)
{
	string namestring;
	std::set<string> SenderNames;
	
	if(Sendername[0]) { // was a valid name passed
		// Get the current list to update the passed list
		if(GetSenderSet(SenderNames)) {
			// Does the name exist
			if(SenderNames.find(Sendername) != SenderNames.end() ) {
				return true;
			}
		}
	}

	return false;
}


// Function to return the set of Sender names in shared memory.
bool spoutSenderNames::GetSenderNames(std::set<string> *Sendernames)
{
	string namestring;
	std::set<string> SenderNames;
	std::set<string>::iterator iter;

	// Get the current list to update the passed list
	if(GetSenderSet(SenderNames)) {
		for(iter = SenderNames.begin(); iter != SenderNames.end(); iter++) {
			namestring = *iter; // the string to copy
			// printf("    Name : %s\n", namestring.c_str());
			Sendernames[0].insert(namestring);
		}
		return true;
	}

	return false;
}


int spoutSenderNames::GetSenderCount() {

	std::set<string> SenderSet;
	std::set<string>::iterator iter;
	string namestring;
	char name[256];
	SharedTextureInfo info;

	// get the name list in shared memory into a local list
	GetSenderNames(&SenderSet);

	// Now we have a local set of names
	// 27.12.13 - noted that if a Processing sketch is stopped by closing the window
	// all is OK and either the "stop" or "dispose" overrides work, but if STOP is used, 
	// or the sketch is closed, neither the exit or dispose functions are called and
	// the sketch does not release the sender.
	// So here we run through again and check whether the sender exists and if it does not
	// release the sender from the local sender list
	if(SenderSet.size() > 0) {
		for(iter = SenderSet.begin(); iter != SenderSet.end(); iter++) {
			namestring = *iter; // the Sender name string
			strcpy_s(name, namestring.c_str());
			// we have the name already, so look for it's info
			if(!getSharedInfo(name, &info)) {
				// Sender does not exist any more
				ReleaseSenderName(name); // release from the shared memory list
			}
		}
	}

	// Get the new set back
	if(GetSenderNames(&SenderSet)) {
		return(SenderSet.size());
	}

	return 0;
}


// Get sender info given a sender index and knowing the sender count
// index						- in
// sendername					- out
// sendernameMaxSize			- in
// width, height, dxShareHandle - out
bool spoutSenderNames::GetSenderNameInfo(int index, char* sendername, int sendernameMaxSize, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle)
{
	char name[256];
	std::set<string> SenderNameSet;
	std::set<string>::iterator iter;
	string namestring;
	int i;
	DWORD format;

	if(GetSenderNames(&SenderNameSet)) {
		if(SenderNameSet.size() < (unsigned int)index)
			return false;

		i = 0;
		for(iter = SenderNameSet.begin(); iter != SenderNameSet.end(); iter++) {
			namestring = *iter; // the name string
			strcpy_s(name, 256, namestring.c_str()); // the 256 byte name char array
			if(i == index) {
				strcpy_s(sendername, sendernameMaxSize, name); // the passed name char array
				break;
			}
			i++;
		}
		
		// Does the retrieved sender exist or has it crashed?
		// Find out by getting the sender info and returning it
		if(GetSenderInfo(sendername, width, height, dxShareHandle, format))
			return true;

	}

	return false;

} // end GetSenderNameInfo



// This retrieves the info from the requested sender and fails if the sender does not exist
// For external access to getSharedInfo - redundancy
bool spoutSenderNames::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	SharedTextureInfo info;

	if(getSharedInfo(sendername, &info)) {
		width		  = (unsigned int)info.width;
		height		  = (unsigned int)info.height;
		dxShareHandle = (HANDLE)info.shareHandle;
		dwFormat      = info.format;
		return true;
	}
	return false;
}


//
// Set texture info to a sender shared memory map without affecting the 
// interop class globals used for GL/DX interop texture sharing
// TODO - use pointer from initial map creation
bool spoutSenderNames::SetSenderInfo(const char* sendername, unsigned int width, unsigned int height, HANDLE dxShareHandle, DWORD dwFormat) 
{
	SharedTextureInfo info;
	HANDLE hMap; // handle to the shared memory map
	char* pBuf; // pointer to the memory map
	HANDLE hLock;

	LockMap(sendername, hLock);
	
	info.width			= (unsigned __int32)width;
	info.height			= (unsigned __int32)height;
	info.shareHandle	= (unsigned __int32)dxShareHandle; 
	info.format			= (unsigned __int32)dwFormat;
	// Usage not used

	// Open the named memory map for the sender and return a pointer to the memory
	pBuf = OpenMap(sendername, sizeof(SharedTextureInfo), hMap );
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}
	memcpy((void *)pBuf, (void *)&info, sizeof(SharedTextureInfo) );
	
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end SetSenderInfo



// Functions to set or get the active Sender name
// The "active" Sender is the one of the multiple Senders
// that is top of the list or is the one selected by the user from this list. 
// This active Sender information is saved in a separated shared
// memory from other Senders, identified by the name "ActiveSenderName"
// so it can be recalled at any time by clients if the user
// has selected a required Sender from a dialog or executable.
// The dialog or executable sets the info of the selected Sender
// into the ActiveSender shared memory so the clients can picks it up.
//  !!! The active Sender has to be a member of the Sender list !!!
bool spoutSenderNames::SetActiveSender(const char *Sendername)
{
	std::set<string> SenderNames;

	// Get the current list to check whether the passed name is in it
	if(GetSenderSet(SenderNames)) {
		if(SenderNames.find(Sendername) != SenderNames.end() ) {
			if(setActiveSenderName(Sendername)) { // set the active Sender name to shared memory
				return true;
			}
		}
	}
	return false;
} // end SetActiveSender


// Function for clients to retrieve the current active Sender name
bool spoutSenderNames::GetActiveSender(char* Sendername)
{
	char ActiveSender[256];
	SharedTextureInfo info;

	if(getActiveSenderName(ActiveSender)) {
		// Does it still exist ?
		if(getSharedInfo(ActiveSender, &info)) {
			strcpy_s(Sendername, 256, ActiveSender);
			return true;
		}
	}
	
	return false;
} // end GetActiveSender



// Function for clients to get the shared info of the active Sender
bool spoutSenderNames::GetActiveSenderInfo(SharedTextureInfo* info)
{
	string namestring;

	// See if the shared memory of the active Sender exists
	if(!getSharedInfo("ActiveSenderName", info)) {
		// printf("GetActiveSenderInfo - Could not get active sender info\n");
		return false;
	}
	// It should exist because it is set whenever a Sender registers
	return true;
} // end GetActiveSenderInfo



//
// Retrieve the texture info of the active sender
// - redundancy 
bool spoutSenderNames::FindActiveSender(char *sendername, unsigned int &theWidth, unsigned int &theHeight, HANDLE &hSharehandle, DWORD &dwFormat)
{
    SharedTextureInfo TextureInfo;
	char sname[256];

    if(GetActiveSender(sname)) { // there is an active sender
		if(getSharedInfo(sname, &TextureInfo)) {
			strcpy_s(sendername, 256, sname); // pass back sender name
			theWidth        = (unsigned int)TextureInfo.width;
			theHeight       = (unsigned int)TextureInfo.height;
			hSharehandle	= (HANDLE)TextureInfo.shareHandle;
			dwFormat        = (DWORD)TextureInfo.format;
			return true;
		}
	}

    return false;

} // end FindActiveSender


/////////////////////////////////////////////////////////////////////////////////////
// Functions to Create, Update and Close a sender and retrieve sender texture info //
// without initializing DirectX or the GL/DX interop functions                     //
/////////////////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------
//	Create a sender with the info of a shared DirectX texture
//		1) Create a new named sender shared memory map
//		2) Set the sender texture info to the map
//		3) Register the sender name in the list of Spout senders
//
//	This sender is specific to this instance. 
//	There cannot be more than one sender per object.
//
// ---------------------------------------------------------
bool spoutSenderNames::CreateSender(const char *sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat)
{
	string namestring;
	// HANDLE hMap;
	// char *pBuf;

	// printf("CreateSender - %s, %dx%d, [%x] [%d]\n", sendername, width, height, hSharehandle, dwFormat);

	/*
	// Is the sender of the same name already running ?
	// Problem here with Max sender if there has been a context
	// change and the sender is released and recreated.
	// needs debugging	
	pBuf = OpenMap(sendername, 256, hMap);
	if(pBuf) {
		char temp[512];
		// Serious enough for a messagebox
		sprintf_s(temp, "Cannot create sender\n(%s)\nIs one already running?", 512, sendername);
		MessageBoxA(NULL, temp, "Spout", MB_OK);
		CloseMap(pBuf, hMap);
		return false;
	}
	*/

	// Create or open a shared memory map for this sender - allocate enough for the texture info
	m_pSenderMap = CreateMap(sendername, sizeof(SharedTextureInfo), m_hSenderMap);
	if(!m_pSenderMap) {
		return false;
	}

	// Create a named mutex for map locks
	CreateMapLock(sendername, m_hSenderMutex);

	// Register the sender name in the list of spout senders
	RegisterSenderName(sendername);
	
	// TODO - createsender with just a name
	if(width > 0 && height > 0) {
		// Save the info for this sender in the shared memory map
		if(!SetSenderInfo(sendername, width, height, hSharehandle, dwFormat)) {
			return false;
		}
	}

	return true;
		
} // end CreateSender


// ---------------------------------------------------------
//	Update the texture info of a sender
//	Used for example when a sender's texture changes size
// ---------------------------------------------------------
bool spoutSenderNames::UpdateSender(const char *sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat)
{
	// Save the info for this sender in the sender shared memory map
	if(!SetSenderInfo(sendername, width, height, hSharehandle, dwFormat))
		return false;

	return true;
		
} // end UpdateSender



// ---------------------------------------------------------
//	Close a sender - for external access
//	See - ReleaseSenderName - redundancy or reorganise
// ---------------------------------------------------------
bool spoutSenderNames::CloseSender(const char* sendername)
{
	ReleaseSenderName(sendername);
	return true;
}



// ===============================================================================
//	Functions to retrieve information about the shared texture of a sender
//
//	Possible detection by the caller of DX9 or DX11 sender from the Format field
//	Format is always fixed as D3DFMT_A8R8G8B8 for a DirectX9 sender and Format is set to 0
//	For a DirectX11 sender, the format field is set to the DXGI_FORMAT texture format 
//	Usage is fixed :
//		DX9  - D3DUSAGE_RENDERTARGET
//		DX11 - D3D11_USAGE_DEFAULT 
// ===============================================================================

// Find a sender and return the name, width and height, sharhandle and format
bool spoutSenderNames::FindSender(char *sendername, unsigned int &width, unsigned int &height, HANDLE &hSharehandle, DWORD &dwFormat)
{
	SharedTextureInfo info;

	// ---------------------------------------------------------
	//	For a receiver check the user entered Sender name, if one, to see if it exists
	if(sendername[0]) {
		// Is the given sender registered ?
		if(!FindSenderName(sendername)) {
			return false;
		}
	}
	else {
		// Passed name was null, so find the active sender
		if(!GetActiveSender(sendername)) {
			return false;
		}
	}
	// now we have either an existing sender name or the active sender name

	// Try to get the sender information
	if(getSharedInfo(sendername, &info)) {
		width			= (unsigned int)info.width; // pass back sender size
		height			= (unsigned int)info.height;
		hSharehandle	= (HANDLE)info.shareHandle;
		dwFormat		= (DWORD)info.format;
		return true;
	}

	return false;

} // end FindSender


//
//	Check the details of an existing sender
//
//	1) Find the sender
//	2) Get it's texture info
//	3) Return the sharehandle, width, height, and format
//
//	Returns :
//		true	- all OK.
//			width and height are returned changed for sender size change
//		false	- sender not found or size changed
//			width and height are returned zero for sender not found
//
bool spoutSenderNames::CheckSender(const char *sendername, unsigned int &theWidth, unsigned int &theHeight, HANDLE &hSharehandle, DWORD &dwFormat)
{
	SharedTextureInfo info;
	char sname[256];

	// Is the given sender registered ?
	if(FindSenderName(sendername)) {
		// Does it still exist ?
		if(getSharedInfo(sendername, &info)) {
			// Return the texture info
			// strcpy_s(sendername, 256, sname);
			theWidth		= (unsigned int)info.width;
			theHeight		= (unsigned int)info.height;
			hSharehandle	= (HANDLE)info.shareHandle;
			dwFormat		= (DWORD)info.format;
			return true;
		}
		else {
			// Sender is registered but does not exist so close it
			ReleaseSenderName(sname);
		}
	}
	
	// Return zero width and height to indicate sender not found
	theHeight = 0;
	theWidth  = 0;

	return false;

} // end CheckSender
// ==================



//
// Functions to manage creating, releasing, opening and closing of named memory maps
//
char* spoutSenderNames::CreateMap(const char* MapName, int MapSize, HANDLE &hMap)
{
	HANDLE hMapFile;
	char* pBuf;
	DWORD errnum;

	// Set up Shared Memory
	// Must create the file mapping to the maximum size 
	// needed because it can't be changed afterwards
	hMapFile = CreateFileMappingA (	INVALID_HANDLE_VALUE,	// hFile - use paging file
								NULL,						// LPSECURITY_ATTRIBUTES - default security 
								PAGE_READWRITE,				// flProtect - read/write access
								0,							// The high-order DWORD - dwMaximumSizeHigh - max. object size 
								MapSize,					// The low-order DWORD - dwMaximumSizeLow - buffer size  
								(LPCSTR)MapName);			// name of mapping object
	
	if (hMapFile == NULL) {
		// printf("	CreateMap null handle (%s)\n", MapName);
		return NULL;
	}

	errnum = GetLastError();
	// printf("	CreateMap (%s) GetLastError = %d\n", MapName, errnum);

	if(errnum == ERROR_INVALID_HANDLE) {
		// printf("	CreateMap (%s) invalid handle\n", MapName);
	}

	if(errnum == ERROR_ALREADY_EXISTS) {
		// printf("	CreateMap [%s][%x] already exists\n", MapName, hMapFile);
	}
	else {
		// printf("	CreateMap [%s][%x] NEW MAP\n", MapName, hMapFile);
	}

	pBuf = (char *) MapViewOfFile(hMapFile,				// handle to map object
								  FILE_MAP_ALL_ACCESS,	// read/write permission
								  0,
								  0,
								  MapSize );			// 
	
	if (pBuf == NULL) {
		DWORD errnum = GetLastError(); 
		if(errnum > 0) {
			// printf("    CreateMap (%s) : GetLastError() = %d\n", MapName, errnum);
			// ERROR_INVALID_HANDLE 6 (0x6) The handle is invalid.
		}
		// CloseHandle(hMapFile); // Is this OK if the view failed
		// hMap = NULL;
		return NULL;
	}

	hMap = hMapFile;

	return pBuf;

}


char* spoutSenderNames::OpenMap(const char* MapName, int MapSize, HANDLE &hMap)
{
	char* pBuf;
	HANDLE hMapFile;

	hMapFile = OpenFileMappingA (FILE_MAP_ALL_ACCESS, // read/write access
								 FALSE,				  // do not inherit the name
								 MapName);			  // name of mapping object

	if (hMapFile == NULL) {
		// no map file means no sender is present
		return NULL;
	}
	pBuf = (char *) MapViewOfFile(hMapFile,				// handle to map object
								  FILE_MAP_ALL_ACCESS,	// read/write permission
								  0,
								  0,
								  MapSize );			// 
	if (pBuf == NULL) {
		DWORD errnum = GetLastError(); 
		if(errnum > 0) {
			// printf("    OpenMap (%s) : GetLastError() = %d\n", MapName, errnum);
			// ERROR_INVALID_HANDLE 6 (0x6) The handle is invalid.
		}
		if(hMapFile) CloseHandle(hMapFile);
		hMap = NULL;
		return NULL;
	}

	hMap = hMapFile;

	return pBuf;
	
} // end OpenMap



// Here we can unmap the view of the map but not close the handle by sending a null handle
// or close the map by sending a valid handle but NULL buffer pointer
void spoutSenderNames::CloseMap(const char* MapBuffer, HANDLE hMap)
{
	if(MapBuffer) {
		UnmapViewOfFile((LPCVOID)MapBuffer);
	}

	// When the process no longer needs access to the file mapping object, it should call the CloseHandle function. 
	// When all handles are closed, the system can free the section of the paging file that the object uses.
	if(hMap != NULL) {
		CloseHandle(hMap); // Handle is closed but not released
	}

}  // end CloseMap



// ==========================================================================
//	Event locks used to control read/write on top of interop object lock
//	Events are created or opended by different processes.
//	https://en.wikipedia.org/wiki/Readers-writers_problem
//
//	LJ DEBUG - Used in SpoutSDK.cpp. Disabled until there is evidence of a problem with the interop lock
//
bool spoutSenderNames::InitEvents(const char *eventname, HANDLE &hReadEvent, HANDLE &hWriteEvent)
{
	DWORD errnum;
	char szReadEventName[256];	// name of the read event
	char szWriteEventName[256];	// name of the write event

	// Create or open events to control access to the shared texture
	sprintf_s((char*)szReadEventName,  256, "%s_SpoutReadEvent", eventname);
	sprintf_s((char*)szWriteEventName, 256, "%s_SpoutWriteEvent", eventname);

	// Create or open read event depending, on whether it already exists or not
    hReadEvent = CreateEventA (	NULL,				// default security
								false,				// auto reset
								true,				// default state signaled
								(LPSTR)szReadEventName);

	if (hReadEvent == NULL) {
		// printf("	CreateEvent : Read : failed\n");
		hReadEvent = hWriteEvent = NULL;
        return false;
	}
	else {
		errnum = GetLastError();
		// printf("read event GetLastError() = %d\n", errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			// printf("	read event invalid handle\n");
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			// printf("	read event already exists\n");
		}
		else {
			// printf("	read event created\n");
		}
	}

    // Create or open write event, depending on whether it already exists or not
	hWriteEvent = CreateEventA(NULL,					// default security
								false,					// auto reset
								true,					// default state signaled
								(LPSTR)szWriteEventName);
     
    if (hWriteEvent == NULL) {
		// printf("	CreateEvent : Write : failed\n");
		CloseHandle(hReadEvent);
		hReadEvent = hWriteEvent = NULL;
        return false;
	}
	else {
		errnum = GetLastError();
		// printf("write event GetLastError() = %d\n", errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			// printf("	write event invalid handle\n");
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			// printf("	write event already exists\n");
		}
		else {
			// printf("	write event created\n");
		}
	}

	// Returns the event handles
	// printf("	handles [%04x] [%04x]\n", hReadEvent, hWriteEvent);

	return true;

} // end InitEvents


void spoutSenderNames::CloseEvents(HANDLE &hReadEvent, HANDLE &hWriteEvent)
{
	if(hReadEvent) CloseHandle(hReadEvent);
	if(hWriteEvent) CloseHandle(hWriteEvent);
	hReadEvent = NULL;
	hWriteEvent = NULL;
}

//
//		Texture event locks
//
bool spoutSenderNames::CheckAccess(HANDLE hEvent)
{
	DWORD dwWaitResult;

	if(hEvent == NULL) {
		return false;
	}

	dwWaitResult = WaitForSingleObject(hEvent, SPOUT_WAIT_TIMEOUT );
	if(dwWaitResult == SPOUT_WAIT_TIMEOUT) { // Timeout problem
		// The time-out interval elapsed, and the object's state is nonsignaled.
		// printf("CheckAccess : Timeout waiting for event\n");
		return false;
	}
	else if (dwWaitResult == WAIT_OBJECT_0 ) {
		// The state of the object is signaled.
		return true;
	}
	else {
		switch(dwWaitResult) {
			case WAIT_ABANDONED : // Could return here
				// printf("CheckAccess : WAIT_ABANDONED\n");
				break;
			case SPOUT_WAIT_TIMEOUT : // The time-out interval elapsed, and the object's state is nonsignaled.
				// printf("CheckAccess : SPOUT_WAIT_TIMEOUT\n");
				break;
			case WAIT_FAILED : // Could use call GetLastError
				// printf("CheckAccess : WAIT_FAILED\n");
				break;
			default :
				break;
		}

	}
	return false;

}

void spoutSenderNames::AllowAccess(HANDLE hReadEvent, HANDLE hWriteEvent)
{
	// Set the Write Event to signal readers to read
	if(hWriteEvent != NULL) {
		SetEvent(hWriteEvent);
	}

    // Set the Read Event to signal the writer it can write
    if(hReadEvent != NULL) {
		SetEvent(hReadEvent);
	}

}
// ================================================


///////////////////////////////////////////////////
// Private functions for multiple Sender support //
///////////////////////////////////////////////////

//
//  Functions to read and write the list of Sender names to/from shared memory
//

// Create a shared memory map and copy the Sender names set to shared memory
bool spoutSenderNames::CreateSenderSet() 
{

	// Set up Shared Memory for all the sender names
	m_pSenderNamesMap = CreateMap("SpoutSenderNames", MaxSenders*256, m_hSenderNamesMap);
	if(!m_pSenderNamesMap) {
		return false;
	}

	// create a map lock mutex for the name set
	CreateMapLock("SpoutSenderNames", m_hSenderNamesMutex); 

	return true;

} // end CreateSenderSet


// TODO - use pointer from initial map creation
bool spoutSenderNames::GetSenderSet(std::set<string>& SenderNames) {

	int i;
	string namestring;	// local string to retrieve names
	char name[256];		// char array to test for nulls
	char *pBuf;			// pointer to shared memory buffer
	HANDLE hMap;		// handle to shared memory
	HANDLE hLock;
	char *buffer;		// local buffer for data transfer
	char *buf;			// pointer within the buffer
	std::set<string>::iterator iter;

	// Lock the map
	LockMap("SpoutSenderNames", hLock);

	pBuf = OpenMap("SpoutSenderNames", MaxSenders*256, hMap );
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}

	// The data has been stored with 256 bytes reserved for each Sender name
	// and nothing will have changed with the map yet
	buffer = (char *)malloc(MaxSenders*256*sizeof(unsigned char));
	memset( (void *)buffer, 0, MaxSenders*256 ); // make sure it is clear because we rely on nulls

	// copy the shared memory to the local buffer
	memcpy ( (void *)buffer, (void *)pBuf, MaxSenders*256 );

	// Read back from the buffer and rebuild the set
	
	// first empty the set
	if(SenderNames.size() > 0) {
		SenderNames.erase (SenderNames.begin(), SenderNames.end() );
	}

	buf = (char *)buffer;
	i = 0;
	do {
		// the actual string retrieved from shared memory should terminate
		// with a null within the 256 chars.
		// At the end of the map there will be a null in the data.
		// Must use a character array to ensure testing for null.
		strncpy_s(name, buf, 256);
		if(name[0] > 0) {
			// printf("    Retrieving %s\n", name);
			// insert name into set
			// seems OK with a char array instead of converting to a string first
			SenderNames.insert(name);
		}
		// increment by 256 bytes for the next name
		buf += 256;
		i++;
	} while(name[0] > 0 && i < MaxSenders);

	free((void *)buffer);

	// Close the open memory map
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end GetSenderSet



// Copy the Sender names set to shared memory which must exist first
// TODO - use pointer from initial map creation - can be done because this is local
bool spoutSenderNames::SetSenderSet(std::set<string>& SenderNames) 
{
	char* pBuf;
	HANDLE hMap;
	int i;
	string namestring;	// local string to retrieve names
	char* buffer;		// local buffer for data transfer
	char* buf;			// pointer within the buffer
	std::set<string>::iterator iter;
	HANDLE hLock;

	// Lock the shared memory map with it's mutex - wait maximum 4 frames for access
	LockMap("SpoutSenderNames", hLock);

	pBuf = OpenMap("SpoutSenderNames", MaxSenders*256, hMap );
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}

	// transfer the Sender name set to shared memory
	// First write the set strings to a buffer
	// We don't know the exact size of each Sender name, so save each one as 256 bytes
	// so we can read them back later even if the string lengths are different
	buffer = (char *)malloc(MaxSenders*256*sizeof(unsigned char));
	memset( (void *)buffer, 0, MaxSenders*256 ); // make sure it is clear because we rely on nulls
	buf = (char *)buffer; // pointer within the buffer
	i = 0;
	for(iter = SenderNames.begin(); iter != SenderNames.end(); iter++) {
		namestring = *iter; // the string to copy to the buffer
		// copy it with 256 max length although only the string length will be copied
		// namestring.copy(buf, 256, 0);
		// printf("    Writing %s\n", namestring.c_str());
		strcpy_s(buf, 256, namestring.c_str());
		// move the buffer pointer on for the next Sender name
		buf += 256;
		i++;
		if(i > MaxSenders) break; // do not exceed the size of the local buffer
	}

	// Write the whole buffer containing the set of Sender names to shared memory
	memcpy ( (void *)pBuf, (void *)buffer, MaxSenders*256 );

	// Cleanup
	free((void *)buffer);
	
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end SetSenderSet



// Create a shared memory map to set the active Sender name to shared memory
// This is a separate small shared memory with a fixed sharing name
// that clients can use to retrieve the current active Sender
// TODO - use pointer from initial map creation
bool spoutSenderNames::setActiveSenderName(const char* SenderName) 
{
	char* pBuf;
	HANDLE hMap;
	HANDLE hLock;

	if(strlen(SenderName) == 0)	return false;

	LockMap("ActiveSenderName", hLock);

	// Open shared memory for the active sender name to access it
	pBuf = OpenMap("ActiveSenderName", 256, hMap);
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}

	// Fill it with the Sender name string
	memcpy( (void *)pBuf, (void *)SenderName, 256 ); // write the Sender name string to the shared memory
	
	// Close the map, but it can be accessed again
	// The map is not released until the last sender is unregistered
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;
} // end setActiveSenderName



// Get the active Sender name from shared memory
// TODO - use pointer from initial map creation
bool spoutSenderNames::getActiveSenderName(const char* SenderName) 
{
	HANDLE hMap; // handle to the memory map
	char *pBuf; // pointer to the memory map
	HANDLE hLock;

	// Lock the shared memory map
	LockMap("ActiveSenderName", hLock);

	// Open the named memory map for the active sender and return a pointer to the memory
	pBuf = OpenMap("ActiveSenderName", 256, hMap);
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}

	memcpy( (void *)SenderName, (void *)pBuf, 256 ); // get the name string from shared memory
	
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end getActiveSenderName



// Return current sharing handle, width and height of a Sender
// A receiver checks this all the time so it has to be compact
// Does not have to be the info of this instance
// so the creation pointer and handle may not be known
bool spoutSenderNames::getSharedInfo(const char* sharedMemoryName, SharedTextureInfo* info) 
{
	HANDLE hMap; // handle to the shared memory map
	char* pBuf; // pointer to the memory map
	HANDLE hLock;

	LockMap(sharedMemoryName, hLock);

	// Open the named memory map for the sender and return a pointer to the memory
	pBuf = OpenMap(sharedMemoryName, sizeof(SharedTextureInfo), hMap );
	if(hMap == NULL || pBuf == NULL) {
		UnlockMap(hLock);
		return false;
	}
	memcpy((void *)info, (void *)pBuf, sizeof(SharedTextureInfo) );
	
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end getSharedInfo


// =====================================
//	Mutex locks for shared memory :
//		1) Sender name map
//		2) Active sender info structure
//		3) Sender info structure
// ======================================
bool spoutSenderNames::CreateMapLock(const char *mapname, HANDLE &hMutex)
{
	char mutexname[256];

	// Make the mutex name different to the mapname, but connected so it can be opened and released
	if(hMutex) return true;

	// Create a Mutex to control access to the name map
	sprintf_s(mutexname, "%s_mutex", mapname);

	// No initial ownership
	hMutex = CreateMutexA(NULL, 0, mutexname); // Creates or opens existing
	if (hMutex == NULL) { 
		return false;
	}

	return true;
}


// Waits for 4 frames before deciding the lock has failed
// if it does, a receiver will assume the information has not changed
// LockMap returns false if the mutex does not exist - i.e. the sender has closed
// Sender mutex existence checked and works OK
bool spoutSenderNames::LockMap(const char *mapname, HANDLE &hLock)
{
	HANDLE hMutex;
	DWORD dwWaitResult;
	char mutexname[256];

	// DWORD dwStartTime, dwInterval;

	// LJ DEBUG - locks do not seem to be necessary probably because
	// most operations are read, and write operations are infrequent.
	// Needs careful testing to determine whether they will stop an operation
	// and failsafe for functions if they do - timeout duration ?

	// Check the name map mutex, if it does not exist just quit
	sprintf_s(mutexname, "%s_mutex", mapname);
	hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexname);
	if(!hMutex) {
		// printf("LockMap(%s) -  NO MUTEX\n", mapname);
		hLock = NULL;
		return false;
	}

	// printf("LockMap(%s) OK\n", mapname);
	// dwStartTime = timeGetTime();

	// Request ownership of the mutex
	
	// A thread can specify a mutex that it already owns in a
	// call to one of the wait functions without blocking its execution.

	// If the mutex exists, wait until it is clear
	dwWaitResult = WaitForSingleObject(hMutex, 67); // 4 frames at 16.6msec each - DEBUG
	
	// If dwMilliseconds is zero, the function does not
	// enter a wait state if the object is not signaled
	// it always returns immediately. If dwMilliseconds is INFINITE,
	// the function will return only when the object is signaled.

	// dwWaitResult = WaitForSingleObject(hMutex, 0); // Return immediately
    if(dwWaitResult != WAIT_OBJECT_0) { // The mutex wait failed so just exit
		// printf("LockMap(%s) - TIMEOUT\n", mapname);
		// ownership not obtained so no release
		CloseHandle(hMutex);
		hLock = NULL; // UnlockMap will do nothing
		return false; // Check for consequence - read failure ?
	}

	// Sleep here is vsync related

	// Sleep(1); // to check interval calc
	//
	// Sleep 1 16	1 frame
	//   -
	// Sleep 15 16
	// Sleep 16 32	2 frames
	//   -
	// Sleep 31 32	2 frames
	// Sleep 32 48	3 frames
	//   -
	// Sleep 48 63	4 frames
	//   -
	// Sleep 64 78	5 frames
	//
	// linked to Vsync but not affected by wglSwapIntervalEXT or GLfinish();
	// Conclusion - wait time of 1 msec (maybe less) will skip a frame
	// Wait time of 4 frames is unacceptable performance but is obvious if map locks are waiting
	// Fail on wait needs checking as to the return values of other functions
	// i.e. has the sender closed or is it just a lockout problem.
	//
	// dwInterval = timeGetTime() - dwStartTime;
	// printf("Lock wait = %d\n", dwInterval);

	// Now ownership is achieved (WAIT_OBJECT_0)
	// The mutex must be released subsequently
	hLock = hMutex;

	return true;
}


void spoutSenderNames::UnlockMap(HANDLE hMutex)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms685066%28v=vs.85%29.aspx
	// to release its ownership, the thread must call ReleaseMutex one time for each 
	// time that it obtained ownership (either through CreateMutex or a wait function).

	// The ReleaseMutex function fails if the calling thread does not own the mutex object.

	// Generally, an application should call CloseHandle once for each handle it opens.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724211%28v=vs.85%29.aspx

	if(hMutex) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex); // must close because it is a new handle
	}

}

// Close the handle from initial creation
// All handles will then be closed
void spoutSenderNames::CloseMapLock(HANDLE hMutex)
{
	//
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682411%28v=vs.85%29.aspx
	//
	// Use the CloseHandle function to close the handle. 
	// The system closes the handle automatically when the process terminates. 
	// The mutex object is destroyed when its last handle has been closed.
	if(hMutex) CloseHandle(hMutex);

}



//---------------------------------------------------------
bool spoutSenderNames::SenderDebug(const char *Sendername, int size)
{
	HANDLE hMap1 = NULL;
	HANDLE hMap2 = NULL;
	HANDLE hMap3 = NULL;
	HANDLE hMap4 = NULL;
	std::set<string> SenderNames;
	std::set<string>::iterator iter;
	string namestring;

	printf("**** SENDER DEBUG ****\n");

	printf("1) hSenderNamesMap = [%x], pSenderNamesMap = [%x]\n", m_hSenderNamesMap, m_pSenderNamesMap);

	// Check the sender names
	/*
	printf("    GetSenderSet\n");
	if(GetSenderSet(SenderNames)) {
		printf("        SenderNames size = [%d]\n", SenderNames.size());
		if (SenderNames.size() > 0) {
			for(iter = SenderNames.begin(); iter != SenderNames.end(); iter++) {
				namestring = *iter;
				printf("            Sender : [%s]\n", namestring.c_str());
			}
		}
	}
	else {
		printf("    GetSenderSet failed\n");
	}
	*/

	printf("    GetSenderNames\n");
	if(GetSenderNames(&SenderNames)) {
		// printf("        SenderNames size = [%d]\n", SenderNames.size());
		if (SenderNames.size() > 0) {
			for(iter = SenderNames.begin(); iter != SenderNames.end(); iter++) {
				namestring = *iter;
				printf("            Sender : [%s]\n", namestring.c_str());
			}
		}
		else {
			printf("    SenderNames size = 0\n");
		}
	}
	else {
		printf("    GetSenderSet failed\n");
	}

	// LJ DEBUG Try to open the names map directly
	hMap1 = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "SpoutSenderNames");
	if(hMap1) {
		printf("    Opened sendernames map [%x] OK\n", hMap1);
		CloseHandle(hMap1);
	}
	else {
		printf("    Could not open sendernames map\n");
	}
	
	printf("2) Closing - hSenderNamesMap = [%x], pSenderNamesMap = [%x]\n", m_hSenderNamesMap, m_pSenderNamesMap);

	// Close and try to reopen
	CloseMap(m_pSenderNamesMap, m_hSenderNamesMap);

	hMap2 = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "SpoutSenderNames");
	if(hMap2) {
		printf("    Sendernames map [%x] did not close\n", hMap2);
		CloseHandle(hMap2);
	}
	else {
		printf("    Closed sendernames map OK\n");
	}

	CloseMap(m_pActiveSenderMap, m_hActiveSenderMap);

	// Open shared memory for the active sender name to access it
	hMap3 = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "ActiveSenderName");
	if(hMap3) {
		printf("    Active sender map not closed [%x]\n", hMap3);
		CloseHandle(hMap3);
	}
	else {
		printf("    Active sender map closed OK\n");
	}

	return true;
}


