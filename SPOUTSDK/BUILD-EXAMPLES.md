## Building the OpenGL examples

Examples are provided for Cinder and Openframeworks. For Cinder, this is limited to one example. Refer to "readme-first.md" in the Cinder\FboBasic folder

The "ofSpoutExample" examples are dependent on Openframeworks and Visual Studio. Openframeworks has to be downloaded and the SpoutGL files and example project have to be copied to the right locations.

If you familiar with Visual Studio and Openframeworks, the project solution file "ofSpoutDemo.sln" can then be used directly. The project can also be built without the Visual Studio IDE and without any previous knowledge of Visual Studio or Openframeworks, by using "MSBuild" with only one line from a command window.

### Openframeworks

#### Download

- Download from https://openframeworks.cc/download/
- Choose download openFrameworks for "visual studio"
- Save the zip file, for example "of_v0.12.0_vs_release.zip", to a convenient location for the build.
- Right click on the zip file and "Extract all".
- Remove "of_v0.12.0_vs_release" from the end of the extract path.<br>
  This is a very large file and it can take some time to download.

After completion, you will have a new folder "of_v0.12.0_vs_release" with several sub-folders.
The important one is "apps"
<pre>
apps
  myApps
    emptyExample
</pre>

## The OpenGL example project

### Download the Spout repository

- Go to the root of the repository https://github.com/leadedge/Spout2
- Click the green "Code" button
- Scroll down and click "Download Zip" and download to a convenient location.
- Right click and "Extract all"
- This will create a folder "Spout2-master\".
- Copy the "Spout2-master\SPOUTSDK\SpoutGL" folder to the Openframeworks folder "apps\".
- Copy the "Spout2-master\SPOUTSDK\Examples\ofSpoutExample" folder to "apps\SpoutGL\"
- Now there will be :

<pre>
  apps
    myApps
    SpoutGL
	  ofSpoutExample
	   ..
	   .. SpoutGL files ..
	   ..
 </pre>

Now that the code is in place, it's ready for build with either Visual Studio or MSBuild.

 ## Visual Studio
 
If you have Visual Studio installed, open "apps\SpoutGL\ofSpoutExample.sln" and set to "Release".<br>
The first time you use Openframeworks you have to compile the library.<br>
Right click on "openframeworksLib" in the project explorer and "Build".<br>
Now you can build the project.<br> 
Expand "ofSpoutExample" in the "Solution Explorer" pane and open "ofApp.cpp".<br>
Build options are documented and can be explored.<br>
ofApp::draw() has options for sending fbo, texture or image.<br>
Right click on "ofSpoutExample" and "Build".<br>
You will find "ofSpoutExample.exe" in the "bin" folder. Rename it to "SpoutSender.exe".<br>
Before opening it, go to the "x64dlls" folder and copy all the dll files to "bin".

### Building other examples

Additional examples can be found in the "ofSpoutExample\Source" folder

<pre>
ofSpoutExample
  Source
    Receiver
	  Data
	  Graphics
	  Multiple
	  Sync
    Sender
	  Data
	  Graphics
	  Sync
	  Video
	  Webcam
 </pre>
 
Each of these contains "main.cpp", "ofApp.cpp", "ofApp.h". Copy these three files from the example of interest to "ofSpoutExample\src\". "Rebuild" the ofSpoutExample project. All builds result in "ofSpoutExample.exe". Rename it as required.

   
## MSBuild

If you are not familiar with Visual Studio or do not have it installed, you can still build the project using MSBuild to automate the process.

### Download

Download from :

https://visualstudio.microsoft.com/visual-cpp-build-tools/

Click "Download Build Tools" and save the file (vs_BuildTools.exe).

### Installation

- Run "vs_BuildTools.exe"
- "Visual Studio Installer" will appear but this will<br>only install the Visual Studio Build Tools.
- Click "Continue" for the getting started message and wait for completion.
- Click on "Desktop development with C++" at top left in the installation window.
- Then click "Install" at bottom right and wait for completion of the installation.

### Environment path

Confirm the location of MSBuild.exe at :

C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin

MsBuild is run from the command line from a console window. Create an Environment Variable with this path so that you don't have to enter the full path every time.

- In the search window, enter "Environment" and choose<br>"Edit the system environment variables"
- Click the "Environment variables" button.
- In the top pane you will see the "Path" variable.
- Double click this to bring up the "Edit environment variable" window.
- Click "New" and enter the path as found above.
- Click OK to enter the new path and OK again to exit.

MSbuild is now ready to run from a console window without having to enter the full path.


## Build the Openframeworks library

The Visual Studio project for Openframeworks is "openframeworksLib.vcxproj"<br>
and is located in :

of_v0.12.0_vs_release\libs\openFrameworksCompiled\project\vs\

- In File Explorer left click on the address bar to highlight this location
- Type cmd to replace the highlighted text and press enter.<br>
A command window will open in that folder.
- Now type in or copy/paste :

MSBuild openframeworksLib.vcxproj -p:Configuration=RELEASE -p:Platform=x64

- Enter to build.

You might see warnings, but the important thing after the build is complete is "0 Errors".
Now the Openframeworks library is compiled.

	  
## Build the example project

In File Explorer move to the project folder within Openframeworks :

"of_v0.12.0_vs_release\apps\SpoutGL\ofSpoutExample" 

- As before, left click on the address bar to highlight the location
- Type "cmd" to replace the highlighted text and enter.
- Type in or copy/paste :

MSBuild ofSpoutExample.sln -p:Configuration=RELEASE -p:Platform=x64

- Press enter to build.

The build should succeed with 0 Warnings and 0 Errors.<br>
You will find "ofSpoutExample.exe" in the "bin" folder.<br>

The project build is now complete.

