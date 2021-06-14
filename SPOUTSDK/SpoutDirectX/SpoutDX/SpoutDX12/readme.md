Examples of how to use the Spout 2.007 SDK and SpoutDX support class for sharing D3D12 texture resources.

The Spout SDK Version 2.007 shares between applications using DirectX11 textures, but it is possible to share with DirectX12 applications using the D3D11On12 mapping layer (https://github.com/microsoft/D3D11On12) which creates a D3D11 device wrapped around the D3D12 device. The wrapped D3D11 resource can then be used to copy to or from a sender shared texture.

Two examples from the from the [Microsoft DirectX Graphics Examples](https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop) have been modified for Spout functions.
- D3D12TriangleSender - sender
- D3D12TextureReceiver - receiver

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

SpoutDX.h\
SpoutDX.cpp\
SpoutDX12.h\
SpoutDX12.cpp

The example project files are for Visual Studio 2017. To build them, retain the folder structure of the Spout distribution. The Spout SDK files should be in a folder "SpoutSDK" as follows :

      SpoutGL <- this folder has all the Spout SDK source files in it for OpenGL. 
	  SpoutDirectX <- this folder is for DirectX examples and the SpoutDX support class
	      SpoutDX <- using the SpoutDX support class
	          SpoutDX12 <- the DirectX12 class and examples
	              D3D12TriangleSender <- D3D12 to DirectX11 texture sender
	              D3D12TextureReceiver <- DirectX11 to D3D12 texture receiver
		  
Open each solution file, change to release and build. For all examples, search for "SPOUT" to see the changes made to the original code. The *SpoutDX12\Binaries* folder contains pre-built executables. Refer to the source code for further information.

