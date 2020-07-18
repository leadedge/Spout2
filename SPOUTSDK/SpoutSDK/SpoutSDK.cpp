// ================================================================
//
//		SpoutSDK
//
//		The Main Spout class - used by Sender and Receiver classes
//
//		Revisions :
//
//		14-07-14	- SelectSenderPanel - return true was missing.
//		16-07-14	- deleted fbo & texture in SpoutCleanup - test for OpenGL context
//					- used CopyMemory in FlipVertical instead of memcpy
//					- cleanup
//		18-07-14	- removed SpoutSDK local fbo and texture - used in the interop class now
//		22-07-14	- added option for DX9 or DX11
//		25-07-14	- Malcolm Bechard mods to header to enable compilation as a dll
//					- ReceiveTexture - release receiver if the sender no longer exists
//					- ReceiveImage same change - to be tested
//		27-07-14	- CreateReceiver - bUseActive flag instead of null name
//		31-07-14	- Corrected DrawTexture aspect argument
//		01-08-14	- TODO - work on OpenReceiver for memoryshare
//		03-08-14	- CheckSpoutPanel allow for unregistered sender
//		04-08-14	- revise CheckSpoutPanel
//		05-08-14	- default true for setverticalsync in sender and receiver classes
//		11-08-14	- fixed incorrect name arg in OpenReceiver for ReceiveTexture / ReceiveImage
//					2.004 release 19-08-14
//		24-08-14	- changed back to WM_PAINT message instead of RedrawWindow due to FFGL receiver bug appearing again
//		27-08-14	- removed texture init check from SelectSenderPanel
//		29-08-14	- changed SelectSenderPanel to use revised SpoutPanel with user message support
//		03.09.14	- cleanup
//		15.09.14	- protect against null string copy in SelectSenderPanel
//		22.09.14	- checking of bUseAspect function in CreateReceiver
//		23.09.14	- test for DirectX 11 support in SetDX9 and GetDX9
//		24.09.14	- updated project file for DLL to include SpoutShareMemory class
//		28.09.14	- Added GL format for SendImage and FlipVertical
//					- Added bAlignment  (4 byte alignment) flag for SendImage
//					- Added Host FBO for SendTexture, DrawToSharedTexture
//					- Added Host FBO for ReceiveTexture
//		11.10.14	- Corrected UpdateSender to recreate sender using CreateInterop
//					- Corrected SelectSenderpanel so that an un-initialized string is not used
//		12.10.14	- Included SpoutPanel always bring to topmost in SelectSenderPanel
//					- allowed for change of sender size in DrawToSharedTexture
//		15.10.14	- added debugging aid for texture access locks
//		29.10.14	- changes to SendImage
//		23.12.14	- added host fbo arg to ReceiveImage
//		30.01.15	- Read SpoutPanel path from registry (dependent on revised installer)
//					  Next path checked is module path, then current working directory
//		06.02.15	- added #pragma comment(lib,.. for "Shell32.lib" and "Advapi32.lib"
//		10.02.15	- added Optimus NvOptimusEnablement export to Spout.h - should apply to all apps including this SDK.
//		22.02.15	- added FindFileVersion for future use
//		24.05.15	- Registry read of sender name for CheckSpoutPanel (see SpoutPanel)
//		29.05.15	- Included SetAdapter for multiple adapters - Franz Hildgen.
//		01.06.15	- Read/Write DX9 mode from registry
//		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
//		04.07.15	- corrected "const char *" arg for GetSenderInfo
//		08.07.15	- Recompile for global DX9 flag
// 		01.08.15	- OpenReceiver - safety in case no opengl context
//		22.08.15	- Change to CheckSpoutPanel to wait for SpoutPanel mutex to open and then close
//		24.08.15	- Added GetHostPath to retrieve the path of the host that produced the sender
//		01.09.15	- added MessageBox error warnings in InitSender for better user diagnostics
//					  also added MessageBox warnings in SpoutGLDXinterop::CreateInterop
//		09.09.15	- included g_ShareHandle in CheckSpoutPanel
//					- removed bMemoryShareInitOK becasue there is no single initialization any more
//		12.09.15	- Incremented application sender name if one already exists with the same name
//					- Finalised revised SpoutMemoryShare class and functions
//		15.09.15	- Disable memoryshare if the 2.005 installer has not set the "MemoryShare" key
//					  to avoid problems with 2.004 apps.
//					- Change logic of OpenSpout so that fails for incompatible hardware
//					  if memoryshare is not set. Only 2.005 apps can set memoryshare.\
//		19.09.15	- Changed GetImageSize to look for NULL sharehandle of a sender to determine
//					  if it is memoryshare. Used by SpoutCam.
//		22.09.15	- Fixed memoryshare sender update in UpdateSender
//		25.09.15	- Changed SetMemoryShareMode for 2.005 - now will only set true for 2.005 and above
//		09.10.15	- DrawToSharedTexture - invert default false instead of true
//		10.10.15	- CreateSender - introduced a temporary DX shared texture for 2.005 memoryshare to prevent
//					  a crash with existing 2.004 apps
//		22.10.15	- Changed CheckSpoutPanel so that function variables are only created if SpoutPanel has been opened
//		26.10.15	- Added bIsSending and bIsReceiving for safety release of sender in destructor.
//		14.11.15	- changed functions to "const char *" where required
//		18.11.15	- added CheckReceiver so that DrawSharedTexture can be used by a receiver
//		24.11.15	- changes to CheckSpoutPanel to favour ActiveSender over the Registry sender name (used by VVVV)
//					- Reintroduced 250msec sleep after SpoutPanel activation
//		29.11.15	- fixed const char problem in ReadPathFromRegistry
//		18.01.16	- added CleanSenders before opening a new sender in case of orphaned sender names in the list
//		10.02.16	- added RemovePathFromRegistry
//		26.02.16	- recompile for Processing library 2.0.5.2 release
//		06.03.16	- added GetSpoutSenderName() and IsSpoutInitialized() for access to globals
//		17.03.16	- removed alignment argument from ReceiveImage
//					  Check for bgra extensions in receiveimage and sendimage
//					  Support only for rgba or bgra
//					  Changed to const unsigned char for Sendimage buffer
//		21.03.16	- Added glFormat and bInvert to SendImage
//					- Included LoadGLextensions in InitSender and InitReceiver for memoryshare mode.
//		24.03.16	- Added HostFBO argument to WriteMemory and ReadMemory function calls.
//		04.04.16	- Added HostFBO argument to SendImage - only used for texture share
//					  Merge from Smokhov https://github.com/leadedge/Spout2/pull/14
//					- Changed default invert flag for SendImage to true.
//		24.04.16	- Added IsPBOavailable to test for PBO support.
//		04.05.16	- SetPBOavailable(true/false) added to enable/disable pbo functions
//		07.05.16	- SetPBOavailable changed to SetBufferMode
//		18.06.16	- Add invert to ReceiveImage
//					2.005 release 23-06-16
//		29.06.16	- Added ReportMemory() for debugging
//					- Changed OpenSpout to fail for DX9 if no hwnd
//					  https://github.com/leadedge/Spout2/issues/18
//		03.07.16	- Fix dwFormat repeat declaration in InitSender
//		15.01.17	- Add GetShareMode, SetShareMode
//		18.01.17	- GetImageSize redundant for 2.006
//		22.01.17	- include zero char in SelectSenderPanel NULL arg checks
//		25.05.17	- corrected SendImage UpdateSender to use passed width and height
//			2.006 release 08-02-17
//
//		VS2015
//
//		02.06.17	- Registry functions moved to SpoutUtils
//		06.06.17	- Added GLDXavailable to OpenSpout
//		09.06.17	- removed g_TexID - not used
//		05.10.17	- https://github.com/leadedge/Spout2/issues/24
//					- OpenReceiver simplify code
//					- CheckSpoutPanel simplify code, remove text file sender retrieval
//					- Add InitReceiver override to include sharehandle and format args
//		10.03.18	- Noted that change to OpenReceiver for offscreen rendering
//					  not needed because hwnd can be null for spoutdx.CreateDX9device
//					  https://github.com/leadedge/Spout2/issues/18
//
//		VS2017
//
//		23.08.18	- Add SendFboTexture - see changes to WriteGLDXtexture in SpoutGLDXinterop.cpp
//		17.10.18	- Retrieve global render window handle in OpenSpout
//		01.11.18	- SendImage bInvert default false to align with SpoutSender.cpp		
//		01.11.18	- Changes to SelectSenderPanel to terminate SpoutPanel if it has crashed.
//		03.11.18	- Texture creation patch for compatibility with 2.004 removed for Spout 2.007
//		13.11.18	- Remove CPU mode
//		24.11.18	- Remove redundant GetImageSize
//		27.11.18	- Add RemovePadding for correction of image stride
//		28.11.18	- Add IsFrameNew and HasNewFrame
//		14.12.18	- Clean up for SpoutLibrary
//		15.12.18	- UpdateSender - release and re-create sender to avoid memory leak
//		17.12.18	- Change Spout dll Project properties to / MT
//		28.12.18	- Check mutex handle before close in CheckSpoutPanel
//		28.12.18	- Rebuild Spout.dll 32 / 64bit - Version 2.007
//		03.01.19	- Changed to revised registry functions in SpoutUtils
//		04.01.19	- Add OpenGL window creation functions for SpoutLibrary
//		05.01.19	- Change names for high level receiver functions for SpoutLibrary
//		16.01.19	- Fix ReceiveTextureData for sender name change
//		22.01.19	- Remove unsused bIsReceiving flag
//		05.03.19	- Add log notice for ReleaseSender
//		05.04.19	- Change GetSenderName to GetSender
//					  Reserve const char * GetSenderName for receiver class
//		17.06.19	- Fix missing log warning argument in UpdateSender
//		26.06.19	- Cleanup changes to UpdateSender
//		13.01.20	- Removed sleep time for SpoutPanel to open
//		19.01.20	- Change SendFboTexture to SendFbo
//		20.01.20	- Corrected SendFbo for width/height < shared texture
//		21.01.20	- Remove auto sender update in send functions
//					  Remove debug print from InitSender
//		25.05.20	- Correct filename case for all #includes throughout
//		14.07.20	- Removed unused bChangeRequested flag
//		18.07.20	- Rebuild binaries Win32 and x64 /MT VS2017
//
// ================================================================
/*
	Copyright (c) 2014-2020, Lynn Jarvis. All rights reserved.

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
#include "SpoutSDK.h"

Spout::Spout()
{
	// Class variables for sender and receiver
	m_SenderNameSetup[0]  = 0;
	m_SenderName[0]       = 0;
	m_glFormat            = GL_RGBA;
	m_bInvert             = true;
	m_bUpdate             = false;
	m_bUseActive          = true;
	m_Width               = 0;
	m_Height              = 0;

	// Globals
	g_Width               = 0;
	g_Height              = 0;
	g_ShareHandle         = 0;
	g_Format              = 0;
	g_PartnerID           = 0;
	g_hWnd                = NULL;   // handle to render window
	g_SharedMemoryName[0] = 0;      // No name to start 

	bDxInitOK             = false;  // DirectX initialization
	bMemory               = false;  // User or compatibility memoryshare mode

	bInitialized          = false;  // Has a sender or receiver been initialized
	bUseActive            = false;  // Use the active sender for CreateReceiver
	bIsSending             = false; // Not sending yet
	
	bSpoutPanelOpened     = false;  // Selection panel "spoutpanel.exe" opened
	bSpoutPanelActive     = false;  // The SpoutPanel window has been activated
	ZeroMemory(&m_ShExecInfo, sizeof(m_ShExecInfo));

}


//---------------------------------------------------------
Spout::~Spout()
{
	// Release the sender (does not matter if not registered)
	if(bIsSending && g_SharedMemoryName[0])
		interop.senders.ReleaseSenderName(g_SharedMemoryName);

	// Cleanup and close directx or memoryshare
	SpoutCleanUp();
	
}

//
// Public functions
//
bool Spout::CreateSender(const char* sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	// Make sure Spout has been initialized and OpenGL context is available
	// Global render window handle g_hWnd is also set in OpenSpout
	if(!OpenSpout())
		return false;

	SpoutLogNotice("Spout::CreateSender [%s] (%dx%d) - format %d", sendername, width, height, dwFormat);

	// Release any orphaned senders if the name exists
	// in the sender list but the shared memory info does not
	CleanSenders();

	// Initialize as a sender for either texture or memoryshare mode
	if (!InitSender(g_hWnd, sendername, width, height, dwFormat, bMemory)) {
		g_SharedMemoryName[0] = 0;
		bIsSending = false;
		return false;
	}

	bIsSending = true;

	return true;

} // end CreateSender


// ------------------------------------------
//	Update a sender
//	Used when a sender's texture changes size
//  The DirectX texture or memory map has to be re-created and the sender info updated
// ------------------------------------------
bool Spout::UpdateSender(const char *sendername, unsigned int width, unsigned int height)
{
	// Make sure it has initialized
	// Quit if no sender name
	// Quit if size zero
	if (!bInitialized || !sendername[0] || width == 0 || height == 0)
		return false;

	HANDLE hSharehandle = NULL;
	DWORD dwFormat = 0;
	unsigned int w = 0;
	unsigned int h = 0;

	// Is the sender still there? (use local vars to receive the details)
	if (interop.senders.GetSenderInfo(sendername, w, h, hSharehandle, dwFormat)) {
		// For a name change, close the sender and set up again
		if (strcmp(sendername, g_SharedMemoryName) != 0) {
			SpoutLogNotice("Spout::UpdateSender - sender change");
			SpoutLogNotice("    From [%s] (%dx%d)", g_SharedMemoryName, g_Width, g_Height);
			SpoutLogNotice("    To   [%s] (%dx%d)", sendername, width, height);
			ReleaseSender();
			CreateSender(sendername, width, height, dwFormat);
		}
		else if (width != g_Width || height != g_Height) {
			SpoutLogNotice("Spout::UpdateSender [%s] size change from (%dx%d) to (%dx%d)", sendername, g_Width, g_Height, width, height);
			if (!bMemory) {
				// For texture share mode, re-create the sender's directX shared texture
				// with the new dimensions and update the sender info
				// No need to re-initialize DirectX, only the GLDX interop
				// which is re-registered for the new texture
				interop.CreateInterop(g_hWnd, sendername, width, height, dwFormat, false); // false means a sender
				
				/*
				interop.senders.UpdateSender(sendername, width, height, hSharehandle, dwFormat);
				if (interop.GetDX9())
					interop.CreateDX9interop(width, height, dwFormat, false);
				else
					interop.CreateDX11interop(width, height, dwFormat, false);
				*/
			}
			else {
				SpoutLog("    Memoryshare - updating shared memory");
				// Memoryshare just has to update the sender information and the memory map size
				interop.senders.UpdateSender(sendername, width, height, NULL, 0);
				// Only the sender can update the memory map (see SpoutMemoryShare.cpp).
				interop.memoryshare.UpdateSenderMemorySize(sendername, width, height);
			}
		}

		// Update class variables
		g_Width = width;
		g_Height = height;
		strcpy_s(g_SharedMemoryName, 256, sendername);

		return true;
	}
	else {
		SpoutLogWarning("Spout::UpdateSender - could not find [%s]", sendername);
	}

	return false;

} // end UpdateSender

