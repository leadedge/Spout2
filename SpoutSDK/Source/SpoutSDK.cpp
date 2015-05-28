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
//
// ================================================================
/*
		Copyright (c) 2014, Lynn Jarvis. All rights reserved.

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

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("Spout::Spout()\n");
	*/


	g_Width				= 0;
	g_Height			= 0;
	g_ShareHandle		= 0;
	g_Format			= 0;
	g_TexID				= 0;
	g_hWnd				= NULL;		// handle to render window
	g_SharedMemoryName[0] = 0;		// No name to start 
	bDxInitOK			= false;
	bMemoryShareInitOK	= false;
	bMemory				= false;	// user memoryshare override
	bInitialized		= false;
	bChangeRequested	= true;		// set for initial
	bUseActive			= false;	// Use the active sender for CreateReceiver
	bSpoutPanelOpened	= false;	// Selection panel "spoutpanel.exe" opened

	// LJ DEBUG
	// FPS calcs
	timeNow = timeThen = elapsedTime = frameTime = lastFrameTime = PCFreq = waitMillis = 0.0;
	fps = 60.0;
	frameRate = 60.0;
	CounterStart = 0;
	millisForFrame = 16.66667; // 60fps default

}


//---------------------------------------------------------
Spout::~Spout()
{
	// This is the end, so cleanup and close directx or memoryshare
	SpoutCleanUp(true);
}

// Public functions
bool Spout::CreateSender(char* sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	bool bMemoryMode = true;

	// Make sure it has initialized
	// A render window must be visible for initSharing to work
	if(!OpenSpout()) {
		printf("Spout::CreateSender error 1\n");
		return false;
	}

	if(bDxInitOK) {
		bMemoryMode = false;
		strcpy_s(g_SharedMemoryName, 256, sendername);
	}
	
	// Initialize as a sender in either memory or texture mode
	return(InitSender(g_hWnd, sendername, width, height, dwFormat, bMemoryMode));

} // end CreateSender


// ------------------------------------------
//	Update a sender
//	Used when a sender's texture changes size
//  The DirectX texture has to be re-created and the sender info updated
// ------------------------------------------
bool Spout::UpdateSender(char *sendername, unsigned int width, unsigned int height)
{
	HANDLE hSharehandle;
	DWORD dwFormat;
	unsigned int w, h;

	// Make sure it has initialized
	if(!bInitialized || !bDxInitOK) return false;
	if(strcmp(g_SharedMemoryName, sendername) != 0) return false;

	// Retrieve the shared texture sharehandle and format (not passed)
	if(interop.senders.GetSenderInfo(sendername, w, h, hSharehandle, dwFormat)) {
	
		// Re-create the sender directX shared texture
		// with the new dimensions and update the sender info
		// No need to re-initialize DirectX, only the GLDX interop
		// which is re-registered for the new texture
		interop.CreateInterop(g_hWnd, sendername, width, height, dwFormat, false); // false means a sender

		//
		// Get the new sender width, height and share handle into local globals
		//
		interop.senders.GetSenderInfo(g_SharedMemoryName, g_Width, g_Height, g_ShareHandle, g_Format);

		return true;
	}

	return false;
		
} // end UpdateSender



void Spout::ReleaseSender(DWORD dwMsec) 
{
	if(bMemoryShareInitOK) {
		return;
	}

	if(g_SharedMemoryName[0] > 0) {
		interop.senders.ReleaseSenderName(g_SharedMemoryName); // if not registered it does not matter
	}
	SpoutCleanUp();
	bInitialized = false; // DEBUG - needs tracing
	
	Sleep(dwMsec); // DEBUG - not needed - debugging aid only

}

// 27.07-14 - change logic to allow an optional user flag to use the active sender
bool Spout::CreateReceiver(char* sendername, unsigned int &width, unsigned int &height, bool bActive)
{

	char UserName[256];
	UserName[0] = 0; // OK to do this internally

	// Use the active sender if the user wants it or the sender name is not set
	if(bActive || sendername[0] == 0) {
		bUseActive = true;
	}
	else {
		// Try to find the sender with the name sent or over-ride with user flag
		strcpy_s(UserName, 256, sendername);
		bUseActive = false; // set global flag to use the active sender or not
	}

	if(OpenReceiver(UserName, width, height)) {
		strcpy_s(sendername, 256, UserName); // pass back the sendername used
		return true;
	}

	return false;
}


void Spout::ReleaseReceiver() 
{
	if(bMemoryShareInitOK) {
		return;
	}

	// can be done without a check here if(bDxInitOK || bMemoryShareInitOK)
	SpoutCleanUp();

	// LJ DEBUG
	bInitialized = false; // DEBUG - needs tracing
	Sleep(100); // DEBUG - not needed - debugging aid only
}


// If the local texure has changed dimensions this will return false
bool Spout::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	unsigned char * pDib;
	unsigned char * pBits;
	unsigned char * rgbBuffer;
	BITMAPINFOHEADER * pbmih; // pointer to it

	if(bDxInitOK) {
		// width, g_Width should all be the same
		// But it is the responsibility of the application to reset any texture that is being sent out.
		if(width != g_Width || height != g_Height) {
			return(UpdateSender(g_SharedMemoryName, width, height));
		}
		return(interop.WriteTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO));
	}
	else if(bMemoryShareInitOK) {

		//
		// *** Note RGB only ***
		//
		// Memoryshare mode has to get the texture pixels into a bitmap and and write them to shared memory
		int imagesize = width*height*3+sizeof(BITMAPINFOHEADER); // RGB bitmap
		rgbBuffer = (unsigned char *)malloc(imagesize*sizeof(unsigned char));
		if(rgbBuffer) {
			// Create a bitmap header
			pDib = rgbBuffer;
			pbmih = (BITMAPINFOHEADER FAR *)(pDib); // local memory pointer
			pBits = (unsigned char *)(pDib + sizeof(BITMAPINFOHEADER)); // pointer to DIB image bits
			// Fill the header
			pbmih->biSize          = sizeof(BITMAPINFOHEADER);
			pbmih->biWidth         = (LONG)width;
			pbmih->biHeight        = (LONG)height;
			pbmih->biPlanes        = 1;
			pbmih->biBitCount      = 24;
			pbmih->biCompression   = 0;
			pbmih->biSizeImage     = 0;
			pbmih->biXPelsPerMeter = 0;
			pbmih->biYPelsPerMeter = 0;
			pbmih->biClrUsed       = 0;
			pbmih->biClrImportant  = 0;

			// Get the pixels of the passed texture into the bitmap
			glBindTexture(TextureTarget, TextureID);
			glGetTexImage(TextureTarget, 0,  GL_RGB,  GL_UNSIGNED_BYTE, pBits);
			glBindTexture(TextureTarget, 0);

			// Default invert flag is true so do the flip to get it the
			// right way up unless the user has specifically indicated not to
			// LJ DEBUG - needs tracing semder/receiver - possible double invert - default false?
			// printf("bInvert = %d\n", bInvert);
			if(bInvert) FlipVertical(pBits, width, height, GL_RGB); // Default RGBA

			// Write the header plus the image data to shared memory
			interop.MemoryShare.WriteToMemory(pDib, imagesize);
				
			free((void *)rgbBuffer);

			return true;
			
		}
	}
	return false;

} // end SendTexture


