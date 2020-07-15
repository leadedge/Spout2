/*

				spoutGLDXinterop.cpp

		See also - spoutDirectX, spoutSenderNames

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
	========================

		15-07-14	- ReadTexturePixels - allowed for variable OpenGL format instead of RGB only.
					- Needs testing. 
					- TODO - variable gl format for WriteTexturePixels
		21.07.14	- removed local fbo and replaced with temporary fbo within
					  texture functions due to problems with Max / Jitter
		22-07-14	- added option for DX9 or DX11
		23-07-14	- cleanup of DX9 / DX11 functions
		29-07-14	- pass format 0 for DX9 sender
		31-07-14	- Corrected DrawTexture aspect argument
		13-08-14	- OpenGL texture retained on cleanup
		14-08-14	- Corrected texture delete without context
		16-08-14	- created DrawToSharedTexture
		18-08-14	- debugging with WriteTexture method
		 __ names class revision additions __
		19-08-14	- activated event locks
		01.09.14	- removed temp fbo for texture transfers and returned to use of a common fbo
					- delete texture and fbo on cleanup
					- set texture and fbo to zero on cleanup, otherwise errors in Jitter
					- changed to vertex array draw for DrawToSharedTexture
					- Removed PAINT message from OpenDirectX9 due to crash of sender in Magic
		03.09.14	- Replaced with UpdateWindow and limited to Resolume only.
					- Cleanup
		15.09.14	- corrected access lock for DrawToSharedTexture and ReadTexturePixels
		21.09.14	- mutex texture access locks
		23.09.14	- moved general mutex texture access lock to the SpoutDirectX class
		23.09.14	- test for DirectX 11 support in UseDX9, IsDX9 and OpenDirectX
		24.09.14	- save and restore fbo for read:write:drawto texture
		28.09.14	- Added GL format argument for WriteTexturePixels
					- Added bAlignment  (4 byte alignment) flag for WriteTexturePixels
					- Changed GLformat argument from int to GLenum in ReadTexturePixels
					- Changed default GLformat from GL_RGB to GL_RGBA in ReadTexturePixels
					- Added Host FBO argument for ReadTexture, DrawToSharedTexture, WriteTexture
		12.10.14	- cleaned up CreateInterop for sender updates
		15.10.14	- added safety release of texture in CreateDX9interop in case of previous application crash
		17.10.14	- Directx 11 release context before device
		21.10.14	- removed keyed mutex lock due to reported driver problems
		21.10.14	- Allow for compatible texture formats
					  DirectX 11 format 87, DirectX9 D3DFMT_X8R8G8B8, and the default D3DFMT_A8R8G8B8
		21.10.14	- Allow DirectX texture formats to be registered in CreateInterop
		24.10.14	- Fall back to DirectX 9 if DirectX11 init fails
		24.11.14	- removed context print statement in CleanupInterop
		23.12.14	- added host fbo arg to ReadTexturePixels
					  Changed readback method to glReadPixels
		04.02.15	- Changed header default to DirectX 9 instead of DirectX 11
		09.02.15	- added invert flag to DrawSharedTexture (default true with no args)
		12.02.15	- Changed OpenDirectX to check for Intel graphics and open DirectX 9 if present
		13.02.15	- OpenDirectX9 included SendMessageTimeout before attempting to get the fg window text
		14.02.15	- Used PathStripPath function requiring shlwapi.h (see SpoutSDK.h)
		--
		21.05.15	- OpenDirectX : Intel auto detection removed and replaced by an installer with option for DX11
					- Programmer must SetDX9(true) for compilation using DirectX 11 functions
		29.05.15	- Included SetAdapter for multiple adapters : Franz Hildgen.
		01.06.15	- Read/Write DX9 mode from registry
		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
		08.06.15	- removed dx9 flag from setadapter
		20.06.15	- removed Intel : Optimus graphics detecion from GetAdapterInfo
		08.07.15	- Only reads registry for DX9 mode but does not write it
		25.08.15	- moved release texture before release of device - to be checked for a receiver
		26.08.15	- set the executable path to the sender's shared info
					- Added GetHostPath to retrieve the path of the host that produced the sender
		28.08.15	- Introduced RedrawWindow again instead of WM_PAINT due to crash with Windows 10
					  Window invalidate and redraw works OK - Win 7 32bit.
		01.09.15	- added MessageBox error warnings in CreateInterop and SpoutGLextensions::LoadGLextensions
		12.09.15	- Finalised revised SpoutMemoryShare class and functions
		14.09.15	- ReadTexture : WriteTexture change default invert to true in line with Spout class default
		15.09.15	- GetMemoryShare : do not return memoryshare true if the 2.005 installer has not set the 
					  "MemoryShare" key to avoid problems with 2.004 apps.
		21.09.15	- Change SetMemoryShareMode to apply only if there is a 2.005 installation or later
		22.09.15	- fixed source pointer start offset for memoryshare flip vertically between pBuf and pixels
		24.09.15	- Removed Enable/Disable texture target from texture bind. When a texture is first bound,
					  it assumes the specified target and we pass the required target already.
					  Problem noted with Cinder in memorshare mode.
		25.09.15	- Changed SetMemoryShare to allow for true or false.
		09.10.15	- TODO : check invert defaults for WriteTexture, WriteMemory, DrawSharedTexture, DrawToSharedTexture
					- Removed FlipVertical function - now unused with 2.005
		10.10.15	- Created DrawSharedMemory and DrawToSharedMemory - to be tested
					- made m_dxShareHandle public
		11.10.15	- Protect aganst NULL texture in ReadMemory
		12.10.15	- Add glCheckFramebufferStatusEXT to ReadTexture
		15.10.15	- Add PrintFBOstatus function
					- Add glCheckFramebufferStatusEXT and error report to all functions using glFramebufferTexture2DEXT
		22.10.15	- Moved DX11available from the directx class and changed to a test of DX11 device open
					  rather than operating system detection
		14.11.15	- changed functions to "const char *" where required
		20.11.15	- Registry read/write moved to SpoutDirectX class
		25.02.16	- Introduced read of MaxSenders from the registry for 2.005
		10.03.16	- introduced try : catch for memoryshare copymemory function
		16.03.16	- alignment 1 pixel for GL_RGB and GL_BGR_EXT in WriteTexturePixels and ReadTexturePixels
		17.03.16	- added bBGRAavailable flag to indicate whether BGRA extensions are supported
					- added function IsBGRAavailable to retrieve availability flag
					- changed WriteTexturePixels and WriteMemoryPixels to const unsigned char for pixel buffer
		21.03.16	- Added glFormat, bInvert and hostfbo to WriteMemoryPixels
					  and changed to use a local OpenGL texture for data conversion and flip
					- Changed ReadMemory and WriteMemory to use a local OpenGL texture
					- Added CopyTexture function
					- Added buffer flip and format conversion as utilities 
		28.03.16	- Added bGLDXavailable and switch to memoryshare in LoadGLextensions
		04.04.16	- Texture copy functions revised
					  Changed WriteTexture, ReadTexture and CopyTexture
					  to always use fbo blit if blit extensions are available
		19.04.16	- used glTexSubImage2D in WriteTexturePixels and ReadMemory
		27.04.16	- PBO functions for pixels transfer
					  LoadTexturePixels and UnloadTexturePixels used if PBO extensions available
					  in ReadTexturePixels and WriteTexturePixels
		28.04.16	- variable format for LoadTexturePixels
		01.05.16	- pbo functions for memoryshare ReadMemory and WriteMemory
		03.05.16	- SetPBOavailable(true/false) added to enable/disable pbo functions
		07.05.16	- SetPBOavailable changed to SetBufferMode
		22.05.16	- CleanupDirectX in interop cleanup
		09.06.16	- Corrected interop and mutex lock checks for fail in all functions
		16.06.16	- Added WriteDX9surface
		18.06.16	- Add invert to ReadTexturePixels
		23.06.16	- change back to 2.004 logic for mutex and interop lock checks
					- Mutex or interop lock fail does not mean read failure, but just no access
					  The current texture is re-used for a missed frame.
		03.07-16	- Use helper functions for conversion of 64bit HANDLE to unsigned __int32
					  and unsigned __int32 to 64bit HANDLE
					  https://msdn.microsoft.com/en-us/library/aa384267%28VS.85%29.aspx
		09.07-16	- Rebuild with VS2015
		14.07.16	- CreateDX11interop - release the texture not the device
		16.07.16	- Added exit flag to CleanupDirectX to avoid releasing device
					  Added immediatecontext flush to CleanupDX11
					  Restored wglDXUnregisterObjectNV to SpoutCleanup for DX9

		27.07.16	- Assembler memory copy functions for optimum speed for PBO and memoryshare
					  CopyImage instead of memcpy to support assembler functions
		16.08.16	- removed LoadTexturePixels : PBO upload, no performance advantage
		18.08.16	- moved memory copy functions to a separate "SpoutCopy" class
		20.08.16	- introduced SpoutCopy class 
					      CopyPixels with options for assembler functions
						  rgb/rgba <> bgr/bgra conversions moved from this class
		17.09.16	- removed CheckSpout2004()
					- introduced GetSpoutVersion() - returns version number for 2.005 and greater
					- restored LoadTexturePixels for staging texture data copy
					- introduced GetBufferMode and included in constructore
					- SetBufferMode is used by SpoutDXmode and check for PBO extension availability is made then

		04.01.16	- Added invert argument to UnloadTexturePixels and LoadTexturePixels
					- Finalise and test all CPU texture data access functions for DX11 and DX9
					- Included CS_OWNDC in InitOpenGL
					- Reset m_fbo etc to zero in CleanupInterop even if there is no OpenGL context
		05.01.17	- Initialize m_TextureInfo in constructor
					- Return false on fbo error in DrawToSharedMemory
					- Fixed target for texture unbind in CopyTexture
		06.01.17	- Use class variables and properly kill OpenGL window for InitOpenGL/CloseOpenGL
					- Change existing texture function names to GLDX for common selective function
		07.01.17	- Add SetCPUmode.
					- Add registry write to SetMemoryShareMode
		10.01.17	- Add FlushWait() function for Read DX11 texture
					- Add GetShareMode()
		13.01.17	- Remove interop compatibility test from GetMemoryShareMode
					- LoadExtensions : use CPU texture access if not interop compatible
					- GLDXcompatible : use memoryshare if DirectX did not load
					- UseDX9 - write result to registry
					- SetDX9 - return false if registry write failed
		15.01.17	- change GetShareMode to return : 0 - texture, 1 - cpu, 2 - memory
		22.01.17	- use DirectX texture format to create staging texture and surfaces
					- change messagebox errors to identify "SPOUT"
					- CleanupInterop after sender creation fail
					- CleanupDX9 change to prevent crash with Milkdrop
					- add pQuery->Release() to FlushWait
		04.02.17	- corrected test for fbo blit extension
		21.05.17	- Testing functions for frame sync
		02.06.17	- Registry functions called from SpoutUtils
		05.06.17	- Added IsBLITavailable, IsSWAPavailable, IsGLDXavailable
					  to access result of loadGLextensions
					- Added GLDXready to test for correct GL/DX interop function
					- Added exception catch to interop functions in LinkGLDXtextures
					- Introduced m_bUseGLDX flag instead of testing extension availability
		06.06.17	- Removed LoadExtensions from GetVerticalSync and SetVerticalSync
		07.06.17	- GLDXcompatible is called by Spout::OpenSpout to test system capability
		09.06.17	- Introduced m_SpoutVersion so that the registry is only read once
		15.06.17	- Added semaphore frame counter and IsNewFrame
		26.06.17	- Added GetSenderFps
		17.03.18	- removed gluErrorString from glError
					- More error log notices
		12.06.18	- LinkGLDXtextures
					  close interop device on failure
					  add unknown error code report
		16.06.18	- Retrieve context pointer from SpoutDirectX class
					  with spoutdx.GetImmediateContext()
					- Release context in CleanupDX11
					- Release DX11 textures with spoutdx.ReleaseDX11Texture
					- Remove unused variables g_driverType, g_featureLevel;
		18.06.18	- Unbind fbo in WriteDX11texture if not invert
					- m_pImmediateContext NULL test in all staging texture functions
		21.08.18	- changes to CopyTexture and WriteGLDXtexture
					  for source and destination texture sizes during blit
		22.08.18	- DrawToGLDXtexture : clear fbo to black before draw 
		23.08.18	- WriteGLDXtexture : allow for read from currently bound fbo
		15.09.18	- include SpoutDirectX class before SenderNames
					- move maxsenders registry read to the senderNames class
		17.10.18	- Save the global render window handle in GLDXcompatible
					- Add GetRenderWindow() to return render window handle
					- LoadGLextensions return true if already loaded
		19.10.18	- Clear fbo to black for all legacy Drawto functions
					- Add direct fbo read for WriteDX11texture, WriteDX9texture, and WriteMemory
		21.10.18	- GLDXcompatible : skip DirectX and compatibility test for Memoryshare mode
		23.10.18	- Profile performance and simplify WriteMemory and ReadMemory
		27.10.18	- GLDXcompatible reorganised : rendering context tested in LoadGLextensions
		31.10.18	- Add invert code to ReadMemory and WriteMemory
		02.11.18	- Increased PBO from 2 to 4 for higher memoryshare resolutions
					- allow for zero texture ID in UnloadTexturePixels
		12.11.18	- Always release DX9 device. Fix Milkdrop crash.
		13.11.18	- change GetShareMode/SetShareMode for memoryshare option only
		21.11.18	- Add copy extensions for future use
		23.11.18	- Set and get new sender frame for all texture functions
		24.11.18	- Release sender mutex only for successful access
		26.11.18	- Use ReleaseDX11Texture to release test texture in GLDXready
					  Remove exit flag from CleanupDirectX - directx devices always freed
					  Remove exit flag from CleanupInterop - object always released.
		29.11.18	- SetShareMode : Set memoryshare mode to 2 to allow for older applcations
					  GetShareMode : Return 2 for MemoryShare, 0 for texture
		09.12.18	- Some logging added to GLDXcompatible.
		10.12.18	- Remove redundant DX11available test from OpenDirectX
		25.12.18	- FlushWait moved to DirectX class
		03.01.19	- Changed to revised registry functions in SpoutUtils
		02.05.19	- Change GetSpoutVersion to int. 
					  Return -1 if unable to read registry. Probably not installed.
		17.05.19	- Add sender name argument to :
					      ReadTexture, ReadTexturePixels, ReadMemory, ReadMemoryPixels
					- For ReadMemory and ReadMemoryPixels, open the sender memory map
					  and close it afterwards so that the receiver does not hold a handle
					  to the map and prevent the sender from re-sizing it.
					  https://github.com/leadedge/Spout2/issues/41
		25.05.19	- Do not fail for null return of PBO mapping in LoadTexturePixels and UnloadTexturePixels
		30.05.19	- Use CopyTexture in SetSharedTextureData instead of duplicated code.
		16.06.19	- Change dash to "|" for all 2.007 fatal notices
		27.06.19	- Removed release of shared texture in CreateDX11interop
					  Texture released in SpoutDirectX class by CreateSharedDX11Texture
		10.09.19	- Corrected header file #include "SpoutFramecount.h" to "SpoutFrameCount.h"
		18.09.19	- Remove Registry write from UseDX9 and SetMemoryShareMode
		09.10.19	- Revise WriteDX9surface to use application device
					  Add SetDX9device to set application device
		13.10.19	- Add WriteTextureReadback
		20.01.20	- Changed GetGLtextureID() to GetSharedTextureID()
		27.01.20	- Write to registry for SetMemoryShareMode
		18.02.20	- Change getSharedInfo and setSharedInfo to const sendername
		              Change all g_ global to m_ class variables
		03.03.20	- Introduce DX11 keyed mutex locks in addition to named mutex
		29.04.20	- Use GL_LINEAR for all textures and blit
					  Remove isOptimus() - dll detection does not work for Windows 10
					  https://devtalk.nvidia.com/default/topic/510885/optimus-detection-c-/
					  Added missing define check for legacy OpenGL functions
		05.05.20	- Shared texture read/write timing tests documented within functions
		18.06.20	- Update WriteTexture shared texture flush and comments
		06.07.20	- Remove log notice from CleanupInterop if already closed

*/

