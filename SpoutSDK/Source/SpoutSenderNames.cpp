/**

	spoutSenderNames.cpp

	LJ - leadedge@adam.com.au

	Spout sender management
	25.04.14 - started class file
	27.05.14 - cleanup using memory map creation, open, close, lock
	05.06.14 - FindSenderName - allow for a null name entered
	08.06.14 - rebuild
	12.06.13 - major revision, included map handling
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


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
#include "spoutSenderNames.h"

spoutSenderNames::spoutSenderNames() {

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
bool spoutSenderNames::RegisterSenderName(char* Sendername) {

	std::pair<std::set<string>::iterator, bool> ret;
	std::set<string> SenderNames; // set of names
	HANDLE hMap;

	// Create the shared memory for the sender name set if it does not exist
	if(!GetSenderSet(SenderNames)) {
		CreateSenderSet();
		hMap = CreateMemoryMap("SpoutSenderNames", MaxSenders*256);
		if(hMap == NULL) {
			return false;
		}
		GetSenderSet(SenderNames); // not really needed becasue the set is empty
	}

	//
	// Add the Sender name to the set of names
	//
	ret = SenderNames.insert(Sendername);
	// The pair::second element in the pair is set to true if a new element
	// was inserted or false if an equivalent element already existed.
	if(ret.second) {
		// write the new map to shared memory
		SetSenderSet(SenderNames);
		// Set as the active Sender if it is the first one registered
		// Thereafter the user can select an active Sender using SpoutPanel or SpoutSenders
		if(SenderNames.size() == 1) {
			// Createmap will either create a new map or open the map if it exists
			CreateMemoryMap("ActiveSenderName", 256);
			// Set the current sender name as active
			SetActiveSender(Sendername);  
		}
	}
	return ret.second;
}

// Function to release a Sender name from the list of names
// Removes the Sender name and if it was the last one, 
// closes the shared memory map for the Sender name list
// See also RemoveSender
bool spoutSenderNames::ReleaseSenderName(char* Sendername) 
{
	string namestring;
	std::set<string> SenderNames; // set of names
	std::map<std::string, HANDLE> maphandles;
	std::set<string>::iterator iter;
	bool bRet = false;

	// Get the current list to update the passed list
	if(GetSenderSet(SenderNames)) {
		if (SenderNames.size() > 0) {
			// RemoveSender deletes the sender from the name set
			// deletes it's shared memory map and removes it from the handles map
			if (RemoveSender(Sendername)) { 
				// The name existed when it was removed from the list
				// its shared memory map is now also removed)
				// and if it was the active sender, that is updated if more senders exist.
				if(GetSenderSet(SenderNames)) { // get the new map
					if ( SenderNames.size() == 0 ) { // the last one left
						// close the shared memory map for the list of Sender names
						ReleaseMemoryMap("SpoutSenderNames");
						// close the shared memory map for the active Sender name
						ReleaseMemoryMap("ActiveSenderName");
						// Close the shared memory map for the map handles
						ReleaseMemoryMap("SpoutMapHandles");
					}
				}
				bRet = true;
			}
		}
	}
	return bRet; // the Sender name did not exist

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
			Sendernames[0].insert(namestring);
		}
		return true;
	}

	return false;
}


// This retrieves the info from the requested sender and fails if the sender does not exist
// Possible redundancy with getSharedInfo 
bool spoutSenderNames::GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
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
//
bool spoutSenderNames::SetSenderInfo(char* sendername, unsigned int width, unsigned int height, HANDLE dxShareHandle, DWORD dwFormat) 
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
		return false;
	}
	memcpy((void *)pBuf, (void *)&info, sizeof(SharedTextureInfo) );
	CloseMap(pBuf, hMap);
	
	UnlockMap(hLock);

	return true;

} // end SetSenderInfo


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


// Can be used wihout OpenGL context
bool spoutSenderNames::GetImageSize(char* name, unsigned int &width, unsigned int &height, bool &bMemoryMode)
{
	char newname[256];
	SharedTextureInfo TextureInfo;
	BITMAPINFOHEADER * pbmih;
	unsigned char * rgbBuffer;

	// Was initialized so get the sender details
	// Test to see whether the current sender is still there
	if(!getSharedInfo(newname, &TextureInfo)) {
		// Try the active sender
		if(GetActiveSender(newname)) {
			if(getSharedInfo(newname, &TextureInfo)) {
				// Pass back the new name and size
				strcpy_s(name, 256, newname);
				width  = TextureInfo.width;
				height = TextureInfo.height;
				bMemoryMode = false;
				return true;
			}
		}
	} // texture mode sender was running
	
	// Try for Memoryshare mode - read the image header into an RGB buffer
	rgbBuffer = (unsigned char *)malloc(sizeof(BITMAPINFOHEADER));
	if(rgbBuffer) {
		MemoryShare.Initialize();
		if(MemoryShare.ReadFromMemory(rgbBuffer, sizeof(BITMAPINFOHEADER))) {
			pbmih = (BITMAPINFOHEADER *)rgbBuffer;
			// return for zero width and height
			if(pbmih->biWidth == 0 || pbmih->biHeight == 0) {
				free((void *)rgbBuffer);
				return false;
			}
			// return the size received
			width  = (unsigned int)pbmih->biWidth;
			height = (unsigned int)pbmih->biHeight;
			MemoryShare.DeInitialize(); 
			free((void *)rgbBuffer);
			bMemoryMode = true;
			return true;
		} // endif MemoryShare.ReadFromMemory
		free((void *)rgbBuffer);
	} // end buffer alloc OK

	return false;
} // end GetImageSize



// Test to see if the Sender name exists
bool spoutSenderNames::FindSenderName(char* Sendername)
{
	string namestring;
	std::set<string> SenderNames;
	
	if(Sendername[0]) { // was a valid name passed
		// printf("spoutSenderNames::FindSenderName (%s)(%d)\n", Sendername, strlen(Sendername));
		// Get the current list to update the passed list
		if(GetSenderSet(SenderNames)) {
			// Does the name exist
			if(SenderNames.find(Sendername) != SenderNames.end() ) {
				return true;
			}
			else {
				// printf("spoutSenderNames::FindSenderName (%s) - not found\n", Sendername);
			}
		}
		else {
			// printf("spoutSenderNames::FindSenderName - sender set not found\n");
		}
	}
	else {
		// printf("spoutSenderNames::FindSenderName - no name\n");
	}
	return false;
}

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
bool spoutSenderNames::SetActiveSender(char *Sendername)
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

	// See if the shared memory of the acive Sender exists
	if(!getSharedInfo("ActiveSenderName", info)) {
		return false;
	}
	// It should exist because it is set whenever a Sender registers
	return true;
} // end GetActiveSenderInfo




/////////////////////////////////////////////////////////////////////////////////////
// Functions to Create, Update and Close a sender and retrieve sender texture info //
// without initializing DirectX or the GL/DX interop functions                     //
/////////////////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------
//	Create a sender with the info of a shared DirectX texture
//		1) Create a new named sender shared memory map
//		2) Set the sender texture info to the map
//		3) Register the sender name in the list of Spout senders
// ---------------------------------------------------------
bool spoutSenderNames::CreateSender(char *sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat)
{
	std::map<std::string, HANDLE> maphandles;
	std::map<std::string, HANDLE>::iterator it;
	string namestring;
	HANDLE hMap;
	DWORD errnum;

	// printf("CreateSender - %s, %dx%d, [%x] [%d]\n", sendername, width, height, hSharehandle, dwFormat);

	// If there is no handle map, create an empty one
	// There are MaxSenders senders
	//		Set map handle
	//		Active map handle
	//		MaxSenders sender map handles 
	// Try to create a handle map. CreateFileMapping will either create a new map or open the map if it exists
	hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (MaxSenders*256 + 16), (LPCSTR)"SpoutMapHandles");
	if(hMap != NULL) {
		errnum = GetLastError();
		if(errnum == ERROR_ALREADY_EXISTS) {
			GetHandleMap("SpoutMapHandles", maphandles);
			for(it = maphandles.begin(); it != maphandles.end(); it++) {
				namestring = it->first; // the key name string
				hMap = it->second; // the map handle for that name key
				// printf("        [%s][%x]\n", namestring.c_str(), hMap);
			}
		}
		else {
			// The first entry in the map is the name and handle of the map itself
			// so that it can release itself at the end of the program if it is empty
			maphandles.insert( std::pair<std::string, HANDLE>("SpoutMapHandles", hMap) );
			CreateMapLock("SpoutMapHandles"); // create a map lock mutex
		}
	}

	SetHandleMap("SpoutMapHandles", maphandles);

	// Create or open a shared memory map for this sender - allocate enough for the texture info
	if(!CreateMemoryMap(sendername, sizeof(SharedTextureInfo))) {
		return false;
	}

	// Register the sender name in the list of spout senders
	// Allow for the same nem for sender update
	RegisterSenderName(sendername);
	
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
bool spoutSenderNames::UpdateSender(char *sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat)
{
	// Save the info for this sender in the sender shared memory map
	if(!SetSenderInfo(sendername, width, height, hSharehandle, dwFormat))
		return false;

	return true;
		
} // end UpdateSender


// ---------------------------------------------------------
//	Close a sender
//	See - ReleaseSenderName
// ---------------------------------------------------------
bool spoutSenderNames::CloseSender(char* sendername)
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
			// printf("    spoutSenderNames::FindSender error 1\n");
			return false;
		}
	}
	else {
		// Passed name was null, so find the active sender
		if(!GetActiveSender(sendername)) {
			// printf("    spoutSenderNames::FindSender error 2\n");
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
		// printf("FindSender - found (%s) %dx%d\n", sendername, width, height);
		return true;
	}

	printf("    spoutSenderNames::FindSender error 3\n");

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
//		false	- sender not found or size changed
//			1) width and height are returned zero for sender not found
//			2) width and height are returned changed for sender size change
//			   The local texture then has to be resized.

bool spoutSenderNames::CheckSender(char *sendername, unsigned int &theWidth, unsigned int &theHeight, HANDLE &hSharehandle, DWORD &dwFormat)
{
	SharedTextureInfo info;
	char sname[256];
	unsigned int width = 0;
	unsigned int height = 0;
	bool bRet = false;

	// Is the given sender registered ?
	if(FindSenderName(sendername)) {
		// Does it still exist ?
		if(getSharedInfo(sendername, &info)) {
			// Get the size to check it
			width  = (unsigned int)info.width;
			height = (unsigned int)info.height;
			// Has the size changed ? If so, return false to indicate a change
			if(width != theWidth || height != theHeight) {
				bRet = false;
			}
			else {
				bRet = true;
			}

			// Return the texture info
			// strcpy_s(sendername, 256, sname);
			theWidth		= (unsigned int)info.width;
			theHeight		= (unsigned int)info.height;
			hSharehandle	= (HANDLE)info.shareHandle;
			dwFormat		= (DWORD)info.format;
			// Return the result
			return bRet;
		}
		else {
			// Sender is registered but does not exist so unregister it
			ReleaseSenderName(sname);
		}
	}
	
	// Return zero width and height to indicate sender not found
	theHeight = 0;
	theWidth  = 0;

	return false;

} // end CheckSender


//
// Retrieve the texture info of the active sender
//
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
// ===============================================================================





//
// Functions to manage creating, releasing, opening and closing of named memory maps
//
HANDLE spoutSenderNames::CreateMap(char *MemoryMapName, int MapSize)
{
	HANDLE hMapFile;
	DWORD errnum;

	// Set up Shared Memory
	// Must create the file mapping to the maximum size 
	// needed because it can't be changed afterwards
	hMapFile = CreateFileMappingA (	INVALID_HANDLE_VALUE,	// hFile - use paging file
								NULL,						// LPSECURITY_ATTRIBUTES - default security 
								PAGE_READWRITE,				// flProtect - read/write access
								0,							// The high-order DWORD - dwMaximumSizeHigh - max. object size 
								MapSize,					// The low-order DWORD - dwMaximumSizeLow - buffer size  
								(LPCSTR)MemoryMapName);		// name of mapping object
	
	errnum = GetLastError();
	// printf("	CreateMap (%s) GetLastError = %d\n", MemoryMapName, errnum);
	if (hMapFile == NULL) {
		// printf("	CreateMap (%s) failed\n", MemoryMapName);
	}

	if(errnum == ERROR_INVALID_HANDLE) {
		// printf("	CreateMap (%s) invalid handle\n", MemoryMapName);
	}

	if(errnum == ERROR_ALREADY_EXISTS) {
		// printf("	CreateMap [%s][%x] already exists\n", MemoryMapName, hMapFile);
	}
	else {
		// printf("	CreateMap [%s][%x] NEW MAP\n", MemoryMapName, hMapFile);
	}

	return hMapFile;

}


char* spoutSenderNames::OpenMap(char* MapName, int MapSize, HANDLE &hMap)
{
	char* pBuf;
	HANDLE hMapFile;

	hMapFile = OpenFileMappingA (FILE_MAP_ALL_ACCESS, // read/write access
								 FALSE,				  // do not inherit the name
								 MapName);			  // name of mapping object

	if (hMapFile == NULL) {
		// no map file means no sender is active
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
		// CloseHandle(hMapFile); // LJ DEBUG - should be closed before returning NULL ???
		// hMap = NULL;
		return NULL;
	}

	hMap = hMapFile;

	return pBuf;
	
} // end OpenMap


// Here we can unmap the view of the map but not close the handle
// When the process no longer needs access to the file mapping object, it should call the CloseHandle function. 
// When all handles are closed, the system can free the section of the paging file that the object uses.
void spoutSenderNames::CloseMap(char* MapBuffer, HANDLE hMap)
{
	if(MapBuffer) {
		UnmapViewOfFile((LPCVOID)MapBuffer);
	}

	if(hMap != NULL) {
		CloseHandle(hMap); // Handle is closed but not released
	}

}  // end CloseMap



HANDLE spoutSenderNames::CreateMemoryMap(char *MemoryMapName, int MapSize)
{

	HANDLE hMap;
	// char* pBuf;
	DWORD errnum;
	std::map<std::string, HANDLE> maphandles;
		
	// Create or open a shared memory map depending on whether it exists
	hMap = CreateFileMappingA (	INVALID_HANDLE_VALUE,		// hFile - use paging file
								NULL,						// LPSECURITY_ATTRIBUTES - default security 
								PAGE_READWRITE,				// flProtect - read/write access
								0,							// The high-order DWORD - dwMaximumSizeHigh - max. object size 
								MapSize,					// The low-order DWORD - dwMaximumSizeLow - buffer size  
								(LPCSTR)MemoryMapName);		// name of mapping object
	
	errnum = GetLastError();
	// printf("	CreateMemoryMap (%s) GetLastError = %d\n", MemoryMapName, errnum);
	if (hMap == NULL) {
		// printf("	CreateMemoryMap (%s) failed\n", MemoryMapName);
		return NULL;
	}

	if(errnum == ERROR_INVALID_HANDLE) {
		// printf("	CreateMemoryMap (%s) invalid handle\n", MemoryMapName);
		return NULL;
	}

	if(errnum == ERROR_ALREADY_EXISTS) {
		// printf("	CreateMemoryMap [%s][%x] already exists\n", MemoryMapName, hMap);
	}
	else {
		// printf("    CreateMemoryMap (%s) : created new map\n", MemoryMapName);
		// LJ DEBUG ?? Necessary to create a view for a new map or it is lost on close ?
		// pBuf = (char *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MapSize);	
	}
	
	// Create a lock mutex with a matching name which is tested with LockMap and UnlockMap
	// The mutex is destroyed when this instance closes but we make sure in cleanup
	CreateMapLock(MemoryMapName);

	// Leave the handle to shared memory open so that the map is not closed
	// It will be closed when the last handle to the map (this one) is closed

	// Save it in an std::map of handles keyed by the map name
	// and when no longer needed, the map handle is retrieved and closed by ReleaseMemoryMap

	// Get the set of map handles
	GetHandleMap("SpoutMapHandles", maphandles);
	maphandles.insert( std::pair<std::string, HANDLE>(MemoryMapName, hMap) );
	SetHandleMap("SpoutMapHandles", maphandles);

	return hMap;

}


// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366537%28v=vs.85%29.aspx
// Mapped views of a file mapping object maintain internal references to the object, 
// and a file mapping object does not close until all references to it are released. 
// Therefore, to fully close a file mapping object, an application must unmap all 
// mapped views of the file mapping object by calling UnmapViewOfFile and close the
// file mapping object handle by calling CloseHandle.
bool spoutSenderNames::ReleaseMemoryMap(char* MapName)
{
	HANDLE hMap;
	std::map<std::string, HANDLE> maphandles;
	std::map<std::string, HANDLE>::iterator it;
	string namestring;

	// Get the set of map handles
	if(!GetHandleMap("SpoutMapHandles", maphandles)) {
		return false;
	}

	// Find the handle to the map when it was first created
	it = maphandles.find(MapName);
	if(it != maphandles.end()) {
		namestring = it->first;
		if(namestring.c_str()[0]) {
			hMap = it->second;
			// Close this handle to release the map
			CloseHandle(hMap);
			maphandles.erase(it); // remove this sender from the map
		}
	}
	else {
		return false;
	}

	// Set the map back to shared memory - even if it is empty
	SetHandleMap("SpoutMapHandles", maphandles);

	// Release the named mutex for this map
	ReleaseMapLock(MapName);

	return true;

}


// =====================================
//	Mutex locks for shared memory :
//		1) Sender name map
//		2) Active sender info structure
//		3) Sender info structure
// ======================================
bool spoutSenderNames::CreateMapLock(char *mapname)
{
	HANDLE hMutex;
	char mutexname[256];

	// If the mutex is a named mutex and the object existed before this function call, 
	// the return value is a handle to the existing object, GetLastError returns ERROR_ALREADY_EXISTS
	// so make the mutex name different to the mapname, but connected so it can be opened and released

	// Create a Mutex to control access to the name map
	sprintf_s(mutexname, "%s_mutex", mapname);
	hMutex = CreateMutexA(NULL, 0, mutexname);
	if (hMutex == NULL) { 
		return false;
	}

	return true;
}


// Waits for 4 frames before deciding the lock has failed
// if it does, a receiver will assume the information has not changed
// LockMap returns NULL if the mutex does not exist - i.e. the sender has closed
bool spoutSenderNames::LockMap(char *mapname, HANDLE &hLock)
{
	HANDLE hMutex;
	DWORD dwWaitResult;
	char mutexname[256];

	// LJ DEBUG - locks do not seem to be necessary probably because
	// most operations are read and write operations are infrequent.
	// Needs careful testing to determine whether they will stop an operation
	// and failsafe for functions if they do - timeout duration ?
	hLock = NULL; // the unlock has no effect
	return true;

	// Check the name map mutex, if it does not exist just quit
	sprintf_s(mutexname, "%s_mutex", mapname);
	hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexname);
	if(!hMutex) {
		// printf("LockMap(%s) no mutex\n", mapname);
		return false;
	}

	// If the mutex exists, wait until it is clear
	dwWaitResult = WaitForSingleObject(hMutex, 67); // 4 frames at 16.6msec each
    if(dwWaitResult != WAIT_OBJECT_0) { // The mutex wait failed so just exit
		ReleaseMutex(hMutex);
		hLock = NULL; // the unlock has no effect
		return true;
	}

	// Now ownership is achieved
	
	hLock = hMutex;

	return true;
}


void spoutSenderNames::UnlockMap(HANDLE hMutex)
{
	if(hMutex) ReleaseMutex(hMutex);
}


void spoutSenderNames::ReleaseMapLock(char *mapname)
{
	HANDLE hMutex;
	char mutexname[256];

	sprintf_s(mutexname, "%s_mutex", mapname);
	hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexname);
	if(hMutex) CloseHandle(hMutex);

}


// ==========================================================================
//	Event locks used to control read/write on top of interop object lock
//	Events are created or opended by different processes.
//	https://en.wikipedia.org/wiki/Readers-writers_problem
//
//	LJ DEBUG - Used in SpoutSDK.cpp. Disabled until there is evidence of a problem with the interop lock
//
bool spoutSenderNames::InitEvents(char *eventname, HANDLE &hReadEvent, HANDLE &hWriteEvent)
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
				// mutex object that was not released by the thread that owned
				// the mutex object before the owning thread terminated. 
				// Ownership of the mutex object is granted to the calling thread
				// and the mutex state is set to nonsignaled.
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


// Texture access synchronisation - possibly not needed
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

// ==================================================================================


///////////////////////////////////////////////////
// Private functions for multiple Sender support //
///////////////////////////////////////////////////

//
//  Functions to read and write the list of Sender names to/from shared memory
//


// Create a shared memory map and copy the Sender names set to shared memory
bool spoutSenderNames::CreateSenderSet() 
{
	HANDLE hMap;
	char *pBuf;

	// Set up Shared Memory for all the sender names
	// Leave the handle open so that the sender name map is not destroyed on exit
	hMap = CreateMap("SpoutSenderNames", MaxSenders*256);
	if(hMap == NULL) {
		return false;
	}
	// Creating a view seems necessary or the map is lost on close
	pBuf = (char *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MaxSenders*256);		
	CreateMapLock("SpoutSenderNames"); // create a map lock mutex
	return true;

} // end CreateSenderSet

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
		// UnlockMap(hLock);
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
	// Unlock the map
	UnlockMap(hLock);

	return true;

} // end SetSenderSet

// Copy the map handles set to shared memory which must exist first
bool spoutSenderNames::SetHandleMap(char* MapName, std::map<std::string, HANDLE> maphandles)
{
	char* pBuf;
	HANDLE hMap;
	int i = 0;
	string namestring;	// local string to retrieve names
	HANDLE hSetMap;
	HANDLE hLock;
	char temp[256];
	char* buffer;		// local buffer for data transfer
	char* buf;			// pointer within the buffer
	int MapSize;		// size of the buffer
	std::map<std::string, HANDLE>::iterator it;

	if(maphandles.size() == 0) {
		return false;
	}

	// Lock the shared memory map with it's mutex
	LockMap(MapName, hLock);

	MapSize = (MaxSenders*256 + 16);

	// Open the handles map directly
	hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, // read/write access
							 FALSE,				  // do not inherit the name
							 "SpoutMapHandles");   // name of mapping object
	
	if (hMap == NULL) {
		// printf("    SetHandleMap : could not open MapHandles map\n");
		UnlockMap(hLock);
		return false;
	}

	pBuf = (char *) MapViewOfFile(hMap,				// handle to map object
								  FILE_MAP_ALL_ACCESS,	// read/write permission
								  0,
								  0,
								  MapSize );			// 
	
	if (pBuf == NULL) {
		DWORD errnum = GetLastError(); 
		if(errnum > 0) {
			// printf("    SetHandleMap : GetLastError() = %d\n", errnum);
			// ERROR_INVALID_HANDLE 6 (0x6) The handle is invalid.
		}
		return false;
	}


	// transfer the Map of handles set to shared memory
	// First write the handle set to a buffer
	buffer = (char *)malloc(MapSize*sizeof(HANDLE));
	memset( (void *)buffer, 0, MapSize);
	buf = (char *)buffer; // pointer within the buffer
	// The first entry is the name and handle of the map itself
	// so that it can release itself at the end of the program
	for(it = maphandles.begin(); it != maphandles.end(); it++) {

		namestring = it->first; // the key name string
		hSetMap = it->second; // the map handle for that name key
		sprintf_s(temp, 256, "%s", namestring.c_str());
		strcpy_s(buf, 256, temp);
		// move the buffer pointer on for the handle
		buf += 256;
		if(hSetMap != NULL) {
			sprintf_s(temp, 256, "%4.4d", hSetMap); // copy it into the buffer - plenty of space
			strcpy_s(buf, 5, temp);
		}

		// move the buffer pointer on 4 characters for the next Sender name
		buf += 4;
		i++;
		if(i > MaxSenders+4) break; // do not exceed the size of the map buffer
	}

	// Write the whole buffer containing the map of Sender names and handles to shared memory
	memcpy ( (void *)pBuf, (void *)buffer, MapSize );

	// Cleanup
	free((void *)buffer);

	CloseHandle(hMap);

	// Unlock the map
	UnlockMap(hLock);

	return true;

} // end SetHandleMap


// Get the map handles from shared memory into a std::map
bool spoutSenderNames::GetHandleMap(char* MapName, std::map<std::string, HANDLE> &maphandles) 
{
	int i;
	string namestring;	// local string to retrieve names
	HANDLE hSetMap;
	char keyname[256];		// char array to test for nulls
	char temp[16];		// char array for handle
	char *pBuf;			// pointer to shared memory buffer
	HANDLE hMap;		// handle to shared memory
	HANDLE hLock;
	char *buffer;		// local buffer for data transfer
	char *buf;			// pointer within the buffer
	int MapSize;		// size of the buffer
	std::map<std::string, HANDLE>::iterator it;

	MapSize = (MaxSenders*256 + 16);

	// Lock the map
	LockMap(MapName, hLock);

	// Open the named memory map and return a pointer to the memory
	pBuf = OpenMap(MapName, MapSize, hMap );
	if(hMap == NULL || pBuf == NULL) {
		return NULL;
	}

	// The data has been stored with 256 bytes reserved for each Sender name
	// and 4 bytes for each map handle keyed by that name
	// and nothing will have changed with the map yet
	buffer = (char *)malloc(MapSize*sizeof(unsigned char));
	memset( (void *)buffer, 0, MapSize ); // make sure it is clear because we rely on nulls

	// copy the shared memory to the local buffer
	memcpy ((void *)buffer, (void *)pBuf, MapSize );

	// If the MapHandles map is empty
	if(!buffer[0]) {
		free((void *)buffer);
		CloseHandle(hMap);
		return true;
	}

	// Read back from the buffer and rebuild the set
	
	// first empty the set
	if(maphandles.size() > 0) {
		maphandles.erase (maphandles.begin(), maphandles.end() );
	}

	buf = (char *)buffer;
	i = 0;
	do {
		// the name string retrieved from shared memory should terminate
		// with a null within the 256 chars.
		// At the end of the map there will be a null in the data for the name.
		// Must use a character array to ensure testing for null.
		strncpy_s(keyname, buf, 256); // Get the first name key
		if(keyname[0] > 0) {
			buf += 256; // move on to the handle for that name
			strncpy_s(temp, buf, 4); // Get the 4 byte for the handle
			temp[4] = 0; // make sure it is terminated
			hSetMap = (HANDLE)atoi(temp); // get the handle value
			if(hSetMap != NULL) {
				// insert name and handle into set
				maphandles.insert( std::pair<std::string, HANDLE>(keyname, hSetMap) );
			}
			// increment by 4 bytes for the next name
			buf += 4;
		}
		// If the name was null the loop will stop
		i++;
	} while(keyname[0] > 0 && hSetMap != NULL && i < MaxSenders+4);

	free((void *)buffer);

	// Close the open maphandles
	CloseMap(pBuf, hMap);
	UnlockMap(hLock);

	return true;

} // end GetHandleMap


//
// Removes a Sender from the set of Sender names
//
bool spoutSenderNames::RemoveSender(char* Sendername) 
{
	std::set<string> SenderNames;
	std::set<string>::iterator iter;
	string namestring;
	char name[256];

	// Discovered that the project properties had been set to CLI
	// Properties -> General -> Common Language Runtime Support
	// and this caused the set "find" function not to work.
	// It also disabled intellisense.
	// printf("spoutSenderNames::RemoveSender(%s)\n", Sendername);

	// Get the current map to update the list
	if(GetSenderSet(SenderNames)) {
		if(SenderNames.find(Sendername) != SenderNames.end() ) {
			SenderNames.erase(Sendername); // erase the matching Sender
			// Release the sender info in shared memory
			// Also remove the shared memory handle from the handle map
			ReleaseMemoryMap(Sendername); // release the shared memory map for the sender
			// LJ DEBUG moved to here otherwise the set is retrieved again if there was only one sender
			SetSenderSet(SenderNames); // set the map back to shared memory again
			// Is there a set left ?
			if(SenderNames.size() > 0) {
				// LJ DEBUG - will this work ?
				// Should be OK because the user selects the active sender
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

} // end CloseSender


// Create a shared memory map to set the active Sender name to shared memory
// This is a separate small shared memory with a fixed sharing name
// that clients can use to retrieve the current active Sender
bool spoutSenderNames::setActiveSenderName(char* SenderName) 
{

	char* pBuf;
	HANDLE hMap;
	HANDLE hLock;

	if(strlen(SenderName) == 0)	return false;

	LockMap(SenderName, hLock);

	// Open shared memory for the active sender name to access it
	pBuf = OpenMap("ActiveSenderName", 256, hMap);
	if(hMap == NULL || pBuf == NULL) {
		return false;
	}

	// Fill it with the Sender name string
	memcpy( (void *)pBuf, (void *)SenderName, 256 ); // write the Sender name string to the shared memory
	
	// Close the map, but it can be accessed again
	CloseMap(pBuf, hMap);
	// The map is not released until the last sender is unregistered

	UnlockMap(hLock);

	return true;
} // end setActiveSenderName


// Get the active Sender name from shared memory
bool spoutSenderNames::getActiveSenderName(char* SenderName) 
{

	HANDLE hMap; // handle to the memory map
	char *pBuf; // pointer to the memory map
	HANDLE hLock;

	// Lock the shared memory map
	LockMap("ActiveSenderName", hLock);

	// Open the named memory map for the active sender and return a pointer to the memory
	pBuf = OpenMap("ActiveSenderName", 256, hMap);
	if(hMap == NULL || pBuf == NULL) {
		return false;
	}
	memcpy( (void *)SenderName, (void *)pBuf, 256 ); // get the name string from shared memory
	CloseMap(pBuf, hMap);

	UnlockMap(hLock);

	return true;

} // end getActiveSenderName



// Return current sharing handle, width and height of a Sender
// A receiver checks this all the time so it has to be compact
bool spoutSenderNames::getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info) 
{
	HANDLE hMap; // handle to the shared memory map
	char* pBuf; // pointer to the memory map
	HANDLE hLock;

	LockMap(sharedMemoryName, hLock);

	// Open the named memory map for the sender and return a pointer to the memory
	pBuf = OpenMap(sharedMemoryName, sizeof(SharedTextureInfo), hMap );
	if(hMap == NULL || pBuf == NULL) {

		// printf("getSharedInfo(%s) map not found\n", sharedMemoryName);

		return false;
	}
	memcpy((void *)info, (void *)pBuf, sizeof(SharedTextureInfo) );
	CloseMap(pBuf, hMap);
	
	UnlockMap(hLock);

	return true;

} // end getSharedInfo

//
// Utility functions
//

//
//	SenderChanged
//
// Check to see if the Sender has changed anything - does not depend on directx being initialized
bool spoutSenderNames::SenderChanged(char *theSendername, int theWidth, int theHeight, DWORD theFormat, HANDLE theShareHandle) 
{
		unsigned int width, height;
		HANDLE sharehandle;
		DWORD format;

		// Test to see if the image size or sharehandle has changed
		// even though the name is the same or the sender could have closed
		if(GetSenderInfo(theSendername, width, height, sharehandle, format)) {
			if(width > 0 || height > 0) {
				if(width		!= theWidth 
				|| height		!= theHeight
				|| format		!= theFormat
				|| sharehandle	!= theShareHandle) {
					return true;
				}
			} // endif sender changed
		} // This sender has closed

		return false;

} // end SenderChanged




//---------------------------------------------------------
bool spoutSenderNames::SenderDebug(char *Sendername, int size)
{
	// LJ DEBUG
	HANDLE hMap; // handle to the shared memory map
	HANDLE hMap2;
	char* pBuf; // pointer to the memory map
	std::set<string> SenderNames;
	std::set<string>::iterator iter;
	string namestring;
	// char name[256];

	std::map<std::string, HANDLE> maphandles;
	std::map<std::string, HANDLE>::iterator it;

	// SharedTextureInfo info;
	DWORD MapSize;
	char sendername[256];

	printf("spoutSenderNames::SenderDebug(%s)\n", Sendername);

	// Check the sender names
	if(GetSenderSet(SenderNames)) {
		printf("    SenderNames size = [%d]\n", SenderNames.size());
		if (SenderNames.size() > 0) {
			for(iter = SenderNames.begin(); iter != SenderNames.end(); iter++) {
				namestring = *iter;
				printf("        Sender : [%s]\n", namestring.c_str());
			}
		}
		else {
			printf("    SenderNames size = 0\n");
		}
	}
	else {
		printf("    Could not find sendernames map\n");
	}

	// Try to open it directly
	hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "SpoutSenderNames");
	if(hMap) {
		printf("    Opened sendernames map directly OK\n");
		CloseHandle(hMap);
	}
	else {
		printf("    Could not open sendernames map directly\n");
	}
	

	// Open shared memory for the active sender name to access it
	hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "ActiveSenderName");
	if(hMap) {
		printf("    Opened active sendername map directly OK\n");
		CloseHandle(hMap);
	}
	else {
		printf("    Could not open active sendername map directly\n");
	}

				// hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, Sendername);
				hMap = CreateFileMappingA (	INVALID_HANDLE_VALUE,		// hFile - use paging file
								NULL,						// LPSECURITY_ATTRIBUTES - default security 
								PAGE_READWRITE,				// flProtect - read/write access
								0,							// The high-order DWORD - dwMaximumSizeHigh - max. object size 
								256,					// The low-order DWORD - dwMaximumSizeLow - buffer size  
								(LPCSTR)"ActiveSenderName");		// name of mapping object
	
				if(hMap != NULL) {
					DWORD errnum = GetLastError();
					if(errnum == ERROR_ALREADY_EXISTS) {
						printf("        ActiveSenderName[%x] exists\n", hMap);
					}
					else {
						printf("        ActiveSenderName [%x] map did not exist\n", hMap);
					}
					CloseHandle(hMap);
				}

			// Now the map will not exist, try to open it
			hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, "ActiveSenderName");
			if(hMap == NULL) {
				printf("        ActiveSenderName closed OK - cannot re-open map\n");
			}
			else {
				printf("        ActiveSenderName was found again\n", sendername);
				CloseHandle(hMap);
			}


			
	// Get the set of map handles
	if(!GetHandleMap("SpoutMapHandles", maphandles)) {
		// printf("    could not get handle map\n");
		return false;
	}
	else {
		// printf("    MapHandles size = [%d]\n", maphandles.size());
		for(it = maphandles.begin(); it != maphandles.end(); it++) {
			namestring = it->first; // the key name string
			hMap = it->second; // the map handle for that name key
			// printf("        [%s][%x]\n", namestring.c_str(), hMap);
		}
	}

	if(!Sendername[0]) {
		// printf("    no map name to test\n");
		return false;
	}

	// Try to see if the sendernames map still exists
	it = maphandles.find("SpoutSenderNames");
	if(it != maphandles.end()) {
		namestring = it->first;
		if(namestring.c_str()[0]) {
			// printf("            found 'SpoutSenderNames' map [%x]\n", hMap);
			hMap = it->second;
			// Try to get a view of it
			pBuf = (char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MaxSenders*256);
			if(pBuf) {
				// printf("            Opened view of handle OK\n");
				UnmapViewOfFile((LPCVOID)pBuf);
			}
			else {
				// printf("            Could not open view of handle\n");
			}

			// Use CreateMap to get a handle
			hMap2 = CreateMap("SpoutSenderNames", MaxSenders*256);
			// printf("            CreateMap returned [%x] saved was [%x]\n", hMap2, hMap);
			// Try to get a view of it
			pBuf = (char *)MapViewOfFile(hMap2, FILE_MAP_ALL_ACCESS, 0, 0, MaxSenders*256);
			if(pBuf) {
				// printf("            Opened view of CreateMap OK\n");
				UnmapViewOfFile((LPCVOID)pBuf);
			}
			else {
				// printf("            Could not open CreateMap view\n");
			}
		}
	}
	else {
		// printf("            'SpoutSenderNames' map not found\n", sendername);
	}

	strcpy_s(sendername, 256, Sendername); // "SpoutSenderNames");
	MapSize = (DWORD)size; // MaxSenders*256;

	// Find the sender
	it = maphandles.find(sendername);
	if(it != maphandles.end()) {
		namestring = it->first;
		if(namestring.c_str()[0]) {
			// printf("            found [%s]\n", namestring.c_str());
		}
	}
	else {
		// printf("            (%s) not found\n", sendername);
	}


	// if(GetSenderInfo(Sendername, width, height, hSender, dwFormat)) {
	// printf("    *** WARNING (%s) info still exists ****\n    %dx%d, [%x] [%d]\n", Sendername, width, height, hSender, dwFormat);
	// if(getSharedInfo(Sendername, &info)) {
		// printf("    *** WARNING (%s) info still exists ****\n    %dx%d\n", Sendername, info.width, info.height);

		// printf("    *** DOUBLE CHECK ***\n");
		// Double check - Open the named memory map for the sender
		// pBuf = OpenMap(Sendername, sizeof(SharedTextureInfo), hMap );
		// if(hMap == NULL || pBuf == NULL) {
				// printf("    getSharedInfo - cannot open map\n");
		// }
		// else {
			// printf("    *** (%s) info map was found again ***\n", Sendername);
			// CloseMap(pBuf, hMap);
			// Check again directly

			// To fully close a file-mapping object, an application must unmap all mapped views
			// of the file-mapping object by calling UnmapViewOfFile, and close the file-mapping
			// object handle by calling CloseHandle.

			// int i = 0;
			// do {
				// hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, Sendername);
				hMap = CreateFileMappingA (	INVALID_HANDLE_VALUE,		// hFile - use paging file
								NULL,						// LPSECURITY_ATTRIBUTES - default security 
								PAGE_READWRITE,				// flProtect - read/write access
								0,							// The high-order DWORD - dwMaximumSizeHigh - max. object size 
								MapSize,					// The low-order DWORD - dwMaximumSizeLow - buffer size  
								(LPCSTR)sendername);		// name of mapping object
	
				if(hMap != NULL) {
					DWORD errnum = GetLastError();
					if(errnum == ERROR_ALREADY_EXISTS) {
						// printf("        [%s][%x] exists\n", sendername, hMap);
					}
					else {
						// printf("        [%s][%x] map did not exist\n", sendername, hMap);
					}
					CloseHandle(hMap);
				}
				// i++;
			// } while(hMap != NULL && i < 10);

			// Now the map will not exist, try to open it
			hMap = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, sendername);
			if(hMap == NULL) {
				// printf("        Map closed OK - cannot re-open map\n");
			}
			else {
				// printf("        Map (%s) was found again\n", sendername);
				CloseHandle(hMap);
			}

			/*
			if(size == sizeof(SharedTextureInfo)) {
				if(getSharedInfo(sendername, &info)) {
					// printf("    Sender (%s) info still exists : %dx%d\n", sendername, info.width, info.height);
				}
				else {
					// printf("    No sender info exists\n");
				}
			}
			*/
		// }
	// } // end debug


	/*
	std::map<std::string, HANDLE> maphandles;
	std::map<std::string, HANDLE>::iterator it;
	string namestring;

	GetHandleMap("SpoutMapHandles", maphandles);

	for(it = maphandles.begin(); it != maphandles.end(); it++) {
		namestring = it->first; // the key name string
		hMap = it->second; // the map handle for that name key
		// printf("    MapHandle [%s][%x]\n", namestring.c_str(), hMap);
	}
	*/
	// MessageBoxA(NULL, "SenderDebug", "spoutSenderNames", MB_OK);

	return true;
}


