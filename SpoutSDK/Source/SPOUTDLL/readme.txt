"SpoutDLL" is a C compatible DLL for use of the Spout SDK
with compilers other than Microsoft Visual Studio.

The OpenFrameworks examples use CodeBlocks and MingW GCC compiler.

The "VS2010" and "VS2012" folders contain Visual Studio projects to build the dll.
The installation folders are set up correctly for building the project.
However, building from within the installation folder is not recommended.
Copy the distribution "SPOUTSDK" folder elsewhere before compilation.
Open the "SpoutDLL" solution, change to "Release" and build the project.
Spout2.dll and Spout2.lib will be in the Win32\Release folder.

THIS VERSION IS INTENDED FOR USE WITH COMPILERS OTHER THAN VISUAL STUDIO


ALTERNATIVE

The complete Spout SDK can also be compiled as a dll with Visual Studio
which will give access to all functions in the Spout SDK and using exactly the
same code as when compiling the SDK files directly into the project.

However, this is suitable only for programs compiled with Visual Studio. 
See the installation folder "SPOUTSDK\SpoutSDK\VS2010". 
Credit to Malcolm Bechard for the VS2010 dll project.






