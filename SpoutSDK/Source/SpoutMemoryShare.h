/*
	spoutMemoryShare.h

	Class used for Spout inter-process communication

		Semaphore single reader/writer version

		Last modifed : 11.02.14

		 The purpose is to establish a fixed shared memory map for an RGB image the size of the desktop (1920x1080)
		 and semaphores to control read/write access for sharing a bitmap image between sender and receiver
		 Only one sender and receiver is assumed.

		bool Initialize();
		Initialize memory map and read/write semaphores

		void DeInitialize();
		Close memory map and semaphore handles

		void CreateSenderMutex();
		// Create a mutex by the sender which is checked by a receiver

		void ReleaseSenderMutex();
		// for a sender to release its mutex on close

		bool CheckSenderMutex();
		// for a receiver to check the presence of a sender
		// If the mutex does not exist, the semaphore read wait is skipped

		bool ReadFromMemory(unsigned char *buffer, int numBytes, int offset = 0);
		// Read from shared memory
		// Offset is useful for example when reading a bitmap to skip the bitmap header

		bool WriteToMemory(unsigned char *buffer, int numBytes);
		// Write to shared memory

		void setSharedMemoryName(char* sharedMemoryName); 
		// to set names of mapping and semaphores externally 
		// otherwise a default name is used


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

#ifndef __spoutMemoryShare__
#define __spoutMemoryShare__

// #define CONSOLE_DEBUG  // to activate debug console messages

#include "SpoutCommon.h"
#include <Windows.h>
#include <stdio.h>

class SPOUT_DLLEXP spoutMemoryShare {

	public:

        spoutMemoryShare();
        ~spoutMemoryShare();
		bool Initialize();
		void DeInitialize();
		// Create a mutex by the sender which is checked by a receiver
		// If the mutex does not exist, the semaphore read wait is skipped
		void CreateSenderMutex();	// for a sender to set a mutex for a receiver to test
		void ReleaseSenderMutex();	// for a receiver to release the mutex if a sender is present
		bool CheckSenderMutex();	// for a receiver to check the presence of a sender
		bool GetImageSizeFromSharedMemory(unsigned int &width, unsigned int &height);
		bool ReadFromMemory(unsigned char *buffer, int numBytes, int offset = 0);
		bool WriteToMemory(unsigned char *buffer, int numBytes);
		void setSharedMemoryName(char* sharedMemoryName); // to set names of mapping and semaphores externally 
		char szShareMemoryName[256];		// name of the shared memory (made public for FFGL plugins)

	protected:

		LPTSTR  pBuffer;					// shared memory pointer
		HANDLE  hSharedMemory;				// handle to shared memory
		HANDLE	hReadSemaphore;
		HANDLE	hWriteSemaphore;
		HANDLE	hSenderMutex;				// Mutex that gets set by a sender
		char	szReadSemaphoreName[256];	// name of the read semaphore
		char	szWriteSemaphoreName[256];	// name of the read semaphore


};

#endif

