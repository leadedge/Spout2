Examples of how to use the Spout 2.007 SDK for sharing D3D9ex textures.

Although the Spout SDK Version 2.007 does not contain support for DirectX9, shared textures created with D3D9ex and with format D3DFMT_A8R8G8B8 or D3DFMT_X8R8G8B8 are compatible and will still be be received.

Two examples from the from the [Microsoft DirectX SDK (June 2010)](https://www.microsoft.com/en-au/download/details.aspx?id=6812) have been modified for Spout functions.
- Tut06_Meshes - sender
- Tut05_Textures - receiver

The following source files are included in the projects.

SpoutCommon.h\
SpoutCopy.cpp\
SpoutCopy.h\
SpoutDirectX.cpp\
SpoutDirectX.h\
SpoutFrameCount.cpp\
SpoutFrameCount.h\
SpoutSenderNames.cpp\
SpoutSenderNames.h\
SpoutSharedMemory.cpp\
SpoutSharedMemory.h\
SpoutUtils.cpp\
SpoutUtils.h  

The following additional files are also required :

SpoutDirectX9.h\
SpoutDirectX9.cpp

The example project files are for Visual Studio 2017. To build them, retain the folder structure of the Spout distribution. The Spout SDK files should be in a folder "SpoutSDK" as follows :

      SpoutGL <- this folder has all the Spout SDK source files in it for OpenGL. 
	  SpoutDirectX <- this folder is for DirectX examples and the SpoutDX support class
	      Basic <- using the Spout SDK classes directly
	          SpoutDX9 <- the DirectX9 class and examples
	              Tut06_Meshes <- DirectX9 texture sender
	              Tut05_Textures <- DirectX9 texture receiver
		  
Open each solution file, change to release and build. For all examples, search for "SPOUT" to see the changes made to the original code. The *SpoutDX9\Binaries* folder contains pre-built executables. Refer to the source code for further information.