// If the local texure has changed dimensions this will return false
bool Spout::SendImage(unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bAlignment, bool bInvert)
{
	unsigned char * pDib;
	unsigned char * pBits;
	unsigned char * rgbBuffer;
	unsigned int imagesize, imagewidth;
	BITMAPINFOHEADER * pbmih;

	if(bDxInitOK) {
		/*
		// Check for 4-byte alignment (default) for 
		if(bAlignment) {
			imagewidth = ((width*24+31)/32)*4; // each line is 4-byte aligned in size
		}
		else {
			imagewidth = width;
		}
		*/
		imagewidth = width;

		if(glFormat == GL_RGB) {
			imagesize = imagewidth*height*3; // RGB
		}
		else {
			imagesize = imagewidth*height*4; // RGBA
		}

		// width, g_Width should all be the same
		if(width != g_Width || height != g_Height) {
			UpdateSender(g_SharedMemoryName, g_Width, g_Height);
		}

		if(bInvert) {
			rgbBuffer = (unsigned char *)malloc(imagesize*sizeof(unsigned char));
			if(!rgbBuffer) return false;
			CopyMemory(rgbBuffer, pixels, imagesize);
			FlipVertical(rgbBuffer, width, height, glFormat); // Can be RGB or RGBA
			interop.WriteTexturePixels(rgbBuffer, width, height, glFormat, bAlignment);
			free((void *)rgbBuffer);
		}
		else {
			interop.WriteTexturePixels(pixels, width, height, glFormat, bAlignment);
		}

		return true; // no checks for now
	}
	else if(bMemoryShareInitOK) {
		//
		// Memoryshare mode 
		//
		//		*** NOTE RGB ONLY ***
		//
		// Memoryshare mode has to get the texture pixels into a bitmap and and write them to shared memory
		int imagesize = width*height*3+sizeof(BITMAPINFOHEADER); // RGB bitmap
		rgbBuffer = (unsigned char *)malloc(imagesize*sizeof(unsigned char));
		if(rgbBuffer) {
			// Create a bitmap header
			pDib = rgbBuffer;
			pbmih = (BITMAPINFOHEADER FAR *)(pDib); // local memory pointer
			pBits = (unsigned char *)(pDib + sizeof(BITMAPINFOHEADER)); // pointer to DIB image bits
			// Fill the header
			pbmih->biSize          = sizeof(BITMAPINFOHEADER);
			pbmih->biWidth         = (LONG)width;
			pbmih->biHeight        = (LONG)height;
			pbmih->biPlanes        = 1;
			pbmih->biBitCount      = 24;
			pbmih->biCompression   = 0;
			pbmih->biSizeImage     = 0;
			pbmih->biXPelsPerMeter = 0;
			pbmih->biYPelsPerMeter = 0;
			pbmih->biClrUsed       = 0;
			pbmih->biClrImportant  = 0;

			// Get the pixels of the passed image into the bitmap
			CopyMemory(pBits, pixels, width*height*3); //format is RGB

			// Default invert flag is true so do the flip to get it the
			// right way up unless the user has specifically indicated not to
			if(bInvert) FlipVertical(pBits, width, height, GL_RGB); // Default is RGBA

			// Write the header plus the image data to shared memory
			interop.MemoryShare.WriteToMemory(pDib, imagesize);
				
			free((void *)rgbBuffer);

			return true;
			
		}
	}

	return false;

} // end SendImage