void Spout::ReleaseSender(DWORD dwMsec) 
{
	if (bIsSending && g_SharedMemoryName[0] > 0) {
		SpoutLogNotice("Spout::ReleaseSender [%s]", g_SharedMemoryName);
		interop.senders.ReleaseSenderName(g_SharedMemoryName); // if not registered it does not matter
	}

	SpoutCleanUp();
	bInitialized = false;
	bIsSending = false;
	
	Sleep(dwMsec); // Unused - keep for back-compatibility

}


//---------------------------------------------------------
// Send texture attached to the currently bound fbo
//
// If this function is used instead of the equivalent in the SpoutSender class,
// the input texture can be larger than the shared texture.
// Width and height are the used portion and only the used part is copied.
// Only available for GL/DX texture write, not memoryshare
// (see spoutGLDXinterop::WriteTexture)
//
// The fbo must be bound with a texture attached
// Completeness tested in WriteGLDXtexture
// gives GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
// if no images are attached to the framebuffer.
//
bool Spout::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	// For texture sharing, the fbo must be equal to or larger than the shared texture
	if (!bInitialized || bMemory || width == 0 || height == 0 || width < g_Width || height < g_Height)
		return false;

	return interop.WriteTexture(0, 0, width, height, bInvert, FboID);

} // end SendFbo