#include "SpoutGLDXinterop.h"


spoutGLDXinterop::spoutGLDXinterop() {

	m_hWnd           = NULL;
	m_hInteropObject = NULL;
	m_hSharedMemory  = NULL;
	m_hInteropDevice = NULL;

	m_glTexture = 0; // The shared OpenGL texture linked to DirectX
	m_fbo       = 0; // Fbo used thoughput
	
	m_TexID     = 0; // A local OpenGL texture used for copy functions
	m_TexWidth  = 0; // its's width
	m_TexHeight = 0; // and height

	// Sender texture info
	m_TextureInfo.width       = 0;
	m_TextureInfo.height      = 0;
	m_TextureInfo.format      = 0;
	m_TextureInfo.partnerId   = 0;
	m_TextureInfo.usage       = 0;
	m_TextureInfo.shareHandle = 0;

	// For CreateOpenGL and CloseOpenGL
	m_hdc = NULL;
	m_hwndButton = NULL;
	m_hRc = NULL;
	
	m_pD3D       = NULL;
	m_pDevice    = NULL;
	m_dxTexture  = NULL;
	DX9format    = D3DFMT_A8R8G8B8; // default format for DX9 (21)
	
	// DX11
	m_pd3dDevice        = NULL; // DX11 device
	m_pSharedTexture    = NULL; // DX11 shared texture
	DX11format = DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9

	// OpenGL extensions
	m_caps = 0; // nothing loaded yet
	m_bBGRAavailable    = false;
	m_bExtensionsLoaded	= false;
	m_bFBOavailable     = false;
	m_bBLITavailable    = false;
	m_bPBOavailable     = false;
	m_bSWAPavailable    = false;
	m_bGLDXavailable    = false;
	m_bCOPYavailable    = false;

	// PBO extension availability is checked by SetBufferMode 
	// and when the user selects Buffering from SpoutDXmode
	// Check the mode currently in the registry
	DWORD dwMode = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffering", &dwMode)) {
		m_bPBOavailable = (dwMode == 1);
	}

	// PBO support
	PboIndex = 0;
	NextPboIndex = 0;
	m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;

	// General initialization flag
	m_bInitialized = false;

	// ===============================================================
	//            Get mode flags from the registry
	//         User can set the modes using SpoutDXmode
	//   Mode flags are modified by GLDXcompatible depending on the
	//   availability of Directx and OpenGL GL/DX interop extensions
	// ===============================================================

	m_bUseDX9    = false; // Use DX9 (true) or DX11 (default false)
	m_bUseGLDX   = true;  // Use GPU texture processing (default)
	m_bUseMemory = false; // Memoryshare
	
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);
	m_bUseDX9 = (dwDX9 == 1);

	// Memoryshare - 2.005 and greater
	// User selection of Memoryshare depends on 2.004 SpoutDirectX, 2.005 SpoutDXmode, or 2.006 SpoutSettings.
	// 2.004 apps will not have the registry set by the installer and memoryshare methods are incompatible.
	// If the hardware is not interop compatible they will fail to work.
	DWORD dwMemory = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMemory)) {
		m_bUseMemory = (dwMemory == 1);
	}

	// Disable texture processing for memoryshare
	if (m_bUseMemory)
		m_bUseGLDX = false;

	// Find the Spout version from the registry if Spout is installed (2005, 2006, etc.)
	DWORD dwVersion = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Version", &dwVersion))
		m_SpoutVersion = dwVersion; // 0 for earlier than 2.005
	else
		m_SpoutVersion = -1; // Spout not installed

}

spoutGLDXinterop::~spoutGLDXinterop() 
{
	// CleanupInterop is called by the Spout class destructor
	m_bInitialized = false;
}


//
// GLDXcompatible
//
// Hardware compatibility test
// Over-rides user selection via SpoutSettings
//
//  o LoadGLextensions
//      Checks for availability of OpenGL extensions
//      Availability can then be retrieved from this class
//
//  o OpenDirectX
//      Attempts to create a DirectX device to allow use of DirectX textures
//
//  o GLDXready
//      Checks OpenGL GL/DX extension functions
//		and creates an interop device for success
//
bool spoutGLDXinterop::GLDXcompatible()
{

	// OpenGL device context is needed
	HDC hdc = wglGetCurrentDC();
	if (!hdc) {
		SpoutLogFatal("spoutGLDXinterop::GLDXcompatible | Cannot get GL device context");
		return false;
	}

	// If OpenGL extensions fail to load quit now
	if (!LoadGLextensions()) {
		SpoutLogFatal("spoutGLDXinterop::GLDXcompatible | OpenGL extensions failed to load");
		return false;
	}

	SpoutLogNotice("spoutGLDXinterop::GLDXcompatible - testing for texture share compatibility");
	if(m_bUseGLDX) 
		SpoutLogNotice("    Texture sharing option selected");
	else
		SpoutLogNotice("    Memory sharing option selected");

	// Get a window handle for DirectX initialization
	// If not available it can be NULL
	// This is always done for all modes so save the global window handle for this class
	m_hWnd = WindowFromDC(hdc);

	// Defaults are already set by installation or by the user with SpoutSettings
	// The following tests over-ride user selection

	// Check for correct DirectX initialization
	// unless the user has selected Memoryshare mode
	if (m_bUseMemory) {
		m_bUseGLDX = false;
	}
	else {
		if (OpenDirectX(m_hWnd, m_bUseDX9)) {
			// DirectX is OK but check for availabilty of the GL/DX extensions.
			if (!IsGLDXavailable()) {
				// The extensions required for texture access are not available.
				// The user can specify memoryshare mode but even if the user
				// has not made a selection, default to Memoryshare

				// Pop out a 3 second warning if switching to memoryshare
				if (m_bUseGLDX)
					SpoutMessageBox("Warning - GL/DX extensions not available\nusing memory share mode", 3000);

				m_bUseGLDX = false;
				m_bUseMemory = true;
				SpoutLogWarning("spoutGLDXinterop::GLDXcompatible - GL/DX extensions not available");
			}
			else {
				SpoutLogNotice("    GL/DX interop extensions available");
				if (!GLDXready()) {
					// It is possible that extensions load OK but that the GL/DX interop functions fail.
					// This has been noted on dual graphics machines with the NVIDIA Optimus driver.
					// If the compatibility test fails, fall back to memoryshare
					
					// Pop out a 3 second warning if switching to memoryshare
					if (m_bUseGLDX)
						SpoutMessageBox("Warning - GL/DX interop functions failed\nusing memory share mode", 3000);

					m_bUseGLDX = false;
					m_bUseMemory = true;
					SpoutLogWarning("spoutGLDXinterop::GLDXcompatible - GL/DX interop functions failed");
				}
				else {
					SpoutLogNotice("    GL/DX interop functions working");
				}
			}
			// All passes - don't change user settings
		}
		else {
			// Failed to open DirectX - must use memoryshare
			
			// Pop out a 3 second warning if switching to memoryshare
			if (m_bUseGLDX)
				SpoutMessageBox("Warning - OpenDirectX failed\nusing memory share mode", 3000);
			
			m_bUseGLDX = false;
			m_bUseMemory = true;

			SpoutLogWarning("spoutGLDXinterop::GLDXcompatible - OpenDirectX failed");
		}

		// Over-ride SpoutSettings memory share mode
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", (DWORD)m_bUseMemory);

		// Using either GL/DX texture or memoryshare mode
		// OpenSpout will log the mode being used
		// For texture share mode, CreateInterop should not report any errors
		
	}

	return true;

} // end GLDXcompatible


// Return render window handle retrieved in GLDXcompatible
HWND spoutGLDXinterop::GetRenderWindow()
{
	return m_hWnd;
}


// For external access so that the global class variables are used
bool spoutGLDXinterop::OpenDirectX(HWND hWnd, bool bDX9)
{
	SpoutLogNotice("spoutGLDXinterop::OpenDirectX - hWnd = 0x%x, DX9 = %d", hWnd, bDX9);

	// If user requested DX9 then use it.
	if (bDX9) {
		m_bUseDX9 = true;
		return (OpenDirectX9(hWnd));
	}

	// Try to to Open DX11	
	if (OpenDirectX11()) {
		m_bUseDX9 = false; // Set to indicate intialized as DX11
		// Return here if OK
		return true;
	}
	else {
		// Try DX9 if it failed
		m_bUseDX9 = true;
		if (OpenDirectX9(hWnd))
			return true;
	}

	// DirectX did not initialize
	SpoutLogError("spoutGLDXinterop::OpenDirectX - DirectX did not initialize");
	return false;

}


// Initialize and prepare DirectX 9
bool spoutGLDXinterop::OpenDirectX9(HWND hWnd)
{
	HWND fgWnd = NULL;
	char fgwndName[MAX_PATH];

	SpoutLogNotice("spoutGLDXinterop::OpenDirectX9 - hWnd = 0x%x", hWnd);

	// Already initialized ?
	if(m_pD3D != NULL) {
		SpoutLogNotice("    Device already initialized");
		return true;
	}

	// Create a IDirect3D9Ex object if not already created
	if(!m_pD3D) {
		m_pD3D = spoutdx.CreateDX9object(); 
	}

	if(m_pD3D == NULL) {
		SpoutLogWarning("    Could not create DX9 object");
		return false;
	}

	// Create a DX9 device
	if(!m_pDevice) {
		m_pDevice = spoutdx.CreateDX9device(m_pD3D, hWnd); 
	}

	if(m_pDevice == NULL) {
		SpoutLogWarning("    Could not create DX9 device");
		return false;
	}

	// Problem for FFGL plugins - might be a problem for other FFGL hosts or applications.
	// DirectX 9 device initialization creates black areas and the host window has to be redrawn.
	// But this causes a crash for a sender in Magic when the render window size is changed.
	// Not a problem for DirectX 11.
	// Not needed in Isadora.
	// Needed for Resolume.
	// For now, limit this to Resolume only.

	fgWnd = GetForegroundWindow();
	if(fgWnd) {
		// SMTO_ABORTIFHUNG : The function returns without waiting for the time-out
		// period to elapse if the receiving thread appears to not respond or "hangs."
		if(SendMessageTimeoutA(fgWnd, WM_GETTEXT, MAX_PATH, (LPARAM)fgwndName, SMTO_ABORTIFHUNG, 128, NULL) != 0) {
			// Returns the full path - get just the window name
			PathStripPathA(fgwndName);
			if(fgwndName[0]) {
				if(strstr(fgwndName, "Resolume") != NULL // Is resolume in the window title ?
				&& strstr(fgwndName, "magic") == NULL) { // Make sure it is not a user named magic project.
					// DirectX device initialization needs the window to be redrawn (creates black areas)
					// 03.05.15 - user observation that UpDateWindow does not work and Resolume GUI is still corrupted
					// 28.08.15 - user observation of a crash with Windows 10 
					// try RedrawWindow again (with InvalidateRect as well) - confirmed working with Win 7 32bit
					// https://msdn.microsoft.com/en-us/library/windows/desktop/dd145213%28v=vs.85%29.aspx
					// The WM_PAINT message is generated by the system and should not be sent by an application.					
					// SendMessage(fgWnd, WM_PAINT, NULL, NULL ); // causes problems
					InvalidateRect(fgWnd, NULL, false); // make sure
			        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_INTERNALPAINT);
				}
			}
		}
	}

	return true;
}

// Initialize and prepare Directx 11
bool spoutGLDXinterop::OpenDirectX11()
{

	SpoutLogNotice("spoutGLDXinterop::OpenDirectX11()");

	// Quit if already initialized
	if (m_pd3dDevice != NULL) {
		SpoutLogNotice("    Device already initialized");
		return true;
	}

	// Create a DirectX 11 device
	if(!m_pd3dDevice) 
		m_pd3dDevice = spoutdx.CreateDX11device();

	if(!m_pd3dDevice)
		return false;

	// Retrieve the context pointer
	m_pImmediateContext = spoutdx.GetImmediateContext();

	SpoutLogNotice("    Device 0x%Ix : Context 0x%Ix", m_pd3dDevice, m_pImmediateContext);

	return true;
}


// Test for DX11 capability
bool spoutGLDXinterop::DX11available()
{
	// Return silently if DX11 is already initialized
	if(m_pd3dDevice != NULL) 
		return true;

	SpoutLogNotice("spoutGLDXinterop::DX11available() - testing for DirectX 11 availability");

	// Try to create a DirectX 11 device
	ID3D11Device* pd3dDevice;
	pd3dDevice = spoutdx.CreateDX11device();
	if (pd3dDevice == NULL) {
		SpoutLogNotice("    DirectX 11 is not available");
		return false;
	}

	// Release the device because this is just a test
	spoutdx.ReleaseDX11Device(pd3dDevice);
	pd3dDevice = NULL;

	SpoutLogNotice("    DirectX 11 is available");

	return true;
}