//
// ReceiveTexture
//
bool Spout::ReceiveTexture(char* name, unsigned int &width, unsigned int &height, GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFBO)
{
	char newname[256];
	unsigned int newWidth, newHeight;
	DWORD dwFormat;
	HANDLE hShareHandle;
	unsigned char *src;
	BITMAPINFOHEADER * pbmih; // pointer to it
	unsigned int imagesize;
	unsigned char * rgbBuffer;

	// printf("Spout::ReceiveTexture HostFBO = %d\n", HostFBO);

	// Has it initialized yet ?
	if(!bInitialized) {

		// The name passed is the name to try to connect to 
		// unless the bUseActive flag is set or the name is not initialized
		// in which case it will try to find the active sender
		// Width and height are passed back as well
		if(name[0] != 0)
			strcpy_s(newname, 256, name);
		else
			newname[0] = 0;

		if(OpenReceiver(newname, newWidth, newHeight)) {
			// OpenReceiver will also set the global name, width, height and format
			// Pass back the new name, width and height to the caller
			// The change has to be detected by the application
			strcpy_s(name, 256, newname);
			width  = newWidth;
			height = newHeight;
			return true; 
		}
		else {
			// Initialization failure - the sender is not there 
			// Quit to let the app try again
			return false;
		}
	} // endif not initialized

	if(bDxInitOK && !bMemoryShareInitOK) {

		// Check to see if SpoutPanel has been opened 
		// the globals are reset if it has been
		// And the sender name will be different to that passed
		CheckSpoutPanel();

		// Test to see whether the current sender is still there
		if(interop.senders.CheckSender(g_SharedMemoryName, newWidth, newHeight, hShareHandle, dwFormat)) {

			// Current sender still exists
			// Has the width, height, texture format changed
			// DEBUG no global sharehandle
			if(newWidth > 0 && newHeight > 0) {
				if(newWidth  != g_Width 
				|| newHeight != g_Height
				|| dwFormat  != g_Format
				|| strcmp(name, g_SharedMemoryName) != 0 ) {

					// Re-initialize the receiver
					if(OpenReceiver(g_SharedMemoryName, newWidth, newHeight)) {				
						// TODO - Set the global texture ID here
						// g_TexID = TextureID;
						g_Width = newWidth;
						g_Height = newHeight;
						// Pass back the new current name and size
						strcpy_s(name, 256, g_SharedMemoryName);
						width  = g_Width;
						height = g_Height;
						// Return the new sender name and dimensions
						// The change has to be detected by the application
						return true;
					} // OpenReceiver OK
					else {
						// need what here
						return false;
					}
				} // width, height, format or name have changed
			} // width and height are zero
			else {
				// need what here
				return false;
			}
		} // endif CheckSender found a sender
		else {
			g_SharedMemoryName[0] = 0; // sender no longer exists
			ReleaseReceiver(); // Start again
			return false;
		} // CheckSender did not find the sender - probably closed

		// Sender exists and everything matched
		// globals are now all current, so pass back the current name and size
		// so that there is no change found by the host
		strcpy_s(name, 256, g_SharedMemoryName);
		width  = g_Width;
		height = g_Height;

		// If a valid texture was passed, read the shared texture into it
		// Otherwise skip it, all the checks are done
		if(TextureID > 0 && TextureTarget > 0) {
			if(!interop.ReadTexture(TextureID, TextureTarget, g_Width, g_Height, bInvert, HostFBO)) {
				return false;
			}
		}
		// All OK - drop though to return true
	} // was initialized in texture mode
	else {
		//
		// Memoryshare mode 
		//
		//		*** NOTE RGB ONLY ***
		//
		// Memoryshare mode - problem for reading the size beforehand is that
		// the framerate is halved. Reading the whole image assumes that the sender
		// does not reduce in size, but it has worked successfully so far.
		// Only solution is to always allocate a buffer of the desktop size.
		imagesize = g_Width*g_Height*3;
		rgbBuffer = (unsigned char *)malloc(GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN) + sizeof(BITMAPINFOHEADER));
		if(rgbBuffer) {
			if(interop.MemoryShare.ReadFromMemory(rgbBuffer, (sizeof(BITMAPINFOHEADER) + imagesize))) {
				pbmih = (BITMAPINFOHEADER *)rgbBuffer;
				// return for zero width and height
				if(pbmih->biWidth == 0 || pbmih->biHeight == 0) {
					free((void *)rgbBuffer);
					return false;
				}
				// check the size received to see if it matches the size passed in
				if((unsigned int)pbmih->biWidth != width || (unsigned int)pbmih->biHeight != height) {
					// return changed width and height
					width  = (unsigned int)pbmih->biWidth;
					height = (unsigned int)pbmih->biHeight;
				} // endif size changed
				else {
					// otherwise transfer the image data to the texture pixels - Note RGB only
					src = rgbBuffer + sizeof(BITMAPINFOHEADER);
					glBindTexture(TextureTarget, TextureID);
					glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, src);
					glBindTexture(TextureTarget, 0);
				} // endif size matches
			} // endif MemoryShare.ReadFromMemory
			else {
				free((void *)rgbBuffer);
				return false;
			}
			free((void *)rgbBuffer);
		} // end buffer alloc OK
		// All OK - drop though to return true
	}

	return true;

} // end ReceiveTexture


// Note was RGB only, but the format passed should go through now and work. 
// Default format is now GL_RGBA but Memoryshare mode remains RGB only.
// The host application must ensure a sufficient size for the pixel buffer
bool Spout::ReceiveImage(char* name, unsigned int &width, unsigned int &height, unsigned char* pixels, GLenum glFormat, GLuint HostFBO)
{
	char newname[256];
	unsigned int newWidth, newHeight;
	DWORD dwFormat;
	HANDLE hShareHandle;
	unsigned char *src;
	BITMAPINFOHEADER * pbmih;
	unsigned int imagesize;
	unsigned char * rgbBuffer;

	// Has it initialized yet ?
	if(!bInitialized) {
		// The name passed is the name to try to connect to 
		// unless the bUseActive flag is set or the name is not initialized
		// in which case it will try to find the active sender
		// Width and height are passed back as well
		if(name[0] != 0)
			strcpy_s(newname, 256, name);
		else
			newname[0] = 0;
		if(OpenReceiver(newname, newWidth, newHeight)) {
			// OpenReceiver will also set the global name, width, height and format
			// Pass back the new name, width and height to the caller
			// The change has to be detected by the application
			strcpy_s(name, 256, newname);
			width  = newWidth;
			height = newHeight;
			return true; 
		}
		else {
			// Initialization failure - the sender is not there 
			// Quit to let the app try again
			return false;
		}
	} // endif not initialized

	if(!bMemoryShareInitOK && bDxInitOK) {

		// Check to see if SpoutPanel has been opened 
		// the globals are reset if it has been
		CheckSpoutPanel();
		// To be tested - receivetexture works OK
		// Test to see whether the current sender is still there
		if(interop.senders.CheckSender(g_SharedMemoryName, newWidth, newHeight, hShareHandle, dwFormat)) {
			// Current sender still exists
			// Has the width, height, texture format changed
			// DEBUG no global sharehandle
			if(newWidth > 0 && newHeight > 0) {
				if(newWidth  != g_Width 
				|| newHeight != g_Height
				|| dwFormat  != g_Format
				|| strcmp(name, g_SharedMemoryName) != 0 ) {
					// Re-initialize the receiver
					if(OpenReceiver(g_SharedMemoryName, newWidth, newHeight)) {				
						// OpenReceiver will set the global name, width, height and texture format
						// TODO - set the global texture ID here
						// g_TexID = TextureID;
						// Pass back the new current name and size
						strcpy_s(name, 256, g_SharedMemoryName);
						width  = g_Width;
						height = g_Height;
						// Return the new sender name and dimensions
						// The change has to be detected by the application
						return true;
					} // OpenReceiver OK
					else {
						// need what here
						return false;
					}
				} // width, height, format or name have changed
			} // width and height are zero
			else {
				// need what here
				return false;
			}
		} // endif CheckSender found a sender
		else {
			g_SharedMemoryName[0] = 0; // sender no longer exists
			ReleaseReceiver(); // Start again
			return false;
		} // CheckSender did not find the sender - probably closed

		// Sender exists and everything matched

		// globals are now all current, so pass back the current name and size
		// so that there is no change found by the host
		strcpy_s(name, 256, g_SharedMemoryName);
		width  = g_Width;
		height = g_Height;

		// If a valid pixel pointer was passed, read the shared texture into it
		if(pixels) {
			// Default format is GL_RGBA
			if(interop.ReadTexturePixels(pixels, g_Width, g_Height, glFormat, HostFBO)) {
				return true;
			}
			else {
				return false;
			}
		}
		return true;
	} // was initialized in texture mode
	else {
		//
		// Memoryshare mode 
		//
		//		*** NOTE RGB ONLY ***
		//
		// problem for reading the size beforehand is that
		// the framerate is halved. Reading the whole image assumes that the sender
		// does not reduce in size, but it has worked successfully so far.
		// Only solution is to always allocate a buffer of the desktop size.
		imagesize = g_Width*g_Height*3;
		rgbBuffer = (unsigned char *)malloc(GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN) + sizeof(BITMAPINFOHEADER));
		if(rgbBuffer) {
			if(interop.MemoryShare.ReadFromMemory(rgbBuffer, (sizeof(BITMAPINFOHEADER) + imagesize))) {
				pbmih = (BITMAPINFOHEADER *)rgbBuffer;
				// return for zero width and height
				if(pbmih->biWidth == 0 || pbmih->biHeight == 0) {
					free((void *)rgbBuffer);
					return false;
				}
				// check the size received to see if it matches the size passed in
				if((unsigned int)pbmih->biWidth != width || (unsigned int)pbmih->biHeight != height) {
					// return changed width and height
					width  = (unsigned int)pbmih->biWidth;
					height = (unsigned int)pbmih->biHeight;
				} // endif size changed
				else {
					// otherwise transfer the image data to the texture pixels - Note RGB only
					src = rgbBuffer + sizeof(BITMAPINFOHEADER);
					CopyMemory(pixels, src, width*height*3); // assumes format is RGB
				} // endif size matches
			} // endif MemoryShare.ReadFromMemory
			free((void *)rgbBuffer);
			return true;
		} // end buffer alloc OK
	}
	return false;
}