//---------------------------------------------------------
// If the local texture has changed dimensions this will return false
bool Spout::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	if (!bInitialized || width == 0 || height == 0 || width != g_Width || height != g_Height)
		return false;

	return interop.WriteTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);

} // end SendTexture

//---------------------------------------------------------
bool Spout::SendImage(const unsigned char* pixels,
					  unsigned int width, unsigned int height, 
					  GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	GLenum glformat = glFormat;

	if (!pixels || !bInitialized || width == 0 || height == 0 || width != g_Width || height != g_Height)
		return false;

	// Only RGBA, BGRA, RGB, BGR supported
	// GL_BGRA_EXT - 0x80E1
	if (!(glformat == GL_RGBA || glFormat == 0x80E1 || glformat == GL_RGB || glFormat == 0x80E0))
		return false;

	// Dimensions should be the same as the sender
	if (width != g_Width || height != g_Height)
		return false;

	// Check for BGRA support
	if (!interop.IsBGRAavailable()) {
		// If the bgra extensions are not available and the user
		// provided GL_BGR_EXT or GL_BGRA_EXT do not use them
		if (glFormat == 0x80E0) glformat = GL_RGB; // GL_BGR_EXT
		if (glFormat == 0x80E1) glformat = GL_RGBA; // GL_BGRA_EXT
	}

	// Write the pixel data to the rgba shared texture from the user pixel format
	return interop.WriteTexturePixels(pixels, width, height, glformat, bInvert, HostFBO);

} // end SendImage

//---------------------------------------------------------
void Spout::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	return interop.spoutcopy.RemovePadding(source, dest, width, height, stride, glFormat);
}

