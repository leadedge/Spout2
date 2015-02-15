Spout2 - Spout SDK examples

The CINDER and OpenFrameWorks examples depend on these host systems being installed
and experience with them.

These examples also require all the files from the Spout SDK.

https://github.com/leadedge/Spout2

You can arrange the folders as you wish. However, the examples have been compiled with
the Spout SDK files installed in a folder two levels above the application source. 

For example for OpenFrameWorks:

OpenFrameWorks
    examples
    apps
        myApps
            ofSpoutExample
                src
                bin
        SpoutSDK

SpoutPanel.exe is an executable program used by a Spout receiver application. It's path is detected from the registry after installation. Otherwise it must be copied to the folder for the binary e.g. "bin".

