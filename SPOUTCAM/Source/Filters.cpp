/*

	TDO - detect a memoryshare sender in findsender and getactivesender ?
	Return null name but width and height ?
	
				SpoutCam
	
	A Memory or texture share Share Virtual webcam

	Receives frames from a memory share or texture share Sender
	Simulates a webcam to other programs.

	The Sender must be started before the webcam is loaded
	by other software because this sets the frame size
	to the frames being sent by the Sender
	Currently there is no way to change the frame size
	after the filter has been loaded. If there is no Sender, 
	the default is 320x240 with a noise image

	Copyright 2013 Lynn Jarvis - leadedge@adam.com.au

	"Spout" in all and any of it's parts, is free software: 
	you can redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software Foundation, 
	either version 3 of the License, or (at your option) any later version.

	Credit for original capture source filter authored by Vivek :

	https://plus.google.com/100981910546057697518/posts

	Downloaded from :

	http://tmhare.mvps.org/downloads/vcam.zip

	SpoutCam is a modification of this code and changes are clearly
	documented in this file. Any problems cannot be attributed
	to authors of previous versions. 
	
	"Spout" distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	See also the GNU general public license text file provided.
	If not found, see <http://www.gnu.org/licenses/>.
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

*/


#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include <streams.h>
#include <stdio.h>
#include <conio.h>
#include <olectl.h>
#include <dvdmedia.h>
#include "filters.h"

#define RGB2BGR(a_ulColor) (a_ulColor & 0xFF000000) | ((a_ulColor & 0xFF0000) >> 16) | (a_ulColor & 0x00FF00) | ((a_ulColor & 0x0000FF) << 16)

//////////////////////////////////////////////////////////////////////////
//  CVCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI CVCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

	/*
	// debug console window
	AllocConsole();
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutCam - 24-08-14\n");
	*/

    CUnknown *punk = new CVCam(lpunk, phr);

    return punk;
}

CVCam::CVCam(LPUNKNOWN lpunk, HRESULT *phr) : 
    CSource(NAME("Spout Cam"), lpunk, CLSID_SpoutCam)
{
    ASSERT(phr);

    CAutoLock cAutoLock(&m_cStateLock);
    
	// Create the one and only output pin
    m_paStreams = (CSourceStream **)new CVCamStream*[1];
    m_paStreams[0] = new CVCamStream(phr, this, L"Spout Cam");

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

// LJ addition - maybe not necessary - needs checking - comments from Microsoft
// Live sources, such as capture devices, are an exception to this general architecture. 
// With a live source, it is not appropriate to cue any data in advance. 
// The application might pause the graph and then wait for a long time before running it. 
// The graph should not render "stale" samples. Therefore, a live source produces 
// no samples while paused, only while running. To signal this fact to the Filter Graph Manager, 
// the source filter's IMediaFilter::GetState method returns VFW_S_CANT_CUE. 
// This return code indicates that the filter has switched to the paused state, 
// even though the renderer did not receive any data.
// A capture filter should produce data only when the filter is running. Do not send data
// from your pins when the filter is paused. Also, return VFW_S_CANT_CUE from the
// CBaseFilter::GetState method when the filter is paused. This return code informs the
// Filter Graph Manager that it should not wait for any data from your filter while the
// filter is paused. For more information, see Filter States.
// The following code shows how to implement the GetState method:
// live source - override GetState to return VFW_S_CANT_CUE when pausing
// since we won't be sending any data when paused
// If a filter cannot deliver data for some reason, it returns VFW_S_CANT_CUE. 
// Live capture filters return this value while paused, because they do not deliver data in the paused state.
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

//////////////////////////////////////////////////////////////////////////
// CVCamStream is the one and only output pin of CVCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
CVCamStream::CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName) :
    CSourceStream(NAME("Spout Cam"), phr, pParent, pPinName), m_pParent(pParent)
{

	bMemoryMode		= false; // Default mode is texture, true means memoryshare
	bInitialized	= false; 
	bGLinitialized	= false;
	bConnected		= false;
	bDisconnected	= false; // has to connect before can disconnect or it will never connect
	glContext		= 0;
	ShareHandle		= NULL; // local copy of texture share handle
	g_Width			= 320;	// if there is no Sender, getmediatype will use defaults
	g_Height		= 240;
	senderWidth		= 0;
	senderHeight	= 0;
	g_fbo			= 0;
	g_fbo_texture	= 0;
	SharedMemoryName[0] = 0;

	//
	// On startup get the active Sender name if any.
	//
	// The purpose of an ActiveSender' is to set the user requested name from a dialog
	// which is independent of the program, so an external means of finding the selection is required
	// If there is no active Sender, find out if any are registered, and if so, use the first one in the list
	// Currently if no Sender exists, the camera will go static and has to be closed and opened
	// again to detect a Sender. Resizing is needed before any more can be done.
	//
	//
	if(receiver.GetImageSize(SharedMemoryName, senderWidth, senderHeight, bMemoryMode)) {
		// printf(" *** First size = %dx%d - ,bMemoryMode = %d\n", senderWidth, senderHeight, bMemoryMode);
		if(bMemoryMode) receiver.SetMemoryShareMode(true); // memory mode from now on
		// Set the global width and height
		g_Width  = senderWidth;
		g_Height = senderHeight;
	}

	// Set mediatype to shared width and height or if it did not connect set defaults
	GetMediaType(&m_mt);

	NumDroppedFrames = 0;
	NumFrames = 0;

}