// Can be used without OpenGL context
// NOTE : initializes and then de-initiaize memoryshare
// use before OpenReceiver and should not be called repeatedly
bool Spout::GetImageSize(char* name, unsigned int &width, unsigned int &height, bool &bMemoryMode)
{
	char newname[256];
	SharedTextureInfo TextureInfo;
	BITMAPINFOHEADER * pbmih;
	unsigned char * rgbBuffer;

	// Was initialized so get the sender details
	// Test to see whether the current sender is still there
	if(!interop.getSharedInfo(newname, &TextureInfo)) {
		// Try the active sender
		if(interop.senders.GetActiveSender(newname)) {
			if(interop.getSharedInfo(newname, &TextureInfo)) {
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
		interop.MemoryShare.Initialize();
		if(interop.MemoryShare.ReadFromMemory(rgbBuffer, sizeof(BITMAPINFOHEADER))) {
			pbmih = (BITMAPINFOHEADER *)rgbBuffer;
			// return for zero width and height
			if(pbmih->biWidth == 0 || pbmih->biHeight == 0) {
				free((void *)rgbBuffer);
				return false;
			}
			
			// Send back a name
			strcpy_s(name, 256, "memoryshare");
			
			// return the size received
			width  = (unsigned int)pbmih->biWidth;
			height = (unsigned int)pbmih->biHeight;

			interop.MemoryShare.DeInitialize(); 
			free((void *)rgbBuffer);
			bMemoryMode = true;

			return true;
		} // endif MemoryShare.ReadFromMemory
		free((void *)rgbBuffer);
	} // end buffer alloc OK

	return false;

} // end GetImageSize


//---------------------------------------------------------
bool Spout::BindSharedTexture()
{
	return interop.BindSharedTexture();
}


//---------------------------------------------------------
bool Spout::UnBindSharedTexture()
{
	return interop.UnBindSharedTexture();
}


//---------------------------------------------------------
bool Spout::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert)
{
	return interop.DrawSharedTexture(max_x, max_y, aspect, bInvert);
}


//---------------------------------------------------------
// 
bool Spout::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	//
	// Allow for change of sender size, even though the draw is independent of the 
	// shared texture size, otherwise receivers will get a constant size for this sender
	//
	if(bDxInitOK) {
		// width, g_Width should all be the same
		// But it is the responsibility of the application to reset any texture that is being sent out.
		if(width != g_Width || height != g_Height) {
			return(UpdateSender(g_SharedMemoryName, width, height));
		}
		return(interop.DrawToSharedTexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO));
	}

	return false;

}


// Compatibility is tested by loading OpenGL extensions
// and initializing DirectX and calling wglDXOpenDeviceNV
bool Spout::GetMemoryShareMode()
{
	// If already initialized, return what it initialized as
	if(bDxInitOK || bMemoryShareInitOK) {
		if(bMemoryShareInitOK) return true;
		else return false;
	}
	else {
		// otherwise do the compatibiliy test
		if(interop.GLDXcompatible()) {
			return false;
		}
		else {
			return true;
		}
	}
}


// Set memoryshare mode true or false
bool Spout::SetMemoryShareMode(bool bMemoryMode)
{

	bMemory = bMemoryMode; // force memoryshare mode

	// If already initialized, re-initialze
	if(bDxInitOK || bMemoryShareInitOK) {
		SpoutCleanUp();
		return OpenSpout();
	}

	return true;
}