//---------------------------------------------------------
// Make sure Spout has been initialized
// Try to find the sender with the name passed
// or use the active sender if sender name set to zero
bool Spout::CreateReceiver(char* sendername, unsigned int &width, unsigned int &height, bool bActive)
{
	char UserName[256];
	UserName[0] = 0; // OK to do this internally

	// Use the active sender if the user has specified it
	// or is the sender name set to zero
	// bUseActive is a class flag
	if (bActive || sendername[0] == 0) {
		bUseActive = true;
	}
	else {
		// Try to find the sender with the name passed
		strcpy_s(UserName, 256, sendername);
		bUseActive = false;
	}

	// Make sure it has been initialized
	// OpenReceiver	checks g_ShareHandle for NULL which indicates memoryshare sender
	if (OpenReceiver(UserName, width, height)) {
		strcpy_s(sendername, 256, UserName); // pass back the sendername used
		SpoutLogNotice("Spout::CreateReceiver(%s) %dx%d, bActive = %d", UserName, width, height, bActive);
		return true;
	}

	// OpenReceiver failed - if already initialized, release the receiver
	if (bInitialized)
		ReleaseReceiver();

	return false;
}

//---------------------------------------------------------
void Spout::ReleaseReceiver()
{
	if (bInitialized) {
		// can be done without a check here
		SpoutCleanUp();
		bInitialized = false;
		Sleep(100); // Debugging aid, but leave for safety
	}
}
  
//---------------------------------------------------------
bool Spout::ReceiveTexture(char* name, unsigned int &width, unsigned int &height,
						   GLuint TextureID, GLuint TextureTarget,
						   bool bInvert, GLuint HostFBO)
{
	bool bConnected = true;

	// Test for sender change and user selection
	if (!CheckReceiver(name, width, height, bConnected)) {
		// Failure can mean changes to the connected sender (return bConnected = true)
		// Or that the sender is no longer there (return bConnected = false)
		return bConnected;
	}

	// Sender exists and everything matched.
	// Globals are now all current, so pass back the current name and size
	// so that there is no change found by the host.
	strcpy_s(name, 256, g_SharedMemoryName);
	width = g_Width;
	height = g_Height;

	// If a valid texture was passed, read the
	// shared texture or shared memory into it.
	if (TextureID > 0 && TextureTarget > 0) {
		return(interop.ReadTexture(g_SharedMemoryName, TextureID, TextureTarget, g_Width, g_Height, bInvert, HostFBO));
	}

	// Otherwise skip it. All the other checks for name and size are already done.
	// Just depend on the shared texture being updated and don't return one
	// e.g. can use BindSharedTexture to use the shared texture directly
	return true;

} // end ReceiveTexture

//---------------------------------------------------------
bool Spout::ReceiveImage(char* name, unsigned int &width, unsigned int &height, 
						 unsigned char* pixels, GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	bool bConnected = true;
	GLenum glformat = glFormat;

	// Only RGBA, BGRA, RGB and BGR supported
	if(!(glformat == GL_RGBA || glFormat == 0x80E1  || glFormat == GL_RGB || glFormat == 0x80E0))
		return false;

	// Check for BGRA support
	if(!interop.IsBGRAavailable()) {
		// If the bgra extensions are not available and the user
		// provided GL_BGR_EXT or GL_BGRA_EXT do not use them
		if(glFormat == 0x80E0) glformat = GL_RGB; // GL_BGR_EXT
		if(glFormat == 0x80E1) glformat = GL_RGBA; // GL_BGRA_EXT
	}

	// Test for sender change and user selection
	if(!CheckReceiver(name, width, height, bConnected))
		return bConnected;

	// globals are all current, so pass back the current name and size
	strcpy_s(name, 256, g_SharedMemoryName);
	width  = g_Width;
	height = g_Height;

	// If a pixel buffer was specified, read the shared texture into it
	// Copy functions handle the formats supported
	if (pixels) {
		return(interop.ReadTexturePixels(g_SharedMemoryName, pixels, width, height, glformat, bInvert, HostFBO));
	}

	return true;


}  // end ReceiveImage

//---------------------------------------------------------
bool Spout::IsSpoutInitialized()
{
	return bInitialized;
}

//---------------------------------------------------------
bool Spout::BindSharedTexture()
{
	return(interop.BindSharedTexture());
}

//---------------------------------------------------------
bool Spout::UnBindSharedTexture()
{
	return(interop.UnBindSharedTexture());
}


// ====================================================================//
//                   Legacy Draw and Drawto functions                  //
// ====================================================================//
#ifdef legacyOpenGL // defined in SpoutCommon.h
//---------------------------------------------------------
bool Spout::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	return(interop.DrawSharedTexture(max_x, max_y, aspect, bInvert, HostFBO));
}

//---------------------------------------------------------
bool Spout::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	// Allow for change of sender size, even though the draw is independent of the 
	// shared texture size, otherwise receivers will get a constant size for this sender
	if(!bMemory) {
		// width and height are the size of the texture that is being drawn to.
		// width, g_Width should all be the same
		if(width != g_Width || height != g_Height) {
			return(UpdateSender(g_SharedMemoryName, width, height));
		}
	}
	return(interop.DrawToSharedTexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO));

}
#endif

//---------------------------------------------------------
bool Spout::SetMemoryShareMode(bool bMem)
{
	return (interop.SetMemoryShareMode(bMem));
}

//---------------------------------------------------------
bool Spout::GetMemoryShareMode()
{
	// Gets interop class global memoryshare flag and sets a flag in this class
	bMemory = interop.GetMemoryShareMode(); // set global flag - TODO : rename globals
	return bMemory;
}

//---------------------------------------------------------
int Spout::GetShareMode()
{
	return interop.GetShareMode();
}

//---------------------------------------------------------
bool Spout::SetShareMode(int mode)
{
	return (interop.SetShareMode(mode));
}

//
// Maximum sender functions - for development testing only
//
int Spout::GetMaxSenders()
{
	// Gets the maximum senders allowed from the sendernames class
	return(interop.senders.GetMaxSenders());
}

void Spout::SetMaxSenders(int maxSenders)
{
	// Sets the maximum senders allowed
	interop.senders.SetMaxSenders(maxSenders);
}

// Get the global sender name for this instance
bool Spout::GetSpoutSenderName(char * sendername, int maxchars)
{
	if(g_SharedMemoryName && g_SharedMemoryName[0] > 0) {
		strcpy_s(sendername, maxchars, g_SharedMemoryName);
		return true;
	}
	else {
		return false;
	}
}

// Switch pbo functions on or off (default is off).
void Spout::SetBufferMode(bool bActive)
{
	interop.SetBufferMode(bActive);
}

