/*
	
	SpoutReceiver2.dll

	Lynn Jarvis - spout.zeal.co

	FFGL plugin for receiving DirectX texture from an equivalent
	sending application	either using wglDxInterop or memory share
	Note fix to FFGL.cpp to allow setting string parameters
	http://resolume.com/forum/viewtopic.php?f=14&t=10324
	----------------------------------------------------------------------
	26.06.14 - major change to use Spout SDK
	08-07-14 - Version 3.000
	14.07-14 - changed to fixed SpoutReceiver object
	16.07.14 - restored host fbo binding after readtexture otherwise texture draw does not work
			 - used a local texture for both textureshare and memoryshare
	25.07.14 - Version 3.001 - corrected ReceiveTexture in SpoutSDK.cpp for reset if the sender was closed
	01.08.14 - Version 3.002 - external sender registration
	13.08.14 - Version 3.003 - restore viewport
	18.08.14 - recompiled for testing and copied to GitHub
	20.08.14 - activated event locks
			 - included DX9 mode compile flag (default true for Version 1 compatibility)
			 - included DX9 arg for SelectSenderPanel
			 - Version 3.004
			 - recompiled for testing and copied to GitHub
	=======================================================================================================
	24.08.14 - recompiled with MB sendernames class revision
			 - disabled mouse hook for SpoutPanel
			 - Version 3.005
	26.08.14 - removed mouse hook
			 - detect existing sender name on restart after Magic full screen.
			   Version 3.006
	29.08.14 - detect host name and dll start
			 - user messages for revised SpoutPanel instead of MessageBox
			 - Version 3.007
	31.08.14 - changed from quad to vertex array for draw
	01.09.14 - leak test and some changes made to SpoutGLDXinterop
	02.09.14 - added more information in plugin Description and About
			 - Version 3.008
			 - Uploaded to GitHub
	15-09-14 - added RestoreOpenGLstate before return for sender size change
			 - Version 3.009
	16-09-14 - change from saving state matrices to just the viewport
			 - Version 3.010
	17-09-14 - change from viewport to vertex for aspect control
			 - Version 3.011
	19-09-14 - Recompiled for DirectX 11
			   A receiver should be compatible with all apps, but a sender will not
			 - Version 3.012
	21-09-14 - recompiled for DirectX 11 mutex texture access locks
			 - Introduced bUseActive flag instead of empty name
			 - Corrected inverted draw
			 - Version 3.013
	23-09-14 - corrected User entered name reset for a saved entry
			 - Version 3.014
	30-09-14 - Host fbo argument for ReceiveTexture
			 - Version 3.015
	12-10-14 - recompiled for release
			 - SpoutSDK.cpp - updated SelectSenderPanel to set SpoutPanel.exe topmost if it exists
			 - Version 3.016
	21.10.14 - Recompile for update V 2.001 beta
			 - Version 3.017
	12.11.14 - fixed bug for ReceiveTexture passing host fbo	
			 - change to default startup for button detection to button up
			 - Version 3.018
	23.11.14 - fixed bug for FFPARAM_ASPECT which could be turned on but not off
	24.11.14 - FFPARAM_SharingName - test for null string as well as null parameter address
			 - Version 3.019
	16.12.14 - included NvOptimusEnablement export
			 - Version 3.020
	31.01.15 - Changed ID to LJ48/49 instead of OF48/49
			   Included define for DirectX 9 compile
			   Recompiled for DirectX 11, DirectX9 and Memoryshare for 2015 release
			   Version 3.021
	07.02.15 - corrected unregistered sender logic in FFPARM_UPDATE
			   Version 3.022
	23.02.15 - Removed OptimusEnablement export because it does not work in a plugin
	25.04.15 - Changed from graphics auto detection to set DirectX mode to optional installer
	01.05.15 - Changed project Linker > Debugging > Generate debugging info to YES
			   Version 3.023
	01.05.15 - Recompiled with WM_PAINT in SpoutGLDXinterop OpenDirectX9 instead of UpdateWindow
			   Version 3.024
	26.05.15 - Registry write of entered sender name (see also change to SpoutPanel)
			   Version 3.025
	17.08.15 - Removed DX mode from description
			   Removed SetDX9 - now done by registry setting
			   Recompile for 2.004 release
			   Version 3.026
	28.08.15 - Recompiled with RedrawWindow in SpoutGLDXinterop OpenDirectX9
			   Version 3.027
	13.09.15 - Remove Memoryshare define for 2.005 memoryshare release
	15.09.15 - Recompile VS2010 /MT
			   Version 3.028
	11.10.15 - Note local texture is only needed for aspect ratio change
			   otherwise DrawSharedTexture could be used
	23.06.15 - Rebuild for Spout 2.005 release
			   Version 3.029
	10.01.17 - Rebuild for Spout 2.006
			   Rename class to SpoutReceiverSDK3
			   Change ID t0 LJ58
			   Change name to SpoutReceiverSDK3
	23.01.17 - Name changed back to SpoutReceiver2
			 - Rebuild for Spout 2.006 VS2012 /MD - Version 3.030
	08.01.17 - Rebuild with current SDK
	16.04.17 - Change ID back to LJ48 due to problems with Isadora
			   Update version number to 3.030 in PluginInfo
			   Rebuild VS2012 /MD


*/
#include "SpoutReceiverSDK3.h"
#include <FFGL.h>
#include <FFGLLib.h>


