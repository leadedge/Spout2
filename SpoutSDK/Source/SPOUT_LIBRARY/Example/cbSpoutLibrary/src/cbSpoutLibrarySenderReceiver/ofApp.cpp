/*

	Spout OpenFrameworks Sender Receiver example
	using the SpoutLibrary C-compatible dll

    Include SpoutLibrary.h in the "src" source files folder
    Include SpoutLibrary.lib where the linker can find it
    Include SpoutLibrary.dll in the "bin" executable folder

	Copyright (C) 2016 Lynn Jarvis.

	=========================================================================
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================
*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0, 0, 0);

	// Set the window title to show that it is a Spout Receiver
	ofSetWindowTitle("Spout Sender Receiver");

    // ====== SPOUT =====
    spoutsender = GetSpout(); // Create an instance of the Spout library
    spoutreceiver = GetSpout(); // Create another instance for the second receiver
  	if(!spoutsender || !spoutreceiver) {
        MessageBoxA(NULL, "Load Spout library failed", "Spout Sender Receiver", MB_ICONERROR);
        exit();
  	}
	bSenderInitialized	= false;						// Spout sender initialization
	bReceverInitialized = false;						// Spout receiver initialization
	bReceived			= false;						// Nothing received yet
	bMemoryShare		= false;						// Memoryshare mode not detected yet

	strcpy(senderName, "Spout Sender/Receiver");		// Set the sender name
	strcpy(receiverName, "Spout Sender/Receiver");	// Sender name for the receiver to try to connect to

	g_Width  = 640;										// set starting global width and height for the sender
	g_Height = 360;
	InitGLtexture(senderTexture, g_Width, g_Height);	// Create an OpenGL texture for data transfers
	senderImage.loadImage("SpoutBox1.png");				// Load a texture image for the sender demo
	senderImage.mirror(false, true);					// correct orientation

	receiverWidth  = 256;								// Set  the receiver size to something
	receiverHeight = 256;								// It will be adjused when it connects to a sender
	InitGLtexture(senderTexture, g_Width, g_Height);
	receiverImage.allocate(g_Width, g_Height, OF_IMAGE_COLOR_ALPHA);

	ofSetWindowTitle("Spout Sender/Receiver");			// set the title bar
	ofSetWindowShape(g_Width, g_Height);				// Set the window size

    // 3D drawing setup for a sender
	glEnable(GL_DEPTH_TEST);							// enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	ofDisableArbTex();									// needed for textures to work
	ofEnableNormalizedTexCoords();						// explicitly normalize tex coords for ofBox
	ofBackground(10, 100, 140);

} // end setup

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	char str[256];
	unsigned int width, height;
	ofSetColor(255);

	// A render window must be available for Spout initialization and might not
	// be available in "update", so do it now when there is definitely a render window.

	//
	// INITIALIZE A SENDER
	//
	if(!bSenderInitialized) {

		// Optionally set for DirectX 9 instead of default DirectX 11 functions
		// spoutsender->SetDX9(true);

		// Create the sender
		bSenderInitialized = spoutsender->CreateSender(senderName, g_Width, g_Height);

		// Test for textureshare compatibility - only used for screen display
		bMemoryShare = spoutsender->GetMemoryShareMode();

	}
	// Sender initialization will only fail if something is wrong


	//
	// INITIALIZE A RECEIVER
	//
	// The receiver will attempt to connect to the name it is sent.
	// Alternatively set the optional bUseActive flag to attempt to connect to the active sender.
	// If the sender name is not initialized it will also attempt to find the active sender
	// If the receiver does not find any senders the initialization will fail
	// and "CreateReceiver" can be called repeatedly until a sender is found.
	// "CreateReceiver" will update the passed name, and dimensions.
	if(!bReceverInitialized) {

		if(spoutreceiver->CreateReceiver(receiverName, width, height, true)) {
			// Is the size of the detected sender different ?
			if(width != receiverWidth || height != receiverWidth ) {
				// The sender dimensions have changed so update the global width and height
				receiverWidth  = width;
				receiverWidth = height;
				// Update the local texture to receive the new dimensions
				receiverImage.resize(receiverWidth, receiverHeight);
				bReceverInitialized = true;
			}
		}
	}

	// If the receiver initialized, get the texture of it's sender
	// and update the local texture and dimensions if the sender size changes
	if(bReceverInitialized) {
		// Save current global width and height - they will be changed
		// by ReceiveTexture if the sender changes dimensions
		width  = receiverWidth;
		height = receiverHeight;
		// Try to receive the texture at the current size
		if(spoutreceiver->ReceiveTexture(receiverName,
										  width, height,
										  receiverImage.getTextureReference().getTextureData().textureID,
										  receiverImage.getTextureReference().getTextureData().textureTarget)) {
			//	Width and height are changed for sender change - the local texture then has to be resized.
			if(width != receiverWidth || height != receiverHeight ) {
				// The sender dimensions have changed so update the global width and height
				receiverWidth  = width;
				receiverHeight = height;
				// Update the local texture to receive the new dimensions
				receiverImage.resize(receiverWidth, receiverHeight);
				// For this application, the received texture is used
				// to render the cube, so nothing more to be done
			}
			bReceived = true;
		} // received texture OK
	} // receiver was initialized


	// Specific to this application, if no other sender is running
	// it will have received from itself, so reset the bReceived flag
	if(strcmp(receiverName, senderName) == 0) bReceived = false;

	// - - - - - - - - - - - - - - - - - - - - - - - - -
	// Draw a 3D graphics demo - this could be anything
	// - - - - - - - - - - - - - - - - - - - - - - - - -
	ofPushMatrix();
	glTranslatef((float)ofGetWidth()/2.0, (float)ofGetHeight()/2.0, 0); // centre
	ofRotateY((float)ofGetFrameNum()/2.0); // rotate - must be float
	ofRotateX((float)ofGetFrameNum()/2.0);
	if(bReceived) receiverImage.getTextureReference().bind(); // bind our texture
	else		  senderImage.getTextureReference().bind();
	ofBox(0.4*(float)ofGetHeight()); // Draw the graphic
	if(bReceived) receiverImage.getTextureReference().unbind();
	else		  senderImage.getTextureReference().unbind();
	ofPopMatrix();
	// - - - - - - - - - - - - - - - - - - - - - - - - -

	// If the sender initialized, grab the screen into the local texture and send it out
	if(bSenderInitialized) {

		// Grab the screen
		glBindTexture(GL_TEXTURE_2D, senderTexture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Send the texture out
		spoutsender->SendTexture(senderTexture, GL_TEXTURE_2D, g_Width, g_Height);

		// Show what it is sending
		ofSetColor(255);
		if(!bMemoryShare) sprintf(str, "Sending as : [%s]", senderName);
		else sprintf(str, "Memoryshare - sending as : [%s]", senderName);
		ofDrawBitmapString(str, 20, 20);

		sprintf(str, "RH click select a sender");
		ofDrawBitmapString(str, 15, ofGetHeight()-20);


	}

	// Show fps
	ofSetColor(255);
	sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
	ofDrawBitmapString(str, ofGetWidth()-120, 20);

}


//--------------------------------------------------------------
void ofApp::exit() {

    if(spoutsender) {
        spoutsender->ReleaseSender(); // Release the sender
        spoutsender->Release(); // Release the Spout SDK library instance
    }
    if(spoutreceiver) {
        spoutreceiver->ReleaseReceiver(); // Release the receiver
        spoutreceiver->Release(); // Release the Spout SDK library instance
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	if(button == 2) {
         spoutreceiver->SelectSenderPanel();
	}
}

// ====== SPOUT =====
bool ofApp::InitGLtexture(GLuint &texID, unsigned int width, unsigned int height)
{
    if(texID != 0) glDeleteTextures(1, &texID);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
