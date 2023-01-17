Build the *INSTALL* project separately.\
An INSTALL folder is produced at the same level as the BUILD folder\
and contains separate folders for include and library files.\

The INSTALL folder included with the repository contains\
libraries built with Visual Studio 2022\
x64 / Win32\
/MD (without Visual Studio runtimes)\
/MT (include Visual Studio runtimes)

The libraries can be re-built for a particular compiler using CMake.\
Build the *ALL_BUILD* project for a comprehensive build.\
Files are genereated in the BUILD folder.\
See the Readme file in that folder for details.