//
// selectSenderPanel - used by a receiver
// Optional message argument
bool Spout::SelectSenderPanel(const char *message)
{
	HANDLE hMutex1;
	HMODULE module;
	char UserMessage[512];
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	if(message != NULL) {
		strcpy_s(UserMessage, 512, message); // could be an arg or a user message
	}
	else {
		UserMessage[0] = 0; // make sure SpoutPanel does not see an un-initialized string
	}

	if(bMemory || bMemoryShareInitOK) {
		sprintf_s(UserMessage, 512, "Spout running in memoryshare mode\nThere can only be one sender\nno sender selection available");
	}

	// For a texture share receiver pop up SpoutPanel to allow the user to select a sender
	// The selected sender is then the "Active" sender and this receiver switches to it.
	// SpoutPanel.exe has to be in the same folder as this executable
	// This rather complicated process avoids having to use a dialog within a dll
	// which causes problems with FreeFrameGL plugins and Max eternals

	// First check whether the panel is already running
	// Try to open the application mutex.
	bool bUsesRegistry = true; // Spout vers 2.002 or later
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
	if (!hMutex1) {
		// No mutex, so not running, so can open it
		// See if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
		if(!ReadPathFromRegistry(path, "Software\\Leading Edge\\SpoutPanel", "InstallPath")) {
			bUsesRegistry = false;
			// Path not registered so find the path of the host program
			// where SpoutPanel should have been copied
			module = GetModuleHandle(NULL);
			GetModuleFileNameA(module, path, MAX_PATH);
			_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
			_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
			// Does SpoutPanel.exe exist in this path ?
			if(!PathFileExistsA(path) ) {
				// Try the current working directory
				if(_getcwd(path, MAX_PATH)) {
					strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
					// Does SpoutPanel exist here?
					if(!PathFileExistsA(path) ) {
						return false;
					}
				}
			}
		}

		/*
		printf("SpoutPanel path [%s]\n", path); // Spoutpanel exists
		// Can get version information here
		DWORD dwFileVersionMS = 0;
		DWORD dwFileVersionLS = 0;
		if(FindFileVersion(path, dwFileVersionMS, dwFileVersionLS)) {
			printf( "File Version: %d.%d.%d.%d\n",
				( dwFileVersionMS >> 16 ) & 0xffff,
				( dwFileVersionMS >>  0 ) & 0xffff,
				( dwFileVersionLS >> 16 ) & 0xffff,
				( dwFileVersionLS >>  0 ) & 0xffff
			);
		}
		*/

		// 
		// Use  ShellExecuteEx so we can test its return value later
		//
		ZeroMemory(&ShExecInfo, sizeof(ShExecInfo));
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = (LPCSTR)path;
		ShExecInfo.lpParameters = UserMessage;
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;	

		ShellExecuteExA(&ShExecInfo);

		Sleep(125); // alow time for SpoutPanel to open 0.125s

		// Returns straight away here but multiple instances of SpoutPanel
		// are prevented in it's WinMain procedure by the mutex.
		// An infinite wait here causes problems.
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		bSpoutPanelOpened = true;
	}
	else {
		// We opened it so close it, otherwise it is never released
		CloseHandle(hMutex1);
	}

	// The mutex exists, so another instance is already running
	// Find the dialog window and bring it to the top
	// the spout dll dialog is opened as topmost anyway but pop it to
	// the front in case anything else has stolen topmost
	HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
	if(IsWindow(hWnd)) {
		SetForegroundWindow(hWnd); 
		// prevent other windows from hiding the dialog
		// and open the window wherever the user clicked
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	}

	return true;

} // end selectSenderPanel


// 22.02.15 - find the SpoutPanel version
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
        printf( "Error in GetFileVersionInfoSize: %d\n", GetLastError() );
        return false;
    }

    pbVersionInfo = new BYTE[ dwSize ];

    if ( !GetFileVersionInfoA( FilePath, 0, dwSize, pbVersionInfo ) )  {
        printf( "Error in GetFileVersionInfo: %d\n", GetLastError() );
        delete[] pbVersionInfo;
        return false;
    }

    if ( !VerQueryValueA( pbVersionInfo, "\\", (LPVOID*) &pFileInfo, &puLenFileInfo ) ) {
        printf( "Error in VerQueryValue: %d\n", GetLastError() );
        delete[] pbVersionInfo;
        return false;
    }

	versMS = pFileInfo->dwFileVersionMS;
	versLS = pFileInfo->dwFileVersionLS;

    /*
	printf( "File Version: %d.%d.%d.%d\n",
		( pFileInfo->dwFileVersionMS >> 16 ) & 0xffff,
        ( pFileInfo->dwFileVersionMS >>  0 ) & 0xffff,
        ( pFileInfo->dwFileVersionLS >> 16 ) & 0xffff,
        ( pFileInfo->dwFileVersionLS >>  0 ) & 0xffff
        );

    printf( "Product Version: %d.%d.%d.%d\n",
        ( pFileInfo->dwProductVersionMS >> 24 ) & 0xffff,
        ( pFileInfo->dwProductVersionMS >> 16 ) & 0xffff,
        ( pFileInfo->dwProductVersionLS >>  8 ) & 0xffff,
        ( pFileInfo->dwProductVersionLS >>  0 ) & 0xffff
        );
	*/

	return true;

}
// ======================



int Spout::GetSenderCount() {
	std::set<string> SenderNameSet;
	if(interop.senders.GetSenderNames(&SenderNameSet)) {
		return((int)SenderNameSet.size());
	}
	return 0;
}

