//
//		SpoutCam - filters.h
//
//	Updated 23.08.13
//	Updated 24.12.13
//	Updated 03.01.14 - cleanup
//	Updated 10.04.14
//

#pragma once
#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

#define GLEW_STATIC // to use glew32s.lib instead of glew32.lib otherwise there is a redefinition error

#include "../../../SpoutSDK/Spout.h"
#include "../../../SpoutSDK/SpoutMemoryShare.h" //for initial memoryshare detection
#include <glut.h>



EXTERN_C const GUID CLSID_SpoutCam;


class CVCamStream;
class CVCam : public CSource
{
public:
    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

	// LJ additon
	STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    IFilterGraph *GetGraph() {return m_pGraph;}

private:
    CVCam(LPUNKNOWN lpunk, HRESULT *phr);
};

class CVCamStream : public CSourceStream, public IAMStreamConfig, public IKsPropertySet
{

public:

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
    STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

    //////////////////////////////////////////////////////////////////////////
    //  IQualityControl
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

	/////////////////// RED5 /////////////////////////////////////////////////
    //  IAMDroppedFrames
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE GetAverageFrameSize( long* plAverageSize);
	HRESULT STDMETHODCALLTYPE GetDroppedInfo(long  lSize,long* plArray,long* plNumCopied);
	HRESULT STDMETHODCALLTYPE GetNumDropped(long *plDropped);
	HRESULT STDMETHODCALLTYPE GetNumNotDropped(long *plNotDropped);

    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
    
    //////////////////////////////////////////////////////////////////////////
    //  CSourceStream
    //////////////////////////////////////////////////////////////////////////
    CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName);
    ~CVCamStream();

    HRESULT FillBuffer(IMediaSample *pms);
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
	HRESULT GetMediaType(CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT OnThreadCreate(void);

	// ============== IPC functions ==============
	//
	//	opengl/directx interop texture sharing
	//
	SpoutReceiver receiver; // Spoutcam is a receiver

	// spoutMemoryShare * MemoryShare; // for initial memoryshare detection
	spoutMemoryShare MemoryShare; // for initial memoryshare detection

	HGLRC glContext;
	HWND GLhwnd; // OpenGL window handle
	char SharedMemoryName[256];
	HANDLE ShareHandle;		// local copy of texture share handle
	bool bMemoryMode;		// true = memory, false = texture
	bool bDebug;
	bool bInitialized;
	bool bGLinitialized;
	bool bConnected;		// Sender has started up and is sending images
	bool bDisconnected;		// Sender had started but it has stopped or changed image size

	unsigned int g_Width;	// The global filter image width
	unsigned int g_Height;	// The glonbal filter image height
	unsigned int senderWidth;	// The global filter image width
	unsigned int senderHeight;	// The glonbal filter image height

	GLuint g_fbo;			// The frame buffer object
	GLuint g_fbo_texture;	// The opengl texture object
	
	bool InitTexture(unsigned int width, unsigned int height);
	HRESULT OpenReceiver();

	bool FlipVertical(unsigned char *src, unsigned int width, unsigned int height) ;
	bool VertFlipBuf(unsigned char *inbuf, long widthBytes, long height);
    
    
private:

	CVCam *m_pParent;
	long  NumDroppedFrames,NumFrames;
	REFERENCE_TIME 
		m_rtLastTime,	// running timestamp
		refSync1,		// Graphmanager clock time, to compute dropped frames.
		refSync2,		// Clock time for Sleeping each frame if not dropping.
		refStart,		// Real time at start from Graphmanager clock time.
		rtStreamOff;	// IAMPushSource Get/Set data member.

	DWORD dwLastTime;
    CCritSec m_cSharedState;
    IReferenceClock *m_pClock;

};


