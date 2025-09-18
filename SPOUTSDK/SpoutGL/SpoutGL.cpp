//
//		SpoutGL
//
//		Base class for OpenGL texture sharing using the NVIDIA GL/DX interop extensions
//
//		See also - spoutDirectX, spoutSenderNames
//
// ====================================================================================
//		Revisions :
//
//		07.10.20	- Started class based on previous work with SpoutGLDXinterop.cpp
//					  for 2.006 and 2.007 beta : 15-07-14 - 03-09-20
//					  with reference to the SpoutDX class for consolidation of global variables.
//					  Compatibility with NVIDIA GL/DX interop is tested with fall-back to CPU share
//					  using DirectX11 staging textures for failure.
//					  MemoryShare is supported for receive only.
//					  DX9 support is removed.
//		09.12.20	- Correct ReadDX11texture for staging texture pitch
//		27.12.20	- Functions allocated to SpoutSDK class where appropriate
//		14.01.21	- Add GetDX11Device() and GetDX11Context()
//					  add bInvert and HostFBO options to WriteTextureReadback
//		04.02.21	- SetHostPath and SetSenderCPUmode public
//					  Add GetCPUshare and SetCPUshare for forced CPU share testing
//		05.02.21	- Introduced m_bTextureShare and m_bCPUshare flags to handle mutiple options
//		08.02.21	- WriteDX11texture, ReadTextureData, OpenSpout, LoadGLextensions cleanup
//					  OpenSpout look for DirectX to prevent repeat
//		26.02.21	- Change m_bSenderCPUmode to m_bSenderCPU
//					- Add m_bSenderGLDX
//					- Change SetSenderCPUmode to include CPU sharing mode and GLDX compatibility
//					- Change SetSenderCPUmode name to SetSenderID
//		13.03.21	- Add m_bMemoryShare for possible older 2.006 apps
//					  Add memoryshare.CreateSenderMemory
//					  memoryshare.CloseSenderMemory() in destructor
//					  Add WriteMemoryPixels
//					  Change ReadMemoryPixels to accept GL_LUMINANCE
//					  Use reinterpret_cast for memoryshare.LockSenderMemory()
//		15.03.21	- Remove m_bNewFrame
//		20.03.21	- Add memoryshare.GetSenderMemoryName()
//		25.03.21	- Disable frame sync in destructor
//		02.04.21	- Change ReadMemory to ReadMemoryTexture
//		04.04.21	- Add GetSenderMemory
//		08.05.21	- Remove ReadMemoryBuffer open error log
//		10.05.21	- Remove memoryshare struct from header
//					  and replace with SpoutSharedMemory object.
//					  Close shared memory and sync event in SpoutGL class destructor
//		27.05.21	- Add GetMemoryBufferSize
//		09.06.21	- Add CreateMemoryBuffer, DeleteMemoryBuffer
//					  Revise and test data functions
//					  All data functions return false if 2.006 memoryshare mode.
//		26.07.21	- Remove memorysize check from GetMemoryBufferSize for receiver
//		10.08.21	- Correct LoadGLextensions to set no PBO availabliity if FBO fails
//					  WriteDX11texture - unmap staging texture if data read fails
//					  ReadTextureData - allow for no FBO support for low end graphics
//		29.09.21	- OpenSpout and LinkGLDXtextures - test for GL/DX extensions
//		15.10.21	- Remove interop object test for repeat from OpenSpout
//		09.11.21	- Revise UnloadTexturePixels
//		12.11.21	- Add OpenGL context check in destructor
//		13.11.21	- Remove code redundancy in destructor
//					  CleanupGL, CleanupInterop - add warning logs if no context
//					  CleanupDX11 - add warning log if no device
//		14.11.21	- Correct ReadTextureData for RGB source
//		16.11.21	- Remove GLerror from destructor
//		18.11.21	- InitTexture - restore current texture binding
//		19.11.21	- LoadGLextensions in constructor as well as OpenSpout
//		22.11.21	- OpenSpout new line for start changed from printf to SpoutLog
//		23.11.21	- Use SpoutDirectX ReleaseDX11Texture to release shared texture
//		11.12.21	- OpenSpout - return false for no OpenGL context or GL extensions
//				      Change CleanupInterop from void to bool
//				      CleanupDX11() - test CleanupInterop before releasing textures
//					  CleanupGL() - release interop objects before releasing shared texture
//					  OpenDirectX and OpenDirectX11 optional device argument
//		14.12.21	- Remove gl texture delete from GLDXready test.
//		15.12.21	- Change no context log warning to notice in CleanupGL and CleanupDX11
//					  LoadGLextensions - warn if pbo extensions not available or user disable
//					  CleanupGL() - release staging textures
//		16.12.21	- Add "No error: case comment to LinkGLDXtextures
//					  Remove un-necessary wglDXSetResourceShareHandle from LinkGLDXtextures
//		17.12.21	- Device argument only for OpenDirectx11
//					  Remove wglDXSetResourceShareHandleNV from LinkGLDXtextures
//					  Remove dxShareHandle argument from LinkGLDXtextures
//		18.12.21	- Restore default draw for all fbo functions
//					  Release interop objects after LinkGLDXtextures in GLDXready
//					  Create m_bGLDXdone flag for GLDXready to avoid repeats
//		27.12.21	- Restore default fbo in SetSharedTextureData if texture ID is zero
//		23.01.22	- Change pointer comparision from >0 to nullptr in OpenSpout (PR #80)
//		25.01.22	- Remove m_hInteropDevice created check in OpenSpout
//					  Clean up logs in LoadGLextensions
//		21.02.22	- Restore glBufferData method for in UnloadTexturePixels
//					  Pending implementation of glFencSync for glMapBufferRange method
//		16.03.22	- Use m_hInteropObject in LinkGLDXtextures rather than local variable
//					  so that CleanupInterp releases the interop object
//					- Allow for success test in GLDXReady();
//		21.03.22	- LoadGLextensions - correct  && to & for (m_caps & GLEXT_SUPPORT_PBO)
//					  UnloadTexturePixels - casting pitch*width for size compare avoids warning C26451
//		29.03.22	- OpenDeviceKey - correct dwSize from MAX_PATH to 256 in RegOpenKeyExA
//					  ReadTextureData - create unsigned long variables for temp src char array
//					  ReadTextureData - Delete temporary "src" char array created with "new"
//		19.04.22	- Restore host fbo in SetSharedTextureData instead of default 0
//		04.06.22	- SetSharedTextureData - corrected glCheckFramebufferStatus from != to == for textureID 0
//		29.07.22	- OpenSpout - default CPU share until tested
//		28.10.22	- Documentation cleanup
//		26.11.22	- Change SetVerticalSync argument to integer to allow adaptive vsync
//		18.12.22	- ReadTextureData - use std::unique_ptr to create intermediate invert buffer
//					  OpenDeviceKey use std::string "at" instead of direct index
//					  Catch any exception by using cleanup functions in destructor
//		19.12.22	- Add DoDiagnostics to create a log file for CreateInterop failure
//		22.12.22	- Compiler compatibility testing
//					  Remove std::unique_ptr and go back to new/delete
//					  Change all {} initializations to "={}"
//		30.12.22	- Check and confirm fix for issue #85 and PR #86
//					  Check and confirm fix for issue #87
//		05-01-23	- ReadGLDXtexture - Test for no texture read or zero texture
//					  moved to the beginning to avoid redundant texture lock.
//		06-01-23	- ReadDX11pixels check pixels arg for null
//		08.01.23	- Option for keyed shared texture for testing
//					  Code review - Use Microsoft Native Recommended rules
//		18.01.23	- Remove GetSharedTextureData and replace with CopyTexture
//		20.01.23	- changes to gl definitions in SpoutGLextensions for compatibility with Glew
//					  SpoutGL.h - include SpoutGLextensions first to prevent gl.h before Glew.h error
//					  GLerror gluErrorString conditional on Glew define
//		14.04.23	- Correct CreateMemoryBuffer null name check
// Version 2.007.011
//		15.03.23	- If no new sender frame, return true and do not block for all receiving functions
//					  to avoid un-necessary Acquire/Release/Lock/Unlock
//		22.04.23	- Add compute shader utility for OpenGL texture copy
//					  CreateInterop - delete m_glTexture before creating new
//		04.05.23	- CopyTexture - detach textures from fbo. Blit method only for texture invert.
//		07.05.23	- CreateOpenGL - load extensions.
//		17.05.23	- Remove "m_bInitialized = false" check from CleanupGL and put in release functions
//		18/05.23	- CleanupGL
//					  Remove m_SenderName clear
//					  Unbind textures before CleanupInterop
//		19.05.23	- Add GetInteropObject() and GetDXsharedTexture()
//		22.05.23	- Remove CleanupInterop from within CleanupGL and use as separate functions
//		07.06.23	- UnloadTexturePixels - specify number of PBOs created in log
//		09.06.23	- Use glCopyImageSubData for CopyTexture if no invert
//		22.06.23	- CreateComputeCopyShader adjust Y workgroup number for aspect ratio
//		03.07.23	- CreateInterop - code cleanup
//					  ReadTextureData - change cast (#PR93)
//		06.07.23	- Code cleanup
//		13.07.23	- Make InitTexture public
//		17.07.23	- CopyTexture - remove glCopyImageSubData due to format limitations
//				      Add SwapRGB utility
//		18.07.23	- Make GLerror() public
//		22.07.23	- Some extra checks for null m_pSharedTexture for a receiver
// Version 2.007.012
//		24.07.23	- Remove  global m_bKeyed flag
//		31.07.23	- Add OpenGL format functions
//		04.08.23	- Remove unused compute shaders
//		28.08.23	- UnloadTexturePixels public
//		07.10.23	- Conditional compile options for _M_ARM64
//		09.10.23	- Log first line indicate if ARM build
//		19.10.23	- GLDXformat - default changed from RGBA to RGBA8
//					  and include DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM
//		20.10.23	- ReadTextureData - correct glReadPixels format for RGBA data
//					  InitTexture - add pixel type
//					  GLFormatName - correct BGRA name
//		01.11.23	- CreateInterop - correct uint printf formatting for error message
//					  Avoid repeats if interop failure flag is set. Cleared by CleaunpInterop.
//		30.11.23	- ReadMemoryTexture - remove new frame test
//		07.12.23	- DoDiagnostics - use spoututils GetExeName
//		08.12.23	- Remove DXGI_FORMAT_UNKNOWN from DX11format GL>DX11 conversion function
//					  Use default DXGI_FORMAT_B8G8R8A8_UNORM
//		14.12.23	- WriteGLDXpixels - return WriteGLDXtexture instead of true
//					- CreateOpenGL return false if extensions fail to load
//	Version 2.007.013
//		26.02.24	- CreateInterop - check for dimensions out of bounds (> 16384)
//		06.03.24	- GLerror - add error number
//		03.04.24	- Add ReadTexturePixels for multiple format and RGB/RGBA textures
//		08.04.24	- ReadTexturePixels - check OpenGL texture size and return if different
//					  PrintFBOstatus - no warning if complete
//					  InitTexture - corrected internal format
//					  CreateOpenGL - report version created
//		25.04.24	- Correct GLDXformat for default GL_RGBA
//		26.04.24	- GLformatName - revise names
//		06.05.24	- Add more logs for wglDX function failure
//	Version 2.007.014
//		26.06.14	- Restore LoadTexturePixels for 20% speed gain
//		10.09.24    - ReadTextureData - Create a local OpenGL texture
//				      Read back in the required format
//				    - Destructor - specify sender only for name release
//		21.09.24	- Revise ReadTextureData for buffer pitch, format and type
//					  Replace ReadTexturePixels with ReadTextureData
//					  Revise code comments for ReadTextureData
//		23.09.24	- LoadTetxurePixels - independent pbos and indices
//					  ReadTextureData - allow zero argument row pitch for RGB/RGBA
//		25.09.24	- Test with all texture formats. Revise code comments.
//		01.10.24	- Revise ReadTextureData -
//					  Check that the source texture format matches the pixel data type
//					  Specify correct format for glReadPixels and glGetTexImage
//					- CleanupInterop - remove warning if already released
//		02.10.24	- InitTexture - internal texture format GL_RGBA8
//		07.10.24	- Remove unused flags m_bMirror and m_bSwapRB
//		08.10.24	- Initialize OpenGL version class variable in constructor
//					  Add GetGLversion()
//		03.03.25	- Move "#include <algorithm>" in header to SpoutUtils.h (PR #120)
//		05.03.25	- Add m_bSender flag for sender/receiver
//					  Set by Spout::CheckSender and also by Spout::InitReceiver
//					  Used in destructor for ReleaseSenderName
//					  SetVerticalSync - add comments in header file
//		05.05.25	- CreateOpenGL- rreturn silently if a context exists
//		05.08.25	- Add null pixels check to ReadGLDXpixels
//		07.08.25	- PrintFBOstatus -
//					  change GL_FRAMEBUFFER_UNDEFINED_EXT to GL_FRAMEBUFFER_UNDEFINED
//		08.08.25	- Add Khronos GL/DX interop using GL memory extensions.
//					  Avoids dependence on Nvidia. Option to activate in SpoutSettings.
//		09.08.25	- Change all "={}" initializations back to "{}"
//		31.08.25	- Remove DoDiagnostics - never used
//		01.09.25	- Correct RegOpenKeyExA options arg from NULL to 0
//		02.09.25	- Change all spoutdx.GetDX11Context()->Flush() to spoutdx.Flush()
//
// ====================================================================================
//
//	Copyright (c) 2021-2025, Lynn Jarvis. All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without modification, 
//	are permitted provided that the following conditions are met:
//
//		1. Redistributions of source code must retain the above copyright notice, 
//		   this list of conditions and the following disclaimer.
//
//		2. Redistributions in binary form must reproduce the above copyright notice, 
//		   this list of conditions and the following disclaimer in the documentation 
//		   and/or other materials provided with the distribution.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
//	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
//	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
//	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
//	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "SpoutGL.h"

// ================================================


// Class: spoutGL
// Base class for OpenGL texture sharing using the NVIDIA GL/DX interop extensions.
// This class should not be used directly because it is the base for the Spout, SpoutSender and SpoutReceiver classes.
// Refer to the Spout class for further documentation and details.
spoutGL::spoutGL()
{
	m_SenderName[0] = 0;
	m_SenderNameSetup[0] = 0;
	m_Width = 0;
	m_Height = 0;

	// User controled from SpoutSettings
	m_bAuto         = true;  // Texture share unless found incompatible
	m_bCPU          = false; // CPU share

	// Compatibility settings
	m_bGLDX         = false; // Nvidia GL/DX interop available
	m_bGLmemory     = false; // Khronos GL memory avaliable
	m_bUseGLDX      = true;  // Use Nvidia interop
	m_bTextureShare = true;
	m_bCPUshare     = false; // Texture share assumed by default
	m_bConnected    = false;
	m_bInitialized  = false;
	m_bSender       = true;  // Sender or receiver (default sender)
	m_bUpdated      = false;
	m_bGLDXdone     = false; // Compatibility test (not done yet)
	m_bSpoutPanelOpened = false;
	m_bSpoutPanelActive = false;

	m_glTexture = 0;
	m_TexID = 0;
	m_TexWidth = 0;
	m_TexHeight = 0;
	m_TexFormat = GL_RGBA;
	m_fbo = 0;

	m_dxShareHandle = NULL;     // Shared texture handle
	m_pSharedTexture = nullptr; // DX11 shared texture
	m_DX11format = DXGI_FORMAT_B8G8R8A8_UNORM; // Default compatible with DX9
	m_dwFormat = m_DX11format;
	m_pStaging[0] = nullptr;    // DX11 staging textures
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;

	// GL/DX interop
	m_hInteropDevice = NULL;  // NV interop device
	m_hInteropObject = NULL;  // NV interop obkect
	m_glMemObj = 0;           // OpenGL memory object
	m_bInteropFailed = false; // Interop failure flag to avoid repeats
	m_hWnd = nullptr;

	// For CreateOpenGL and CloseOpenGL
	m_hdc = nullptr;
	m_hwndButton = nullptr;
	m_hRc = nullptr;

	// OpenGL extensions
	m_caps = 0; // nothing loaded yet
	m_bExtensionsLoaded = false;
	m_bBGRAavailable    = false;
	m_bFBOavailable     = false;
	m_bBLITavailable    = false;
	m_bSWAPavailable    = false;
	m_bGLDXavailable    = false;
	m_bCOPYavailable    = false;
	m_bGLMEMavailable   = false;
	m_bCONTEXTavailable = false;
	m_bPBOavailable     = true; // Assume true until tested by user setting or LoadGLextensions

	// PBO support
	// default number of buffers
	m_nBuffers = 2;
	// For UnloadTeturePixels
	PboIndex = 0;
	NextPboIndex = 0;
	m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
	// For LoadTexturePixels
	PboLoadIndex = 0;
	NextPboLoadIndex = 0;
	m_loadpbo[0] = m_loadpbo[1] = m_loadpbo[2] = m_loadpbo[3] = 0;

	// User selected Auto share mode
	DWORD dwValue = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Auto", &dwValue))
		m_bAuto = (dwValue == 1);

	// 2.006 registry CPU mode
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwValue))
		m_bCPU = (dwValue == 1);

	// User selected buffering mode
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffering", &dwValue))
		m_bPBOavailable = (dwValue == 1);

	// Number of PBO buffers user selected
	m_nBuffers = 2;
	if(ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Buffers", &dwValue))
		m_nBuffers = (int)dwValue;

	// Find version number from the registry
	// Set by Spout Installer (2005, 2006, etc.) or by SpoutSettings for 2.007 and later
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "Version", &dwValue)) {
		m_SpoutVersion = (int)dwValue;
	}
	else {
		// Get number string e.g. "2.007.009"
		std::string str = GetSDKversion();
		// Remove all "." chars
		str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
		m_SpoutVersion = std::stoi(str);
	}

	// 2.006 memoryshare mode
	// Only set if 2.006 SpoutSettings has been used
	// Removed by 2.007 SpoutSettings
	m_bMemoryShare = GetMemoryShareMode();

	// OpenGL extensions are loaded in OpenSpout()

}

//---------------------------------------------------------
// Function: ~spoutGL()
// Destructor
// Note that an OpenGL context is necessary for release of
// OpenGL objects in CleanupGL and CleanupInterop.
// Similarly, a DirectX device is necessary in CleanupDX11.
// If the context or device is lost, release of memory allocated
// to these objects is handled by the driver.
//
spoutGL::~spoutGL()
{
	try {

		// Sender only for sender name release
		// if not already done by the application
		if (m_bInitialized && m_bSender)
			sendernames.ReleaseSenderName(m_SenderName);

		// Close 2.006 or buffer shared memory if used
		memoryshare.Close();

		// Release sync event if used
		frame.CloseFrameSync();

		// Release semaphore and mutex
		frame.CleanupFrameCount();

		// Release named texture access mutex
		frame.CloseAccessMutex();

		// Release interop
		CleanupInterop();

		// Release OpenGL resources 
		CleanupGL();

		// Finally release DirectX resources and device
		CleanupDX11();

	}
	catch (...) {
		MessageBoxA(NULL, "Exception in SpoutGL destructor", NULL, MB_OK);
	}
}