//
// Get a sender name given an index and knowing the sender count
// index             - in
// sendername        - out
// sendernameMaxSize - in
bool Spout::GetSenderName(int index, char* sendername, int sendernameMaxSize)
{
	std::set<string> SenderNameSet;
	std::set<string>::iterator iter;
	string namestring;
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


// All of these redundant - can be directly in the Receiver class . TODO - Change/Test
//---------------------------------------------------------
bool Spout::GetActiveSender(char* Sendername)
{
	return interop.senders.GetActiveSender(Sendername);
}


//---------------------------------------------------------
bool Spout::SetActiveSender(char* Sendername)
{
	return interop.senders.SetActiveSender(Sendername);
}


bool Spout::GetSenderInfo(char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return interop.senders.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}



int Spout::GetVerticalSync()
{
	return interop.GetVerticalSync();
}


bool Spout::SetVerticalSync(bool bSync)
{
	return interop.SetVerticalSync(bSync);
}



// ========================================================== //
// ==================== LOCAL FUNCTIONS ===================== //
// ========================================================== //

// Find if the sender exists
// If the name begins with a null character, or the bUseActive flag has been set
// return the active sender name if that exists
bool Spout::OpenReceiver (char* theName, unsigned int& theWidth, unsigned int& theHeight)
{

	char Sendername[256]; // user entered Sender name
	DWORD dwFormat;
	unsigned int width;
	unsigned int height;
	bool bMemoryMode = true;

	// printf("OpenReceiver\n");
	// A valid name is sent and the user does not want to use the active sender
	if(theName[0] != 0 && !bUseActive) {
		strcpy_s(Sendername, 256, theName);
	}
	else {
		Sendername[0] = 0;
	}

	// Set initial size to that passed in
	width  = theWidth;
	height = theHeight;

	// Make sure it has been initialized
	if(!OpenSpout()) {
		return false;
	}

	// Render window must be visible for initSharing to work
	g_hWnd = WindowFromDC(wglGetCurrentDC()); 

	if(!bMemoryShareInitOK && bDxInitOK) bMemoryMode = false;

	// Check compatibility
	if(!bMemoryShareInitOK && bDxInitOK) { 
		bMemoryMode = false;
		// Find if the sender exists
		// Or if a null name given return the active sender if that exists
		if(!interop.senders.FindSender(Sendername, width, height, g_ShareHandle, dwFormat)) {
			// Given name not found ? - has SpoutPanel been opened ?
			// the globals are reset if it has been
			if(CheckSpoutPanel()) {
				// set vars for below
				strcpy_s(Sendername, 256, g_SharedMemoryName);
				width    = g_Width;
				height   = g_Height;
				dwFormat = g_Format;
			}
			else {
			    return false;
			}
		}
		else if(bMemoryShareInitOK) {
			// TODO : Find a memoryshare sender if running
		}

		// Set the globals
		strcpy_s(g_SharedMemoryName, 256, Sendername);
		g_Width  = width;
		g_Height = height;
		g_Format = dwFormat;

	}

	// Initialize a receiver in either memoryshare or texture mode
	if(InitReceiver(g_hWnd, Sendername, width, height, bMemoryMode)) {
		// Pass back the sender name and size now that the global
		// width and height have been set
		strcpy_s(theName, 256, Sendername); // LJ DEBUG global?
		theWidth  = g_Width;
		theHeight = g_Height;
		return true;
	}

	return false;

} // end OpenReceiver


/*
01.08.14 - unfinished - memoryshare work
bool Spout::OpenReceiver (char* theName, unsigned int& theWidth, unsigned int& theHeight)
{

	char Sendername[256]; // user entered Sender name
	DWORD dwFormat;
	unsigned int width;
	unsigned int height;
	bool bMemoryMode = true;

	// A valid name is sent and the user does not want to use the active sender
	if(theName[0] && !bUseActive) {
		// printf("    (%s) %dx%d - bUseActive = %d\n", theName, theWidth, theHeight, bUseActive);
		strcpy_s(Sendername, 256, theName);
	}
	else {
		// printf("Spout::OpenReceiver (use active sender) %dx%d -  - bUseActive = %d\n", theWidth, theHeight, bUseActive);
		Sendername[0] = 0;
	}

	// Set initial size to that passed in
	width  = theWidth;
	height = theHeight;

	// Make sure it has been initialized
	if(!OpenSpout()) {
		// printf("    Spout::OpenReceiver error 1\n");
		return false;
	}

	// printf("    bMemoryShareInitOK = %d bDxInitOK = %d\n", bMemoryShareInitOK, bDxInitOK);

	// No senders for texture mode - just return false
	if(!bMemoryShareInitOK && bDxInitOK && GetSenderCount() == 0) {
		// printf("    Spout::OpenReceiver error 2\n");
		return false;
	}

	// Render window must be visible for initSharing to work
	g_hWnd = WindowFromDC(wglGetCurrentDC()); 

	// Check the user memoryshare override as well as whether it initialized memoryshare
	if(!bMemory && !bMemoryShareInitOK && bDxInitOK) {
		bMemoryMode = false;
		// Find if the sender exists
		// Or if a null name given return the active sender if that exists
		if(!interop.senders.FindSender(Sendername, width, height, g_ShareHandle, dwFormat)) {

			// Given name not found ? - has SpoutPanel been opened ?
			// the globals are reset if it has been
			if(CheckSpoutPanel()) {
				// set vars for below
				strcpy_s(Sendername, 256, g_SharedMemoryName);
				width    = g_Width;
				height   = g_Height;
				dwFormat = g_Format;
			}
			else {
				// printf("    Spout::OpenReceiver error 3\n");
			    return false;
			}
		}
	}
	else if(bMemoryShareInitOK) {
		// printf("    Spout::OpenReceiver memoryshare\n");
		// Find a memoryshare sender if running
		if(interop.MemoryShare.GetImageSizeFromSharedMemory(width, height)) {
				// global width and height have now been set
		// if(GetImageSize(Sendername, width, height, bMemoryMode)) {
			// Set the globals
			strcpy_s(g_SharedMemoryName, 256, "memoryshare");
			g_Width  = width;
			g_Height = height;	

			strcpy_s(theName, 256, g_SharedMemoryName);
			theWidth  = g_Width;
			theHeight = g_Height;
			// printf("  memoryshare sender found (%s) %dx%d\n", g_SharedMemoryName, width, height);
			bInitialized = true;

			return true;
		}
		else {
			// printf("  memoryshare sender not found\n"); 
			return false;
		}
	}

	// Set the globals
	strcpy_s(g_SharedMemoryName, 256, Sendername);
	g_Width  = width;
	g_Height = height;
	g_Format = dwFormat;

	// printf("Spout::OpenReceiver found (%s) %dx%d)\n", Sendername, width, height);

	// Initialize a receiver in either memoryshare or texture mode
	if(InitReceiver(g_hWnd, Sendername, width, height, bMemoryMode)) {
		// Pass back the sender name and size now that the global
		// width and height have been set
		strcpy_s(theName, 256, Sendername); // LJ DEBUG global?
		theWidth  = g_Width;
		theHeight = g_Height;
		return true;
	}

	// printf("    Spout::OpenReceiver error 4\n");

	return false;

} // end OpenReceiver
*/


bool Spout::InitSender (HWND hwnd, char* theSendername, unsigned int theWidth, unsigned int theHeight, DWORD dwFormat, bool bMemoryMode) 
{

	// Texture share mode quit if there is no image size to initialize with
	// Memoryshare can detect a Sender while the receiver is running
	if(!bMemoryMode && (theWidth == 0 || theHeight == 0)) {
		printf("Spout::InitSender error 1\n");
		return false;
	}

	// only try dx if :
	//	- the user memory mode flag is not set
	//	- Hardware is compatible
	if(bGLDXcompatible && !bMemoryMode) {

		// Initialize the GL/DX interop and create a new shared texture (false = sender)
		if(!interop.CreateInterop(hwnd, theSendername, theWidth, theHeight, dwFormat, false)) {  // False for a sender
			printf("Spout::InitSender error 2\n");
			return false;
		}

		// Set global name
		strcpy_s(g_SharedMemoryName, 256, theSendername);
				
		// Get the sender width, height and share handle into local copy
		interop.senders.GetSenderInfo(g_SharedMemoryName, g_Width, g_Height, g_ShareHandle, g_Format);

		bDxInitOK = true;
		bMemoryShareInitOK	= false;
		bInitialized = true;

		return true;
	} 
	// ================== end sender initialize ==============================

	// if it did not initialize, try to set up for memory share transfer
	if(!bInitialized) {

		// printf("Spout::InitSender error 3\n");
		
		// Set globals - they will be reset by a receiver but are needed for a sender
		g_Width  = theWidth;
		g_Height = theHeight;
		bMemoryShareInitOK = InitMemoryShare(false); // sender
		if(bMemoryShareInitOK) {
			bDxInitOK = false;
			bInitialized = true;
			return true;
		}
	}

	// printf("Spout::InitSender error 4\n");

	return false;

} // end InitSender


bool Spout::InitReceiver (HWND hwnd, char* theSendername, unsigned int theWidth, unsigned int theHeight, bool bMemoryMode) 
{

	char sendername[256];
	unsigned int width = 0;
	unsigned int height = 0;
	DWORD format;
	HANDLE sharehandle;
	
	if(theSendername[0]) {
		strcpy_s(sendername, 256, theSendername); // working name local to this function
	}
	else {
		sendername[0] = 0;
	}


	// Texture share mode quit if there is no image size to initialize with
	// Memoryshare can detect a Sender while the receiver is running
	if(!bMemoryMode && (theWidth == 0 || theHeight == 0)) {
		return false;
	}

	// bChangeRequested is set when the Sender name, image size or share handle changes
	// or the user selects another Sender - everything has to be reset if already initialized
	if(bChangeRequested) {
		SpoutCleanUp();
		bDxInitOK			= false;
		bMemoryShareInitOK	= false;
		bInitialized		= false;
		bChangeRequested	= false; // only do it once
	}
	
	//
	// only try dx if :
	//	- the user memory mode flag is not set
	//	- Hardware is compatible
	if(bGLDXcompatible && !bMemoryMode) {
		//
		// ============== Set up for a RECEIVER ============
		//
		// Find a sender and return the name, width, height, sharehandle and format
		if(!interop.senders.FindSender(sendername, width, height, sharehandle, format)) {
			return false;
		}

		// Initialize the receiver interop (this will create globals local to the interop class)
		if(!interop.CreateInterop(hwnd, sendername, width, height, format, true)) { // true meaning receiver
			return false;
		}

		// Set globals here
		g_Width  = width;
		g_Height = height;
		g_ShareHandle = sharehandle;
		g_Format = format;
		strcpy_s(g_SharedMemoryName, 256, sendername);

		bDxInitOK = true;
		bMemoryShareInitOK = false;
		bInitialized = true;

		return true;
	} // ================== end receiver initialize ==============================

	// if it did not initialize, try to set up for memory share transfer
	if(!bInitialized) {
		// Set globals - they will be reset by a receiver but are needed for a sender
		g_Width  = theWidth;
		g_Height = theHeight;
		bMemoryShareInitOK = InitMemoryShare(true); // receiver
		if(bMemoryShareInitOK) {
			bDxInitOK	 = false;
			bInitialized = true;
			return true;
		}
	}

	return false;

} // end InitReceiver


bool Spout::InitMemoryShare(bool bReceiver) 
{
	// initialize shared memory
	if(!bMemoryShareInitOK) bMemoryShareInitOK = interop.MemoryShare.Initialize();

	if(bMemoryShareInitOK) {
		if(!bReceiver) {
			// Set the sender mutex so that if a receiver attempts to read
			// and a sender is not present, there is no wait delay
			interop.MemoryShare.CreateSenderMutex();
			return true;
		}
		else {
			// A receiver - is a memoryshare sender running ?
			if(interop.MemoryShare.GetImageSizeFromSharedMemory(g_Width, g_Height)) {
				// global width and height have now been set
				return true;
			}
			else {
				// DEBUG - Do we want this - or just wait ?
				interop.MemoryShare.DeInitialize();
				return false;
			}
		}
	}
	else {
		interop.MemoryShare.DeInitialize();
		bMemoryShareInitOK = false;
		// drop though and return fail
	} // end memory share initialize

	return false;

} // end InitMemoryShare


bool Spout::ReleaseMemoryShare()
{
	if(bMemoryShareInitOK) interop.MemoryShare.DeInitialize();
	bMemoryShareInitOK = false;

	return true;
}

//
// SpoutCleanup
//
void Spout::SpoutCleanUp(bool bExit)
{
	interop.CleanupInterop(bExit); // true means it is the exit so don't call wglDXUnregisterObjectNV
	
	bDxInitOK = false;
	g_ShareHandle = NULL;
	g_Width	= 0;
	g_Height= 0;
	g_Format = 0;

	// important - we no longer want the global shared memory name and need to reset it
	g_SharedMemoryName[0] = 0; 

	// Set default for CreateReceiver
	bUseActive = false;

	// Important - everything is reset (see ReceiveTexture)
	bInitialized = false;

	ReleaseMemoryShare(); // de-init MemoryShare if it has been initiaized

}

// ========= USER SELECTION PANEL TEST =====
//
//	This is necessary because the exit code needs to be tested
//
bool Spout::CheckSpoutPanel()
{
		SharedTextureInfo TextureInfo;
		HANDLE hMutex;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// If SpoutPanel has been activated, test if the user has clicked OK
		if(bDxInitOK && bSpoutPanelOpened) { // User has activated spout panel
			hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
			if (!hMutex) { // It has now closed
				bSpoutPanelOpened = false; // Don't do this part again
				// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
				// to get the exit code from SpoutPanel
				if(ShExecInfo.hProcess) {
					GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
					// Only act if exit code = 0 (OK)
					if(dwExitCode == 0) {
						// SpoutPanel has been activated
						// Check for an unregistered sender first because this will not have been set as active yet
						// Try to get the current sender name from the registry (24.05.15 instead of text file)
						// Text file method does not work if SpoutPanel is in the Program Files folder without Admin privileges
						// SpoutPanel now always writes the selected sender name to the registry
						// so this first check should always work
						newname[0] = 0;
						if(!ReadPathFromRegistry(newname, "Software\\Leading Edge\\SpoutPanel", "Sendername")) {
							// Otherwise try the text file method
							string line;
							HMODULE module;
							char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];
						
							// Find the path of the host program where SpoutPanel should have been copied
							module = GetModuleHandle(NULL);
							GetModuleFileNameA(module, path, MAX_PATH);
							_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
							_makepath_s(path, MAX_PATH, drive, dir, "spoutpanel", ".txt");

							ifstream infile(path, ios::in);
							if (infile.is_open()) {
								if(getline(infile, line)) {
									strcpy_s(newname, 256, line.c_str());
								}
								infile.close();
								remove(path);
							}
						}

						// Do we have a sender name from the registry or a text file ?
						if(newname[0] != 0) {
							// Does the sender exist ?
							if(interop.senders.getSharedInfo(newname, &TextureInfo)) {
								strcpy_s(g_SharedMemoryName, 256, newname);
								g_Width  = (unsigned int)TextureInfo.width;
								g_Height = (unsigned int)TextureInfo.height;
								g_Format = TextureInfo.format;
								// Register in the list of senders and make it the active sender
								interop.senders.RegisterSenderName(newname);
								interop.senders.SetActiveSender(newname);
								bRet = true; // will pass on next call to receivetexture
							}
						}
						else {
							// No name in registry or text file, so get the active sender which is set by spoutpanel
							if(interop.senders.GetActiveSender(newname)) { // returns the active sender name
								if(interop.getSharedInfo(newname, &TextureInfo)) {
									strcpy_s(g_SharedMemoryName, 256, newname);
									g_Width  = (unsigned int)TextureInfo.width;
									g_Height = (unsigned int)TextureInfo.height;
									g_Format = TextureInfo.format;
									bRet = true; // will pass on next call to receivetexture
								}
							} // no active sender
						} // no active sender or unregistered sender
					} // endif SpoutPanel OK
				} // got the exit code
			} // endif no mutex so SpoutPanel has closed
			CloseHandle(hMutex);
		}
		return bRet;
} // ========= END USER SELECTION PANEL =====



