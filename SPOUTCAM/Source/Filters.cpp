/*

				SpoutCam
	
	Receives frames from a memory share or texture share Sender
	Simulates a webcam to other programs.

	The Sender must be started before the webcam is loaded
	by other software because this sets the frame size
	to the frames being sent by the Sender
	Currently there is no way to change the frame size
	after the filter has been loaded. If there is no Sender, 
	the default is 640x480 with a noise image

	Copyright 2013-2016 Lynn Jarvis - spout@zeal.co

	"SpoutCam" is free software: 
	you can redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	Credit for original capture source filter authored by Vivek :

	https://plus.google.com/100981910546057697518/posts

	Downloaded from :

	http://tmhare.mvps.org/downloads/vcam.zip

	Credit for alterations for Skype compatibility - John MacCormick, 2012.

	https://github.com/johnmaccormick/MultiCam
		
	"Spout" distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	See also the GNU general public license text file provided.
	If not found, see <http://www.gnu.org/licenses/>.

	SpoutCam is a modification of code by these authors and changes are clearly
	documented in this file. Any problems cannot be attributed to authors of
	previous versions.

	//
	// Revisions :
	//

	19-08-13 -started changes for synchronisation of FillBuffer
	with whether the filter is active or inactive - based on :
	22-08-13
		- used RED5 mods http://comSender.googlecode.com/svn/trunk/
		- succeeded in getting it working with Resolume and VLC
		- cleanup
	24.08-13
	found that 854 width is not supported, probably because widths have to be multiples of 4
	More info here :
	http://www.codeproject.com/Articles/437617/DirectShow-Virtual-Video-Capture-Source-Filter-in
	17-11-13 - noted a crash if "settings" are selected to show the property page
				which does not exist : possible information here :
				http://msdn.microsoft.com/en-us/library/windows/desktop/dd375480%28v=vs.85%29.aspx
	01.09.13 - minor cleanup
	28-09-13 - first release combined wgl interop and memory share
	13-10-13 - revision and cleanup
	16.10.13 - changed to ofxConnector to avoid confusion with original dxConnector
	09.11.13 - changed from memcpy to CopyMemory in vertflipbuf - very slight speed advantage
	11.11.13 - change project name to SpoutCam
	14.11.13 - check for intialized to avoid repeat of InitSharing in FillBuffer
	16.11.13 - added disconnect flag to protect against incorrect texture sharehandle or image size
	17.11.13 - added check for gl context in destructor to free fbo and texture and delete the context
	06.12.13 - changed ofxConnector to the same as for the FFGL plugins SpoutReceiver and videoshare
	06.12.13 - noted that the share handle can change even though the size remains the same
			   if a Vizzable device is dropped into the chain in Ableton. Allowed for that by reset
			   only if the image size remains the same. Still a problem if the image size changes.
	07-12-13 - added check for load of interop even though extensions loaded OK
			   It is possible that the extensions load OK, but that initialization will still fail
			   This occurs when wglDXOpenDeviceNV fails - noted on dual graphics machines with NVIDIA Optimus
	09.12.13 - changed to dll instead of constructor class
	12.12.13 - added a separate function to detect texture Sender change
	12.12.13 - allowed for change of active Sender as long as the image size is the same
	12.12.13 - changed back to spxConnector class instead of Spout.dll
	13.13.13 - started looking at a property page
	14.12.13 - found bug that if a Sender not running would leave initsharing with connected true
			 - changed default size from 640x480 to 320x240
	16.12.13 - moved hardware compatibility test to constructor but found that OPenGL context is needed
			   due to wglGetProcAddress
	19.12.13 - updated spxConnector after work with Processing sender project
	19.12.13 - removed cleanup after compatibility check and modified function in connector code
	19.12.13 - tidied up logic of initial compatibility check
	26.12.13 - added modified spxconnector again after more work with JSpout
	31.12.13 - Used Glew instead of dynamic load of extensions - in new version of spxConnector
	03.01.14 - changed to cleanded up connector code named back to ofxConnector
	17.01.14 - removed console window - inadvertently left active
	29.01.14 - included modified memoryshare with sender mutex
	07.02.14 - cleanup
	10.02.14 - major change to render the shared texture to a local texture then fillbuffer
			   This allows the camera image size to be independent of the shared texture image size
			   Limitation is that the camera has no properties to change size and remains as started
			   Benefit is that the initial size can be any size or shape
	14.02.14 - added modified memoryshare with check for sharing name on init
	26.02.14 - cleanup
	10.04.14 - changed all int width and height and functions to unsigned int
			 - changed to spxConnector common for 64bit compatibility
	24.04-14 - changed default frame rate to 60fps instead of 30fps - cured "stuttering" in Yawcam
			   VLC still needs strict timing control - simple method freezes
	05-07-14 - major change to use Spout SDK
	14-07-14 - used copymemory in FlipVertical instead of memcpy
	--------------------------------------------------------
	24.08.14 - recompiled with MB sendernames class revision
	29.08.14 - recompiled after changes with FFGL plugins
	03.09.14 - used DrawSharedTexture in render to local texture in FillBuffer
			 - update after SDK testing
	30.08.14 - Recompiled with SDK update
	12.10.14 - recompiled for release
	21.10.14 - Recompile for update V 2.001 beta
	02.01.15 - Added GL_BGR to extensions to avoid using GLEW
			   Recompile after SDK changes
	04.02.15 - added conditional compile for DirectX 9 or DirectX 11
			 - changed to direct OpenGL context creation rather than using glut
	08.02.15 - Set texture format for ReceiveImage to GL_RGB. SDK default is now GL_RGBA.
	25.04.15 - Changed from graphics auto detection to set DirectX mode to optional installer
	08.06.15 - Created a dummy window for OpenGL context creation due to SetPixelFormat error in Mapio
			 - needs testing with different programs and 64bit Windows 7 and 8
	17.06.15 - Transferred project to Win7 64bit SSD drive
			   Missing strmbase.lib - generated from DirectShow base classes - copied to a "libs" folder
	07.07.15 - Transferred project back to Win 7 32bit 
	17.08.15 - Removed SetDX9 - now done by registry setting
			   Recompile for 2.004 release 32bit Win32 VS2010 /MT
	15.09.15 - Recompiled for 2.005 memoryshare, changed memoryshare read section. 32bit Win32 VS2010 /MT.
	19.09.15 - Removed DX9 option
	20.09.15 - Sorted memoryshare readtexture RGBA external and RGB internal and local invert
	22.09.15 - Work on property page archived and removed from working source.
	27.09.15 - Modifications for Skype compatibility based on MultiCam by John MacCormick.
			 - changed back to simple timing method due to crash in Skype with RED5 method.
	28.09.15 - Found NULL clock with GetSyncSource for Skype, so fixed crash and enabled detailed timimg again.
	11.10.15 - Renamed DrawSharedTexture to DrawReceivedTexture to avoid confusion with SDK function
	15.12.15 - Changed dll name from "Spout Cam to "SpoutCam" - in dll.cpp
	15.12.15 - Rebuild for 2.005 VS2012 /MT
	11.03.16 - Major change for ReceiveImage into a local texture and rgb buffer and rescaling functions
	25.03.16 - Revised isExtensionSupported for glGetStringi
	20.05.16 - Remove testing of sender selection - not working.
			 - Rebuild for 2.005 release  VS2012 /MT

*/


