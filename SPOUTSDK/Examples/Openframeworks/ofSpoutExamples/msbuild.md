## Using MSBuild for example projects

After setting up OpenFrameworks and copying the required files as explained in "readme.md",\
MSBuild can be used to to automate the build process instead of using the Visual Studio IDE.

If Visual Studio 2022 has previously been installed, skip to the <b>"Environment path"</b> section.

If you do not have Visual Studio, you can still use MSBuild to build the example projects.\
The Build Tools can be installed independently as follows.

### Download

Download from :
<pre>https://visualstudio.microsoft.com/visual-cpp-build-tools/</pre>
Click "Download Build Tools" and save the file (vs_BuildTools.exe).

### Installation

- Run "vs_BuildTools.exe"
- "Visual Studio Installer" will appear but this will only install the Visual Studio Build Tools.
- Click "Continue" for the getting started message and wait for completion.
- Click on "Desktop development with C++" at top left in the installation window.
- Then click "Install" at bottom right and wait for completion of the installation.

### Environment path

After installation of Build Tools, confirm the location of MSBuild.exe at :
<pre>C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin</pre>
or, if Visual Studio 2022 has previously been installed, there will be a copy in :
<pre>C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin</pre>
MsBuild is run from the command line from a console window.\
Create an Environment Variable so that you don't have to enter the full path every time.

- In the search window, enter "Environment" and choose<br>"Edit the system environment variables"
- Click the "Environment variables" button.
- In the top pane you will see the "Path" variable.
- Double click this to bring up the "Edit environment variable" window.
- Click "New" and enter the path as found above.
- Click OK to enter the new path and OK again to exit.

MSbuild is now ready to run from a console window without having to enter the full path.

### Build the Openframeworks library

The first time the project is used, re-build "openframeworkslib".\
This only needs to be done once, and ensures that the Openframeworks library is created with the correct build type.\
The Visual Studio project for Openframeworks is "openframeworksLib.vcxproj" and is located in :

<pre>of_v0.12.0_vs_release\libs\openFrameworksCompiled\project\vs\</pre>

- In File Explorer move to this location and left click on an empty space in the address bar to highlight it.
- Type cmd to replace the highlighted text and press enter.<br>
A command window will open in that folder.
- Now type in or copy/paste :

<pre>MSBuild openframeworksLib.vcxproj -p:Configuration=RELEASE -p:Platform=x64</pre>

- Enter to build.

After the build is complete you should see "Build succeeded."\
There may be warnings, but the important result is "0 Error(s)".\
Now that the Openframeworks library is compiled, close the console window.

### Build an example

The command to build an example project "SenderGraphics" is :

<pre>msbuild.exe Sender\SenderGraphics\SenderGraphics.vcxproj -vm /p:Configuration=Release /p:Platform=x64</pre>

This is contained in a batch file "Build.bat".\
Modify this batch file for the required example.

Run "Build.bat". The build should succeed with 0 Warnings and 0 Errors.\
Example applications can be found in the relevant project "bin" folder.

### Build all examples

Run "Build-All.bat" to build all the example projects.
Applications are accumulated in the project "Binaries" folder.





