This branch contains code, libraries, examples and plugins for Spout 2.007. There have been significant changes and so practical testing is necessary before release. Your time and feedback is most welcome. Contact via spout@zeal.co or my personal email address if I have contacted you directly.

INSTALLATION
Installation is not required.
If Spout is installed, un-install via Windows or "C:\Program Files (x86)\Spout2\unins000.exe".
Download the repository and unzip to any convenient location.

SPOUT SETTINGS
Run "SpoutSettings" from it's location and choose the settings you require.
OK and close will write these settings to the registry. 
"Frame count" is not recognized or changed by 2.006 applications.
CPU mode is ignored for 2.007.

SPOUTCAM
Run "SpoutCamSettings" and select the "Register" button.
Both 32 bit and 64 bit versions of SpoutCam will be registered on a 64 bit system.
Once registered you can use the same button to un-register it.

SPOUTPANEL
If Spout has been installed, copy SpoutPanel.exe to "C:\Program Files (x86)\Spout2\SPOUTPANEL\"
Otherwise, run the program once to establish it's location for all receivers to use.

SPOUT DEMO PROGRAMS
SpoutSender and SpoutReceiver are compatible with 2.006 applications.
Diagnostics have been improved and you can examine logs for all applications that produce a log file.
Help > Diagnostics > Logs.

SPOUT SDK EXAMPLES
Example code is well commented and serves as documentation until that is updated.
The Openframeworks graphics sender gives the most information on new 2.007 functions.
The Spout SDK source is also well documented.
New DirectX examples show how Spout can be used without OpenGL.
Cinder does not support Visual Studio 2017 so the example solution file is for VS2012.
All examples have pre-built binaries for testing.

SPOUTLIBRARY
Library files are updated for 2.007 and should be compatible with 2.006 applications.
The example is for Openframeworks and the Codeblocks example is discontinued.
The library has been tested successfully with QT/MingW 32 and 64 bit.
Importantly use the updated SpoutLibrary.h header to build with the new library and dll files.
Unless you are building the library itself you don't need to include SpoutFunctions.h.

VISUAL STUDIO
Visual Studio 2017 solution files should load and build straight away from their current locations.
Change to the project properties might be necessary to the Windows SDK version being used with your compiler.
Properties > Configuration Properties > General > Windows SDK version.

PROCESSING
Un-install Spout 2.006 from the import manager.
Close and restart Processing to delete the library.
Install the 2.007 Spout library manually.
https://github.com/processing/processing/wiki/How-to-Install-a-Contributed-Library
  Unzip spout-7.zip to create a "spout" folder (you will need to rename from "spout-7").
  Copy the spout folder with all it's contents to the "libraries"
  folder within your Processing sketchbook folder.
Example sketches are in : ..\libraries\spout\examples and are well commented.
Existing 2.006 sketches should work OK apart from the receiver
where you will need to add "spout.drawTexture();" after receiving the texture.
The Processing repository will be updated before 2.007 release.

FREEFRAMEGL
Freeframe plugins remain part of the Spout distribution for programs that still need them.
Hosts currently supporting Spout can use them for testing.

MILKDROP
Copy vis_milk2.dll from the Spout distribution to the Winamp plugins folder.
The plugin is built with Visual Studio 17 using the Visual Studio 2010 toolset v100 and the Spout 2.007 SDK
and is compatible with previous Spout versions.

OTHER PLUGINS
Will be completed as time allows.

VISUAL STUDIO RUNTIME
If you do not have Visual Studio 2017 installed and experience reports of missing dlls, you may need to install the VS2017 runtime. The installer files are located in the SPOUTSDK\VS2017-runtime folder. On a 64 bit system, run vc_redist.x64.exe before vc_redist.x86.exe.

REMOVAL
To completely remove Spout from your system :
1) Start > Run > Regedit
     Click on HKEY_CURRENT_USER
     Edit > Find > Spout
     Look for and delete : HKEY_CURRENT_USER\Software\Leading Edge\Spout
     Or if there are no ther Spout applications installed, remove :
     HKEY_CURRENT_USER\Software\Leading Edge
2) Start the 2.007 Spout demo sender.
     Help > Diagnostics > Logs will open the folder in File Explorer.
     Close the demo sender.	
     Back out one level to "Roaming" and delete the "Spout" folder.
   Alternatively : Start > Run > %appdata%
     This will open the "C:\Users\User\AppData\Roaming" folder.
     Find the "Spout" folder and delete it.
