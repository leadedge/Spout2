### Spout example for [Cinder](https://libcinder.org)

This is an example of using the Spout SDK source files with a Cinder OpenGL application.\
The example has been created for Visual Studio 2017 and Cinder 0.9.3.

Note that the project target is Visual Studio 2017 (V141).\
and the Cinder library should also be re-targeted and rebuilt.

To rebuild the Cinder library for VS2017
1) Copy the proj\vc2015 folder to a new one proj\vc2017
2) Remove any existing build files
3) Open Cinder.sln with VS2017 and Re-Target
4) Build the library x64 or Win32

Building the example project
1) In your Cinder installation folder, create a "SpoutGL" folder.
2) Copy all the source files of the Spout SDK to the "SpoutGL" folder.
3) Find the Cinder example project "..\samples\_opengl\FboBasic
4) Back up the original folder.
5) Replace with this example folder.
6) Open the solution file with Visual Studio 2017.
7) Examine the source for sender or receiver.

Sender / Receiver option

  // #define _receiver
  Enable this define to create a receiver. Disable it to create a sender.

SpoutPanel

The receiver example depends on SpoutPanel.exe to select senders.
It's path is detected after either SpoutSettings or SpoutPanel has been run once.

