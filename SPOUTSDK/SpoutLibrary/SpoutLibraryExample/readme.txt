SpoutLibrary example

This is an example for Visual Studio 2017 and Openframeworks 11.

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
         bin <- SpoutLibrary.dll and data files
		 libs <- SpoutLibrary.lib

Open the solution file with Visual Studio 2017, set to "Release" and build.

Source files

The "src" folder contains files for a Sender example.
The "Sources" folder contains files for both Sender and Receiver.
For the one you want copy "ofApp.h" and "ofApp.cpp" to the "src" folder and "Rebuild".
