SPOUTLIBRARY

"SpoutLibrary" is a C compatible DLL for using the Spout SDK with compilers other than Microsoft Visual Studio.

The "VS2017" folder contains Visual Studio projects to build the dll. The Spout SDK files should be located in a folder "SpoutGL" two levels above the project folder.

Open the SpoutLibrary solution, change to "Release" and build the project. SpoutLibrary.dll and SpoutLibrary.lib will be in the Binaries\Win32\ or Binaries\x64 folder.

o Include SpoutLibrary.h in your application header file.
o Include SpoutLibrary.lib in your project for the linker.
o Include SpoutLibrary.dll in the application executable folder.

All functions are the same as for using the Spout SDK files directly or as a Visual Studio dll.

EXAMPLE

Although SpoutLibrary is designed for C compatibilty and does not depend on Visual Studio, an Openfameworks 11 example project "ofSpoutLibrary" is provided to show how to use the library. Changes from the Spout SDK examples are minor and it may be sufficient to simply compare the source code.

SPOUT DLL FOR VISUAL STUDIO

If you are using Visual Studio, and you prefer to use a dll rather than include the SDK source files in you project, the Spout SDK can also be compiled as a dll which will give access to all class functions (see "SPOUTSDK\SpoutGL\VS2017"). 








