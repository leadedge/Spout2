---------------------------------------------------------
                     VVVV and Spout
---------------------------------------------------------

SENDER EXAMPLE

Currently this example is for DirectX 9 only

VVVV must be started with the /dx9ex command line option.

A batch file "vvvv_dx9ex.bat" is provided, but is hard coded with a path that
might not be suitable for you, so you need to modify it for your own system.

The command in the batch file is :

C:\VVVV\vvvv_45beta32_x86\vvvv.exe /dx9ex

Edit the batch file to change the VVVV executable path. Then run it to open VVVV.

If this does not work, use the VVVV utility "crack" to find out what the problem might be.

- - - - - - - - - - - - - - - - - 
Start VVVV using the dx9 argument.

Drag and drop "SpoutSenderExample.v4p" into the VVVV window.

Then it will be sending a texture.

1) To receive the texture output with the Spout demo receiver

    - Start the demo receiver.
    - Either RH click on the window or choose File > Select Sender.
    - In the SpoutPanel sender name entry field, type in "videoshare" and ENTER.

Now you have output from VVVV being received by the Spout demo receiver.


2) To receive the texture output in Resolume

    - Start Resolume Avenue or Arena. 
    - Select “SpoutReceiver” from the list of Sources and drag it to an empty cell. 
    - Enter the VVVV patch sender name by typing in its name “videoshare”
      into the “Sender name” field. Then click “Update”.

Now you have output from VVVV being received by Resolume. 


After maunal entry in this way the sender name will be registered as a Spout sender and can be picked up by other Spout receivers. 

If the receiving programs are closed, or the patch closed and re-started the VVVV sender may no longer be detected by other receivers. This is becasue the patch saves sender details to memory directly and does not initialize or de-initialize as required. If the name is entered again in the receiver, the VVVV sender patch will be registered correctly once more.


Credit: Elio Wahlen
---------------------------------------------------------

RECEIVER EXAMPLE

The DirectX 11 addon pack must be installed.

http://vvvv.org/contribution/directx11-nodes-alpha


    - Run the Spout DirectX 11 demo sender.
      The default sender name will be : "Spout SDK DX11 Sender 32bit.exe"
    - Double click on "SpoutReceiverExample.v4p".
      
The receiver should immediately start to display the sender output.
The sender name can be changed for other senders.

Credits : OysterBoy. Newemka.
---------------------------------------------------------

WEBCAM EXAMPLE

    - Start any Spout sender.
    - Double click on "spoutcamIn.v4p".

The output of the sender can be picked up in VVVV by selecting SpoutCam as the webcam. 

Notes :

1) The option to install "SpoutCam must have been selected on Spout installation
2) Make sure a Spout sender is running before starting this patch.
3) For other webcams make sure the width and height match for "VideoIn".

---------------------------------------------------------