// Adapted from FreeImage function
// Flip the image vertically along the horizontal axis.
// Default format is RGBA
bool Spout::FlipVertical(unsigned char *src, unsigned int width, unsigned int height, GLenum glFormat) 
{
		BYTE *From, *Mid;
		int pitch;

		// swap the buffer
		if(glFormat == GL_RGB) // not the default
			pitch = width*3;
		else
			pitch = width*4; // RGBA assumed otherwise

		// copy between aligned memories
		Mid = (BYTE*)malloc(pitch * sizeof(BYTE));
		if (!Mid) return false;

		From = src;
	
		unsigned int line_s = 0;
		unsigned int line_t = (height-1)*pitch;

		for(unsigned int y = 0; y<height/2; y++) {
			CopyMemory(Mid, From + line_s, pitch);
			CopyMemory(From + line_s, From + line_t, pitch);
			CopyMemory(From + line_t, Mid, pitch);
			line_s += pitch;
			line_t -= pitch;
		}

		free((void *)Mid);

		return true;
}


/*
//
// http://stackoverflow.com/questions/19738898/2d-array-flip-vertical
//
void Spout::ppmFlipVertical(int (&image)[MAXROWS][MAXCOLS])
{
    for (int r = 0; r < (MAXROWS/2); r++)
    {
        for (int c = 0; c != MAXCOLUMNS; ++c)
        {
             std::swap(image[r][c], image[MAXROWS - 1 - r][c]);
        }
    }
}
*/

