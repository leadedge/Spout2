If you are using Microsoft Visual Studio, you have the option to compile the Spout SDK as a dll rather than including the source files in your project.

The installation folders are set up correctly for building the sdk dll project. Open the "SPOUTSDK_DLL\SpoutSDK" solution, change to "Release" and build the project. Spout.dll and Spout.lib will be in the Win32\Release or x64\Release folder.

Prebuilt binaries can be found in the Binaries folder.

If you use the .dll created from this project, define SPOUT_IMPORT_DLL in your preprocessor compilation defines.

Thanks and credit to Malcolm Bechard for creating the dll project.

https://github.com/mbechard

COMPATIBILITY

The dll created from this project has the advantage that all the functions in the Spout SDK classes are available. However, it is only suitable for use with Visual Studio compilers. For other compilers, see the C compatible dll in the "SPOUTSDK\SpoutLibrary" folder.