CVCamStream::~CVCamStream()
{
	// If there is no gl context here, deletebuffers causes a crash
	// so we assume the fbo and texture are destroyed on exit
	// also specific call to dxconnector.cleanup crashes so we must assume it happens OK
	// (needs modification to connector cleanup as in jit.gl.spoutSender)
	HGLRC ctx = wglGetCurrentContext();
	if(ctx != NULL) {
		if(bInitialized) receiver.ReleaseReceiver();
		if(glContext != NULL) { // global context handle
			wglDeleteContext(glContext); // try to prevent initgl twice
		}
	}

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
	unsigned char *buffer;
	unsigned char *src;
	unsigned char *dst;
	unsigned char red, grn, blu;

	unsigned int i, imagesize, width, height;
	long l, lDataLen;
	HRESULT hr=S_OK;;
    BYTE *pData;
	HGLRC glCtx;
	float dim[4]; // for saving the viewport dimensions
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();

	// If graph is inactive stop cueing samples
	if(!m_pParent->IsActive()) {
		return S_FALSE;
	}

	// first get the timing right
	// create some working info
	REFERENCE_TIME rtNow, rtDelta, rtDelta2=0; // delta for dropped, delta 2 for sleep.
	REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;
	
	// Simple method - avoids "stuttering" in yawcam but VLC fails !
	/*
	rtNow = m_rtLastTime;
    m_rtLastTime += avgFrameTime;
    pms->SetTime(&rtNow, &m_rtLastTime);
    pms->SetSyncPoint(TRUE);
	*/

	// What Time is it REALLY ???
	m_pParent->GetSyncSource(&m_pClock);
	
	m_pClock->GetTime(&refSync1);
	
	if(m_pClock) m_pClock->Release();

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


	// don't do anything if disconnected because it will already have connected
	// previously and something has changed. It can only disconnect after it has connected.
	if(!bDisconnected) {
		
		// This also initialises OpenGL and Glew - bInitialized is set if all is OK
		if(!bInitialized) {
			if(OpenReceiver() == NOERROR) {
				bInitialized = true;
				bDisconnected = false;
				if(!bMemoryMode) {
					InitTexture(g_Width, g_Height); // the size of the camera window
				}
			}
			else {
				bInitialized = false;
				bDisconnected = true;
			}
			return NOERROR;
		}

		// check gl context again
		glCtx = wglGetCurrentContext();
		if(glCtx == NULL) {
			receiver.ReleaseReceiver();
			bInitialized = false;
			bDisconnected = true; // don't try again
			return NOERROR;
		}

		// Check that a texture Sender has not changed size
		// If connected, sizes should be OK, but check again
		unsigned int size = (unsigned int)pms->GetSize();
		imagesize = width*height*3; // also retrieved above
		if(size != imagesize) {
			receiver.ReleaseReceiver();
			bInitialized = false;
			bDisconnected = true; // don't try again
			return NOERROR;
		}
		//
		// everything matches so go ahead with the shared texture read
		if(!bDisconnected) {
			if(bMemoryMode) {
				//
				// Memoryshare instead of interop texture share
				//
				// A memoryshare sender cannot change from startup like a texture sender
				// so the global width and height are always the same as the camera width and height
				//
				// Read the bitmap from shared memory into a local buffer
				buffer = (unsigned char *)malloc(g_Width*g_Height*3);
				if(buffer) {
					// Format for Receiveimage in memoryshare mode is GL_RGB
					// because there is no texture to receive
					if(receiver.ReceiveImage(SharedMemoryName, senderWidth, senderHeight, buffer)) {
						// first check that the image size has not changed
						if(senderWidth == g_Width && senderHeight == g_Height) {
							// all is OK - flip the data for correct orientation and change pixel format
							// VertFlipBuf(buffer, (long)g_Width, (long)g_Height);
							FlipVertical(buffer, g_Width, g_Height);
							dst = (unsigned char *)pData;
							src = buffer;
							for(i=0; i<g_Width*g_Height; i++) {
								red = *src++;
								grn = *src++;
								blu = *src++;
								*dst++ = blu;
								*dst++ = grn;
								*dst++ = red;
							}
						}
						else {
							// Sender has changed the image size
							// no way presently to deal with it so deinit
							receiver.ReleaseReceiver();
							bMemoryMode = false; // it will go to a static image from now on
							bDisconnected = true; // and not try again
						} // end image size check
					} // received OK
					free((void *)buffer);
				} // endif buffer OK
				NumFrames++;
				return NOERROR;
			}
			else if(receiver.ReceiveTexture(SharedMemoryName, senderWidth, senderHeight)) {
				//
				// ======= RENDER THE SHARED TEXTURE INVERTED TO A LOCAL TEXTURE VIA FBO ==========
				//
				// The shared texture can be a different size to the local texture because this is 
				// rendering and not copying. The local texture is always the same size as the filter.
				//
				glMatrixMode(GL_TEXTURE);
				glPushMatrix();
				glLoadIdentity();
				glPushAttrib(GL_TRANSFORM_BIT);
				glGetFloatv(GL_VIEWPORT, dim);
				glViewport(0, 0, g_Width, g_Height); // size of the camera window
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity(); // reset the current matrix back to its default state
				glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();

				// Attach the local texture (desination) to the color buffer in our frame buffer  
				// and draw into it with the shared texture
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbo); 
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_fbo_texture, 0);
				receiver.DrawSharedTexture();
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 
					
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glPopAttrib();
				glViewport(dim[0], dim[1], dim[2], dim[3]);
				glMatrixMode(GL_TEXTURE);
				glPopMatrix();

				// now read the local texture into the sample's data buffer because the sizes match
				glBindTexture(GL_TEXTURE_2D, g_fbo_texture);
				glEnable(GL_TEXTURE_2D);
				glGetTexImage(GL_TEXTURE_2D, 0,  GL_BGR,  GL_UNSIGNED_BYTE, (void *)pData);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);

				NumFrames++;
				return NOERROR;

			} // endif received OK
			else {

				receiver.ReleaseReceiver();
				bInitialized = false;
				bDisconnected = true; // don't try again
			} // endif received texture OK
		} // endif texture mode
	} // endif not disconnected

	// drop through to default static image if it did not work
	pms->GetPointer(&pData);
	lDataLen = pms->GetSize();
	for(l = 0; l <lDataLen; ++l) 
		pData[l] = rand();

	NumFrames++;

	return NOERROR;

} // FillBuffer

