/*

	Spout OpenFrameworks Sender example - testApp.cpp

	Copyright (C) 2014 Lynn Jarvis.

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

	================================================================================================
*/

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofBackground(10, 100, 140);
    ofSetVerticalSync(true);							// lock to monitor
	ofEnableNormalizedTexCoords();						// explicitly normalize tex coords for ofBox

	bInitialized	= false;							// Spout sender initialization
	g_Width			= 640;								// set starting global width and height for the sender
	g_Height		= 480;
	
	strcpy_s(sendername, "OF Spout Sender");			// Set the sender name
	ofSetWindowTitle(sendername);						// show it on the title bar
	ofSetWindowShape(g_Width, g_Height);				// Set the window size
	
    // 3D drawing setup for a sender
	glEnable(GL_DEPTH_TEST);							// enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	ofDisableArbTex();									// needed for textures to work
	
	myTextureImage.loadImage("SpoutBox1.png");			// Load a texture image for the demo
	myTextureImage.mirror(false, true);					// correct orientation

	spoutsender = new SpoutSender;						// Create a new Spout sender
	sendertexture = 0;									// Initially there is no local OpenGL texture ID
	InitGLtexture(sendertexture, g_Width, g_Height);	// Create an OpenGL texture for data transfers


} // end setup


//--------------------------------------------------------------
void testApp::update () {

}

//--------------------------------------------------------------
void testApp::draw() {

	char str[256];
	ofSetColor(255);

	// A render window must be available for Spout initialization
	// and might not be available in "update", so do it now 
	// when there is definitely a render window.

	// INITIALIZE A SENDER
	if(!bInitialized) {
		bInitialized = spoutsender->CreateSender(sendername, g_Width, g_Height);
		// Detect texture share compatibility (optional)
		bMemoryShare = spoutsender->GetMemoryShareMode();
	}
	// Sender initialization will only fail if something is wrong

	// - - - - - - - - - - - - - - - - - - - - - - - - -
	// Draw 3D graphics demo - this could be anything
	// - - - - - - - - - - - - - - - - - - - - - - - - -
	ofPushMatrix();
	glTranslatef((float)ofGetWidth()/2.0, (float)ofGetHeight()/2.0, 0); // centre
	ofRotateY((float)ofGetFrameNum()/2.0); // rotate - must be float
	ofRotateX((float)ofGetFrameNum()/2.0);
	myTextureImage.getTextureReference().bind(); // bind our texture
	ofBox(0.4*(float)ofGetHeight()); // Draw the graphic
	ofPopMatrix();
	// - - - - - - - - - - - - - - - - - - - - - - - - -

	if(bInitialized) {

		// Grab the screen into the local spout texture
		glBindTexture(GL_TEXTURE_2D, sendertexture); 
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		glBindTexture(GL_TEXTURE_2D, 0); 

		//
		// Send the texture out for all receivers to use
		//
		// Notes :
		// (1)	If a host calls SendTexture with a framebuffer object actively bound
		//		it must restore that binding afterwards because Spout makes use of its
		//		own FBO for intermediate rendering.
		// (2)	openGL/DirectX coordinates make our texture come out inverted so the texture
		//		is inverted	when transferring it to the shared texture. You can specify false
		//		to disable this default then the result comes out apparently inverted.
		//
		spoutsender->SendTexture(sendertexture, GL_TEXTURE_2D, g_Width, g_Height);

		// Show what it is sending
		ofSetColor(255); 
		if(!bMemoryShare) sprintf_s(str, "Sending as : [%s]", sendername);
		else sprintf_s(str, "Memoryshare sender");
		ofDrawBitmapString(str, 20, 20);

		// Show fps
		sprintf_s(str, "fps: %3.3d", (int)ofGetFrameRate());
		ofDrawBitmapString(str, ofGetWidth()-120, 20);
	}

}


//--------------------------------------------------------------
void testApp::exit() { 

	// Release the sender - this can be used for repeated 
	// Create and Release of a sender within the program
	spoutsender->ReleaseSender(); 

	// At program termination, clean up everything
	delete spoutsender;
	spoutsender = NULL;

}

void testApp::windowResized(int w, int h)
{

	// Update the global width and height
	g_Width  = ofGetWidth();
	g_Height = ofGetHeight();

	// Update the sender texture to receive the new dimensions
	InitGLtexture(sendertexture, g_Width, g_Height);


}

bool testApp::InitGLtexture(GLuint &texID, unsigned int width, unsigned int height)
{
	// printf("testApp::InitGLtexture %dx%d\n", width, height);

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

