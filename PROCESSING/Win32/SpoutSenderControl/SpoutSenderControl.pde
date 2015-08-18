//
//          Spout Sender Control
//
//      Demonstrates using "SpoutControls"
//      for controlling a Spout Sender
//
//      After it is running start a Spout Controller :
//
//      "SpoutController.dll" freeframe plugin running in a host
//        or
//      "spoutController.exe" controller program.
//
//      Based on a Processing example sketch by by Dave Bollinger
//      http://processing.org/examples/texturecube.html
//
//      See Spout.pde for function details
//

// DECLARE A SPOUT OBJECT HERE
Spout spout;

PImage tex;

String sendername;

String[] controlName;
int[] controlType;
float[] controlValue;
String[] controlText;

boolean bRotate = true;
float RotationSpeed = 1.0;
float RotX = 0;
float RotY = 0;
String UserText = "";

PFont f;


void setup() {

  size(640, 360, P3D);
  
  tex = loadImage("SpoutLogoMarble3.bmp");
  textureMode(NORMAL);
  
  // CREATE A NEW SPOUT OBJECT HERE
  spout = new Spout();

  // INITIALIZE A SPOUT SENDER HERE
  sendername = "Spout Processing";
  spout.initSender(sendername, width, height);
  
  //
  // SPOUTCONTROLS
  //
  
  // Create control arrays
  controlName = new String[20];
  controlType = new int[20];
  controlValue = new float[20];
  controlText = new String[20];
  
  // Create controls
  spout.CreateSpoutControl("User text", "text");
  spout.CreateSpoutControl("Rotate", "bool", 1);
  spout.CreateSpoutControl("Speed", "float", 0, 4, 1);
  
  // Open the controls for the controller using the sender name
  spout.OpenSpoutControls(sendername);
  
  
  f = loadFont( "Verdana-48.vlw" );   
  
} 

void draw()  { 
  
    // SPOUTCONTROLS -  Check for updated controls
    int nControls = spout.CheckSpoutControls(controlName, controlType, controlValue, controlText);
    // print("nControls = " + nControls + "\n");
    if(nControls > 0) {
      for(int i = 0; i < nControls; i++) {
         // print("(" + i + ") : [" + controlName[i] + "] : Type [" + controlType[i] + "] : Value [" + controlValue[i] + "] : Text [" + controlText[i] + "]" + "\n");
         if(controlName[i].equals("User text")) {
          if(controlText[i] != null && !controlText[i].isEmpty())
            UserText = controlText[i];
         }
         if(controlName[i].equals("Speed"))
             RotationSpeed = controlValue[i];
         if(controlName[i].equals("Rotate"))
             bRotate = (boolean)(controlValue[i] == 1);
      }
    }

    // Draw something
    background(0, 90, 100);
    noStroke();
    pushMatrix();
    translate(width/2.0, height/2.0, -100);
    rotateX(RotX);
    rotateY(RotY);    scale(120);
    TexturedCube(tex);
    popMatrix();
    
    // Text overlay
    textFont(f, 32);
    fill(255);
    // Must test both for null and empty here
    if(UserText != null && !UserText.isEmpty()) {
      text (UserText, 10, 40);
    }
        
    // SEND A SHARED TEXTURE HERE
    spout.sendTexture();
  
    if(bRotate) {
      RotY += RotationSpeed * 0.01;
      RotX += RotationSpeed * 0.01;
    }
    
}


void TexturedCube(PImage tex) {
  
  beginShape(QUADS);
  texture(tex);

  // +Z "front" face
  vertex(-1, -1,  1, 0, 0);
  vertex( 1, -1,  1, 1, 0);
  vertex( 1,  1,  1, 1, 1);
  vertex(-1,  1,  1, 0, 1);

  // -Z "back" face
  vertex( 1, -1, -1, 0, 0);
  vertex(-1, -1, -1, 1, 0);
  vertex(-1,  1, -1, 1, 1);
  vertex( 1,  1, -1, 0, 1);

  // +Y "bottom" face
  vertex(-1,  1,  1, 0, 0);
  vertex( 1,  1,  1, 1, 0);
  vertex( 1,  1, -1, 1, 1);
  vertex(-1,  1, -1, 0, 1);

  // -Y "top" face
  vertex(-1, -1, -1, 0, 0);
  vertex( 1, -1, -1, 1, 0);
  vertex( 1, -1,  1, 1, 1);
  vertex(-1, -1,  1, 0, 1);

  // +X "right" face
  vertex( 1, -1,  1, 0, 0);
  vertex( 1, -1, -1, 1, 0);
  vertex( 1,  1, -1, 1, 1);
  vertex( 1,  1,  1, 0, 1);

  // -X "left" face
  vertex(-1, -1, -1, 0, 0);
  vertex(-1, -1,  1, 1, 0);
  vertex(-1,  1,  1, 1, 1);
  vertex(-1,  1, -1, 0, 1);

  endShape();
}


// over-ride exit to release sharing
void exit() {
  
  // CLOSE THE SPOUT SENDER HERE
  spout.closeSender();
  
  // SPOUTCONTROLS
  spout.CloseSpoutControls();

  super.exit();
} 


