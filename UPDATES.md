===========================================\
12.06.24\
Update Master from Beta branch.

Version 2.007.014

Spout.cpp
- 14.01.24	- CheckSender - return false if OpenSpout fails
- 13.02.24	- SelectSenderPanel
 			  m_ShExecInfo.lpParameters : receiver graphics adapter index by default
- 19.04.24	- SelectSenderPanel - allow for unicode build for PROCESSENTRY32
- 24.04.24	- ReceiveImage - allow for multiple OpenGL formats
- 06.05.24	- SelectSenderPanel - removed unused "value" variable
- 21.05.24	- SetSenderName - move sender name increment to SenderNames class
- 22.05.24	- Add GetReceiverName
			  CheckSpoutPanel - do not to register twice if already registered
- 25.05.24	- SetSenderName - remove name increment
- 08.06.24	- Add GetSenderList
- 09.06.24	- SelectSender - use SpoutMessageBox if SelectSenderPanel fails
			  Add hwnd argument to centre MessageBox dialog if used. 

SpoutCopy.cpp

SpoutDirectX.cpp
- 18.02.24	- GetNumAdapters
		    Change adapter pointer to IDXGIAdapter1 and use EnumAdapters1
		    to identify and skip the Basic Render Driver adapter

SpoutFrameCount.cpp
- 31.12.23	- Add comments to clarify the purpose of "EnableFrameSync"

SpoutGL.cpp
- 26.02.24	- CreateInterop - check for dimensions out of bounds (> 16384)
- 06.03.24	- GLerror - add error number
- 03.04.24	- Add ReadTexturePixels for multiple format and RGB/RGBA textures
- 08.04.24	- ReadTexturePixels - check OpenGL texture size and return if different
			  PrintFBOstatus - no warning if complete
			  InitTexture - corrected internal format
			  CreateOpenGL - report version created
- 25.04.24	- Correct GLDXformat for default GL_RGBA
- 26.04.24	- GLformatName - revise names
- 06.05.24	- Add more logs for wglDX function failure

SpoutGLextensions.cpp
- 29.03.24	- Correct glUnmapBufferPROC as Glboolean
			  Correct glGenBuffersPROC - GLuint* buffers
- 19.04.24	- Add #ifndef for pre-defined constants WGL_CONTEXT_FLAGS
			  and GL consts that are not present in GL.h

SpoutReceiver.cpp
- 22.05.24	- Add GetReceiverName
- 08.06.24	- SelectSender - bool instead of void
			- Add GetSenderList
- 09.06.24	- SelectSender > spout SelectSender instead of SelectSenderPanel
			  Add hwnd argument to centre MessageBox dialog if used.

SpoutSender.cpp

SpoutSenderNames.cpp
- 21.05.24 - RegisterSenderName - increment existing sender name
		   CreateSender/RegisterSenderName remove const for name
- 22.05.24 - RegisterSenderName add newname condition for name increment
	
SpoutSharedMemory.cpp

SpoutUtils.cpp
- 28.12.23 - SpoutMessageBox - add MB_RIGHT for right aligned text
- 11.03.24 - Add MessageBox dialog with an edit control for text input
		    Add MessageBox dialog with a combo box control for item selection
			Update Taskdialog callback to create the controls and return input
- 19.03.24 - Add icon/button option for variable arguments
- 29.03.24 - Correct ReadPathFromRegistry definition for default size argument
		    Correct EndTiming definition for microseconds argument

SpoutDX.cpp
02.01.24	- ReadPixelData - wait for command completion using FlushWait
06.03.24	- SetReceiverName - clear the receiver name if null passed
13.04.24	- Add SpoutMessageBox functions for dll access
25.04.24	- Correct SpoutMessageBox(const char* caption, UINT uType, const char* format)
			to apps though uType
21.05.24	- CheckSenderFormat remove const from name argument
22.05.24	  CheckSpoutPanel - Register sender only if not already registered
23.05.24	- ReadPixelData/ReadTexurePixels - use global m_bSwapRB flag instead of false
			ReadPixelData - RGBA and BGRA texture data to BGR pixels default, RGB for swap
25.05.24	- Add SetMirror/SetSwap/GetMirror/GetSwap for SpoutCam instead of using globals directly


ofSpoutExample
	Add utilities sender example
	
===========================================\
25.12.23\
Update Master from Beta branch.

Version 2.007.013

Spout.cpp
- 09.10.23 - SelectSenderPanel -if SpoutPanel.exe is not found
  show a SpoutMessageBox with Spout releases page url
- 18.10.23 - ReceiveSenderData - check for texture format supported
  by OpenGL/DirectX interop
- 07.12.23 - use _access in place of shlwapi Path functions
Version 2.007.013

SpoutCopy.cpp
- 07.10.23 - Conditional compile options for _M_ARM64 in CheckSSE and header
- 20.10.23 - FlipBuffer / CopyPixels - default pitch width*4
Version 2.007.013

SpoutDirectX.cpp
- 07.08.23 - Comment out code for debug layers
- 19.10.23 - GetNumAdapters - remove unused adapter description and output list
Version 2.007.013

SpoutFrameCount.cpp
- 03.08.23	- EnableFrameCount, CreateAccessMutex - revise logs
- 06.08.23	- Add m_SystemFps to avoid repeated calls to GetRefreshRate()
- 07.08.23	- Add EnableFrameSync/IsFrameSyncEnabled and global option flag
EnableFrameCount
- 08.08.23	- correct semaphore name
EnableFrameSync - close sync event on disable
- 09.08.23	- WaitFrameSync remove warning log if event not found
 Change timeout log from error to warning
