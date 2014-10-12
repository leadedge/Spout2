//
//          Spout Sender
//
//      Demonstrates drawing onto the Processing
//      screen and sending out as a shared texture
//      to a Spout receiver.
//
//      Based on a Processing example sketch by by Dave Bollinger
//      http://processing.org/examples/texturecube.html
//
//      See Spout.pde for function details
//

// DECLARE A SPOUT OBJECT HERE
Spout spout;

PImage tex;

void setup() {

  // size(640, 360, removeFrameBorder(P3D));
  size(640, 360, P3D);
  
  tex = loadImage("SpoutLogoMarble3.bmp");
  textureMode(NORMAL);
  fill(255);
  stroke(color(44,48,32));
  
  // CREATE A NEW SPOUT OBJECT HERE
  spout = new Spout();

  // INITIALIZE A SPOUT SENDER HERE
  spout.initSender("Spout Processing", width, height);
  
} 

void draw()  { 

    // Draw something
    background(0, 90, 100);
    noStroke();
    translate(width/2.0, height/2.0, -100);
    rotateX(frameCount * 0.01);
    rotateY(frameCount * 0.01);      
    scale(120);
    TexturedCube(tex);
    
    // SEND A SHARED TEXTURE HERE
    spout.sendTexture();
  
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
  super.exit();
} 


