===================================\
24-02-22\
Update Master from Beta branch.\
Version 2.007.007\
For changes since 24-10-21, please refer to the individual source files



===================================\
24-10-21\
Update Master from Beta branch.\
Version 2.007.005

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
- 26.07.21 - Update Version to "2.007.005"




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