#define FFPARAM_SharingName		(0)
#define FFPARAM_Update			(1)
#define FFPARAM_Select			(2)
#define FFPARAM_Aspect			(3)
        
////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////
static CFFGLPluginInfo PluginInfo (
	SpoutReceiverSDK3::CreateInstance,				// Create method
	"LJ48",										// Plugin unique ID
	"SpoutReceiver2",							// Plugin name (receive texture from DX)
	1,											// API major version number
	5,											// API minor version number
	3,											// Plugin major version number
	30,										    // Plugin minor version number
	FF_SOURCE,									// Plugin type
	"Spout Receiver - Vers 3.030\nReceives textures from Spout Senders\n\nSender Name : enter a sender name\nUpdate : update the name entry\nSelect : select a sender using 'SpoutPanel'\nAspect : preserve aspect ratio of the received sender", // Plugin description
	"S P O U T - Version 2.006\nspout.zeal.co"		// About
);

/////////////////////////////////
//  Constructor and destructor //
/////////////////////////////////
SpoutReceiverSDK3::SpoutReceiverSDK3()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_maxCoordsLocation(-1)
{

	HMODULE module;
	char path[MAX_PATH];

	/*
	// Debug console window so printf works
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("SpoutReceiver3 Vers 3.030\n");
	*/

	// Input properties - this is a source and has no inputs
	SetMinInputs(0);
	SetMaxInputs(0);
	
	//
	// ======== initial values ========
	//

	bInitialized      = false;  // not initialized yet by either means
	bAspect           = false;  // preserve aspect ratio of received texture in draw
	bUseActive        = true;   // connect to the active sender
	bStarted          = false;  // Do not allow a starting cycle

	UserSenderName[0] = 0;      // User entered sender name
	g_Width	          = 640;
	g_Height          = 480;    // arbitrary initial image size
	myTexture         = 0;      // only used for memoryshare mode

	//
	// Parameters
	//
	SetParamInfo(FFPARAM_SharingName, "Sender Name",   FF_TYPE_TEXT, "");
	SetParamInfo(FFPARAM_Update,      "Update",        FF_TYPE_EVENT, false );
	SetParamInfo(FFPARAM_Select,      "Select",        FF_TYPE_EVENT, false );
	SetParamInfo(FFPARAM_Aspect,      "Aspect",        FF_TYPE_BOOLEAN, false );

	// Find the host executable name
	module = GetModuleHandle(NULL);
	GetModuleFileNameA(module, path, MAX_PATH);
	_splitpath_s(path, NULL, 0, NULL, 0, HostName, MAX_PATH, NULL, 0);
	
	// Isadora and Resolume act on button down
	// Isadora activates all parameters on plugin load, so allow one cycle for starting.
	// Magic reacts on button-up, so when the dll loads the parameters are not activated. 
	// Magic and default Windows apps act on button up so all is OK.
	if(strstr(HostName, "Avenue") == 0 || strstr(HostName, "Arena") == 0 || strstr(HostName, "Isadora") == 0) {
		bStarted = false;
	}
	else {
		bStarted = true;
	}

	// Get share mode for debugging
	// ShareMode = receiver.spout.interop.GetShareMode();
	// printf("ShareMode = %d\n", ShareMode);

	

}


SpoutReceiverSDK3::~SpoutReceiverSDK3()
{
	// OpenGL context required
	if(wglGetCurrentContext()) {
		// ReleaseReceiver does nothing if there is no receiver
		receiver.ReleaseReceiver();
		if(myTexture != 0) glDeleteTextures(1, &myTexture);
		myTexture = 0;
	}

}


////////////////////////////////////////////////////////////
//						Methods                           //
////////////////////////////////////////////////////////////
DWORD SpoutReceiverSDK3::InitGL(const FFGLViewportStruct *vp)
{
	// Viewport dimensions might not be supplied by the host here
	return FF_SUCCESS;
}