#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include <streams.h>
#include <stdio.h>
#include <conio.h>
#include <olectl.h>
#include <dvdmedia.h>
#include "filters.h"

static HWND hwndButton = NULL; // dummy window for opengl context

//////////////////////////////////////////////////////////////////////////
//  CVCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI CVCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

	/*
	// debug console window
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutCam ~~ 30-03-16\n");
	*/

    CUnknown *punk = new CVCam(lpunk, phr);

    return punk;
}

CVCam::CVCam(LPUNKNOWN lpunk, HRESULT *phr) : 
    CSource(NAME("SpoutCam"), lpunk, CLSID_SpoutCam)
{
    ASSERT(phr);

    CAutoLock cAutoLock(&m_cStateLock);
    
	// Create the one and only output pin
    m_paStreams = (CSourceStream **)new CVCamStream*[1];
    m_paStreams[0] = new CVCamStream(phr, this, L"SpoutCam");

}


// Retrieves pointers to the supported interfaces on an object.
// This method calls IUnknown::AddRef on the pointer it returns.
HRESULT CVCam::QueryInterface(REFIID riid, void **ppv)
{
	//Forward request for IAMStreamConfig & IKsPropertySet to the pin
    if(riid == _uuidof(IAMStreamConfig) || 
		riid == _uuidof(IAMDroppedFrames)  ||
		riid == _uuidof(IKsPropertySet))
        return m_paStreams[0]->QueryInterface(riid, ppv);
    else
        return CSource::QueryInterface(riid, ppv);
}



// If a filter cannot deliver data for some reason, it returns VFW_S_CANT_CUE. 
HRESULT CVCam::GetState(DWORD dw, FILTER_STATE *pState)
{
	CheckPointer(pState, E_POINTER);
	*pState = m_State;
	if (m_State == State_Paused) {
		return VFW_S_CANT_CUE;
	}
	else
		return S_OK;
}




STDMETHODIMP CVCam::JoinFilterGraph(
		__inout_opt IFilterGraph * pGraph,
		__in_opt LPCWSTR pName)
{
	HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
	return hr;
}


///////////////////////////////////////////////////////////
// all inherited virtual functions
///////////////////////////////////////////////////////////
CBasePin *CVCam::GetPin(int n)
{
	CBasePin *pin = CSource::GetPin(n);
	ASSERT(pin!=NULL);
	return pin;
}


int CVCam::GetPinCount()
{
	return CSource::GetPinCount();
}


HRESULT CVCam::StreamTime(CRefTime& rtStream)
{
	return CSource::StreamTime(rtStream);
}

LONG CVCam::GetPinVersion()
{
	return CSource::GetPinVersion();
}

__out_opt LPAMOVIESETUP_FILTER CVCam::GetSetupData()
{
	return CSource::GetSetupData();
}

HRESULT STDMETHODCALLTYPE CVCam::EnumPins(__out  IEnumPins **ppEnum)
{
	return CSource::EnumPins(ppEnum);
}

HRESULT STDMETHODCALLTYPE CVCam::FindPin(LPCWSTR Id, __out  IPin **ppPin)
{
	return CSource::FindPin(Id, ppPin);
}

HRESULT STDMETHODCALLTYPE CVCam::QueryFilterInfo(__out  FILTER_INFO *pInfo)
{
	return CSource::QueryFilterInfo(pInfo);
}