//
// Group: OpenGL shared texture
//

//---------------------------------------------------------
// Function: BindSharedTexture
// Bind OpenGL shared texture.
bool spoutGL::BindSharedTexture()
{
	// Only for GL/DX interop mode
	if(!InteropReady())
		return false;

	// Test for shared DX11 texture
	if(!m_pSharedTexture)
		return false;

	bool bRet = true;
	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// lock dx object
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Bind our shared OpenGL texture
			glBindTexture(GL_TEXTURE_2D, m_glTexture);
			// Leave interop and mutex both locked for success
		}
		else {
			// Release interop lock and allow texture access for fail
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			frame.AllowTextureAccess(m_pSharedTexture);
			bRet = false;
		}
	}
	return bRet;

} // end BindSharedTexture

//---------------------------------------------------------
// Function: UnBindSharedTexture
// Un-bind OpenGL shared texture.
bool spoutGL::UnBindSharedTexture()
{
	// Only for GL/DX interop mode
	if(!InteropReady())
		return false;

	// Test for shared DX11 texture
	if(!m_pSharedTexture)
		return false;

	// Unbind OpenGL shared texture while the interop is locked
	glBindTexture(GL_TEXTURE_2D, 0);

	// unlock dx object
	UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);

	// Release mutex and allow access to the texture
	frame.AllowTextureAccess(m_pSharedTexture);
	return true;

} // end UnBindSharedTexture

//---------------------------------------------------------
// Function: GetSharedTextureID
// OpenGL shared texture ID.
GLuint spoutGL::GetSharedTextureID()
{
	return m_glTexture;
}

//
// Group: Graphics compatibility
//

//---------------------------------------------------------
// Function: GetAutoShare
// Get auto GPU/CPU share depending on compatibility.
bool spoutGL::GetAutoShare()
{
	return m_bAuto;
}

//---------------------------------------------------------
// Function: SetAutoShare
// Set auto GPU/CPU share depending on compatibility.
void spoutGL::SetAutoShare(bool bAuto)
{
	m_bAuto = bAuto;
}

//---------------------------------------------------------
// Function: GetCPUShare
// Get CPU share for the application.
// This is determined by compatibility or set by the application
bool spoutGL::GetCPUshare()
{
	return m_bCPUshare;
}

//---------------------------------------------------------
// Function: SetCPUshare
// Set CPU share for the application.
// If set false, GL/DX compatibility is re-tested.
// CPU share can also be set globally with "SetCPUmode".
void spoutGL::SetCPUshare(bool bCPU)
{
	m_bCPUshare = bCPU;

	if (m_bCPUshare) {
		m_bTextureShare = false;
	}
	else {
		// Re-test GL/DX compatibility
		SpoutLogNotice("spoutGL::SetCPUshare(%d) - re-testing GL/DX compatibility", bCPU);
		OpenSpout(true);
		if (m_bCPUshare) {
			SpoutLogWarning("    Cannot disable CPU sharing mode. System is not GL/DX compatible");
		}
	}

}

//---------------------------------------------------------
// Function: IsGLDXready
// OpenGL texture share compatibility.
bool spoutGL::IsGLDXready()
{
	if(m_bGLDX)
		return true;
	else if(m_bGLmemory)
		return true;
	else
		return false;

}

//
// Group: For direct access if necessary
//

//---------------------------------------------------------
// Function: OpenSpout
//
// Initialize OpenGL and DX11.
//
//     - Open DirectX and check for availability
//     - Load extensions and check for availability and function
//     - Compatibility test for use or GL/DX interop
//     - Optionally re-test compatibility even if already initialized
//
// Failure means that DirectX is not available.
//
// OpenGL GPU texture sharing is used only if GL/DX compatible
//
// DirectX CPU backup is used if :
//     - Graphics is incompatible
//   and
//     - The user has selected "Auto" share in SpoutSettings
//   or
//     - The user has selected CPU mode (2.006 setting or by registry edit of the CPU entry)
//
// Class flags :
//   m_bGLDX         -  Nvidia GL/DX interop compatible
//   m_bGLmemory     -  Khronos OpenGL memory compatible
//   m_bUseGLDX      -  Use Nvidia GL/DX interop
//   m_bTextureShare -  Use GL/DX methods
//   m_bCPUshare     -  Use DirectX CPU methods
//   Neither method  -  Do not process at all
//
	
bool spoutGL::OpenSpout(bool bRetest)
{
	// Return if already initialized and not re-testing compatibility
	// Look for DirectX device to prevent repeat
	// m_hInteropDevice is created in CreateInterop 
	if (spoutdx.GetDX11Device() != nullptr && !bRetest)
		return true;

	 // This is the start, so make a new line in the log
	SpoutLog("");
#if defined _M_X64
	SpoutLogNotice("spoutGL::OpenSpout - 64bit 2.007 - this 0x%.7X", PtrToUint(this));
#elif defined _M_ARM64
	SpoutLogNotice("spoutGL::OpenSpout - 64bit ARM 2.007 - this 0x%.7X", PtrToUint(this));
#else
	SpoutLogNotice("spoutGL::OpenSpout - 32bit 2.007 - this 0x%.7X", PtrToUint(this));
#endif

	// Starting values (tested in GLDXready)
	m_bGLDX = true;
	m_bGLmemory = false;
	m_bUseGLDX = true;
	m_bTextureShare = true;
	m_bCPUshare = false;

	// DirectX capability is the minimum
	if (!OpenDirectX()) {
		SpoutLogFatal("spoutGL::OpenSpout - Could not initialize DirectX 11");
		return false;
	}

	// DirectX is OK
	// OpenGL device context is needed to go on
	HDC hdc = wglGetCurrentDC();
	if (hdc) {
		// Get a window handle
		m_hWnd = WindowFromDC(hdc);
		// Load extensions
		if (LoadGLextensions()) {
			// If DirectX and OpenGL are both OK - test GLDX compatibility
			// For a re-test, create a new interop device in GLDXReady()
			if (bRetest) {
				CleanupInterop();
			}
			SpoutLogNotice("spoutGL::OpenSpout - OpenGL extensions loaded");
			// Drop through for detail notices
		}
		else {
			SpoutLogFatal("spoutGL::OpenSpout - Could not load OpenGL extensions");
			return false;
		}
	}
	else {
		SpoutLogFatal("spoutGL::OpenSpout - Cannot get GL device context");
		// This is OpenGL, but DirectX shared textures might still work OK (see SpoutDX)
		return false;
	}
	
	//
	// OpenGL GPU texture sharing is used if GL/DX compatible
	//
	// DirectX CPU backup is used if :
	//     Graphics is incompatible
	//   and
	//     The user has selected "Auto" share in SpoutSettings
	//   or
	//     The user has selected CPU mode (2.006 setting or by registry edit of the CPU entry)
	//
	//   m_bGLDX           - Nvidia graphics GL/DX compatible
	//   m_bGLmemory       - Khronos GL memory compatible
	//   m_bUseGLDX        - Use Nvidia GL/DX interop
	//   m_bTextureShare   - use OpenGL GL/DX methods
	//   m_bCPUshare       - use DirectX CPU methods
	//   Neither method    - do not process at all
	//
	if (GLDXready()) {

		// GL/DX compatibility
		if(m_bGLDX)
			SpoutLogNotice("spoutGL::OpenSpout - Nvidia WGL_NV_DX interop compatible");
		else
			SpoutLogNotice("spoutGL::OpenSpout - Nvidia WGL_NV_DX interop not available");

		if(m_bGLmemory)
			SpoutLogNotice("spoutGL::OpenSpout - Khronos GL memory interop compatible");
		else
			SpoutLogNotice("spoutGL::OpenSpout - Khronos GL memory interop not available");

		// User selection of Nvidia/GL memory if both are supported
		if (m_bGLmemory && m_bGLDX) {
			DWORD dwValue = 0;
			if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "GLmemory", &dwValue)) {
				// Disable Nvidia if GL memory was selected
				if (dwValue == 1) {
					m_bUseGLDX = false;
					SpoutLogNotice("spoutGL::OpenSpout - Khronos GL memory interop user selected");
				}
			}
		}
	}
	else {
		// Not GL/DX compatible
		SpoutLogWarning("spoutGL::OpenSpout - system is not compatible with GL/DX interop");
	}

	// Work out sharing methods
	m_bTextureShare = false; // use GL/DX methods
	m_bCPUshare = false; // Use DirectX CPU methods

	// Texture sharing is used if Nvidia GL/DX or Khronos GL memory compatible
	m_bTextureShare  = (m_bGLDX || m_bGLmemory);

	// If not compatible, use CPU share only if Auto has been set in SpoutSettings
	// Errors will be generated for debugging.
	if (!m_bTextureShare && m_bAuto)
		m_bCPUshare = true;

	// CPU share over-ride for 2.006 setting or direct registry edit
	if (m_bCPU) {
		m_bTextureShare = false; // Do not use texture share
		m_bCPUshare = true; // Use CPU share
	}

	// Show the sharing method to be used
	if (m_bTextureShare) {
		if (m_bUseGLDX) {
			SpoutLogNotice("spoutGL::OpenSpout - Using Nvidia driver WGL_NV_DX methods"); // \n
		}
		else if (m_bGLmemory)
			SpoutLogNotice("spoutGL::OpenSpout - Using Khronos GL memory methods\n");
		else
			SpoutLogNotice("spoutGL::OpenSpout - Not using GL/DX interop");
	}
	else if (m_bCPUshare) {
		SpoutLogWarning("spoutGL::OpenSpout - Using CPU DirectX methods\n");
	}
	else {
		SpoutLogWarning("spoutGL::OpenSpout - Cannot share textures\n");
	}
	return true;
}

//---------------------------------------------------------
// Function: OpenDirectX
// Initialize DirectX (D3D11 only)
bool spoutGL::OpenDirectX()
{
	SpoutLogNotice("spoutGL::OpenDirectX");
	if(!spoutdx.OpenDirectX11())
		return false;
	return true;
}

//---------------------------------------------------------
// Function: CloseDirectX
// Close DirectX and free resources
void spoutGL::CloseDirectX()
{
	SpoutLogNotice("spoutGL::CloseDirectX()");

	// Release linked DirectX shared texture
	if (m_pSharedTexture)
		spoutdx.ReleaseDX11Texture(GetDX11Device(), m_pSharedTexture);
	m_pSharedTexture = nullptr;
	m_dxShareHandle = nullptr;

	// Flush context to avoid deferred release
	spoutdx.Flush();

	spoutdx.CloseDirectX11();

}

//---------------------------------------------------------
// Function: GetDX11format
//   Get sender DX11 shared texture format
DXGI_FORMAT spoutGL::GetDX11format()
{
	return m_DX11format;
}

//---------------------------------------------------------
// Function: SetDX11format
//   Set sender DX11 shared texture format
//
//   Texture formats compatible with WGL_NV_DX_interop
//
//   https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop.txt
//   https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt
//
//   D3DFMT_A8R8G8B8                 = 21
//   D3DFMT_X8R8G8B8                 = 22
//   DXGI_FORMAT_R32G32B32A32_FLOAT  = 2
//   DXGI_FORMAT_R16G16B16A16_FLOAT  = 10
//   DXGI_FORMAT_R16G16B16A16_UNORM  = 11
//   DXGI_FORMAT_R16G16B16A16_SNORM  = 13
//   DXGI_FORMAT_R10G10B10A2_UNORM   = 24
//   DXGI_FORMAT_R8G8B8A8_UNORM      = 28
//   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29
//   DXGI_FORMAT_R8G8B8A8_SNORM      = 31
//   DXGI_FORMAT_B8G8R8A8_UNORM      = 87 (default)
//   DXGI_FORMAT_B8G8R8X8_UNORM      = 88
//
void spoutGL::SetDX11format(DXGI_FORMAT textureformat)
{
	m_DX11format = textureformat;
	m_dwFormat = static_cast<DWORD>(m_DX11format); // DWORD used throughout
}

//---------------------------------------------------------
// Function: DX11format
//
//  OpenGL compatible DX11 format
//
//	GL_RGBA8                 (DXGI_FORMAT_R8G8B8A8_UNORM)
//	GL_RGB10_A2              (DXGI_FORMAT_R10G10B10A2_UNORM)
//	GL_RGB16, GL_RGBA16	     (DXGI_FORMAT_R16G16B16A16_UNORM)
//	GL_RGB16F, GL_RGBA16F    (DXGI_FORMAT_R16G16B16A16_FLOAT)
//	GL_RGB32F, GL_RGBA32F    (DXGI_FORMAT_R32G32B32A32_FLOAT)
//  GL_RGB, GL_RGB8, GL_RGBA, GL_BGRA (DXGI_FORMAT_B8G8R8A8_UNORM) // default
//
DXGI_FORMAT spoutGL::DX11format(GLint glformat)
{
	DXGI_FORMAT d3dformat = DXGI_FORMAT_B8G8R8A8_UNORM;
	switch (glformat) {
	case GL_RGBA8: // 0x8058
		d3dformat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case GL_RGB10_A2: // 0x8059
		d3dformat = DXGI_FORMAT_R10G10B10A2_UNORM;
		break;
	case GL_RGB16:  // 0x8054
	case GL_RGBA16: // 0x805B
		d3dformat = DXGI_FORMAT_R16G16B16A16_UNORM;
		break;
	case GL_RGB16F:  // 0x881B
	case GL_RGBA16F: // 0x881A
		d3dformat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case GL_RGB32F:  // 0x8815
	case GL_RGBA32F: // 0x8814
		d3dformat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		// GL_RGB  0x1907
		// GL_RGB8 0x8051
		// GL_RGBA 0x1908
		// GL_BGRA 0x08E1
		// Use default DXGI_FORMAT_B8G8R8A8_UNORM
		break;
	}

	return d3dformat;
}

//---------------------------------------------------------
// Function: GLDXformat
//   Return DX11 compatible OpenGL format
GLint spoutGL::GLDXformat(DXGI_FORMAT textureformat)
{
	DXGI_FORMAT d3dformat = textureformat;
	if (d3dformat == DXGI_FORMAT_UNKNOWN)
		d3dformat = m_DX11format;

	GLint glformat = GL_RGBA;
	switch (d3dformat) {
		// DirectX 9
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		// DirectX 11
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			glformat = GL_RGBA;
			break;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			glformat = GL_RGBA8;
			break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			glformat = GL_RGB10_A2;
			break;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			glformat = GL_RGBA16;
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			glformat = GL_RGBA16F;
			break;
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			glformat = GL_RGBA32F;
			break;
		default:
			break;
	}
	return glformat;
}

//---------------------------------------------------------
// Function:GLformat
//   Return OpenGL texture internal format
GLint spoutGL::GLformat(GLuint TextureID, GLuint TextureTarget)
{
	// https://www.khronos.org/opengl/wiki/Image_Format
	// https://moderngl.readthedocs.io/en/latest/topics/texture_formats.html
	GLint glformat = 0;
	glBindTexture(TextureTarget, TextureID);
	glGetTexLevelParameteriv(TextureTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &glformat);
	glBindTexture(TextureTarget, 0);
	return glformat;
}

//---------------------------------------------------------
// Function:GLformatName
//   Return OpenGL format description for compatible DX11 formats
std::string spoutGL::GLformatName(GLint glformat)
{
	std::string formatname = "unknown";
	GLint format = glformat;

	// If not specified, get from the global DX11 format
	if (format == 0)
		format = GLDXformat();

	switch (format) {
		case GL_RGBA16:
			formatname = "16 bit RGBA";
			break;
		case GL_RGBA16F:
			formatname = "16 bit RGBA float";
			break;
		case GL_RGBA32F:
			formatname = "32 bit RGBA float";
			break;
		case GL_RGB10_A2:
			formatname = "10 bit RGB A2";
			break;
		case GL_RGBA8:
			formatname = "8 bit RGBA";
			break;
		case GL_RGBA:
			formatname = "RGBA";
			break;
		default:
			break;
	}
	return formatname;
}

//---------------------------------------------------------
// Function: CreateOpenGL
//
// Create an OpenGL window and context for situations where there is none.
// The version created depends on that supported by the operating system
// and is reported after creation.
//
// Not necessary if an OpenGL context is already available.
// Always call CloseOpenGL() on application close.
//
// OpenGL support is required.
// Include in your application header file :
//     #include <gl/GL.h>
//     #pragma comment (lib, "opengl32.lib")
//
bool spoutGL::CreateOpenGL(HWND hwnd)
{
	// Return silently if a context exists
	if (wglGetCurrentContext())
		return true;

	m_hdc = nullptr;
	m_hwndButton = nullptr;
	m_hRc = nullptr;

	if (hwnd == nullptr) {
		// We only need an OpenGL context with no render window because we don't draw to it
		// so create an invisible dummy button window. This is then independent from the host
		// program window (GetForegroundWindow). If SetPixelFormat has been called on the
		// host window it cannot be called again. This caused a problem in Mapio.
		// https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx
		//
		// CS_OWNDC allocates a unique device context for each window in the class. 
		//
		if (!m_hwndButton || !IsWindow(m_hwndButton)) {
			m_hwndButton = CreateWindowA("BUTTON",
				"SpoutOpenGL",
				WS_OVERLAPPEDWINDOW | CS_OWNDC,
				0, 0, 32, 32,
				NULL, NULL, NULL, NULL);
		}

		if (!m_hwndButton) {
			SpoutLogError("spoutGL::CreateOpenGL - no hwnd");
			return false;
		}
	}
	else {
		m_hwndButton = hwnd;
	}

	m_hdc = GetDC(m_hwndButton);
	if (!m_hdc) {
		SpoutLogError("spoutGL::CreateOpenGL - no hdc");
		CloseOpenGL();
		return false;
	}

	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-pixelformatdescriptor
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	const int iFormat = ChoosePixelFormat(m_hdc, &pfd);
	if (!iFormat) {
		SpoutLogError("spoutGL::CreateOpenGL - pixel format error");
		CloseOpenGL();
		return false;
	}

	if (!SetPixelFormat(m_hdc, iFormat, &pfd)) {
		const DWORD dwError = GetLastError();
		// 2000 (0x7D0) The pixel format is invalid.
		// Caused by repeated call of the SetPixelFormat function
		char temp[128]{};
		sprintf_s(temp, "spoutGL::CreateOpenGL - SetPixelFormat Error %lu (0x%4.4lX)", dwError, dwError);
		SpoutLogError("%s", temp);
		CloseOpenGL();
		return false;
	}

	m_hRc = wglCreateContext(m_hdc);
	if (!m_hRc) {
		SpoutLogError("spoutGL::CreateOpenGL - could not create OpenGL context");
		CloseOpenGL();
		return false;
	}

	wglMakeCurrent(m_hdc, m_hRc);
	if (!wglGetCurrentContext()) {
		SpoutLogError("spoutGL::CreateOpenGL - no OpenGL context");
		CloseOpenGL();
		return false;
	}
	SpoutLogNotice("    OpenGL window created OK");

	// Load the extensions (returns true if already loaded)
	if (!LoadGLextensions()) {
		SpoutLogWarning("OpenGL extensions failed to load");
		return false;
	}

	int glVersion[2] = { 0, 0 };
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
	SpoutLogNotice("CreateOpenGL - Version %d.%d", glVersion[0], glVersion[1]);

	return true;

}


