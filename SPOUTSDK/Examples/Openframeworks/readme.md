### Spout OpenGL examples

Examples of using the Spout SDK source files with an OpenGL application.\
The examples have been developed for Visual Studio 2022 and [Openframeworks 12](https://openframeworks.cc/).

Openframeworks can be [downloaded](https://openframeworks.cc/download/) for Visual Studio. No installation is necessary.\
Extract the download zip file to a convenient location and rename the folder if required.

The SpoutGL folder and contents must be copied to the Openframeworks "apps" folder. Finally copy the "Examples\Openframeworks" folder and contents to "apps\SpoutGL".

      of_v0.12.0_vs_release <- this folder contains your Openframeworks installation
	    addons <- Openframeworks addons
	    examples <- Openframeworks examples
	    apps
		   SpoutGL <- Copy the Spout SDK "SpoutGL" folder here
	             .
		     . Spout SDK source files
	             .
	          Openframeworks <- copy the entire Openframeworks folder here
                  ofSpoutExample.sln
                  ofSpoutExample.vcxproj
                  ofSpoutExample.vcxproj.filters
                  bin <- application files are generated here
                  Binaries <- application files are accumulated here
                  src <- the required source files
                  Source <- example source files
                  CopySource.bat <- To select the example of interest
                  CopyExe.bat <- The project copies the application files to "Binaries"
                  msbuild.md <- Details for using MSBuild
                  Build.bat <- Build project with MSBuild
                  Build-All.bat <- Build all examples with MSBuild

- Open the solution file with Visual Studio 2022 and set to "Release" and "x64".
- The first time the project is used, re-build "openframeworkslib".\
This ensures that the Openframeworks library is created with the correct build type.
- Build the application "Release/x64".

After build, the example executable files are copied to "Openframeworks\Binaries".

### Source files

The "src" folder contains files for a the example.\
The "Sources" folder contains examples for both Sender and Receiver.\
"ofApp.cpp" and "ofApp.h" are copied the example "src" folder.

Open "CopySource.bat" to select the required example and Rebuild the project.

### C++17 pre-processor definition

Openframeworks 12.0 requires C++ Language Standard C++17.
This causes an error with D3D11.h due to std::byte definition conflict
due to the use of "using namespace std" in ofMain.h. To avoid this
a preprocessor definition _HAS_STD_BYTE=0 is included in the project.

### MSBuild

Refer to "msbuild.md" for using MSBuild with the example project instead of the Visual Studio IDE.






