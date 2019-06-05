FREEFRAMEGL

“SpoutReceiver” and “SpoutSender” (32 and 64) are FreeframeGL plugins that provide Spout output and input for host applications that support FreeframeGL. They are built for Spout 2.007 with FreeFrameGL SDK version 1.6 for both 32 bit and 64 bit applications.

Spout Sender - Vers 4.000
Spout Receiver - Vers 4.000

They are not needed where Spout is integrated into the application itself, but will add Spout functionality to FreeframeGL host applications that do not.

The dlls must be copied into the plugin folder that has been defined for the the host application.

For example, detailed instructions for using the 32 bit versions for Isadora by TroikaTronix can be found here :

http://troikatronix.com/support/kb/using-spout-ffgl-video-sharing/

Refer to the Spout pdf manual for more details. For other applications refer to their documentation.

==========================
Update since 2.006 release
16.04.17 - rebuild with VS2012 with original IDs for Isadora
31.10.17 - SpoutReceiver2.dll
         - close receiver on receivetexture fail
           https://github.com/leadedge/Spout2/issues/25
           Version 3.031
17.12.18 - rebuild for Spout 2.007 VS2017 /MT
	   Version 4.000