HRESULT CVCamStream::OpenReceiver()
{
	HDC GLhdc;

	glContext = wglGetCurrentContext();
	// Once created it seems stable and retained
	if(glContext == NULL) {
		// You need to create a rendering context BEFORE calling glewInit()
		// First you need to create a valid OpenGL rendering context and call glewInit() 
		// to initialize the extension entry points. 
		int argc = 1;
		char *argv = (char*)"vCam";
		char **vptr = &argv;
		glutInit(&argc, vptr);
		// In this case there is not be a rendering context. There is if an external window is present
		// but we don't know what it is. So create a window here but it will not show.
		glutCreateWindow("vCamGL");
		GLhdc = wglGetCurrentDC();
		GLhwnd = WindowFromDC(GLhdc);
	} // end no glcontext 

	// This is a receiver so try to connect
	if(receiver.CreateReceiver(SharedMemoryName, senderWidth, senderHeight)) {
		return NO_ERROR;
	}

	return S_FALSE;

} // end OpenReceiver


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


// LJ version for one media type
//
// GetMediaType: This method tells the downstream pin what types we support.
//
// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when 
//		(a) our filter proposes a media type, 
//		(b) the other filter proposes a type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int,CMediaType*) AND CheckMediaType.
//
// In this case we support only one type, which we obtain from the bitmap.
//
// Can be called repeatedly
//
HRESULT CVCamStream::GetMediaType(CMediaType *pmt)
{
	unsigned int width, height;

	if(pmt == NULL) {
        return E_POINTER;
    }

    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

	// Allow for default as well as width and height of memory share image
	if(g_Width == 0 || g_Height == 0) {
		width  = 320;
		height = 240;
	}
	else {
		// as per sending app
		width	=  g_Width;
		height	=  g_Height;
	}

	pvi->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth				= (LONG)width;
	pvi->bmiHeader.biHeight				= (LONG)height;
	pvi->bmiHeader.biPlanes				= 1;
	pvi->bmiHeader.biBitCount			= 24;
	pvi->bmiHeader.biCompression		= 0;
	pvi->bmiHeader.biSizeImage			= 0;             // default 
	// pvi->bmiHeader.biXPelsPerMeter	= 0;             // default 
	// pvi->bmiHeader.biYPelsPerMeter	= 0;             // default 
	// pvi->bmiHeader.biClrUsed			= 0;
	pvi->bmiHeader.biClrImportant		= 0;
	pvi->bmiHeader.biSizeImage			= GetBitmapSize(&pvi->bmiHeader);

	// The desired average display time of the video frames, in 100-nanosecond units. 
	// 60fps = 166667
	// 30fps = 333333
	pvi->AvgTimePerFrame = 166667; // 60fps
	// pvi->AvgTimePerFrame = 200000; // 50fps
	// pvi->AvgTimePerFrame = 333333; // 30fps

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

}



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
	if(!pmt)return S_OK;//Default? red5

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

