This is an OpenFrameworks example for CodeBlocks and has been tested with CodeBlocks 13.12 and Openframeworks 0.8.4 for CodeBlocks and MingW. 

The project is dependent on the Openframeworks environment and so the installation "cbSpoutExample" folder has to be copied to the Openframeworks folder. For example :

C:\OPENFRAMEWORKS
    addons
    examples
    myApps
      cbSpoutExample <- copy the entire example folder here
         cbSpoutExample.cbp
         cbSpoutExample.workspace
         src <- the required source files and Spout2.lib will be in here
         bin <- the Spout2.dll is in here


The "src" folder contains files for a sender application. The folders "src\cbSpoutReceiver" and "src\cbSpoutSender" contain the application source files for a receiver and sender respectively. Copy the required ones to the "src" folder. The other source files are common to both.

Pre-built binaries in the "bin" folder have been renamed and will not be affected by the build.



