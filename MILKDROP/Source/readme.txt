Full original plugin source is available here :

http://sourceforge.net/projects/milkdrop2/files/milkdrop2_v2.25c_OPEN_SOURCED_20130514_orig_code.zip/download

It comes with a VC2008 project which has to be modified to enable the SpoutSDK files to be included as well as the required dependencies for DirectX and OpenGL. 

Revisions :

22.10.14 - changed from Ctrl-Z on and off to default Spout output when the plugin starts
           and Ctrl-Z to disable and enable while it is running.
           Otherwise Spout has to be re-enabled every time another track is selected.
30.10.14 - changed from Glut to pixelformat and OpenGL context creation
31.10.14 - changed initialization section to renderframe to ensure correct frame size
	 - added Ctrl-D user selection of DirectX mode
	 - flag bUseDX11 to select either DirectX 9 or DirectX 11
	 - saved DX mode flag in configuration file
05.11.14 - Included Spout options in the Visualization configuration control panel
		Options -> Visualizatiosn -> Configure Plugin
		MORE SETTINGS tab
			Enable Spout output
			Enable Spout DirectX 11 mode
			Settings are saved with OK
	 - retained Ctrl-Z for spout on / off while the Visualizer is running
	 - included Ctrl-D to change from DirectX 9 to DirectX 11 
	   (this might be removed in a future release if it gives trouble)
	   The selected settings are saved when the Visualizer is stopped.
25.04.15 - Changed Spout SDK from graphics auto detection to set DirectX mode to optional installer
	 - Recompile for dual DX option installer
17.06.15 - User observation that custom messages do not work.
	   This is isolated to "RenderStringToTitleTexture" and seems to be related to
	   generating the fonts from GDI to DX9. Not sure of the reason. Could be DX9 libraries.
	   As a a workaround, custom message rendering is replaced with the same as used for 
	   title animation which works OK. The limitation is that this gives a fixed font,
	   but the colour should come out the same as in the custom message setup file.
07.07.15 - Recompile for 2.004 release
15.09.15 - Recompile for 2.005 release - revised memoryshare SDK
08.11.15 - removed directX9/directX11 option for 2.005
	 - OpenSender and milkdropfs.cpp - removed XRGB format option
30.03.16 - Rebuild for Spout 2.005

