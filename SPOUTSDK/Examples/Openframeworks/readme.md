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
	          ofSpoutExamples <- copy the ofSpoutExamples folder here (in the SpoutGL folder)
                ofSpoutExamples.sln
				  Receiver
				    ReceiverData
					ReceiverGraphics
					ReceiverMultiple
					ReceiverSync
				  Sender
				    SenderData
					SenderGraphics
					SenderSync
					SenderUtilities
					SenderVideo
					SenderWebcam
- Open the solution file with Visual Studio 2022 and set to "Release" and "x64".
- The first time the project is used, re-build "openframeworkslib".\
This ensures that the Openframeworks library is created with the correct build type.
- Build the library "Release/x64".\
A "dll" folder is created in the ofSpoutExamples folder. This is used by all projects to copy the required dlls to the executable folder.

After build of the Openframeworks library, each example project can be selected indpendently.
Right click on the example and "Set as Startup Project". The top level "ofExamples" solution
can also be selected for build. This will build all the example projects.\
Applications are accumulated in the project "Binaries" folder.

### C++17 pre-processor definition

Openframeworks 12.0 requires C++ Language Standard C++17.
This causes an error with D3D11.h due to std::byte definition conflict
due to the use of "using namespace std" in ofMain.h. To avoid this
a preprocessor definition _HAS_STD_BYTE=0 is included in the example projects.

### MSBuild

Refer to "msbuild.md" for using MSBuild instead of the Visual Studio IDE.






