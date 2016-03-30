/*

	Spout OpenFrameworks Sender example - ofApp.cpp

    Visual Studio using the Spout SDK

    Search for SPOUT for additions to a typical Openframeworks application

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

	ofBackground(10, 100, 140);
	ofEnableNormalizedTexCoords();  // explicitly normalize tex coords for ofBox

    // ====== SPOUT =====
	spoutsender = new SpoutSender;			// Create a Spout sender object
	bInitialized	= false;		        // Spout sender initialization
	strcpy(sendername, "OF Spout Sender");	// Set the sender name
	ofSetWindowTitle(sendername);			// show it on the title bar
	// Create an OpenGL texture for data transfers
	sendertexture = 0; // make sure the ID is zero for the first time
	InitGLtexture(sendertexture, ofGetWidth(), ofGetHeight());
	// Set the window icon from resources
	SetClassLong(GetActiveWindow(), GCL_HICON, (LONG)LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1)));
    // ===================

    // 3D drawing setup for a sender
	glEnable(GL_DEPTH_TEST);							// enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	ofDisableArbTex();									// needed for textures to work

	myTextureImage.loadImage("SpoutBox1.png");			// Load a texture image for the demo

	rotX = 0;
	rotY = 0;

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	char str[256];
	ofSetColor(255);

	// ====== SPOUT =====
	// A render window must be available for Spout initialization and might not be
	// available in "update" so do it now when there is definitely a render window.
	if(!bInitialized) {
		// Create the sender
		bInitialized = spoutsender->CreateSender(sendername, ofGetWidth(), ofGetHeight()); 
	}
    // ===================

	// - - - - - - - - - - - - - - - - - - - - - - - - -
	// Draw 3D graphics demo - this could be anything
	ofPushMatrix();
	ofTranslate((float)ofGetWidth()/2.0, (float)ofGetHeight()/2.0, 0);
	ofRotateY(rotX); // rotate - must be float
	ofRotateX(rotY);
	myTextureImage.getTextureReference().bind(); // bind our texture
	ofDrawBox(0.4*(float)ofGetHeight()); // Draw the graphic
	myTextureImage.getTextureReference().unbind(); // bind our texture
	ofPopMatrix();
	rotX+=0.5;
	rotY+=0.5;

	// ====== SPOUT =====
	if(bInitialized) {

        if(ofGetWidth() > 0 && ofGetHeight() > 0) { // protect against user minimize

            // Grab the screen into the local spout texture
            glBindTexture(GL_TEXTURE_2D, sendertexture);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, ofGetWidth(), ofGetHeight());
            glBindTexture(GL_TEXTURE_2D, 0);

            // Send the texture out for all receivers to use
            spoutsender->SendTexture(sendertexture, GL_TEXTURE_2D, ofGetWidth(), ofGetHeight());

            // Show what it is sending
            ofSetColor(255);
            sprintf(str, "Sending as : [%s]", sendername);
            ofDrawBitmapString(str, 20, 20);

            // Show fps
            sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
            ofDrawBitmapString(str, ofGetWidth()-120, 20);

        }
	}
    // ===================


}

//--------------------------------------------------------------
void ofApp::exit() {

    // ====== SPOUT =====
	spoutsender->ReleaseSender(); // Release the sender

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	
	// ====== SPOUT =====
	// Update the sender texture to receive the new dimensions
	// Change of width and height is handled within the SendTexture function
	if(w > 0 && h > 0) // protect against user minimize
        InitGLtexture(sendertexture, w, h);
	
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