HRESULT STDMETHODCALLTYPE CVCam::QueryVendorInfo(__out  LPWSTR *pVendorInfo)
{
	return CSource::QueryVendorInfo(pVendorInfo);
}


HRESULT STDMETHODCALLTYPE CVCam::Stop( void)
{
	return CSource::Stop( );
}

HRESULT STDMETHODCALLTYPE CVCam::Pause( void)
{
	return CSource::Pause( );
}

HRESULT STDMETHODCALLTYPE CVCam::Run(REFERENCE_TIME tStart)
{
	HRESULT hr = CSource::Run(tStart);
	return hr;
}


HRESULT STDMETHODCALLTYPE CVCam::SetSyncSource(__in_opt  IReferenceClock *pClock)
{
	return CSource::SetSyncSource(pClock);
}

HRESULT STDMETHODCALLTYPE CVCam::GetSyncSource(__deref_out_opt  IReferenceClock **pClock)
{
	return CSource::GetSyncSource(pClock);
}

STDMETHODIMP CVCam::GetClassID(__out CLSID *pClsID)
{
	return CSource::GetClassID(pClsID);
}

ULONG STDMETHODCALLTYPE CVCam::AddRef( void)
{
	return CSource::AddRef( );
}

ULONG STDMETHODCALLTYPE CVCam::Release( void)
{
	return CSource::Release( );
}

HRESULT STDMETHODCALLTYPE CVCam::Register( void)
{
	return CSource::Register( );
}

HRESULT STDMETHODCALLTYPE CVCam::Unregister( void)
{
	return CSource::Unregister( );
}
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// CVCamStream is the one and only output pin of CVCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
CVCamStream::CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName) :
    CSourceStream(NAME("SpoutCam"), phr, pParent, pPinName), m_pParent(pParent)
{

	bMemoryMode		= false; // Default mode is texture, true means memoryshare
	bDX9mode        = false; // Not currently used
	bInvert         = true;  // Not currently used
	bInitialized	= false; // Spoutcam reiver
	bGLinitialized	= false; // OpenGL
	bDisconnected	= false; // Has to connect before can disconnect or it will never connect
	glContext		= NULL;  // Context is established within this application
	g_Width			= 640;	 // if there is no Sender, getmediatype will use defaults
	g_Height		= 480; 
	g_SenderWidth	= 640;	 // give it an initial size - this will be changed if a sender is running at start
	g_SenderHeight	= 480;
	g_senderBuffer  = NULL;  // local rgb buffer the same size as the sender (can be a different size to the filter)
	g_SenderName[0] = 0;

	//
	// On startup get the active Sender name if any.
	//
	if(receiver.GetActiveSender(g_SenderName)) {
		// Set the global width and height
		receiver.GetImageSize(g_SenderName, g_SenderWidth, g_SenderHeight, bMemoryMode);
		g_Width  = g_SenderWidth;
		g_Height = g_SenderHeight;
	}


	// Cannot use receiver.GetMemoryShareMode() here because 
	// it requires an OpenGL context, so look at the registry directly.
	DWORD dwMemory = 0;
	if(receiver.spout.interop.spoutdx.ReadDwordFromRegistry(&dwMemory, "Software\\Leading Edge\\Spout", "MemoryShare")) {
		if(dwMemory == 1) {
			bMemoryMode = true;
		}
	}

	bDX9mode = receiver.GetDX9(); // Currently not used, might use this flag later

	// Set mediatype to shared width and height or if it did not connect set defaults
	GetMediaType(4, &m_mt);

	NumDroppedFrames = 0;
	NumFrames = 0;
	hwndButton = NULL; // ensure NULL of static variable for the OpenGL window handle

}

CVCamStream::~CVCamStream()
{
	/*
	DWORD dwSpoutPanel = 1;
	char sendername[256];
	sendername[0] = 0;

	if(!m_pParent->IsActive() && !bDisconnected) {
		// Is there an instance running and a sender running ?
		if(receiver.spout.ReadPathFromRegistry(sendername, "Software\\Leading Edge\\SpoutCam\\", "Sender")) {
			if(sendername[0] != 0) { // a SpoutCam instance has started
				receiver.SelectSenderPanel(); // Choose the active sender
				// Write a registry flag to inform the other instance
				receiver.spout.interop.spoutdx.WriteDwordToRegistry(dwSpoutPanel, "Software\\Leading Edge\\SpoutCam\\", "SpoutPanel");
			}
		}
	}
	*/

	HGLRC ctx = wglGetCurrentContext();
	if(ctx != NULL) {
		if(bInitialized) receiver.ReleaseReceiver();
		if(glContext != NULL) { 
			// global context handle
			wglDeleteContext(glContext); // try to prevent initgl twice
		}
	}

	// if(bInitialized) receiver.ReleaseReceiver(); // TODO : check for context required
	if(g_senderBuffer) free((void *)g_senderBuffer);

	// Destroy dummy window used for OpenGL context creation
	if(hwndButton) DestroyWindow(hwndButton);

} 

