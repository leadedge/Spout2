//
//
//                  Spout.pde
//
//    Adds support to the basic functions of the 
//    "JSpout" class and JNI dll, defined in JSpout.java
//
//    07.05.14 - updated for screen resize with sender change
//             - updated Jspout ReadTexture as well
//    06.08-14 - updated for Spout SDK
//    05.09.14 - update with revised SDK
//
import processing.opengl.*;
import java.awt.*; // needed for frame insets

class Spout
{
  
  PGraphicsOpenGL pgl;
  int[] dim = new int[2];
  int memorymode; // memorymode flag
   
  Spout () {
    pgl = (PGraphicsOpenGL) g;
    dim[0] = 0;
    dim[1] = 0;
    memorymode = -1; // default
  }  
  
  //
  // Initialize a sender 
  //
  
  // For texture sharing, the name provided 
  // is registered in the list of senders
  // Texture share initialization only succeeds if 
  // the graphic hardware is compatible, otherwise
  // it defaults to memoryshare mode
  void initSender(String name, int Width, int Height) {
    // Try Texture share (mode 0)
    // If texture init fails, the mode returned is memoryshare
    // unless that fails too, then it returns -1
    memorymode = JSpout.InitSender(name, Width, Height, 0);
    if(memorymode == 0)
      print("Sender initialized texture sharing\n");
    else if(memorymode == 1)
      print("Sender texture sharing not supported - using memory sharing\n");
    else if(memorymode == -1)
      print("Sender sharing initialization failed\n");
    
  }
  
  // Write the sketch drawing surface texture to 
  // an opengl/directx shared texture
  void sendTexture() {
      pgl.beginPGL();
      // Load the current contents of the renderer's
      // drawing surface into its texture.
      pgl.loadTexture();
      // getTexture returns the texture associated with the
      // renderer's. drawing surface, making sure is updated 
      // to reflect the current contents off the screen 
      // (or offscreen drawing surface).      
      Texture tex = pgl.getTexture();
      // Processing Y axis is inverted with respect to OpenGL
      // so we need to invert the texture
      JSpout.WriteTexture(tex.glWidth, tex.glHeight, tex.glName, tex.glTarget, true); // invert
      pgl.endPGL();
  }
  

  void closeSender() {
    if(JSpout.ReleaseSender())
      print("Sender closed" + "\n");
    else
      print("No sender to close" + "\n");
  } 

  //
  // Initialize a Receiver 
  //
  // For texture sharing, the name provided 
  // is searched in the list of senders and 
  // used if it is there. If not, the receiver will 
  // connect to the active sender selected by the user
  // or, if no sender has been selected, this will be
  // the first in the list if any are running.
  //
  void initReceiver(String name, PImage img) {
    
    // Image size values passed in are modified and passed back
    // as the size of the sender that the receiver connects to.
    // Then the screen has to be reset. The same happens when 
    // receiving a texture if the sender or image size changes.
    dim[0] = img.width;
    dim[1] = img.height;
    
    // Already initialized ?
    if(memorymode > 0) {
      print("Receiver already initialized - teture sharing mode\n");
      return;
    }

    // Try Texture share (mode 0)
    // Returns 0 or 1 depending on capabilities or -1 on failure
    memorymode = JSpout.InitReceiver(name, dim, 0);
    
    // FAILURE
    if(memorymode == -1) {
      print("No sender running - start one and try again.\n");
      return;
    }
    else if(memorymode == 0)
      print("Receiver initialized texture sharing\n");
    else if(memorymode == 1)
      print("Receiver texture sharing not supported - using memory sharing\n");
      
    
    // Texture sharing succeeded and there was a sender running
    String newname = JSpout.GetSenderName();
    print("Receiver found sender : " + newname + " " + dim[0] + "x" + dim[1] + "\n");
    // dim will be returned with ths size of the sender it connected to
    
    // Reset the screen size to the connected sender size if necessary
    if(dim[0] != img.width || dim[1] != img.height && dim[0] > 0 && dim[1] > 0) {
      // reset the image size to that of the sender
      img.resize(dim[0], dim[1]);
      // Reset the frame size - include borders and caption
      Insets insets = frame.getInsets();
      frame.setSize(dim[0] + (insets.left + insets.right), dim[1] + (insets.top + insets.bottom));            
    }
 
    img.updatePixels();
    // All done
    
  } // end Receiver initialization
  
  
   
  PImage receiveTexture(PImage img) {
  
    boolean bRet = true;
    
    // Quit if no server running
    if(memorymode < 0) return img;
    
    // Dimensions are sent as well as returned
    dim[0] = img.width;
    dim[1] = img.height;
    
    img.loadPixels();
    if(JSpout.ReadTexture(dim, img.pixels)) {
      // If the sender read was OK, test the image
      // size returned and resize if it is different
      // Otherwise update the image for return
      if(dim[0] != img.width || dim[1] != img.height && dim[0] > 0 && dim[1] > 0) {
         img.resize(dim[0], dim[1]);
         // Include borders and caption
         Insets insets = frame.getInsets();
         frame.setSize(dim[0] + (insets.left + insets.right), dim[1] + (insets.top + insets.bottom));            
      }
      else {
          img.updatePixels();
      }
    }
    return img;
  }



  void closeReceiver() {
    if(JSpout.ReleaseReceiver())
      print("Receiver closed" + "\n");
    else
      print("No receiver to close" + "\n");
  } 

} // end class Spout


