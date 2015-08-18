This is an Openframeworks example for Visual Studio 2012 and has been tested with Openframeworks 0.8.4. 

The project is dependent on the Openframeworks environment and so the installation "ofSpoutExample" folder and contents has to be copied to the Openframeworks folder. Also the source files of the Spout SDK are required and will be compiled into the application. These are not set up as an addon, so they must be copied to a "SpoutSDK" folder within the Openframeworks folder

C:\OPENFRAMEWORKS
    addons
    examples
    apps
      SpoutSDK <- copy the Spout SDK source files into here (under "apps")
      myApps
        ofSpoutExample <- copy the entire example folder here (under "apps\myApps")
         ofSpoutExample.sln
         ofSpoutExample.vcxproj
         ofSpoutExample.vcxproj.filters
         src <- the required source files will be in here
         bin <- the required dlls and data are in here

Open the solution fle with Visual Studio 2012, set to "Release" and build "ofSpoutExample".

The "src" folder contains files for a sender application. The folders "src\ofSpoutReceiver" and "src\ofSpoutSender" contain the application source files for a receiver and sender respectively. Copy the required ones to the "src" folder. The other source files are common to both.

Pre-built binaries are in the "bin" folder and renamed, so will not be affected by the build.