//---------------------------------------------------------
// Function: CloseOpenGL
// Close OpenGL window and release resources
bool spoutGL::CloseOpenGL()
{
	SpoutLogNotice("spoutGL::CloseOpenGL() - m_hRc = 0x%.7X : m_hdc = 0x%.7X", PtrToUint(m_hRc), PtrToUint(m_hdc) );

	// Properly kill the OpenGL window
	if (m_hRc) {
		if (!wglMakeCurrent(NULL, NULL)) { // Are We Able To Release The DC And RC Contexts?
			SpoutLogError("spoutGL::CloseOpenGL - release of DC and RC failed");
			return false;
		}
		if (!wglDeleteContext(m_hRc)) { // Are We Able To Delete The RC?
			SpoutLogError("spoutGL::CloseOpenGL - release rendering context failed");
			return false;
		}
		m_hRc = NULL;
	}

	if (m_hdc && !ReleaseDC(m_hwndButton, m_hdc)) { // Are We Able To Release The DC
		SpoutLogError("spoutGL::CloseOpenGL - release device context Failed");
		m_hdc = NULL;
		return false;
	}

	if (m_hwndButton && !DestroyWindow(m_hwndButton)) { // Are We Able To Destroy The Window?
		SpoutLogError("spoutGL::CloseOpenGL - could not release hWnd");
		m_hwndButton = NULL;
		return false;
	}

	SpoutLogNotice("    Closed OpenGL window OK");

	return true;
}

//---------------------------------------------------------
// Class initialization status
bool spoutGL::IsSpoutInitialized()
{
	return m_bInitialized;
}

//
// GLDXready
//
// Hardware compatibility test
//
// Nvidia driver interop
//  o Check that extensions for GL/DX interop are available
//  o GLDXready
//      Checks operation of GL/DX interop functions
//		and creates an interop device for success
//	o m_bGLDX - true for Nvidia GL/DX interop availability
//
bool spoutGL::GLDXready(bool bRetest)
{
	// Return if the test has already been done
	if (!bRetest && m_bGLDXdone) {
		SpoutLogNotice("spoutGL::GLDXready - test previously completed");
		return m_bGLDX;
	}

	if (!spoutdx.GetDX11Device()) {
		SpoutLogError("spoutGL::GLDXready - No D3D11 device");
		return false;
	}

	SpoutLog(" "); // New line in the log
	SpoutLogNotice("spoutGL::GLDXready()");

	// Testing dimension and format
	unsigned int width  = 640;
	unsigned int height = 360;
	DXGI_FORMAT dxFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

	// Test whether the interop extensions function correctly.
	//   It is possible that extensions for the GL/DX interop load OK
	//   but that the interop functions fail. This has been noted on
	//   dual graphics machines with the NVIDIA Optimus driver.

	// ============================================================================
	// Test GL memory extension functions first
	//
	SpoutLogNotice("spoutGL::GLDXready - Testing for Khronos GL_EXT_memory interop");
	m_bGLmemory = false; // Default until tested
	if (m_bGLMEMavailable) {
		SpoutLogNotice("spoutGL::GLDXready - GL_EXT_memory extensions avaliable");
		// OpenGL 4.3 is required for GL_EXT_memory_object extensions
		int major = 0;
		int minor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		if ((major > 4) || (major == 4 && minor >= 6)) {

			// OpenGL 4.6 or greater for memory extensions and OpenGL direct access
			SpoutLogNotice("spoutGL::GLDXready - OpenGL Version %d.%d", major, minor);

			// Deafault BGRA D3D11 texture
			HANDLE dxHandle = nullptr;
			ID3D11Texture2D * pSharedTexture = nullptr;
			if (!spoutdx.CreateSharedDX11Texture(spoutdx.GetDX11Device(),
					width, height, dxFormat, &pSharedTexture, dxHandle)) {
				SpoutLogWarning("spoutGL::GLDXready - Could not create shared DirectX texture");
				return false;
			}
			SpoutLogNotice("spoutGL::GLDXready - Created shared DirectX texture");

			// GL texture and memory object
			GLuint glTexture = 0;
			glCreateTextures(GL_TEXTURE_2D, 1, &glTexture);
			if (glGetError() != GL_NO_ERROR) {
				SpoutLogWarning("spoutGL::GLDXready - Could not create OpenGL texture");
				pSharedTexture->Release();
				return false;
			}
			SpoutLogNotice("spoutGL::GLDXready - Created OpenGL texture");

			GLuint memObj = 0;
			glCreateMemoryObjectsEXT(1, &memObj);
			if (glGetError() != GL_NO_ERROR) {
				SpoutLogWarning("spoutGL::GLDXready - Could not create memory object");
				glDeleteTextures(1, &glTexture);
				pSharedTexture->Release();
				return false;
			}
			SpoutLogNotice("spoutGL::GLDXready - Created memory object");

			// Complete all commands
			glFlush();
			spoutdx.FlushWait();

			// Import the D3D11 texture into the OpenGL memory object
			GLenum err = GL_NO_ERROR;
			glImportMemoryWin32HandleEXT(memObj, 0ULL, GL_HANDLE_TYPE_D3D11_IMAGE_KMT_EXT, (void *)dxHandle);
			if (glGetError() != GL_NO_ERROR) {
				SpoutLogWarning("spoutGL::GLDXready - glImportMemoryWin32HandleEXT failed - error 0x%X", err);
			}
			else {
				SpoutLogNotice("spoutGL::GLDXready - glImportMemoryWin32HandleEXT suceeded");
				// Create storage for the OpenGL texture backed by the memory object
				// The internal OpenGL format should correspond to the format of the D3D11 texture.
				// Default GL_BGRA8_EXT matches DXGI_FORMAT_B8G8R8A8_UNORM
				m_glFormat = GL_BGRA8_EXT; 
				// If BGRA extensions are available (SpoutGLextensions)
				if (m_bBGRAavailable) {
					GLint bgrasupported = 0;
					// Test support for GL_BGRA8_EXT internal format
					glGetInternalformativ(GL_TEXTURE_2D, GL_BGRA8_EXT, GL_INTERNALFORMAT_SUPPORTED, 1, &bgrasupported);
					if (bgrasupported == 1) {
						SpoutLogNotice("spoutGL::GLDXready - GL_BGRA8_EXT supported");
						// Test storage function with GL_BGRA8_EXT
						glTextureStorageMem2DEXT(glTexture, 1, m_glFormat, width, height, memObj, 0ULL);
						err = glGetError();
						if (err != GL_NO_ERROR) {
							SpoutLogWarning("spoutGL::GLDXready - glTextureStorageMem2DEXT failed with GL_BGRA8_EXT");
						}
						else {
							SpoutLogNotice("spoutGL::GLDXready - glTextureStorageMem2DEXT succeeded with GL_BGRA8_EXT");
							// Import and storage functions successful
							m_bGLmemory = true;
						}
					}
					else {
						SpoutLogWarning("spoutGL::GLDXready - GL_BGRA8_EXT not supported");
					}
				}
				if (err != GL_NO_ERROR) {
					// Start again and test storage function with GL_RGBA8
					m_glFormat = GL_RGBA8;
					glFlush();
					spoutdx.Flush();
					if (m_glTexture) glDeleteTextures(1, &m_glTexture);
					if (m_glMemObj) glDeleteMemoryObjectsEXT(1, &m_glMemObj);
					glCreateTextures(GL_TEXTURE_2D, 1, &glTexture);
					glCreateMemoryObjectsEXT(1, &memObj);
					glImportMemoryWin32HandleEXT(memObj, 0ULL, GL_HANDLE_TYPE_D3D11_IMAGE_KMT_EXT, (void *)dxHandle);
					glTextureStorageMem2DEXT(glTexture, 1, m_glFormat, width, height, memObj, 0ULL);
					if (glGetError() != GL_NO_ERROR) {
						SpoutLogWarning("spoutGL::GLDXready - glTextureStorageMem2DEXT failed with GL_RGBA8 - error 0x%X", err);
					}
					else {
						SpoutLogNotice("spoutGL::GLDXready - glTextureStorageMem2DEXT succeeded with GL_RGBA8");
						// Import and storage functions successful
						m_bGLmemory = true;
					}
				}

				if (m_bGLmemory) {
					SpoutLogNotice("spoutGL::GLDXready - Khronos GL_EXT_memory methods successful");
				}
				else {
					SpoutLogWarning("spoutGL::GLDXready - Khronos GL_EXT_memory methods not available");
				}

				// Release GL memory interop objects
				// The OpenGL backing memory object must be released before the linked texture.
				// OpenGL owns the handle until the memory object is deleted.
				if (m_glTexture) glDeleteTextures(1, &m_glTexture);
				m_glTexture = 0;
				if (m_glMemObj) glDeleteMemoryObjectsEXT(1, &m_glMemObj);
				m_glMemObj = 0;
				if (pSharedTexture) pSharedTexture->Release();
				pSharedTexture = nullptr;
				glFlush();
				spoutdx.Flush();
			}
		}
		else {
			SpoutLogWarning("spoutGL::GLDXready - OpenGL 4.6 is required for Khronos GL memory extensions");
		}
	}
	else {
		// Warning in LoadGLextensions
		SpoutLogNotice("spoutGL::GLDXready - Khronos GL_EXT_memory extensions not avaliable");
	}

	// ============================================================================
	// Test Nvidia GL/DX interop
	//
	if (m_bGLDXavailable) {
		SpoutLogNotice("spoutGL::GLDXready - testing for Nvidia WGL_NV_DX_interop");
		// Test Nvidia extensions for GL/DX interop - set true until tested
		m_bGLDX = true; // 
		m_bUseGLDX = true;
		// Save the OpenGL format if tested for GL memory abive
		GLenum oldglFormat = m_glFormat;
		if (!CreateInterop(width, height, dxFormat, false)) {
			SpoutLogWarning("spoutGL::GLDXready - Nvidia WGL_NV_DX interop methods not available");
			m_bGLDX = false; // Nvidia not available
			m_bUseGLDX = false; // Not using Nvidia
		}
		else {
			SpoutLogNotice("spoutGL::GLDXready - Nvidia WGL_NV_DX interop methods successful");
		}
		// Restore OpenGL format
		m_glFormat = oldglFormat;
	}
	else {
		// Warning in LoadGLextensions
		SpoutLogNotice("spoutGL::GLDXready - Nvidia WGL_NV_DX interop extensions not available");
		m_bGLDX = false;
		m_bUseGLDX = false;
	}

	CleanupInterop();
	CleanupGL();

	//
	// Now
	//
	// m_bGLDX        - Nvidia WGL_NV_DX interop available/successful
	// m_bGLmemory    - Khronos GL memory extensions available/successful
	// m_bUseGLDX     - Using Nvidia WGL_NV_DX interop
	//				 
	
	// Use of texture sharing or CPU backup is assessed from
	// user settings (retrieved with GetAutoShare)
	// and the actual GL/DX compatibility here (can be retrieved with GetGLDXready)
	//
	// The result is tested in OpenSpout
	//
	// Texture sharing is used if GL/DX compatible
	//
	// CPU backup is used if :
	//     Graphics is incompatible
	//   and
	//     The user has selected "Auto" share in SpoutSettings
	//   or
	//     The user has forced CPU mode by registry edit of the CPU entry 	
	//
	// Otherwise no sharing is performed.

	// If not GLDX compatible, ReadDX11Texture and WriteDX11Texture
	// will be used via CPU staging textures 

	// Set a class flag so the test is not repeated
	m_bGLDXdone = true;

	// Return result for either or both interop methods
	return (m_bGLDX || m_bGLmemory);

}


//---------------------------------------------------------
bool spoutGL::SetHostPath(const char *sendername)
{
	SharedTextureInfo info{};
	if (!sendernames.getSharedInfo(sendername, &info)) {
		SpoutLogWarning("spoutGL::SetHostPath(%s) - could not get sender info", sendername);
		return false;
	}
	char exepath[256]{};
	GetModuleFileNameA(NULL, exepath, sizeof(exepath));
	// Description is defined as wide chars, but the path is stored as byte chars
	strcpy_s((char*)info.description, 256, exepath);
	if (!sendernames.setSharedInfo(sendername, &info)) {
		SpoutLogWarning("spoutGL::SetHostPath(%s) - could not set sender info", sendername);
	}

	return true;

}

//----------------------------------------------------------
// SetSenderID - set top two bits of 32 bit partner ID field
//
//   bCPU      - using CPU sharing methods
//     1000 0000 0000 0000 0000 0000 0000 0000 = 0x80000000
//   bGLDX     - hardware is compatible with Nvidia OpenGL/DirectX interop
//     0100 0000 0000 0000 0000 0000 0000 0000 = 0x40000000
//   bGLmemory - using Khronos GL memory methods for GL/DX interop
//     0010 0000 0000 0000 0000 0000 0000 0000 = 0x20000000
//
// 2.006 senders may or may not have these bits set, but will rarely have the exact values.
//
// This information is used by "SpoutPanel" opened by "SelectSender"
//
bool spoutGL::SetSenderID(const char *sendername, bool bCPU, bool bGLDX, bool bGLmemory)
{
	return sendernames.SetSenderID(sendername, bCPU, bGLDX, bGLmemory);
}

//
// Protected functions
//

//
// Create shared DirectX texture and OpenGL texture and link with GL/DX interop
//
bool spoutGL::CreateInterop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive)
{
	// Avoid repeats if interop failure flag is set
	// Cleared by CleaunpInterop
	if (m_bInteropFailed)
		return false;

	// Cannot create interop if GLDXready test failed
	if(!m_bGLmemory && !m_bGLDX)
		return false;

	// Check for dimensions out of bounds
	// D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION (16384)
	if (width > 16384 || height > 16384)
		return false;

	SpoutLogNotice("spoutGL::CreateInterop");

	// The texture to link with OpenGL
	ID3D11Texture2D* pLinkedTexture = nullptr;

	// Compatible formats - see SetDX11format
	// A DirectX 11 receiver accepts DX9 formats
	// Default DXGI_FORMAT_B8G8R8A8_UNORM
	// unless set otherwise by SetDX11format(DXGI_FORMAT textureformat)
	DWORD format = m_DX11format;
	if (dwFormat > 0 && dwFormat != format) {
		format = dwFormat; // Use the passed texture format
		SetDX11format((DXGI_FORMAT)format); // Set the global texture format
	}

	if (bReceive) {
		// If the receiver retrieved a handle from the sender,
		// a DirectX texture is opened from the sender share handle.
		if (!m_dxShareHandle || !m_pSharedTexture) {
			SpoutLogError("spoutGL::CreateInterop (Nvidia) - no receiver texture handle");
			// Set interop failure flag to avoid repeats (cleared in CleaunpInterop)
			m_bInteropFailed = true;
			return false;
		}
		// Use the texture already created from the sender share handle for linking to OpenGL.
		pLinkedTexture = m_pSharedTexture;
	}
	else {
		// If the sender is using GL memory and if GL_BGRGA8 is not supported,
		// and the sender DirectX texture format is DXGI_FORMAT_B8G8R8A8_UNORM,
		// change the DirectX format to DXGI_FORMAT_R8G8B8A8_UNORM to match with
		// OpenGL GL_RGBA8 to produce an RGBA DirectX texture below
		if (!m_bUseGLDX && m_bGLmemory
			&& m_glFormat == GL_RGBA8 && m_DX11format == DXGI_FORMAT_B8G8R8A8_UNORM) {
			SpoutLogWarning("Changing default D3D11 format to RGBA to match with OpenGL GL_RGBA8");
			SetDX11format(DXGI_FORMAT_R8G8B8A8_UNORM);
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		// A sender creates or re-creates the linked DX11 texture
		// and a new share handle that receivers can use.
		m_dxShareHandle = nullptr;
		if (!spoutdx.CreateSharedDX11Texture(spoutdx.GetDX11Device(),
			width, height,
			(DXGI_FORMAT)format, // Default DXGI_FORMAT_B8G8R8A8_UNORM
			&m_pSharedTexture, m_dxShareHandle)) {
				if (m_pSharedTexture) m_pSharedTexture->Release();
				m_pSharedTexture = nullptr;
				m_dxShareHandle = nullptr;
				char tmp[256]{};
				sprintf_s(tmp, 256, "spoutGL::CreateInterop (Nvidia) - sender CreateSharedDX11Texture failed (%ux%u format 0x%X)",
					width, height, format);
				SpoutLogWarning(tmp);
				// Set interop failure flag to avoid repeats (cleared in CleaunpInterop)
				m_bInteropFailed = true;
				return false;
		}
		pLinkedTexture = m_pSharedTexture;
	}

	//
	// Create or re-create an OpenGL texture
	// to be linked to the DirectX texture
	// 
	// For the OpenGL memory interop, the OpenGL texture must
	// be released before the linked texture and handle
	if (m_glTexture) glDeleteTextures(1, &m_glTexture);
	m_glTexture = 0;

	// glCreateTextures creates the handle and initializes the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &m_glTexture);

	// Use Nvidia interop
	if (m_bUseGLDX && m_bGLDX) {

		//
		// Nvidia WGL_NV_DX interop
		//
		// https://registry.khronos.org/OpenGL/extensions/NV/WGL_NV_DX_interop.txt
		// https://registry.khronos.org/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt
		//

		// The interop may already be released but check
		// here so the function can be used independently.
		if (m_hInteropDevice && m_hInteropObject) {
			SpoutLogNotice("    Re-registering Nvidia interop");
			if (!wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject))
				SpoutLogNotice("spoutGL::CreateInterop (Nvidia) - wglDXUnregisterObjectNV failed");
			m_hInteropObject = nullptr;
		}

		// Link the shared DirectX texture to the OpenGL texture
		// This registers for interop and associates the opengl texture with the dx texture
		// by calling wglDXRegisterObjectNV which returns a handle to the interop object
		// to manage access to the textures. An interop device is created if it does not exist yet.
		m_hInteropObject = LinkGLDXtextures((void *)spoutdx.GetDX11Device(), pLinkedTexture, m_glTexture);
		// Nvidia interop should succeed after testing in GLDXready but check again here.
		if (!m_hInteropObject) {
			SpoutLogWarning("spoutGL::CreateInterop (Nvidia) - LinkGLDXtextures failed");
			// Cannot use Nvidia
			m_bGLDX = false;
			m_bUseGLDX = false;
			// Use GL memory mode if supported
			if (!m_bGLmemory) {
				// Khronos memory interop methods not available
				SpoutLogWarning("spoutGL::CreateInterop (Nvidia) - Khronos GL memory methods not available");
				// Set interop failure flag to avoid repeats (cleared in CleaunpInterop)
				m_bInteropFailed = true;
				return false;
			}
			// drop through
		}
	}
	else if(m_bGLmemory) {

		//
		// Khronos OpenGL memory GL/DX interop
		//
		// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects_win32.txt
		//

		//
		// Release existing objects
		//
		// The OpenGL backing memory object must be released before the linked texture.
		// OpenGL owns the handle until the memory object is deleted.
		if (m_glMemObj) glDeleteMemoryObjectsEXT(1, &m_glMemObj);
		m_glMemObj = 0;

		// Create an OpenGL memory object
		// (must be retained along with the OpenGL texture)
		glCreateMemoryObjectsEXT(1, &m_glMemObj);

		// Link the D3D11 texture with the OpenGL texture
		// by way of the texture handle and memory object
		if (!LinkGLDXmemoryTextures(width, height, m_DX11format,
			m_dxShareHandle, m_glMemObj, m_glTexture)) {
			SpoutLogWarning("spoutGL::CreateInterop (GL memory) - LinkGLDXmemoryTextures failed");
			// Set interop failure flag to avoid repeats (cleared in CleaunpInterop)
			m_bInteropFailed = true;
			return false;
		}
	}
	else {
		SpoutLogError("spoutGL::CreateInterop (GL memory) - Khronos GL memory methods not available");
		m_bInteropFailed = true;
		return false;
	}

	// Update class dimensions
	m_Width  = width;
	m_Height = height;

	// Create an fbo if not already
	// A utility texture (m_TexID) will be created later if needed for texture invert
	if (m_fbo == 0)	glGenFramebuffersEXT(1, &m_fbo);

	// Important to reset PBO indices
	// Pbos are created or re-created in UnloadTexturePixels/LoadTexturePixels
	PboIndex = 0;
	NextPboIndex = 0;
	PboLoadIndex = 0;
	NextPboLoadIndex = 0;

	// Also reset staging texture index
	m_Index = 0;
	m_NextIndex = 0;

	// Clear interop failed flag
	m_bInteropFailed = false;

	return true;

}

