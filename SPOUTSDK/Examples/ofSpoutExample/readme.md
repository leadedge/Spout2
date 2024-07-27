### Spout SDK OpenGL examples

Examples of using the Spout SDK source files with an OpenGL application.\
The examples have been developed for Visual Studio 2022 and [Openframeworks 12](https://openframeworks.cc/).

Openframeworks can be [downloaded](https://openframeworks.cc/download/) for Visual Studio. No installation is necessary.\
Extract the download zip file to a convenient location and rename the folder if required.

Copy the "ofSpoutExample" folder and contents to the Openframeworks "apps\myApps" folder.

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
	          ofSpoutExample <- copy the entire ofSpoutExample folder here (under "apps\myApps")\
                  ofSpoutExample.sln
                  ofSpoutExample.vcxproj
                  ofSpoutExample.vcxproj.filters
                  src <- the required source files
                  bin <- the required data files
			      Source <- examples

- Open the solution file with Visual Studio 2022, set to "Release".
- The first time the project is used, re-build "openframeworkslib".\
This ensures that the Openframeworks library is created with the correct build type\
and that the required dependent dll files are copied to a "dll" folder in the project folder.
- Build the application.

### Source files

The "src" folder contains files for a the example.\
The "Sources" folder contains examples for both Sender and Receiver.\
"main.cpp", "ofApp.cpp" and "ofApp.h" are copied the example "src" folder.

Open "CopySource.bat" to select the required example and Rebuild the project.

### Legacy examples

The Spout2 > 2.006 branch contains Sender and Receiver examples. Methods used remain compatible with 2.007. 

### C++17 pre-processor definition

Openframeworks 12.0 requires C++ Language Standard C++17.
This causes an error with D3D11.h due to std::byte definition conflict
due to the use of "using namespace std" in ofMain.h. To avoid this
a preprocessor definition _HAS_STD_BYTE=0 is included in the project.