DWORD SpoutReceiverSDK3::DeInitGL()
{
	// OpenGL context required
	if(wglGetCurrentContext()) {
		// ReleaseReceiver does nothing if there is no receiver
		receiver.ReleaseReceiver();
		if(myTexture != 0) glDeleteTextures(1, &myTexture);
		myTexture = 0;
	}
	return FF_SUCCESS;
}


DWORD SpoutReceiverSDK3::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	bStarted = true;
	
	//
	// Initialize a receiver
	//

	// If already initialized and the user has entered a different name, reset the receiver
	if(bInitialized && UserSenderName[0] && strcmp(UserSenderName, SenderName) != 0) {
		receiver.ReleaseReceiver();
		bInitialized = false;
	}

	if(!bInitialized) {

		// If UserSenderName is set, use it. Otherwise find the active sender
		if(UserSenderName[0]) {
			strcpy_s(SenderName, UserSenderName); // Create a receiver with this name
			bUseActive = false;
		}
		else {
			bUseActive = true;
		}

		// CreateReceiver will return true only if it finds a sender running.
		// If a sender name is specified and does not exist it will return false.
		// This also sets the global width and height
		if(receiver.CreateReceiver(SenderName, g_Width, g_Height, bUseActive)) {
			InitTexture(); // Initialize a texture
			bInitialized = true;
		}
		return FF_SUCCESS;
	}
	else {
		//
		// Receive the shared texture to local copy
		//
		//	Success : Returns the sender name, width and height and a local copy of the shared texture
		//	Failure : No sender detected
		//
		// Important - pass the host FBO to restore the binding
		if(receiver.ReceiveTexture(SenderName, width, height, myTexture, GL_TEXTURE_2D, false, pGL->HostFBO)) {
			// Received the texture OK, but the sender or texture dimensions could have changed
			// Reset the global width and height so that the viewport can be set for aspect ratio control
			if(width != g_Width || height != g_Height) {
				g_Width  = width;
				g_Height = height;
				InitTexture(); // Reset the local texture
				return FF_SUCCESS;
			} // endif sender has changed

			// All matches so draw the shared texture
			DrawReceivedTexture(myTexture, GL_TEXTURE_2D,  g_Width, g_Height);
		}
	}

	return FF_SUCCESS;

}


DWORD SpoutReceiverSDK3::GetParameter(DWORD dwIndex)
{
	DWORD dwRet = FF_FAIL;

	switch (dwIndex) {
		case FFPARAM_SharingName:
			dwRet = (DWORD)UserSenderName;
			return dwRet;
		default:
			return FF_FAIL;
	}

	return FF_FAIL;
}


