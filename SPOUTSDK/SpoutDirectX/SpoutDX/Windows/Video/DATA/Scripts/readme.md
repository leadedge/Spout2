##Command line for SpoutDXvideo##

Edit the file "aa-video.bat" to change the video file

This can be :

1) A full path.
"..\..\SpoutDXvideo" " .. full path to file .. \videofile.mp4"

2) Relative to the executable folder
"..\..\SpoutDXvideo" "\DATA\videofile.mp4"

3) A file in the "\DATA\Videos" folder.
"..\..\SpoutDXvideo" "videofile.mp4"

"aa-video.bat" can be run from a command window.

If run directly, a console window will the open for the command prompt.
If you don't want a console window to show, run "aa-start.vbs".
SpoutDXvideo.exe will star minimized on the taskbar.
You will know it's working by opening a Spout receiver.

SpoutDXvideo can also be activated by command line
from a program using ShellExecute of similar