//
//	Link a shared DirectX texture to an OpenGL texture and
//	create an object handle for the Nvidia WGL_NV_DX interop
//
//	IN	pSharedTexture  Pointer to shared the DirectX texture
//	IN	dxShareHandle   Handle of the DirectX texture to be shared
//	IN	glTextureID     ID of the OpenGL texture that is to be linked to the shared DirectX texture
//	Returns             Handle to the GL/DirectX interop object (the shared texture)
//
HANDLE spoutGL::LinkGLDXtextures(void* pDXdevice, void* pSharedTexture,  GLuint glTexture)
{
	HANDLE hInteropObject = nullptr;
	DWORD dwError = 0;
	char tmp[128]{};

	SpoutLogNotice("spoutGL::LinkGLDXtextures (Nvidia) - device 0x%X, texture 0x%X, GL texture %d",
		PtrToUint(pDXdevice), PtrToUint(pSharedTexture), glTexture);

	// Are the GL/DX interop extensions loaded ?
	if (!wglDXOpenDeviceNV
		|| !wglDXSetResourceShareHandleNV
		|| !wglDXRegisterObjectNV
		|| !wglDXCloseDeviceNV) {
		SpoutLogError("spoutGL::LinkGLDXtextures - no GL/DX extensions");
		return nullptr;
	}

	// Prepare the DirectX device for interoperability with OpenGL
	// The return value is a handle to a GL/DirectX interop device.
	if (!m_hInteropDevice) {
		try {
			m_hInteropDevice = wglDXOpenDeviceNV(pDXdevice);
		}
		catch (...) {
			SpoutLogError("spoutGL::LinkGLDXtextures - wglDXOpenDeviceNV exception");
			return NULL;
		}
	}

	// Report the error if wglDXOpenDeviceNV failed
	if (!m_hInteropDevice) {
		dwError = GetLastError();
		sprintf_s(tmp, 128, "spoutGL::LinkGLDXtextures : wglDXOpenDeviceNV(0x%.7X) no Interop device - error %lu (0x%.X)\n",
			LOWORD(pDXdevice), dwError, LOWORD(dwError));
		// Other errors reported
		// 1008, 0x3F0 - ERROR_NO_TOKEN
		switch (LOWORD(dwError)) {
		case 0:
			strcat_s(tmp, 128, "    No error");
			break;
		case ERROR_OPEN_FAILED:
			strcat_s(tmp, 128, "    Could not open the Direct3D device.");
			break;
		case ERROR_NOT_SUPPORTED:
			// This can be caused either by passing in a device from an unsupported DirectX
			// version, or by passing in a device referencing a display adapter that is
			// not accessible to the GL.
			strcat_s(tmp, 128, "    The dxDevice is not supported.");
			break;
		default:
			strcat_s(tmp, 128, "    Unknown error.");
			break;
		}
		SpoutLogError("%s", tmp);

		return NULL;
	}

	// wglDXSetResourceShareHandle does not need to be called for DirectX
	// version 10 and 11 resources. Calling this function for DirectX 10
	// and 11 resources is not an error but has no effect.
	
	// Prepare the DirectX texture for use by OpenGL
	// register for interop and associate the opengl texture with the dx texture
	// Returns a handle that can be used for sharing functions
	try {
		hInteropObject = wglDXRegisterObjectNV(m_hInteropDevice,
			pSharedTexture,	// DX texture
			glTexture,		// OpenGL texture
			GL_TEXTURE_2D,	// Must be TEXTURE_2D - multisampling not supported
			WGL_ACCESS_READ_WRITE_NV); // A sender will write and a receiver will read
	}
	catch (...) {
		SpoutLogError("spoutGL::LinkGLDXtextures - wglDXRegisterObjectNV exception");
		return NULL;
	}

	if (!hInteropObject) {
		// Noted C007006E returned on failure.
		// Error codes are 32-bit values, but expected results are in the low word.
		// 006E is ERROR_OPEN_FAILED (110L)
		dwError = GetLastError();
		sprintf_s(tmp, 128, "spoutGL::LinkGLDXtextures - wglDXRegisterObjectNV :error %u, (0x%.X)\n",
			LOWORD(dwError), LOWORD(dwError));
		switch (LOWORD(dwError)) {
		case 0:
			strcat_s(tmp, 128, "    No error");
		break;
		case ERROR_INVALID_HANDLE:
			strcat_s(tmp, 128, "    No GL context is current.");
			break;
		case ERROR_INVALID_DATA:
			strcat_s(tmp, 128, "    Incorrect GL name, type or access parameters.");
			break;
		case ERROR_OPEN_FAILED:
			strcat_s(tmp, 128, "    Failed to open the Direct3D resource.");
			break;
		default:
			strcat_s(tmp, 128, "    Unknown error.");
			break;
		}
		SpoutLogError("%s", tmp);

		// Error so close interop device
		if (m_hInteropDevice) {
			if (!wglDXCloseDeviceNV(m_hInteropDevice)) {
				SpoutLogError("spoutGL::LinkGLDXtextures - wglDXCloseDeviceNV failed");
				return nullptr;
			}
			m_hInteropDevice = nullptr;
		}

	}

	return hInteropObject;

}


//---------------------------------------------------------
// Function: LinkGLDXmemoryTextures
//
// Link an OpenGL texture with a shared D3D11 texture
// for the Khronos GL memory GL/DX interop
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects.txt
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects_win32.txt
//
//    - Import the D3D11 texture into an OpenGL memory object
//    - Create storage for an OpenGL texture backed by the memory object
//
// IN  width       - D3D11 texture width
// IN  height      - D3D11 texture height
// IN  dxFormat    - D3D11 texture format
// IN  texhandle   - D3D11 texture handle
// IN  glMemObj    - GL memory object for D3D11 texture import
// IN  glTextureID - GL texture to be linked with the D3D11 texture, backed by the memory object
// Returns           Success
//
bool spoutGL::LinkGLDXmemoryTextures(unsigned int width, unsigned int height,
	DXGI_FORMAT dxFormat, HANDLE dxTexHandle, GLuint glMemObj, GLuint glTextureID)
{
	if(!GetDX11Device()) {
		SpoutLogError("spoutGL::LinkGLDXmemoryTextures - no DirectX device");
		return false;
	}

	SpoutLogNotice("spoutGL::LinkGLDXmemoryTextures (GL memory) - handle 0x%X", PtrToUint(dxTexHandle));

	// Complete all GL commands
	glFlush();
	// Wait until all DirectX commands are complete
	spoutdx.FlushWait();

	//
	// Import the D3D11 texture into the OpenGL memory object
	//
	// glImportMemoryWin32HandleEXT has a size argument (GLuint64) for the memory object.
	//     https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects_win32.txt
	// The specification notes :
	//   If <handleType> is one of:
	//        HANDLE_TYPE_D3D12_RESOURCE_EXT
    //        HANDLE_TYPE_D3D11_IMAGE_EXT
    //        HANDLE_TYPE_D3D11_KMT_IMAGE_EXT
	//   <size> is ignored.
	//   Some implementations incorrectly require <size> be set to '0'
    //   when importing such handles, so applications should set <size> to
    //   '0' for broader compatibility.
	//
	// In other words, even though size should be ignored, in practice some drivers
	// require it to be set to zero. So it's safer to always set to zero to avoid
	// issues with those drivers. Practical observations suggest that using zero for
	// the size argument is safe and reliable.
	//     https://github.com/KhronosGroup/OpenGL-Registry/issues/575
	//     https://gist.github.com/pabloko/42e28680e04cb15059c9c1a92b4ae622
	//
	// Type should be GL_HANDLE_TYPE_D3D11_IMAGE_KMT_EXT for non-NT handles
	//     https://developer.nvidia.com/getting-vulkan-ready-vr
	// GL_HANDLE_TYPE_OPAQUE_IMAGE_KMT_EXT is also compatible
	//
	glImportMemoryWin32HandleEXT(glMemObj, 0ULL, GL_HANDLE_TYPE_D3D11_IMAGE_KMT_EXT, (void *)dxTexHandle);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		SpoutLogError("    glImportMemoryWin32HandleEXT failed - error 0x%X", err);
		// 0x500 - 1280 - GL_INVALID_ENUM
		// 0x501 - 1281 - GL_INVALID_VALUE
		// 0x502 - 1282 - GL_INVALID_OPERATION
		// 0x505 - 1285 - out of memory
		return false;
	}

	// Create storage for the OpenGL texture backed by the memory object
	//    The internal format should correspond to the format of the D3D11 texture.
	//    The width and height values should correspond to the extent of the texture
	// Default D3D11 format is DXGI_FORMAT_B8G8R8A8_UNORM
	// Default OpenGL format is GL_BGRA8_EXT or GL_RGBA8 if not supported (see GLDXready)
	switch (dxFormat) {
		case DXGI_FORMAT_R8G8B8A8_UNORM: 
			m_glFormat = GL_RGBA8; //0x8058
			break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			m_glFormat = GL_RGB10_A2; // 0x8059
			break;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			m_glFormat = GL_RGBA16; // 0x805B
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			m_glFormat = GL_RGBA16F; // 0x881A
			break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			m_glFormat = GL_RGBA32F; // 0x8814
			break;
		default:
			break;
	}
	SpoutLogNotice("    (%d x %d) DirectX format = %d (0x%X), GL format = 0x%X", width, height, dxFormat, dxFormat, m_glFormat);
	glTextureStorageMem2DEXT(glTextureID, 1, m_glFormat, width, height, glMemObj, 0ull);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		SpoutLogError("    glImportMemoryWin32HandleEXT failed - error 0x%X", err);
		return false; 
	}

	// Complete all OpenGL commands after the interop
	glFlush();

	return true;
}

//---------------------------------------------------------
// Function: InteropReady
//
// Test or readiness of Nvidia or Khronos interop
//
bool spoutGL::InteropReady()
{
	// Nvidia compatible and using Nvidia
	if (m_bGLDX && m_bUseGLDX) {
		// Test success of GL/DX interop
		if (!m_hInteropDevice || !m_hInteropObject)
			return false;
	}
	// GL memory compatible
	else if (m_bGLmemory) {
		// Test success of Khronos GL memory interop
		if (!m_pSharedTexture || !m_glTexture || !m_glMemObj)
			return false;
	}
	return true;
}

//---------------------------------------------------------
// Function: GetNVready
// Nvidia WGL_NV_DX capability
bool spoutGL::GetNVready()
{
	return m_bGLDX;
}

//---------------------------------------------------------
// Function: GetGLready
// Khronos GL memory capability
bool spoutGL::GetGLready()
{
	return m_bGLmemory;
}

// Get application Nvidia WGL_NV_DX usage (for testing)
bool spoutGL::GetNVusage()
{
	return m_bUseGLDX;
}

// Set application Nvidia WGL_NV_DX usage (for testing)
// Also sets a registry key that can be tested when an application starts
void spoutGL::SetNVusage(bool bReady)
{
	m_bUseGLDX = bReady;
	if (WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "GLmemory", (DWORD)(!m_bUseGLDX))) {
		if(!m_bUseGLDX)
			SpoutLogNotice("spoutGL::SetNVusage - Khronos GL memory interop registry key enabled");
		else
			SpoutLogNotice("spoutGL::SetNVusage - Khronos GL memory interop registry key disabled");
	}
}

HANDLE spoutGL::GetInteropDevice()
{
	return m_hInteropDevice; // Handle to the GL/DX interop device
}

HANDLE spoutGL::GetInteropObject()
{
	return m_hInteropObject; // Handle to the GL/DX interop object
}

ID3D11Texture2D* spoutGL::GetDXsharedTexture()
{
	return m_pSharedTexture; // Shared texture
}

//
//	Nvidia GL/DX Interop lock
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
//	DISCUSSION: The Lock/Unlock calls serve as synchronization points
//    between OpenGL and DirectX. They ensure that any rendering
//    operations that affect the resource on one driver are complete
//    before the other driver takes ownership of it.
//
//	This function assumes only one object
//
//	Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGL::LockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	if (!InteropReady())
		return E_HANDLE;

	DWORD dwError = 0;
	HRESULT hr = NOERROR;

	// Using Nvidia and compatible
	if (m_bGLDX && m_bUseGLDX) {
		// Lock dx object
		if (!wglDXLockObjectsNV(hDevice, 1, hObject)) {
			dwError = GetLastError();
			switch (LOWORD(dwError)) {
				case ERROR_BUSY: // One or more of the objects in <hObjects> was already locked.
					hr = E_ACCESSDENIED; // General access denied error
					SpoutLogError("spoutGL::LockInteropObject - ERROR_BUSY");
					break;
				case ERROR_INVALID_DATA: // One or more of the objects in <hObjects>
					// does not belong to the interop device
					// specified by <hDevice>.
					hr = E_ABORT; // Operation aborted
					SpoutLogError("spoutGL::LockInteropObject - ERROR_INVALID_DATA");
					break;
				case ERROR_LOCK_FAILED: // One or more of the objects in <hObjects> failed to
					hr = E_ABORT; // Operation aborted
					SpoutLogError("spoutGL::LockInteropObject - ERROR_LOCK_FAILED");
					break;
				default:
					hr = E_FAIL; // unspecified error
					SpoutLogError("spoutGL::LockInteropObject - UNKNOWN_ERROR");
					break;
			} // end switch
		} // end false
		else {
			hr = NOERROR;
		}
		// Drop through
	}
	// else if(m_bGLmemory) {
		// TODO - Needed?
		// The DirectX device is used only to create the texture and link
		// the memory object. This has no visible effect but adds 0.4-0.5 msec.
		// Leave out for testing for hesitations or tearing.
		// Wait for DirectX to be ready
		// spoutdx.FlushWait();
	// }
	return hr;

} // LockInteropObject


//
// Must return S_OK (0) - otherwise the error can be checked.
//
HRESULT spoutGL::UnlockInteropObject(HANDLE hDevice, HANDLE *hObject)
{
	if (!InteropReady())
		return E_HANDLE;

	DWORD dwError = 0;
	HRESULT hr = NOERROR;

	// Using Nvidia and compatible
	if (m_bGLDX && m_bUseGLDX) {
		if (!wglDXUnlockObjectsNV(hDevice, 1, hObject)) {
			dwError = GetLastError();
			switch (LOWORD(dwError)) {
				case ERROR_NOT_LOCKED:
					hr = E_ACCESSDENIED;
					SpoutLogError("spoutGL::UnLockInteropObject - ERROR_NOT_LOCKED");
					break;
				case ERROR_INVALID_DATA:
					SpoutLogError("spoutGL::UnLockInteropObject - ERROR_INVALID_DATA");
					hr = E_ABORT;
					break;
				case ERROR_LOCK_FAILED:
					hr = E_ABORT;
					SpoutLogError("spoutGL::UnLockInteropObject - ERROR_LOCK_FAILED");
					break;
				default:
					hr = E_FAIL;
					SpoutLogError("spoutGL::UnLockInteropObject - UNKNOWN_ERROR");
					break;
			} // end switch
		} // end fail
		// Drop though
	}
	return hr;

} // end UnlockInteropObject


