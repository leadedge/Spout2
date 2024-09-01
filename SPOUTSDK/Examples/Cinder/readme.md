## Spout example for [Cinder](https://libcinder.org)

This is an example of using the Spout SDK source files with a Cinder OpenGL application.\
The example has been created for Visual Studio 2022 and Cinder 0.9.3.

### To build the Cinder library with VS2022

- Create a new folder "VS2022" under the Cinder root folder "proj"
- Copy the contents of "proj\vc2015" to "proj\vc2022"
- Remove any existing "build" folder
- Open Cinder.sln with VS2022
- When you see the Retarget projects dialog, click CANCEL\
The solution will be renamed to "cinder (Visual Studio 2015)"
- Select Project > Properties > General
- Configuration > All Configurations / Platform > All Platforms
- Platform Toolset > Visual Studio 2017 (v141)
- OK to establish settings\
The solution will be renamed to "cinder (Visual Studio 2017)"
- File > Save All - to save the changes
- Rebuild the library x64

### Building the example project

- Create a "SpoutGL" folder under the Cinder root folder.
- Copy all the source files of the Spout2 SDK "SpoutGL" folder into it.
- Find the Cinder example project "..\samples\_opengl\FboBasic"
- Back up the original FboBasic folder.
- Replace with the Spout example FboBasic folder.
- Open "vc2022 > FboBasic.sln" with Visual Studio 2022.
- As before, click CANCEL for the Retarget projects dialog.
- Examine FboBasicApp.cpp for sender or receiver compile definition :
  <pre>// #define _receiver</pre>
  - Enable this define to create a receiver.
  - Disable it to create a sender.
- Build Release x64
- Select "File > Save All" before closing to save the project settings



