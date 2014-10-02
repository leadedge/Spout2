//
//             Spout Receiver
//
//      Sharing image frames between applications
//      by Opengl/directx texture or memory map sharing
//
//      Demonstrates receiving an image from a Spout sender
//      The sender can be any of the Spout example senders.
//
//      See Spout.pde for function details
//
import java.awt.*; // needed for frame insets

PImage img;
Insets insets; // Frame caption and borders for resizing

// DECLARE A SPOUT OBJECT HERE
Spout spout;


void setup() {
  
  size(640, 360, P2D);
  frame.setResizable(true); // Optionally adapt to sender frame size
  insets = frame.getInsets(); // Get the caption and borders for frame resizing
  background(0);
  
  // Create an image to receive the data.
  img = createImage(width, height, ARGB);
  
  // CREATE A NEW SPOUT OBJECT HERE
  spout = new Spout();
  
  // INITIALIZE A SPOUT RECEIVER HERE
  // Give it the name of the sender you want to connect to
  // Otherwise it will connect to the active sender
  // img will be updated to the sender size
  if(!spout.initReceiver("", img)) exit();
  
} 

void draw() {
  
  // RECEIVE A SHARED TEXTURE HERE
  img = spout.receiveTexture(img);
  
  // If the image has been resized, optionally resize the frame to match.
  if(img.width != width || img.height != height && img.width > 0 && img.height > 0) {
      // Reset the frame size - include borders and caption
      frame.setSize(img.width + (insets.left + insets.right), img.height + (insets.top + insets.bottom));      
  }
  
  // Draw the result
  image(img, 0, 0, width, height);
}


// RH click to select a sender
void mousePressed() {
  // SELECT A SPOUT SENDER HERE
  if (mouseButton == RIGHT) {
    // Bring up a dialog to select a sender from
    // the list of all senders running.
    JSpout.SenderDialog();
  }
}

// over-ride exit to release sharing
void exit() {
  // CLOSE THE SPOUT RECEIVER HERE
  spout.closeReceiver();
  super.exit();
} 