// Clean up the GL/DX interop
bool spoutGL::CleanupInterop()
{
	// Clear interop failure flag
	m_bInteropFailed = false;
	bool bRet = true;

	// Using Nvidia driver GL/DX interop
	if (m_bGLDX) {
		//
		// Nvidia GL/DX interop
		//
		// Already released ?
		if (m_hInteropDevice || m_hInteropObject) {
			// These things need an opengl context so check
			if (wglGetCurrentContext()) {
				SpoutLogNotice("spoutGL::CleanupInterop - interop device = 0x%7.7X, interop object = 0x%7.7X", PtrToUint(m_hInteropDevice), PtrToUint(m_hInteropObject));
				if (m_hInteropDevice && m_hInteropObject) {
					SpoutLogNotice("    wglDXUnregisterObjectNV");
					if (!wglDXUnregisterObjectNV(m_hInteropDevice, m_hInteropObject)) {
						SpoutLogNotice("spoutGL::CleanupInterop - wglDXUnregisterObjectNV failed : could not un-register interop");
						bRet = false;
					}
					m_hInteropObject = nullptr;
				}
				else {
					if (!m_hInteropDevice)
						SpoutLogWarning("spoutGL::CleanupInterop - null interop device");
					if (!m_hInteropObject)
						SpoutLogWarning("spoutGL::CleanupInterop - null interop object");
					bRet = false;
				}
				if (m_hInteropDevice) {
					SpoutLogNotice("    wglDXCloseDeviceNV");
					if (!wglDXCloseDeviceNV(m_hInteropDevice)) {
						SpoutLogNotice("spoutGL::CleanupInterop - wglDXCloseDeviceNV failed : could not close interop");
						bRet = false;
					}
					m_hInteropDevice = nullptr;
				}
			}
			else {
				SpoutLogWarning("spoutGL::CleanupInterop() - no GL context");
				bRet = false;
			}
		}
	}
	else if (m_bGLmemory) {
		// Release GL memory interop objects
		// The OpenGL backing memory object must be released before the linked texture.
		// OpenGL owns the handle until the memory object is deleted.
		if (m_glTexture) glDeleteTextures(1, &m_glTexture);
		if (m_glMemObj) glDeleteMemoryObjectsEXT(1, &m_glMemObj);
		m_glTexture = 0;
		m_glMemObj = 0;
		spoutdx.Flush();
	}

	m_hInteropObject = nullptr;
	m_hInteropDevice = nullptr;

	return bRet;

}

//---------------------------------------------------------
void spoutGL::CleanupGL()
{
	// Release OpenGL resources if there is a context
	if (wglGetCurrentContext()) {

		SpoutLogNotice("spoutGL::CleanupGL");

		// Make sure no texture is bound
		glBindTexture(GL_TEXTURE_2D, 0);

		// Delete the fbo before the texture
		if (m_fbo > 0) glDeleteFramebuffersEXT(1, &m_fbo);
		m_fbo = 0;

		// Delete the linked OpenGL texture
		if (m_glTexture > 0) glDeleteTextures(1, &m_glTexture);
		m_glTexture = 0;

		if (m_TexID > 0)
			glDeleteTextures(1, &m_TexID);

		if (m_pbo[0] > 0)
			glDeleteBuffers(m_nBuffers, m_pbo);

		if (m_loadpbo[0] > 0)
			glDeleteBuffers(m_nBuffers, m_loadpbo);

		m_TexID = 0;
		m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
		m_loadpbo[0] = m_loadpbo[1] = m_loadpbo[2] = m_loadpbo[3] = 0;
	}
	else {
		SpoutLogNotice("spoutGL::CleanupGL() (no GL context)");
	}

	// Release the received D3D11 shared texture
	if (m_pSharedTexture) {
		spoutdx.ReleaseDX11Texture(GetDX11Device(), m_pSharedTexture);
		m_pSharedTexture = nullptr;
	}

	// The share handle is created again for a sender
	// and retrieved from the sender for a receiver
	m_dxShareHandle = nullptr;

	// Staging textures for CPU share are also released in CleanupDX11
	// But release them here to allow for situations where DirectX is not released
	if (m_pStaging[0]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[0]);
	if (m_pStaging[1]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[1]);
	m_pStaging[0] = nullptr;
	m_pStaging[1] = nullptr;
	m_Index = 0;
	m_NextIndex = 0;

	m_Width = 0;
	m_Height = 0;

	// ReleaseReceiver calls CleanupGL before restoring m_SenderName
	// ReleaseSender clears the name

	// Flush D3D11 context to avoid deferred release
	spoutdx.Flush();

	// Do not close DirectX device

}

//
// If the receiver is using GL memory, and the DirectX texture format
// is DXGI_FORMAT_B8G8R8A8_UNORM, some hardware does not support GL_BGRA8_EXT
// and GL_RGBA8 must be used instead for glTextureStorageMem2DEXT.
// Swizzle the OpenGL texture from RGBA to BGRA to match with DirectX BGRA.
//
void spoutGL::CheckOpenGLformat(GLuint TextureID, GLuint TextureTarget)
{
	if (!m_bUseGLDX && m_bGLmemory
		&& m_glFormat == GL_RGBA8 && m_DX11format == DXGI_FORMAT_B8G8R8A8_UNORM) {
		glBindTexture(TextureTarget, TextureID);
		GLint swizzle[] = { GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA };
		glTexParameteriv(TextureTarget, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		glBindTexture(TextureTarget, 0);
	}
}


// Create a new class OpenGL texture -
//   - if not been created yet
//   - if a different size or format
// Typically used for texture copy and invert
void spoutGL::CheckOpenGLTexture(GLuint &texID, GLenum GLformat, unsigned int width,  unsigned int height)
{
	if (texID == 0
		|| texID    != m_TexID 
		|| GLformat != m_TexFormat 
		|| width    != m_TexWidth 
		|| height   != m_TexHeight) {
			InitTexture(texID, GLformat, width, height);
			m_TexID = texID;
			m_TexWidth  = width;
			m_TexHeight = height;
			m_TexFormat = (DWORD)GLformat;
	}
}

// Initialize OpenGL texture
void spoutGL::InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height)
{
	if (texID != 0) glDeleteTextures(1, &texID);
	glGenTextures(1, &texID);

	// Get current texture binding
	GLint texturebinding=0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texturebinding);

	// For glTexImage2D the internal texture format is GL_RGBA8,
	// "GLformat" is the format of the pixel data provided and
	// "type" is the data type of the pixel data

	// Pixel data type
	GLenum type = GL_UNSIGNED_BYTE; // 8 bit RGBA
	if (GLformat == GL_RGBA16) // Bit depth 16 bits
		type = GL_UNSIGNED_SHORT;
	else if (GLformat == GL_RGBA16F || GLformat == GL_RGBA32F) // float
		type = GL_FLOAT;

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GLformat, type, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, texturebinding);

}


//
// COPY AN OPENGL TEXTURE TO THE SHARED OPENGL TEXTURE
//
// Allows for a texture attached to an fbo
// and for the OpenGL framebuffer to be used
//
bool spoutGL::WriteGLDXtexture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// Only for GL/DX interop mode
	// No shared DX11 texture, no copy
	if(!InteropReady() || !m_pSharedTexture)
		return false;

	// Specify greater here because the width/height passed can be smaller
	if (width > m_Width || height > m_Height)
		return false;

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

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
// COPY THE SHARED OPENGL TEXTURE TO AN OPENGL TEXTURE
//
bool spoutGL::ReadGLDXtexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// No interop, no copy
	if(!InteropReady())
		return false;

	// No shared DX11 texture, no copy
	if(!m_pSharedTexture)
		return false;

	// No texture read or zero OpenGL texture (allowed for by ReceiveTexture)
	// the shared texture can be accessed directly
	if (TextureID == 0)
		return true;

	// width and height must be the same as the shared texture
	if (width != m_Width || height != m_Height) {
		return false;
	}

	// GetNewFrame updates sender frame count and fps
	if (!frame.GetNewFrame()) {
		return true;
	}
	// No new frame, do not block

	// Read the shared texture if the sender has produced a new frame
	bool bRet = true; // Error only if texture read fails

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Copy the linked OpenGL texture (m_glTexture) to the user OpenGL texture
			bRet = CopyTexture(m_glTexture, GL_TEXTURE_2D, TextureID, TextureTarget, width, height, bInvert, HostFBO);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
		}
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;

} // end ReadGLDXTexture

//
// Write a texture to the shared texture
//
// Allows for a texture attached to an fbo (TextureID = 0)
// and for the OpenGL framebuffer to be used (HostFBO = 0)
// and also for the input texture to be larger than the destination.
// Interop must be locked for this function to access the shared texture
//
bool spoutGL::SetSharedTextureData(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	GLenum status = 0;
	bool bRet = false;

	// "TextureID" can be zero if it is attached to a bound fbo.
	// If HostFBO is also zero, the default OpenGL framebuffer is used.
	if (TextureID == 0) {
		// Because there are two fbos, the input texture can be larger than the destination.
		// Width and height are the used portion and only that part is copied
		// Input and output textures can have different formats.
		if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
			// Create a class fbo if not already
			if (m_fbo == 0)	glGenFramebuffersEXT(1, &m_fbo);
			// Bind the local fbo for draw
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
			// Draw to the first attachment point
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
			// Attach the destination texture (the shared texture) to point 0 of the second fbo
			glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
			// Check draw fbo for completeness
			status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
				if (m_bBLITavailable) {
					if (bInvert)
						// Copy from one framebuffer to the other while flipping upside down 
						glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
					else
						// Do not flip during blit
						glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				}
				else {
					// No fbo blit extension (< OpenGL v3.0).
					// Copy from the host fbo (input texture attached) to the shared texture
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
		}
		else {
			PrintFBOstatus(status);
			bRet = false;
		}
		// restore host fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
	}
	else if (TextureID > 0) {
		// If there is a valid texture passed in, copy to the
		// shared texture (m_glTexture), inverting as necessary.
		// Both textures must be the same size but can have different formats.
		bRet = CopyTexture(TextureID, TextureTarget, m_glTexture, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
	}
	return bRet;
}

//
// COPY IMAGE PIXELS TO THE OPENGL SHARED TEXTURE
//
// RGBA, BGRA, RGB, BGR formats and unsigned char pixel data type
//
bool spoutGL::WriteGLDXpixels(const unsigned char* pixels,
	unsigned int width, unsigned int height,
	GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	if (width != m_Width || height != m_Height || !pixels)
		return false;
	
	// Create or resize the class OpenGL texture for WriteTexture to use
	// Note that the class OpenGL texture has internal format GL_RGBA8
	// and is created with pixels of data format and type as provided.
	CheckOpenGLTexture(m_TexID, glFormat, width, height);

	// Transfer the pixels to the local texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // In case of RGB pixel data
	if (m_bPBOavailable) {
		// 17-30 msec 3840x2160
		LoadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, pixels, glFormat);
	}
	else {
		// 25-45 msec 3840x2160
		glBindTexture(GL_TEXTURE_2D, m_TexID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// Write the local texture to the shared texture and invert if necessary
	return WriteGLDXtexture(m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);

} // end WriteGLDXpixels


//
// COPY OPENGL SHARED TEXTURE TO IMAGE PIXELS
//
bool spoutGL::ReadGLDXpixels(unsigned char* pixels,
	unsigned int width, unsigned int height,
	GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	// Only for GL/DX interop
	if(!InteropReady())
		return false;

	// No shared DX11 texture, no copy
	if(!m_pSharedTexture)
		return false;

	if (!pixels || width != m_Width || height != m_Height)
		return false;

	// No new frame, do not block
	if (!frame.GetNewFrame()) {
		return true;
	}

	// Read texture pixels for a new frame
	bool bRet = true; // Error only if pixel read fails

	// retrieve opengl texture data directly to image pixels

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {

		// lock gl/dx interop object for access by OpenGL
		if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {

			// Set single pixel alignment in case of rgb source
			if (glFormat == GL_RGB || glFormat == GL_BGR_EXT)
				glPixelStorei(GL_PACK_ALIGNMENT, 1);

			if (bInvert) {
				// Create or resize a local OpenGL texture
				CheckOpenGLTexture(m_TexID, glFormat, width, height);
				// Copy the shared texture to the local texture, inverting if necessary
				CopyTexture(m_glTexture, GL_TEXTURE_2D, m_TexID, GL_TEXTURE_2D, width, height, bInvert, HostFBO);
				// Extract the pixels from the local texture - changing to the user passed format.
				// Use PBO method for maximum speed.
				if (m_bPBOavailable)
					bRet = UnloadTexturePixels(m_TexID, GL_TEXTURE_2D, width, height, 0, pixels, glFormat, false, HostFBO);
				else
					bRet = ReadTextureData(m_TexID, GL_TEXTURE_2D, pixels, width, height, 0, glFormat, GL_UNSIGNED_BYTE, false, HostFBO);
			}
			else {
				// Extract the pixels directly from the shared texture
				if (m_bPBOavailable)
					bRet = UnloadTexturePixels(m_glTexture, GL_TEXTURE_2D, width, height, 0, pixels, glFormat, false, HostFBO);
				else
					bRet = ReadTextureData(m_glTexture, GL_TEXTURE_2D, pixels, width, height, 0, glFormat, GL_UNSIGNED_BYTE, false, HostFBO);
			}

			// default alignment
			glPixelStorei(GL_PACK_ALIGNMENT, 4);

		} // interop lock failed

		// Ensure interop object is unlocked
		UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);

		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);

	} // mutex access failed

	return bRet;

} // end ReadGLDXpixels 

//
// Asynchronous Read-back from an OpenGL texture
//
// Reads from an OpenGL texture to and RGBA buffer using pbo
// Used by a receiver to read pixels from a shared texture (ReceiveImage)
//
// Adapted from : http://www.songho.ca/opengl/gl_pbo.html
// Also see : https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
//
bool spoutGL::UnloadTexturePixels(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, unsigned int rowpitch,
	unsigned char* data, GLenum glFormat,
	bool bInvert, GLuint HostFBO)
{
	if (!data)
		return false;

	void* pboMemory = nullptr;

	// Row pitch passed in can be greater than the width
	// to allow for DX11 staging textures
	GLint pitch = (GLint)rowpitch;
	GLint nchannels = 4; // RGBA or RGB
	if (glFormat == GL_RGB || glFormat == GL_BGR_EXT)
		nchannels = 3;
	// If passed in as zero, calculate from width for RGB or RGBA
	if (rowpitch == 0)
		pitch = (GLint)width*nchannels; // RGB or RGBA

	// Create class fbo if not already
	if (m_fbo == 0) {
		SpoutLogNotice("spoutGL::UnloadTexturePixels - creating FBO");
		glGenFramebuffersEXT(1, &m_fbo);
	}

	// Create pbos if not already
	if (m_pbo[0] == 0) {
		SpoutLogNotice("spoutGL::UnloadTexturePixels - creating %d PBOs", m_nBuffers);
		glGenBuffers(m_nBuffers, m_pbo);
		PboIndex = 0;
		NextPboIndex = 0;
	}

	PboIndex = (PboIndex + 1) % m_nBuffers;
	NextPboIndex = (PboIndex + 1) % m_nBuffers;

	// If Texture ID is zero do nothing, the texture is already attached to the Host Fbo.
	// If not, attach the user texture to the class fbo just created.
	if (TextureID > 0) {
		// Attach the texture to point 0
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0);
		// Set the target framebuffer to read
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	}
	else if (HostFBO == 0) {
		// If no texture ID, a Host FBO must be provided
		// Console print for testing
		SpoutLogError("spoutGL::UnloadTexturePixels - a Host FBO must be provided\n");
		return false;
	}

	// Bind the PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Check it's size
	GLint size = 0;
	glGetBufferParameteriv(GL_PIXEL_PACK_BUFFER, GL_BUFFER_SIZE, &size);
	if (size > 0 && size != (GLint)(pitch * height)) {
		// All PBOs must be re-created
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		glDeleteBuffers(m_nBuffers, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = m_pbo[3] = 0;
		return false;
	}

	// Null existing PBO data to avoid a stall
	// This allocates memory for the PBO pitch*height wide
	glBufferData(GL_PIXEL_PACK_BUFFER, (GLint)(pitch*height), 0, GL_STREAM_READ);

	// Read pixels from framebuffer to PBO - glReadPixels() should return immediately.
	const GLint rowlength = { pitch/nchannels }; // row length in pixels - not bytes
	glPixelStorei(GL_PACK_ROW_LENGTH, rowlength);
	glReadPixels(0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	// If there is data in the next pbo from the previous call, read it back
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);

	// Map the PBO to process its data by CPU
	pboMemory = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	// glMapBuffer can return NULL when called the first time
	// when the next pbo has not been filled with data yet
	glGetError(); // remove the last error

	if (pboMemory && data) {
		// Update data directly from the mapped buffer.
		spoutcopy.CopyPixels((const unsigned char*)pboMemory, (unsigned char*)data,
			rowlength, height, glFormat, bInvert);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	// skip the copy rather than return false.

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Restore the previous fbo binding
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

}


// Streaming texture pixel load
// From : http://www.songho.ca/opengl/gl_pbo.html
// Approximately 20% faster than using glTexSubImage2D alone
// GLformat can be default GL_BGRA or GL_RGBA
bool spoutGL::LoadTexturePixels(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, 
	const unsigned char* data, int GLformat, bool bInvert)
{
	if (!data)
		return false;

	void* pboMemory = nullptr;

	// Create load pbos if not already
	if (m_loadpbo[0] == 0) {
		SpoutLogNotice("spoutGL::LoadTexturePixels - creating %d PBOs", m_nBuffers);
		glGenBuffers(m_nBuffers, m_loadpbo);
		PboLoadIndex = 0;
		NextPboLoadIndex = 0;
	}

	PboLoadIndex = (PboLoadIndex + 1) % m_nBuffers;
	NextPboLoadIndex = (PboLoadIndex + 1) % m_nBuffers;

	// Bind the texture and PBO
	glBindTexture(TextureTarget, TextureID);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_loadpbo[PboLoadIndex]);

	// Copy pixels from PBO to the texture - use offset instead of pointer.
	// glTexSubImage2D redefines a contiguous subregion of an existing
	// two-dimensional texture image. NULL data pointer reserves space.
	glTexSubImage2D(TextureTarget, 0, 0, 0, width, height, GLformat, GL_UNSIGNED_BYTE, 0);

	// Bind PBO to update the texture
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_loadpbo[NextPboLoadIndex]);

	// Check it's size
	GLint size = 0;
	glGetBufferParameteriv(GL_PIXEL_PACK_BUFFER, GL_BUFFER_SIZE, &size);
	if (size > 0 && size != (int)(width*height*4)) {
		// All PBOs must be re-created
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glDeleteBuffers(m_nBuffers, m_loadpbo);
		m_loadpbo[0] = m_loadpbo[1] = m_loadpbo[2] = m_loadpbo[3] = 0;
		return false;
	}

	// Call glBufferData() with a NULL pointer to clear the PBO data and avoid a stall.
	glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)width*height*4, 0, GL_STREAM_DRAW);

	// Map the buffer object into client's memory
	pboMemory = (void*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	// Update the mapped buffer directly
	if (pboMemory) {
		// Copy pixel data
		// Use sse2 if the width is divisible by 16
		spoutcopy.CopyPixels(data, (unsigned char*)pboMemory, 
			width, height, GLformat, bInvert);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
	}
	else {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		return false;
	}

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	return true;

}