- 13.08.23	- EnableFrameCount - correct semaphore name
Version 2.007.013

SpoutGL.cpp
- 01.11.23 - CreateInterop - correct uint printf formatting for error message
  Avoid repeats if interop failure flag is set. Cleared by CleaunpInterop.
- 30.11.23 - ReadMemoryTexture - remove new frame test
- 07.12.23 - DoDiagnostics - use spoututils GetExeName
- 08.12.23 - Remove DXGI_FORMAT_UNKNOWN from DX11format GL>DX11 conversion function
 Use default DXGI_FORMAT_B8G8R8A8_UNORM
- 14.12.23 - WriteGLDXpixels - return WriteGLDXtexture instead of true
 CreateOpenGL return false if extensions fail to load
Version 2.007.013

SpoutGLextensions.cpp
- 24.07.23	- Add glMemoryObjectParameterivEXT
- 30.07.23	- Add GL_RGBA16F, GL_RGB16F, GL_RGBA32F, GL_RGB32F
- 31.07.23	- Add defines GL_IMPLEMENTATION_COLOR_READ_TYPE, GL_IMPLEMENTATION_COLOR_READ_FORMAT
- 02.08.23	- Add glGetTextureParameteriv
- 21.11.23	- Add defines for : GL_MAX_COMPUTE_WORK_GROUP_COUNT, GL_MAX_COMPUTE_WORK_GROUP_SIZE
  GL_ATTACHED_SHADERS, GL_INFO_LOG_LENGTH
  Add glGetProgramInfoLog, glGetShaderInfoLog, glGetIntegeri_v
Version 2.007.013

SpoutReceiver.cpp
- 04.08.23 - Add format functions
- 07.08.23 - Add frame sync option functions 
Version 2.007.013

SpoutSender.cpp
- 04.08.23 - Add format functions
- 07.08.23 - Add frame sync option functions
Version 2.007.013

SpoutSenderNames.cpp
- 07.10.23 - Conditional compile options for _M_ARM64\
 Moved additonal includes from cpp to header
- 28.10.23 - SetSenderInfo - use QueryFullProcessImageNameA
- 07.12.23 - SetSenderInfo - use spoututils GetEexePath()\
 Remove unused d3d9.h and d3d11.h from header
- 16.12.23 - SetSenderInfo - correct buffer size for GetModuleFileNameA
Version 2.007.013
	
SpoutSharedMemory.cpp
- 07.12.23 - Remove unused <d3d9.h> from header
Version 2.007.013

SpoutUtils.cpp
- 01.08.23 - Add MessageTaskDialog instead of dependence on SpoutPanel
- 04.08.23 - const WCHAR* in MessageTaskDialog
- 13.08.23 - MessageTaskDialog - remove MB_TOPMOST
- 20.08.23 - Change TaskdialogcallbackProc to TDcallbackProc to avoid naming conflicts
- 21.08.23 - MessageTaskDialog - Restore topmost function\
  Change bTopmost to bTopMost to avoid naming conflicts
- 23.08.23 - MessageTaskDialog - Fixed topmost recover for calling application
- 26.08.23 - PFTASKDIALOGCALLBACK cast for TDcallbackProc
- 04.09.23 - MessageTaskDialog - add MB_ICONINFORMATION option. Default no icon.\
  Add MB_ICONSTOP and MB_ICONHAND. MB_TOPMOST flag removal only if specified.
- 05.09.23 - Add SpoutMessageBoxIcon for custom icon
- 07.09.23 - Add round to ElapsedMicroseconds
- 08.09.23 - Check TDN_CREATED for taskdialog topmost
- 11.09.23 - MessageTaskDialog - correct button and icon type detection\
  Allow for NULL caption
- 12.09.23 - Add SpoutMessageBox overload including main instruction large text\
  Correct missing SPOUT_DLLEXP for SpoutMessageBox standard function
