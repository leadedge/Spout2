### Spout SDK OpenGL examples

Examples of using the Spout SDK source files with an OpenGL application.\
The examples have been developed for Visual Studio 2022 and Openframeworks 12.

The "ofSpoutExample" folder and contents has to be copied to the Openframeworks apps\myApps folder.

The source files of the Spout SDK are required and will be compiled into the application.
These are not set up as an addon, so they must be copied to a "SpoutGL" folder within the Openframeworks folder. 
This could be renamed from the original distribution folder.

      C:\OPENFRAMEWORKS <- this folder contains your Openframeworks installation
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

Open the solution file with Visual Studio 2022, set to "Release" and build "ofSpoutExample".

Source files

The "src" folder contains files for a Sender example.\
The "Sources" folder contains files for both Sender and Receiver and options for each.\
For the one you want copy "ofApp.h" and "ofApp.cpp" to the "src" folder and "Rebuild".

The Spout2 > 2.006 branch also contains Sender and Receiver examples and methods used remain compatible. 

C++17 pre-processor definition

Openframeworks 12.0 requires C++ Language Standard C++17.
This causes an error with D3D11.h due to std::byte definition conflict
due to the use of "using namespace std" in ofMain.h. To avoid this
a preprocessor definition _HAS_STD_BYTE=0 is included in the project.