DWORD SpoutReceiverSDK3::SetParameter(const SetParameterStruct* pParam)
{
	unsigned int width, height;
	HANDLE dxShareHandle;
	DWORD dwFormat;
	char name[256];

	if (pParam != NULL) {

		switch (pParam->ParameterNumber) {

		case FFPARAM_SharingName:

			if(pParam->NewParameterValue && (char*)pParam->NewParameterValue && strlen((char*)pParam->NewParameterValue) > 0) {

				// If there is anything already in this field at startup, it is set by a saved composition
				strcpy_s(name, 256, (char*)pParam->NewParameterValue);

				// If it is a different name, copy to the username
				if(strcmp(name, UserSenderName) != 0) {
					strcpy_s(UserSenderName, 256, (char*)pParam->NewParameterValue);

					// Does the sender exist ?
					if(receiver.GetSenderInfo(UserSenderName, width, height, dxShareHandle, dwFormat)) {
						// Is it an external unregistered sender - e.g. VVVV ?
						if(!receiver.spout.interop.senders.FindSenderName(UserSenderName) ) {
							// register it
							receiver.spout.interop.senders.RegisterSenderName(UserSenderName);
						}
						// The user has typed it in, so make it the active sender
						receiver.spout.interop.senders.SetActiveSender(UserSenderName);

						// Write the sender name to the registry
						WritePathToRegistry(UserSenderName, "Software\\Leading Edge\\SpoutPanel", "Sendername");

						// Start again
						if(bInitialized) receiver.ReleaseReceiver();
						bInitialized = false;
					}
				}
			}
			else {
				// Reset to an empty string so that the active sender 
				// is used and SelectSenderPanel works
				UserSenderName[0] = 0;
			}
			break;

		case FFPARAM_Update :

			// Update the user entered name
			if(pParam->NewParameterValue) { // name entry toggle is on
				// Is there a  user entered name
				if(UserSenderName[0] != 0) {
					// Does the sender exist ?
					if(receiver.GetSenderInfo(UserSenderName, width, height, dxShareHandle, dwFormat)) {
						// Is it an external unregistered sender - e.g. VVVV ?
						if(!receiver.spout.interop.senders.FindSenderName(UserSenderName) ) {
							// register it
							receiver.spout.interop.senders.RegisterSenderName(UserSenderName);
						}
						// The user has typed it in, so make it the active sender
						receiver.spout.interop.senders.SetActiveSender(UserSenderName);

						// Write the sender name to the registry
						WritePathToRegistry(UserSenderName, "Software\\Leading Edge\\SpoutPanel", "Sendername");

						// Start again
						if(bInitialized) receiver.ReleaseReceiver();
						bInitialized = false;
					}
				} // endif user name entered
			} // endif Update
			break;

		// SpoutPanel sender selection
		case FFPARAM_Select :
			// bStarted is set as soon as ProcessOpenGL is called
			if (pParam->NewParameterValue && bStarted) {
				if(UserSenderName[0]) {
					receiver.SelectSenderPanel("Using 'Sender Name' entry\nClear the name entry first");
				}
				else {
					receiver.SelectSenderPanel();
				}
			}
			break;

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

// Initialize a local texture
void SpoutReceiverSDK3::InitTexture()
{
	if(myTexture != 0) {
		glDeleteTextures(1, &myTexture);
		myTexture = 0;
	}

	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA, g_Width, g_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void SpoutReceiverSDK3::DrawReceivedTexture(GLuint TextureID, GLuint TextureTarget,  unsigned int width, unsigned int height)
{
	
	float image_aspect, vp_aspect;
	int vpdim[4];
	
	// Default offsets
	float vx = 1.0;
	float vy = 1.0;

	// find the current viewport dimensions to scale to the aspect ratio required
	glGetIntegerv(GL_VIEWPORT, vpdim);

	// Calculate aspect ratios
	vp_aspect = (float)vpdim[2]/(float)vpdim[3];
	image_aspect = (float)width/(float)height;

	// Preserve image aspect ratio for draw
	if(bAspect) {
		if(image_aspect > vp_aspect) {
			// Calculate the offset in Y
			vy = 1.0f/image_aspect;
			// Adjust to the viewport aspect ratio
			vy = vy*vp_aspect;
			vx = 1.0;
		}
		else { // Otherwise adjust the horizontal offset
			// Calculate the offset in X
			vx = image_aspect;
			// Adjust to the viewport aspect ratio
			vx = vx/vp_aspect;
			vy = 1.0;
		}
	}

	// Invert the texture coords from DirectX to OpenGL
	GLfloat tc[] = {
			 0.0, 1.0,
			 0.0, 0.0,
			 1.0, 0.0,
			 1.0, 1.0 };

	GLfloat verts[] =  {
			-vx,  -vy,   // bottom left
			-vx,   vy,   // top left
			 vx,   vy,   // top right
			 vx,  -vy }; // bottom right


	glPushMatrix();

	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	glEnable(TextureTarget);
	glBindTexture(TextureTarget, TextureID);

	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer(2, GL_FLOAT, 0, tc );
	glEnableClientState(GL_VERTEX_ARRAY);		
	glVertexPointer(2, GL_FLOAT, 0, verts );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(TextureTarget, 0);
	glDisable(TextureTarget);
	glPopMatrix();


}


// Note registry flush not needed (see SpoutPanel) because this is the receiver
bool SpoutReceiverSDK3::WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename)
{
	HKEY  hRegKey;
	LONG  regres;
	char  mySubKey[512];

	// The required key
	strcpy_s(mySubKey, 512, subkey);

	// Does the key already exist ?
	regres = RegOpenKeyExA(HKEY_CURRENT_USER, mySubKey, NULL, KEY_ALL_ACCESS, &hRegKey);
	if(regres != ERROR_SUCCESS) { 
		// Create a new key
		regres = RegCreateKeyExA(HKEY_CURRENT_USER, mySubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, &hRegKey, NULL);
	}

	if(regres == ERROR_SUCCESS && hRegKey != NULL) {
		// Write the path
		regres = RegSetValueExA(hRegKey, valuename, 0, REG_SZ, (BYTE*)filepath, ((DWORD)strlen(filepath) + 1)*sizeof(unsigned char));
		RegCloseKey(hRegKey);
    }

	if(regres == ERROR_SUCCESS)
		return true;
	else
		return false;

}
