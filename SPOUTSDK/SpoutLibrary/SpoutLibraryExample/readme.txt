SpoutLibrary example

This is an example for Visual Studio 2017 and Openframeworks 10

The project is dependent on the Openframeworks environment and so the "SpoutLibraryExample" folder should be copied as follows.

C:\OPENFRAMEWORKS
    addons
    examples
    apps
      myApps
        SpoutLibraryExample <- copy the entire example folder here (under "apps\myApps")
         ofSpoutLibrary.sln
		 ofSpoutLibrary.vcxproj
		 ofSpoutLibrary.vcxproj.filters
         src <- SpoutLibrary.h and source files
         bin <- SpoutLibrary.dll, openframeworks dlls and data
		 libs <- SpoutLibrary.lib

Open the solution file with Visual Studio 2017, set to "Release" and build.

The "src" folder contains files for a sender application.

Folders within the "src" folder contain the source for a sender and receiver. 
Copy the required ones to the "src" folder.

Pre-built binaries are in the "bin" folder and renamed, so will not be affected by the build.