//
// Must be called after DirectX initialization, not before
//
// https://code.google.com/p/chromium/issues/detail?id=106438
//
// NOTES : On a “normal” system EnumDisplayDevices and IDXGIAdapter::GetDesc always concur
// i.e. the device that owns the head will be the device that performs the rendering. 
// On an Optimus system IDXGIAdapter::GetDesc will return whichever device has been selected for rendering.
// So on an Optimus system it is possible that IDXGIAdapter::GetDesc will return the dGPU whereas 
// EnumDisplayDevices will return the iGPU.
//
// This function compares the adapter descriptions of the two
// The string "Intel" reveals that it is an Intel device but 
// the Vendor ID could also be used
//
//	0x10DE	NVIDIA
//	0x163C	intel
//	0x8086  Intel
//	0x8087  Intel
//
bool spoutGLDXinterop::GetAdapterInfo(char* renderadapter, 
									  char* renderdescription, char* renderversion,
									  char* displaydescription, char* displayversion,
									  int maxsize, bool &bDX9)
{

	IDXGIDevice * pDXGIDevice = NULL;

	renderadapter[0] = 0; // DirectX adapter
	renderdescription[0] = 0;
	renderversion[0] = 0;
	displaydescription[0] = 0;
	displayversion[0] = 0;

	if(bDX9) {
		if(m_pDevice == NULL) {
			SpoutLogError("spoutGLDXinterop::GetAdapterInfo - no DX9 device");
			return false; 
		}

		D3DADAPTER_IDENTIFIER9 adapterinfo;
		// char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
		// char            Description[MAX_DEVICE_IDENTIFIER_STRING];
		// char            DeviceName[32];         // Device name for GDI (ex. \\.\DISPLAY1)
		// LARGE_INTEGER   DriverVersion;          // Defined for 32 bit components
		// DWORD           VendorId;
		// DWORD           DeviceId;
		// DWORD           SubSysId;
		// DWORD           Revision;
		// GUID            DeviceIdentifier;
		// DWORD           WHQLLevel;
		m_pD3D->GetAdapterIdentifier (D3DADAPTER_DEFAULT, 0, &adapterinfo);
		// printf("Driver = [%s]\n", adapterinfo.Driver);
		// printf("    Description = [%s]\n", adapterinfo.Description);
		// printf("DeviceName = [%s]\n", adapterinfo.DeviceName);
		// printf("DriverVersion = [%d] [%x]\n", adapterinfo.DriverVersion, adapterinfo.DriverVersion);
		// printf("VendorId = [%d] [%x]\n", adapterinfo.VendorId, adapterinfo.VendorId);
		// printf("DeviceId = [%d] [%x]\n", adapterinfo.DeviceId, adapterinfo.DeviceId);
		// printf("SubSysId = [%d] [%x]\n", adapterinfo.SubSysId, adapterinfo.SubSysId);
		// printf("Revision = [%d] [%x]\n", adapterinfo.Revision, adapterinfo.Revision);
		strcpy_s(renderadapter, maxsize, adapterinfo.Description);

	}
	else {
		if(m_pd3dDevice == NULL) { 
			SpoutLogError("spoutGLDXinterop::GetAdapterInfo - no DX11 device");
			return false; 
		}

		m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
		IDXGIAdapter * pDXGIAdapter;
		pDXGIDevice->GetAdapter(&pDXGIAdapter);
		DXGI_ADAPTER_DESC adapterinfo;
		pDXGIAdapter->GetDesc(&adapterinfo);
		// WCHAR Description[ 128 ];
		// UINT VendorId;
		// UINT DeviceId;
		// UINT SubSysId;
		// UINT Revision;
		// SIZE_T DedicatedVideoMemory;
		// SIZE_T DedicatedSystemMemory;
		// SIZE_T SharedSystemMemory;
		// LUID AdapterLuid;

		char output[256];
		size_t charsConverted = 0;
		wcstombs_s(&charsConverted, output, 129, adapterinfo.Description, 128);
		// printf("    Description = [%s]\n", output);
		// printf("    VendorId = [%d] [%x]\n", adapterinfo.VendorId, adapterinfo.VendorId);
		// printf("SubSysId = [%d] [%x]\n", adapterinfo.SubSysId, adapterinfo.SubSysId);
		// printf("DeviceId = [%d] [%x]\n", adapterinfo.DeviceId, adapterinfo.DeviceId);
		// printf("Revision = [%d] [%x]\n", adapterinfo.Revision, adapterinfo.Revision);
		strcpy_s(renderadapter, maxsize, output);
	}

	// DEBUG - default render adapter is the DirectX one ???
	if(renderadapter) {
		strcpy_s(renderdescription, maxsize, renderadapter);
	}

	// Use Windows functions to look for Intel graphics to see if it is
	// the same render adapter that was detected with DirectX
	char driverdescription[256];
	char driverversion[256];
	char regkey[256];
	size_t charsConverted = 0;
	
	// Additional info
	DISPLAY_DEVICE DisplayDevice;
	DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

	// 31.10.14 detect the adapter attached to the desktop
	// To query all display devices in the current session, 
	// call this function in a loop, starting with iDevNum set to 0, 
	// and incrementing iDevNum until the function fails. 
	// To select all display devices in the desktop, use only the display devices
	// that have the DISPLAY_DEVICE_ATTACHED_TO_DESKTOP flag in the DISPLAY_DEVICE structure.

	int nDevices = 0;
	for(int i=0; i<10; i++) { // should be much less than 10 adapters
		if(EnumDisplayDevices(NULL, i, &DisplayDevice, 0)) {
			// This will list all the devices
			nDevices++;
			// Get the registry key
			wcstombs_s( &charsConverted, regkey, 129, (const wchar_t *)DisplayDevice.DeviceKey, 128);
			// This is the registry key with all the information about the adapter
			OpenDeviceKey(regkey, 256, driverdescription, driverversion);
			// Is it a render adapter ?
			if(renderadapter && strcmp(driverdescription, renderadapter) == 0) {
				// printf("Windows render adapter matches : [%s] Vers [%s]\n", driverdescription, driverversion);
				strcpy_s(renderdescription, maxsize, driverdescription);
				strcpy_s(renderversion, maxsize, driverversion);
			}
			// Is it a display adapter
			if(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
				// printf("Display adapter : [%s] Vers: %s ", driverdescription, driverversion);
				strcpy_s(displaydescription, 256, driverdescription);
				strcpy_s(displayversion, 256, driverversion);
				// printf("(Attached to desktop)\n");
			} // endif attached to desktop

		} // endif EnumDisplayDevices
	} // end search loop

	// The render adapter
	if(renderdescription) trim(renderdescription);

	// 16-06-18
	if (pDXGIDevice) pDXGIDevice->Release();

	return true;
}

// Main function to create a sender or receiver and
// set up the GL/DX interop functions for texture sharing
bool spoutGLDXinterop::CreateInterop(HWND hWnd, const char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	bool bRet = true;
	DWORD format;

	// Needs an openGL context to work
	if(!wglGetCurrentContext()) {
		SpoutLogFatal("spoutGLDXinterop::CreateInterop | no GL context");
		return false;
	}

	//
	// Texture format tests
	//
	// DX9 compatible formats
	// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
	// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
	//
	// Other formats that work with DX11 but not with DX9
	// DXGI_FORMAT_R16G16B16A16_FLOAT
	// DXGI_FORMAT_R16G16B16A16_SNORM
	// DXGI_FORMAT_R10G10B10A2_UNORM
	//
	// To change any of these you can use :
	//
	// void spoutGLDXinterop::SetDX11format(DXGI_FORMAT textureformat)
	//
	// Allow for compatible DirectX 11 senders (format 87)
	// And compatible DirectX9 senders D3DFMT_X8R8G8B8 - 22
	// and the default D3DFMT_A8R8G8B8 - 21
	if(m_bUseDX9) {
		// DirectX 9
		if(dwFormat > 0) {
			if(dwFormat == 87) {
				format = (DWORD)D3DFMT_A8R8G8B8; // (21)
			}
			else if(dwFormat == D3DFMT_X8R8G8B8 || dwFormat == D3DFMT_A8R8G8B8) {
				format = (DWORD)dwFormat; // (22)
			}
			else {
				SpoutLogError("spoutGLDXinterop::CreateInterop - incompatible user format (%d)", dwFormat);
				return false;
			}
			SetDX9format((D3DFORMAT)format); // Set the global texture format
		}
		else { // format is passed as zero so we assume a DX9 sender D3DFMT_A8R8G8B8
			format = (DWORD)DX9format;
		}
	}
	else {
		// TODO : Is this a DX11 or a DX9 sender texture?
		// A directX 11 receiver accepts DX9 formats
		if(!bReceive && dwFormat > 0) {
			format = (DXGI_FORMAT)dwFormat;
			SetDX11format((DXGI_FORMAT)format); // Set the global texture format
		}
		else {
			// TODO : compatible format tests
			// printf("CreateInterop - Sender format %d - receiving format %d\n", dwFormat, (DWORD)DX11format);
			// format = dwFormat; // Sender format
			format = (DWORD)DX11format; // DXGI_FORMAT_B8G8R8A8_UNORM (87) default compatible with DX9
		}
	}

	// Quit now if the receiver can't access the shared memory info of the sender
	// Otherwise m_dxShareHandle is set by getSharedTextureInfo and is the
	// shared texture handle of the Sender texture
	if (bReceive && !getSharedTextureInfo(sendername)) {
		SpoutLogFatal("spoutGLDXinterop::CreateInterop | Cannot retrieve sender information");
		return false;
	}

	SpoutLogNotice("spoutGLDXinterop::CreateInterop [%s] %dx%d - DX format %d", sendername, width, height, format);

	// TODO - move this section above ?
	// Check the sender format for a DX9 receiver
	// It can only be from a DX9 sender (format 0, 22, 21)
	// or from a compatible DX11 sender (format 87)
	if(bReceive && m_bUseDX9) {
		if(!(m_TextureInfo.format == 0 
			|| m_TextureInfo.format == 22
			|| m_TextureInfo.format == 21
			|| m_TextureInfo.format == 87)) {
			SpoutLogFatal("spoutGLDXinterop::CreateInterop | Incompatible sender texture format %d", m_TextureInfo.format);
			return false;
		}
	}

	// Make sure DirectX has been initialized
	// Creates a global pointer to the DirectX device (DX11 m_pd3dDevice or DX9 m_pDevice)
	if(!OpenDirectX(hWnd, m_bUseDX9)) {
		SpoutLogFatal("spoutGLDXinterop::CreateInterop | Cannot open DirectX");
		return false;
	}

	// Create an fbo for copying textures
	if(m_fbo > 0) {
		// Delete the fbo before the texture so that any texture attachment is released
		// SpoutLogNotice("spoutGLDXinterop::CreateInterop - re-creating fbo");
		glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;
	}
	// else SpoutLogNotice("spoutGLDXinterop::CreateInterop - creating fbo");
	glGenFramebuffersEXT(1, &m_fbo); 

	// Create a local opengl texture that will be linked to a shared DirectX texture
	// This is never initialized using OpenGL, but has size and can be accessed when
	// it is linked to the shared DirectX texture with the GL/DX Interop.
	if(m_glTexture) {
		glDeleteTextures(1, &m_glTexture);
		m_glTexture = 0;
	}
	glGenTextures(1, &m_glTexture);

	// Create textures and GL/DX interop objects for texture access
	if(m_bUseDX9)
		bRet = CreateDX9interop(width, height, format, bReceive);
	else
		bRet = CreateDX11interop(width, height, format, bReceive);

	if(!bRet) {
		CleanupInterop(); // release everything for this class
		SpoutLogFatal("spoutGLDXinterop::CreateInterop | Cannot create DirectX/OpenGL interop");
		return false;
	}

	// Now the global shared texture handle (m_dxShareHandle)
	// has been set so a sender can be created.
	// This creates the sender shared memory map and registers the sender
	if (!bReceive) {
		// We are done with the format
		// So for DirectX 9, set to zero to identify the sender as DirectX 9
		// Allow the sender format to be registered becasue it is tested
		// by SpoutPanel and by the texture formats above
		if(!senders.CreateSender(sendername, width, height, m_dxShareHandle, format)) {
			CleanupInterop(); // 22.01.17 - Release
			SpoutLogFatal("spoutGLDXinterop::CreateInterop | Cannot create Spout sender");
			return false;
		}
	}

	// Set up local values for this instance
	// Needed for texture read and write size checks
	m_TextureInfo.width       = (unsigned __int32)width;
	m_TextureInfo.height      = (unsigned __int32)height;
#ifdef _M_X64
	m_TextureInfo.shareHandle = (unsigned __int32)(HandleToLong(m_dxShareHandle));
#else
	m_TextureInfo.shareHandle = (unsigned __int32)m_dxShareHandle;
#endif
	// Additional unused fields available
	// DWORD usage; // texture usage
	m_TextureInfo.usage = 0;
	// wchar_t description[128]; // Wyhon compatible description
	// 26.08.15 - set the executable path to the sender's shared info 
	// (not documented and could be removed)
	// unsigned __int32 partnerId; // Wyphon id of partner that shared it with us
	m_TextureInfo.partnerId = 0;

	if(!bReceive) {
		SharedTextureInfo info;
		// Access the info directly from the memory map to include the description string
		if(senders.getSharedInfo(sendername, &info)) {
			char exepath[256];
			GetModuleFileNameA(NULL, exepath, sizeof(exepath));
			// Description is defined as wide chars, but the path is stored as byte chars
			strcpy_s((char*)info.description, 256, exepath);
			info.partnerId = 0; // initialize
			senders.setSharedInfo(sendername, &info);
		}
	}

	// Initialize a texture transfer sync mutex either sender or receiver can do this.
	// Memoryshare does not need one because the shared memory mutex lock is used.
	frame.CreateAccessMutex(sendername);

	//
	// Now we have globals for this instance
	//
	// m_TextureInfo.width			- width of the shared DirectX texture
	// m_TextureInfo.height			- height of the shared DirectX texture
	// m_TextureInfo.shareHandle	- handle of the shared DirectX texture
	// m_TextureInfo.format			- format of the shared DirectX texture
	// m_TextureInfo.usage			- unused
	// m_TextureInfo.partnerId		- unused
	// m_TextureInfo.description    - path of the executable that created the sender

	// m_glTexture					- a linked opengl texture
	// m_dxTexture					- a linked, shared DirectX texture created GL/DX mode
	// m_hInteropDevice				- handle to interop device created by wglDXOpenDeviceNV by init
	// m_hInteropObject				- handle to the connected texture created by wglDXRegisterObjectNV
	// m_bInitialized				- whether it initialized OK

	// true means the init was OK, not the connection
	SpoutLogNotice("    CreateInterop successful");

	return true; 

}

//
// =================== DX9 ===============================
//
//      CreateDX9interop()
//	
// bReceive : when receiving a texture from a DX application this must be set to true (default)
//            when sending a texture from GL to the DX application, set to false
//
bool spoutGLDXinterop::CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive) 
{

	// printf("CreateDX9interop(%dx%d, [Format = %d], %d (m_pDevice = %x)\n", width, height, dwFormat, bReceive, m_pDevice);

	// The shared texture handle of the Sender texture "m_dxShareHandle" 
	// is already set by getSharedTextureInfo, but should be NULL for a sender
	if (!bReceive) {
		// Create a new shared DirectX resource m_dxTexture 
		// with new local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture
	}

	// Safety in case an application has crashed
	if (m_dxTexture != NULL) {
		m_dxTexture->Release();
	}
	m_dxTexture = NULL;

	// Create a shared DirectX9 texture - m_dxTexture
	// by giving it a sharehandle variable - m_dxShareHandle
	// For a SENDER : the sharehandle is NULL and a new texture is created
	// For a RECEIVER : the sharehandle is valid and becomes a handle to the existing shared texture
	// USAGE is D3DUSAGE_RENDERTARGET
	if (!spoutdx.CreateSharedDX9Texture(m_pDevice,
		width,
		height,
		(D3DFORMAT)dwFormat,  // default is D3DFMT_A8R8G8B8
		m_dxTexture,
		m_dxShareHandle)) {
		// printf("    CreateSharedDX9Texture failed\n");
		return false;
	}

	// For the GL/DX interop, link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	// When a sender size changes, the new texture has to be re-registered
	if (m_hInteropDevice != NULL && m_hInteropObject != NULL) {
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}
	m_hInteropObject = LinkGLDXtextures(m_pDevice, m_dxTexture, m_dxShareHandle, m_glTexture);
	if (!m_hInteropObject) {
		// printf("    DX9 LinkGLDXtextures failed\n");
		return false;
	}

	return true;
}

//
// =================== DX11 ==============================
//
bool spoutGLDXinterop::CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive ) 
{

	// Create or use a shared DirectX texture that will be linked
	// to the OpenGL texture and get it's share handle for sharing textures
	if (bReceive) {
		// Retrieve the shared texture pointer via the sharehandle
		if(!spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
			SpoutLogError("spoutGLDXinterop::CreateDX11interop error - device = 0X%x, sharehandle = 0X%x", m_pd3dDevice, m_dxShareHandle);
			return false;
		}
	} else {
		// otherwise create a new shared DirectX resource m_pSharedTexture 
		// with local handle m_dxShareHandle for a sender
		m_dxShareHandle = NULL; // A sender creates a new texture with a new share handle
		if (!spoutdx.CreateSharedDX11Texture(m_pd3dDevice,
			width, height,
			(DXGI_FORMAT)dwFormat, // default is DXGI_FORMAT_B8G8R8A8_UNORM
			&m_pSharedTexture, m_dxShareHandle)) {
			SpoutLogError("spoutGLDXinterop::CreateDX11interop - CreateSharedDX11Texture failed");
			return false;
		}
	}
	// For the GL/DX interop, link the shared DirectX texture to the OpenGL texture
	// This registers for interop and associates the opengl texture with the dx texture
	// by calling wglDXRegisterObjectNV which returns a handle to the interop object
	// (the shared texture) (m_hInteropObject)
	// When a sender size changes, the new texture has to be re-registered
	if(m_hInteropDevice != NULL &&  m_hInteropObject != NULL) {
		SpoutLogNotice("spoutGLDXinterop::CreateDX11interop - LinkGLDXtextures - unregistering interop");
		wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject);
		m_hInteropObject = NULL;
	}
	m_hInteropObject = LinkGLDXtextures(m_pd3dDevice, m_pSharedTexture, m_dxShareHandle, m_glTexture); 
	if(!m_hInteropObject) {
		SpoutLogError("spoutGLDXinterop::CreateDX11interop - LinkGLDXtextures failed");
		return false;
	}

	SpoutLogNotice("spoutGLDXinterop::CreateDX11interop - LinkGLDXtextures : m_hInteropObject = %x", m_hInteropObject);


	return true;

}

