//
//	SpoutLibrary.dll
//
//	Spout SDK dll compatible with any C++ compiler
//
//	Based on the CodeProject "HowTo: Export C++ classes from a DLL" by Alex Blekhman.
//	http://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
//  https://web.archive.org/web/20250831095225/https://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
//
//		30.03.16 - Build for 2.005 release - VS2012 /MT
//		13.05.16 - Rearrange folders - rebuild 2.005 - VS2012 /MT
//		23.06.16 - Add invert to ReceiveImage
//		23.06.16 - Rebuild for 2.005 release - VS2012 /MT
//		03.07.16 - Rebuild with VS2015
//		13.01.17 - Rebuild for Spout 2.006
//				 - Add SetCPUmode, GetCPUmode, SetBufferMode, GetBufferMode
//				 - Add HostFBO arg to DrawSharedTexture
//		17.01.17 - Add GetShareMode, SetShareMode
//		23.01.17 - Rebuild for Spout 2.006 - VS2012 /MT
//		08.01.17 - Rebuild - VS2012 /MT
//		20.01.17 - Rebuild update - VS2012 /MT
//		05-04-19 - Introduce SpoutFunctions class
//				   Add CreateOpenGL() and CloseOpenGL()
//				   Test with QT and MingW compiler
//		11-05-19 - Build for 32 bit and 64 bit - VS2017 /MT
//		24.05.19 - Add OpenSpoutConsole for debugging
//		03.06.19 - Add CloseSpoutConsole for debugging
//		04-06-19 - Re-build for 32 bit and 64 bit - VS2017 /MT
//		06-06-19 - Re-build for 256 max senders - 32 bit and 64 bit - VS2017 /MT
//		18.06.19 - Change sender Update to include sender name
//		26.09.19 - Remove redundant 2.007 functions for single class
//				 - Remove Update and use UpdateSender
//				 - Remove CloseSender/CloseReceiver and use ReleaseSender/ReleaseSender
//		29.09.19 - Change IsDX9 to GetDX9 to avoid repeated compatibility test
//		09.10.19 - Add WriteDX9surface and SetDX9device
//		22.10.19 - Add include guard in SpoutLibrary.h
//				 - Add SendFboTexture
//				 - Relocate ReleaseSender
//				   Re-build for 32 bit and 64 bit - VS2017 /MT
//		27.11.19 - Re-build for revised Spout SDK - 32 bit and 64 bit - VS2017 / MT
//		18.01.20 - Add CopyTexture, GetSenderTextureID. Update receiver example
//		19.01.20 - Remove send data functions
//				   Change SendFboTexture to SendFbo
//		20.01.20 - Change GetSenderTextureID() to GetSharedTextureID
//		30.05.20 - Change SendTextureData/SendImageData back to SendTexture/SendImage
//				   Remove 2.006 overloads. Library is now exclusively 2.007.
//				   Re-build for revised Spout SDK - 32 bit and 64 bit - VS2017 / MT
//		18.06.20 - Add GetSenderFormat()
//		12.07.20 - Simplify and use 2.007 sender / receiver classes
//				 - Remove GetShareMode / SetShareMode
//		18.07.20 - Re-build 32/64 bit - VS2017 / MT
//		28.12.20 - Update functions using the SpoutGL class for 2.007
//		29.12.20 - Re-arrange files for CMake and modify project files
//		31.12.20 - Re-build 32/64 bit - VS2017 / MT
//		06.01.20 - Re-build 32/64 bit - VS2017 / MT
//		18.01.21 - Add RegisterSenderName, ReleaseSenderName, FindSenderName
//		19.01.21 - Remove RegisterSenderName, ReleaseSenderName - too low level
//		24.01.21 - Re-build 32/64 bit - VS2017 / MT
//		09.04.21 - Add event functions SetFrameSync/WaitFrameSync
//				 - Add data functions WriteMemoryBuffer/ReadMemoryBuffer
//		10.04.21 - Add GetSenderGLDX(), GetCPU() and GetGLDX()
//				   Update documentation comments
//				   Re-build 32/64 bit - VS2017 / MT
//		24.04.21 - ReceiveTexture - return if flagged for update
//		10.06.21 - Re-build 32/64 bit - VS2017 / MT
//		20.07.21 - Change generic local log level definition to avoid
//				   conflict with other libraries and applications.
//		25.07.21   Re-build 32/64 bit - VS2017 / Multi-threaded DLL (/MD)
//		24.10.21 - Rebuild with updated SpoutGL files 32/64 bit /MD
//		24.11.21 - Add SelectSenderPanel for 2.006 compatibility
//		17.12.21 - Add timing utility functions
//		27.12.21 - Rebuild 32/64 bit /MD for update 2.007.006
//		28.01.22 - Remove <d3d9.h> from SpoutLibrary.h (Issue #77)
//		24.02.22 - Rebuild 32/64 bit /MD Version 2.007.007
//		21.03.22 - Change local LogLevel enum to more unique SpoutLibLogLevel
//				   Initialize Spout object pointer
//		11.04.22 - Option disable warning C26812 (unscoped enums) for Visual Studio
//		10.05.22 - Correct OpenSpoutConsole to use "spoututils::"
//		12.05.22 - Rebuild VS2022 - 32/64 bit /MD
//				   Spout Version 2.007.008
//		31.10.22 - Add GetPerformancePreference, SetPerformancePreference, GetPreferredAdapterName
//				   Corrected SpoutLog to use _dolog
//		01.11.22 - Add SetPreferredAdapter, GetSDKversion, IsLaptop
//		03.11.22 - Add IsPreferenceAvailable
//		22.11.23 - Revise SpoutSenderNames UpdateSenderFps / HoldFps
//				   Add GetRefreshRate
//		30.11.22 - Add IsApplicationPath
//		22.12.22 - Compiler compatibility check
//				   Conditional compile of preference functions
//		26.12.22 - Add missing SPOUT_LOG_NONE to SpoutLibLogLevel
//				   Rebuild release VS2022 - 32/64 bit /MD
//				   Spout Version 2.007.009
//		17.03.23 - SpoutLibrary.h - add redefinitons to avoid include of GL.h 
//				   Spout.cpp ReceiveSenderData - if there is a valid D3D11 format, use it.
//		18.03.23 - For MingW compatibility
//				     Remove old style include guard from header
//				     Test for _MSC_VER for pragma warnings in header
//				     Test for NTDDI_WIN10_RS4 define for graphics preferences
//		07.04.23   Rebuild with SDK version 2.007.010 - release VS2022 - 32/64 bit /MD
//		03.05.23   Add GL_BGRA define to SpoutLibrary.h
//		08.07.23   CreateReceiver - remove unused bUseActive flag
//		23.07.23   Rebuild with SDK version 2.007.011 - release VS2022 - 32/64 bit /MD
//		05.08.23   Add format functions
//		28.09.23   Add EnableFrameSync
//				   Rebuild with SDK version 2.007.012 - release VS2022 - 32/64 bit /MD
//		16.10.23   Add SpoutMessageBoxIcon and SpoutMessageBoxButton
//		03.12.23   Rebuild with SDK version 2.007.013 /MD and /MT using CMake
//		08.12.23   Rebuild all libraries /MT and /MD with Openframeworks 12.0 files using CMake
//		28.12.23   Add SpoutMessageBoxModeless and SpoutMessageBoxWindow
//		12.03.24   Restore SpoutMessgaBox with variable arguments
//				   Add SpoutMessageBox with text entry and with combobox controls
//		12.06.24   Add GetSenderList
//		31.07.24   Add GetSenderTexture, GetCurrentModule, GetExeVersion,
//				   GetExePath, GetExeName, RemovePath, RemoveName
//				   Remove GetSpoutVersion
//		13.10.25   Add GetReceiverName, SelectSender, CloseFrameSync, IsFrameSyncEnabled
//				   DisableSpoutLogFile, RemoveSpoutLogFile, DisableSpoutLog, DisableLogs,
//				   EnableLogs, LogsEnabled, LogFileEnabled, GetSpoutLogPath, ReadTextureData,
//				   WriteBinaryToRegistry, Update SelectSenderPanel, GetExepath
//				   Rename : RemoveName->GetPath, RemovePath->GetName
//		15.10.25   Update CreateOpenGL - add hwnd argument
//				   Add InitTexture, ClearAlpha
//		13.10.25   Rebuild with SDK version 2.007.017 /MD and /MT using CMake
//		20.10.28   Add FlipBuffer
//		29.06.26   Add SpoutMessageBoxAllowCancel
//		03.07.26   Add destructor to allow SpoutLibrary object delete
//				   Initialize spout object with nullptr
//		04.07.26   All function declarations in cpp file "override"
//				   Destructor declaration protected in header file
//				   Destructor deletes the spout object in cpp file
//				   Release() function deletes "this" only
//				   CMakeLists.txt uses Spout SDK source files for static build
//		05.07.26   Remove SpoutUtils wrapper functions.
//				   Namespace is available directly with CMake change.
//				   Add #define WIN32_LEAN_AND_MEAN to header.
//		06.07.26   Remove override warning disable 26433 from header.
//		22.07.26   Restore SpoutUtils wrapper functions.
//		22.09.26 - Change HoldFps from int to double
//
/*
		Copyright (c) 2016-2026, Lynn Jarvis. All rights reserved.

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

#include <stdio.h>
#include "SpoutLibrary.h"
#include "../SpoutGL/Spout.h"

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")// the Direct3D 11 Library file


////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the SPOUT interface.
//

// Title: SpoutLibrary
//
// <https://spout.zeal.co/>
//
//	Spout SDK dll compatible with any C++ compiler
//
//	Based on the CodeProject :
//
//	"HowTo: Export C++ classes from a DLL" by Alex Blekhman.
//
//	http://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
//
// Group: Building the library
//
// To build the library using CMake.
//
// Refer to "Building the libraries.pdf" at the root of the repository.
//
// To build using Visual Studio 2022.
// 
// This folder contains a Visual Studio 2022 project to build the dll.
//
// The OpenGL source files used should be located in a "SpoutGL" folder
//
// at the same level as the project folder.
// 
// o SpoutGL
// o SpoutLibrary
//
// Open the SpoutLibrary solution file with Visual Studio 2022
//
// change to "Release" "x64" and build the project.
//
// SpoutLibrary.dll and SpoutLibrary.lib are copied to the Binaries folder.
// 
// Group: Building applications with the library
//
// o Include SpoutLibrary.h in the application header file.
// o Include SpoutLibrary.lib or SpoutLibary_static.lib
//   in the project for the linker.
// o Include SpoutLibrary.dll in the application executable
//   folder if using the shared library
//
// All functions are the same as described in the Spout SDK documentation.
//
// Group: Using the library
//
// 1) Include SpoutLibrary.h in the application header file
//        #include "SpoutLibrary.h"
// 2) Specify SpoutLibrary.lib for the linker, either shared or static
//    SpoutLibrary.dll is required in the application executable folder
//    if using a shared library
//    Shared
//        #pragma comment(lib, "libs/SpoutLibrary.lib")
//    Static
//        #pragma comment(lib, "libs/SpoutLibrary_static.lib")
//
// SpoutLibrary can be used in two ways
//	o Using a library pointer - legacy method
//	o Using a library object  - class method
//
// o Library pointer - legacy method
//	 For compatibility with existing projects
//		1) Create a SpoutLibrary pointer
//			SPOUTLIBRARY* sender;
//		2) Create an instance of the library
//			sender = GetSpout();
//		3) Use functions with the library pointer
//			sender->SendTexture(...) etc.
//		4) Release the library on exit
//			sender->Release();
//
// o Library object - class method
//	 For code compatibility with Spout classes
//		1) Create a SpoutLibrary object
//			SpoutLib sender;
//		3) Use functions with the library object
//			sender.SendTexture(...) etc.
//
// Refer to the source code of the SpoutLibrary examples.
// All functions are the same as documented in the Spout SDK.
// Changes from the Spout SDK examples are minor.
//

class SPOUTImpl : public SPOUTLIBRARY
{

public:

	// Spout SDK functions object pointer for the SPOUTImpl class
	// Initialize in GetSpout()
	Spout* spout = nullptr;

	// Destructor
	~SPOUTImpl() override { delete spout; }

private: // Spout SDK functions

	//
	// Group: Sender
	//
	// SendFbo, SendTexture and SendImage create or update a sender as required.
	//
	// - If a sender has not been created yet :
	//
	//    - Make sure Spout has been initialized and OpenGL context is available
	//    - Perform a compatibility check for GL/DX interop
	//    - If compatible, create interop for GL/DX transfer
	//    - If not compatible, create a DirectX 11 shared texture for the sender
	//    - Create a sender using the DX11 shared texture handle
	//
	// - If the sender exists, test for size change :
	//
	//    - If compatible, update the shared textures and GL/DX interop
	//    - If not compatible, re-create the class DirectX shared texture to the new size
	//    - Update the sender and class variables	
	//

	// Function: SetSenderName
	// Set name for sender creation
	//
	//     If no name is specified, the executable name is used. 
	//     Thereafter, all sending functions create and update a sender
	//     based on the size passed and the name that has been set
	void SetSenderName(const char* sendername = nullptr) override;

	// Function: SetSenderFormat
	// Set the sender DX11 shared texture format
	void SetSenderFormat(DWORD dwFormat) override;

	// Function: ReleaseSender
	// Close receiver and release resources.
	//
	// A new sender is created or updated by all sending functions
	void ReleaseSender(DWORD dwMsec = 0) override;

	// Function: SendFbo
	// Send texture attached to fbo
	//
	//   The fbo must be currently bound
	//   The sending texture can be larger than the size that the sender is set up for
	//   For example, if the application is using only a portion of the allocated texture space,  
	//   such as for Freeframe plugins. (The 2.006 equivalent is DrawToSharedTexture)
	//
	bool SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert = true) override;

	// Function: SendTexture
	// Send OpenGL texture
	//
	//     SendTexture creates a shared texture for all receivers to access.
	//
	//     The invert flag is optional and by default true. This flips the texture
	//     in the Y axis, which is necessary because DirectX and OpenGL textures
	//     are opposite in Y. If it is set to false no flip occurs and the result
	//     may appear upside down.
	//
	//     The host fbo argument is optional (default 0) but an fbo ID is necessary
	//     if it is currently bound, then that binding is restored. Otherwise the
	//     binding is lost.
	//
	bool SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = true, GLuint HostFBO = 0) override;

	// Function: SendImage
	// Send pixel image
	//
	//     SendImage creates a shared texture using image pixels as the source
	//     instead of an OpenGL texture. The format of the image to be sent is RGBA 
	//     by default but can be a different OpenGL format, for example GL_RGB or GL_BGRA_EXT.
	//
	//     The invert flag is optional and false by default.
	//
	//     As for SendTexture, the ID of a currently bound fbo can be passed in.
	//
	bool SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bInvert = false) override;

	// Function: IsInitialized
	// Sender status
	virtual bool IsInitialized() override;

	// Function: GetName
	// Sender name
	const char* GetName() override;
	
	// Function: GetWidth
	// Sender width
	unsigned int GetWidth() override;
	
	// Function: GetHeight
	// Sender height
	unsigned int GetHeight() override;
	
	// Function: GetFps
	// Sender frame rate
	double GetFps() override;
	
	// Function: GetFrame
	// Sender frame number
	long GetFrame() override;
	
	// Function: GetHandle
	// Sender share handle
	HANDLE GetHandle() override;

	// Function: GetCPU
	// Sender sharing method.
	// Returns true if the sender is using CPU methods
	bool GetCPU() override;

	// Function: GetGLDX
	// Sender sharing compatibility.
	// Returns true if the sender graphics hardware is 
	// compatible with NVIDIA NV_DX_interop2 extension
	bool GetGLDX() override;


	//
	// Group: Receiver
	//
	// Receiving functions
	//
	// ReceiveTexture and ReceiveImage 
	//
	//		- Connect to a sender
	//
	//		- Set class variables for sender name, width and height
	//
	//		- If the sender has changed size, set a flag for the application to update the receiving texture or image if IsUpdated() returns true.
	//
	//		- Copy the sender shared texture to the user texture or image.
	//
	// Any changes to sender size are managed. However, if you are receiving to a local texture or image,
	// the application must check for update at every cycle before receiving any data using "IsUpdated()"

	// Function: SetReceiverName
	// Specify sender for connection
	//
	//   - If a name is specified, the receiver will not connect to any other unless the user selects one.
	//   - If that sender closes, the receiver will wait for the nominated sender to open. 
	//   - If no name is specified, the receiver will connect to the active sender.
	void SetReceiverName(const char * SenderName = nullptr) override;

	// Function: GetReceiverName
	// Get sender for connection
	bool GetReceiverName(char* SenderName, int maxchars = 256) override;

	// Function: ReleaseReceiver
	// Close receiver and release resources ready to connect to another sender
	void ReleaseReceiver() override;

	// Function: ReceiveTexture
	//
	//   For a valid OpenGL receving texture :
	//
	//   Copy from the sender shared texture if there is a texture to receive into.
	//   The receiving OpenGL texture can only be RGBA of dimension (width * height)
	//   and must be re-allocated if IsUpdated() returns true. Return if flagged for update.
	//   The update flag is reset when the receiving application calls IsUpdated().
	//
	//   If no arguments are passed :
	//
	//   Connect to a sender and retrieve shared texture details,
	//	 initialize GL/DX interop for OpenGL texture access, and update
	//   the sender shared texture, frame count and framerate.
	//   The texture can then be accessed using :
	//		- BindSharedTexture() override;
	//		- UnBindSharedTexture() override;
	//		- GetSharedTextureID() override;
	//
	//   As for SendTexture, the host fbo argument is optional (default 0)
	//   but an fbo ID is necessary if it is currently bound, then that binding
	//   is restored. Otherwise the binding is lost.
	bool ReceiveTexture(GLuint TextureID = 0, GLuint TextureTarget = 0, bool bInvert = false, GLuint HostFbo = 0) override;
	
	// Function: ReceiveImage
	// Copy the sender texture to image pixels.
	//
	//    Formats supported are : GL_RGBA, GL_RGB, GL_BGRA_EXT, GL_BGR_EXT.
	//    GL_BGRA_EXT and GL_BGR_EXT are dependent on those extensions being supported at runtime.
	//    If they are not, the rgba and rgb equivalents are used.
	//    The same sender size changes are handled with IsUpdated() as for ReceiveTexture.
	//    and the receiving buffer must be re-allocated if IsUpdated() returns true.
	//    NOTE : images with padding on each line are not supported.
	//    Also the width should be a multiple of 4.
	//
	//    As for ReceiveTexture, the ID of a currently bound fbo should be passed in.
	bool ReceiveImage(unsigned char *pixels, GLenum glFormat = GL_RGBA, bool bInvert = false, GLuint HostFbo = 0) override;
	
	// Function: IsUpdated
	// Query whether the sender has changed.
	//
	//   Must be checked at every cycle before receiving data. 
	//
	//   If this is not done, the receiving functions fail.
	bool IsUpdated() override;
	
	// Function: IsConnected
	// Query sender connection.
	//
	//   If the sender closes, receiving functions return false,  
	//   but connection can be tested at any time.
	bool IsConnected() override;
	
	// Function: IsFrameNew
	// Query received frame status
	//
	//   The receiving texture or pixel buffer is refreshed if the sender has produced a new frame  
	//   This can be queried to process texture data only for new frames
	bool IsFrameNew() override;
	
	// Function: GetSenderName
	// Get sender name
	const char * GetSenderName() override;
	
	// Function: GetSenderWidth
	// Get sender width
	unsigned int GetSenderWidth() override;
	
	// Function: GetSenderHeight
	// Get sender height
	unsigned int GetSenderHeight() override;
	
	// Function: GetSenderFormat
	// Get sender DirectX texture format
	DWORD GetSenderFormat() override;
	
	// Function: GetSenderFps
	// Get sender frame rate
	double GetSenderFps() override;
	
	// Function: GetSenderFrame
	// Get sender frame number
	long GetSenderFrame() override;
	
	// Function: GetSenderHandle
	// Received sender share handle
	HANDLE GetSenderHandle() override;

	// Function: GetSenderTexture
	// Received sender texture
	ID3D11Texture2D* GetSenderTexture() override;
	
	// Function: GetSenderCPU
	// Received sender sharing mode.
	// Returns true if the sender is using CPU methods
	bool GetSenderCPU() override;

	// Function: GetSenderGLDX
	// Received sender sharing compatibility.
	//     Returns true if the sender graphics hardware is 
	//     compatible with NVIDIA NV_DX_interop2 extension
	bool GetSenderGLDX() override;

	// Function: GetHostPath
	// The path of the host that produced the sender
	// Retrieved from the description string in the sender info memory map
	bool GetHostPath(const char* sendername, char* hostpath, int maxchars) override;

	// Function: GetSenderList
	// Return a list of current senders
	std::vector<std::string> GetSenderList() override;

	// Function: SelectSender
	// Open sender selection dialog
	void SelectSender(HWND hwnd = NULL) override;

	// Function: SelectSenderPanel
	// Open sender selection dialog with optional message - 2.006 compatibility
	void SelectSenderPanel(const char* message) override;

	//
	// Group: Frame counting
	//

	// Function: SetFrameCount
	// Enable or disable frame counting globally
	void SetFrameCount(bool bEnable) override;
	
	// Function: DisableFrameCount
	// Disable frame counting specifically for this application
	void DisableFrameCount() override;
	
	// Function: IsFrameCountEnabled
	// Return frame count status
	bool IsFrameCountEnabled() override;
	
	// Function: HoldFps
	// Frame rate control
	void HoldFps(double fps) override;

	// Function: GetRefreshRate
	// Get system refresh rate
	double GetRefreshRate() override;
	
	// Function: SetFrameSync
	// Signal sync event 
	void SetFrameSync(const char* SenderName) override;
	
	// Function: WaitFrameSync
	// Wait or test for a sync event
	bool WaitFrameSync(const char *SenderName, DWORD dwTimeout = 0) override;

	// Function: EnableFrameSync
	// Enable / disable frame sync
	void EnableFrameSync(bool bSync = true) override;

	// Function: CloseFrameSync
	// Close frame sync
	void CloseFrameSync() override;

	// Function: IsFrameSyncEnabled
	// Check for frame sync option
	bool IsFrameSyncEnabled() override;

	// Function: GetVerticalSync
	// Vertical sync status
	int GetVerticalSync() override;

	// Function: SetVerticalSync
	// Lock to monitor vertical sync
	bool SetVerticalSync(bool bSync = true) override;

	//
	// Group: Data sharing
	//
	//   General purpose data exchange functions using shared memory.
	//   These functions can be used in addition to texture sharing.
	//   Typical uses will be for data attached to the video frame,
	//   commonly referred to as "per frame Metadata".
	//
	//
	//   Notes for synchronisation.
	//
	//   If used before sending and after receiving, the data will be 
	//   associated with the same video frame, but frames may be missed 
	//   if the receiver has a lower frame rate than the sender.
	//
	//   If strict synchronization is required, the data sharing functions
	//   should be used in combination with event signal functions. The sender
	//   frame rate will be matched exactly to that of the receiver and the 
	//   receiver will not miss any frames.
	//
	//      - void SetFrameSync(const char* SenderName) override;
	//      - bool WaitFrameSync(const char *SenderName, DWORD dwTimeout = 0) override;
	//      - void EnableFrameSync(bool bSync) override;
	//
	//   WaitFrameSync
	//   A sender should use this before rendering or sending texture or data and
	//   wait for a signal from the receiver that it is ready to read another frame.
	//
	//   SetFrameSync
	//   After receiving a texture, rendering the result and reading data
	//   a receiver should signal that it is ready to read another. 
	//
	//   EnableFrameSync
	//   Enable or disable frame sync during operation
	//
	//
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
	bool WriteMemoryBuffer(const char *sendername, const char* data, int length) override;

	// Function: ReadMemoryBuffer
	// Read shared memory to a buffer.
	//
	//    Open a memory map and retain the handle.
	//    The map is closed when the receiver is released.
	int  ReadMemoryBuffer(const char* sendername, char* data, int maxlength) override;

	// Function: CreateMemoryBuffer
	// Create a shared memory buffer.
	//
	//    Create a memory map and retain the handle.
	//    This function should be called before any buffer write
	//    if the length of the data to send will vary.
	//    The map is closed when the sender is released.
	bool CreateMemoryBuffer(const char *name, int length) override;

	// Function: DeleteMemoryBuffer
	// Delete a sender shared memory buffer.
	bool DeleteMemoryBuffer() override;

	// Function: GetMemoryBufferSize
	// Get the number of bytes available for data transfer.
	int GetMemoryBufferSize(const char *name) override;

	//
	// Group: OpenGL shared texture
	//

	// Function: BindSharedTexture
	// Bind OpenGL shared texture
	bool BindSharedTexture() override;
	
	// Function: UnBindSharedTexture
	// Un-bind OpenGL shared texture
	bool UnBindSharedTexture() override;
	
	// Function: GetSharedTextureID
	// OpenGL shared texture ID
	GLuint GetSharedTextureID() override;

	//
	// Group: Sender names
	//

	// Function: GetSenderCount
	// Number of senders
	int  GetSenderCount() override;
	
	// Function: GetSender
	// Sender item name in the sender names list
	bool GetSender(int index, char* sendername, int MaxSize = 256) override;
	
	// Function: FindSenderName
	// Find a sender in the sender names list
	bool FindSenderName(const char* sendername) override;
	
	// Function: GetSenderInfo
	// Sender information
	bool GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat) override;
	
	// Function: GetActiveSender
	// Current active sender
	bool GetActiveSender(char* Sendername) override;
	
	// Function: SetActiveSender
	// Set sender as active
	bool SetActiveSender(const char* Sendername) override;

	//
	// Group: User registry settings
	// recorded by "SpoutSettings"
	//

	// Function: GetBufferMode
	// Get user buffering mode
	bool GetBufferMode() override;
	
	// Function: SetBufferMode
	// Set application buffering mode
	void SetBufferMode(bool bActive = true) override;
	
	// Function: GetBuffers
	// Get user number of pixel buffers
	int GetBuffers() override;
	
	// Function: SetBuffers
	// Set application number of pixel buffers
	void SetBuffers(int nBuffers) override;
	
	// Function: GetMaxSenders
	// Get user Maximum senders allowed
	int GetMaxSenders() override;
	
	// Function: SetMaxSenders
	// Set user Maximum senders allowed
	void SetMaxSenders(int maxSenders) override;
		
	//
	// Group: 2.006 compatibility
	//
	// These functions are retained for compatibility with existing 2.006 code.
	//

	// Function: CreateSender
	// Create a sender
	bool CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat = 0) override;
	
	// Function: UpdateSender
	// Update a sender
	bool UpdateSender(const char* Sendername, unsigned int width, unsigned int height) override;
	
	// Function: CreateReceiver
	// Create receiver connection
	bool CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height) override;
	
	// Function: CheckReceiver
	// Check receiver connection
	bool CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected) override;
	
	// Function: GetDX9
	// Get user DX9 mode
	bool GetDX9() override;
	
	// Function: SetDX9
	// Set user DX9 mode
	bool SetDX9(bool bDX9 = true) override;
	
	// Function: GetMemoryShareMode
	// Get user memory share mode
	bool GetMemoryShareMode() override;
	
	// Function: SetMemoryShareMode
	// Set user memory share mode
	bool SetMemoryShareMode(bool bMem = true) override;
	
	// Function: GetCPUmode
	// Get user CPU mode
	bool GetCPUmode() override;
	
	// Function: SetCPUmode
	// Set user CPU mode
	bool SetCPUmode(bool bCPU) override;
	
	// Function: GetShareMode
	// Get user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	int GetShareMode() override;
	
	// Function: SetShareMode
	// Set user share mode
	//  0 - texture, 1 - memory, 2 - CPU
	void SetShareMode(int mode) override;

	//
	// Group: Graphics compatibility
	//

	// Function: GetAutoShare
	// Get auto GPU/CPU share depending on compatibility
	bool GetAutoShare() override;
	
	// Function: SetAutoShare
	// Set auto GPU/CPU share depending on compatibility
	void SetAutoShare(bool bAuto = true) override;

	// Function: SetCPUShare
	// Set application CPU share
	// (re-test GL/DX compatibility if set to false)
	void SetCPUshare(bool bCPU = true) override;

	// Function: IsGLDXready
	// OpenGL texture share compatibility
	bool IsGLDXready() override;

	//
	// Group: Graphics adapter
	//
	// Note that both the Sender and Receiver must use the same graphics adapter.
	//

	// Function: GetNumAdapters
	// The number of graphics adapters in the system
	int GetNumAdapters() override;
	
	// Function: GetAdapterName
	// Get adapter item name
	bool GetAdapterName(int index, char *adaptername, int maxchars) override;
	
	// Function: AdapterName
	// Current adapter name
	char* AdapterName() override;
	
	// Function: GetAdapter
	// Get adapter index
	int GetAdapter() override;

	// Function: GetAdapterInfo
	// Get the description and output display name of the current adapter
	bool GetAdapterInfo(char* description, char* output, int maxchars) override;

	// Function: GetAdapterInfo
	// Get the description and output display name for a given adapter
	bool GetAdapterInfo(int index, char* description, char* output, int maxchars) override;
	
	//
	// Group: Graphics preference
	//
	// Windows 10+ SDK required
	//
	// Performance prefrence settings are available from Windows 10
	// April 2018 update "Redstone 4" (Version 1803, build 17134) and later.
	// Windows 10 SDK required included in Visual Studio 2017 ver.15.7 
	//
#ifdef NTDDI_WIN10_RS4

	//---------------------------------------------------------
	// Function: GetPerformancePreference
	// Get the Windows graphics preference for an application
	//
	//	-1 - Not registered
	//
	//	 0 - DXGI_GPU_PREFERENCE_UNSPECIFIED
	//
	//	 1 - DXGI_GPU_PREFERENCE_MINIMUM_POWER
	//
	//	 2 - DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
	//
	int GetPerformancePreference(const char* path) override;

	//---------------------------------------------------------
	// Function: SetPerformancePreference
	// Set the Windows graphics preference for an application
	//
	//     -1 - No preference
	//
	//      0 - Default
	//
	//      1 - Power saving
	//
	//      2 - High performance
	//
	bool SetPerformancePreference(int preference, const char* path) override;

	//---------------------------------------------------------
	// Function: GetPreferredAdapterName
	//
	// Get the graphics adapter name for a Windows preference
	// This is the first adapter for the given preference :
	//
	//    DXGI_GPU_PREFERENCE_UNSPECIFIED - (0) Equivalent to EnumAdapters1
	//
	//    DXGI_GPU_PREFERENCE_MINIMUM_POWER - (1) Integrated GPU
	//
	//    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE - (2) External GPU / Discrete GPU
	//
	bool GetPreferredAdapterName(int preference, char* adaptername, int maxchars) override;

	//---------------------------------------------------------
	// Function: SetPreferredAdapter
	//
	// Set graphics adapter index for a Windows preference
	//
	// This index is used by CreateDX11device when DirectX is intitialized
	//
	//    DXGI_GPU_PREFERENCE_UNSPECIFIED - (0) Equivalent to EnumAdapters1
	//
	//    DXGI_GPU_PREFERENCE_MINIMUM_POWER - (1) Integrated GPU
	//
	//    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE - (2) External GPU / Discrete GPU
	//
	bool SetPreferredAdapter(int preference) override;

	//---------------------------------------------------------
	// Function: IsPreferenceAvailable()
	// Availability of Windows graphics preference settings.
	//
	// Settings are available from Windows 10 April 2018 update 
	// (Version 1803, build 17134) and later.
	bool IsPreferenceAvailable() override;

	//---------------------------------------------------------
	// Function: IsApplicationPath
	//
	// Is the path a valid application
	//
	// A valid application path will have a drive letter and terminate with ".exe"
	bool IsApplicationPath(const char* path) override;
#endif

	//
	// Group: OpenGL utilities
	//

	// Function: CreateOpenGL
	// Create an OpenGL window and context for situations where there is none.
	//   Not used if applications already have an OpenGL context.
	//   Always call CloseOpenGL afterwards.
	bool CreateOpenGL(HWND hwnd = nullptr) override;
	
	// Function: CloseOpenGL
	// Close OpenGL window
	bool CloseOpenGL() override;

	// Function: InitTexture
	// Create OpenGL texture
	void InitTexture(GLuint& texID, GLenum GLformat, unsigned int width, unsigned int height) override;
	
	// Function: CopyTexture
	// Copy OpenGL texture with optional invert
	//   Textures must be the same size
	bool CopyTexture(GLuint SourceID, GLuint SourceTarget,
		GLuint DestID, GLuint DestTarget,
		unsigned int width, unsigned int height,
		bool bInvert = false, GLuint HostFBO = 0) override;

	// Function: ReadTextureData
	// Copy OpenGL texture data to a pixel buffer
	bool ReadTextureData(GLuint SourceID, GLuint SourceTarget,
		void* data, unsigned int width, unsigned int height, unsigned int rowpitch,
		GLenum dataformat, GLenum datatype, bool bInvert = false, GLuint HostFBO = false) override;

	// Function: ClearAlpha
	// Clear alpha of rgba image pixels to the required value
	void ClearAlpha(unsigned char* src, unsigned int width, unsigned int height, unsigned char alpha) override;

	//
	//  Group: Pixel buffer utilities
	//

	// Function: FlipBuffer
	// Flip a pixel buffer from source to destination
	void FlipBuffer(const unsigned char *src, unsigned char *dst,
		unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA) override;

	// Function: FlipBuffer
	// Flip a pixel buffer in place
	void FlipBuffer(unsigned char* src,
		unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA) override;


	//
	//  Group: Formats
	//

	// Get sender DX11 shared texture format
	DXGI_FORMAT GetDX11format() override;
	// Set sender DX11 shared texture format
	void SetDX11format(DXGI_FORMAT textureformat) override;
	// Return OpenGL compatible DX11 format
	DXGI_FORMAT DX11format(GLint glformat) override;
	// Return DX11 compatible OpenGL format
	GLint GLDXformat(DXGI_FORMAT textureformat = DXGI_FORMAT_UNKNOWN) override;
	// Return OpenGL texture internal format
	GLint GLformat(GLuint TextureID, GLuint TextureTarget) override;
	// Return OpenGL texture format description
	std::string GLformatName(GLint glformat = 0) override;


	//
	// Group: DirectX utilities
	//

	bool OpenDirectX() override;
	void CloseDirectX() override;
	
	// Function: OpenDirectX11
	// Initialize and prepare DirectX 11
	bool OpenDirectX11(void * pDevice = nullptr) override;
	void CloseDirectX11() override;

	// Function: GetDX11Device
	// Return the class device
	void * GetDX11Device() override;

	// Function: GetDX11Context
	// Return the class context
	void * GetDX11Context() override;

	//
	// Group: Utilities
	//
	// SpoutUtils namespace functions
	// Refer to SpoutUtils.h for function details
	//
	void OpenSpoutConsole();
	void CloseSpoutConsole(bool bWarning = false);
	void EnableSpoutLog();
	void EnableSpoutLogFile(const char *filename, bool append = false);
	void DisableSpoutLogFile();
	void RemoveSpoutLogFile(const char* filename);
	void DisableSpoutLog();
	void DisableLogs();
	void EnableLogs();
	bool LogsEnabled();
	bool LogFileEnabled();
	std::string GetSpoutLogPath();
	std::string GetSpoutLog();
	void ShowSpoutLogs();
	void SetSpoutLogLevel(SpoutLibLogLevel level);
	void SpoutLog(const char* format, ...);
	void SpoutLogVerbose(const char* format, ...);
	void SpoutLogNotice(const char* format, ...);
	void SpoutLogWarning(const char* format, ...);
	void SpoutLogError(const char* format, ...);
	void SpoutLogFatal(const char* format, ...);
	int SpoutMessageBox(const char* message, DWORD dwMilliseconds = 0);
	int SpoutMessageBox(const char* caption, const char* format, ...);
	int SpoutMessageBox(const char* caption, UINT uType, const char * format, ...);
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0);
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, const char* instruction, DWORD dwMilliseconds = 0);
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::string& text);
	int SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::vector<std::string> items, int& selected);
	void SpoutMessageBoxIcon(HICON hIcon);
	bool SpoutMessageBoxIcon(std::string iconfile);
	void SpoutMessageBoxButton(int ID, std::wstring title);
	void SpoutMessageBoxModeless(bool bMode);
	void SpoutMessageBoxWindow(HWND hWnd);
	void SpoutMessageBoxPosition(POINT pt);
	void SpoutMessageBoxAllowCancel(bool bCancel = true, bool bRetain = false);
	bool CopyToClipBoard(HWND hwnd, const char* caps);
	bool OpenSpoutLogs();
	bool ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue);
	bool WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue);
	bool ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath);
	bool WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath);
	bool WriteBinaryToRegistry(HKEY hKey, const char* subkey, const char* valuename, const unsigned char* hexdata, DWORD nchars);
	bool RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename);
	bool RemoveSubKey(HKEY hKey, const char *subkey);
	bool FindSubKey(HKEY hKey, const char *subkey);
	std::string GetSDKversion(int* pNumber);
	bool IsLaptop();
	HMODULE GetCurrentModule();
	std::string GetExeVersion(const char* path);
	std::string GetExePath(bool bFull);
	std::string GetExeName();
	std::string GetPath(std::string fullpath);
	std::string GetName(std::string fullpath);
	void StartTiming();
	double EndTiming(bool microseconds, bool bPrint);


	//
	// Group: Class release
	//

	// Function: Release
	// Release the class instance
	void Release() override;

};


// ---------------------------------------------------------------
// 2.007
//

//
// Sender
//

void SPOUTImpl::SetSenderName(const char* sendername)
{
		spout->SetSenderName(sendername);
}

void SPOUTImpl::SetSenderFormat(DWORD dwFormat)
{
		spout->SetSenderFormat(dwFormat);
}

void SPOUTImpl::ReleaseSender(DWORD dwMsec)
{
	dwMsec = 0; // Not used for 2.007
	spout->ReleaseSender();
}

bool SPOUTImpl::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	return spout->SendFbo(FboID, width, height, bInvert);
}

bool SPOUTImpl::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spout->SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

bool SPOUTImpl::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert)
{
	return spout->SendImage(pixels, width, height, glFormat, bInvert);
}

bool SPOUTImpl::IsInitialized()
{
	return spout->IsInitialized();
}

const char* SPOUTImpl::GetName()
{
	return spout->GetName();
}

unsigned int SPOUTImpl::GetWidth()
{
	return spout->GetWidth();
}

unsigned int SPOUTImpl::GetHeight()
{
	return spout->GetHeight();
}

double SPOUTImpl::GetFps()
{
	return spout->GetFps();
}

long SPOUTImpl::GetFrame()
{
	return spout->GetFrame();
}

HANDLE SPOUTImpl::GetHandle()
{
	return spout->GetHandle();
}

bool SPOUTImpl::GetCPU()
{
	return spout->GetCPU();
}

bool SPOUTImpl::GetGLDX()
{
	return spout->GetGLDX();
}


//
// Receiver
//

void SPOUTImpl::SetReceiverName(const char* SenderName)
{
	spout->SetReceiverName(SenderName);
}

bool SPOUTImpl::GetReceiverName(char* SenderName, int maxchars)
{
	return spout->GetReceiverName(SenderName, maxchars);
}

void SPOUTImpl::ReleaseReceiver()
{
	spout->ReleaseReceiver();
}

bool SPOUTImpl::ReceiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
{
	return spout->ReceiveTexture(TextureID, TextureTarget, bInvert, HostFbo);
}

bool SPOUTImpl::ReceiveImage(unsigned char *pixels, GLenum glFormat, bool bInvert, GLuint HostFbo)
{
	return spout->ReceiveImage(pixels, glFormat, bInvert, HostFbo);
}

bool SPOUTImpl::IsUpdated()
{
	return spout->IsUpdated();
}

bool SPOUTImpl::IsConnected()
{
	return spout->IsConnected();
}

bool SPOUTImpl::IsFrameNew()
{
	return spout->IsFrameNew();
}

const char * SPOUTImpl::GetSenderName()
{
	return spout->GetSenderName();
}

unsigned int SPOUTImpl::GetSenderWidth()
{
	return spout->GetSenderWidth();
}

unsigned int SPOUTImpl::GetSenderHeight()
{
	return spout->GetSenderHeight();
}

DWORD SPOUTImpl::GetSenderFormat()
{
	return spout->GetSenderFormat();
}

double SPOUTImpl::GetSenderFps()
{
	return spout->GetSenderFps();
}

long SPOUTImpl::GetSenderFrame()
{
	return spout->GetSenderFrame();
}

HANDLE SPOUTImpl::GetSenderHandle()
{
	return spout->GetSenderHandle();
}

ID3D11Texture2D* SPOUTImpl::GetSenderTexture()
{
	return spout->GetSenderTexture();
}

bool SPOUTImpl::GetSenderCPU()
{
	return spout->GetSenderCPU();
}

bool SPOUTImpl::GetSenderGLDX()
{
	return spout->GetSenderGLDX();
}

bool SPOUTImpl::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	return spout->GetHostPath(sendername, hostpath, maxchars);
}

std::vector<std::string> SPOUTImpl::GetSenderList()
{
	return spout->GetSenderList();
}

void SPOUTImpl::SelectSender(HWND hwnd)
{
	spout->SelectSender(hwnd);
}

void SPOUTImpl::SelectSenderPanel(const char* message)
{
	spout->SelectSenderPanel(message);
}

//
// Frame count
//

void SPOUTImpl::SetFrameCount(bool bEnable)
{
	spout->SetFrameCount(bEnable);
}


void SPOUTImpl::DisableFrameCount()
{
	spout->DisableFrameCount();
}

bool SPOUTImpl::IsFrameCountEnabled()
{
	return spout->IsFrameCountEnabled();
}

void SPOUTImpl::HoldFps(double fps)
{
	return spout->HoldFps(fps);
}

double SPOUTImpl::GetRefreshRate()
{
	return spoututils::GetRefreshRate();
}

void SPOUTImpl::SetFrameSync(const char* SenderName)
{
	return spout->SetFrameSync(SenderName);
}

bool SPOUTImpl::WaitFrameSync(const char *SenderName, DWORD dwTimeout)
{
	return spout->WaitFrameSync(SenderName, dwTimeout);
}

void SPOUTImpl::EnableFrameSync(bool bSync)
{
	return spout->EnableFrameSync(bSync);
}

void SPOUTImpl::CloseFrameSync()
{
	spout->CloseFrameSync();
}

bool SPOUTImpl::IsFrameSyncEnabled()
{
	return spout->IsFrameSyncEnabled();
}


int SPOUTImpl::GetVerticalSync()
{
	return spout->GetVerticalSync();
}

bool SPOUTImpl::SetVerticalSync(bool bSync)
{
	return spout->SetVerticalSync(bSync);
}


bool SPOUTImpl::WriteMemoryBuffer(const char *name, const char* data, int length)
{
	return spout->WriteMemoryBuffer(name, data, length);
}

int SPOUTImpl::ReadMemoryBuffer(const char* name, char* data, int maxlength)
{
	return spout->ReadMemoryBuffer(name, data, maxlength);
}

bool SPOUTImpl::CreateMemoryBuffer(const char *name, int length)
{
	return spout->CreateMemoryBuffer(name, length);
}

bool SPOUTImpl::DeleteMemoryBuffer()
{
	return spout->DeleteMemoryBuffer();
}

int SPOUTImpl::GetMemoryBufferSize(const char *name)
{
	return spout->GetMemoryBufferSize(name);
}

bool SPOUTImpl::BindSharedTexture()
{
	return spout->BindSharedTexture();
}

bool SPOUTImpl::UnBindSharedTexture()
{
	return spout->UnBindSharedTexture();
}

GLuint SPOUTImpl::GetSharedTextureID()
{
	return spout->GetSharedTextureID();
}

int  SPOUTImpl::GetSenderCount()
{
	return spout->GetSenderCount();
}

bool SPOUTImpl::GetSender(int index, char* sendername, int MaxSize)
{
	return spout->GetSender(index, sendername, MaxSize);
}

bool SPOUTImpl::FindSenderName(const char* sendername)
{
	return spout->sendernames.FindSenderName(sendername);
}

bool SPOUTImpl::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spout->GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

bool SPOUTImpl::GetActiveSender(char* Sendername)
{
	return spout->GetActiveSender(Sendername);
}

bool SPOUTImpl::SetActiveSender(const char* Sendername)
{
	return spout->SetActiveSender(Sendername);
}

bool SPOUTImpl::GetBufferMode()
{
	return spout->GetBufferMode();
}

void SPOUTImpl::SetBufferMode(bool bActive)
{
	spout->SetBufferMode(bActive);
}

int SPOUTImpl::GetBuffers()
{
	return spout->GetBuffers();
}

void SPOUTImpl::SetBuffers(int nBuffers)
{
	spout->SetBuffers(nBuffers);
}

int  SPOUTImpl::GetMaxSenders()
{
	return spout->GetMaxSenders();
}

void SPOUTImpl::SetMaxSenders(int maxSenders)
{
	spout->SetMaxSenders(maxSenders);
}


//
// For 2.006 compatibility
//

//
// ======================= 2.006 and earlier =======================
//

//
// Sender
//
bool SPOUTImpl::CreateSender(const char *Sendername, unsigned int width, unsigned int height, DWORD dwFormat)
{
	return spout->CreateSender(Sendername, width, height, dwFormat);
}

bool SPOUTImpl::UpdateSender(const char* Sendername, unsigned int width, unsigned int height)
{
	return spout->UpdateSender(Sendername, width, height);
}

// 
// Receiver
//
bool SPOUTImpl::CreateReceiver(char* Sendername, unsigned int &width, unsigned int &height)
{
	return spout->CreateReceiver(Sendername, width, height);
}

bool SPOUTImpl::CheckReceiver(char* Sendername, unsigned int &width, unsigned int &height, bool &bConnected)
{
	return spout->CheckReceiver(Sendername, width, height, bConnected);
}

bool SPOUTImpl::GetDX9()
{
	return spout->GetDX9();
}

bool SPOUTImpl::SetDX9(bool bDX9)
{
	return spout->SetDX9(bDX9);
}

bool SPOUTImpl::GetMemoryShareMode()
{
	return spout->GetMemoryShareMode();
}

bool SPOUTImpl::SetMemoryShareMode(bool bMem)
{
	return spout->SetMemoryShareMode(bMem);
}

bool SPOUTImpl::GetCPUmode()
{
	return spout->GetCPUmode();
}

bool SPOUTImpl::SetCPUmode(bool bCPU)
{
	return spout->SetCPUmode(bCPU);
}

int SPOUTImpl::GetShareMode()
{
	return spout->GetShareMode();
}

void SPOUTImpl::SetShareMode(int mode)
{
	spout->SetShareMode(mode);
}

//
// Graphics compatibility
//

bool SPOUTImpl::GetAutoShare()
{
	return spout->GetAutoShare();
}

void SPOUTImpl::SetAutoShare(bool bAuto)
{
	spout->SetAutoShare(bAuto);
}

void SPOUTImpl::SetCPUshare(bool bCPU)
{
	spout->SetCPUshare(bCPU);
}

bool SPOUTImpl::IsGLDXready()
{
	return spout->IsGLDXready();
}

//
// Adapter functions
//

int SPOUTImpl::GetNumAdapters()
{
	return spout->GetNumAdapters();
}

bool SPOUTImpl::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spout->GetAdapterName(index, adaptername, maxchars);
}

char* SPOUTImpl::AdapterName()
{
	return spout->AdapterName();
}

int SPOUTImpl::GetAdapter()
{
	return spout->GetAdapter();
}

bool SPOUTImpl::GetAdapterInfo(char* description, char* output, int maxchars)
{
	return spout->GetAdapterInfo(description, output, maxchars);
}

bool SPOUTImpl::GetAdapterInfo(int index, char* description, char* output, int maxchars)
{
	return spout->GetAdapterInfo(index, description, output, maxchars);
}

// Windows 10+ SDK required
#ifdef NTDDI_WIN10_RS4

int SPOUTImpl::GetPerformancePreference(const char* path)
{
	return spout->GetPerformancePreference(path);
}

bool SPOUTImpl::SetPerformancePreference(int preference, const char* path)
{
	return spout->SetPerformancePreference(preference, path);
}

bool SPOUTImpl::GetPreferredAdapterName(int preference, char* adaptername, int maxchars)
{
	return spout->GetPreferredAdapterName(preference, adaptername, maxchars);
}

bool SPOUTImpl::SetPreferredAdapter(int preference)
{
	return spout->SetPreferredAdapter(preference);
}

bool SPOUTImpl::IsPreferenceAvailable()
{
	return spout->IsPreferenceAvailable();
}

bool SPOUTImpl::IsApplicationPath(const char* path)
{
	return spout->IsApplicationPath(path);
}
#endif


//
// OpenGL utilities
//

bool SPOUTImpl::CreateOpenGL(HWND hwnd)
{
	return spout->CreateOpenGL(hwnd);
}

bool SPOUTImpl::CloseOpenGL()
{
	return spout->CloseOpenGL();
}

void SPOUTImpl::InitTexture(GLuint& texID, GLenum GLformat, unsigned int width, unsigned int height)
{
	spout->InitTexture(texID, GLformat, width, height);
}

bool SPOUTImpl::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	return spout->CopyTexture(SourceID, SourceTarget, DestID, DestTarget,
								width, height, bInvert, HostFBO);
}

bool SPOUTImpl::ReadTextureData(GLuint SourceID, GLuint SourceTarget,
		void* data, unsigned int width, unsigned int height, unsigned int rowpitch,
		GLenum dataformat, GLenum datatype, bool bInvert, GLuint HostFBO)
{
	return spout->ReadTextureData(SourceID, SourceTarget,
		data, width, height, rowpitch,
		dataformat, datatype, bInvert, HostFBO);
}


//
// Pixel buffer utilities
//

void SPOUTImpl::ClearAlpha(unsigned char* src, unsigned int width, unsigned int height, unsigned char alpha)
{
	spout->ClearAlpha(src, width, height, alpha);
}

void SPOUTImpl::FlipBuffer(const unsigned char* src, unsigned char* dst,
	unsigned int width, unsigned int height, GLenum glFormat)
{
	spout->spoutcopy.FlipBuffer(src, dst, width, height, glFormat);
}

void SPOUTImpl::FlipBuffer(unsigned char* src,
		unsigned int width, unsigned int height, GLenum glFormat)
{
	spout->spoutcopy.FlipBuffer(src, width, height, glFormat);
}


//
// Formats
//

DXGI_FORMAT SPOUTImpl::GetDX11format()
{
	return spout->GetDX11format();
}

void SPOUTImpl::SetDX11format(DXGI_FORMAT textureformat)
{
	spout->SetDX11format(textureformat);
}

DXGI_FORMAT SPOUTImpl::DX11format(GLint glformat)
{
	return spout->DX11format(glformat);
}

GLint SPOUTImpl::GLDXformat(DXGI_FORMAT textureformat)
{
	return spout->GLDXformat(textureformat);
}

GLint SPOUTImpl::GLformat(GLuint TextureID, GLuint TextureTarget)
{
	return spout->GLformat(TextureID, TextureTarget);
}

std::string SPOUTImpl::GLformatName(GLint glformat)
{
	return spout->GLformatName(glformat);
}


//
// DirectX utilities
//

bool SPOUTImpl::OpenDirectX()
{
	return spout->OpenDirectX();
}

void SPOUTImpl::CloseDirectX()
{
	spout->CloseDirectX();
}

bool SPOUTImpl::OpenDirectX11(void* pDevice)
{
	// A cast from void* can use static_cast
	return spout->OpenDirectX11(static_cast<ID3D11Device*>(pDevice));
}

void SPOUTImpl::CloseDirectX11()
{
	spout->spoutdx.CloseDirectX11();
}

void* SPOUTImpl::GetDX11Device()
{
	// void cast conversion can be implicit
	return spout->GetDX11Device();
}

void* SPOUTImpl::GetDX11Context()
{
	return spout->GetDX11Context();
}


// LJ DEBUG
//
// Group: Utilities
//
void SPOUTImpl::OpenSpoutConsole() {
	spoututils::OpenSpoutConsole();
}

void SPOUTImpl::CloseSpoutConsole(bool bWarning)
{
	spoututils::CloseSpoutConsole(bWarning);
}

void SPOUTImpl::EnableSpoutLog()
{
	spoututils::EnableSpoutLog();
}

void SPOUTImpl::EnableSpoutLogFile(const char* filename, bool append)
{
	spoututils::EnableSpoutLogFile(filename, append);
}

void SPOUTImpl::DisableSpoutLogFile()
{
	spoututils::DisableSpoutLog();
}

void SPOUTImpl::DisableLogs()
{
	spoututils::DisableLogs();
}

void SPOUTImpl::RemoveSpoutLogFile(const char* filename)
{
	spoututils::RemoveSpoutLogFile(filename);
}

void SPOUTImpl::DisableSpoutLog()
{
	spoututils::DisableSpoutLog();
}

void SPOUTImpl::EnableLogs()
{
	spoututils::EnableLogs();
}

bool SPOUTImpl::LogsEnabled()
{
	return spoututils::LogsEnabled();
}

bool SPOUTImpl::LogFileEnabled()
{
	return spoututils::LogFileEnabled();
}

std::string SPOUTImpl::GetSpoutLogPath()
{
	return spoututils::GetSpoutLogPath();
}

std::string SPOUTImpl::GetSpoutLog()
{
	return spoututils::GetSpoutLog();
}

void SPOUTImpl::ShowSpoutLogs()
{
	spoututils::ShowSpoutLogs();
}

void SPOUTImpl::SetSpoutLogLevel(SpoutLibLogLevel level)
{
	spoututils::SetSpoutLogLevel(static_cast<spoututils::SpoutLogLevel>(level));
}

void SPOUTImpl::SpoutLog(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_NONE, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogVerbose(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_VERBOSE, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogNotice(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_NOTICE, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_WARNING, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_ERROR, format, args);
	va_end(args);
}

void SPOUTImpl::SpoutLogFatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	spoututils::_doLog(spoututils::SPOUT_LOG_FATAL, format, args);
	va_end(args);
}

int SPOUTImpl::SpoutMessageBox(const char * message, DWORD dwMilliseconds)
{
	return spoututils::SpoutMessageBox(message, dwMilliseconds);
}

int SPOUTImpl::SpoutMessageBox(const char* caption, const char* format, ...)
{
	std::string strmessage;
	std::string strcaption;
	char logChars[1024]={};

	// Construct the message
	va_list args;
	va_start(args, format);
	vsprintf_s(logChars, 1024, format, args);
	strmessage = logChars;
	va_end(args);

	if (caption && *caption)
		strcaption = caption;
	else
		strcaption = "Message";

	return spoututils::SpoutMessageBox(NULL, strmessage.c_str(), caption, strcaption.c_str(), MB_OK, 0);

}

int SPOUTImpl::SpoutMessageBox(const char* caption, UINT uType, const char* format, ...)
{
	std::string strmessage;
	std::string strcaption;
	char logChars[1024]={};

	// Construct the message
	va_list args;
	va_start(args, format);
	vsprintf_s(logChars, 1024, format, args);
	strmessage = logChars;
	va_end(args);

	if (caption && *caption)
		strcaption = caption;
	else
		strcaption = "Message";

	return spoututils::SpoutMessageBox(NULL, strmessage.c_str(), caption, strcaption.c_str(), uType, 0);

}

int SPOUTImpl::SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds)
{
	return spoututils::SpoutMessageBox(hwnd, message, caption, uType, dwMilliseconds);
}

int SPOUTImpl::SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, const char* instruction, DWORD dwMilliseconds)
{
	return spoututils::SpoutMessageBox(hwnd, message, caption, uType, instruction, dwMilliseconds);
}

int SPOUTImpl::SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::string& text)
{
	return spoututils::SpoutMessageBox(hwnd, message, caption, uType, text);
}

int SPOUTImpl::SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, std::vector<std::string> items, int& index)
{
	return spoututils::SpoutMessageBox(hwnd, message, caption, uType, items, index);
}



void SPOUTImpl::SpoutMessageBoxIcon(HICON hIcon)
{
	spoututils::SpoutMessageBoxIcon(hIcon);
}

bool SPOUTImpl::SpoutMessageBoxIcon(std::string iconfile)
{
	return spoututils::SpoutMessageBoxIcon(iconfile);
}

void SPOUTImpl::SpoutMessageBoxButton(int ID, std::wstring title)
{
	spoututils::SpoutMessageBoxButton(ID, title);
}

void SPOUTImpl::SpoutMessageBoxModeless(bool bMode)
{
	spoututils::SpoutMessageBoxModeless(bMode);
}


void SPOUTImpl::SpoutMessageBoxWindow(HWND hWnd)
{
	spoututils::SpoutMessageBoxWindow(hWnd);
}

void SPOUTImpl::SpoutMessageBoxPosition(POINT pt)
{
	spoututils::SpoutMessageBoxPosition(pt);
}

void SPOUTImpl::SpoutMessageBoxAllowCancel(bool bCancel, bool bRetain)
{
	spoututils::SpoutMessageBoxAllowCancel(bCancel, bRetain);
}

bool SPOUTImpl::CopyToClipBoard(HWND hwnd, const char* caps)
{
	return spoututils::CopyToClipBoard(hwnd, caps);
}

bool SPOUTImpl::OpenSpoutLogs()
{
	return spoututils::OpenSpoutLogs();
}

//
// Registry utilities
//

bool SPOUTImpl::ReadDwordFromRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD *pValue)
{
	return spoututils::ReadDwordFromRegistry(hKey, subkey, valuename, pValue);
}

bool SPOUTImpl::WriteDwordToRegistry(HKEY hKey, const char *subkey, const char *valuename, DWORD dwValue)
{
	return spoututils::WriteDwordToRegistry(hKey, subkey, valuename, dwValue);
}

bool SPOUTImpl::ReadPathFromRegistry(HKEY hKey, const char *subkey, const char *valuename, char *filepath)
{
	return spoututils::ReadPathFromRegistry(hKey, subkey, valuename, filepath);
}

bool SPOUTImpl::WritePathToRegistry(HKEY hKey, const char *subkey, const char *valuename, const char *filepath)
{
	return spoututils::WritePathToRegistry(hKey, subkey, valuename, filepath);
}

bool SPOUTImpl::WriteBinaryToRegistry(HKEY hKey, const char* subkey, const char* valuename, const unsigned char* hexdata, DWORD nchars)
{
	return spoututils::WriteBinaryToRegistry(hKey, subkey, valuename, hexdata, nchars);
}

bool SPOUTImpl::RemovePathFromRegistry(HKEY hKey, const char *subkey, const char *valuename)
{
	return spoututils::RemovePathFromRegistry(hKey, subkey, valuename);
}

bool SPOUTImpl::RemoveSubKey(HKEY hKey, const char *subkey)
{
	return spoututils::RemoveSubKey(hKey, subkey);
}

bool SPOUTImpl::FindSubKey(HKEY hKey, const char *subkey)
{
	return spoututils::FindSubKey(hKey, subkey);
}

std::string SPOUTImpl::GetSDKversion(int* pNumber)
{
	return spoututils::GetSDKversion(pNumber);
}

bool SPOUTImpl::IsLaptop()
{
	return spoututils::IsLaptop();
}

HMODULE SPOUTImpl::GetCurrentModule()
{
	return spoututils::GetCurrentModule();
}

std::string SPOUTImpl::GetExeVersion(const char* path)
{
	return spoututils::GetExeVersion(path);
}

std::string SPOUTImpl::GetExePath(bool bFull)
{
	return spoututils::GetExePath(bFull);
}

std::string SPOUTImpl::GetExeName()
{
	return spoututils::GetExeName();
}

std::string SPOUTImpl::GetPath(std::string fullpath)
{
	return spoututils::GetPath(fullpath);
}

std::string SPOUTImpl::GetName(std::string fullpath)
{
	return spoututils::GetName(fullpath);
}

void SPOUTImpl::StartTiming()
{
	spoututils::StartTiming();
}

double SPOUTImpl::EndTiming(bool microseconds, bool bPrint)
{
	return spoututils::EndTiming(microseconds, bPrint);
}

//
// Class function
//

void SPOUTImpl::Release()
{
	// Delete this class instance
	// Destructor deletes the spout object
	delete this;
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the SPOUT object.
//
// Export both decorated and undecorated names. For example :
//		 GetSpout    - Undecorated name, which can be easily used with GetProcAddress
//                     Win32 API function.
//		_GetSpout@0  - Common name decoration for __stdcall functions in C language.
//
// For more information on name decoration see here:
// "Format of a C Decorated Name"
// http://msdn.microsoft.com/en-us/library/x7kb4e2f.aspx

#if !defined(_WIN64)
// This pragma is required only for 32-bit builds.
// In a 64-bit environment, C functions are not decorated.
#pragma comment(linker, "/export:GetSpout=_GetSpout@0")
#endif  // _WIN64

extern "C" SPOUTAPI SPOUTHANDLE APIENTRY GetSpout()
{
	// The Spout class implementation
	SPOUTImpl* pSpout = new SPOUTImpl;

	// Create a new spout pointer for this class
	pSpout->spout = new Spout;

	return pSpout;
}

////////////////////////////////////////////////////////////////////////////////
