"SpoutLibrary" is a C compatible DLL for using the Spout SDK
with compilers other than Microsoft Visual Studio.

The OpenFrameworks examples use CodeBlocks and MingW GCC compiler.

The "VS2012" folder contains a Visual Studio project and source files to build the dll.
The Spout SDK should be located in a folder "SpoutSDK" two levels above the project folder.

Open the "SpoutLibrary" solution, change to "Release" and build the project.
SpoutLibrary.dll and SpoutLibrary.lib will be in the Win32\Release or x64\Release folder.

o Include "SpoutLibrary.h" in your application header file.
o Include SpoutLibrary.lib in your project for the linker.
o Include SpoutLibrary in the application executable folder.

All functions are the same as documented in the Spout SDK pdf manual.

If you are using Visual Studio, and you prefer to use a dll rather than include the
SDK source files in you project, the Spout SDK can also be compiled as a dll which will
give access to all class functions (see the installation folder "SPOUTSDK\SpoutSDK\VS2012"). 







