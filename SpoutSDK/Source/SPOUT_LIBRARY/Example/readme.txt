SpoutLibrary dll example

This is an Openframeworks example for Codeblocks and has been tested with Openframeworks 0.8.4. 

The project is dependent on the Openframeworks environment and so the installation "ofSpoutLibraryExample" folder and contents has to be copied to the Openframeworks folder. Also the source files of the Spout SDK are required and will be compiled into the application. These are not set up as an addon, so they must be copied to a "SpoutSDK" folder within the Openframeworks folder

C:\OPENFRAMEWORKS
    addons
    examples
    apps
      SpoutSDK <- copy the Spout SDK source files into here (under "apps")
      myApps
        cbSpoutLibrary <- copy the entire example folder here (under "apps\myApps")
         ofSpoutLibrary.cbp
         src <- the required source files will be in here
         bin <- the required dlls and data are in here

Open the project fle with Codeblocks, set to "Release" and build.

The "src" folder contains files for a sender application.

Folders within the "src" folder contain the source for a sender, receiver, multiple receiver and a sender/receiver. 
Copy the required ones to the "src" folder.

Pre-built binaries are in the "bin" folder and renamed, so will not be affected by the build.