//
//	Link a shared DirectX texture to an OpenGL texture
//	and create a GLDX interop object handle
//
//	IN	pSharedTexture  Pointer to shared the DirectX texture
//	IN	dxShareHandle   Handle of the DirectX texture to be shared
//	IN	glTextureID     ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returns             Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGLDXinterop::LinkGLDXtextures (	void* pDXdevice,
											void* pSharedTexture,
											HANDLE dxShareHandle,
											GLuint glTexture) 
{

	HANDLE hInteropObject = NULL;
	BOOL bResult = 0;
	DWORD dwError = 0;
	char tmp[128];

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if(!m_hInteropDevice) {
		try {
			m_hInteropDevice = wglDXOpenDeviceNV(pDXdevice);
		}
		catch (...) {
			SpoutLogError("spoutGLDXinterop::LinkGLDXtextures - wglDXOpenDeviceNV failed");
			return NULL;
		}
	}

	if (m_hInteropDevice == NULL) {
		dwError = GetLastError();
		sprintf_s(tmp, 128, "spoutGLDXinterop::LinkGLDXtextures - wglDXOpenDeviceNV error\n");
		switch (dwError) {
			case ERROR_OPEN_FAILED:
				strcat_s(tmp, 128, "    Could not open the Direct3D device.");
				break;
			case ERROR_NOT_SUPPORTED:
				strcat_s(tmp, 128, "    The <dxDevice> is not supported.");
				break;
		}
		SpoutLogError("%s", tmp);
		return NULL;
	}

	// prepare shared resource
	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	try {
		bResult = wglDXSetResourceShareHandleNV(pSharedTexture, dxShareHandle);
	}
	catch (...) {
		SpoutLogError("spoutGLDXinterop::LinkGLDXtextures - wglDXSetResourceShareHandleNV failed");
		return NULL;
	}

	if(!bResult) {
		SpoutLogError("spoutGLDXinterop::LinkGLDXtextures - wglDXSetResourceShareHandleNV error");
		return NULL;
	}

	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	// Returns a handle that can be used for sharing functions
	try {
		hInteropObject = wglDXRegisterObjectNV(m_hInteropDevice,
										pSharedTexture,	// DX texture
										glTexture,		// OpenGL texture
										GL_TEXTURE_2D,	// Must be TEXTURE_2D - multisampling not supported
										WGL_ACCESS_READ_WRITE_NV); // We will write and the receiver will read
	}
	catch (...) {
		SpoutLogError("spoutGLDXinterop::LinkGLDXtextures - wglDXRegisterObjectNV failed");
		return NULL;
	}

	if (!hInteropObject) {
		dwError = GetLastError();
		// What is c007006e returned on failure ?
		// printf("LinkGLDXtextures error = %x\n", (unsigned int)dwError);
		sprintf_s(tmp, 128, "spoutGLDXinterop::LinkGLDXtextures - wglDXRegisterObjectNV :error\n");
		switch (dwError) {
			case ERROR_INVALID_HANDLE :
				strcat_s(tmp, 128, "    No GL context is made current to the calling thread.");
				break;
			case ERROR_INVALID_DATA :
				strcat_s(tmp, 128, "    Incorrect <name> <type> or <access>	parameters.");
				break;
			case ERROR_OPEN_FAILED :
				strcat_s(tmp, 128, "    Opening the Direct3D resource failed.");
				break;
			default :
				strcat_s(tmp, 128, "    Unknown error.");
				break;
		}
		SpoutLogError("%s", tmp);

		if (m_hInteropDevice != NULL) {
			wglDXCloseDeviceNV(m_hInteropDevice);
			m_hInteropDevice = NULL;
		}

	}

	return hInteropObject;

}

//
// Test whether the NVIDIA OpenGL/DirectX interop extensions function correctly. 
// Creates dummy textures and uses the interop functions.
// Creates an interop device on success.
// Must be called after OpenDirectX.
// Failure means fall back to Memoryshare mode
// Success means the GLDX interop functions can be used.
// Other errors should not happen if OpenDirectX succeeded
bool spoutGLDXinterop::GLDXready()
{
	HANDLE	dxShareHandle = NULL; // Shared texture handle for a sender texture
	LPDIRECT3DTEXTURE9  dxTexture = NULL; // the shared DX9 texture
	ID3D11Texture2D* pSharedTexture = NULL; // the shared DX11 texture
	HANDLE hInteropObject = NULL; // handle to the DX/GL interop object
	GLuint glTexture = 0; // the OpenGL texture linked to the shared DX texture

	// Create an opengl texture for the test
	glGenTextures(1, &glTexture);
	if (glTexture == 0) {
		SpoutLogError("spoutGLDXinterop::GLDXready - glGenTextures failed");
		return false;
	}

	SpoutLogNotice("spoutGLDXinterop::GLDXready - testing GL/DX interop functions");
	//
	// Create a directX texture and link using the NVIDIA GLDX interop functions
	//
	if (GetDX9()) {
		if (m_pDevice == NULL) {
			glDeleteTextures(1, &glTexture);
			SpoutLogError("spoutGLDXinterop::GLDXready (DX9) - No D3D9ex device");
			return false;
		}

		SpoutLogNotice("    Creating test DX9 texture");

		// Create a shared DirectX9 texture for the test
		if (!spoutdx.CreateSharedDX9Texture(m_pDevice,
											256, 256,
											D3DFMT_A8R8G8B8, // default
											dxTexture, dxShareHandle)) {
			glDeleteTextures(1, &glTexture);
			SpoutLogError("spoutGLDXinterop::GLDXready (DX9) - CreateSharedDX9Texture failed");
			return false;
		}

		SpoutLogNotice("    Linking test DX9 texture");

		// Link the shared DirectX9 texture to the OpenGL texture
		// If sucessful, LinkGLDXtextures initializes a class handle
		// to a GL/DirectX interop device - m_hInteropDevice
		hInteropObject = LinkGLDXtextures(m_pDevice, dxTexture, dxShareHandle, glTexture);
		if (hInteropObject == NULL) {
			SpoutLogError("spoutGLDXinterop::GLDXready (DX9) - LinkGLDXtextures failed");
			dxTexture->Release();
			glDeleteTextures(1, &glTexture);
			return false;
		}
		SpoutLogNotice("    Test DX9 texture created and linked OK");

		if (m_hInteropDevice && hInteropObject) 
			wglDXUnregisterObjectNV(m_hInteropDevice, hInteropObject);
		if (dxTexture) 
			dxTexture->Release();
		if (glTexture) 
			glDeleteTextures(1, &glTexture);

	} // endif DX9
	else {

		if (m_pd3dDevice == NULL) {
			glDeleteTextures(1, &glTexture);
			SpoutLogError("spoutGLDXinterop::GLDXready (DX11) - No D3D11 device");
			return false;
		}

		SpoutLogNotice("    Creating test DX11 texture");

		// Create a new shared DirectX resource
		if (!spoutdx.CreateSharedDX11Texture(m_pd3dDevice,
											256, 256,
											DXGI_FORMAT_B8G8R8A8_UNORM, // default
											&pSharedTexture, dxShareHandle)) {
			glDeleteTextures(1, &glTexture);
			SpoutLogError("spoutGLDXinterop::GLDXready (DX11) - CreateSharedDX11Texture failed");
			return false;
		}

		SpoutLogNotice("    Linking test DX11 texture");

		// Link the shared DirectX texture to the OpenGL texture
		// If sucessful, LinkGLDXtextures initializes a class handle
		// to a GL/DirectX interop device - m_hInteropDevice
		hInteropObject = LinkGLDXtextures(m_pd3dDevice, pSharedTexture, dxShareHandle, glTexture);
		if (!hInteropObject) {
			spoutdx.ReleaseDX11Texture(m_pd3dDevice, pSharedTexture);
			SpoutLogError("spoutGLDXinterop::GLDXready (DX11) - LinkGLDXtextures failed");
			return false;
		}

		SpoutLogNotice("    Test DX11 texture created and linked OK");

		// All passes, so unregister and release textures
		// m_hInteropDevice remains and does not need to be created again
		if(m_hInteropDevice && hInteropObject) 
			wglDXUnregisterObjectNV(m_hInteropDevice, hInteropObject);

		spoutdx.ReleaseDX11Texture(m_pd3dDevice, pSharedTexture);

		if(glTexture) 
			glDeleteTextures(1, &glTexture);

	}

	return true;

}

void spoutGLDXinterop::CleanupDirectX()
{
	if (m_bUseDX9)
		CleanupDX9();
	else
		CleanupDX11();

}

void spoutGLDXinterop::CleanupDX9()
{
	// 04.10.19 - do not block if m_pD3D exists
	SpoutLogNotice("spoutGLDXinterop::CleanupDX9()");

	// 01.09.14 - texture release was missing for a receiver - caused a VRAM leak
	// If an existing texture exists, CreateTexture can fail with and "unknown error"
	// 25.08.15 - moved before release of device
	if (m_dxTexture != NULL) {
		m_dxTexture->Release();
		m_dxTexture = NULL;
	}

	// 25.08.15 - release device before the object !
	// 22.01.17 - will crash if refcount is 1 for MilkDrop.
	// 12.11.18 - must always be freed, not only on exit. Fix for MilkDop.
	//            Device recreated for a new sender.
	// 08.10.19 - do not release device if DX9 object does not exist
	if (m_pD3D != NULL) {
		if (m_pDevice != NULL)
			m_pDevice->Release();
		m_pD3D->Release();
	}

	m_pDevice = NULL;
	m_pD3D = NULL;

}

void spoutGLDXinterop::CleanupDX11()
{
	if (m_pd3dDevice != NULL) {

		SpoutLogNotice("spoutGLDXinterop::CleanupDX11()");

		unsigned long refcount = 0;

		if (m_pd3dDevice) {

			if (m_pSharedTexture)
				refcount = spoutdx.ReleaseDX11Texture(m_pd3dDevice, m_pSharedTexture);

			// Important to set pointer to NULL or it will crash if released again
			m_pSharedTexture = nullptr;

			// 12.11.18 - To avoid memory leak with dynamic objects
			//            must always be freed, not only on exit.
			//            Device recreated for a new sender.
			refcount += spoutdx.ReleaseDX11Device(m_pd3dDevice);

			if (refcount > 0)
				SpoutLogWarning("CleanupDX11:CleanupDX11() - refcount = %d", refcount);
		}

		// NULL the pointers
		m_pImmediateContext = NULL;
		m_pd3dDevice = NULL;

	}
}


// this is the function that cleans up Direct3D and the gldx interop
// 26.11.18 - removed exit flag fix.
void spoutGLDXinterop::CleanupInterop()
{

	// 04.10.19 - Release OpenGL objects etc. even if DX9 has been released

	// Skip if already done
	if (m_hInteropDevice == NULL && m_hInteropObject == NULL
		&& m_fbo == 0 && m_pbo[0] == 0 
		&& m_glTexture == 0 && m_TexID == 0
		&& m_pSharedTexture == NULL && m_dxTexture == NULL
		&& m_bInitialized == false) {
		// SpoutLogNotice("spoutGLDXinterop::CleanupInterop - already closed");
		return;
	}

	// These things need an opengl context so check
	if (wglGetCurrentContext()) {

		SpoutLogNotice("spoutGLDXinterop::CleanupInterop");

		if (m_hInteropDevice != NULL && m_hInteropObject != NULL) {
			if (!wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject)) {
				SpoutLogWarning("spoutGLDXinterop::CleanupInterop - could not un-register interop");
			}
			m_hInteropObject = NULL;
		}

		if (m_hInteropDevice != NULL) {
			if (!wglDXCloseDeviceNV(m_hInteropDevice)) {
				SpoutLogWarning("spoutGLDXinterop::CleanupInterop - could not close interop");
			}
			m_hInteropDevice = NULL;
		}

		if (m_fbo > 0) {
			// Delete the fbo before the texture so that any texture attachment 
			// is released even though it should have been
			glDeleteFramebuffersEXT(1, &m_fbo);
			m_fbo = 0;
		}

		if (m_pbo[0] > 0) {
			glDeleteBuffersEXT(4, m_pbo);
			m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
		}

		if (m_glTexture > 0) {
			glDeleteTextures(1, &m_glTexture);
			m_glTexture = 0;
		}

		if (m_TexID > 0) {
			glDeleteTextures(1, &m_TexID);
			m_TexID = 0;
			m_TexWidth = 0;
			m_TexHeight = 0;
		}

	} // endif there is an opengl context
	else {
		SpoutLogWarning("spoutGLDXinterop::CleanupInterop - no OpenGL context");
	}

	// Close directX and free resources
	CleanupDirectX();

	// Clean up sender frame counting semaphore
	frame.CleanupFrameCount();

	// Close texture access mutex
	frame.CloseAccessMutex();

	m_bInitialized = false;

}

//
//	Load the Nvidia gl/dx extensions
//
bool spoutGLDXinterop::LoadGLextensions() 
{
	// Return silently if already loaded
	if (m_caps > 0)
		return true;

	m_caps = loadGLextensions(); // in spoutGLextensions

	if(m_caps == 0) {
		SpoutLogError("spoutGLDXinterop::LoadGLextensions failed");
		return false;
	}

	// GLEXT_SUPPORT_PBO - set by SetBufferMode()
	if(m_caps & GLEXT_SUPPORT_NVINTEROP) m_bGLDXavailable = true; // Interop needed for texture sharing
	if(m_caps & GLEXT_SUPPORT_FBO)       m_bFBOavailable  = true;
	if(m_caps & GLEXT_SUPPORT_FBO_BLIT)  m_bBLITavailable = true;
	if(m_caps & GLEXT_SUPPORT_SWAP)      m_bSWAPavailable = true;
	if(m_caps & GLEXT_SUPPORT_BGRA)      m_bBGRAavailable = true;
	if(m_caps & GLEXT_SUPPORT_COPY)      m_bCOPYavailable = true;

	 // FBO not available is terminal
	if (!m_bFBOavailable) {
		SpoutLogError("spoutGLDXinterop::LoadGLextensions - no FBO extensions available");
		return false;
	}
	
	m_bExtensionsLoaded = true;

	return true;
}

bool spoutGLDXinterop::IsBGRAavailable()
{
	return m_bBGRAavailable;
}

bool spoutGLDXinterop::IsPBOavailable()
{
	return m_bPBOavailable;
}

bool spoutGLDXinterop::IsBLITavailable()
{
	return m_bBLITavailable;
}

bool spoutGLDXinterop::IsSWAPavailable()
{
	return m_bSWAPavailable;
}

bool spoutGLDXinterop::IsGLDXavailable()
{
	return m_bGLDXavailable;
}

bool spoutGLDXinterop::IsCOPYavailable()
{
	return m_bCOPYavailable;
}

// Switch pbo functions on or off (default is off).
// Will fail silently if extensions are not loaded
// or PBO functions are not supported
void spoutGLDXinterop::SetBufferMode(bool bActive)
{
	if(m_bExtensionsLoaded) {
		if(bActive) {
			if(m_caps & GLEXT_SUPPORT_PBO) {
				m_bPBOavailable = true;
			}
		}
		else {
			m_bPBOavailable = false;
		}
		// Write to the registry now - this function is called by SpoutDXmode when it starts
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffering", (DWORD)m_bPBOavailable);
	}

}

bool spoutGLDXinterop::GetBufferMode()
{
	DWORD dwMode = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffering", &dwMode);
	return (dwMode == 1);
	
}


// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedTextureInfo(const char* sharedMemoryName) {

	unsigned int w, h;
	HANDLE handle;
	DWORD format;
	char name[256];
	strcpy_s(name, 256, sharedMemoryName);

	if (!senders.FindSender(name, w, h, handle, format)) {
		SpoutLogWarning("spoutGLDXinterop::getSharedTextureInfo - can't find sender [%s]", sharedMemoryName);
		return false;
	}

	m_dxShareHandle = (HANDLE)handle;
	m_TextureInfo.width = w;
	m_TextureInfo.height = h;
#ifdef _M_X64
	m_TextureInfo.shareHandle = (unsigned __int32)(HandleToLong(handle));
#else
	m_TextureInfo.shareHandle = (unsigned __int32)handle;
#endif
	// m_TextureInfo.shareHandle = (__int32)handle;
	m_TextureInfo.format = format;

	return true;

}


// Set texture info to shared memory for the sender init
// width and height must have been set first
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::setSharedTextureInfo(const char* sharedMemoryName) {

	return senders.UpdateSender(sharedMemoryName, 
							m_TextureInfo.width,
							m_TextureInfo.height,
							m_dxShareHandle,
							m_TextureInfo.format);


}

// Return current sharing handle, width and height of a Sender
// Note - use the map directly - we must not use getSharedTextureInfo
// which resets the local info structure from shared memory !!!
// A receiver checks this all the time so it has to be compact
// 03.09.14 - MB mods for names map class
bool spoutGLDXinterop::getSharedInfo(const char* sharedMemoryName, SharedTextureInfo* info) 
{
	return senders.getSharedInfo(sharedMemoryName, info);
}


// Sets the given info structure to shared memory with the given name
// IMPORTANT: this modifies the local structure
// Used to change the texture dimensions before init
bool spoutGLDXinterop::setSharedInfo(const char* sharedMemoryName, SharedTextureInfo* info)
{
	m_TextureInfo.width			= info->width;
	m_TextureInfo.height		= info->height;
#ifdef _M_X64
	m_dxShareHandle = (HANDLE)(LongToHandle((long)info->shareHandle));
#else
	m_dxShareHandle = (HANDLE)info->shareHandle;
#endif	
	// the local info structure handle "m_TextureInfo.shareHandle" gets converted 
	// into (unsigned __int32) from "m_dxShareHandle" by setSharedTextureInfo
	if(setSharedTextureInfo(sharedMemoryName)) {
		return true;
	}
	else {
		SpoutLogError("spoutGLDXinterop::setSharedInfo failed");
		return false;
	}
}


bool spoutGLDXinterop::WriteTexture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	bool bRet = false;

	// Zero texture is supported for all sharing modes
	// A texture must be attached to the Host FBO attachment point 0 for read
	if (m_bUseMemory) { // Memoryshare
		bRet = WriteMemory(TextureID, TextureTarget, width, height, bInvert, HostFBO);
	}
	else if (m_bUseGLDX) { // GL/DX interop
		bRet = WriteGLDXtexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
	}
	else {
		SpoutLogError("spoutGLDXinterop::WriteTexture failed");
		bRet = false;
	}

	return bRet;

} // end WriteTexture


bool spoutGLDXinterop::ReadTexture (const char* sendername,
									GLuint TextureID, GLuint TextureTarget,
									unsigned int width, unsigned int height,
									bool bInvert, GLuint HostFBO)
{
	if(m_bUseMemory) { // Memoryshare
		return(ReadMemory(sendername, TextureID, TextureTarget, width, height, bInvert, HostFBO));
	}
	else if(m_bUseGLDX) { // GL/DX interop
		return(ReadGLDXtexture(TextureID, TextureTarget, width, height, bInvert, HostFBO));
	}
	else {
		SpoutLogError("spoutGLDXinterop::ReadTexture failed");
		return false;
	}
} // end ReadTexture


bool spoutGLDXinterop::WriteTexturePixels (const unsigned char* pixels, 
										   unsigned int width, unsigned int height, 
										   GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	bool bRet = false;

	if(m_bUseMemory) { // Memoryshare
		bRet = WriteMemoryPixels(pixels, width, height, glFormat, bInvert);
	}
	else if(m_bUseGLDX) { // GL/DX interop
		bRet = WriteGLDXpixels(pixels, width, height, glFormat, bInvert, HostFBO);
	}
	else {
		SpoutLogError("spoutGLDXinterop::WriteTexturePixels failed");
		return false;
	}

	return bRet;

} // end WriteTexturePixels


bool spoutGLDXinterop::ReadTexturePixels (const char* sendername, 
										  unsigned char* pixels,
										  unsigned int width, unsigned int height, 
										  GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	if(m_bUseMemory) { // Memoryshare
		return(ReadMemoryPixels(sendername, pixels, width, height, glFormat, bInvert));
	}
	else if(m_bUseGLDX) { // GL/DX interop
		return(ReadGLDXpixels(pixels, width, height, glFormat, bInvert, HostFBO));
	}
	else {
		SpoutLogError("spoutGLDXinterop::ReadTexturePixels failed");
		return false;
	}
} // end ReadTexturePixels


//
// BIND THE SHARED TEXTURE
//
// for use within an application - this locks the interop object and binds the shared texture
// Locks remain in place, so afterwards a call to UnbindSharedTexture MUST be called
//
bool spoutGLDXinterop::BindSharedTexture()
{
	// Only for GL/DX interop mode
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	bool bRet = false;
	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// lock dx object
		if(LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			// Leave interop and mutex both locked for success
			bRet = true;
		}
		else {
			// Release interop lock and allow texture access for fail
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			// Release mutex and allow access to the texture
			frame.AllowTextureAccess(m_pSharedTexture);
			bRet = false;
		}
	}

	return bRet;

} // end BindSharedTexture


//
// UNBIND THE SHARED TEXTURE
//
// for use within an application
// this unbinds the shared texture,
// unlocks the interop object and releases the mutex
// Must be used after BindSharedTexture success but not for fail
//
bool spoutGLDXinterop::UnBindSharedTexture()
{
	// Only for GL/DX interop mode
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;
	
	// Unbind our shared OpenGL texture
	glBindTexture(GL_TEXTURE_2D,0);
	// unlock dx object
	UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
	// Release mutex and allow access to the texture
	// frame.AllowAccess();
	frame.AllowTextureAccess(m_pSharedTexture);
	
	return true;

} // end UnBindSharedTexture

GLuint spoutGLDXinterop::GetSharedTextureID()
{
	return m_glTexture;
}

// ----------------------------------------------------------
//		Access to texture using DX/GL interop functions
// ----------------------------------------------------------


//
// COPY AN OPENGL TEXTURE TO THE SHARED OPENGL TEXTURE
//
// Allows for a texture attached to the host fbo
// Where the input texture can be larger than the shared texture
// and Width and height are the used portion. Only the used part is copied.
// For example Freeframe textures.
//
bool spoutGLDXinterop::WriteGLDXtexture (
	GLuint TextureID,
	GLuint TextureTarget,
	unsigned int width,
	unsigned int height,
	bool bInvert,
	GLuint HostFBO)
{
	// Only for GL/DX interop mode
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	// Specify greater here because the width/height passed can be smaller
	if (width > m_TextureInfo.width || height > m_TextureInfo.height)
		return false;

	// Total time approximately 450-500 microseconds
	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// lock dx interop object
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Write to the shared texture
			if (SetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO)) {
				// Increment the sender frame counter for successful write
				frame.SetNewFrame();
			}
			// unlock dx object
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;

} // end WriteGLDXTexture

//
bool spoutGLDXinterop::SetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	GLenum status = 0;
	bool bRet = false;

	// "TextureID" can be NULL if it is attached to the host fbo
	// m_fbo is a local FBO
	// "m_glTexture" is destination texture
	// width/height are the dimensions of the destination texture
	// Because two fbos are used, the input texture can be larger than the shared texture
	// Width and height are the used portion and only the used part is copied

	if (TextureID == 0 && HostFBO > 0 && glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
		
		// Enter with the input texture attached to
		// attachment point 0 of the currently bound fbo
		// and set for read or read/write

		// Bind our local fbo for draw
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
		// Draw to the first attachment point
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		// Attach the texture we write into (the shared texture)
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
		// Check fbo for completeness
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
			if (m_bBLITavailable) {
				if (bInvert)
					// copy one texture buffer to the other while flipping upside down 
					glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				else
					// Do not flip during blit
					glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
			else {
				// No fbo blit extension
				// Copy from the host fbo (input texture attached)
				// to the shared texture
				glBindTexture(GL_TEXTURE_2D, m_glTexture);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			bRet = true;
		}
		else {
			PrintFBOstatus(status);
			bRet = false;
		}
		// restore the host fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
	}
	else if (TextureID > 0) {
		// There is a valid texture passed in.
		// Copy the input texture to the destination texture.
		// Both textures must be the same size.
		bRet = CopyTexture(TextureID, TextureTarget, m_glTexture, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
	}

	return bRet;

}


bool spoutGLDXinterop::ReadGLDXtexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// Total time approximately 450-500 microseconds

	// No interop, no copy
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		return false;
	}

	// width and height must be the same as the shared texture
	// m_TextureInfo is established in CreateInterop
	if (width != (unsigned int)m_TextureInfo.width || height != (unsigned int)m_TextureInfo.height) {
		return false;
	}

	bool bRet = true; // Error only if texture read fails

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {

		// Read the shared texture if the sender has produced a new frame
		if (frame.GetNewFrame()) {
			if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
				bRet = GetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO);
				UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			}
		}

		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;

} // end ReadGLDXTexture


// Copy shared texture via fbo blit
bool spoutGLDXinterop::GetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// 80-90 microseconds

	GLenum status = 0;
	bool bRet = false;

	// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	// Attach the Input texture (the shared texture) to the color buffer in our frame buffer - note texturetarget 
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Attach target texture (the one we write into and return) to second attachment point
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, TextureTarget, TextureID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status == GL_FRAMEBUFFER_COMPLETE_EXT) {
		if(m_bBLITavailable) {
			// Flip if the user wants that
			if(bInvert) {
				// copy one texture buffer to the other while flipping upside down
				glBlitFramebufferEXT(0,     0,		// srcX0, srcY0, 
									 width, height, // srcX1, srcY1
									 0,     height,	// dstX0, dstY0,
									 width, 0,		// dstX1, dstY1,
									 GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
			else { 
				// Do not flip during blit
				glBlitFramebufferEXT(0, 0,			// srcX0, srcY0, 
									 width, height,	// srcX1, srcY1
									 0, 0,			// dstX0, dstY0,
									 width, height,	// dstX1, dstY1,
									 GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}
		else { 
			// No fbo blit extension available
			// Copy from the fbo (shared texture attached) to the dest texture
			glBindTexture(TextureTarget, TextureID);
			glCopyTexSubImage2D(TextureTarget, 0, 0, 0, 0, 0, width, height);
			glBindTexture(TextureTarget, 0);
		}
		bRet = true;
	}
	else {
		PrintFBOstatus(status);
		bRet = false;
	}

	// restore the previous fbo - default is 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); // 04.01.16
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
	
	return bRet;

}

//
// COPY IMAGE PIXELS TO THE SHARED TEXTURE
//
bool spoutGLDXinterop::WriteGLDXpixels(const unsigned char* pixels, 
	                                      unsigned int width, 
										  unsigned int height, 
										  GLenum glFormat,
										  bool bInvert,
										  GLuint HostFBO)
{
	if(width != m_TextureInfo.width || height != m_TextureInfo.height || !pixels)
		return false;

	// Use a GL texture so that WriteTexture can be used
	GLenum glformat = glFormat;

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);

	// Transfer the pixels to the local texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if(IsPBOavailable()) {
		LoadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, (const unsigned char*)pixels, glFormat);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, m_TexID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glformat, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// Write the local texture to the shared texture and invert if necessary
	WriteTexture(m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);

	return true;

} // end WriteGLDXpixels


//
// COPY THE SHARED TEXTURE TO IMAGE PIXELS
//
bool spoutGLDXinterop::ReadGLDXpixels(unsigned char* pixels, 
										 unsigned int width, 
										 unsigned int height, 
										 GLenum glFormat,
										 bool bInvert, 
										 GLuint HostFBO)
{
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	if(width != m_TextureInfo.width || height != m_TextureInfo.height)
		return false;

	GLenum status;
	GLenum glformat = glFormat;
	bool bRet = true; // Error only if pixel read fails

	// retrieve opengl texture data directly to image pixels

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// read texture for a new frame
		if (frame.GetNewFrame()) {
			// lock gl/dx interop object
			if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
				// Set single pixel alignment in case of rgb source
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				// Create or resize a local OpenGL texture
				CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);
				// Copy the shared texture to the local texture, inverting if necessary
				CopyTexture(m_glTexture, GL_TEXTURE_2D, m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
				// Extract the pixels from the local texture - changing to the user passed format
				if (IsPBOavailable()) { // PBO method
					bRet = UnloadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, pixels, glFormat, false, HostFBO);
				}
				else { // fbo attachment method - current fbo has to be passed in
					// Bind our local fbo
					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
					// Attach the local rgba texture to the color buffer in our frame buffer
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_TexID, 0);
					status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
					if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
						// read the pixels from the framebuffer in the user provided format
						glReadPixels(0, 0, width, height, glformat, GL_UNSIGNED_BYTE, pixels);
						bRet = true;
					}
					else {
						PrintFBOstatus(status);
						bRet = false;
					}
					// restore the previous fbo - default is 0
					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
				}
				// default alignment
				glPixelStorei(GL_PACK_ALIGNMENT, 4);
				// Unlock interop object
				UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			} // interop lock failed
		} // no new frame
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	} // mutex access failed

	return bRet;

} // end ReadGLDXpixels 


//
// Set the DX9 device externally
//
// The sender shared texture is created using this device so it must be DX9ex.
// Direct3D 10.0, Direct3D 9c, and older Direct3D runtimes do not support shared surfaces.
// See: https://docs.microsoft.com/en-us/windows/win32/direct3darticles/surface-sharing-between-windows-graphics-apis
//
bool spoutGLDXinterop::SetDX9device(IDirect3DDevice9Ex* pDevice)
{
	if (!GetDX9()) {
		SpoutLogError("spoutGLDXinterop::SetDX9device - only for DX9 mode");
		return false;
	}

	// The Spout DX9 object is not used if the device is set externally
	if (m_pD3D) {
		m_pD3D->Release();
		// If set externally, the device is also released externally,
		// so it must not be released in this class.
		// The Spout DX9 device can be released here because
		// it will not be released again if m_pD3D is NULL
		if (m_pDevice) 
			m_pDevice->Release();
		m_pD3D = NULL;
		m_pDevice = NULL;
	}

	SpoutLogNotice("spoutGLDXinterop::SetDX9device (%x)", pDevice);

	// Already initialized ?
	if (pDevice != NULL && m_pDevice == pDevice) {
		SpoutLogWarning("SetDX9device - Device (%x) already initialized", pDevice);
	}

	// Set the Spout DX9 device to the application device
	m_pDevice = pDevice;

	return true;
}

//
// Write a DirectX 9 system memory surface to the shared texture (sizes must be the same)
//
bool spoutGLDXinterop::WriteDX9memory(LPDIRECT3DSURFACE9 source_surface)
{
	// Only for DX9 mode
	if (!source_surface || !GetDX9())
		return false;

	if (frame.CheckTextureAccess()) {
		if (spoutdx.WriteDX9memory(m_pDevice, source_surface, m_dxTexture)) {
			frame.SetNewFrame();
			frame.AllowTextureAccess();
			return true;
		}
		frame.AllowTextureAccess();
	}

	return false;

} // end WriteDX9memory


