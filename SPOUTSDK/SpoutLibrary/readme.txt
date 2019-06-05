SPOUTLIBRARY

"SpoutLibrary" is a C compatible DLL for using the Spout SDK with compilers other than Microsoft Visual Studio.

The "VS2012", "VS2015" and "VS2017" folders contain Visual Studio projects to build the dll. The Spout SDK should be located in a folder "SpoutSDK" two levels above the project folder.

Open the "SpoutLibrary" solution, change to "Release" and build the project. SpoutLibrary.dll and SpoutLibrary.lib will be in the Win32\Release or x64\Release folder.

o Include "SpoutLibrary.h" in your application header file.
o Include SpoutLibrary.lib in your project for the linker.
o Include SpoutLibrary.dll in the application executable folder.

All functions are the same as documented in the Spout SDK pdf manual.

EXAMPLE

Although SpoutLibrary is designed for C compatibilty and does not depend on Visual Studio, an Openfameworks 10  example project "ofSpoutLibraryExample" is provided to show how to use the library. CHanges are minor and It may be sufficient to simply browse the source code and compare with the other example code.

SPOUT DLL FOR VISUAL STUDIO

If you are using Visual Studio, and you prefer to use a dll rather than include the SDK source files in you project, the Spout SDK can also be compiled as a dll which will give access to all class functions (see the installation folder "SPOUTSDK\SpoutSDK\VS2017"). 








