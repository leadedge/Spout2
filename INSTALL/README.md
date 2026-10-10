### The CMake INSTALL project

The libraries can be built for a particular compiler using CMake.

Build the *INSTALL* project separately. An INSTALL folder is produced at the same level as the BUILD folder and contains separate folders for include and library files.

### The INSTALL folder

- bin (dynamic link libraries)
- lib (link libraries for dlls or static libraries)
- include (required header files)
  - SpoutDX
  - SpoutGL
  - SpoutLibrary

The INSTALL folder included with the repository contains libraries built with Visual Studio 2022 /MD (without Visual Studio runtimes)

x64 / Win32\

*Note that if building with the SpoutDX dynamic link library and the required header files are in the same folder, edit SpoutDX.h to refer to include files directly. See more information in SPOUTSDK/DirectX.*

### BUILD

Build the *ALL_BUILD* project for a comprehensive build. Files are genereated in the BUILD folder. See the Readme file in that folder for details.
