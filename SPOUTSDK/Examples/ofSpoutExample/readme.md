### Spout SDK OpenGL example

This is an example of using the Spout SDK source files with an OpenGL application.\
The example is for Visual Studio 2017 and Openframeworks 11.

The "ofSpoutExample" folder and contents has to be copied to the Openframeworks apps\myApps folder.

The source files of the Spout SDK are required and will be compiled into the application.
These are not set up as an addon, so they must be copied to a "SpoutGL" folder within the Openframeworks folder

      C:\OPENFRAMEWORKSSpoutGL <- this folder contains your Openframeworks installation
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

Open the solution file with Visual Studio 2017, set to "Release" and build "ofSpoutExample".

Source files

The "src" folder contains files for a Sender example.\
The "Sources" folder contains files for both Sender and Receiver.\
For the one you want copy "ofApp.h" and "ofApp.cpp" to the "src" folder and "Rebuild".

Methods for 2.006 remain compatible. They are lower level and may be preferred for your application if you want more control over the process. The 2.006 folder contains Sender and Receiver examples.

SpoutPanel

The receiver example depends on SpoutPanel.exe to select senders.
It's path is detected after either SpoutSettings or SpoutPanel has been run once.
Otherwise it must be copied to the folder for the binary