HRESULT CVCamStream::QueryInterface(REFIID riid, void **ppv)
{   
	// Standard OLE stuff
    if(riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*)this;
    else if(riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*)this;
	else if(riid == _uuidof(IAMDroppedFrames)  )
		*ppv = (IAMDroppedFrames*)this;
	else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef();

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//	Modified as per red5 to allow for dropped frames and reset of time stamps
//
//  http://comSender.googlecode.com/svn/trunk/
//
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::FillBuffer(IMediaSample *pms)
{
	unsigned int imagesize, width, height;
	long l, lDataLen;
	bool bResult = false;
	// DWORD dwSpoutPanel = 0;
	HRESULT hr=S_OK;;
    BYTE *pData;
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();


	// If graph is inactive stop cueing samples
	if(!m_pParent->IsActive()) {
		return S_FALSE;
	}

	// first get the timing right
	// create some working info
	REFERENCE_TIME rtNow, rtDelta, rtDelta2=0; // delta for dropped, delta 2 for sleep.
	REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;
	
	// What Time is it REALLY ???
	// m_pClock is returned NULL with Skype, but OK for YawCam and VLC
	m_pParent->GetSyncSource(&m_pClock); 
	if(m_pClock) {
		m_pClock->GetTime(&refSync1);
		m_pClock->Release();
	}
	else {
		refSync1 = NumFrames*avgFrameTime;
	}


	if(NumFrames <= 1) {
		// initiate values
		refStart = refSync1; // FirstFrame No Drop.
		refSync2 = 0;
 	}

	// Set the timestamps that will govern playback frame rate.
    // The current time is the sample's start
	rtNow = m_rtLastTime;
	m_rtLastTime = avgFrameTime + m_rtLastTime;
	
	// IAMDropppedFrame. We only have avgFrameTime to generate image.
	// Find generated stream time and compare to real elapsed time
	rtDelta=((refSync1-refStart)-(((NumFrames)*avgFrameTime)-avgFrameTime));

	if(rtDelta-refSync2 < 0) { 
		//we are early
		rtDelta2=rtDelta-refSync2;
		if( abs(rtDelta2/10000)>=1)
			Sleep(abs(rtDelta2/10000));
	} // endif (rtDelta-refSync2 < 0)
	else if(rtDelta/avgFrameTime>NumDroppedFrames) {
		// new dropped frame
		NumDroppedFrames = rtDelta/avgFrameTime;
		// Figure new RT for sleeping
		refSync2 = NumDroppedFrames*avgFrameTime;
		// Our time stamping needs adjustment.
		// Find total real stream time from start time
		rtNow = refSync1-refStart;
		m_rtLastTime = rtNow+avgFrameTime;
		pms->SetDiscontinuity(true);
	} // end else if(rtDelta/avgFrameTime>NumDroppedFrames)

	// The SetTime method sets the stream times when this sample should begin and finish.
    hr = pms->SetTime(&rtNow, &m_rtLastTime);
	// Set true on every sample for uncompressed frames
    hr = pms->SetSyncPoint(true);
	// ============== END OF INITIAL TIMING ============

	// Check access to the sample's data buffer
    pms->GetPointer(&pData);
	if(pData == NULL) {
		return NOERROR;
	}


	// Get the current frame size for texture transfers
    imagesize = (unsigned int)pvi->bmiHeader.biSizeImage;
	width = (unsigned int)pvi->bmiHeader.biWidth;
	height = (unsigned int)pvi->bmiHeader.biHeight;
	if(width == 0 || height == 0) {
		return NOERROR;
	}

	// Don't do anything if disconnected because it will already have connected
	// previously and something has changed. It can only disconnect after it has connected.
	if(!bDisconnected) {

		// If connected, sizes should be OK, but check again
		unsigned int size = (unsigned int)pms->GetSize();
		imagesize = width*height*3; // Retrieved above
		if(size != imagesize) {
			if(bInitialized) receiver.ReleaseReceiver();
			bInitialized = false;
			bDisconnected = true; // don't try again
			return NOERROR;
		}


		// Quit if nothing running at all
		if(!receiver.GetActiveSender(g_ActiveSender)) {
			if(bInitialized) {
				receiver.ReleaseReceiver();
				bInitialized = false;
				/*
				// Reset the registry entries for SpoutCam
				dwSpoutPanel = 0;
				receiver.spout.interop.spoutdx.WriteDwordToRegistry(dwSpoutPanel, "Software\\Leading Edge\\SpoutCam\\", "SpoutPanel");
				receiver.spout.WritePathToRegistry("", "Software\\Leading Edge\\SpoutCam\\", "Sender");
				*/
			}
			goto ShowStatic;
		}

		/*
		// Has SpoutPanel been opened
		HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
		if(hMutex) {
			bSpoutPanelOpened = true;
			// We opened it so close it, otherwise it is never released
			CloseHandle(hMutex);
		}
		else {
			// Wait for SpoutPanel to close
			if(bSpoutPanelOpened) {
				// Check the registry for the SpoutPanel flag
				dwSpoutPanel = 0;
				if(receiver.spout.interop.spoutdx.ReadDwordFromRegistry(&dwSpoutPanel, "Software\\Leading Edge\\SpoutCam\\", "SpoutPanel")) {
					if(dwSpoutPanel == 1) {
						if(bInitialized) receiver.ReleaseReceiver();
						bInitialized = false; // start again
					}
				}
				// Reset the registry flag
				dwSpoutPanel = 0;
				receiver.spout.interop.spoutdx.WriteDwordToRegistry(dwSpoutPanel, "Software\\Leading Edge\\SpoutCam\\", "SpoutPanel");
				bSpoutPanelOpened = false;
			}
		} // end SpoutPanel check
		*/

		// everything ready
		if(!bInitialized) {

			// If not initialized, look for a sender
			if(receiver.GetActiveSender(g_SenderName)) {
				
				// Initialize OpenGl if is has not been done
				if(!bGLinitialized) {
					if(InitOpenGL()) {
						// Call OpenSpout so that OpenGL extensions are loaded
						receiver.spout.OpenSpout();
					}
					else {
						bGLinitialized = false;
						bDisconnected = true; // don't try again
						return NOERROR;
					}
				}

				// Found a sender so initialize the receiver
				if(receiver.CreateReceiver(g_SenderName, g_SenderWidth, g_SenderHeight)) {
					
					// Create a local rgb buffer for data tranfser from the shared texture
					if(g_senderBuffer) free((void *)g_senderBuffer);
					g_senderBuffer = (unsigned char *)malloc(g_SenderWidth*g_SenderHeight*3*sizeof(unsigned char));
							
					// Write the sender path to the registry for SpoutPanel
					receiver.spout.WritePathToRegistry(g_SenderName, "Software\\Leading Edge\\SpoutCam", "Sender");
					bInitialized = true;
					NumFrames++;
					return NOERROR; // no more for this frame
				}
				else {
					// TODO : what
				}
			} // end found a sender
		} // end not initialized
		else {

			// Receive the shared texture or memoryshare pixels into a local rgba OpenGL texture
			width = g_SenderWidth; // for sender size check
			height = g_SenderHeight;

			if(receiver.ReceiveImage(g_SenderName, width, height, g_senderBuffer, GL_RGB)) {

				// Sender size check
				if(g_SenderWidth != width || g_SenderHeight != height) {
					g_SenderWidth  = width;
					g_SenderHeight = height;
					// restart to initialize with the new size
					receiver.ReleaseReceiver();
					bInitialized = false;
					NumFrames++;
					return NOERROR;					
				}

				if(g_SenderWidth != g_Width || g_SenderHeight != g_Height) {
					// For different sender and filter sizes, resample the sender buffer into the filter buffer.
					rgb2bgrResample(g_senderBuffer, (unsigned char *)pData, g_SenderWidth, g_SenderHeight, g_Width, g_Height, true);
				}
				else {
					// Otherwise if the buffer dimensions match just convert rgb to bgr
					rgb2bgr((void *)g_senderBuffer, (void *)pData, g_SenderWidth, g_SenderHeight, true);
				}

				NumFrames++;
				return NOERROR;

			} // endif received OK
			else {
				receiver.ReleaseReceiver();
				bInitialized = false;
			} // endif received texture OK
		} // endif initialized
	} // endif not disconnected

ShowStatic :

	// drop through to default static image if it did not work
	pms->GetPointer(&pData);
	lDataLen = pms->GetSize();
	for(l = 0; l <lDataLen; ++l) 
		pData[l] = rand();

	NumFrames++;

	return NOERROR;

} // FillBuffer




//
// Notify
// Ignore quality management messages sent from the downstream filter
STDMETHODIMP CVCamStream::Notify(IBaseFilter * pSender, Quality q)
{
    return E_NOTIMPL;
} // Notify


//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
// Called when a media type is agreed between filters
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::SetMediaType(const CMediaType *pmt)
{
	DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());
    
	// Pass the call up to my base class
	HRESULT hr = CSourceStream::SetMediaType(pmt);

    return hr;
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT CVCamStream::GetMediaType(int iPosition, CMediaType *pmt)
{
	unsigned int width, height;

	if(iPosition < 0) {
		return E_INVALIDARG;
	}
    if(iPosition > 8) { // TODO - needs work - only one position
		return VFW_S_NO_MORE_ITEMS;
	}
	
    if(iPosition == 0) {
        *pmt = m_mt;
        return S_OK;
    }

    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

 	// Allow for default
	if(g_Width == 0 || g_Height == 0) {
		width  = 640;
		height = 480;
	}
	else {
		// as per Spout sender received
		width	=  g_Width;
		height	=  g_Height;
	}
	
	pvi->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth				= (LONG)width;
	pvi->bmiHeader.biHeight				= (LONG)height;
	pvi->bmiHeader.biPlanes				= 1;
	pvi->bmiHeader.biBitCount			= 24;
	pvi->bmiHeader.biCompression		= 0; // defaults 
	pvi->bmiHeader.biSizeImage			= 0;
	pvi->bmiHeader.biClrImportant		= 0;
	pvi->bmiHeader.biSizeImage			= GetBitmapSize(&pvi->bmiHeader);

	// The desired average display time of the video frames, in 100-nanosecond units. 
	// 60fps = 166667
	// 30fps = 333333
	pvi->AvgTimePerFrame = 166667; // 60fps

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(false);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
	pmt->SetVariableSize(); // LJ - to be checked

    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return NOERROR;

} // GetMediaType


// This method is called to see if a given output format is supported
HRESULT CVCamStream::CheckMediaType(const CMediaType *pMediaType)
{
	if(*pMediaType != m_mt) 
        return E_INVALIDARG;

    return S_OK;
} // CheckMediaType




//
// This method is called after the pins are connected to allocate buffers to stream data
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CVCamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

	// printf("CVCamStream::DecideBufferSize()\n");

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory. NOTE: the function
    // can succeed (return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted.
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties, &Actual);

    if(FAILED(hr)) return hr;

	// Is this allocator unsuitable?
    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;

    return NOERROR;

} // DecideBufferSize


