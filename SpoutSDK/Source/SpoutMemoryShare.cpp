#include "spoutMemoryShare.h"

/*
	spoutMemoryShare.cpp

	Class used for Spout inter-process communication


		Copyright (c) 2014-2015, Lynn Jarvis. All rights reserved.

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

//
// The purpose of this class is to share image frames
//
//
// Semaphore single reader/writer version
//

//
// Last modified : 01.09.13
// incuded in Openframeworks test app 08.09.13
//
// 07.10.13 
//		- included szShareMemoryName - name of the shared memory (made public for FFGL plugins)
//		- included function setSharedMemoryName to set names of mapping and semaphores externally 
//		- included setSharedMemoryName in constructor
// 13.10.13
//		- added offset for ReadFromMemory
//
// 09.11.13
//		- changed from memcpy to CopyMemory - very slight speed advantage
// 26.01.14
//		- added a sender mutex which is checked on a memory read
//		  If it does not exist, there is no sender to read from so the sempaphore is not tested 
//		  This removes the wait delay of no sender is present
// 11.02.14 - added check for no sharing name in intitialize
// 15.06.14 - added ReleaseSenderMutex() to deinitialize
//
//

//----------------------------------------
spoutMemoryShare::spoutMemoryShare()
{
	setSharedMemoryName("ofxMemoryShare"); // can be set externally
}


//---------------------------------------------------------
spoutMemoryShare::~spoutMemoryShare()
{

}


void spoutMemoryShare::setSharedMemoryName(char* sharedMemoryName) 
{
	if ( strcmp(szShareMemoryName, sharedMemoryName) == 0 ) {
		return;
	}
	strcpy_s(szShareMemoryName, 256, sharedMemoryName);
	sprintf_s((char*)szReadSemaphoreName,	256,	"%s Read Semaphore",  szShareMemoryName);
	sprintf_s((char*)szWriteSemaphoreName,	256,	"%s Write Semaphore", szShareMemoryName);
	sprintf_s((char*)szShareMemoryName,		256,	"%s Shared Memory",   szShareMemoryName);
}

//---------------------------------------------------------
bool spoutMemoryShare::Initialize()
{
	#ifdef CONSOLE_DEBUG
	DWORD errnum;
	#endif

	// Have semaphore names been set ?
	if(!szShareMemoryName[0]) {
		setSharedMemoryName("spoutMemoryShare");
	}

	// Create or open read semaphore depending, on whether it already exists or not
	hReadSemaphore = CreateSemaphoreA ( 
						NULL,	// default security attributes
						1,		// initial count (do not allow a read until write allows it)
						1,		// maximum count
						(LPSTR)szReadSemaphoreName);

    if (hReadSemaphore == NULL) {
		#ifdef CONSOLE_DEBUG
        printf("Create Read Semaphore error\n");
		#endif
        return false;
    }
	#ifdef CONSOLE_DEBUG
	else {
		errnum = GetLastError();
		// printf("Read Semaphore GetLastError() = %d\n", errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			printf("Read Semaphore invalid handle\n");
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			printf("Read Semaphore [%s] already exists\n", szReadSemaphoreName);
		}
		else {
			printf("Read Semaphore [%s] created\n", szReadSemaphoreName);
		}
	}
	#endif

	hWriteSemaphore = CreateSemaphoreA ( 
						NULL,   // default security attributes
						1,		// initial count (allow write)
						1,		// maximum count
						(LPSTR)szWriteSemaphoreName);

    if (hWriteSemaphore == NULL) {
		#ifdef CONSOLE_DEBUG
        printf("Create Write Semaphore error\n");
		#endif
        return false;
    }
	#ifdef CONSOLE_DEBUG
	else {
		errnum = GetLastError();
		// printf("Write Semaphore GetLastError() = %d\n", errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			printf("Write Semaphore invalid handle\n");
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			printf("Write Semaphore [%s] already exists\n", szWriteSemaphoreName);
		}
		else {
			printf("Write Semaphore [%s] created\n", szWriteSemaphoreName);
		}
	}
	#endif

	// Each time a thread completes a wait for a semaphore object, 
	// the count of the semaphore object is decremented by one. 
	// When the thread has finished, it calls the ReleaseSemaphore function, 
	// which increments the count of the semaphore object.
     
    // Set up Shared Memory
	// allocate space for an rgb bitmap the size of the desktop, then nothing will be bigger
	DWORD size = (DWORD)( sizeof(BITMAPINFOHEADER)+GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN)*3 );

    // Create or open shared memory, depending on whether it already exists or not
	// Must be CreateFileMappingA, not CreateFileMapping or it doesn't work
    hSharedMemory = CreateFileMappingA ( INVALID_HANDLE_VALUE,			// use paging file
										 NULL,							// default security 
										 PAGE_READWRITE,				// read/write access
										 0,								// max. object size 
										 size,							// buffer size  
										 (LPCSTR)szShareMemoryName);	// name of mapping object
	
    if (hSharedMemory == NULL || hSharedMemory == INVALID_HANDLE_VALUE) { 
		#ifdef CONSOLE_DEBUG
		printf("Error occured while creating file mapping object : %d\n", GetLastError() );
		#endif
        return false;
    }
	#ifdef CONSOLE_DEBUG
	else {
		errnum = GetLastError();
		// printf("map name = %s : GetLastError() = %d\n", szShareMemoryName, errnum);
		if(errnum == ERROR_INVALID_HANDLE) {
			printf("map invalid handle\n");
		}
		if(errnum == ERROR_ALREADY_EXISTS) {
			printf("map already exists\n");
			// here we can set bIsClient also
		}
		else {
			printf("map created OK\n");
		}
	}
	#endif

    pBuffer = (LPTSTR)MapViewOfFile(	hSharedMemory,			// handle to map object
										FILE_MAP_ALL_ACCESS,	// read/write permission
										0,                   
										0,                   
										size);

    if (NULL == pBuffer) { 
		#ifdef CONSOLE_DEBUG
		printf("Error occured while mapping view of the file : %d\n", GetLastError() );
		#endif
        return false;
    }

	// ==== SEMAPHORES ====

     
    return true;

} // end Initialize


// for a sender to set a mutex for a receiver to test
void spoutMemoryShare::CreateSenderMutex()
{
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutMemorySender");

	// If no sender is running yet, create a mutex that a receiver can check
	if (!hMutex) {
		#ifdef CONSOLE_DEBUG 
		printf("Creating sender mutex\n"); 
		#endif
		hSenderMutex = CreateMutexA(0, 0, "SpoutMemorySender");
	}

	CloseHandle(hMutex);
}

// for a receiver to release the mutex if a sender is present
void spoutMemoryShare::ReleaseSenderMutex()
{
	HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutMemorySender");

	if(hMutex) {
		#ifdef CONSOLE_DEBUG 
		printf("ReleaseSenderMutex - mutex exists\n");
		#endif
		ReleaseMutex(hMutex);
	}

}

// for a receiver to check the presence of a sender
bool spoutMemoryShare::CheckSenderMutex()
{
	HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutMemorySender");
	if(hMutex) {
		#ifdef CONSOLE_DEBUG 
		// printf("CheckSenderMutex - mutex exists\n");
		#endif
		CloseHandle(hMutex);
		return true;
	}
	else {
		#ifdef CONSOLE_DEBUG 
		printf("CheckSenderMutex - mutex does not exist\n");
		#endif
		return false;
	}
}



void spoutMemoryShare::DeInitialize()
{
    CloseHandle(hReadSemaphore);
    CloseHandle(hWriteSemaphore);
    UnmapViewOfFile(pBuffer);
	
	// When the process no longer needs access to the file mapping object, 
	// it should call the CloseHandle function. When all handles are closed, 
	// the system can free the section of the paging file that the object uses.
	CloseHandle(hSharedMemory);

	// Release the sender mutex if it exists
	ReleaseSenderMutex();

} // end DeInitialize


// Get the size of an image if a memoryshare sender is running
bool spoutMemoryShare::GetImageSizeFromSharedMemory(unsigned int &width, unsigned int &height)
{
		BITMAPINFOHEADER bmih;
		BITMAPINFOHEADER * pbmih;
		pbmih = (BITMAPINFOHEADER *)&bmih; // pointer to a bitmap info header
	
		pbmih->biWidth = 0;
		pbmih->biHeight = 0;

		// int i = sizeof(BITMAPINFOHEADER);

		if(ReadFromMemory((unsigned char *)pbmih, sizeof(BITMAPINFOHEADER))) {
			if(pbmih->biWidth > 0 && pbmih->biHeight > 0) {
				// if(width == 0 && height == 0) {
					width  = pbmih->biWidth;
					height = pbmih->biHeight;
				// }
				return true;
			}
		}
		return false;
}


bool spoutMemoryShare::WriteToMemory(unsigned char *buffer, int numBytes)
{

	DWORD dwWaitResult; // result of wait

	// 
	// Check whether the reader has signalled ready and the write gate is open
	//
    dwWaitResult = WaitForSingleObject( 
						hReadSemaphore,		// handle to semaphore
						0L);				// zero-second time-out interval
	
	// The semaphore object was not signaled the gate is closed, so just exit
    if(dwWaitResult != WAIT_OBJECT_0) {
			return false;
	}

	// The read semaphore was signaled so the write gate is open, so perform the write
	// Each time a thread completes a wait for a semaphore object, 
	// the count of the semaphore object is decremented by one
	// so the read semaphore count is 0 which means the write gate is closed
	// Write the shared memory
	// memcpy(pBuffer, buffer, numBytes); // imagesize);
	// might be a very slight speed increase by using CopyMemory instead of memcpy
	CopyMemory(pBuffer, buffer, numBytes);

	// now that writing is finished, set the write semahphore
	// by increasing it's count, then the reader is signalled that it can read
	// This opens the read gate
	ReleaseSemaphore(hWriteSemaphore,	// handle to semaphore
                        1,				// increase count by one
                        NULL);			// not interested in previous count

	return true;

} // end WriteToMemory



// Modify to return the address of the memory buffer if the write event has been signalled ??

bool spoutMemoryShare::ReadFromMemory(unsigned char *buffer, int numBytes, int offset)
{
	DWORD dwWaitResult;

	// Test for a sender mutex
	if(!CheckSenderMutex())
		return false;

	// Check whether the writer has signalled ready and whether the read gate is open
    dwWaitResult = WaitForSingleObject( 
						hWriteSemaphore,	// handle to semaphore
						83L); // 0L);		// zero-second time-out interval gives problems
	
	
	// The semaphore object was not signaled, so the read gate is closed.
    if(dwWaitResult != WAIT_OBJECT_0) {
		#ifdef CONSOLE_DEBUG
		switch(dwWaitResult) {
			case WAIT_ABANDONED :
				printf("ReadFromMemory : WAIT_ABANDONED\n");
				break;
			case WAIT_TIMEOUT :
				printf("ReadFromMemory : WAIT_TIMEOUT\n");
				break;
			case WAIT_FAILED :
				printf("ReadFromMemory : WAIT_FAILED\n");
				break;
			default :
				break;
		}
		#endif
		return false;
	}


	// The write semaphore was signalled so the read gate is open, so perform the read
	// Each time a thread completes a wait for a semaphore object, 
	// the count of the semaphore object is decremented by one
	// so the write semaphore count is 0 which means the read gate is closed

    // Read the shared memory
	// memcpy(buffer, pBuffer+offset, numBytes);
	// might be a very slight speed increase by using CopyMemory instead of memcpy
	CopyMemory(buffer, pBuffer+offset, numBytes); // no speed advantage that I can tell

	// now that reading is finished, set the read semaphore
	// by increasing it's count, then the writer is signalled that it can write
	// This opens the write gate
	ReleaseSemaphore(hReadSemaphore,	// handle to semaphore
                        1,				// increase count by one
                        NULL);			// not interested in previous count

	return true;

} // end ReadFromMemory