//
// COPY FROM A GPU DX9 SURFACE TO THE SHARED DX9 TEXTURE
//
// Surface and shared texture must have been created with the same device
// See : SetDX9device
//
bool spoutGLDXinterop::WriteDX9surface(IDirect3DDevice9Ex* pDevice, LPDIRECT3DSURFACE9 surface)
{
	// Only for DX9 mode
	if (!GetDX9()) {
		SpoutLogError("spoutGLDXinterop::WriteDX9surface - only for DX9 mode");
		return false;
	}

	if (!surface) {
		SpoutLogError("spoutGLDXinterop::WriteDX9surface - null surface");
		return false;
	}

	if (!pDevice) {
		SpoutLogError("spoutGLDXinterop::WriteDX9surface - null device");
		return false;
	}

	// The device must match
	if (pDevice != m_pDevice) {
		SpoutLogError("spoutGLDXinterop::WriteDX9surface - invalid device\n    Use SetDX9device");
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess()) {
		// Write the surface to the shared texture
		if (spoutdx.WriteDX9surface(m_pDevice, surface, m_dxTexture)) {
			// The necessary flush and wait done by WriteDX9surface
			// Increment the sender frame counter
			frame.SetNewFrame();
			// Release mutex and allow access to the texture
			frame.AllowTextureAccess();
			return true;
		}
		frame.AllowTextureAccess();
	}

	return false;
}


//
// DX11 versions - https://github.com/DashW/Spout2
//

//
// COPY FROM THE SHARED DX11 TEXTURE TO A DX11 TEXTURE
//
bool spoutGLDXinterop::ReadTexture(ID3D11Texture2D** texture)
{
	// Only for DX11 mode
	if(!texture || !*texture || GetDX9() || !m_pImmediateContext)
		return false;

	D3D11_TEXTURE2D_DESC desc = { 0 };
	(*texture)->GetDesc(&desc);
	if(desc.Width != (unsigned int)m_TextureInfo.width || desc.Height != (unsigned int)m_TextureInfo.height) {
		return false;
	}
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Copy the shared texture if the sender has produced a new frame
		if (frame.GetNewFrame()) {
			m_pImmediateContext->CopyResource(*texture, m_pSharedTexture);
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;

} // end ReadTexture

//
// COPY A DX11 TEXTURE TO THE SHARED DX11 TEXTURE
//
bool spoutGLDXinterop::WriteTexture(ID3D11Texture2D** texture)
{
	// Only for DX11 mode
	if (!texture || GetDX9() || !m_pImmediateContext) {
		SpoutLogWarning("spoutGLDXinterop::WriteTexture(ID3D11Texture2D** texture) failed");
		if (GetDX9())
			SpoutLogWarning("    only for DX11");
		if (!texture)
			SpoutLogWarning("    ID3D11Texture2D** NULL");
		if (!m_pImmediateContext)
			SpoutLogVerbose("    pImmediateContext NULL");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc = { 0 };

	(*texture)->GetDesc(&desc);
	if (desc.Width != m_TextureInfo.width || desc.Height != m_TextureInfo.height) {
		SpoutLogWarning("spoutGLDXinterop::WriteTexture(ID3D11Texture2D** texture) sizes do not match");
		SpoutLogWarning("    texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_TextureInfo.width, m_TextureInfo.height);
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		m_pImmediateContext->CopyResource(m_pSharedTexture, *texture);
		// Flush after update of the shared texture on this device
		m_pImmediateContext->Flush();
		// Optionally wait until the flush and copy complete
		// TODO : testing required 
		// spoutdx.hWait(m_pd3dDevice, m_pImmediateContext);
		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
		bRet = true;
	}

	return bRet;
}

//
// COPY A DX11 TEXTURE TO THE SHARED DX11 TEXTURE
// COPY THE LINKED OPENGL TEXTURE BACK TO AN OPENGL TEXTURE
//
bool spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture,
											GLuint TextureID, GLuint TextureTarget,
											unsigned int width, unsigned int height,
											bool bInvert, GLuint HostFBO)	
{
	// Only for DX11 mode
	if (!texture || GetDX9() || !m_pImmediateContext) {
		SpoutLogWarning("spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture) failed");
		if (GetDX9())
			SpoutLogWarning("    only for DX11");
		if (!texture)
			SpoutLogWarning("    ID3D11Texture2D** NULL");
		if (!m_pImmediateContext)
			SpoutLogVerbose("    pImmediateContext NULL");
		return false;
	}

	if (m_hInteropDevice == NULL || m_hInteropObject == NULL) {
		SpoutLogWarning("spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture) no interop device");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc = { 0 };

	(*texture)->GetDesc(&desc);
	if (desc.Width != m_TextureInfo.width || desc.Height != m_TextureInfo.height) {
		SpoutLogWarning("spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture) sizes do not match");
		SpoutLogWarning("    texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_TextureInfo.width, m_TextureInfo.height);
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		bRet = true;
		// Copy the DirectX texture to the shared texture
		m_pImmediateContext->CopyResource(m_pSharedTexture, *texture);
		// Flush after update of the shared texture on this device
		m_pImmediateContext->Flush();
		// Copy the linked OpenGL texture back to the user texture
		if (width != m_TextureInfo.width || height != m_TextureInfo.height) {
			SpoutLogWarning("spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture) sizes do not match");
			SpoutLogWarning("    OpenGL texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_TextureInfo.width, m_TextureInfo.height);
			bRet = false;
		}
		else if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			bRet = GetSharedTextureData(TextureID, TextureTarget, width, height, bInvert, HostFBO);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			if(!bRet)
				SpoutLogWarning("spoutGLDXinterop::WriteTextureReadback(ID3D11Texture2D** texture) readback failed");
		}

		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;
}



//
// COPY IMAGE PIXELS TO A TEXTURE
//
//
// Streaming Texture Upload
//
// From : http://www.songho.ca/opengl/gl_pbo.html
//
// No FBO used so none has to be passed
//
bool spoutGLDXinterop::LoadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
										 unsigned int width, unsigned int height, 
										 const unsigned char* data, 
										 GLenum glFormat, bool bInvert)
{
	void *pboMemory = NULL;
	int channels = 4; // RGBA or RGB

	if(TextureID == 0 || data == NULL)
		return false;

	if(glFormat == GL_RGB || glFormat == GL_BGR_EXT) 
		channels = 3;

	if(m_fbo == 0) {
		SpoutLogNotice("spoutGLDXinterop::LoadTexturePixels - creating FBO");
		glGenFramebuffersEXT(1, &m_fbo);
	}

	// Create pbos if not already
	if (m_pbo[0] == 0 || m_pbo[1] == 0) {
		SpoutLogNotice("spoutGLDXinterop::LoadTexturePixels - creating PBO");
		glGenBuffersEXT(4, m_pbo);
	}

	PboIndex = (PboIndex + 1) % 4;
	NextPboIndex = (PboIndex + 1) % 4;

	// Bind the texture and PBO
	glBindTexture(TextureTarget, TextureID);
	glBindBufferEXT(GL_PIXEL_UNPACK_BUFFER, m_pbo[PboIndex]);

	// Copy pixels from PBO to the texture - use offset instead of pointer.
	glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, 0);

	// Bind PBO to update the texture
	glBindBufferEXT(GL_PIXEL_UNPACK_BUFFER, m_pbo[NextPboIndex]);

	// Call glBufferData() with a NULL pointer to clear the PBO data and avoid a stall.
	glBufferDataEXT(GL_PIXEL_UNPACK_BUFFER, width*height*channels, 0, GL_STREAM_DRAW);

	// Map the buffer object into client's memory
	pboMemory = (void *)glMapBufferEXT(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	// glMapBuffer can return NULL when called the first time
	// when the next pbo has not been filled with data yet
	glGetError(); // remove the last error

	if(pboMemory) {
		// Update data directly to the mapped buffer
		spoutcopy.CopyPixels((const unsigned char*)data, (unsigned char*)pboMemory, width, height, glFormat, bInvert);
		glUnmapBufferEXT(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
	}
	// Skip the copy rather than return false.

	// Release PBOs
	glBindBufferEXT(GL_PIXEL_UNPACK_BUFFER, 0);
	
	return true;

}

//
// Asynchronous Read-back from a texture
//
// Adapted from : http://www.songho.ca/opengl/gl_pbo.html
//
bool spoutGLDXinterop::UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget, 
										   unsigned int width, unsigned int height, 
										   unsigned char* data, GLenum glFormat, 
										   bool bInvert, GLuint HostFBO)
{
	void *pboMemory = NULL;
	int channels = 4; // RGBA or RGB

	if (data == NULL) {
		return false;
	}

	if (glFormat == GL_RGB || glFormat == GL_BGR_EXT) {
		channels = 3;
	}

	if (m_fbo == 0) {
		SpoutLogNotice("spoutGLDXinterop::UnloadTexturePixels - creating fbo");
		glGenFramebuffersEXT(1, &m_fbo);
	}

	// Create pbos if not already
	if(m_pbo[0] == 0 || m_pbo[1] == 0) {
		SpoutLogNotice("spoutGLDXinterop::UnloadTexturePixels - creating PBO");
		glGenBuffersEXT(4, m_pbo);
	}

	PboIndex = (PboIndex + 1) % 4;
	NextPboIndex = (PboIndex + 1) % 4;

	// If Texture ID is zero, the texture is already attached to the Host Fbo
	// and we do nothing. If not we need to crate an fbo and attach the user texture
	if (TextureID > 0) {
		// Attach the texture to point 0
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
		// Set the target framebuffer to read
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	}
	else if (HostFBO == 0) {
		// If no texture ID, a Host FBO must be provided
		return false;
	}

	// Bind the PBO
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Null existing data to avoid a stall
	glBufferDataEXT(GL_PIXEL_PACK_BUFFER, width*height*channels, 0, GL_STREAM_READ);

	// Read pixels from framebuffer to PBO - glReadPixels() should return immediately.
	glReadPixels(0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, (GLvoid *)0);

	// If there is data in the next pbo from the previous call, read it back
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);

	// Map the PBO to process its data by CPU
	pboMemory = glMapBufferEXT(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	// glMapBuffer can return NULL when called the first time
	// when the next pbo has not been filled with data yet
	glGetError(); // remove the last error

	if (pboMemory) {
		// Update data directly from the mapped buffer
		spoutcopy.CopyPixels((const unsigned char*)pboMemory, (unsigned char*)data, width, height, glFormat, bInvert);
		glUnmapBufferEXT(GL_PIXEL_PACK_BUFFER);
	}
	// skip the copy rather than return false.

	// Back to conventional pixel operation
	glBindBufferEXT(GL_PIXEL_PACK_BUFFER, 0);
	
	// Restore the previous fbo binding
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

}


//
//	GL/DX Interop lock
//
//	A return value of S_OK indicates that all objects were
//    successfully locked.  Other return values indicate an
//    error. If the function returns false, none of the objects will be locked.
//
//	Attempting to access an interop object via GL when the object is
//    not locked, or attempting to access the DirectX resource through
//    the DirectX API when it is locked by GL, will result in undefined
//    behavior and may result in data corruption or program
//    termination. Likewise, passing invalid interop device or object
//    handles to this function has undefined results, including program
//    termination.
//
//	Note that only one GL context may hold the lock on the
//    resource at any given time --- concurrent access from multiple GL
//    contexts is not currently supported.
//
//	http://halogenica.net/sharing-resources-between-directx-and-opengl/
//
//	This lock triggers the GPU to perform the necessary flushing and stalling
//	to guarantee that the surface has finished being written to before reading from it. 
//
//	DISCUSSION: The Lock/Unlock calls serve as synchronization points
//    between OpenGL and DirectX. They ensure that any rendering
//    operations that affect the resource on one driver are complete
//    before the other driver takes ownership of it.
//
//	This function assumes only one object
//
//	Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGLDXinterop::LockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	// 180 microseconds

	// lock dx object
	if(wglDXLockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_BUSY :			// One or more of the objects in <hObjects> was already locked.
				hr = E_ACCESSDENIED;	// General access denied error
				SpoutLogError("spoutGLDXinterop::LockInteropObject - ERROR_BUSY");
				break;
			case ERROR_INVALID_DATA :	// One or more of the objects in <hObjects>
										// does not belong to the interop device
										// specified by <hDevice>.
				hr = E_ABORT;			// Operation aborted
				SpoutLogError("spoutGLDXinterop::LockInteropObject - ERROR_INVALID_DATA");
				break;
			case ERROR_LOCK_FAILED :	// One or more of the objects in <hObjects> failed to 
				hr = E_ABORT;			// Operation aborted
				SpoutLogError("spoutGLDXinterop::LockInteropObject - ERROR_LOCK_FAILED");
				break;
			default:
				hr = E_FAIL;			// unspecified error
				SpoutLogError("spoutGLDXinterop::LockInteropObject - UNKNOWN_ERROR");
				break;
		} // end switch
	} // end false

	return hr;

} // LockInteropObject


//
// Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGLDXinterop::UnlockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	DWORD dwError;
	HRESULT hr;

	if(hDevice == NULL || hObject == NULL || *hObject == NULL) {
		return E_HANDLE;
	}

	// 180 microseconds

	if (wglDXUnlockObjectsNV(hDevice, 1, hObject) == TRUE) {
		return S_OK;
	}
	else {
		dwError = GetLastError();
		switch (dwError) {
			case ERROR_NOT_LOCKED :
				hr = E_ACCESSDENIED;
				SpoutLogError("spoutGLDXinterop::UnLockInteropObject - ERROR_NOT_LOCKED");
				break;
			case ERROR_INVALID_DATA :
				SpoutLogError("spoutGLDXinterop::UnLockInteropObject - ERROR_INVALID_DATA");
				hr = E_ABORT;
				break;
			case ERROR_LOCK_FAILED :
				hr = E_ABORT;
				SpoutLogError("spoutGLDXinterop::UnLockInteropObject - ERROR_LOCK_FAILED");
				break;
			default:
				hr = E_FAIL;
				SpoutLogError("spoutGLDXinterop::UnLockInteropObject - UNKNOWN_ERROR");
				break;
		} // end switch
	} // end fail

	return hr;

} // end UnlockInteropObject



// Set DX9 off or not with a DX11 compatibility check
// Application only - does not affect user settings
// Returns false if failed due to DX11 compatibility check
bool spoutGLDXinterop::UseDX9(bool bDX9)
{
	bool bRet = false;

	if(bDX9 == true) {
		// Request to set to DirectX 9
		// DirectX 11 is the default but is checked by OpenDirectX.
		m_bUseDX9 = true;
		bRet = true;
	}
	// Check for DirectX 11 availability if the user requested it
	else if(DX11available()) {
		m_bUseDX9 = false;
		bRet = true;
	}
	else {
		// Set to use DirectX 9 if DirectX 11 is not available
		SpoutLogError("spoutGLDXinterop::UseDX9 - DX11 not available, using DX9");
		m_bUseDX9 = true;
		bRet = false;
	}

	return bRet;
}

// Check support for DirectX 11 and return DX9 setting
// Used to test for DX11 in case it failed to initialize
bool spoutGLDXinterop::isDX9()
{
	if(!DX11available()) {
		SpoutLogWarning("spoutGLDXinterop::isDX9() - DX11 not available using DX9");
		m_bUseDX9 = true;
	}
	// Otherwise return what has been set
	// This can be checked again after directX initialization
	// to find out if DirectX 11 initialization failed
	return m_bUseDX9;
}

// Set DX9 mode for all applications
bool spoutGLDXinterop::SetDX9(bool bDX9)
{
	if(WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", (DWORD)m_bUseDX9)) {
		m_bUseDX9 = bDX9;
		return true;
	}
	return false;
}

// Return current DX9 mode
bool spoutGLDXinterop::GetDX9()
{
	return m_bUseDX9;
}

bool spoutGLDXinterop::GetMemoryShareMode()
{
	bool bRet = false;
	DWORD dwMem = 0;
	if(ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem)) {
		bRet = (dwMem == 1);
	}	
	return bRet;
}

