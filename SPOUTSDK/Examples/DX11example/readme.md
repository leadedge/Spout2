Examples of how to use the Spout SDK with DirectX textures without using OpenGL.\
Based on D3D11 tutorials by Chuck Walbourn updated for Visual Studio 2017 and the Windows 10 SDK.

 https://github.com/walbourn/directx-sdk-samples/tree/master/Direct3D11Tutorials

Tutorial04 - sender\
Tutorial07 - receiver

The following source files of the Spout SDK are required :

SpoutCommon.h\
SpoutDirectX.cpp\
SpoutFrameCount.cpp\
SpoutSenderNames.cpp\
SpoutSharedMemory.cpp\
SpoutUtils.cpp\
SpoutDirectX.h\
SpoutFrameCount.h\
SpoutSenderNames.h\
SpoutSharedMemory.h\
SpoutUtils.h  

The default example source files are stand-alone versions using methods directly from the Spout SDK classes as above. They are saved as "Tutorial04_Basic.cpp" and "Tutorial07_Basic.cpp" in the respective Source folders. Please compare with versions using the "SpoutDX" support class "Tutorial04_SpoutDX.cpp" and "Tutorial07_SpoutDX.cpp" which could be suitable for your application. These versions also support Memoryshare mode. Please note that the SpoutDX class is subject to change.\
Copy the required file to the build folder and rename to "Tutorial04.cpp" or "Tutorial07.cpp" as required.

To build these examples, retain the folder structure of the Spout distribution.\
The Spout SDK files should be in a folder "SpoutSDK" two levels above the example solution folder.

      SpoutSDK <- this folder has all the Spout SDK source files in it 
	  Examples <- this folder is for Spout examples
	      DX11example <- the complete DX11 example folder
		      Tutorial04 <- Sender example
		          Tutorial04.sln <- Visual Studio 2017 solution
				  .
				  . 
		      Tutorial07 <- Receiver example
		          Tutorial07.sln <- Visual Studio 2017 solution
				  .
				  .

Open the solution file, change to release and build either x64 or x86.

Search Tutorial04.cpp or Tutorial07.cpp for "SPOUT" to see the changes made to the original tutorial code.


 
 
