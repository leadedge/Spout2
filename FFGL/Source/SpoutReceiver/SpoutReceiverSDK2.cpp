/*
	
	SpoutReceiverSDK2.dll

	LJ - leadedge@adam.com.au

	FFGL plugin for receiving DirectX texture from an equivalent
	sending application	either using wglDxInterop or memory share
	Note fix to FFGL.cpp to allow setting string parameters
	http://resolume.com/forum/viewtopic.php?f=14&t=10324
	----------------------------------------------------------------------
	26.06.14 - major change to use Spout SDK
	08-07-14 - Version 3.000
	14.07-14 - changed to fixed SpoutReceiver object

*/
#include "SpoutReceiverSDK2.h"
#include <FFGL.h>
#include <FFGLLib.h>

// To force memoryshare, enable the define in below
// #define MemoryShareMode

#ifndef MemoryShareMode
	#define FFPARAM_SharingName		(0)
	#define FFPARAM_Update			(1)
	#define FFPARAM_Select			(2)
	#define FFPARAM_Aspect			(3)
#else
	#define FFPARAM_Aspect			(0)
#endif


//
// Mouse hook used for "Update" button to prevent multiple calls while the mouse 
// button is pressed down, otherwise many calls may be made even for a short click.
static HHOOK g_hMouseHook = NULL;
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
static bool bClicked = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////
static CFFGLPluginInfo PluginInfo (
	SpoutReceiverSDK2::CreateInstance,				// Create method
	#ifndef MemoryShareMode
	"OF48",										// Plugin unique ID
	"SpoutReceiver2",							// Plugin name (receive texture from DX)
	1,											// API major version number
	001,										// API minor version number
	2,											// Plugin major version number
	001,										// Plugin minor version number
	FF_SOURCE,									// Plugin type
	"Spout SDK DX11 receiver",					// Plugin description
	#else
	"OF49",										// Plugin unique ID
	"SpoutReceiver2M",						// Plugin name (receive texture from DX)
	1,											// API major version number
	001,										// API minor version number
	2,											// Plugin major version number
	001,										// Plugin minor version number
	FF_SOURCE,									// Plugin type
	"Spout Memoryshare receiver",				// Plugin description
	#endif
	"- - - - - - Vers 3.000 - - - - - -"		// About
);

/////////////////////////////////
//  Constructor and destructor //
/////////////////////////////////
SpoutReceiverSDK2::SpoutReceiverSDK2()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_maxCoordsLocation(-1)
{

	/*
	// Debug console window so printf works
	AllocConsole();
	freopen("CONIN$",  "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("\nSpoutReceiverSDK2 Vers 3.000\n");
	*/

	// Input properties - this is a source and has no inputs
	SetMinInputs(0);
	SetMaxInputs(0);
	
	//
	// ======== initial values ========
	//

	// Memoryshare define
	// if set to true (memory share only) this over-rides bMemoryMode below
	// and it connects as memory share and there is no user option to select
	// default is false (automatic)
	#ifdef MemoryShareMode
	bMemoryMode			= true;
	#else
	bMemoryMode			= false;
	#endif
	
	g_Width				= 0;
	g_Height			= 0; // zero initial image size
	myTexture			= NULL; // only used for memoryshare mode

	bInitialized		= false;
	bMemoryMode			= false; // default mode is texture rather than memory
	bInitialized		= false; // not initialized yet by either means
	bAspect				= false; // preserve aspect ratio of received texture in draw
	UserSenderName[0]	= 0;

	//
	// Parameters
	//
	#ifndef MemoryShareMode
	SetParamInfo(FFPARAM_SharingName,	"Sender Name",		FF_TYPE_TEXT, "");
	SetParamInfo(FFPARAM_Update,		"Update",			FF_TYPE_EVENT, false );
	SetParamInfo(FFPARAM_Select,		"Select Sender",	FF_TYPE_EVENT, false );
	bMemoryMode = false;
	#else
	bMemoryMode = true;
	#endif
	SetParamInfo(FFPARAM_Aspect,		"Aspect",			FF_TYPE_BOOLEAN, false );

	// For memory mode, tell Spout to use memoryshare
	if(bMemoryMode) {
		receiver.SetMemoryShareMode();
		// Give it a user name for ProcessOpenGL
		strcpy_s(UserSenderName, 256, "MemoryShare"); 
	}
	
	// mouse hook - Resolume polls parameters all the time, so this is needed
	if(g_hMouseHook == NULL)
		g_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

}