// Set memoryshare mode for all applications
bool spoutGLDXinterop::SetMemoryShareMode(bool bMem)
{
	m_bUseMemory = bMem;
	if (WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", (DWORD)bMem))
		return true;
	return false;
}

//
// Return sharing mode set by user or by an application
// Reads the registry - avoid repeated use every frame.
// 0 - texture, 1 - CPU (disabled for 2.007) 2 - memory
// Left for compatibility with 2.006 applications.
// May be removed for future versions and replaced by GetMemoryShareMode().
//
int spoutGLDXinterop::GetShareMode()
{
	DWORD dwCPU = 0;

	if(GetMemoryShareMode()) {
			return 2;
	}

	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwCPU)) {
		// In case 2.006 SpoutDXmode mode has been used
		if (dwCPU == 1)
			return 1;
	}

	return 0;

}


//---------------------------------------------------------
// 0 - texture, 1 & 2 - memory
// Left for compatibility with 2.006 applications.
// May be removed for future versions and replaced by SetMemoryShareMode().
bool spoutGLDXinterop::SetShareMode(int mode)
{
	switch (mode) {

		case 1 : // CPU mode (disabled for 2.007)
		case 2 : // Shared memory
			if (WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 1)) {
				if(WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 0)) {
					m_bUseGLDX = false;
					m_bUseMemory = true;
					return true;
				}
			}
			break;

		default : // GL/DX texture
			if(WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 0)) {
				m_bUseGLDX = true;
				m_bUseMemory = false;
				return true;
			}
			break;
	}

	return false;
}


void spoutGLDXinterop::SetDX11format(DXGI_FORMAT textureformat)
{
	DX11format = textureformat;
}

void spoutGLDXinterop::SetDX9format(D3DFORMAT textureformat)
{
	DX9format = textureformat;
}

// Set graphics adapter for Spout output
bool spoutGLDXinterop::SetAdapter(int index) 
{
	if(spoutdx.SetAdapter(index)) {
		return true;
	}

	SpoutLogError("spoutGLDXinterop::SetAdapter(%d) failed", index);
	spoutdx.SetAdapter(-1); // make sure globals are reset to default

	return false;
}

// Get current adapter index
int spoutGLDXinterop::GetAdapter() 
{
	return spoutdx.GetAdapter();
}


// Get the path of the host that produced the sender
// from the description string in the sender info memory map
// Description is defined as wide chars, but the path is stored as byte chars
// Not a permanent thing - just for testing.
// The description string could be used for other things in future
bool spoutGLDXinterop::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	SharedTextureInfo info;
	int n;

	if (!senders.getSharedInfo(sendername, &info)) {
		// Just quit if the key does not exist
		SpoutLogWarning("spoutGLDXinterop::GetHostPath - could not get sender info [%s]", sendername);
		return false;
	}

	n = maxchars;
	if(n > 256) n = 256; // maximum field width in shared memory

	strcpy_s(hostpath, n, (char*)info.description);

	return true;
}

// Get the number of graphics adapters in the system
int spoutGLDXinterop::GetNumAdapters()
{
	return spoutdx.GetNumAdapters();
}

// Get an adapter name
bool spoutGLDXinterop::GetAdapterName(int index, char* adaptername, int maxchars)
{
	return spoutdx.GetAdapterName(index, adaptername, maxchars);
}


// Needs OpenGL context
int spoutGLDXinterop::GetVerticalSync()
{
	// needed for both sender and receiver
	if(m_bSWAPavailable) {
		return(wglGetSwapIntervalEXT());
	}
	return 0;
}


bool spoutGLDXinterop::SetVerticalSync(bool bSync)
{
	if(m_bSWAPavailable) {
		if(bSync)
			wglSwapIntervalEXT(1); // lock to monitor vsync
		else
			wglSwapIntervalEXT(0); // unlock from monitor vsync
		return true;
	}
	return false;
}


// Get Spout version from the registry if the key exists
// Set by the Spout installer for 2.005 and greater
int spoutGLDXinterop::GetSpoutVersion()
{
	// Version number is found at class initialization
	// Integer number 2005, 2006, 2007 etc.
	// 0 for earlier than 2.005
	return m_SpoutVersion; 
}

bool spoutGLDXinterop::GLerror() {
	GLenum err = GL_NO_ERROR;
	bool bError = false;
	while ((err = glGetError()) != GL_NO_ERROR) {
		SpoutLogError("    GLerror - OpenGL error = %d (0x%x)", err, err);
		bError = true;
		// gluErrorString needs glu32.lib
		// printf("GL error = %d (0x%x) %s\n", err, err, gluErrorString(err));
	}
	return bError;
}	

void spoutGLDXinterop::PrintFBOstatus(GLenum status)
{
	char tmp[256];
	sprintf_s(tmp, 256,"FBO status error %d (0x%x) - ", status, status);
	if (status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_UNSUPPORTED_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT - width-height problems?");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
	else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
	// else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
	// 	strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT\n");
	else 
		strcat_s(tmp, 256, "Unknown Code");
	SpoutLogError("%s", tmp);
	GLerror();
}
	

// =======================================================
//               2.005 Memoryshare functions
// =======================================================

//
// Write user texture pixel data to shared memory
// rgba textures only
//
bool spoutGLDXinterop::WriteMemory(GLuint TexID,
	GLuint TextureTarget,
	unsigned int width,
	unsigned int height,
	bool bInvert,
	GLuint HostFBO)
{
	bool bRet = false;

	unsigned char* pBuffer = memoryshare.LockSenderMemory();

	if (!pBuffer)
		return false;
	
	if (m_bPBOavailable)
		bRet = UnloadTexturePixels(TexID, TextureTarget, width, height, pBuffer, GL_RGBA, bInvert, HostFBO);
	else
		bRet = ReadTexturePixels(TexID, TextureTarget, width, height, pBuffer, GL_RGBA, bInvert, HostFBO);

	// For successful write, increment the sender
	// frame counter while the buffer is locked
	if(bRet) 
		frame.SetNewFrame();

	memoryshare.UnlockSenderMemory();

	return bRet;
}


//
// COPY A TEXTURE DATA TO A PIXEL BUFFER VIA FBO
//
bool spoutGLDXinterop::ReadTexturePixels(
	GLuint SourceID,
	GLuint SourceTarget,
	unsigned int width,
	unsigned int height,
	unsigned char* dest,
	GLenum GLformat,
	bool bInvert,
	GLuint HostFBO)
{
	GLenum status;

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);

	// Create a local fbo if not already
	if (m_fbo == 0) {
		SpoutLogNotice("spoutGLDXinterop::ReadTexturePixels - creating fbo");
		glGenFramebuffersEXT(1, &m_fbo);
	}

	// If texture ID is zero, assume the source texture is attached
	// to the host fbo which is bound for read and write
	if (SourceID == 0 && HostFBO > 0) {
		// Bind our local fbo for draw only
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
		// Source texture is already attached to point 0 for read
	}
	else {
		// bind the local fbo for read and write
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// Set to read from attachment point 0
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		// Attach the Source texture to point 0 for read
		glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
	}

	// Set to draw to attachment point 1
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	// Attach the texture we write into (the local texture) to attachment point 1
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_TexID, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {

		if (bInvert && m_bBLITavailable) {
			// copy the source texture (0) to the local texture (1) while flipping upside down 
			glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// Bind local fbo for read
			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo);
			// Read from attachment point 1
			glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
			// Read pixels from it
			glReadPixels(0, 0, width, height, GLformat, GL_UNSIGNED_BYTE, (GLvoid *)dest);
		}
		else {
			// No invert or no fbo blit extension
			// Read from the source texture attachment point 0
			// This will either be the Host fbo if texture ID is zero
			// or the local fbo if a texture ID was passed in
			glReadPixels(0, 0, width, height, GLformat, GL_UNSIGNED_BYTE, (GLvoid *)dest);
		}
	}
	else {
		PrintFBOstatus(status);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

} // end ReadTextureData


//
// Read shared memory to texture pixel data - rgba textures only
// Open the sender memory map and close it afterwards for a read,
// so that the receiver does not retain a handle to the shared memory
//
bool spoutGLDXinterop::ReadMemory(const char* sendername,
								  GLuint TexID,
								  GLuint TextureTarget,
								  unsigned int width,
								  unsigned int height,
								  bool bInvert,
								  GLuint HostFBO)
{
	if (!memoryshare.OpenSenderMemory(sendername))
		return false;
	
	unsigned char* pBuffer = memoryshare.LockSenderMemory();

	if (!pBuffer) {
		memoryshare.CloseSenderMemory();
		return false;
	}

	bool bRet = true; // Error only if pixel read fails

	// Query a new frame and read pixels while the buffer is locked
	if (frame.GetNewFrame()) {
		// Copy the rgba memory map pixels to the user's rgba opengl texture
		if (IsPBOavailable()) {
			bRet = LoadTexturePixels(TexID, TextureTarget, width, height, (const unsigned char*)pBuffer, GL_RGBA, bInvert);
		}
		else {
			if (bInvert) {
				// Create or resize a local OpenGL texture
				CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);
				// Read the memory pixels into it
				glBindTexture(GL_TEXTURE_2D, m_TexID);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
				glBindTexture(GL_TEXTURE_2D, 0);
				// Copy to the user texture, inverting at the same time
				bRet = CopyTexture(m_TexID, GL_TEXTURE_2D, TexID, TextureTarget, width, height, true, HostFBO);
			}
			else {
				// No invert - copy memory pixels directly to the user texture
				glBindTexture(TextureTarget, TexID);
				glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
				glBindTexture(TextureTarget, 0);
			}
		} // PBO not available
	} // No new frame

	memoryshare.UnlockSenderMemory();

	// Close the memory map handle so the sender can close the map
	memoryshare.CloseSenderMemory();
	
	return bRet;

}

//
// Write image pixels to shared memory
// rgba, bgra, rgb, bgr source buffers supported
//
bool spoutGLDXinterop::WriteMemoryPixels(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{

	if (!pixels)
		return false;

	unsigned char* pBuffer = memoryshare.LockSenderMemory();

	if(!pBuffer)
		return false;

	// Write pixels to shared memory
	if(glFormat == GL_RGBA) {
		spoutcopy.CopyPixels(pixels, pBuffer, width, height, GL_RGBA, bInvert);
	}
	else if(glFormat == 0x80E1) { // GL_BGRA_EXT if supported
		spoutcopy.bgra2rgba((void *)pixels, (void *)pBuffer, width, height, bInvert);
	}
	else if(glFormat == 0x80E0) { // GL_BGR_EXT if supported
		spoutcopy.bgr2rgba((void *)pixels, (void *)pBuffer, width, height, bInvert);
	}
	else if(glFormat == GL_RGB) {
		spoutcopy.rgb2rgba((void *)pixels, (void *)pBuffer, width, height, bInvert);
	}

	// Increment the sender frame counter while the buffer is locked
	frame.SetNewFrame();

	memoryshare.UnlockSenderMemory();

	return true;

}


//
// Read shared memory to image pixels
// rgba, bgra, rgb, bgr destination buffers supported
// Most efficient if the receiving buffer is rgba
// Invert currently not used.
// Open the sender memory map and close it afterwards for a read,
// so that the receiver does not retain a handle to the shared memory.
//
bool spoutGLDXinterop::ReadMemoryPixels(const char* sendername, unsigned char* pixels, 
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!pixels)
		return false;

	if (!memoryshare.OpenSenderMemory(sendername))
		return false;

	unsigned char* pBuffer = memoryshare.LockSenderMemory();
	if (!pBuffer) {
		memoryshare.CloseSenderMemory();
		// SpoutLogWarning("spoutGLDXinterop::ReadMemoryPixels - no buffer lock");
		return false;
	}

	// Query a new frame and read pixels while the buffer is locked
	if (frame.GetNewFrame()) {
		// Read pixels from shared memory
		if (glFormat == GL_RGBA) {
			spoutcopy.CopyPixels(pBuffer, pixels, width, height, GL_RGBA, bInvert);
		}
		else if (glFormat == 0x80E1) { // GL_BGRA_EXT if supported
			spoutcopy.rgba2bgra((void *)pBuffer, (void *)pixels, width, height, bInvert);
		}
		else if (glFormat == 0x80E0) { // GL_BGR_EXT if supported
			spoutcopy.rgba2bgr((void *)pBuffer, (void *)pixels, width, height, bInvert);
		}
		else if (glFormat == GL_RGB) {
			spoutcopy.rgba2rgb((void *)pBuffer, (void *)pixels, width, height, bInvert);
		}
	}

	memoryshare.UnlockSenderMemory();

	// Close the memory map handle so the sender can close the map
	memoryshare.CloseSenderMemory();

	return true;

}

#ifdef legacyOpenGL
// ====================================================================//
//                   Legacy Draw and Drawto functions                  //
// ====================================================================//
bool spoutGLDXinterop::DrawSharedTexture(float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
#ifdef legacyOpenGL // defined in SpoutCommon.h
	UNREFERENCED_PARAMETER(HostFBO);

	if (m_bUseMemory) { // Memoryshare
		return(DrawSharedMemory(max_x, max_y, aspect, bInvert));
	}
	else if (m_bUseGLDX) { // GL/DX interop
		return(DrawGLDXtexture(max_x, max_y, aspect, bInvert));
	}
	else {
		SpoutLogError("spoutGLDXinterop::DrawSharedTexture failed");
		return false;
	}
}

bool spoutGLDXinterop::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height,
	float max_x, float max_y, float aspect,
	bool bInvert, GLuint HostFBO)
{
	if (m_bUseMemory) { // Memoryshare
		return(DrawToSharedMemory(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert));
	}
	else if (m_bUseGLDX) { // GL/DX interop
		return(DrawToGLDXtexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO));
	}
	else {
		SpoutLogError("spoutGLDXinterop::DrawToSharedTexture failed");
		return false;
	}
}

//
// Draw shared memory via texture - equivalent to DrawSharedTexture
//
bool spoutGLDXinterop::DrawSharedMemory(float max_x, float max_y, float aspect, bool bInvert)
{
	unsigned int width, height;

	// Get the memoryshare size
	if (!memoryshare.GetSenderMemorySize(width, height))
		return false;

	// Find the shared memory buffer pointer
	unsigned char* pBuffer = memoryshare.LockSenderMemory();
	if (!pBuffer) return false;

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);

	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw the texture
	SaveOpenGLstate(width, height);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_TexID);
	glBegin(GL_QUADS);
	if (bInvert) {
		glTexCoord2f(0.0, max_y);	glVertex2f(-aspect, -1.0); // lower left
		glTexCoord2f(0.0, 0.0);	glVertex2f(-aspect, 1.0); // upper left
		glTexCoord2f(max_x, 0.0);	glVertex2f(aspect, 1.0); // upper right
		glTexCoord2f(max_x, max_y);	glVertex2f(aspect, -1.0); // lower right
	}
	else {
		glTexCoord2f(0.0, 0.0);	glVertex2f(-aspect, -1.0); // lower left
		glTexCoord2f(0.0, max_y);	glVertex2f(-aspect, 1.0); // upper left
		glTexCoord2f(max_x, max_y);	glVertex2f(aspect, 1.0); // upper right
		glTexCoord2f(max_x, 0.0);	glVertex2f(aspect, -1.0); // lower right
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	RestoreOpenGLstate();

	memoryshare.UnlockSenderMemory();

	return true;

} // end DrawSharedMemory


