--------
Versions
--------

The Spout Processing dll "Jspout.dll" has been developed with Windows 7 and 
tested with both 32bit and 64bit versions of Windows 7.

A 64bit version is installed if that option has been selected. All files in the
"x64" example folders are identical to those in "Win32" apart from "Jspout.dll"
in the "\code" folder.

If you are using the 64bit version of Processing, you need to use the 64bit versions
of the example sketches and vice-versa for 32bit Processing.

However, note that the 32bit version of Processing is recommended by Processing.org.

http://wiki.processing.org/w/Supported_Platforms#Windows

32 bit Processing works equally well on a 64bit system. After installation to a suitable
folder, just run Processing.exe once and thereafter all sketches will open using that version. 
Java is provided with Processing and no additional Java installation is required.


--------------------------------
Using Spout in your own sketches
--------------------------------

A library is not supplied, so the JSpout dll and class files have to be copied to a "code" 
folder within your sketch folder. Follow the folder layout of the example sketches and use 
either the 32bit or 64bit version depending on version of Processing you are using.

A Spout sketch can be exported but is compatible with Windows only. 
After the export has completed, Copy "\code\JSpout.dll" to the lib folder :

For example for 32bit : "application.windows32\lib\".

NOTE: SpoutPanel.exe is used by the Receiver sketch and will be detected if Spout has been 
installed. 

If you have not used the installer, you can copy "SpoutPanel.exe" to the Processing Java 
executable path :

This might be for example :

"..\processing-2.2.1\java\bin\"


