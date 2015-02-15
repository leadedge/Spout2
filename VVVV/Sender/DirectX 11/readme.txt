=========================
DIRECTX 11 SENDER EXAMPLE
=========================


The DirectX 11 addon pack must be installed.

http://vvvv.org/contribution/directx11-nodes-alpha

Double click to start "DX11_SenderExample.v4p"
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


After manual entry in this way the sender name will be registered as a Spout sender and 
can be picked up by other Spout receivers. 

If the receiving programs are closed, or the patch closed and re-started the VVVV sender 
may no longer be detected by other receivers. This is becasue the patch saves sender 
details to memory directly and does not initialize or de-initialize as required. If the 
name is entered again in the receiver, the VVVV sender patch will be registered correctly 
once more.

Credit Elio :http://vvvv.org/contribution/directx-freeframegl-bridge