// Called when graph is run
HRESULT CVCamStream::OnThreadCreate()
{
    m_rtLastTime = 0;
	dwLastTime = 0;
	IMediaSample* pSample = NULL;
	NumDroppedFrames = 0;
	NumFrames = 0;

    return NOERROR;

} // OnThreadCreate


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CVCamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
	// http://kbi.theelude.eu/?p=161
	if(!pmt) return S_OK; // Default? red5

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pmt->pbFormat);
	VIDEOINFOHEADER *mvi = (VIDEOINFOHEADER *)(m_mt.Format ());
	if(pvi->bmiHeader.biHeight !=mvi->bmiHeader.biHeight || 
		pvi->bmiHeader.biWidth  != mvi->bmiHeader.biWidth || 
		pvi->bmiHeader.biBitCount !=mvi->bmiHeader.biBitCount  )
		return VFW_E_INVALIDMEDIATYPE;	

	// fps
	// if(pvi->AvgTimePerFrame <10000000/30)
	if(pvi->AvgTimePerFrame <10000000/60)
		return VFW_E_INVALIDMEDIATYPE;
	if(pvi->AvgTimePerFrame <1)
		return VFW_E_INVALIDMEDIATYPE;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    *ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	*piCount = 1; // LJ
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{

	unsigned int width, height;

    *pmt = CreateMediaType(&m_mt);
    DECLARE_PTR(VIDEOINFOHEADER, pvi, (*pmt)->pbFormat);

	if (iIndex == 0) iIndex = 1;

	if(g_Width == 0 || g_Height == 0) {
		width  = 320;
		height = 240;
	}
	else {
		// as per sending app
		width	=  g_Width;
		height	=  g_Height;
	}

	pvi->bmiHeader.biCompression	= BI_RGB;
    pvi->bmiHeader.biBitCount		= 24;
    pvi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth			= (LONG)width;
    pvi->bmiHeader.biHeight			= (LONG)height;
    pvi->bmiHeader.biPlanes			= 1;
    pvi->bmiHeader.biSizeImage		= GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant	= 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    (*pmt)->majortype				= MEDIATYPE_Video;
    (*pmt)->subtype					= MEDIASUBTYPE_RGB24;
    (*pmt)->formattype				= FORMAT_VideoInfo;
    (*pmt)->bTemporalCompression	= false;
    (*pmt)->bFixedSizeSamples		= false;
    (*pmt)->lSampleSize				= pvi->bmiHeader.biSizeImage;
    (*pmt)->cbFormat				= sizeof(VIDEOINFOHEADER);
    
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
    
    pvscc->guid = FORMAT_VideoInfo;
    pvscc->VideoStandard = AnalogVideo_None;
	// Native size of the incoming video signal. 
	// For a compressor, the size is taken from the input pin.
	// For a capture filter, the size is the largest signal the filter 
	// can digitize with every pixel remaining unique.
	// Note  Deprecated.
    pvscc->InputSize.cx			= 1920;
    pvscc->InputSize.cy			= 1080;
    pvscc->MinCroppingSize.cx	= 0; // LJ was 80 but we don't want to limit it
    pvscc->MinCroppingSize.cy	= 0; // was 60
    pvscc->MaxCroppingSize.cx	= 1920;
    pvscc->MaxCroppingSize.cy	= 1080;
    pvscc->CropGranularityX		= 1; // seems 1 is not necessary
    pvscc->CropGranularityY		= 1;
    pvscc->CropAlignX = 0;
    pvscc->CropAlignY = 0;

    pvscc->MinOutputSize.cx		= 80; // LJ fair enough
    pvscc->MinOutputSize.cy		= 60;
    pvscc->MaxOutputSize.cx		= 1920; // 1080p
    pvscc->MaxOutputSize.cy		= 1080;
    pvscc->OutputGranularityX	= 1;
    pvscc->OutputGranularityY	= 1;
    pvscc->StretchTapsX			= 0;
    pvscc->StretchTapsY			= 0;
    pvscc->ShrinkTapsX			= 0;
    pvscc->ShrinkTapsY			= 0;
	pvscc->MinFrameInterval = 166667;   // 60 fps 333333; // 30fps  // LJ what is the consequence of this ?
    pvscc->MaxFrameInterval = 50000000; // 0.2 fps
    pvscc->MinBitsPerSecond = (80 * 60 * 3 * 8) / 5;
    pvscc->MaxBitsPerSecond = 1920 * 1080 * 3 * 8 * 30; // (integral overflow at 60 - anyway we lock on to 30fps and 1920 might not achieve 60fps)

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//			IAMDroppedFrames
///////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CVCamStream::GetNumNotDropped (long* plNotDropped)
{
	
	if (!plNotDropped) 
		return E_POINTER;
	
	*plNotDropped=NumFrames;
		return NOERROR;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetNumDropped (long* plDropped)
{
	if (!plDropped) 
		return E_POINTER;
	
	*plDropped=NumDroppedFrames;
		return NOERROR;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetDroppedInfo (long lSize,long *plArraym,long* plNumCopied)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetAverageFrameSize (long* plAverageSize)
{
	if(!plAverageSize)return E_POINTER;
	*plAverageSize=307200;
	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{
	// Set: Cannot set any properties.
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CVCamStream::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
	)
{

    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK;			// Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;	// The buffer is too small.
        
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;

    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CVCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryPinInfo(__out  PIN_INFO *pInfo)
{
	return CSourceStream::QueryPinInfo(pInfo);
}

STDMETHODIMP  CVCamStream::NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
{
	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT STDMETHODCALLTYPE CVCamStream::ConnectedTo(__out  IPin **pPin)
{
	return CSourceStream::ConnectedTo(pPin);
}

HRESULT STDMETHODCALLTYPE CVCamStream::EnumMediaTypes(__out  IEnumMediaTypes **ppEnum)
{
	return CSourceStream::EnumMediaTypes(ppEnum);
}

LONG  CVCamStream::GetMediaTypeVersion()
{
	return CSourceStream::GetMediaTypeVersion();
}

HRESULT  CVCamStream::CompleteConnect(IPin *pReceivePin)
{
	return CSourceStream::CompleteConnect(pReceivePin);
}

HRESULT  CVCamStream::CheckConnect(IPin *pPin)
{
	return CSourceStream::CheckConnect(pPin);
}

HRESULT  CVCamStream::BreakConnect()
{
	return CSourceStream::BreakConnect();
}

HRESULT STDMETHODCALLTYPE CVCamStream::Connect(IPin *pReceivePin,	__in_opt  const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::Connect( pReceivePin, pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::ReceiveConnection( pConnector,pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::Disconnect( void)
{
	bDisconnected = true;

	/*
	// Reset the registry entries for SpoutCam
	DWORD dwSpoutPanel = 0;
	receiver.spout.interop.spoutdx.WriteDwordToRegistry(dwSpoutPanel, "Software\\Leading Edge\\SpoutCam\\", "SpoutPanel");
	receiver.spout.WritePathToRegistry("", "Software\\Leading Edge\\SpoutCam\\", "Sender");
	*/

	return CSourceStream::Disconnect( );
}


HRESULT STDMETHODCALLTYPE CVCamStream::ConnectionMediaType(__out  AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::ConnectionMediaType(pmt);
}


HRESULT STDMETHODCALLTYPE CVCamStream::QueryDirection(__out  PIN_DIRECTION *pPinDir)
{
	return CSourceStream::QueryDirection(pPinDir);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryId(__out  LPWSTR *Id)
{
	return CSourceStream::QueryId(Id);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::QueryAccept(pmt);
}


HRESULT STDMETHODCALLTYPE CVCamStream::QueryInternalConnections(__out_ecount_part_opt(*nPin, *nPin) IPin **apPin, ULONG *nPin)
{
	return CSourceStream::QueryInternalConnections(apPin, nPin);
}

HRESULT STDMETHODCALLTYPE CVCamStream::EndOfStream( void)
{
	return CSourceStream::EndOfStream( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::BeginFlush( void)
{
	return CSourceStream::BeginFlush( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::EndFlush( void)
{
	return CSourceStream::EndFlush( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	return CSourceStream::NewSegment(tStart,tStop, dRate);
}

HRESULT CVCamStream::Active(void)  {
	return CSourceStream::Active();
}



bool CVCamStream::InitOpenGL()
{
	HDC hdc = NULL;
	HWND hwnd = NULL;
	HGLRC hRc = NULL;

	glContext = wglGetCurrentContext();
	// printf("InitOpenGL (ctx = %d)\n", glContext);

	// Once created it seems stable and retained
	if(glContext == NULL) {

		// printf("InitOpenGL creating OpenGL windows\n");

		// We only need an OpenGL context with no render window because we don't draw to it
		// so create an invisible dummy button window. This is then independent from the host
		// program window (GetForegroundWindow). If SetPixelFormat has been called on the
		// host window it cannot be called again. This caused a problem in Mapio and could be
		// a problem with VirtualDJ.
		//
		// Microsoft :
		//
		// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
		//
		// If hdc references a window, calling the SetPixelFormat function also changes the pixel
		// format of the window. Setting the pixel format of a window more than once can lead to
		// significant complications for the Window Manager and for multithread applications,
		// so it is not allowed. An application can only set the pixel format of a window one time.
		// Once a window's pixel format is set, it cannot be changed.
		//
		if(!hwndButton || !IsWindow(hwndButton)) {
			hwndButton = CreateWindowA("BUTTON",
				            "SpoutCam",
					        WS_OVERLAPPEDWINDOW,
						    0, 0, 32, 32,
							NULL, NULL, NULL, NULL);
		}

		if(!hwndButton) { 
			MessageBoxA(NULL, "Error 1\n", "InitOpenGL", MB_OK);
			return false; 
		}

		hdc = GetDC(hwndButton);
		if(!hdc) { 
			MessageBoxA(NULL, "Error 2\n", "InitOpenGL", MB_OK); 
			return false; 
		}

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory( &pfd, sizeof( pfd ) );
		pfd.nSize = sizeof( pfd );
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int iFormat = ChoosePixelFormat(hdc, &pfd);
		if(!iFormat) { 
			MessageBoxA(NULL, "Error 3\n", "InitOpenGL", MB_OK);
			return false; 
		}

		if(!SetPixelFormat(hdc, iFormat, &pfd)) {
			DWORD dwError = GetLastError();
			// 2000 (0x7D0) The pixel format is invalid.
			// Caused by repeated call of  the SetPixelFormat function
			char temp[128];
			sprintf_s(temp, "InitOpenGL Error 4\nSetPixelFormat\nError %d (%x)", dwError, dwError);
			MessageBoxA(NULL, temp, "InitOpenGL", MB_OK); 
			return false; 
		}

		hRc = wglCreateContext(hdc);
		// GLerror();
		// 1282 (0x502)
		if(!hRc) { 
			MessageBoxA(NULL, "Error 5\n", "InitOpenGL", MB_OK); 
			return false; 
		}

		if(wglMakeCurrent(hdc, hRc)) {
			GLerror();
			glContext = wglGetCurrentContext();
			GLerror();
			if(glContext == NULL) {
				MessageBoxA(NULL, "Error 6\n", "InitOpenGL", MB_OK);
				return false; 
			}
		}
		else {
			MessageBoxA(NULL, "Error 7\n", "InitOpenGL", MB_OK);
			return false; 
		}

		// Drop through to return true
		// int nCurAvailMemoryInKB = 0;
		// glGetIntegerv(0x9049, &nCurAvailMemoryInKB);
		// printf("Memory available [%i]\n", nCurAvailMemoryInKB);

	} // end no glcontext 

	return true;

} // end InitOpenGL


// For debugging - (gluErrorString needs glu32.lib)
void CVCamStream::GLerror() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("GL error = %d (0x%x)\n", err, err);
		// printf("GL error = %d (0x%x) %s\n", err, err, gluErrorString(err));
	}
}	



// Adapted from : http://nehe.gamedev.net/tutorial/playing_avi_files_in_opengl/23001/
// Inline asm only supported for 32bit by Visual Studio.
void CVCamStream::rgb2bgr(void* source, void *dest, unsigned int width, unsigned int height, bool bInvert)
{
    void* a = source;         // Source rgb buffer
	void* b = dest;           // Destination bgr buffer
	unsigned int h = height;  // Line counter
	unsigned int w = width;   // Line size in pixels
	unsigned int s = 0;	      // Source buffer size for invert
	unsigned int t = 0;       // Source line size for invert

	if(bInvert) {
		s = width*height*3;    // Source buffer size
		t = width*3;           // Source line size
	}

    __asm                      // Assembler Code To Follow
    {
        mov ebx, a             // Points ebx to source data (a)
		mov edx, b             // Points ebx To rgb data (b)

		// Invert
		add ebx, s             // end of source buffer
		sub ebx, t             // beginning of the last line

        mov ecx, h             // Line counter

		lines:

		push ecx
		mov ecx, w             // Bytes per line source

        label:                 // Label Used For Looping

		    // loop though 3 bytes rgb and bgr

			// b
            mov al,[ebx+2]     // Loads Value At ebx Into al - rgb source
			mov [edx],al       // Stores Value In al At edx - bgr dest
            inc ebx            // rgb source buffer increment
			inc edx            // bgr dest buffer increment

			// g
            mov al,[ebx]       // Loads Value At ebx Into al      rgb source
			mov [edx],al       // Stores Value In al At edx       bgr dest
            inc ebx            // rgba buffer increment
			inc edx            // rgb buffer increment

			// r
            mov al,[ebx-2]     // Loads Value At ebx Into al      rgb source
			mov [edx],al       // Stores Value In al At edx       bgr dest
            inc ebx            // rgba buffer increment
			inc edx            // rgb buffer increment

            dec ecx            // Decrease Loop Counter (3 bytes per loop)
            jnz label          // If Not Zero Jump Back To Label

			// drop through after doing one line

			pop ecx            // restore line counter

			// Invert
			sub ebx, t
			sub ebx, t         // Next rgb source line up

			dec ecx            // next line
			jnz lines          // do all lines

    }
}


// Adapted from :
// http://tech-algorithm.com/articles/nearest-neighbor-image-scaling/
// http://www.cplusplus.com/forum/general/2615/#msg10482
//
void CVCamStream::rgb2bgrResample(unsigned char* source, unsigned char* dest, 
								  unsigned int sourceWidth, unsigned int sourceHeight, 
								  unsigned int destWidth, unsigned int destHeight, bool bInvert)
{
	unsigned char *srcBuffer = (unsigned char *)source; // void to unsigned char pointer
	unsigned char *dstBuffer = (unsigned char *)dest;

	float x_ratio = (float)sourceWidth/(float)destWidth ;
	float y_ratio = (float)sourceHeight/(float)destHeight ;
	float px, py ; 
	unsigned int i, j;
	unsigned int pixel, nearestMatch;
	for(i = 0; i<destHeight; i++) {
		for(j = 0; j<destWidth; j++) {
			px = floor((float)j*x_ratio);
			py = floor((float)i*y_ratio);
			if(bInvert)
				pixel = (destHeight-i-1)*destWidth*3 + j*3; // flip vertically
			else
				pixel = i*destWidth*3 + j*3;
			nearestMatch = (int)(py*sourceWidth*3 + px*3);
			dstBuffer[pixel + 0] = srcBuffer[nearestMatch + 2];
			dstBuffer[pixel + 1] = srcBuffer[nearestMatch + 1];
			dstBuffer[pixel + 2] = srcBuffer[nearestMatch + 0];
		}
	}
}