//
// LJ - Possible bug here with properties.
// If a host tries to open the property page it can crash.
//

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

bool CVCamStream::InitTexture(unsigned int width, unsigned int height)
{

	if(g_fbo != 0) glDeleteFramebuffersEXT(1, &g_fbo);
	if(g_fbo_texture != 0) glDeleteTextures(1, &g_fbo_texture);	

	glGenFramebuffersEXT(1, &g_fbo); // create a texture to attach to the fbo
	glGenTextures(1, &g_fbo_texture); // Generate one texture

	glBindTexture(GL_TEXTURE_2D, g_fbo_texture); // Bind the fbo_texture
	// Create a standard texture with the width and height of our window
	// this will be filled with data from the movie frame rgb openframeworks image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
	// Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

	return true;
}


// Adapted from FreeImage function
// Flip the image vertically along the horizontal axis.
// Should be faster with one less line copy. Also only one malloc
bool CVCamStream::FlipVertical(unsigned char *src, unsigned int width, unsigned int height) 
{
		BYTE *From, *Mid;

		// swap the buffer
		int pitch = width*3; // RGB

		// copy between aligned memories
		Mid = (BYTE*)malloc(pitch * sizeof(BYTE));
		if (!Mid) return false;

		From = src;
	
		unsigned int line_s = 0;
		unsigned int line_t = (height-1)*pitch;

		for(unsigned int y = 0; y<height/2; y++) {
			// 15-07-14 - changed from memcpy to CopyMemory - very slight speed advantage
			CopyMemory(Mid, From + line_s, pitch);	
			CopyMemory(From + line_s, From + line_t, pitch);
			CopyMemory(From + line_t, Mid, pitch);
			line_s += pitch;
			line_t -= pitch;
		}

		free((void *)Mid);

		return true;
}

// Flip an RGB image vertically
// http://www.codeproject.com/Questions/369873/How-can-i-flip-the-image-Vertically-using-cplusplu
//
bool CVCamStream::VertFlipBuf(unsigned char *inbuf, long widthBytes, long height)
{   
	unsigned char *tb1;
	unsigned char *tb2;
	long bufsize;
	long row_cnt;     
	long off1=0;
	long off2=0;
 
	if (inbuf==NULL)
		return false;
 
	bufsize=widthBytes*3;
 
	tb1 = (unsigned char *)malloc(bufsize);
	if (tb1==NULL) {
		return false;
	}
 
	tb2= (unsigned char *)malloc(bufsize);
	if (tb2==NULL) {
		free((void *)tb1);
		return false;
	}
	
	for (row_cnt=0;row_cnt<(height+1)/2;row_cnt++) 
	{
		off1=row_cnt*bufsize;
		off2=((height-1)-row_cnt)*bufsize;  
		
		//	09.11.13 - changed from memcpy to CopyMemory - very slight speed advantage
		CopyMemory(tb1, inbuf+off1, bufsize*sizeof(unsigned char));	
		CopyMemory(tb2, inbuf+off2, bufsize*sizeof(unsigned char));	
		CopyMemory(inbuf+off1, tb2, bufsize*sizeof(unsigned char));
		CopyMemory(inbuf+off2, tb1, bufsize*sizeof(unsigned char));
	}	
 
	free((void*)tb1);
	free((void*)tb2);
 
	return true;
}