//
// Copy OpenGL to DirectX 11 texture via CPU if the GL/DX interop is not available
//
// GPU read is from OpenGL.
// Use multiple PBOs instead of glReadPixels for best speed.
//
bool spoutGL::WriteDX11texture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource{};

	// Only for DX11 mode
	if (!spoutdx.GetDX11Context()) {
		return false;
	}

	// If a staging texture has not been created or a different size create a new one
	// Only one staging texture is required. Buffering read from GPU is done by OpenGL PBO.
	if (!CheckStagingTextures(width, height, 1))
		return false;

	// Map the DX11 staging texture and write the sender OpenGL texture pixels to it
	if (SUCCEEDED(spoutdx.GetDX11Context()->Map(m_pStaging[0], 0, D3D11_MAP_WRITE, 0, &mappedSubResource))) {

		// Staging texture width is multiples of 16 and pitch can be greater that width*4
		// Copy OpenGL texture pixels to the staging texture taking account of the 
		// destination staging texture row pitch.
		if (m_bPBOavailable) {
			if (!UnloadTexturePixels(TextureID, TextureTarget, width, height,
				mappedSubResource.RowPitch, (unsigned char *)mappedSubResource.pData,
				GL_BGRA_EXT, bInvert, HostFBO)) {
					// OpenGL pixel unload failed, Unmap the DirectX texture and return.
					spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);
					return false;
			}
		}
		else {
			if (!ReadTextureData(TextureID, TextureTarget, // OpenGL source texture
				mappedSubResource.pData, // staging texture pixel buffer
				width, height, // width and height of staging texture buffer
				mappedSubResource.RowPitch, // bytes per line of staging texture buffer
				GL_BGRA_EXT, GL_UNSIGNED_BYTE, bInvert, HostFBO)) {
					spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);
					return false;
			}
		}
		spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);

		// The staging texture is updated with the OpenGL texture data
		// Write it to the sender's shared texture
		return WriteTexture(&m_pStaging[0]);

	}

	return false;

} // end WriteDX11texture

//
// Copy from the shared DX11 texture to an OpenGL texture via CPU staging texture
// GPU write is to OpenGL
//
bool spoutGL::ReadDX11texture(GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource{};

	// Quit for zero texture
	if (TextureID == 0 || TextureTarget == 0) {
		return false;
	}

	// Only for DX11 mode
	if (!spoutdx.GetDX11Context())
		return false;

	// Only one staging texture is required because GPU write is to OpenGL
	if (!CheckStagingTextures(width, height, 1)) {
		return false;
	}

	// Read from from the sender shared texture to a staging texture
	if (!ReadTexture(&m_pStaging[0])) {
		return false;
	}

	// Update the application receiving OpenGL texture from the DX11 staging texture
	// Default format is BGRA. Change if the sender is RGBA.
	GLenum glFormat = GL_BGRA_EXT;
	if (m_dwFormat == 28) glFormat = GL_RGBA;

	// Make sure the GPU is ready to access the staging texture 
	spoutdx.Flush();

	// Map the staging texture to access the sender pixels
	if (SUCCEEDED(spoutdx.GetDX11Context()->Map(m_pStaging[0], 0, D3D11_MAP_READ, 0, &mappedSubResource))) {

		if (bInvert) {
			// Create or resize a local OpenGL texture
			CheckOpenGLTexture(m_TexID, glFormat, width, height);
			// Copy the DX11 pixels to it
			glBindTexture(GL_TEXTURE_2D, m_TexID);
		}
		else {
			// Copy the DX11 pixels to the user texture
			glBindTexture(TextureTarget, TextureID);
		}

		// Allow for the staging texture for row pitch
		glPixelStorei(GL_UNPACK_ROW_LENGTH, mappedSubResource.RowPitch/4); // row length in pixels

		// Get the pixels from the staging texture
		glTexSubImage2D(TextureTarget, 0, 0, 0, width, height,
			glFormat, GL_UNSIGNED_BYTE, (const GLvoid *)mappedSubResource.pData);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		// Copy the local texture to the user texture and invert as necessary
		if(bInvert)	
			CopyTexture(m_TexID, GL_TEXTURE_2D, TextureID, TextureTarget, width, height, bInvert, HostFBO);

		spoutdx.GetDX11Context()->Unmap(m_pStaging[0], 0);

		return true;
	}

	return false;

} // end ReadDX11texture


//
// Group: Data sharing
//
//   General purpose data exchange functions using shared memory.
//   These functions can be used in addition to texture sharing.
//   Typical uses will be for data attached to the video frame,
//   commonly referred to as "per frame Metadata".
//
//   If strict synchronization is required, the data sharing functions
//   should be used in combination with event signal functions.
//
//      - void SetFrameSync(const char* SenderName);
//      - bool WaitFrameSync(const char *SenderName, DWORD dwTimeout = 0);
//

//---------------------------------------------------------
// Function: WriteMemoryBuffer
// Write buffer to shared memory.
//
//    If shared memory has not been created in advance, it will be
//    created on the first call to this function at the length specified.
//
//    This is acceptable if the data to send is fixed in length.
//    Otherwise the shared memory should be created in advance of sufficient
//    size to contain the maximum length expected (see CreateMemoryBuffer).
//
//    The map is closed when the sender is released.
//
bool spoutGL::WriteMemoryBuffer(const char *name, const char* data, int length)
{
	// Quit if 2.006 memoryshare mode
	if (m_bMemoryShare)
		return false;

	if (!name || !*name) {
		SpoutLogError("spoutGL::WriteMemoryBuffer - no name");
		return false;
	}

	if (!data) {
		SpoutLogError("spoutGL::WriteMemoryBuffer - no data");
		return false;
	}

	// Create a shared memory map if it does not exist.
	if (memoryshare.Size() == 0) {
		// Create a name for the map
		std::string namestring = name;
		namestring += "_map";
		// Create a shared memory map
		if (!CreateMemoryBuffer(namestring.c_str(), length))
			return false;
	}

	char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("spoutGL::WriteMemoryBuffer - no buffer lock");
		return false;
	}

	// Write user data to shared memory (skip the first 16 bytes containing the map size)
	memcpy((pBuffer + 16), data, length);

	// Terminate the shared memory data with a null.
	// The map is created larger in advance to allow for it.
	if (memoryshare.Size() > (16 + length))
		*(pBuffer + 16 + length) = 0;

	memoryshare.Unlock();

	return true;

}

//---------------------------------------------------------
// Function: ReadMemoryBuffer
// Read shared memory to a buffer.
//
//    Open a memory map and retain the handle.
//    The map is closed when the receiver is released.
int spoutGL::ReadMemoryBuffer(const char* name, char* data, int maxlength)
{
	// Quit if 2.006 memoryshare mode
	if (m_bMemoryShare)
		return 0;

	if (!name || !*name) {
		SpoutLogError("spoutGL::ReadMemoryBuffer - no name");
		return 0;
	}

	if (!data) {
		SpoutLogError("spoutGL::ReadMemoryBuffer - no data");
		return 0;
	}

	// Create a name for the map
	std::string namestring = name;
	namestring += "_map";

	// Create a shared memory map for read if not done already
	if (!memoryshare.Name()) {
		// Create or open the shared memory. This also creates a mutex
		// for the reader to lock and unlock the map for reads.
		if (!memoryshare.Create(namestring.c_str(), maxlength)) {
			SpoutLogWarning("spoutGL::ReadMemoryBuffer - could not create memory map [%s]", namestring.c_str());
			return 0;
		}
	}

	char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("spoutGL::ReadMemoryBuffer - no buffer lock");
		return 0;
	}

	// The memory map includes it's size, saved as the first 16 bytes
	*(pBuffer + 15) = 0; // End for atoi

	// Number of bytes available for data transfer
	int nbytes = atoi(pBuffer);

	// Reduce if the user buffer max length is less
	if (maxlength < nbytes)
		nbytes = maxlength;

	// Copy bytes from shared memory to the user buffer
	if (nbytes > 0)
		memcpy(data, (pBuffer + 16), nbytes);

	// Done with the shared memory pointer
	memoryshare.Unlock();

	return nbytes;
}

//---------------------------------------------------------
// Function: CreateMemoryBuffer
// Create a shared memory buffer.
//
//    Create a memory map and retain the handle.
//    This function should be called before any buffer write
//    if the length of the data to send will vary.
//    The map is closed when the sender is released (see Spout.cpp ReleaseReceiver).
bool spoutGL::CreateMemoryBuffer(const char *name, int length)
{
	// Quit if 2.006 memoryshare mode
	if (m_bMemoryShare)
		return false;

	if (!name || !*name) {
		SpoutLogError("spoutGL::CreateMemoryBuffer - no name");
		return false;
	}

	if (memoryshare.Size() > 0) {
		SpoutLogError("spoutGL::CreateMemoryBuffer - shared memory already exists");
		return false;
	}

	// Create a name for the map from the sender name
	std::string namestring = name;
	namestring += "_map";

	// The first 16 bytes are reserved to record the number of bytes available
	// for data transfer. Make the map 16 bytes larger to compensate. 
	// Add another 16 bytes to allow for a null terminator.
	// (Use multiples of 16 for alignment to allow for SSE copy : TODO).
	if (memoryshare.Create(namestring.c_str(), length + 32) == SPOUT_CREATE_FAILED) {
		SpoutLogError("spoutGL::CreateMemoryBuffer - could not create shared memory");
		return false;
	}

	// The length requested is the number of bytes to be
	// available for data transfer (map data size).
	char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("spoutGL::CreateMemoryBuffer - no buffer lock");
		return false;
	}

	// Convert the map data size to decimal digit chars
	// directly to the first 16 bytes of the shared memory.
	_itoa_s(length, pBuffer, 16, 10);

	memoryshare.Unlock();
	SpoutLogNotice("spoutGL::CreateMemoryBuffer - created shared memory buffer %d bytes", length);

	return true;
}


//---------------------------------------------------------
// Function: DeleteMemoryBuffer
// Delete a sender shared memory buffer.
//
bool spoutGL::DeleteMemoryBuffer()
{
	// Quit if 2.006 memoryshare mode
	if (m_bMemoryShare)
		return false;

	// Only the application that creates a map can close it.
	// The writer creates the map and records the size (memoryshare.Size()).
	// A reader must open a map to find the size and does not record it.
	if (memoryshare.Size() == 0) {
		SpoutLogError("spoutGL::DeleteMemoryBuffer - no shared memory size");
		return false;
	}

	memoryshare.Close();

	return true;

}


//---------------------------------------------------------
// Function: GetMemoryBufferSize
// Get the number of bytes available for data transfer.
//
int spoutGL::GetMemoryBufferSize(const char *name)
{
	// A writer has created the map (Create) and set the map size.
	// The data length is recorded in the first 16 bytes.
	// Another 16 bytes is added to allow for a terminating NULL. (See CreateMemoryBuffer)
	// The remaining length is the number of bytes available for data transfer.
	if (memoryshare.Size() > 32) {
		return memoryshare.Size()-32;
	}

	// A reader must read the map to get the size.
	// The size is not set in the SpoutSharedMemory class.
	// Open a shared memory map for the buffer if it not already.
	// (after a map is opened, the name is saved in the SpoutSharedMemory class).
	if (!memoryshare.Name()) {
		// Create a name for the map
		std::string namestring = name;
		namestring += "_map";
		// Open the shared memory map.
		// This also creates a mutex for the receiver to lock and unlock the map for reads.
		if (!memoryshare.Open(namestring.c_str())) {
			return 0;
		}
		SpoutLogNotice("spoutGL::GetMemoryBufferSize - opened sender memory map [%s]", memoryshare.Name());
	}

	// The map is open and a name for it has been recorded
	char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("spoutGL::GetMemoryBufferSize - no buffer lock");
		return 0;
	}

	// The number of bytes of the memory map available for data transfer
	// is saved in the first 16 bytes.
	*(pBuffer + 15) = 0; // End for atoi
	const int nbytes = atoi(pBuffer);

	memoryshare.Unlock();

	return nbytes;

}

//---------------------------------------------------------
// Function: WriteDX11pixels
//
// COPY IMAGE PIXELS TO THE SHARED DX11 TEXTURE VIA STAGING TEXTURES
// RGBA/RGB/BGRA/BGR supported
//
// GPU write is to DX11
// Use staging texture to support RGBA/RGB
//
bool spoutGL::WriteDX11pixels(const unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (width != m_Width || height != m_Height || !pixels)
		return false;

	// if(!CheckStagingTextures(width, height, 1))
	if (!CheckStagingTextures(width, height, 2))
		return false;

	// 1) pixels (RGBA or RGB) -> staging texture (RGBA) - CPU
	// 2) staging texture -> DX11 sender texture (RGBA) CopyResource - GPU
	//
	// RGBA :
	//   6.2 msec @ 3840 x 2160
	//   1.6 msec @ 1920 x 1080
	// RGBA Using UpdateSubresource (pixels > texture) instead of pixels > staging > texture
	//   5.8 msec @ 3840 x 2160
	//   1.5 msec @ 1920 x 1080
	// RGB :
	//   10 msec @ 3840 x 2160
	//   2.7 msec @ 1920 x 1080
	//
	// Access the sender shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		// Map the staging texture and write pixels to it (CPU)
		WritePixelData(pixels, m_pStaging[0], width, height, glFormat, bInvert);
		// Copy from the staging texture to the sender shared texture (GPU)
		spoutdx.GetDX11Context()->CopyResource(m_pSharedTexture, m_pStaging[0]);
		spoutdx.Flush();
		frame.SetNewFrame();
		frame.AllowTextureAccess(m_pSharedTexture);
		return true;
	}
	return false;

} // end WriteDX11pixels

//---------------------------------------------------------
// Function: ReadDX11pixels
//
// Receive from a sender via DX11 staging textures to an rgba or rgb buffer of variable size
// A new shared texture pointer (m_pSharedTexture) is retrieved if the sender changed
bool spoutGL::ReadDX11pixels(unsigned char * pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!pixels)
		return false;

	if (!CheckStagingTextures(width, height, 2)) {
		return false;
	}

	// No new frame, do not block
	if (!frame.GetNewFrame())
		return true;
	
	// If the sender has produced a new frame.
	// Read from the sender GPU texture to CPU pixels via two staging textures

	// Access the sender shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		m_Index = (m_Index + 1) % 2;
		m_NextIndex = (m_Index + 1) % 2;
		// Copy from the sender's shared texture to the first staging texture
		spoutdx.GetDX11Context()->CopyResource(m_pStaging[m_Index], m_pSharedTexture);
		// Map and read from the second while the first is occupied
		ReadPixelData(m_pStaging[m_NextIndex], pixels, m_Width, m_Height, glFormat, bInvert);
		// Allow access to the shared texture
		frame.AllowTextureAccess(m_pSharedTexture);
		return true;
	}

	return false;

}