// DRAW A TEXTURE INTO SHARED MEMORY - equivalent to DrawToSharedTexture
bool spoutGLDXinterop::DrawToSharedMemory(GLuint TexID, GLuint TextureTarget, 
										  unsigned int width, unsigned int height, 
										  float max_x, float max_y, float aspect, 
										  bool bInvert, GLuint HostFBO)
{
	unsigned int memWidth, memHeight;
	GLenum status;


	// Get the memoryshare size
	if(!memoryshare.GetSenderMemorySize(memWidth, memHeight))
		return false;

	// Sender size check - quit if not equal
	if(width != memWidth || height != memHeight) 
		return false;

	unsigned char* pBuffer = memoryshare.LockSenderMemory();
	if(!pBuffer) {
		return false;
	}

	// Create or resize a local OpenGL texture
	CheckOpenGLTexture(m_TexID, GL_RGBA, width, height, m_TexWidth, m_TexHeight);

	// Create an fbo if not already
	if(m_fbo == 0) glGenFramebuffersEXT(1, &m_fbo); 

	//
	// Draw the input texture into the local texture via an fbo
	//
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Destination is the fbo with local texture attached
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_TexID, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status == GL_FRAMEBUFFER_COMPLETE_EXT) {
	
		// Draw the input texture
		glColor4f(1.f, 1.f, 1.f, 1.f);
		glEnable(TextureTarget);
		glBindTexture(TextureTarget, TexID);

		GLfloat tc[4][2] = {0};

		// Invert texture coord to user requirements
		if(bInvert) {
			tc[0][0] = 0.0;   tc[0][1] = max_y;
			tc[1][0] = 0.0;   tc[1][1] = 0.0;
			tc[2][0] = max_x; tc[2][1] = 0.0;
			tc[3][0] = max_x; tc[3][1] = max_y;
		}
		else {
			tc[0][0] = 0.0;   tc[0][1] = 0.0;
			tc[1][0] = 0.0;   tc[1][1] = max_y;
			tc[2][0] = max_x; tc[2][1] = max_y;
			tc[3][0] = max_x; tc[3][1] = 0.0;
		}

		GLfloat verts[] =  {
						-aspect, -1.0,   // bottom left
						-aspect,  1.0,   // top left
						 aspect,  1.0,   // top right
						 aspect, -1.0 }; // bottom right
	
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, 0, tc );
		glEnableClientState(GL_VERTEX_ARRAY);		
		glVertexPointer(2, GL_FLOAT, 0, verts );
		glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindTexture(TextureTarget, 0);
		glDisable(TextureTarget);

	}
	else {
		PrintFBOstatus(status);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		memoryshare.UnlockSenderMemory();
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	// Now read the local opengl texture into the memory map buffer
	// Use PBO if supported
	if(IsPBOavailable()) {
		UnloadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, pBuffer, GL_RGBA, false, HostFBO);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, m_TexID);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	memoryshare.UnlockSenderMemory();

	return true;

} // end DrawToSharedMemory


//
// DRAW THE SHARED OPENGL TEXTURE
//
bool spoutGLDXinterop::DrawGLDXtexture(float max_x, float max_y, float aspect, bool bInvert)
{
	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	bool bRet = false;

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// go ahead and access the shared texture to draw it
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			SaveOpenGLstate(m_TextureInfo.width, m_TextureInfo.height);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_glTexture); // bind shared texture
			glColor4f(1.f, 1.f, 1.f, 1.f);
			// Tried to convert to vertex array, but Processing crash
			glBegin(GL_QUADS);
			if (bInvert) {
				glTexCoord2f(0.0, max_y);	glVertex2f(-aspect, -1.0); // lower left
				glTexCoord2f(0.0, 0.0);	glVertex2f(-aspect, 1.0); // upper left
				glTexCoord2f(max_x, 0.0);	glVertex2f(aspect, 1.0); // upper right
				glTexCoord2f(max_x, max_y);	glVertex2f(aspect, -1.0); // lower right
			}
			else {
				glTexCoord2f(0.0, 0.0);	glVertex2f(-aspect, -1.0); // lower left
				glTexCoord2f(0.0, max_y);	glVertex2f(-aspect, 1.0); // upper left
				glTexCoord2f(max_x, max_y);	glVertex2f(aspect, 1.0); // upper right
				glTexCoord2f(max_x, 0.0);	glVertex2f(aspect, -1.0); // lower right
			}
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			RestoreOpenGLstate();
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject); // unlock dx object
			bRet = true;
		} // lock failed
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	} // mutex lock failed

	return bRet;

} // end DrawGLDXTexture

//
// DRAW A TEXTURE INTO THE THE SHARED OPENGL TEXTURE VIA AN FBO
//
bool spoutGLDXinterop::DrawToGLDXtexture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height,
	float max_x, float max_y, float aspect,
	bool bInvert, GLuint HostFBO)
{
	GLenum status;
	bool bRet = false;

	if (m_hInteropDevice == NULL || m_hInteropObject == NULL)
		return false;

	if (width != (unsigned  int)m_TextureInfo.width || height != (unsigned  int)m_TextureInfo.height)
		return false;

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Draw the input texture into the shared texture via an fbo
			// Bind our fbo and attach the shared texture to it
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
			glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
				glColor4f(1.f, 1.f, 1.f, 1.f);
				glEnable(TextureTarget);
				glBindTexture(TextureTarget, TextureID);
				GLfloat tc[4][2] = { 0 };
				// Invert texture coord to user requirements
				if (bInvert) {
					tc[0][0] = 0.0;   tc[0][1] = max_y;
					tc[1][0] = 0.0;   tc[1][1] = 0.0;
					tc[2][0] = max_x; tc[2][1] = 0.0;
					tc[3][0] = max_x; tc[3][1] = max_y;
				}
				else {
					tc[0][0] = 0.0;   tc[0][1] = 0.0;
					tc[1][0] = 0.0;   tc[1][1] = max_y;
					tc[2][0] = max_x; tc[2][1] = max_y;
					tc[3][0] = max_x; tc[3][1] = 0.0;
				}
				GLfloat verts[] = {
								-aspect, -1.0,   // bottom left
								-aspect,  1.0,   // top left
								 aspect,  1.0,   // top right
								 aspect, -1.0 }; // bottom right
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, 0, tc);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(2, GL_FLOAT, 0, verts);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(TextureTarget, 0);
				glDisable(TextureTarget);
				bRet = true; // success
			}
			else {
				PrintFBOstatus(status);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
				UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			}
			// restore the previous fbo - default is 0
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		} // end interop lock
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	} // mutex access failed

	return bRet;

} // end DrawToGLDXtexture
#endif

//
// ============== end legacy draw functions ================
//
#endif


// =========================================================
//                    OpenGL utilities
// =========================================================


//
// COPY AN OPENGL TEXTURE TO ANOTHER OPENGL TEXTURE
//
// Textures must be the same size
//
bool spoutGLDXinterop::CopyTexture(GLuint SourceID,
	GLuint SourceTarget,
	GLuint DestID,
	GLuint DestTarget,
	unsigned int width,
	unsigned int height,
	bool bInvert,
	GLuint HostFBO)
{
	GLenum status;

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

	// bind the FBO (for both, READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	// Attach the Source texture to the color buffer in our frame buffer
	glFramebufferTexture2DEXT(READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Attach destination texture (the texture we write into) to second attachment point
	glFramebufferTexture2DEXT(DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, DestTarget, DestID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {

		if (m_bBLITavailable) {
			if (bInvert) {
				// Blit method with checks - 0.75 - 0.85 msec
				// copy one texture buffer to the other while flipping upside down 
				// (OpenGL and DirectX have different texture origins)
				glBlitFramebufferEXT(0, 0, // srcX0, srcY0, 
					width, height,         // srcX1, srcY1
					0, height,             // dstX0, dstY0,
					width, 0,              // dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
			else {
				// Do not flip during blit
				glBlitFramebufferEXT(0, 0, // srcX0, srcY0, 
					width, height,         // srcX1, srcY1
					0, 0,                  // dstX0, dstY0,
					width, height,         // dstX1, dstY1,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
		}
		else {
			// No fbo blit extension
			// Copy from the fbo (source texture attached) to the dest texture
			glBindTexture(DestTarget, DestID);
			glCopyTexSubImage2D(DestTarget, 0, 0, 0, 0, 0, width, height);
			glBindTexture(DestTarget, 0);
		}
	}
	else {
		PrintFBOstatus(status);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

} // end CopyTexture


// If an OpenGL texture has not been created or it is a different size, create a new one
void spoutGLDXinterop::CheckOpenGLTexture(GLuint &texID, GLenum GLformat,
										  unsigned int newWidth, unsigned int newHeight,
										  unsigned int &texWidth, unsigned int &texHeight)
{
	if(texID == 0 || newWidth != texWidth || newHeight != texHeight) {
		InitTexture(texID, GLformat, newWidth, newHeight);
		texWidth = newWidth;
		texHeight = newHeight;
	}
}


// Initialize local OpenGL texture
void spoutGLDXinterop::InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height)
{
	if(texID != 0) glDeleteTextures(1, &texID);	
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	// glTexImage2D(GL_TEXTURE_2D, 0, GLformat, width, height, 0, GLformat, GL_UNSIGNED_BYTE, NULL); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GLformat, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void spoutGLDXinterop::SaveOpenGLstate(unsigned int width, unsigned int height, bool bFitWindow)
{
	float dim[4];
	float vpScaleX, vpScaleY, vpWidth, vpHeight;
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

	// Fit to window
	if(bFitWindow) {
		// Scale both width and height to the current viewport size
		vpScaleX = dim[2]/(float)width;
		vpScaleY = dim[3]/(float)height;
		vpWidth  = (float)width  * vpScaleX;
		vpHeight = (float)height * vpScaleY;
		vpx = vpy = 0;
	}
	else {
		// Preserve aspect ratio of the sender
		// and fit to the width or the height
		vpWidth = dim[2];
		vpHeight = ((float)height/(float)width)*vpWidth;
		if(vpHeight > dim[3]) {
			vpHeight = dim[3];
			vpWidth = ((float)width/(float)height)*vpHeight;
		}
		vpx = (int)(dim[2]-vpWidth)/2;;
		vpy = (int)(dim[3]-vpHeight)/2;
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


void spoutGLDXinterop::RestoreOpenGLstate()
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


//
// Create an OpenGL window and context for situations where there is none.
// Not used if applications already have an OpenGL context.
// Always call CloseOpenGL afterwards.
//
bool spoutGLDXinterop::CreateOpenGL()
{
	// For CreateOpenGL and CloseOpenGL
	m_hdc = NULL;
	m_hwndButton = NULL;
	m_hRc = NULL;

	SpoutLogNotice("spoutGLDXinterop::CreateOpenGL()");

	HGLRC glContext = wglGetCurrentContext();

	if(glContext == NULL) {

		// We only need an OpenGL context with no render window because we don't draw to it
		// so create an invisible dummy button window. This is then independent from the host
		// program window (GetForegroundWindow). If SetPixelFormat has been called on the
		// host window it cannot be called again. This caused a problem in Mapio.
		// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
		//
		// CS_OWNDC allocates a unique device context for each window in the class. 
		//
		if(!m_hwndButton || !IsWindow(m_hwndButton)) {
			m_hwndButton = CreateWindowA("BUTTON",
				            "SpoutOpenGL",
							WS_OVERLAPPEDWINDOW | CS_OWNDC,
							0, 0, 32, 32,
							NULL, NULL, NULL, NULL);
		}

		if(!m_hwndButton) { 
			SpoutLogError("spoutGLDXinterop::CreateOpenGL - no hwnd");
			return false; 
		}

		m_hdc = GetDC(m_hwndButton);
		if(!m_hdc) { 
			SpoutLogError("spoutGLDXinterop::CreateOpenGL - no hdc");
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
		int iFormat = ChoosePixelFormat(m_hdc, &pfd);
		if(!iFormat) {
			SpoutLogError("spoutGLDXinterop::CreateOpenGL - pixel format error");
			return false; 
		}

		if(!SetPixelFormat(m_hdc, iFormat, &pfd)) {
			DWORD dwError = GetLastError();
			// 2000 (0x7D0) The pixel format is invalid.
			// Caused by repeated call of  the SetPixelFormat function
			char temp[128];
			sprintf_s(temp, "spoutGLDXinterop::CreateOpenGL - SetPixelFormat Error %d (%x)", dwError, dwError);
			SpoutLogError("%s", temp);
			return false; 
		}

		m_hRc = wglCreateContext(m_hdc);
		if(!m_hRc) { 
			SpoutLogError("spoutGLDXinterop::CreateOpenGL - could not create OpenGL context");
			return false; 
		}

		wglMakeCurrent(m_hdc, m_hRc);
		if(wglGetCurrentContext() == NULL) {
			SpoutLogError("spoutGLDXinterop::CreateOpenGL - no OpenGL context");
			return false; 
		}
		SpoutLogNotice("    OpenGL window created OK");
	}
	else {
		SpoutLogNotice("    OpenGL context exists");
	}

	return true;
}


bool spoutGLDXinterop::CloseOpenGL()
{

	SpoutLogNotice("spoutGLDXinterop::CloseOpenGL()");

	// Properly kill the OpenGL window
	if (m_hRc) {

		if (!wglMakeCurrent(NULL,NULL))	{ // Are We Able To Release The DC And RC Contexts?
			SpoutLogError("spoutGLDXinterop::CloseOpenGL - release of DC and RC failed");
			return false; 
		}

		if (!wglDeleteContext(m_hRc)) { // Are We Able To Delete The RC?
			SpoutLogError("spoutGLDXinterop::CloseOpenGL - release rendering context failed");
			return false; 
		}
		m_hRc=NULL;
	}

	if (m_hdc && !ReleaseDC(m_hwndButton, m_hdc)) { // Are We Able To Release The DC
		SpoutLogError("spoutGLDXinterop::CloseOpenGL - release device context Failed");
		m_hdc=NULL;
		return false; 
	}

	if (m_hwndButton && !DestroyWindow(m_hwndButton)) { // Are We Able To Destroy The Window?
		SpoutLogError("spoutGLDXinterop::CloseOpenGL - could not release hWnd");
		m_hwndButton=NULL;
		return false; 
	}

	SpoutLogNotice("    closed the OpenGL window OK");

	return true;
}

//
//	UTILITY
//

void spoutGLDXinterop::trim(char* s) {
	char* p = s;
	int l = (int)strlen(p);

	while (isspace(p[l - 1])) p[--l] = 0;
	while (*p && isspace(*p)) ++p, --l;

	memmove(s, p, l + 1);
}

// Given a DeviceKey string from a DisplayDevice
// read all the information about the adapter.
// Only used by this class.
bool spoutGLDXinterop::OpenDeviceKey(const char* key, int maxsize, char *description, char *version)
{
	// Extract the subkey from the DeviceKey string
	HKEY hRegKey = NULL;
	DWORD dwSize = 0;
	DWORD dwKey = 0;
	char output[256];
	strcpy_s(output, 256, key);
	std::string SubKey = strstr(output, "System");

	// Convert all slash to double slash using a C++ string function
	// to get subkey string required to extract registry information
	for (unsigned int i = 0; i < SubKey.length(); i++) {
		if (SubKey[i] == '\\') {
			SubKey.insert(i, 1, '\\');
			++i; // Skip inserted char
		}
	}

	// Open the key to find the adapter details
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey.c_str(), NULL, KEY_READ, &hRegKey) == 0) {
		dwSize = MAX_PATH;
		// Adapter name
		if (RegQueryValueExA(hRegKey, "DriverDesc", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			strcpy_s(description, maxsize, output);
		}
		if (RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			// Find the last 6 characters of the version string then
			// convert to a float and multiply to get decimal in the right place
			sprintf_s(output, 256, "%5.2f", atof(output + strlen(output) - 6)*100.0);
			strcpy_s(version, maxsize, output);
		} // endif DriverVersion
		RegCloseKey(hRegKey);
	} // endif RegOpenKey

	return true;
}


