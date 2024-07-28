### Using SpoutDX as a dll or static library instead of source files

\
Files from CMake INSTALL build or Spout release are copied to equivalent
folders at the root path of the project :

     include     header files from INSTALL\include\SpoutDX
     lib         SpoutDX.lib and SpoutDX_static.lib from INSTALL\lib\SpoutDX
     bin         SpoutDX.dll from INSTALL\bin\SpoutDX 

For build options see "STATIC OR DYNAMIC LIBRARY" in the application source

The process is the same for the Spout, SpoutDX9 and SpoutDX12 libraries
- Create lib, include and bin folders at the root folder of the project
- Copy files from a CMake INSTALL build or Spout release for the library.
- Insert the "STATIC OR DYNAMIC LIBRARY" code at the beginning of the\
main application source file and modify the library names.
- Remove Spout source files from the project.

Note that the application project build for Release or Debug

    Project Properties > C++ > Code Generation > Runtime Library
    
must match the CMake option "SPOUT_BUILD_CMT" for project generation
and the Spout project build type, Release or Debug.

    SPOUT_BUILD_CMT enabled
      Release  > Multi-threaded (/MT)
      Debug    > Multi-threaded Debug (/MTd)
    SPOUT_BUILD_CMT disabled
      Release > Multi-threaded DLL (/MD)
      Debug   > Multi-threaded Debug DLL (/MDd)

The libraries distributed with this project are built Release with\
SPOUT_BUILD_CMT enabled and the application project is configured for :

    Release / Multi-threaded (/MT)

For Debug configuration of the application project, or for other\
Runtime options, the libraries must be re-built to match.