SpoutReceiverSDK2::~SpoutReceiverSDK2()
{
	// ReleaseReceiver does nothing if there is no receiver
	receiver.ReleaseReceiver();

	if(myTexture != 0) glDeleteTextures(1, &myTexture);
	myTexture = 0;

	// Free mouse hook
	if(g_hMouseHook) {
		UnhookWindowsHookEx(g_hMouseHook);
		g_hMouseHook = NULL; // static - so this is important
	}
}


////////////////////////////////////////////////////////////
//						Methods                           //
////////////////////////////////////////////////////////////
DWORD SpoutReceiverSDK2::InitGL(const FFGLViewportStruct *vp)
{
	if(UserSenderName[0] == 0) {
		// For detection of the active sender leave UserSenderName NULL
		// If it is given some value it will keep trying to connect
		// until the user enters a name of a sender that exists
		//
		// This is the behaviour required, so give it the first part of the 
		// SpoutCam CLSID for an arbitrary name that will never be entered
		strcpy_s(UserSenderName, 256, "0x8e14549a");
	}

	return FF_SUCCESS;
}

DWORD SpoutReceiverSDK2::DeInitGL()
{
	return FF_SUCCESS;
}


DWORD SpoutReceiverSDK2::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	bool bRet;
	//
	// Initialize a receiver
	//
	// If UserSenderName is already set, CreateReceiver will attempt to connect to
	// that sender otherwise if UserSenderName is NULL the active sender will be used.
	// CreateReceiver will return true only if it finds a sender running.
	//
	if(!bInitialized) {
		if(receiver.CreateReceiver(UserSenderName, g_Width, g_Height)) {
			strcpy_s(SenderName, UserSenderName);
			// Did it initialized in Memory share mode ?
			if(receiver.GetMemoryShareMode()) {
				InitTexture(g_Width, g_Height); // initialize an RGB texture
				bMemoryMode = true;
			}
			bInitialized = true;
			return FF_SUCCESS;
		}
		return FF_SUCCESS;
	}

	//
	// Receive a shared texture
	//
	//	Success : Returns the sender name, width and height
	//	Failure : No sender detected
	//
	if(bMemoryMode)
		bRet = receiver.ReceiveTexture(SenderName, width, height, myTexture, GL_TEXTURE_2D);
	else
		bRet = receiver.ReceiveTexture(SenderName, width, height);

	if(bRet) {
		// Received the texture OK, but the sender or texture dimensions could have changed
		// Reset the global width and height so that the viewport can be set for aspect ratio control
		if(width != g_Width || height != g_Height) {
			g_Width  = width;
			g_Height = height;
			// Reset the local texture for memoryshare mode
			if(bMemoryMode)
				InitTexture(g_Width, g_Height);
			return FF_SUCCESS;
		} // endif sender has changed
	
		SaveOpenGLstate(); // Aspect ratio control

		if(bMemoryMode) 
			DrawTexture(myTexture);
		else
			receiver.DrawSharedTexture();

		RestoreOpenGLstate();

	}

	return FF_SUCCESS;

}

void SpoutReceiverSDK2::SaveOpenGLstate()
{
	float dim[4];
	float fx, fy, aspect, vpScaleX, vpScaleY, vpWidth, vpHeight;
	int vpx, vpy;

	// save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_TRANSFORM_BIT);

	// find the current viewport dimensions in order to scale to the aspect ratio required
	glGetFloatv(GL_VIEWPORT, dim);

	// Scale width and height to the current viewport size
	vpScaleX = dim[2]/(float)g_Width;
	vpScaleY = dim[3]/(float)g_Height;
	vpWidth  = (float)g_Width  * vpScaleX;
	vpHeight = (float)g_Height * vpScaleY;
	vpx = vpy = 0;

	// User option "Aspect" to preserve aspect ratio
	if(bAspect) {
		// back to original aspect ratio
		aspect = (float)g_Width/(float)g_Height;
		if(g_Width > g_Height) {
			fy = vpWidth/aspect;
			vpy = (int)(vpHeight-fy)/2;
			vpHeight = fy;
		}
		else {
			fx = vpHeight/aspect;
			vpx = (int)(vpWidth-fx)/2;
			vpWidth = fx;
		}
	}
	glViewport((int)vpx, (int)vpy, (int)vpWidth, (int)vpHeight);
				
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); // reset the current matrix back to its default state
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}


void SpoutReceiverSDK2::RestoreOpenGLstate()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
		
	glPopAttrib();
		
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopClientAttrib();			
	glPopAttrib();

}


