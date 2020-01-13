//
//		SpoutFrameCount
//
//		Semaphore frame counter
//
// ====================================================================================
//		Revisions :
//
//		07.10.18	- project start
//		01.11.18	- Add GetRefreshRate() to set default sender fps to system refresh rate
//		16.11.18	- Profile UpdateSenderFps
//		23.11.18	- Change semaphore access functions to operate within a mutex lock
//		26.11.18	- Add application disable frame counting to avoid variable frame rate problems
//		27.11.18	- Add IsFrameNew
//		02.12.18	- Add sender GetWidth, GetHeight, GetFps and GetFrame
//		23.12.18	- More log warnings
//		26.02.19	- Add IsFrameCountEnabled
//		14.03.19	- CleanupFrameCount - return if no semaphore handle
//					- Remove wait warning from CheckAccess()
//		02.04.19	- Profile timing functions
//		24.04.19	- Add HoldFps
//		19.05.19	- Clean up
//		05.06.19	- HoldFps - use std::chrono if VS2015 or greater
//
// ====================================================================================
//
/*
	Copyright (c) 2019. Lynn Jarvis. All rights reserved.

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

#include "SpoutFrameCount.h"

// -----------------------------------------------
spoutFrameCount::spoutFrameCount()
{
	m_hAccessMutex = NULL;
	m_hCountSemaphore = NULL;
	m_SenderName[0] = 0;
	m_CountSemaphoreName[0] = 0;
	m_FrameCount = 0L;
	m_LastFrameCount = 0L;
	m_FrameTimeTotal = 0.0;
	m_FrameTimeNumber = 0.0;
	m_lastFrame = 0.0;

	// Default sender fps is system refresh rate
	m_Fps = GetRefreshRate();

	// User registry setting is checked in EnableFrameCount
	m_bFrameCount = false; // default not set
	m_bDisabled = false;

	// Initialize fps control
	m_millisForFrame = 0.0;
#if _MSC_VER >= 1900
	m_FrameStartPtr = new std::chrono::steady_clock::time_point;
	m_FrameEndPtr = new std::chrono::steady_clock::time_point;
#else
	m_FrameStart = 0.0;
#endif

	// Initialize counter
	PCFreq = 0.0;
	CounterStart = 0;
	StartCounter();

}

// -----------------------------------------------
spoutFrameCount::~spoutFrameCount()
{

#if _MSC_VER >= 1900
	if (m_FrameStartPtr) delete m_FrameStartPtr;
	if (m_FrameEndPtr) delete m_FrameEndPtr;
#endif

	// Close the frame count semaphore.
	if (m_hCountSemaphore) CloseHandle(m_hCountSemaphore);
	m_hCountSemaphore = NULL;

	// Close the texture access mutex
	if (m_hAccessMutex) CloseHandle(m_hAccessMutex);
	m_hAccessMutex = NULL;

}


// ======================================================================
//								Public
// ======================================================================

// -----------------------------------------------
//
// Create a frame counting semaphore.
//
// Incremented by a sender.
// Tested by a receiver to retrieve the count.
//
void spoutFrameCount::EnableFrameCount(const char* SenderName)
{
	// Check the user setting for frame counting between sender and receiver
	DWORD dwFrame = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Framecount", &dwFrame)) {
		m_bFrameCount = (dwFrame == 1);
	}

	// Return if the user has not selected frame counting
	// Subsequently SetNewFrame and GetNewFrame return without action

	// Return silently if not enabled in SpoutSettings
	if (!m_bFrameCount)
		return;

	// Return silently if application disabled
	if (m_bDisabled)
		return;

	// A sender name is required
	if (SenderName[0] == 0) {
		SpoutLogWarning("SpoutFrameCount::EnableFrameCount - no sender name");
		return;
	}

	// Reset frame count, comparator and fps variables
	m_FrameCount = 0L;
	m_LastFrameCount = 0L;
	m_FrameTimeTotal = 0.0;
	m_FrameTimeNumber = 0.0;
	m_Fps = GetRefreshRate();

	// Return if already enabled for this sender
	if (m_hCountSemaphore && strcmp(SenderName, m_SenderName) == 0) {
		SpoutLogNotice("SpoutFrameCount::EnableFrameCount already enabled [%s]", SenderName);
		return;
	}

	SpoutLogNotice("SpoutFrameCount::EnableFrameCount : sender name [%s]", SenderName);

	// Close any existing semaphore
	if (m_hCountSemaphore) {
		CloseHandle(m_hCountSemaphore);
		m_hCountSemaphore = NULL;
		m_CountSemaphoreName[0] = 0;
	}

	// Set the new name for subsequent checks
	strcpy_s(m_SenderName, 256, SenderName);

	// Create or open a semaphore with this sender name
	sprintf_s(m_CountSemaphoreName, 256, "%s_Count_Semaphore", SenderName);
	HANDLE hSemaphore = CreateSemaphoreA(
		NULL, // default security attributes
		1, // initial count
		LONG_MAX, // maximum count - LONG_MAX (2147483647) at 60fps = 2071 days
		(LPSTR)m_CountSemaphoreName);

	DWORD dwError = GetLastError();
	if (dwError == ERROR_INVALID_HANDLE) {
		SpoutLogError("    Invalid semaphore handle");
		return;
	}
	if (dwError == ERROR_ALREADY_EXISTS) {
		SpoutLogNotice("    Semaphore already exists");
		// OK if it already exists - the sender or receiver can create it
	}
	if (hSemaphore == NULL) {
		SpoutLogError("    Unknown error");
		return;
	}

	m_hCountSemaphore = hSemaphore;

	SpoutLogNotice("    Semaphore handle [%d]", m_hCountSemaphore);

}

// -----------------------------------------------
void spoutFrameCount::DisableFrameCount()
{
	CleanupFrameCount();
	m_bDisabled = true;
}

// -----------------------------------------------
bool spoutFrameCount::IsFrameCountEnabled()
{
	if (!m_bFrameCount || m_bDisabled)
		return false;
	else
		return true;

}

// -----------------------------------------------
//
// Increment the sender frame count.
// Used by a sender for every update of the shared texture.
//
// This function is called within a sender mutex lock so that
// the receiver will not read the semaphore count while the
// sender is incrementing it.
//
// Used internaly to set frame status if frame counting is enabled.
//
void spoutFrameCount::SetNewFrame()
{
	// Return silently if disabled
	if (!m_bFrameCount || m_bDisabled)
		return;

	// Access the frame count semaphore
	// Note: WaitForSingle object will always succeed because
	// the lock count (sender frame count) is greater than zero,
	// but must be called before ReleaseSemaphore can be called
	// or there is an error.
	DWORD dwWaitResult = WaitForSingleObject(m_hCountSemaphore, 0);
	if (dwWaitResult != WAIT_OBJECT_0) {
		if (dwWaitResult == WAIT_ABANDONED)
			SpoutLogWarning("SpoutFrameCount::SetNewFrame - WAIT_ABANDONED");
		if (dwWaitResult == WAIT_FAILED)
			SpoutLogWarning("SpoutFrameCount::SetNewFrame - WAIT_FAILED");
	}
	else {
		// Release the frame counting semaphore to increase it's count.
		// so that the receiver can retrieve the new count.
		// Increment by 2 because WaitForSingleObject decremented it.
		if (ReleaseSemaphore(m_hCountSemaphore, 2, NULL) == false) {
			SpoutLogError("spoutFrameCount::SetNewFrame - ReleaseSemaphore failed");
		}
		else {
			m_FrameCount++; // Increment the sender frame count
			// Update the sender fps calculations for the new frame
			UpdateSenderFps(1);
		}
	}

}

// -----------------------------------------------
//
// Read the semaphore count to determine if the sender
// has produced a new frame and incremented the counter.
// Counts are recorded as class variables for a receiver.
//
// This function is called within a sender mutex lock so that
// the sender will not write a frame and increment the 
// count while a receiver is reading it.
//
// Used internally to check frame status if frame counting is enabled.
//
bool spoutFrameCount::GetNewFrame()
{
	long framecount = 0;

	// Return silently if disabled
	if (!m_bFrameCount || m_bDisabled)
		return true;

	// A receiver creates of opens a named semaphore when it connects to a sender
	// Do not block if semaphore creation failed so that ReceiveTexture can still be called
	if (!m_hCountSemaphore)
		return true;

	// Access the frame count semaphore
	DWORD dwWaitResult = WaitForSingleObject(m_hCountSemaphore, 0);
	if (dwWaitResult != WAIT_OBJECT_0) {
		if (dwWaitResult == WAIT_ABANDONED)
			SpoutLogWarning("SpoutFrameCount::GetNewFrame - WAIT_ABANDONED");
		if (dwWaitResult == WAIT_FAILED)
			SpoutLogWarning("SpoutFrameCount::GetNewFrame - WAIT_FAILED");
	}
	else {
		// Call ReleaseSemaphore with a release count of 1 to return it
		// to what it was before the wait and record the previous count.
		// The next time round it will either be the same count because
		// the receiver released it, or increased because the sender
		// released and incremented it.
		if (ReleaseSemaphore(m_hCountSemaphore, 1, &framecount) == false) {
			SpoutLogError("spoutFrameCount::GetNewFrame - ReleaseSemaphore failed");
			return true; // do not block
		}
	}

	// Update the global frame count
	m_FrameCount = framecount;

	// Count will still be zero for older apps that do not set a frame count
	if (framecount == 0)
		return true;

	// If this count and the last are the same, the sender has not
	// produced a new frame and incremented the counter.
	// Only return false if this frame and the last are the same.
	if (framecount == m_LastFrameCount) {
		m_bIsNewFrame = false;
		return false;
	}

	// Update the sender fps calculations.
	// The sender might have produced more than one frame if the receiver is slower.
	// Pass the number of frames produced since the last.
	UpdateSenderFps(framecount - m_LastFrameCount);

	// Reset the comparator
	m_LastFrameCount = framecount;

	// Signal a new frame
	m_bIsNewFrame = true;

	return true;
}


// -----------------------------------------------
void spoutFrameCount::CleanupFrameCount()
{
	// Return if not enabled in SpoutSettings
	if (!m_bFrameCount)
		return;

	// Return if application disabled
	if (m_bDisabled)
		return;

	// Return if no count semaphore
	// i.e. no sender started or cleanup already done
	if (!m_hCountSemaphore)
		return;

	SpoutLogNotice("SpoutFrameCount::CleanupFrameCount");

	// Close the frame count semaphore. If another application first
	// opened the semaphore it will not be finally closed here.
	CloseHandle(m_hCountSemaphore);
	m_hCountSemaphore = NULL;

	// Clear the sender name in case the same one opens again
	m_SenderName[0] = 0;

	// Reset counters
	m_FrameCount = 0L;
	m_LastFrameCount = 0L;
	m_Fps = GetRefreshRate();
	m_FrameTimeTotal = 0.0;
	m_FrameTimeNumber = 0.0;

}

// -----------------------------------------------
//
//  Is the received frame new ?
//
//  This function can be used by a receiver after ReceiveTexture
//	to determine whether the frame is new.
//
//	Used by an application to avoid time consuming processing
//	after receiving a texture.
//
//	Not usually required because new frame status is always 
//	checked internally if frame counting is enabled.
//
bool spoutFrameCount::IsFrameNew()
{
	return m_bIsNewFrame;
}

// -----------------------------------------------
double spoutFrameCount::GetSenderFps()
{
	return m_Fps;
}

// -----------------------------------------------
long spoutFrameCount::GetSenderFrame()
{
	return m_FrameCount;
}

// -----------------------------------------------
//
// Fps control
//
// Not necessary if the application already has frame rate control.
// Must be called every frame.
// The sender will then signal a new frame at the target rate.
// Purpose is control rather than accuracy.
// Use std::chrono if supported by the compiler VS2015 or greater
//
void spoutFrameCount::HoldFps(int fps)
{
	// Return if incorrect fps entry
	if (fps <= 0)
		return;

	double framerate = static_cast<double>(fps);

#if _MSC_VER >= 1900
	// Initialize frame time at target rate
	if (m_millisForFrame == 0.0) {
		m_millisForFrame = 1000.0 / framerate; // msec per frame
		*m_FrameStartPtr = std::chrono::steady_clock::now();
		SpoutLogNotice("spoutFrameCount::HoldFps(%d)", fps);
	}
	else {
		*m_FrameEndPtr = std::chrono::steady_clock::now();
		// milliseconds elapsed
		double elapsedTime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(*m_FrameEndPtr - *m_FrameStartPtr).count() / 1000.);
		// Sleep to reach the target frame time
		if (elapsedTime < m_millisForFrame) { // milliseconds
			std::this_thread::sleep_for(std::chrono::milliseconds((long)(m_millisForFrame - elapsedTime)));
		}
		// Set start time for the next frame
		*m_FrameStartPtr = std::chrono::steady_clock::now();
	}
#else
	if (m_millisForFrame == 0) {
		m_millisForFrame = 1000.0 / framerate;
		m_FrameStart = GetCounter();
		SpoutLogNotice("spoutFrameCount::HoldFps(%d)", fps);
	}
	else {
		double elapsedTime = GetCounter() - m_FrameStart; // msec
		// Sleep to reach the target frame time
		if (elapsedTime < m_millisForFrame)
			Sleep((DWORD)(m_millisForFrame - elapsedTime)); // can be slighly high
		// Set start time for the next frame
		m_FrameStart = GetCounter();
	}
#endif

}

// =================================================================
//                     Texture access mutex
// =================================================================

// -----------------------------------------------
bool spoutFrameCount::CreateAccessMutex(const char *SenderName)
{
	DWORD errnum = 0;
	char szMutexName[300];
	HANDLE hMutex = NULL;

	// Create the mutex name to control access to the shared texture
	sprintf_s((char*)szMutexName, 300, "%s_SpoutAccessMutex", SenderName);

	// Create or open mutex depending, on whether it already exists or not
	hMutex = CreateMutexA(NULL, false, (LPCSTR)szMutexName);

	if (hMutex == NULL) {
		SpoutLogError("spoutFrameCount::CreateAccessMutex - access mutex NULL invalid handle");
		return false;
	}
	else {
		errnum = GetLastError();
		if (errnum == ERROR_INVALID_HANDLE) {
			SpoutLogError("spoutFrameCount::CreateAccessMutex - access mutex [%s] invalid handle", szMutexName);
			return false;
		}
		SpoutLogNotice("spoutFrameCount::CreateAccessMutex - access mutex [%s] - 0x%x", szMutexName, hMutex);
	}

	m_hAccessMutex = hMutex;

	return true;

}

// -----------------------------------------------
void spoutFrameCount::CloseAccessMutex()
{
	// Close the texture access mutex. If another application first opened
	// the mutex it will not be finally closed here.
	if (m_hAccessMutex) CloseHandle(m_hAccessMutex);
	m_hAccessMutex = NULL;
}

// -----------------------------------------------
//
// Check whether any other process is holding the lock
// and wait for access for 4 frames if so.
// For receiving from Version 1 apps with no mutex lock,
// a reader will have created the mutex and will have
// sole access and rely on the interop locks
//
bool spoutFrameCount::CheckAccess()
{
	DWORD dwWaitResult = WAIT_FAILED;

	// Don't block if no mutex for Spout1 apps
	// or if called when the sender has closed
	// AllowAccess also tests for a null handle
	if (!m_hAccessMutex) {
		// SpoutLogWarning("CheckAccess - no Mutex");
		return true;
	}

	dwWaitResult = WaitForSingleObject(m_hAccessMutex, 67); // 4 frames at 60fps
	if (dwWaitResult == WAIT_OBJECT_0) {
		// The state of the object is signalled.
		return true;
	}
	else {
		switch (dwWaitResult) {
			case WAIT_ABANDONED: // Could return here
				SpoutLogError("CheckAccess - WAIT_ABANDONED");
				break;
			case WAIT_TIMEOUT: // The time-out interval elapsed, and the object's state is nonsignaled.
				// This can happen the first time a receiver connects to a sender
				// SpoutLogError("CheckAccess - WAIT_TIMEOUT");
				break;
			case WAIT_FAILED: // Could use call GetLastError
				SpoutLogError("CheckAccess - WAIT_FAILED");
				break;
			default:
					SpoutLogError("CheckAccess - unknown error");
			break;
		}
	}
	return false;
}

// -----------------------------------------------
void spoutFrameCount::AllowAccess()
{
	// Release ownership of the mutex object.
	// The caller must call ReleaseMutex once for each time that the mutex satisfied a wait.
	// The ReleaseMutex function fails if the caller does not own the mutex object
	if (m_hAccessMutex) 
		ReleaseMutex(m_hAccessMutex);
}


// ===============================================================================
//                                Protected
// ===============================================================================


// -----------------------------------------------
//
// Calculate the sender frames per second
// Applications before 2.007 have a frame rate dependent on the system fps
//
void spoutFrameCount::UpdateSenderFps(long framecount) {

	// If framecount is zero, the sender has not produced a new frame yet
	if (framecount > 0) {

		// Msecs between this frame and the last
		double thisFrame = GetCounter();
		double frametime = thisFrame - m_lastFrame;

		// Set the start time for the next frame
		m_lastFrame = thisFrame;

		// Msecs per frame if more than one frame has been produced by the sender
		if (framecount > 1L)
			frametime = frametime / static_cast<double>(framecount);

		// Average the last 16 frames to minimise variability for damping
		if (m_FrameTimeNumber < 16) {
			m_FrameTimeTotal = m_FrameTimeTotal + frametime;
			m_FrameTimeNumber += 1.0;
		}
		else {
			// Calculate the average frame time
			frametime = m_FrameTimeTotal / m_FrameTimeNumber;
			m_FrameTimeTotal = 0.0;
			m_FrameTimeNumber = 0.0;
			// Calculate frames per second
			// Default fps is system refresh rate
			frametime = frametime / 1000.0; // frame time in seconds
			if (frametime > 0.0001) {
				double fps = (1.0 / frametime); // Fps
				m_Fps = 0.75*m_Fps + 0.25*fps; // damping
			}
		}
	}
}

// -----------------------------------------------
//
// Get system refresh rate for the default fps value
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-enumdisplaysettingsa
//
double spoutFrameCount::GetRefreshRate()
{
	double frequency = 60.0;
	DEVMODE DevMode;
	BOOL bResult = true;
	DWORD dwCurrentSettings = 0;
	DevMode.dmSize = sizeof(DEVMODE);
	// Test all the graphics modes
	while (bResult) {
		bResult = EnumDisplaySettings(NULL, dwCurrentSettings, &DevMode);
		if (bResult)
			frequency = static_cast<double>(DevMode.dmDisplayFrequency);
		dwCurrentSettings++;
	}
	return frequency;
}

// -----------------------------------------------
//
// Set counter start
//
// Information on using QueryPerformanceFrequency for timing
// https://docs.microsoft.com/en-us/windows/desktop/SysInfo/acquiring-high-resolution-time-stamps
//
// Used in favour of std::chrono for compatibility with Visual Studio before VS2015
//
void spoutFrameCount::StartCounter()
{
	LARGE_INTEGER li;
	if (QueryPerformanceFrequency(&li)) {
		// Find the PC frequency if not done yet
		if(PCFreq == 0.0)
			PCFreq = static_cast<double>(li.QuadPart) / 1000.0;
		// Get the counter start
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}

}

// -----------------------------------------------
// Return msec elapsed since counter start
//
double spoutFrameCount::GetCounter()
{
	LARGE_INTEGER li;
	if (QueryPerformanceCounter(&li)) {
		return static_cast<double>(li.QuadPart - CounterStart) / PCFreq;
	}
	else {
		return 0.0;
	}
}

// ===============================================================================