bool Spout::GetBufferMode()
{
	return interop.GetBufferMode();
}

// SelectSenderPanel - used by a receiver
// Optional message argument
bool Spout::SelectSenderPanel(const char *message)
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char UserMessage[512];
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	if(message != NULL && message[0] != 0)
		strcpy_s(UserMessage, 512, message); // could be an arg or a user message
	else
		UserMessage[0] = 0; // make sure SpoutPanel does not see an un-initialized string

	// Pop up SpoutPanel to allow the user to select a sender
	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
		// Does SpoutPanel.exe exist in this path ?
		if (!PathFileExistsA(path)) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (!PathFileExistsA(path)) {
					SpoutLogWarning("Spout::SelectSenderPanel - SpoutPanel path not found");
					return false;
				}
			}
		}
	}

	// Check whether the panel is already running
	// Try to open the application mutex.
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
	if (!hMutex1) {
		//
		// No mutex, so not running, so can open it
		// 
		// Use ShellExecuteEx so we can test its return value later
		//
		ZeroMemory(&m_ShExecInfo, sizeof(m_ShExecInfo));
		m_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		m_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		m_ShExecInfo.hwnd = NULL;
		m_ShExecInfo.lpVerb = NULL;
		m_ShExecInfo.lpFile = (LPCSTR)path;
		m_ShExecInfo.lpParameters = UserMessage;
		m_ShExecInfo.lpDirectory = NULL;
		m_ShExecInfo.nShow = SW_SHOW;
		m_ShExecInfo.hInstApp = NULL;	
		ShellExecuteExA(&m_ShExecInfo);
		// Sleep(125); // allow time for SpoutPanel to open nominally 0.125s
		//
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		bSpoutPanelOpened = true;
	}
	else {
		// The mutex exists, so another instance is already running.
		// Find the SpoutPanel window and bring it to the top.
		// SpoutPanel is opened as topmost anyway but pop it to
		// the front in case anything else has stolen topmost.
		HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		if (hWnd && IsWindow(hWnd)) {
			SetForegroundWindow(hWnd);
			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		}
		else if (path[0]) {
			// If the window was not found but the mutex exists
			// and SpoutPanel is installed, it has crashed.
			// Terminate the process and the mutex or the mutex will remain
			// and SpoutPanel will not be started again.
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				SpoutLogError("Spout::SelectSenderPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("Spout::SelectSenderPanel - Process32First error");
					CloseHandle(hProcessSnap);
				}
				else {
					// Look through all processes
					while (hRes && !done) {
						int value = _tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
						if (value == 0) {
							HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
							if (hProcess != NULL) {
								// Terminate SpoutPanel and it's mutex
								TerminateProcess(hProcess, 9);
								CloseHandle(hProcess);
								done = true;
							}
						}
						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = NULL; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return true;

} // end SelectSenderPanel

// 22.02.15 - Used to find the SpoutPanel version
// http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
//
bool Spout::FindFileVersion(const char *FilePath, DWORD &versMS, DWORD &versLS)
{
    DWORD               dwSize              = 0;
    unsigned char       *pbVersionInfo      = NULL;
    VS_FIXEDFILEINFO    *pFileInfo          = NULL;
    UINT                puLenFileInfo       = 0;

    // get the version info for the file requested
    dwSize = GetFileVersionInfoSizeA(FilePath, NULL );
    if ( dwSize == 0 ) {
		SpoutLogError("Spout::FindFileVersion - Error in GetFileVersionInfoSize: %d", GetLastError() );
        return false;
    }

    pbVersionInfo = new BYTE[ dwSize ];

    if ( !GetFileVersionInfoA( FilePath, 0, dwSize, pbVersionInfo ) )  {
		SpoutLogError("Spout::FindFileVersion - Error in GetFileVersionInfo: %d", GetLastError() );
        delete[] pbVersionInfo;
        return false;
    }

    if ( !VerQueryValueA( pbVersionInfo, "\\", (LPVOID*) &pFileInfo, &puLenFileInfo ) ) {
		SpoutLogError("Spout::FindFileVersion - Error in VerQueryValue: %d", GetLastError() );
        delete[] pbVersionInfo;
        return false;
    }

	versMS = pFileInfo->dwFileVersionMS;
	versLS = pFileInfo->dwFileVersionLS;

    /*
	printf("File Version: %d.%d.%d.%d\n",
		( pFileInfo->dwFileVersionMS >> 16 ) & 0xffff,
        ( pFileInfo->dwFileVersionMS >>  0 ) & 0xffff,
        ( pFileInfo->dwFileVersionLS >> 16 ) & 0xffff,
        ( pFileInfo->dwFileVersionLS >>  0 ) & 0xffff
        );

    printf("Product Version: %d.%d.%d.%d\n",
        ( pFileInfo->dwProductVersionMS >> 24 ) & 0xffff,
        ( pFileInfo->dwProductVersionMS >> 16 ) & 0xffff,
        ( pFileInfo->dwProductVersionLS >>  8 ) & 0xffff,
        ( pFileInfo->dwProductVersionLS >>  0 ) & 0xffff
        );
	*/

	return true;

}

int Spout::GetSenderCount() {
	std::set<std::string> SenderNameSet;
	if(interop.senders.GetSenderNames(&SenderNameSet)) {
		return((int)SenderNameSet.size());
	}
	return 0;
}

//
// Get a sender name given an index into the sender names set
// index             - in
// sendername        - out
// sendernameMaxSize - in
//
bool Spout::GetSender(int index, char* sendername, int sendernameMaxSize)
{
	std::set<std::string> SenderNameSet;
	std::set<std::string>::iterator iter;
	std::string namestring;
	char name[256];
	int i;

	if(interop.senders.GetSenderNames(&SenderNameSet)) {
		if(SenderNameSet.size() < (unsigned int)index) {
			return false;
		}
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
		return true;
	}
	return false;
}

//---------------------------------------------------------
bool Spout::GetActiveSender(char* Sendername)
{
	return interop.senders.GetActiveSender(Sendername);
}

//---------------------------------------------------------
bool Spout::SetActiveSender(const char* Sendername)
{
	return interop.senders.SetActiveSender(Sendername);
}

//---------------------------------------------------------
bool Spout::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return interop.senders.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

//---------------------------------------------------------
int Spout::GetVerticalSync()
{
	return interop.GetVerticalSync();
}

//---------------------------------------------------------
bool Spout::SetVerticalSync(bool bSync)
{
	return interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
bool Spout::CreateOpenGL()
{
	return interop.CreateOpenGL();
}

//---------------------------------------------------------
bool Spout::CloseOpenGL()
{
	return interop.CloseOpenGL();
}


// ========================================================== //
//                      LOCAL FUNCTIONS                       //
// ========================================================== //

//
// OpenReceiver
//
//	05.10.17 - simplify code (also changes to CheckSpoutPanel and InitReceiver)
//  09.03.18 - note change to FindSender in SpoutSenderNames so that the sendernames
//  map is not accessed every frame by a receiver when looking for a named sender
//
// Make sure Spout has been initialized and OpenGL context is available
// A valid name is sent and the user does not want to use the active sender
// The name begins with a null character, or the bUseActive flag has been set
// Find if the sender exists or, if a null name given, return the active sender if that exists
// Return width, height, sharehandle and format.
// Initialize a receiver in either memoryshare or texture mode
// InitReceiver tests for a valid sharehandle if not memory mode
// InitReceiver resets the global name, width, height, sharehandle and format.
//
bool Spout::OpenReceiver (char* theName, unsigned int& theWidth, unsigned int& theHeight)
{
	char Sendername[256]; // user entered Sender name
	DWORD dwFormat = 0;
	HANDLE sharehandle = NULL;
	unsigned int width = 0;
	unsigned int height = 0;

	// Make sure Spout has been initialized and OpenGL context is available
	// g_hWnd is also set in OpenSpout
	if (!OpenSpout())
		return false;

	if(theName[0] != 0 && !bUseActive) {
		// A valid name is sent and the user does not want to use the active sender
		strcpy_s(Sendername, 256, theName);
	}
	else {
		// The name begins with a null character, or the bUseActive flag has been set
		Sendername[0] = 0;
	}

	// Set initial size to that passed in
	width  = theWidth;
	height = theHeight;

	// Find if the sender exists or, if a null name given, return the active sender if that exists
	// Return width, height, sharehandle and format.
	if (!interop.senders.FindSender(Sendername, width, height, sharehandle, dwFormat)) {
		// Given name not found ? - has SpoutPanel been opened ?
		if(!CheckSpoutPanel())
			return false;
		// Globals are reset if it has been, so update local variables for InitReceiver
		strcpy_s(Sendername, 256, g_SharedMemoryName);
		width = g_Width;
		height = g_Height;
		sharehandle = g_ShareHandle;
		dwFormat = g_Format;
	}

	// printf("OpenReceiver %dx%d - format %d\n", width, height, dwFormat);

	// Initialize a receiver in either memoryshare or texture mode
	// InitReceiver tests for a valid sharehandle if not memory mode
	if (InitReceiver(g_hWnd, Sendername, width, height, sharehandle, dwFormat)) {
		// InitReceiver resets the global name, width, height, sharehandle and format.
		// Pass the name, width and height back.
		strcpy_s(theName, 256, g_SharedMemoryName);
		theWidth  = g_Width;
		theHeight = g_Height;
		return true;
	}

	return false;

} // end OpenReceiver


void Spout::CleanSenders()
{
	char name[512];
	std::set<std::string> Senders;
	std::set<std::string>::iterator iter;
	std::string namestring;
	SharedTextureInfo info;

	// get the sender name list in shared memory into a local list
	interop.senders.GetSenderNames(&Senders);

	// Now we have a local set of names "Senders"
	// 27.12.13 - noted that if a Processing sketch is stopped by closing the window
	// all is OK and either the "stop" or "dispose" overrides work, but if STOP is used, 
	// or the sketch is closed, neither the exit or dispose functions are called and
	// the sketch does not release the sender.
	// So here we run through again and check whether the sender exists and if it does not
	// release the sender from the local sender list
	if(Senders.size() > 0) {
		for(iter = Senders.begin(); iter != Senders.end(); iter++) {
			namestring = *iter; // the Sender name string
			strcpy_s(name, namestring.c_str());
			// we have the name already, so look for it's info
			if(!interop.senders.getSharedInfo(name, &info)) {
				// Sender does not exist any more
				interop.senders.ReleaseSenderName(name); // release from the shared memory list
			}
		}
	}

	// Now we have cleaned up the list in shared memory
	Senders.clear();

}


bool Spout::InitSender(HWND hwnd, const char* theSendername,
	unsigned int theWidth, unsigned int theHeight,
	DWORD theFormat, bool bMemoryMode)
{
	char sendername[256];
	SpoutLogNotice("Spout::InitSender [%s] %dx%d, memorymode = %d", theSendername, theWidth, theHeight, bMemoryMode);

	// Quit if there is no image size to initialize with
	if (theWidth == 0 || theHeight == 0) {
		SpoutLogFatal("Spout::InitSender - Cannot initialize sender with zero size");
		return false;
	}

	// Does the sender already exist?
	// If it does, create an incremented name
	int i = 1;
	strcpy_s(sendername, 256, theSendername);
	if (interop.senders.FindSenderName(sendername)) {
		do {
			sprintf_s(sendername, 256, "%s_%d", theSendername, i);
			i++;
		} while (interop.senders.FindSenderName(sendername));
	}

	// Set the global sender name
	strcpy_s(g_SharedMemoryName, 256, sendername);

	// OpenSpout has already loaded OpenGL extensions and set up the sharing mode to be used

	// Only try DirectX if the memory mode flag has not been passed
	if (!bMemoryMode) {
		// Initialize the GL/DX interop and create a new shared texture (false = sender)
		if (!interop.CreateInterop(hwnd, sendername, theWidth, theHeight, theFormat, false)) {  // False for a sender
			SpoutLogFatal("Spout::InitSender - CreateInterop failed");
			return false;
		}
	} // endif texture sharing
	// DirectX is not initialized f
	else { // Memoryshare mode

		//
		// Texture creation patch removed for > Spout 2.006
		//

		// Memoryshare needs to create a sender separately
		// For a 2.004 receiver the result will just be black
		if (!interop.senders.CreateSender(sendername, theWidth, theHeight, interop.m_dxShareHandle)) {
			SpoutLogFatal("Spout::InitSender - Could not create sender");
			return false;
		}

		if (!interop.memoryshare.CreateSenderMemory(sendername, theWidth, theHeight)) {
			SpoutLogFatal("Spout::InitSender - Could not create sender shared memory");
			return false;
		}

	} // endif memoryshare mode

	// All passes - enable frame counting
	interop.frame.EnableFrameCount(sendername);

	// Get the sender width, height and share handle into local copy
	interop.senders.GetSenderInfo(g_SharedMemoryName, g_Width, g_Height, g_ShareHandle, g_Format);

	bInitialized = true;

	return true;

} // end InitSender

bool Spout::InitReceiver(HWND hwnd, char* sendername, unsigned int width, unsigned int height, HANDLE hSharehandle, DWORD dwFormat)
{
	// Quit if there is no image size to initialize with
	if(width == 0 || height == 0)
		return false;

	//
	// ============== Set up for a RECEIVER ============
	//

	bInitialized     = false;

	// OpenSpout has already loaded OpenGL extensions and set up the sharing mode to be used
	// only try dx if the memory mode flag is not set and sharehandle is not NULL
	if(!bMemory && hSharehandle) {
		// Initialize the receiver interop. 
		// This will create globals for texture sharing local to the interop class
		if(!interop.CreateInterop(hwnd, sendername, width, height, dwFormat, true)) // true meaning receiver
			return false;
	}
	// For memoryshare, open and close sender memory map in sendimage 
	// using the global sender name

	// Set globals here
	g_Width       = width;
	g_Height      = height;
	g_ShareHandle = hSharehandle;
	g_Format      = dwFormat;
	strcpy_s(g_SharedMemoryName, 256, sendername);

	// All passes - enable frame counting
	interop.frame.EnableFrameCount(sendername);

	bInitialized = true;

	return true;

} // end InitReceiver

//---------------------------------------------------------
//
// CheckReceiver
//
// If not yet inititalized, connect to the name provided or the active sender
//
// Check to see the whether the user has selected another sender
//
// Check that the sender is present
//
// Check current global values for change with those passed in
//      o any changes, re-initialize the receiver  
//
// Set bConnected to true if the sender is OK
// Set bConnected to false if the sender is closed
//
// Return false if the calling application needs to make any changes
//		o changed values are returned to the caller.
//
// Return true if there have been no changes to the sender
//
bool Spout::CheckReceiver(char* name, unsigned int &width, unsigned int &height, bool &bConnected)
{
	char newname[256];
	unsigned int senderwidth = 0;
	unsigned int senderheight = 0;
	DWORD dwFormat = 0;
	HANDLE hShareHandle = NULL;

	// Has it initialized yet ?
	if (!bInitialized) {
		// The name passed is the name to try to connect to unless the bUseActive flag is set
		// or the name begins with char 0 in which case it will try to find the active sender
		// Width and height are passed back as well
		if (name[0] != 0)
			strcpy_s(newname, 256, name);
		else
			newname[0] = 0;

		if (OpenReceiver(newname, senderwidth, senderheight)) {
			// OpenReceiver will also set the global name, width, height and format
			// Pass back the new name, width and height to the caller
			// The change has to be detected by the application
			strcpy_s(name, 256, newname);
			width = senderwidth;
			height = senderheight;
			bConnected = true; // user needs to check
			return false;
		}
		else {
			// Initialization failure - the sender is not there 
			// Quit to let the app try again
			bConnected = false;
			return false;
		}
	} // endif not initialized


	// Check to see if SpoutPanel has been opened
	// If it has been opened, the globals are reset
	// (g_SharedMemoryName, g_Width, g_Height, g_Format)
	// and the sender name will be different to that passed 
	CheckSpoutPanel();

	// Set initial values to current globals to check for change with those passed in
	strcpy_s(newname, 256, g_SharedMemoryName);
	senderwidth = g_Width;
	senderheight = g_Height;
	hShareHandle = g_ShareHandle;
	dwFormat = g_Format;

	// Is the sender there ?
	if (interop.senders.CheckSender(newname, senderwidth, senderheight, hShareHandle, dwFormat)) {
		// The sender exists, but has the width, height, texture format changed from those passed in
		if (senderwidth > 0 && senderheight > 0) {
			if (senderwidth != width
				|| senderheight != height
				|| dwFormat != g_Format
				|| strcmp(name, g_SharedMemoryName) != 0) { // test of original name allows for CheckSpoutPanel above
				SpoutLogNotice("Spout::CheckReceiver : sender change from (%s) %dx%d to (%s) %dx%d", name, width, height, g_SharedMemoryName, senderwidth, senderheight);
				// Re-initialize the receiver
				// OpenReceiver will also set the global name, width, height and format
				if (OpenReceiver(g_SharedMemoryName, senderwidth, senderheight)) {
					g_Width = senderwidth;
					g_Height = senderheight;
					g_ShareHandle = hShareHandle; // 09.09.15
					g_Format = dwFormat; // 09.09.15
					// Return the new sender name and dimensions
					// Changes have to be detected by the application
					width = g_Width;
					height = g_Height;
					bConnected = true; // user needs to check for changes
					strcpy_s(name, 256, g_SharedMemoryName);
					return false;
				} // OpenReceiver OK
				else {
					// need what here
					bConnected = false;
					return false;
				}
			} // width, height, format or name have changed
			// The sender exists and there are no changes
			// Drop through to return true
		} // width and height are zero
		else {
			// need what here
			bConnected = false;
			return false;
		}
	} // endif CheckSender found a sender
	else {
		g_SharedMemoryName[0] = 0; // sender no longer exists
		// 01.06.15 - safety
		ReleaseReceiver(); // Start again
		bConnected = false;
		return false;
	} // CheckSender did not find the sender - probably closed

	// The sender exists and there are no changes
	bConnected = true;

	return true;

}

//
// SpoutCleanup
//
void Spout::SpoutCleanUp()
{

	// OK for memoryshare because all handles will be NULL 
	interop.CleanupInterop();
	bDxInitOK = false;

	// 04.11.15 - Close memoryshare if created for data transfer
	// Has no effect if not created
	interop.memoryshare.CloseSenderMemory();
	if(bMemory) 
		interop.memoryshare.ReleaseSenderMemory(); // destroys sendermem object
	// bMemory - Registry or user setting - do not change it

	g_ShareHandle = NULL;
	g_Width	= 0;
	g_Height= 0;
	g_Format = 0;

	// important - we no longer want the global shared memory name and need to reset it
	g_SharedMemoryName[0] = 0; 

	// Set default for  
	bUseActive = false;

	// Everything must be reset (see ReceiveTexture)
	bInitialized = false;

}

//
// ========= USER SELECTION PANEL TEST =====
//
//	This is necessary because the exit code needs to be tested
//
// 05.10.17	- simplify code
//			  remove text file sender retrieval
//
bool Spout::CheckSpoutPanel()
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if(bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		unsigned int width = 0;
		unsigned int height = 0;
		DWORD dwFormat = 0;
		HANDLE hShareHandle = NULL;
		bool bRet = false;
		
		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if(!bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if(hMutex) bSpoutPanelActive = true;
		}
		else if (!hMutex) {
			
			// SpoutPanel has been activated and has now closed

			bSpoutPanelOpened = false; // Don't do this part again
			bSpoutPanelActive = false;
			
			// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
			// to get the exit code from SpoutPanel
			if(m_ShExecInfo.hProcess) {
				GetExitCodeProcess(m_ShExecInfo.hProcess, &dwExitCode);
				// Only act if exit code = 0 (OK)
				if(dwExitCode == 0) {
					// SpoutPanel has been activated and OK clicked
					// Test the active sender which should have been set by SpoutPanel
					newname[0] = 0;
					if (!interop.senders.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (interop.senders.getSharedInfo(newname, &TextureInfo)) {
									// Register in the list of senders and make it the active sender
									interop.senders.RegisterSenderName(newname);
									interop.senders.SetActiveSender(newname);
								}
							}
						}
					}

					// Now do we have a valid sender name ?
					if (newname[0] != 0) {
						// Retrieve the sender information
						if (interop.senders.CheckSender(newname, width, height, hShareHandle, dwFormat)) {
							// Texture or CPU mode - the Sender sharehandle must not be NULL
							if (!bMemory && !hShareHandle)
								return false;
							// Update all globals if the sender exists.
							strcpy_s(g_SharedMemoryName, 256, newname);
							g_Width = width;
							g_Height = height;
							g_ShareHandle = hShareHandle;
							g_Format = dwFormat;
							bRet = true; // will pass on next call to receivetexture

						} // endif valid sender name
					} // no sender
				} // endif SpoutPanel OK
			} // got the exit code
		} // endif no mutex so SpoutPanel has closed
		if (hMutex)	CloseHandle(hMutex);

		return bRet;
	} // SpoutPanel has not been opened

	return false;

} // ========= END USER SELECTION PANEL =====


//
// OpenSpout
//
// Compatibility test for use or GL/DX interop
// Sets sharing mode flags for this class, either Texture or memoryshare :
//    bDxInitOK - DirectX initialized OK - can use either GL/DX or CPU sharing modes
//    bMemory   - using memoryshare mode
//
// Failure is final.
//
bool Spout::OpenSpout()
{
	// Return silently if already initialized
	if (g_hWnd > 0)
		return true;

	printf("\n"); // This is the start, so make a new line in the log
#ifdef _M_X64
	SpoutLogNotice("Spout::OpenSpout - 64bit 2.007 - this 0x%x", this);
#else
	SpoutLogNotice("Spout::OpenSpout - 32bit 2.007 - this 0x%x", this);
#endif


	//
	// Compatibility test
	//
	//     o Load extensions and check for availability and function
	//     o For texture share mode, open DirectX and check for availability
	//     o Set flags for either Texture or memoryshare
	//
	// If the compatibility test fails, either LoadGLextensions failed
	// or OpenDirectX failed for texture sharing and nothing can continue
	//
	if (!interop.GLDXcompatible()) {
		SpoutLogWarning("Spout::OpenSpout - not texture share compatible");
		return false;
	}

	// Get the render window handle retrieved in GLDXcompatible
	g_hWnd = interop.GetRenderWindow();

	// Check what GLDXcompatible set to the registry
	if (interop.GetMemoryShareMode()) {
		SpoutLogNotice("Spout::OpenSpout - using memory share mode");
		// DirectX initializaton failed or user had selected memoryshare mode
		bDxInitOK = false; // DirectX initialization failed
		bMemory = true; // Default to memoryshare
	}
	else {
		SpoutLogNotice("Spout::OpenSpout - using texture share mode");
		bDxInitOK = true; // DirectX initialization was OK
		bMemory = false; // Memoryshare not needed
	}

	return true;

}

// Just return the flag that has been set
bool Spout::GetDX9()
{
	return interop.GetDX9();
}

// This is a request from within a program and Spout might not have initialized yet.
// If set OFF the DX9 setting is returned false only after a DX11 compatibility check
bool Spout::SetDX9(bool bDX9)
{
	return(interop.UseDX9(bDX9));
}

// Set the DirectX 9 format fo texture sharing
void Spout::SetDX9format(D3DFORMAT textureformat)
{
	interop.SetDX9format(textureformat);
}

// Set the DirectX 11 format fo texture sharing
void Spout::SetDX11format(DXGI_FORMAT textureformat)
{
	interop.SetDX11format(textureformat);
}

// Set graphics adapter for Spout output
bool Spout::SetAdapter(int index)
{
	bool bRet = interop.SetAdapter(index);
	return bRet;

}

// Get current adapter index
int Spout::GetAdapter()
{
	return interop.GetAdapter();
}

// Get the number of graphics adapters in the system
int Spout::GetNumAdapters()
{
	return interop.GetNumAdapters();
}

// Get an adapter name
bool Spout::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return interop.GetAdapterName(index, adaptername, maxchars);
}

// Get the path of the host that produced the sender
bool Spout::GetHostPath(const char *sendername, char *hostpath, int maxchars)
{
	return interop.GetHostPath(sendername, hostpath, maxchars);
}

int Spout::ReportMemory()
{
	int nTotalAvailMemoryInKB = 0;
	int nCurAvailMemoryInKB = 0;

	glGetIntegerv(0x9048, &nTotalAvailMemoryInKB);
	glGetIntegerv(0x9049, &nCurAvailMemoryInKB);
	// printf("Memory used : Total [%i], Available [%i]\n", nTotalAvailMemoryInKB, nCurAvailMemoryInKB);

	return nCurAvailMemoryInKB;

}