- 12.10.23 - Add SpoutMessageBoxButton and CopyToClipBoard
- 20.10.23 - Add OpenSpoutLogs to open Spout log folder in Windows explorer
- 20-11-23 - OpenSpoutLogs() - allow for getenv if not Microsoft compile (PR #105)
- 22-11-23 - Remove unused buffer length argument from _dupenv_s
- 01.12.24 - Update Version to "2.007.013"
- 07.12.23 - Remove SPOUT_DLLEXP from private MessageTaskDialog\
  Use _access and string find in place of shlwapi path functions
  Add GetExePath, GetExeName, RemovePath\
  Revise : _getLogPath(), _getLogFilePath, _logtofile,\
  EnableSpoutLog, EnableSpoutLogFile, ShowSpoutLogs,\
  OpenSpoutConsole, GetNVIDIAmode, SetNVIDIAmode
- 08.12.23 - #ifdef _MSC_VER for linker manifestdependency pragma comment\
  MessageTaskDialog - correct topmost if a second dialog is opened
- 15.12.23 - Change to #ifdef _WINDOWS for linker manifestdependency pragma comment\
  Conditional compile of TaskDialogIndirect for _WINDOWS.\
  MessageBoxTimeoutA for other compilers.
- 16.12.23 - Replace #define _WINDOWS with _MSC_VER for conditional compile for Visual Studio\
  MessageTaskDialog - first argumnent HWND instead of HINSTANCE\
  SpoutMessageBox - pass in hwnd to MessageTaskDialog\
  Add TDF_POSITION_RELATIVE_TO_WINDOW to TaskDialogIndirect config flags\
  TaskDialogIndirect centers on the window if hwnd passed in or the monitor if NULL\
- 20.12.23 - Remove GetNVIDIAmode, SetNVIDIAmode\
  ExecuteProcess - use ShellExecuteEx instead of CreateProcess\
  Add SpoutMessageBoxModless\
  Restore modeless SpoutMessageBox functionality using SpoutPanel, > v2.72 required.\
  Call MessageTaskDialog directly in all SpoutMessageBox functions\
  Use a custom icon if set for SpoutMessageBox functions that do not specify an icon\
  Clear custom icon handle after TaskDialogIndirect exit\
  Add SpoutMessageBoxWindow
- 21.12.23 - Add std::string GetExeVersion()\
  Revise SpoutMessageBoxModeless to test version of SpoutPanel > 2.072
- 27.12.23 - Send OK button message to close taskdialog instead of DestroyWindow for URL click
  Test for custom icon and multiple buttons in MessageTaskDialog
Version 2.007.013
				   
===========================================\
05.09.23\
Update Master from Beta branch.

    Version 2.007.012

Spout.cpp
- 03.08.23	- InitReceiver - set m_DX11format
- 07.08.23	- Add frame sync option functions
		   
SpoutDirectX.cpp
- 07.08.23	- Comment out code for debug layers

SpoutFrameCount.cpp
- 03.08.23	- EnableFrameCount, CreateAccessMutex - revise logs
- 06.08.23	- Add m_SystemFps to avoid repeated calls to GetRefreshRate()
- 07.08.23	- Add EnableFrameSync/IsFrameSyncEnabled and global option flag\
		  EnableFrameCount - correct semapohore name
- 08.08.23	- EnableFrameSync - close sync event on disable
- 09.08.23	- WaitFrameSync remove warning log if event not found\
		  Change timeout log from error to warning
- 13.08.23	- EnableFrameCount - correct semaphore name

SpoutGL.cpp
- 24.07.23	- Remove  global m_bKeyed flag
- 31.07.23	- Add OpenGL format functions
- 04.08.23	- Remove unused compute shaders
- 28.08.23	- UnloadTexturePixels public

SpoutGLextensions.cpp
- 24.07.23	- Add glMemoryObjectParameterivEXT
- 30.07.23	- Add GL_RGBA16F, GL_RGB16F, GL_RGBA32F, GL_RGB32F
- 31.07.23	- Add defines\
		GL_IMPLEMENTATION_COLOR_READ_TYPE, GL_IMPLEMENTATION_COLOR_READ_FORMAT
- 02.08.23	- Add glGetTextureParameteriv

SpoutReceiver.cpp\
SpoutSender.cpp
- 04.08.23	- Add format functions
- 07.08.23	- Add frame sync option functions

SpoutUtils.cpp
- 01.08.23 - Add MessageTaskDialog instead of dependence on SpoutPanel
- 04.08.23 - const WCHAR* in MessageTaskDialog
- 13.08.23 - MessageTaskDialog - remove MB_TOPMOST
- 20.08.23 - Change TaskdialogcallbackProc to TDcallbackProc to avoid naming conflicts
- 21.08.23 - MessageTaskDialog - Restore topmost function\
	Change bTopmost to bTopMost to avoid naming conflicts
- 23.08.23 - MessageTaskDialog - Fixed topmost recover for calling application
- 26.08.23 - PFTASKDIALOGCALLBACK cast for TDcallbackProc
- 04.09.23 - MessageTaskDialog - add MB_ICONINFORMATION option. Default no icon. Add MB_ICONSTOP and MB_ICONHAND. MB_TOPMOST flag removal only if specified.
- 05.09.23 - Add SpoutMessageBoxIcon for custom icon

============================================\
23.07.23\
Update Master from Beta branch.

    Version 2.007.011

Spout.cpp
- 22.04.23	- Minor code comments cleanup
- 29.04.23	- ReceiveSenderData - test for incorrect sender dimensions
- 07.05.23	- CheckSender - release interop device and object to re-create
- 17.05.23	- ReleaseSender - add m_bInitialized = false and remove from SpoutGL::GleanupG
- 18.05.23	- ReleaseSender - clear m_SenderName
- 28.06.23	- Remove bDX9 option from GetAdapterInfo
- 03.07.23	- CreateReceiver - remove unused bUseActive flag and UNREFERENCED_PARAMETER (#PR93  Fix MinGW error(beta branch)
- 22.07.23	- ReceiveSenderData - ensure m_pSharedTexture is null if OpenSharedResource failed.

SpoutCopy.cpp
- 17.04.23 - Add rgba_to_rgb_sse and rgba_to_bgr_sse
- 18.04.23 - Rename to rgba_to_rgb_sse3 and rgba_to_bgr_sse3. Add bSwapRG to rgba_to_rgb_sse3. Remove rgba_to_bgr_sse3. Add experimental rgb_to_bgra_sse3
		   
SpoutDirectX.cpp
- 24.04.23	- Remove const from 1st arg of ReleaseDX11Texture, ReleaseDX11Device and DebugLog to enable debugging via SDK Layers. ReleaseDX11Device check for release of class device and null the pointer. ReleaseDX11Texture log notice if no outstanding refcount. DebugLog - clean up and add code comments. All object releases - flush context to avoid deferred destruction
- 28.04.23	- Release d3dInfoQueue in DebugLog
- 17.05.23	- Add ClearState to FlushWait
- 18.05.23	- Add Flush() function
- 22.05.23	- CloseDirectX11 - release immediate context before device. OpenDX11shareHandle - catch OpenSharedResource exeption before testing result
- 05.06.23	- Allow feature level 11.1 in CreateDX11device. Update logs in ReleaseDX11Texture for refcount
- 09.06.23	- Add GetDX11FeatureLevel for development
- 16.06.23	- CreateDX11device - allow for D3D_FEATURE_LEVEL_11_1. CreateSharedDX11Texture - add NT handle argument for development. OpenDirectX11 Save global m_featureLevel for external device. Create ID3D11Device1 and ID3D11DeviceContext1 for D3D_EATURE_LEVEL_11_1. ReleaseDX11Device - release ID3D11Device1 and ID3D11DeviceContext1 if created

SpoutFrameCount.cpp
- 24.04.23	- Replace m_bDisabled with m_bCountDisabled
- 03.07.23	- EnableFrameCount, CreateAccessMutex - add detail to logs

SpoutGL.cpp
- 15.03.23	- If no new sender frame, return true and do not block for all receiving functions to avoid un-necessary Acquire/Release/Lock/Unlock
- 22.04.23	- Add compute shader utility for OpenGL texture copy. CreateInterop - delete m_glTexture before creating new 
- 04.05.23	- CopyTexture - detach textures from fbo. Blit method only for texture invert.
- 07.05.23	- CreateOpenGL - load extensions.
- 17.05.23	- Remove "m_bInitialized = false" check from CleanupGL and put in release functions
- 18/05.23	- CleanupGL - Remove m_SenderName clear. Unbind textures before CleanupInterop
- 19.05.23	- Add GetInteropObject() and GetDXsharedTexture() 
- 22.05.23	- Remove CleanupInterop from within CleanupGL and use as separate functions
- 07.06.23	- UnloadTexturePixels - specify number of PBOs created in log
- 09.06.23	- Use glCopyImageSubData for CopyTexture if no invert
- 22.06.23	- CreateComputeCopyShader adjust Y workgroup number for aspect ratio
- 03.07.23	- CreateInterop - code cleanup ReadTextureData - change cast (#PR93)
- 06.07.23	- Code cleanup
- 13.07.23	- Make InitTexture public
- 17.07.23	- CopyTexture - remove glCopyImageSubData due to format limitations. Add SwapRGB utility
- 18.07.23	- Make GLerror() public
- 22.07.23	- Some extra checks for null m_pSharedTexture for a receiver

SpoutGLextensions.cpp
- 20.04.23	- Add compute shader extensions
- 22.04.23	- Correct EXT_LOG prefix for standalone in ExtLog function
- 24.04.23	- Add glGetTexParameteriv and glTextureStorage2D
- 04.05.23	- Define GL_BGRA in case it is used
- 09.05.23	- Add memory object extensions
- 16.06.23	- Add glTextureStorageMem2DEXT
- 24.06.23	- Add glUniform1f
- 14.07.23	- Add glMemoryBarrier
- 21.07.23	- Add glGetMemoryObjectParameterivEXT

SpoutReceiver.cpp
- 06.07.23	- Remove bUseActive from 2.006 CreateReceiver

SpoutSenderNames.cpp
- 13.07.23 - setActiveSenderName - close any existing active sender map

SpoutSharedMemory.cpp
- 12.05.23 - Create and Open - Clear ERROR_ALREADY_EXISTS to avoid detection elsewhere.

SpoutUtils.cpp
- 14.04.23 - Update SDKversion to 2.007.011
- 24.04.23 - GetTimer - independent start and end variables startcount/endcount
- 09.05.23 - Yellow console text for warnings and errors
- 17.05.23 - Set console title to executable name
- 04-07-23 - _getLogPath() - allow for getenv if not Microsoft compiler (PR #95)

CmakesList.txt - Spout, SpoutGL and SpoutDX
- Modifications for MINGW build (PR#93-vkedwardli)

===================================\
07-04-23\
Update Master from Beta branch.\
- Version 2.007.010

Cumulative update.

Spout.cpp
- 06.01.23 - UIntToPtr for cast of uint32_t to HANDLE
cast unsigned int array for glGetIntegerv instead of result
Avoid c-style cast where possible
- 08.01.23 - Code review - Use Microsoft Native Recommended rules
- 08.03.23 - GetSenderAdapter use SetAdpater instead of SetAdapterPointer
- 21.03.23 - ReceiveSenderData - use the format of the D3D11 texture generated
by OpenDX11shareHandle for incorrect sender information.

SpoutCopy.cpp
- 02-04-23 - Corrected source pointer increment in rgba2rgba when not inverted

SpoutDirectX.cpp
- 06.01.23 - Correct IsPreferenceAvailable() to pass array length to registry function
- 08.01.23 - CreateSharedDX11Texture - option for keyed shared texture
- 18.03.23 - CreateDX11StagingTexture - use default DX11 format for zero or DX9 forma

SpoutFrameCount.cpp
- 06.01.23 - CheckKeyedAccess - switch on hr to avoid narrowing cast to DWORD
Avoid c-style cast where possible
- 08.01.23 - CheckTextureAccess/AllowTextureAccess 
remove texture check for default null texture
Code review - Use Microsoft Native Recommended rules
- 19.03.23 - WaitFrameSync - do not block if the sender has not created a sync event

SpoutGLextensions.cpp
- 20.01.23 - Changes to gl definitions for compatibility with Glew
- 15.02.23 - SpoutGLextensions.h
Correct glUnmapBufferPROC from void to GLboolean. Correct glGenBuffersPROC buffers arg from const

SpoutSenderNames.cpp
- 06.01.23 - GetActiveSender, getActiveSenderName, FindActiveSender 
Change from fixed sendername argument to maxlength (default SpoutMaxSenderNameLen)
- 08.01.23 - FindActiveSender - test max length passed
Code review - Use Microsoft Native Recommended rules

SpoutSharedMemory.cpp
- 07.01.23 - Change m_pName from const char* to char* for strdup

SpoutUtils.cpp
- 14.01.23 - OpenSpoutConsole - add MessageBox warning if using a dll
EnableSpoutLog - open console rather than call OpenSpoutConsole
- 15.01.23 - Use SpoutMessageBox so it doesn't freeze the application GUI
- 16.01.23 - Add SpoutMessageBox caption
- 17.01.23 - Add SpoutMessageBox with variable arguments
Add ConPrint for SpoutUtils console (printf replacement)
Remove dll build warning MessageBox.
Change "ConPrint" to "_conprint" and use Writefile instead of cout.
- 18.01.23 - _conprint - cast WriteFile size argument to DWORD
- 19.03.23 - Update SDKversion to 2.007.010	

SpoutDX.cpp
- 08.01.23 - Add SpoutUtils functions
- 23.01.23 - CheckSender - Flush after shared texture release
- 17.03.23 - ReceiveSenderData - if there is a valid D3D11 format, use it.
- 18.03.23 - CreateDX11StagingTexture - use default DX11 format for zero or DX9 formats
- 19.03.23 - Remove redundant CreateDX11StagingTexture and use SpoutDirectX function
ReceiveSenderData - create a DX11 receiving texture with compatible format for unknown or DX9 formats.
- 21.03.23 - ReceiveSenderData - revert to using the format of the D3D11 texture
generated by OpenDX11shareHandle for incorrect sender information.		   

SpoutLibrary.cpp
- 17.03.23 - SpoutLibrary.h - add redefinitons to avoid include of GL.h 

Spout.cpp
-  ReceiveSenderData - if there is a valid D3D11 format, use it.
- 18.03.23 - For MingW compatibility remove old style include guard from header
Test for _MSC_VER for pragma warnings in header
Test for NTDDI_WIN10_RS4 define for graphics preferences
- 07.04.23   Rebuild with SDK version 2.007.010 - release VS2022 - 32/64 bit /MD

===================================
05-01-23\
Update Master from Beta branch.\
- Version 2.007.009
- Release 2.007.009 published


SpoutCommon.h
- Disable Visual Studio warning 26812) (unscoped enums)

Spout.cpp
- 10.04.22	- ReceiveSenderData() - correct duplication of DX9 formats
- 16.04.22	- Add more log notices to GetSenderAdapter
- 18.04.22	- Change default invert from true to false for fbo sending functions
- 28.04.22	- SelectSenderPanel - if SpoutPanel is not found,\
			  show a MessageBox and direct to the Spout home page
- 05.05.22	- SendFbo - mods for default framebuffer
- 30.07.22	- SendFbo - avoid using glGetIntegerv if width and height are passed in\
		      Revert to default invert true after further testing with default framebuffer.
- 28.10.22	- Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName
- 01.11.22	- Add SetPreferredAdapter
- 03.11.22	- Add IsPreferenceAvailable\
			  SetAdapter - GL/DX compatibility is re-tested in OpenSpout
- 07.11.22	- Add IsApplicationPath\
			  Update ReceiveSenderData code comments for Windows Graphics Preferences
- 28.11.22	- SelectSenderPanel - correct warning if SpoutPanel is not found instead of the \
			  ShellExecute Windows error and allow user the option to open the Spout home page.
- 05.12.22	- Add CleanSenders to SetSenderName
- 07.12.22	- SelectSender return bool
- 12.12.22	- SetSenderName - return if the same name
- 13.12.22	- SetSenderName revise for null name passed
- 14.12.22	- Remove SetAdapter. Requires OpenGL setup.
- 18.12.22	- Change unused bDX9 argument to const with default
- 20.12.22	- More checks for null arguments
- 22.12.22	- Compiler compatibility check\
		      Conditional compile of preference functions

SpoutCopy.cpp
- 10.04.22 - Casting changes to avoid Warning C26451 : Arithmetic overflow
- 07.10.22 - Corrected missing alpha from rgb2rba and rgb2bgra functions
- 24.10.22 - Add experimental rgb_to_bgrx_sse
- 28.10.22 - Cleanup / code documentation
- 11.12.22 - test for null args in conversion functions
- 22.12.22 - Compiler compatibility check\
		   Change all {} initializations to "={}"

SpoutDirectX.cpp
- 29.03.22	- CreateDX11Texture, CreateSharedDX11Texture, CreateDX11StagingTexture\
		  Switch on HRESULT instead of LOWORD so that DXGI cases are recognised 
- 07.04.22	- CreateDX11Texture - cast to int the LOWORD from hresult for error report
- 15.05.22	- CreateSharedDX11Texture change Fatal logs to Warning
- 26.10.22	- Correct GetAdapterIndex maximum size for wchar convert
- 28.10.22	- Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName\
			  Code cleanup and documentation
- 01.11.22	- Add SetPreferredAdapter
- 02.11.22	- Add IsPreferenceAvailable to detect availability\
			  of Windows Graphics Performance settings.
- 04.11.22	- Change GetAdapterIndex argument from char* to to const char*\
			  Correct to return -1 on failure instead of "false"
- 07.11.22	- Add IsApplicationPath
- 01.12.22	- GetPerformancePreference change logs to warning
- 03.12.22	- Avoid use of "description" char array due to definition in SpoutSenderNames.h
- 17.12.22	- Corrections for code analysis
- 18.12.22	- Catch any exception by using Release in destructor\
			  Use std::string for error text in CreateDX11StagingTexture / CreateSharedDX11Texture
- 22.12.22	- Compiler compatibility check\
			  Change all {} initializations to "={}"
- 31.12.22	- SpoutDirectX.h\
			   Remove WDK_NTDDI_VERSION in case it's not defined\
			   Add comments concerning use of dxgi_6 with older DirectX SDK.

SpoutFrameCount.cpp
- 28.10.22	- Code documentation
- 10.11.22	- Revise HoldFps\
			  Remove m_millisForFrame\
			  Include TimeBeginPeriod/TimeEndPeriod\ 
			  to reduce Windows timing period to minimum.
- 18.11.22	- Move performance counter functions to SpoutUtils
- 21.11.22	- Extend CleanupFrameCount and use in destructor\
			- Correct average frame rate in UpdateSenderFps\
			- Correct GetNewFrame for receiver started.
- 17.12.22	- Use smart pointers for m_FrameStartPtr etc to avoid using new/delete
- 18.12.22	- Change back to new/delete due to incompatibility with SpoutLibrary
- 22.12.22	- Compiler compatibility check

SpoutGL.cpp
- 21.03.22	- LoadGLextensions - correct  && to & for (m_caps & GLEXT_SUPPORT_PBO)\
			  UnloadTexturePixels - casting pitch*width for size compare avoids warning C26451
- 29.03.22	- OpenDeviceKey - correct dwSize from MAX_PATH to 256 in RegOpenKeyExA\
			  ReadTextureData - create unsigned long variables for temp src char array\
			  ReadTextureData - Delete temporary "src" char array created with "new"
- 19.04.22	- Restore host fbo in SetSharedTextureData instead of default 0
- 04.06.22	- SetSharedTextureData - corrected glCheckFramebufferStatus from != to == for textureID 0
- 29.07.22	- OpenSpout - default CPU share until tested
- 28.10.22	- Documentation cleanup
- 26.11.22	- Change SetVerticalSync argument to integer to allow adaptive vsync
- 18.12.22	- ReadTextureData - use std::unique_ptr to create intermediate invert buffer\
			  OpenDeviceKey use std::string "at" instead of direct index\
			  Catch any exception by using cleanup functions in destructor\
- 19.12.22	- Add DoDiagnostics to create a log file for CreateInterop failure
- 22.12.22	- Compiler compatibility testing\
			  Remove std::unique_ptr and go back to new/delete\
			  Change all {} initializations to "={}"
- 30.12.22	- Check and confirm fix for issue #85 and PR #86\
			  Check and confirm fix for issue #87
- 05-01-23	- ReadGLDXtexture - Test for no texture read or zero texture\
			  moved to the beginning to avoid redundant texture lock.

SpoutGLextensions.cpp
- 18.03.22	- Change LogLevel to ExtLoglevel to make unique for ExtLog function\
			  Change to C++11 enum class for ExtLogLevel to isolate the definitions
- 14.04.22	- Change back to C enums for compatibility with earlier compilers.\
			  Rename ExtLoglevel enum names to be more strongly unique.\
			  Add option in SpoutCommon.h to disable warning 26812 (unscoped enums).
- 18.04.22	- Add glCheckNamedFramebufferStatus
- 17.12.22	- Some cleanup for code analysis
- 22.12.22	- Compiler compatibility check

SpoutReceiver.cpp
- 18.04.22	- Change default invert from true to false for fbo sending functions
- 31.10.22	- Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName
- 01.11.22	- Add SetPreferredAdapter
- 03.11.22	- Add IsPreferenceAvailable
- 07.11.22	- Add IsApplicationPath
- 14.12.23	- Remove SetAdapter. Requires OpenGL setup.

SpoutSender.cpp
- 31.10.22	- Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName
- 01.11.22	- Add SetPreferredAdapter
- 03.11.22	- Add IsPreferenceAvailable
- 07.11.22	- Add IsApplicationPath
- 26.11.22	- Change SetVerticalSync argument to integer to allow adaptive vsync

SpoutSenderNames.cpp
- 29.03.22 - change int len to size_t len in setActiveSenderName
- 25.04.22 - Add create memory check and warning logs to setActiveSenderName\
		   Remove duplicate shared memory creation in RegisterSenderName\
		   GetActiveSender - rename temp name string to be more clear
- 09.05.22 - Include SPOUT_ALREADY_CREATED for setActiveSenderName create shared memory success
- 30.07.22 - Throughout - remove redundant code
- 28.10.22 - Code documentation
- 31.10.22 - Include SPOUT_ALREADY_EXISTS for setActiveSenderName create shared memory success
- 13.12.22 - Add SpoutLogWarning to CleanSenders when an orphaned sender is removed
- 17.12.22 - Some cleanup for code analysis
- 22.12.22 - Compiler compatibility check
	
SpoutSharedMemory.cpp
- 14.04.22 - Add option in SpoutCommon.h to disable warning 26812 (unscoped enums).
- 28.10.22 - Code documentation
- 18.12.22 - Catch any exception from using Close in destructor

SpoutUtils.cpp
- 25.02.22 - OpenSpoutConsole - check AllocConsole error for existing console\
		   Fix for Processing.
- 14.04.22 - Add option in SpoutCommon.h to disable warning 26812 (unscoped enums)
- 23.06.22 - Add ElapsedMicroseconds (usec since epoch)
- 30.10.22 - Code cleanup and documentation
- 01.11.22 - Add IsLaptop(char* computername)
- 30.11.22 - Update Version to "2.007.009"
- 05.12.22 - Change ordering of _logtofile function to avoid ambiguous warning.\
		   GetSpoutLog - optional log file argument. Remove redundant file open.\
		   See SpoutSettongs "Log" option.
- 07.12.22 - EnableSpoutLogFile allow null file name argument.\
		   If a file name was not specified, use the executable name.\
		   Use "GetCurrentModule" instead of NULL for GetModuleFileNameA\
		   in case the process is a dll (https://gist.github.com/davidruhmann/8008844).
- 08.12.22 - _dolog - clean up file log code.\
		 - Corrected ExecuteProcess for EndTiming milliseconds return.
- 11.12.22 - Initialize all char arrays and structures with {}\
		   https://en.cppreference.com/w/cpp/language/zero_initialization
- 14.12.22 - Add RemoveSpoutLogFile
- 18.12.22 - Add buffer size argument to ReadPathFromRegistry\
		   Correct code review warnings where possible\
		   Add more documentation to Group: Logs
- 19.12.22 - Add GetCurrentModule / LogsEnabled / LogFileEnabled / GetSpoutLogPath
- 20.12.22 - Add SPOUT_DLLEXP to all header function declarations for dll export.\
		 - Code cleanup
- 22.12.22 - Compiler compatibility check\
		   Change all {} initializations to "={}"
- 31.12.22 - increase log char buffer from 512 to 1024
- 01.12.22 - Registry functions\
		     check for empty subkey and valuename strings\
			 include valuename in warnings

SpoutDX.cpp
- 01.11.22	- Add GetPerformancePreference, SetPerformancePreference,\
			  GetPreferredAdapterName, SetPreferredAdapter
- 08.11.22	- Add IsPreferenceAvailable, IsApplicationPath
- 16.11.22	- HoldFps double fps argument instead of int
- 26.12.22	- Initialize all arrays and structures\
			  Const arg for SendImage
			  
SpoutLibrary.cpp
- 21.03.22 - Change local LogLevel enum to more unique SpoutLibLogLevel\
		   Initialize Spout object pointer
- 11.04.22 - Option disable warning C26812 (unscoped enums) for Visual Studio
- 10.05.22 - Correct OpenSpoutConsole to use "spoututils::"
- 12.05.22 - Rebuild VS2022 - 32/64 bit /MD\
		   Spout Version 2.007.008
- 31.10.22 - Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName\
		   Corrected SpoutLog to use _dolog
- 01.11.22 - Add SetPreferredAdapter, GetSDKversion, IsLaptop
- 03.11.22 - Add IsPreferenceAvailable
- 25.22.33 - Revise SpoutSenderNmaes UpdateSenderFps / HoldFps\
		   Add GetRefreshRate
- 30.11.22 - Add IsApplicationPath
- 22.12.22 - Compiler compatibility check\
		   Conditional compile of preference functions
- 26.12.22 - Add missing SPOUT_LOG_NONE to SpoutLibLogLevel\
		   Rebuild release VS2022 - 32/64 bit /MD\
		   Spout Version 2.007.009

INSTALL folder
- Header and library files from INSTALL build with CMake
- Update CMake documentation for INSTALL build

Examples updated
- Cinder
- ofExample Graphics Sender/Receiver
- SpoutDX Tutorial04 - code comment update for HoldFps

User binaries updated
- Demo Sender/Receiver rebuild
- SpoutSettings
	Allows user access to Windows Graphics Preferences directly.\
	Diagnostics extended.
- SpoutPanel
	Show more information about the sender texture, computer type, graphics adapters present and the one used by the sender application.


===================================\
26-07-21\
Update Master from Beta branch.\
Version 2.007.003

Spout.cpp
- 22.06.21 - Move code for GetSenderCount and GetSender to SpoutSenderNames class
- 03.07.21 - Use changed SpoutSenderNames "GetSender" function name.
- 04.07.21 - Additional code comments concerning update in ReceiveTexture.

SpoutCopy.cpp
- 09.07.21 - memcpy_sse2 - return for null dst or src
 
SpoutDirectX.cpp
- 19.06.21 - Remove output check from FindNVIDIA	

SpoutFrameCount.cpp
- 21.07.21 - Remove debug comment
 
SpoutGL.cpp
- 26.07.21 - Remove memorysize check from GetMemoryBufferSize for receiver
 
SpoutSenderNames.cpp
- 22.06.21 - Restore 2.006 GetSenderNames function
- 03.07.21 - Change GetSenderNames to GetSender to align with Spout class.\
  Change existing GetSender to FindSenderName.\
  Change duplicate FindSenderName to FindSender overload\
  testing function
  
SpoutUtils.cpp
- 26.07.21 - Update Version to "2.007.003"

BUILD binaries updated
- CMake build 32/64bit /MD - requires Visual Studio 2017 runtime to be installed

===================================\
14-06-21\
Update Master from Beta branch\
Version 2.007.002

Spout.cpp
- ReceiveSenderData : Check if the name is in the sender list
- Add GetSenderGLDXready() for receiver
- Add SetSenderID
- Rename functions GetSenderCPU and GetSenderGLDX
- memoryshare.CloseSenderMemory() in ReleaseSender
- IsFrameNew - return frame.IsFrameNew()
- memoryshare.CloseSenderMemory() in ReleaseReceiver
- Add event functions SetFrameSync/WaitFrameSync
- Add data functions WriteMemoryBuffer/ReadMemoryBuffer
- Close sync event in ReleaseSender
- SendFbo - protect against fail for default framebuffer if iconic
- ReceiveTexture - return if flagged for update only if there is a texture to receive into.
- ReceiveTexture - allow for the possibility of a 2.007 sender with 2.006 memoryshare mode for data functions.

SpoutCopy.cpp
- Change CopyPixels and FlipBuffer to accept GL_LUMINANCE

SpoutDirectX.cpp
- Add zero width/height check to CreateDX11Texture

SpoutFrameCount.cpp
- Add sync event functions SetFrameSync/WaitFrameSync/OpenFrameSync/CloseFrameSync
- CloseFrameSync public for use by other classes
- WaitFrameSync - close handle on error
  
SpoutGL.cpp
- Introduced m_bTextureShare and m_bCPUshare flags to handle mutiple options
- WriteDX11texture, ReadTextureData, OpenSpout, LoadGLextensions cleanup
- OpenSpout look for DirectX to prevent repeat
- Change m_bSenderCPUmode to m_bSenderCPU
- Add m_bSenderGLDX
- Change SetSenderCPUmode to include CPU sharing mode and GLDX compatibility
- Change SetSenderCPUmode name to SetSenderID
- Add m_bMemoryShare for possible older 2.006 apps
- Add memoryshare.CreateSenderMemory
- memoryshare.CloseSenderMemory() in destructor
- Add WriteMemoryPixels
- Change ReadMemoryPixels to accept GL_LUMINANCE
- Use reinterpret_cast for memoryshare.LockSenderMemory()
- Remove m_bNewFrame
- Add memoryshare.GetSenderMemoryName()
- Disable frame sync in destructor
- Change ReadMemory to ReadMemoryTexture
- Add GetSenderMemory
- Remove ReadMemoryBuffer open error log
- Remove memoryshare struct from header and replace with SpoutSharedMemory object.
- Close shared memory and sync event in SpoutGL class destructor
- Add GetMemoryBufferSize
- Add CreateMemoryBuffer, DeleteMemoryBuffer
- All data functions return false if 2.006 memoryshare mode.
  
SpoutReceiver.cpp
- Add GetCPUshare and SetCPUshare
- Add GetSenderGLDXready
- Rename functions GetSenderCPU and GetSenderGLDX
- Add event functions SetFrameSync/WaitFrameSync
- Add data function ReadMemoryBuffer
- Add OpenGL shared texture access functions
- Add GetMemoryBufferSize

SpoutSender.cpp
- Add GetCPUshare and SetCPUshare
- Add event functions SetFrameSync/WaitFrameSync
- Add data function WriteMemoryBuffer
- Add GetCPU and GetGLDX
- Add OpenGL shared texture access functions
- Add CreateMemoryBuffer, DeleteMemoryBuffer, GetMemoryBufferSize

SpoutSenderNames.cpp
- GetActiveSender - erase the active sender memory map if the sender info is closed
- Change SetSenderCPUmode to include CPU sharing mode and GLDX compatibility
- Change SetSenderCPUmode name to SetSenderID
- Add GetSender to retrieve class sender.
- Remove SenderDebug

SpoutUtils.cpp
- Add std::string GetSDKversion()
- Fix code if USE_CHRONO not defined
- Disable close button on console and bring the main window to the top again
- Remove noisy warning from ReadPathFromRegistry
- Update version number to "2.007.002"

SpoutDirectX
- Add SpoutDX9 support class and examples
- Add SpoutDX12 support class and examples

SpoutDX.cpp
- SetNewFrame before texture copy
- Add memoryshare struct, ReadMemoryBuffer and WriteMemoryBuffer
- memoryshare.CloseSenderMemory() in destructor and ReleaseSender
- Add SetFrameSync and WaitFrameSync

Examples
- Add Cinder example

ofSpoutExample
- Add binaries
- Add Data sender/receiver examples
- Update source and binaries

SpoutPanel.exe
- Update compatible texture formats. Vers 2.43

Cmake build
- Update for 32/64 bit - Spout.dll, SpoutLibrary.dll, SpoutDX.dll, Spout_static.lib
