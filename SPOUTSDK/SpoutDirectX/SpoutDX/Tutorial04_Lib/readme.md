## Using SpoutDX as a dll or static library instead of source files

### Libraries

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

IMPORTANT : change to Release configuration before build.

### Runtime

Note that the application project build

    Project Properties > C++ > Code Generation > Runtime Library
    
must match the CMake option "SPOUT_BUILD_CMT" for project generation
and the build type, Release or Debug.

    Release
      SPOUT_BUILD_CMT enabled  > Multi-threaded (/MT)
      SPOUT_BUILD_CMT disabled > Multi-threaded DLL (/MD)
    Debug
      SPOUT_BUILD_CMT enabled  > Multi-threaded Debug (/MTd)
      SPOUT_BUILD_CMT disabled > Multi-threaded Debug DLL (/MDd)

This application project and the distributed libraries are

    Release / Multi-threaded (/MT)

For Debug configuration, or other Runtime Library options
the libraries must be re-built to match.

### Binaries

After build, the application executable "Tutorial04_Lib.exe" can be found in :

..\SpoutDX\Binaries\Examples

Other SpoutDX examples are also copied to this folder.

