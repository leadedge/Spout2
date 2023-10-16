SpoutLibrary example

This is an example for Visual Studio 2022 and Openframeworks 11.

The project is dependent on the Openframeworks environment and so the "SpoutLibraryExample" folder should be copied as follows.

C:\OPENFRAMEWORKS
    addons
    examples
    apps
      myApps
        SpoutLibraryExample <- copy the entire example folder here (under "apps\myApps")
         ofSpoutLibrary.sln
		 ofSpoutLibrary.vcxproj
         src <- SpoutLibrary.h and source files
         bin <- SpoutLibrary.dll and data files
		 libs <- SpoutLibrary.lib

Open the solution file with Visual Studio 2022, set to "Release".
The project is configured for x64 build.
You may need to re-build the Openframeworks library first.

The necessary libraries, Win32 or x64 can be copied from either
libs\Win32 or libs\x64 to the "libs" folder.

SpoutLibrary.h should be copied to the "src" folder.
SpoutLibrary.dll should be copied to the "bin" folder.

If you receive a warning "unresolved external symbol main", check main.cpp
for the System build required and set Console or Windows as required in
Project > Linker > System.


Source files

The "src" folder contains files for a Sender example.
The "Sources" folder contains files for :
    Sender
    Receiver
	Dynamic (Bare bones sender with dynamic load of SpoutLibrary)
For the one you want copy the source files to the "src" folder and "Rebuild".


Binaries

the compiled example SpoutLibraryExample.exe will be in the "bin" folder.
Rename it if multiple examples such as Sender and Receiver are required.