DWORD SpoutReceiverSDK2::GetParameter(DWORD dwIndex)
{
	DWORD dwRet = FF_FAIL;

	#ifndef MemoryShareMode
	switch (dwIndex) {
		case FFPARAM_SharingName:
			if(!bMemoryMode) dwRet = (DWORD)UserSenderName;
			return dwRet;
		default:
			return FF_FAIL;
	}
	#endif

	return FF_FAIL;
}


DWORD SpoutReceiverSDK2::SetParameter(const SetParameterStruct* pParam)
{
	unsigned int width, height;
	HANDLE dxShareHandle;
	DWORD dwFormat;
	

	if (pParam != NULL) {

		switch (pParam->ParameterNumber) {

		// These parameters will not exist for memoryshare mode
		#ifndef MemoryShareMode

			case FFPARAM_SharingName:
			if(pParam->NewParameterValue && strlen((char*)pParam->NewParameterValue) > 0) {
				strcpy_s(UserSenderName, 256, (char*)pParam->NewParameterValue);
			}
			break;

		// Update user entered name with a click - same as above when a name is entered
		case FFPARAM_Update :
			if (pParam->NewParameterValue) { 
				// If there is anything already in this field at startup, it is set by a saved composition
				if(bClicked) { // check mouse hook
					if(bInitialized) {
						// Is there a  user entered name ?
						if(UserSenderName[0]) {
							// Is it different to the current sender name ?
							if(strcmp(SenderName, UserSenderName) != 0) {
								// Does the sender exist ?
								if(receiver.GetSenderInfo(UserSenderName, width, height, dxShareHandle, dwFormat)) {
									// Start again with the new user sender name
									receiver.ReleaseReceiver();
									bInitialized = false;
								}
								else {
									// warning - sender does not exist - desirable to have a popup ??
									// printf("warning - sender (%s) does not exist\n", UserSenderName);
									// receiver.SelectSenderPanel("Warning\nSender does not exist");
								}
							}
							else {
								// warning - sender does not exist
								// printf("warning - same name\n", UserSenderName);
								// receiver.SelectSenderPanel("warning - same name");
							}
						}
						else {
							// warning - no entry
							// printf("warning - no entry\n");
							// receiver.SelectSenderPanel("warning - no entry");
						}

					} // endif Initialized
					bClicked = false; // reset by button up
				} // end if clicked
			} // endif new parameter
			break;

		// SpoutPanel active sender selection
		case FFPARAM_Select :
			if (pParam->NewParameterValue) { 
				if(bClicked) { // check mouse hook
					receiver.SelectSenderPanel();
					bClicked = false; // reset by button up
				} // end clicked
			} // endif new parameter
			break;

		#endif

		case FFPARAM_Aspect:
			if(pParam->NewParameterValue > 0)
				bAspect = true;
			else 
				bAspect = false;
			break;


		default:
				break;

		}
		return FF_SUCCESS;
	}

	return FF_FAIL;

}


// Initialize a local RGB texture for memoryshare mode
void SpoutReceiverSDK2::InitTexture(int width, int height)
{
	if(myTexture != 0) glDeleteTextures(1, &myTexture);

	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

}


// Draw a memoryshare texture
void SpoutReceiverSDK2::DrawTexture(GLuint TextureHandle)
{
	glPushMatrix();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureHandle); // bind our local texture
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0,-1.0); // lower left
	glTexCoord2f(0.0, 0.0);	glVertex2f(-1.0, 1.0); // upper left
	glTexCoord2f(1.0, 0.0);	glVertex2f( 1.0, 1.0); // upper right
	glTexCoord2f(1.0, 1.0);	glVertex2f( 1.0,-1.0); // lower right
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}

//
// Callback-Function for mouse hook
//
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // < 0 doesn't concern us
	// If nCode is less than zero, the hook procedure must return the value returned by CallNextHookEx
    if(nCode < 0) return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);

	// If nCode is greater than or equal to zero, and the hook procedure did not process the message, 
	// it is highly recommended that you call CallNextHookEx and return the value it returns; 
	// otherwise, other applications that have installed WH_MOUSE_LL hooks will not receive
	// hook notifications and may behave incorrectly as a result. 
	// If the hook procedure processed the message, it may return a nonzero value
	// to prevent the system from passing the message to the rest of the hook chain
	// or the target window procedure. 
    if(nCode == HC_ACTION) {
		// Look for button down then button up
		if(wParam == WM_LBUTTONDOWN) {
				bClicked = true;
			}
		}
		else {
			if(wParam == WM_LBUTTONDOWN) {
				bClicked = false;
		}
    }    

    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}
