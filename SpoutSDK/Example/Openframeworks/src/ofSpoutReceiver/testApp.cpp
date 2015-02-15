/*

	Spout Openframeworks Receiver Example

	Copyright (C) 2014 Lynn Jarvis.

	02.01.15 - SDK recompile
	04.02.15 - corrected memoryshare detection after createreceiver
			 - SDK recompile for default DX9 (see SpoutGLDXinterop.h)
	04.02.15 - SDK recompile for default DX11 and auto compatibility detection (see SpoutGLDXinterop.cpp)


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

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofBackground(10, 100, 140);
    ofSetVerticalSync(true);				// lock to monitor
	
	spoutreceiver	= new SpoutReceiver;	// Create a new Spout receiver

	bInitialized	= false;				// Spout receiver initialization
	g_Width			= 320;					// set global width and height to something
	g_Height		= 240;					// they need to be reset when the receiver connects to a sender
	SenderName[0]	= NULL;					// the name will be filled when the receiver connects to a sender
	
	ofSetWindowTitle("OF Spout SDK Receiver");
	ofSetWindowShape(g_Width, g_Height);			// Set the initial window size

	// Allocate a texture for shared texture transfers
	// An openFrameWorks texture is used so that it can be drawn.
	myTexture.allocate(g_Width, g_Height, GL_RGBA);	

} // end setup


//--------------------------------------------------------------
void testApp::update () {

}

//--------------------------------------------------------------
void testApp::draw() {

	char str[256];
	ofSetColor(255);
	unsigned int width, height;
	char tempname[256]; // temp name
	
	// A render window must be available for Spout initialization
	// and might not be available in "update", so do it now 
	// when there is definitely a render window.
	//
	// INITIALIZE A RECEIVER
	//
	// The receiver will attempt to connect to the name it is sent.
	// Alternatively set the optional bUseActive flag to attempt to connect to the active sender. 
	// If the sender name is not initialized it will attempt to find the active sender
	// If the receiver does not find any senders the initialization will fail
	// and "CreateReceiver" can be called repeatedly until a sender is found.
	// "CreateReceiver" will update the passed name, and dimensions.
	if(!bInitialized) {
		// Specify true to attempt to connect to the active sender
		if(spoutreceiver->CreateReceiver(SenderName, width, height, true)) {
			// Detect texture share compatibility for status below
			bMemoryShare = spoutreceiver->GetMemoryShareMode();
			// Is the size of the detected sender different ?
			if(width != g_Width || height != g_Height ) {
				// The sender dimensions have changed so update the global width and height
				g_Width  = width;
				g_Height = height;
				// Update the local texture to receive the new dimensions
				myTexture.allocate(g_Width, g_Height, GL_RGBA);
				// reset render window
				ofSetWindowShape(g_Width, g_Height);
			}
			bInitialized = true;
			return; // quit for next round
		} // receiver was initialized
		else {
			sprintf_s(str, "No sender detected");
			ofDrawBitmapString(str, 20, 20);
		}
	} // already initialized

	// The receiver has initialized so OK to draw
	if(bInitialized) {
		
		// Save current global width and height - they will be changed
		// by receivetexture if the sender changes dimensions
		width  = g_Width;
		height = g_Height;
		
		// Try to receive into the local the texture at the current size 
		// NOTE: If a host calls ReceiveTexture with a framebuffer object bound,
		// include the FBO id in the ReceiveTexture call so that the binding is restored
		// afterwards because Spout makes use of its own FBO for intermediate rendering.		
		if(spoutreceiver->ReceiveTexture(SenderName, width, height, myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {

			//	width and height are changed. The local texture then has to be resized.
			if(width != g_Width || height != g_Height ) {
				// Update the global width and height
				g_Width  = width;
				g_Height = height;
				// Update the local texture to receive the new dimensions
				myTexture.allocate(g_Width, g_Height, GL_RGBA);
				// reset render window
				ofSetWindowShape(g_Width, g_Height);
				return; // quit for next round
			}

			// Otherwise draw the texture and fill the screen
			myTexture.draw(0, 0, ofGetWidth(), ofGetHeight()); 

			// Show what it is receiving
			if(!bMemoryShare) {
				sprintf_s(str, "Receiving from : [%s]", SenderName);
				ofDrawBitmapString(str, 20, 20);
				sprintf_s(str, "RH click select sender", SenderName);
				ofDrawBitmapString(str, 15, ofGetHeight()-20);
			}
			else {
				sprintf_s(str, "Memoryshare receiver");
				ofDrawBitmapString(str, 20, 20);
			}
		}
		else {
			// A texture read failure might happen if the sender
			// is closed. Release the receiver and start again.
			spoutreceiver->ReleaseReceiver();
			bInitialized = false;
			return;
		}
	}
	
	// Show fps
	sprintf_s(str, "fps: %3.3d", (int)ofGetFrameRate());
	ofSetColor(255); 
	ofDrawBitmapString(str, ofGetWidth()-120, 20);

}


//--------------------------------------------------------------
void testApp::exit() { 

	// can be used for repeated Create and Release of a receiver within the program
	spoutreceiver->ReleaseReceiver(); 
	// Program termination so clean up everything
	delete spoutreceiver;
	spoutreceiver = NULL;

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

	if(button == 2) { // rh button
		// SpoutPanel.exe must be in executable path
		spoutreceiver->SelectSenderPanel(); // DirectX 11 by default
	}

}




