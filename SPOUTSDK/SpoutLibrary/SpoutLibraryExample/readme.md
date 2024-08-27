### Spoutlibrary example

This is an example of using SpoutLibrary for Visual Studio 2022 and Openframeworks 12](https://openframeworks.cc/).

Openframeworks can be [downloaded](https://openframeworks.cc/download/) for Visual Studio. No installation is necessary.\
Extract the download zip file to a convenient location and rename the folder if required.

Copy the "SpoutLibraryExample" folder and contents to the Openframeworks "apps\myApps" folder.

The source files of the Spout SDK are required and will be compiled into the application.\
The files must be copied to a "SpoutGL" folder within the Openframeworks folder. 

      of_v0.12.0_vs_release <- this folder contains your Openframeworks installation
	    addons <- Openframeworks addons
	    examples <- Openframeworks examples
	    apps
	       SpoutGL <- create this folder under "apps" and copy the source files in the "SpoutGL" folder into it
	          .
		  . Spout SDK source files
	          .
	       myApps <- Openframeworks application folder
	          SpoutLibraryExample <- copy the entire folder here (under "apps\myApps")\
                  ofSpoutLibrary.sln
                  ofSpoutLibrary.vcxproj
                  ofSpoutLibrary.vcxproj.filters
                  src <- example source files
                  bin <- example data files
				  libs <- SpoutLibrary.lib
			      Source <- examples

- Open the solution file with Visual Studio 2022, set to "Release".
- The first time the project is used, re-build "openframeworkslib".\
This ensures that the Openframeworks library is created with the correct build type\
and that the required dependent dll files are copied to a "dll" folder in the project folder.
- Build the application. After build, the compiled example will be in the "bin" folder.

### Source files

The "src" folder contains files for a the example.\
The "Sources" folder contains files for :
    Sender
    Receiver
	Multiple receivers
	Dynamic (Bare bones sender with dynamic load of SpoutLibrary)
"main.cpp", "ofApp.cpp" and "ofApp.h" are copied the example "src" folder.
	
Open "CopySource.bat" to select the required example and Rebuild the project.