//---------------------------------------------------------
// Function: WritePixelData
//
// COPY FROM A USER RGBA/RGB/BGR PIXEL BUFFER TO A DX11 STAGING TEXTURE
// RGBA/RGB/BGRA/BGR supported
bool spoutGL::WritePixelData(const unsigned char* pixels, ID3D11Texture2D* pStagingTexture,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!spoutdx.GetDX11Context() || !pStagingTexture || !pixels)
		return false;

	// glFormat = GL_RGB      0x1907
	// glFormat = GL_RGBA     0x1908
	// glFormat = GL_BGR_EXT  0x80E0
	// glFormat = GL_BGRA_EXT 0x80E1
	//
	// m_dwFormat = 28 RGBA staging textures
	// m_dwFormat = 87 BGRA staging textures

	// Map the resource so we can access the pixels
	D3D11_MAPPED_SUBRESOURCE mappedSubResource{};
	// Make sure all commands are done before mapping the staging texture
	spoutdx.Flush();
	// Map waits for GPU access
	const HRESULT hr = spoutdx.GetDX11Context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {
		//
		// Copy from the pixel buffer to the staging texture
		//
		// The shared texture format is BGRA or RGBA and the staging textures are the same format.
		// If the texture format is BGRA and the receiving pixel buffer is RGBA/RGB or vice-versa,
		// the data has to be converted from BGRA to RGBA/RGB or RGBA to BGRA/BGR during the pixel copy.
		//
		if (glFormat == GL_RGBA) { // RGBA pixel buffer
			if (m_dwFormat == 28) // RGBA staging textures
				spoutcopy.rgba2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgra((const void *)pixels, mappedSubResource.pData, 
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGRA_EXT) { // BGRA pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgra((const void *)pixels, mappedSubResource.pData,
					width, height, width * 4, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, width*4, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_RGB) { // RGB pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgb2rgba((const void *)pixels, mappedSubResource.pData, 
					width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgb2bgra((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGR_EXT) { // BGR pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.bgr2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgb2rgba((const void *)pixels, mappedSubResource.pData,
					width, height, mappedSubResource.RowPitch, bInvert);
		}
		spoutdx.GetDX11Context()->Unmap(pStagingTexture, 0);

		return true;

	} // endif DX11 map OK

	return false;

} // end WritePixelData


//---------------------------------------------------------
// Function: ReadPixelData
//
// COPY FROM A DX11 STAGING TEXTURE TO A USER RGBA/RGB/BGR PIXEL BUFFER
//
bool spoutGL::ReadPixelData(ID3D11Texture2D* pStagingTexture, unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!spoutdx.GetDX11Context() || !pStagingTexture || !pixels)
		return false;

	// glFormat = GL_RGB      0x1907
	// glFormat = GL_RGBA     0x1908
	// glFormat = GL_BGR_EXT  0x80E0
	// glFormat = GL_BGRA_EXT 0x80E1
	//
	// m_dwFormat = 28 RGBA staging textures
	// m_dwFormat = 87 BGRA staging textures

	// Map the resource so we can access the pixels
	D3D11_MAPPED_SUBRESOURCE mappedSubResource{};
	// Make sure all commands are done before mapping the staging texture
	spoutdx.Flush();
	// Map waits for GPU access
	const HRESULT hr = spoutdx.GetDX11Context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {
		//
		// Copy from staging texture to the pixel buffer
		//
		// The shared texture format is BGRA or RGBA and the staging textures are the same format.
		// If the texture format is BGRA and the receiving pixel buffer is RGBA/RGB or vice-versa,
		// the data has to be converted from BGRA to RGBA/RGB or RGBA to BGRA/BGR during the pixel copy.
		//
		if (glFormat == GL_RGBA) { // RGBA pixel buffer
			if (m_dwFormat == 28) // RGBA staging textures
				spoutcopy.rgba2rgba(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgra(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGRA_EXT) { // BGRA pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgra(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgba(mappedSubResource.pData, pixels, width, height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_RGB) { // RGB pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2rgb(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2bgr(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
		}
		else if (glFormat == GL_BGR_EXT) { // BGR pixel buffer
			if (m_dwFormat == 28)
				spoutcopy.rgba2bgr(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
			else
				spoutcopy.rgba2rgb(mappedSubResource.pData, pixels, m_Width, m_Height, mappedSubResource.RowPitch, bInvert);
		}

		spoutdx.GetDX11Context()->Unmap(pStagingTexture, 0);

		return true;
	} // endif DX11 map OK

	return false;

} // end ReadPixelData


//---------------------------------------------------------
// Function: CheckStagingTextures
//
// Create class staging textures for changed size or if they do not exist yet
// Two are available but only one can be allocated to save memory
// Format is the same as the shared texture - m_dwFormat
bool spoutGL::CheckStagingTextures(unsigned int width, unsigned int height, int nTextures)
{
	if (!spoutdx.GetDX11Device()) {
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};

	if (m_pStaging[0]) {

		// Get the size to test for change
		m_pStaging[0]->GetDesc(&desc);
		if (desc.Width != width || desc.Height != height) {
			// Staging textures must not be mapped before release
			if (m_pStaging[0]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[0]);
			if (m_pStaging[1]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[1]);
			m_pStaging[0] = nullptr;
			m_pStaging[1] = nullptr;

			// Flush context to avoid deferred release
			spoutdx.Flush();

			// Drop through to create new textures
		}
		else {
			return true;
		}
	}

	if (!spoutdx.CreateDX11StagingTexture(spoutdx.GetDX11Device(), width, height, (DXGI_FORMAT)m_dwFormat, &m_pStaging[0]))
		return false;

	if (nTextures > 1) {
		if (!spoutdx.CreateDX11StagingTexture(spoutdx.GetDX11Device(), width, height, (DXGI_FORMAT)m_dwFormat, &m_pStaging[1]))
			return false;
	}

	// Update class width and height
	m_Width = width;
	m_Height = height;

	// Reset staging texture index
	m_Index = 0;
	m_NextIndex = 0;

	// Also reset PBO index
	PboIndex = 0;
	NextPboIndex = 0;

	// Did something go wrong somehow
	if (!m_pStaging[0])
		return false;
	if (nTextures > 1 && !m_pStaging[1])
		return false;

	return true;

} // end CheckStagingTextures


//
// Memoryshare functions - receive only
//

//---------------------------------------------------------
// Function: ReadMemoryTexture
//
// Read rgba shared memory to texture pixel data
//
bool spoutGL::ReadMemoryTexture(const char* sendername, GLuint TexID, GLuint TextureTarget,
	unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	// Open a shared memory map if it not already
	if (!memoryshare.Name()) {
		// Create a name for the map from the sender name
		std::string namestring = sendername;
		namestring += "_map";
		// Try to open the shared memory map.
		if (!memoryshare.Open(namestring.c_str())) {
			// No map
			return false;
		}
		SpoutLogNotice("SpoutSharedMemory::ReadMemoryTexture - opened sender memory map [%s]", memoryshare.Name());
	}

	const char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("SpoutSharedMemory::ReadMemoryTexture - no buffer lock");
		return false;
	}

	bool bRet = true; // Error only if pixel read fails

	// Query a new frame and read pixels while the buffer is locked
	if (bInvert) {
		// Create or resize a local OpenGL texture
		CheckOpenGLTexture(m_TexID, GL_RGBA, width, height);
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

	memoryshare.Unlock();

	return bRet;

}

//---------------------------------------------------------
// Function: ReadMemoryPixels
//
// Read shared memory to image pixels
//
bool spoutGL::ReadMemoryPixels(const char* sendername, unsigned char* pixels,
	unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!pixels || glFormat != GL_RGBA) {
		SpoutLogError("spoutGLDXinterop::ReadMemoryPixels - no data or incorrect format");
		return false;
	}

	// No new frame, do not block
	if (!frame.GetNewFrame())
		return true;

	// Open a shared memory map if it not already
	if (!memoryshare.Name()) {
		// Create a name for the map from the sender name
		std::string namestring = sendername;
		namestring += "_map";
		// Open the shared memory map.
		if (!memoryshare.Open(namestring.c_str())) {
			return false;
		}
		SpoutLogNotice("SpoutSharedMemory::ReadMemoryPixels - opened sender memory map [%s]", memoryshare.Name());
	}

	const char* pBuffer = memoryshare.Lock();
	if (!pBuffer) {
		SpoutLogError("SpoutSharedMemory::ReadMemoryPixels - no buffer lock");
		return false;
	}

	// Query a new frame and read pixels while the buffer is locked
	// Read pixels from shared memory
	spoutcopy.CopyPixels((unsigned char*)pBuffer, pixels, width, height, glFormat, bInvert);

	memoryshare.Unlock();

	return true;

}

//---------------------------------------------------------
// Function: WriteMemoryPixels
//
// Write image pixels to shared memory
//
bool spoutGL::WriteMemoryPixels(const char *sendername, const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	if (!pixels || glFormat != GL_RGBA) {
		SpoutLogError("spoutGLDXinterop::WriteMemoryPixels - no data or incorrect format");
		return false;
	}

	// Create a shared memory map if it does not exist yet
	char* pBuffer = nullptr;
	if (memoryshare.Size() == 0) {
		// Create a name for the map from the sender name
		std::string namestring = sendername;
		namestring += "_map";
		if (memoryshare.Create(sendername, width*4*height) != SPOUT_CREATE_FAILED) {
			SpoutLogError("SpoutSharedMemory::WriteMemoryPixels - could not create shared memory");
			return false;
		}
		pBuffer = memoryshare.Lock();
		if (!pBuffer) {
			SpoutLogError("SpoutSharedMemory::WriteMemoryPixels - no buffer lock");
			return false;
		}
	}

	// Write pixel data to shared memory
	spoutcopy.CopyPixels(pixels, (unsigned char *)pBuffer, width, height, glFormat, bInvert);

	memoryshare.Unlock();

	return true;

}


//
// Directx 11
//

//---------------------------------------------------------
bool spoutGL::OpenDirectX11(ID3D11Device* pDevice)
{
	return spoutdx.OpenDirectX11(pDevice);
}

//---------------------------------------------------------
ID3D11Device* spoutGL::GetDX11Device()
{
	return spoutdx.GetDX11Device();
}

//---------------------------------------------------------
ID3D11DeviceContext* spoutGL::GetDX11Context()
{
	return spoutdx.GetDX11Context();
}

//---------------------------------------------------------
void spoutGL::CleanupDirectX()
{
	// DirectX 9 not supported >= 2.007
	CleanupDX11();
}

//---------------------------------------------------------
void spoutGL::CleanupDX11()
{
	if (spoutdx.GetDX11Device()) {

		SpoutLogNotice("spoutGL::CleanupDX11()");

		// Reference count warnings are in the SpoutDirectX class

		// Release linked DirectX shared texture
		if (m_pSharedTexture) {
			SpoutLogNotice("    Releasing shared texture");
			// Release interop before releasing the texture
			// Requires openGL context
			if (m_hInteropDevice && m_hInteropObject) {
				if (!CleanupInterop()) {
					SpoutLogWarning("    GL/DX Interop could not be released");
				}
			}
			spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pSharedTexture);
		}

		// Important to set pointer to NULL or it will crash if released again
		m_pSharedTexture = nullptr;

		// Re-set shared texture handle
		m_dxShareHandle = nullptr;

		// Release staging texture if they have been used
		if (m_pStaging[0]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[0]);
		if (m_pStaging[1]) spoutdx.ReleaseDX11Texture(spoutdx.GetDX11Device(), m_pStaging[1]);
		m_pStaging[0] = nullptr;
		m_pStaging[1] = nullptr;
		m_Index = 0;
		m_NextIndex = 0;

		// Flush context to avoid deferred release
		spoutdx.Flush();

		// 12.11.18 - To avoid memory leak with dynamic objects
		//            they must always be freed, not only on exit.
		//            Device recreated for a new sender.
		// Releases immediate context and device in the SpoutDirectX class
		// spoutdx.GetDX11Context() and spoutdx.GetDX11Device() are copies of these
		spoutdx.CloseDirectX11();

	}
	else {
		SpoutLogNotice("spoutGL::CleanupDX11() - device closed");
	}

}

//
// Extensions and availability
//

//---------------------------------------------------------
bool spoutGL::LoadGLextensions()
{
	// Return if already loaded
	if (m_caps > 0) {
		SpoutLogNotice("spoutGL::LoadGLextensions - already loaded");
		return true;
	}

	// Needs an OpenGL context
	if (!wglGetCurrentContext()) {
		SpoutLogWarning("spoutGL::LoadGLextensions - no OpenGL context");
		return false;
	}

	m_bFBOavailable   = false;
	m_bGLDXavailable  = false;
	m_bGLMEMavailable = false;
	m_bBLITavailable  = false;
	m_bSWAPavailable  = false;
	m_bBGRAavailable  = false;
	m_bCOPYavailable  = false;
	m_bCONTEXTavailable = false;

	m_caps = loadGLextensions(); // in spoutGLextensions

	if (m_caps == 0) {
		SpoutLogError("spoutGL::LoadGLextensions failed");
		m_bPBOavailable = false;
		return false;
	}

	if (m_caps & GLEXT_SUPPORT_FBO)
		m_bFBOavailable = true;

	// FBO not available is terminal
	if (!m_bFBOavailable) {
		SpoutLogError("spoutGL::LoadGLextensions - no FBO extensions available");
		m_bPBOavailable = false; // No PBO support either - over-ride user selection
		return false;
	}

	if (m_caps & GLEXT_SUPPORT_NVINTEROP)
		m_bGLDXavailable = true; // Nvidia GL/DX interop
	// Simulate failure for testing
	// m_bGLDXavailable = false;

	// GL memory interop
	if (m_caps & GLEXT_SUPPORT_GLMEMORY)
		m_bGLMEMavailable = true;
	// Simulate failure for testing
	// m_bGLMEMavailable = false;

	if (m_caps & GLEXT_SUPPORT_FBO_BLIT)
		m_bBLITavailable = true;

	if (m_caps & GLEXT_SUPPORT_SWAP)
		m_bSWAPavailable = true;

	if (m_caps & GLEXT_SUPPORT_BGRA)
		m_bBGRAavailable = true;

	if (m_caps & GLEXT_SUPPORT_COPY)
		m_bCOPYavailable = true;

	if (m_caps & GLEXT_SUPPORT_CONTEXT)
		m_bCONTEXTavailable = true;

	// Test PBO availability unless user has checked buffering OFF (sets m_bPBOavailable false)
	// m_bPBOavailable can also be set by the application with SetBufferMode()
	if (m_bPBOavailable) { // User selected buffering
		if (!(m_caps & GLEXT_SUPPORT_PBO)) {
			SpoutLogWarning("spoutGL::LoadGLextensions - pbo extensions not available");
			m_bPBOavailable = false;
		}
	}
	else {
		SpoutLogWarning("spoutGL::LoadGLextensions - pbo functions disabled by settings");
	}

	if (!m_bGLDXavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - Nvidia interop extensions not available");
	if(!m_bGLMEMavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - Khronos GL memory extensions not available");
	else if (!m_bBLITavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - fbo blit extension not available");
	else if (!m_bSWAPavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - sync control extensions not available");
	else if (!m_bBGRAavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - bgra extension not available");
	else if (!m_bCOPYavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - copy extensions not available");
	else if (!m_bCONTEXTavailable)
		SpoutLogWarning("spoutGL::LoadGLextensions - context extension not available");
	else
		SpoutLogNotice("spoutGL::LoadGLextensions - all extensions available");

	m_bExtensionsLoaded = true;

	return true;
}

//---------------------------------------------------------
bool spoutGL::IsGLDXavailable()
{
	return m_bGLDXavailable;
}

//---------------------------------------------------------
bool spoutGL::IsBLITavailable()
{
	return m_bBLITavailable;
}

//---------------------------------------------------------
bool spoutGL::IsSWAPavailable()
{
	return m_bSWAPavailable;
}

//---------------------------------------------------------
bool spoutGL::IsBGRAavailable()
{
	return m_bBGRAavailable;
}

//---------------------------------------------------------
bool spoutGL::IsCOPYavailable()
{
	return m_bCOPYavailable;
}

//---------------------------------------------------------
bool spoutGL::IsPBOavailable()
{
	return m_bPBOavailable;
}

//---------------------------------------------------------
bool spoutGL::IsGLMEMavailable()
{
	return m_bGLMEMavailable;
}

//---------------------------------------------------------
bool spoutGL::IsCONTEXTavailable()
{
	return m_bCONTEXTavailable;
}

//---------------------------------------------------------
// OpenGL version - 3.0, 4.0, 4.6 etc
float spoutGL::GetGLversion()
{
	float glversion = 0.0f;
	GLint major = 0;
	GLint minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	glversion = (float)major + (float)minor/10.0f;

	return glversion;
}

// 
// Legacy OpenGL functions
//

#ifdef legacyOpenGL

//---------------------------------------------------------
void spoutGL::SaveOpenGLstate(unsigned int width, unsigned int height, bool bFitWindow)
{
	float dim[4]{};
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
	if (bFitWindow) {
		// Scale both width and height to the current viewport size
		vpScaleX = dim[2] / (float)width;
		vpScaleY = dim[3] / (float)height;
		vpWidth = (float)width  * vpScaleX;
		vpHeight = (float)height * vpScaleY;
		vpx = vpy = 0;
	}
	else {
		// Preserve aspect ratio of the sender
		// and fit to the width or the height
		vpWidth = dim[2]{};
		vpHeight = ((float)height / (float)width)*vpWidth;
		if (vpHeight > dim[3]) {
			vpHeight = dim[3]{};
			vpWidth = ((float)width / (float)height)*vpHeight;
		}
		vpx = (int)(dim[2] - vpWidth) / 2;;
		vpy = (int)(dim[3] - vpHeight) / 2;
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


void spoutGL::RestoreOpenGLstate()
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

#endif

//
// Utility
//

//---------------------------------------------------------
// Given a DeviceKey string from a DisplayDevice
// read all the information about the adapter.
// Only used by this class.
bool spoutGL::OpenDeviceKey(const char* key, int maxsize, char* description, char* version)
{
	if (!key)
		return false;

	// Extract the subkey from the DeviceKey string
	HKEY hRegKey = nullptr;
	DWORD dwSize = 0;
	DWORD dwKey = 0;

	char output[256]{};
	strcpy_s(output, 256, key);
	const char *found = strstr(output, "System");
	if (!found)
		return false;
	std::string SubKey = found;

	// Convert all slash to double slash using a C++ string function
	// to get subkey string required to extract registry information
	for (unsigned int i = 0; i < SubKey.length(); i++) {
		if (SubKey.at(i) == '\\') {
			SubKey.insert(i, 1, '\\');
			++i; // Skip inserted char
		}
	}

	// Open the key to find the adapter details
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey.c_str(), 0, KEY_READ, &hRegKey) == 0) {

		dwSize = 256;
		// Adapter name
		if (RegQueryValueExA(hRegKey, "DriverDesc", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			strcpy_s(description, (rsize_t)maxsize, output);
		}
		if (RegQueryValueExA(hRegKey, "DriverVersion", NULL, &dwKey, (BYTE*)output, &dwSize) == 0) {
			// Find the last 6 characters of the version string then
			// convert to a float and multiply to get decimal in the right place
			sprintf_s(output, 256, "%5.2f", atof(output + strlen(output) - 6)*100.0);
			strcpy_s(version, (rsize_t)maxsize, output);
		} // endif DriverVersion
		RegCloseKey(hRegKey);
	} // endif RegOpenKey

	return true;
}

//---------------------------------------------------------
void spoutGL::trim(char* s) {
	
	char* p = s;
	if (!p) return;

	int l = (int)strlen(p);

	while (isspace(p[l - 1])) p[--l] = 0;
	while (*p && isspace(*p)) ++p, --l;

	// Casting first avoids warning C26451: Arithmetic overflow with VS2022 code review
	// https://docs.microsoft.com/en-us/visualstudio/code-quality/c26451
	const size_t l1 = (size_t)l + 1;
	memmove(s, p, l1);
}

//
// Errors
//

void spoutGL::PrintFBOstatus(GLenum status)
{
	char tmp[256]{};
	sprintf_s(tmp, 256, "FBO status error %u (0x%.7X) - ", status, status);
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
	else if (status == GL_INVALID_ENUM)
		strcat_s(tmp, 256, "GL_INVALID_ENUM");
	else if (status == GL_FRAMEBUFFER_UNDEFINED)
		strcat_s(tmp, 256, "GL_FRAMEBUFFER_UNDEFINED");
	// else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
		// strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT\n");

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		// strcat_s(tmp, 256, "Unknown status code");
		// Additionally, if an error occurs, zero is returned.
		SpoutLogError("%s", tmp);
	}
	GLerror();
}

bool spoutGL::GLerror() {
	GLenum err = GL_NO_ERROR;
	int nError = 0;
	bool bError = false;
	while ((err = glGetError()) != GL_NO_ERROR) {
		SpoutLogError("GL error (%d) = %u (0x%.7X)", nError, err, err);
		nError++;
		bError = true;
#ifdef USE_GLEW
		// gluErrorString needs Glu.h and glu32.lib (or glew)
		SpoutLogError("GL error (%d) = %d (0x%.7X) %s\n", nError, err, err, gluErrorString(err));
#endif
	}
	return bError;
}

//
// Group: User registry settings recorded by "SpoutSettings"
//
// User settings are retrieved in the constructor by registry read
// and can be set for the application (except max senders which must be global)
//

//---------------------------------------------------------
// Function: GetBufferMode
// Get user buffering mode
//
bool spoutGL::GetBufferMode()
{
	return m_bPBOavailable;
}

//---------------------------------------------------------
// Function: SetBufferMode
// Set application buffering mode
void spoutGL::SetBufferMode(bool bActive)
{
	if (m_bExtensionsLoaded) {
		if (bActive) {
			if (m_caps & GLEXT_SUPPORT_PBO) {
				m_bPBOavailable = true;
			}
		}
		else {
			m_bPBOavailable = false;
		}
	}
	else {
		m_bPBOavailable = false;
	}
}

//---------------------------------------------------------
// Function: GetBuffers
// Get user number of pixel buffers
int spoutGL::GetBuffers()
{
	return m_nBuffers;
}

//---------------------------------------------------------
// Function: SetBuffers
// Set application number of pixel buffers
void spoutGL::SetBuffers(int nBuffers)
{
	m_nBuffers = nBuffers;
}

//---------------------------------------------------------
// Function: GetMaxSenders
// Get user Maximum senders allowed
int spoutGL::GetMaxSenders()
{
	return sendernames.GetMaxSenders();
}

//---------------------------------------------------------
// Function: SetMaxSenders
// Set user Maximum senders allowed
void spoutGL::SetMaxSenders(int maxSenders)
{
	// Setting must be global for all applications
	sendernames.SetMaxSenders(maxSenders);
}

//
// Group: Retained for 2.006 compatibility
//

//---------------------------------------------------------
// Function: GetDX9
// Get user DX9 mode
bool spoutGL::GetDX9()
{
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);
	return (dwDX9 == 1);
}

//---------------------------------------------------------
// Function: SetDX9
// Set user DX9 mode
bool spoutGL::SetDX9(bool bDX9)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", (DWORD)bDX9);
}

//---------------------------------------------------------
// Function: GetMemoryShareMode
// Get user memory share mode
bool spoutGL::GetMemoryShareMode()
{
	DWORD dwMem = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem);
	return (dwMem == 1);
}

//---------------------------------------------------------
// Function: SetMemoryShareMode
// Set user memory share mode
bool spoutGL::SetMemoryShareMode(bool bMem)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", (DWORD)bMem);
}

//---------------------------------------------------------
// Function: GetCPUmode
// Get user CPU mode
bool spoutGL::GetCPUmode()
{
	DWORD dwCpu = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwCpu);
	return (dwCpu == 1);
}

//---------------------------------------------------------
// Function: SetCPUmode
// Set user CPU mode
bool spoutGL::SetCPUmode(bool bCPU)
{
	return WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", (DWORD)bCPU);
}

// Function: GetShareMode
// Get user share mode
//  0 - texture, 1 - memory, 2 - CPU
int spoutGL::GetShareMode()
{
	DWORD dwMem = 0;
	DWORD dwCPU = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem);
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", &dwCPU);

	if (dwCPU > 0) {
		return 2;
	}
	if (dwMem > 0) {
		return 1;
	}

	// 0 : Texture share default
	return 0;

}

//---------------------------------------------------------
// Function: SetShareMode
// Set user share mode
//  0 - texture, 1 - memory, 2 - CPU
void spoutGL::SetShareMode(int mode)
{
	switch (mode) {

	case 1: // Memory
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 1);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 0);
		break;
	case 2: // CPU
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 0);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 1);
		break;
	default: // 0 - Texture
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", 0);
		WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "CPU", 0);
		break;
	}
}

