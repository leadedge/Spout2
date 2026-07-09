@echo off
rem
rem Example of command line for a video file
rem Enter the name of the video file, including extension
rem
rem This can be :
rem
rem A full path.
rem "..\..\SpoutDXvideo" " .. full path to file .. \videofile.mp4"
rem
rem Relative assumes that the video file exists there.
rem Relative to the executable folder
rem "..\..\SpoutDXvideo" "\DATA\Wildlife.mp4"
rem Or one level above for example.
rem "..\..\SpoutDXvideo" "..\Wildlife.mp4
rem
rem A file in the "\DATA\Videos" folder.
"..\..\SpoutDXvideo.exe" "Wildlife.mp4"
rem
rem
rem This batch file can be run without showing a console by "aa-start.vbs"
rem

