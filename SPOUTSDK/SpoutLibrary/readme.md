#SPOUTLIBRARY

"SpoutLibrary" is a DLL which is compatible for building with any C++ compiler.

##To build the library using CMake.
- Refer to "Building the libraries.pdf" at the root of the repository.

##To build using Visual Studio 2022.

This folder contains a Visual Studio 2022 project to build the dll. 
The OpenGL source files used should be located in a "SpoutGL" folder 
at the same level as the project folder.

  SpoutGL
  SpoutLibrary

Open the SpoutLibrary solution file with Visual Studio 2022
change to "Release" "x64" and build the project.

SpoutLibrary.dll and SpoutLibrary.lib are copied to the Binaries folder.

o Include SpoutLibrary.h in your application header file.
o Include SpoutLibrary.lib in your project for the linker.
o Include SpoutLibrary.dll in the application executable folder.

<pre>
   #include "SpoutLibrary.h"

   // Specify SpoutLibrary.lib for the linker
   #pragma comment(lib, "libs/SpoutLibrary.lib")

   // Get a pointer to SpoutLibrary
   sender = GetSpout();

   // Use functions with the library pointer
   sender->OpenSpoutConsole(); // Empty console for debugging
</pre>

All functions are the same as documented in the Spout SDK documentation.
Changes from the Spout SDK examples are minor. Compare the source code for details.

## Examples

Example Visual Studio 2022 projects "SpoutLibrarySender" and "SpoutLibraryReceiver" can be found in the 
"SpoutLibraryExamples" folder and show how to use the library.