/* - works
//
// http://stackoverflow.com/questions/14798604/simple-flip-buffer-vertically-issue-in-c-c
//
// void flip(unsigned* buffer, unsigned width, unsigned height)
bool Spout::FlipVertical(unsigned char *buffer, unsigned int width, unsigned int height, GLenum glFormat)
{
    unsigned int rows = height / 2; // Iterate only half the buffer to get a full flip
    unsigned int* tempRow;
	unsigned int pf;
	
	if(glFormat == GL_RGB) // not the default
		pf = 3;
	else
		pf = 4;


		tempRow = (unsigned*)malloc(width * pf * sizeof(unsigned int));

    for (unsigned int rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        memcpy(tempRow, buffer + rowIndex * width, width * sizeof(unsigned));
        memcpy(buffer + rowIndex * width *pf, buffer + (height - rowIndex - 1) * width *pf, width * pf * sizeof(unsigned int));
        memcpy(buffer + (height - rowIndex - 1) * width * pf, tempRow, width * pf * sizeof(unsigned int));
    }

    free(tempRow);

	return true;
}
*/

bool Spout::OpenSpout()
{
	HDC hdc;

	// printf("OpenSpout\n");

	// Safety return if already initialized
	if(bDxInitOK || bMemoryShareInitOK) {
		bGLDXcompatible = bDxInitOK;
		// printf("    Already initialized\n");
		return true;
	}

	// Has the global memoryshare over-ride flag been set?
	if(!bMemory) {
		hdc = wglGetCurrentDC(); // OpenGl device context is needed
		if(!hdc) {
			MessageBoxA(NULL, "    Cannot get GL device context", "OpenSpout", MB_OK);
			return false;
		}

		g_hWnd = WindowFromDC(hdc); // can be null though
		if(interop.LoadGLextensions()) { // did the extensions load OK ?
			// Initialize DirectX and prepare GLDX interop
			if(interop.OpenDirectX(g_hWnd, GetDX9())) { // did the NVIDIA open interop extension work ?
				bDxInitOK = true; // DirectX initialization OK
				bMemoryShareInitOK = false;
				bGLDXcompatible = true; // Set global compatibility flag as well
				// printf("    OpenSpout OK\n");
				return true; 
			}
		}
		else {
			printf("    Extensions failed to load\n");
		}

	}

	// Drop through and try to initialize shared memory
	bDxInitOK = false;
	bGLDXcompatible = false;
	bMemoryShareInitOK = false;
	if(interop.MemoryShare.Initialize()) {
		bMemoryShareInitOK = true;
		// printf("   Initialized shared memory\n");
		return true;
	}

	// printf("    OpenSpout failed\n");

	return false;
}

bool Spout::SetDX9(bool bDX9)
{
	return interop.UseDX9(bDX9);
}


bool Spout::GetDX9()
{
	return interop.isDX9();
}

// High resolution counter
void Spout::StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li)) {
		printf("QueryPerformanceFrequency failed!\n");
	}
    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double Spout::GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}


bool Spout::WritePathToRegistry(const char *filepath, const char *subkey, const char *valuename)
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


bool Spout::ReadPathFromRegistry(const char *filepath, const char *subkey, const char *valuename)
{
	HKEY  hRegKey;
	LONG  regres;
	DWORD  dwSize, dwKey;  

	dwSize = MAX_PATH;

	// Does the key exist
	regres = RegOpenKeyExA(HKEY_CURRENT_USER, subkey, NULL, KEY_READ, &hRegKey);
	if(regres == ERROR_SUCCESS) {
		// Read the key Filepath value
		regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)filepath, &dwSize);
		RegCloseKey(hRegKey);
		if(regres == ERROR_SUCCESS)
			return true;
	}

	// Just quit if the key does not exist
	return false;

}


// For debugging only
void Spout::UseAccessLocks(bool bUseLocks)
{
	interop.spoutdx.bUseAccessLocks = bUseLocks;

}

