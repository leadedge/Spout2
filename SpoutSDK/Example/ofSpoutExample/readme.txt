OPENFRAMEWORKS EXAMPLES

The "ofSpoutExample" folder and contents has to be copied to the Openframeworks apps\myApps folder.

The source files of the Spout SDK are required and will be compiled into the application. These are not set up as an addon, so they must be copied to a "SpoutSDK" folder within the Openframeworks folder

C:\OPENFRAMEWORKS
    addons
    examples
    apps
      SpoutSDK <- create this folder (under "apps") and copy the Spout SDK source files into it 
      myApps
        ofSpoutExample <- copy the entire example folder here (under "apps\myApps")
         ofSpoutExample.sln
         ofSpoutExample.vcxproj
         ofSpoutExample.vcxproj.filters
         src <- the required source files for different examples will be in here
         bin <- the required dlls and data are in here

Open the solution file with Visual Studio 2012 or later, set to "Release" and build "ofSpoutExample".

The "src" folder contains files for example applications.
The folders "src\ofSpoutReceiver" and "src\ofSpoutSender" contain the application source files for a receiver and sender examples respectively.
Copy the required ones to the "src" folder. The other source files are common to both.

Openframeworks examples have been tested with Visual Studio 2012 and Openframeworks 0.8.4. 