//
// Group: Information
//

//---------------------------------------------------------
// Function: GetHostPath
// The path of the host that produced the sender
//
// Retrieved from the description string in the sender info memory map
bool spoutGL::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	SharedTextureInfo info{};
	if (!sendernames.getSharedInfo(sendername, &info)) {
		// Just quit if the key does not exist
		SpoutLogError("spoutGL::GetHostPath - could not get sender info [%s]", sendername);
		return false;
	}

	int n = maxchars;
	if (n > 256) n = 256; // maximum field width in shared memory (128 wide chars)
	strcpy_s(hostpath, n, (char*)info.description);

	return true;
}

//---------------------------------------------------------
// Function: GetVerticalSync
// Vertical sync status
int spoutGL::GetVerticalSync()
{
	// Needs OpenGL context
	if (wglGetCurrentContext()) {
		// needed for both sender and receiver
		if (m_bSWAPavailable) {
			return(wglGetSwapIntervalEXT());
		}
	}
	return 0;
}

//---------------------------------------------------------
// Function: SetVerticalSync
// Specifies the minimum number of video frame periods per buffer swap
//
//   1 - wait for 1 cycle vertical refresh
//
//   0 - buffer swaps are not synchronized to a video frame
//
//  -1 - adaptive vsync
//
// "Adaptive" enables v-blank synchronisation when the frame rate
// is higher than the sync rate to eliminate tearing, but disables
// synchronisation when the frame rate drops below the sync rate
// to minimize stuttering.
//
// Note that driver settings for Vertical sync "on", "off" or "adaptive"
// over-ride this function and should be set to "Use application setting"
// if application control is required.
//
// https://www.khronos.org/opengl/wiki/Swap_Interval#Adaptive_Vsync
//
bool spoutGL::SetVerticalSync(int interval)
{
	if (wglGetCurrentContext()) {
		if (m_bSWAPavailable) {
			if (wglSwapIntervalEXT(interval) == TRUE) {
				return true;
			}
			else {
				SpoutLogWarning("spoutGL::SetVerticalSync(%d) - Error %d\n", interval, GetLastError());
			}
		}
	}

	return false;
}

//---------------------------------------------------------
// Function: GetSpoutVersion
// Get Spout version
//
// Version number is retrieved from the registry at class initialization
// Integer number 2005, 2006, 2007 etc. 0 for earlier than 2.005.
// Set by the Spout installer for 2.005/2.006 or by SpoutSettings for 2.007 and later.
//
// Registry version number now replaced by std::string GetSDKversion() in SpoutUtils
// Version number is created in constructor
// For example : 2007 (2.007) or 2007009 (2.007.009)
//
int spoutGL::GetSpoutVersion()
{
	return m_SpoutVersion;
}

//
// Group: Utilities
//

//---------------------------------------------------------
// Function: CopyTexture
//   Copy OpenGL texture with optional invert
//   Textures must be the same size but can have different formats.
//
bool spoutGL::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget, unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	//
	// Single fbo blit
	//

	// Create an fbo if not already
	if (m_fbo == 0)
		glGenFramebuffersEXT(1, &m_fbo);

	// Bind the FBO for both read and write (READ_FRAMEBUFFER_EXT and DRAW_FRAMEBUFFER_EXT)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	// Attach the Source texture to the first attachment point of the color buffer
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
	// Set attachment 0 to read for the copy
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Attach destination texture to second attachment point
	glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, DestTarget, DestID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

	// Check the fbo status for completeness first
	const GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
		if (m_bBLITavailable) {
			if (bInvert) {
				// Copy one texture buffer to the other while flipping upside down
				// If copying to or from the shared OpenGL texture
				// OpenGL and DirectX have different texture origins.
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
			// No fbo blit extension (< OpenGL v3.0)
			// Copy from the fbo (source texture attached) to the dest texture
			// Invert not available
			glPixelStorei(DestTarget, 1); // Alignment in case of RGB data
			glBindTexture(DestTarget, DestID);
			glCopyTexSubImage2D(DestTarget, 0, 0, 0, 0, 0, width, height);
			glBindTexture(DestTarget, 0);
			glPixelStorei(DestTarget, 4); // Restore default alignment
		}
	}
	else {
		// TODO : prevent repeats
		PrintFBOstatus(status);
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
		return false;
	}

	// Restore default draw
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Unbind the color attachments
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1, DestTarget, 0, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, SourceTarget, 0, 0);

	// Restore the previous fbo - default is 0
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	// To show errors for code debugging
	// GLerror();

	return true;

} // end CopyTexture

//---------------------------------------------------------
// Function: ReadTextureData
// Copy OpenGL texture data to a pixel buffer
//
// SourceID   : Source texture format :
//              GL_RGB, GL_BGR, GL_RGBA, GL_RGBA8, GL_BGRA
//              GL_RGBA16, GL_RGBA16F, GL_RGBA32F, GL_RGB10_A2
// data       : Destination pixel data buffer
// width      : Buffer width
// height     : Buffer height
// rowpitch   : Data line length of the buffer in bytes
//              to allow for pitch greater than the width for DX11 staging textures
//              Can be zero for RGB or RGBA formats
// dataformat : Specifies the format of the pixel data
//              GL_RGBA, GL_BGRA GL_RGB, GL_BGR 
// datatype   : Specifes the pixel data type
//              GL_FLOAT, GL_UNSIGNED_SHORT, GL_UNSIGNED_BYTE (default)
//              Pixel data type must match with the texture format
// bInvert    : Flip the image (default false)
// HostFBO    : Current fbo binding, 0 if none (default)
// 
bool spoutGL::ReadTextureData(GLuint SourceID, GLuint SourceTarget,
	void* data, unsigned int width, unsigned int height, unsigned int rowpitch,
	GLenum dataformat, GLenum datatype, bool bInvert, GLuint HostFBO)
{
	// Pixel data type must match the source texture format
	GLint texformat = GLformat(SourceID, SourceTarget);

	if (texformat == GL_RGBA || texformat == GL_RGBA8 || texformat == GL_BGRA_EXT
		|| texformat == GL_RGB || texformat == GL_BGR_EXT) {
		if (datatype != GL_UNSIGNED_BYTE)
			return false;
	}
	else if (texformat == GL_RGBA16 || texformat == GL_RGB10_A2) {
		if (datatype != GL_UNSIGNED_SHORT)
			return false;
	}
	else if (texformat == GL_RGBA16F || texformat == GL_RGBA32F) {
		if (datatype != GL_FLOAT)
			return false;
	}

	GLuint TexID = 0; // Local texture for fbo invert

	// Components per pixel
	GLint nchannels = 4; // RGBA defualt
	if (dataformat == GL_RGB || dataformat == GL_BGR_EXT)
		nchannels = 3;

	// If row pitch argument is zero, calculate pitch in bytes
	// Casting first avoids warning C26451: Arithmetic overflow
	GLint pitch = (GLint)rowpitch;
	if (rowpitch == 0)
		pitch = (GLint)width*nchannels; // RGB or RGBA

	// The following format arguments are accepted for glReadPixels and glGetTexImage:
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glReadPixels.xhtml
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
	//     GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA
	GLenum glformat = GL_RGBA;
	if (dataformat == GL_RGB || dataformat == GL_BGR_EXT || dataformat == GL_BGRA_EXT)
		glformat = dataformat;

	// No fbo extensions (< OpenGL v3.0) or RGB/BGR pixel data format.
	// Can be 4 component texture if fbo extensions are not available
	if (!m_bFBOavailable || dataformat == GL_RGB || dataformat == GL_BGR_EXT) {
		//
		// Read the texture to pixels using glGetTexImage
		// with GLformat specified as the pixel format for the returned data.
		// RGB/BGR input textures are accepted
		// https://learn.microsoft.com/en-us/windows/win32/opengl/glgetteximage
		//    three-component textures are treated as RGBA buffers with red set to component zero,
		//    green set to component one, blue set to component two, and alpha set to zero.
		//
		if (bInvert	&& (dataformat == GL_RGB || dataformat == GL_BGR_EXT || dataformat == GL_RGBA)) { // rgba or rgb pixels only

			// Intermediate invert buffer
			const unsigned char* src = nullptr;

			// Create unsigned long variables for temp src char array
			const unsigned long WxHx3 = { width * height * 3 };
			const unsigned long WxHx4 = { width * height * 4 };

			if (dataformat == GL_RGB || dataformat == GL_BGR_EXT)
				src = new unsigned char[WxHx3];
			else
				src = new unsigned char[WxHx4];

			glPixelStorei(SourceTarget, 1); // Alignment in case of RGB data
			glBindTexture(SourceTarget, SourceID);
			glGetTexImage(SourceTarget, 0, glformat, datatype, (void*)src);
			glBindTexture(SourceTarget, 0);
			glPixelStorei(SourceTarget, 4); // Restore default alignment

			// Flip the pixel buffer
			spoutcopy.FlipBuffer(src, (unsigned char*)data, width, height, dataformat);

			delete[] src;
		}
		else {
			glPixelStorei(SourceTarget, 1);
			glBindTexture(SourceTarget, SourceID);
			glGetTexImage(SourceTarget, 0, glformat, datatype, data);
			glBindTexture(SourceTarget, 0);
			glPixelStorei(SourceTarget, 4);
		}
		return true;
	}
	else {

		//
		// Four component textures
		//

		GLenum status = 0;

		// Create a local fbo if not already
		if (m_fbo == 0)
			glGenFramebuffersEXT(1, &m_fbo);

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
			// Read from attachment point 0
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
			// Attach the Source texture to point 0 for read
			glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, SourceTarget, SourceID, 0);
		}

		// For invert blit to a local texture first 
		if (bInvert && m_bBLITavailable) {
			// Create a local OpenGL texture
			InitTexture(TexID, glformat, width, height);
			// Draw to attachment point 1
			glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
			// Attach the texture we write into (the local texture) to attachment point 1
			glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, TexID, 0);
		}

		// Check read/draw fbo for completeness
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
			if (bInvert && m_bBLITavailable) {
				// copy the source texture (0) to the local texture (1) while flipping upside down 
				glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				// Bind local fbo for read
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo);
				// Read from attachment point 1
				glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
				// Read pixels from the local texture
				// Allow for pitch greater than the width for DX11 staging textures
				// Pitch is destination line length in bytes. 
				// Define the number of pixels in a row for 4 component pixels
				glPixelStorei(GL_PACK_ROW_LENGTH, pitch/4);
				// Read to the pixel buffer in the required format and data type
				glReadPixels(0, 0, width, height, glformat, datatype, data);
				glPixelStorei(GL_PACK_ROW_LENGTH, 0);
				// The local texture is no longer required
				glDeleteTextures(1, &TexID);
			}
			else {
				// No invert
				// or no fbo blit extension, although unlikely (< OpenGL v3.0).
				// Read from the source texture attachment point 0
				// This will be the local fbo if a texture ID was passed in
				// Allow for pitch greater than the width for DX11 staging textures
				glPixelStorei(GL_PACK_ROW_LENGTH, pitch/4);
				// Read to the pixel buffer in the required format and data type
				glReadPixels(0, 0, width, height, glformat, datatype, data);
				glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			}
		}
		else {
			PrintFBOstatus(status);
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
			return false;
		}

		// restore the previous fbo - default is 0
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	}

	return true;

} // end ReadTextureData

//---------------------------------------------------------
// Function: RemovePadding
// Remove line padding from a source image and crerate a destination image without padding
void spoutGL::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	spoutcopy.RemovePadding(source, dest, width, height, stride, glFormat);
}


//
// Group : DX11 texture copy versions
//
// - https://github.com/DashW/Spout2
//

//---------------------------------------------------------
// Function: ReadTexture
// Copy the sender DirectX shared texture
// Textures must be the same size and created with the same device
bool spoutGL::ReadTexture(ID3D11Texture2D** texture)
{
	// Only for DX11 mode
	if (!texture || !*texture || !spoutdx.GetDX11Context()) {
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};
	(*texture)->GetDesc(&desc);
	if (desc.Width != m_Width || desc.Height != m_Height) {
		return false;
	}

	// No new frame, do not block
	if (!frame.GetNewFrame())
		return true;

	// Copy the shared texture if the sender has produced a new frame
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		spoutdx.GetDX11Context()->CopyResource(*texture, m_pSharedTexture);
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return true;

} // end ReadTexture


//---------------------------------------------------------
// Function: WriteTexture
// Copy to the sender DirectX shared texture
// Textures must be the same size and created with the same device
bool spoutGL::WriteTexture(ID3D11Texture2D** texture)
{
	// Only for DX11
	if (!texture || !spoutdx.GetDX11Context()) {
		SpoutLogWarning("spoutGL::WriteTexture(ID3D11Texture2D** texture) failed");
		if (!texture)
			SpoutLogWarning("    ID3D11Texture2D** NULL");
		if (!spoutdx.GetDX11Context())
			SpoutLogVerbose("    pImmediateContext NULL");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc{};

	(*texture)->GetDesc(&desc);
	if (desc.Width != m_Width || desc.Height != m_Height) {
		SpoutLogWarning("spoutGL::WriteTexture(ID3D11Texture2D** texture) sizes do not match");
		SpoutLogWarning("    texture (%dx%d) : sender (%dx%d)", desc.Width, desc.Height, m_Width, m_Height);
		return false;
	}

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		spoutdx.GetDX11Context()->CopyResource(m_pSharedTexture, *texture);
		// Flush after update of the shared texture on this device
		spoutdx.Flush();
		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
		bRet = true;
	}

	return bRet;
}


//---------------------------------------------------------
// Function: WriteTextureReadback
// Copy a DX11 texture to the DX11 shared texture
// Copy the linked OpenGL texture back to an OpenGL texture
// Textures must be the same size and created with the same device
bool spoutGL::WriteTextureReadback(ID3D11Texture2D** texture,
	GLuint TextureID, GLuint TextureTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	// Zero offsets
	return WriteTextureReadback(texture, TextureID, TextureTarget,
		0, 0, width, height, bInvert, HostFBO);
}

// Copy a region of the DX11 texture
bool spoutGL::WriteTextureReadback(ID3D11Texture2D ** texture,
	GLuint TextureID, GLuint TextureTarget,
	unsigned int xoffset, unsigned int yoffset,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	if (!texture) {
		SpoutLogWarning("spoutGL::WriteTextureReadback : ID3D11Texture2D** NULL");
		return false;
	}

	// Make sure that Spout has been initialized using CreateSender
	// to set up a DirectX device and interop for GL/DX transfer
	if(!m_bInitialized) {
		SpoutLogWarning("spoutGL::WriteTextureReadback : Spout not intialized");
		return false;
	}

	bool bRet = false;
	D3D11_TEXTURE2D_DESC desc{};
	(*texture)->GetDesc(&desc);

	// Get the region to copy
	D3D11_BOX sourceRegion{};
	sourceRegion.left    = xoffset;
	sourceRegion.right   = xoffset+width;
	sourceRegion.top     = yoffset;
	sourceRegion.bottom  = yoffset+height;
	sourceRegion.front   = 0;
	sourceRegion.back    = 1;

	// Check the texture and region sizes
	if (sourceRegion.right >= desc.Width) sourceRegion.right = desc.Width;
	if (sourceRegion.bottom >= desc.Height) sourceRegion.bottom = desc.Height;

	// Wait for access to the shared texture
	if (frame.CheckTextureAccess(m_pSharedTexture)) {
		bRet = true;
		
		// Copy the DirectX texture region to the shared texture
		spoutdx.GetDX11Context()->CopySubresourceRegion(m_pSharedTexture, 0, 0, 0, 0, *texture, 0, &sourceRegion);

		// Flush after update of the shared texture on this device
		spoutdx.Flush();

		// Copy the linked OpenGL texture back to the user texture
		if (width != m_Width || height != m_Height) {
			SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) sizes do not match");
			SpoutLogWarning("    OpenGL texture (%dx%d) : sender [%s] (%dx%d)",
				width, height, m_SenderName, m_Width, m_Height);
			bRet = false;
		}
		else if (LockInteropObject(m_hInteropDevice, &m_hInteropObject) == S_OK) {
			// Copy the OpenGL shared texture (m_glTexture) to the user texture
			bRet = CopyTexture(m_glTexture, GL_TEXTURE_2D, TextureID, TextureTarget, width, height, bInvert, HostFBO);
			UnlockInteropObject(m_hInteropDevice, &m_hInteropObject);
			if (!bRet)
				SpoutLogWarning("spoutGL::WriteTextureReadback(ID3D11Texture2D** texture) readback failed");
		}

		// Increment the sender frame counter
		frame.SetNewFrame();
		// Release mutex and allow access to the texture
		frame.AllowTextureAccess(m_pSharedTexture);
	}

	return bRet;

}
