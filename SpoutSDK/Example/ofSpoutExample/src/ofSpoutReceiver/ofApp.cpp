/*

	Spout OpenFrameworks Receiver example

    Visual Studio using the Spout SDK

    Search for SPOUT for additions to a typical Openframeworks application

	Copyright (C) 2017 Lynn Jarvis.

	03.08.15 - Created project
	10.01.17 - Update for Spout 2.006

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

	ofSetWindowTitle("OF Spout Receiver"); // Set the window title to show that it is a Spout Receiver
	bInitialized  = false; // Spout receiver initialization
	SenderName[0] = 0; // the name will be filled when the receiver connects to a sender

	// Allocate a texture for shared texture transfers
	// An openFrameWorks texture is used so that it can be drawn.
	g_Width  = ofGetWidth();
	g_Height = ofGetHeight();
	myTexture.allocate(g_Width, g_Height, GL_RGBA);

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	char str[256];
	ofSetColor(255);
	unsigned int width, height;

	// ====== SPOUT =====
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
		// Create the receiver and specify true to attempt to connect to the active sender
		if(spoutreceiver.CreateReceiver(SenderName, width, height, true)) {
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
			sprintf(str, "No sender detected");
			ofDrawBitmapString(str, 20, 20);
		}
	} // end initialization

	// The receiver has initialized so OK to draw
	if(bInitialized) {

		// Save current global width and height - they will be changed
		// by ReceiveTexture if the sender changes dimensions
		width  = g_Width;
		height = g_Height;

		// Try to receive into the local the texture at the current size
		if(spoutreceiver.ReceiveTexture(SenderName, width, height, 
			myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {

			//	If the width and height are changed, the local texture has to be resized.
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

			// Otherwise draw the texture
			myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

			// Show what it is receiving
			sprintf(str, "Receiving from : [%s]", SenderName);
			ofDrawBitmapString(str, 20, 20);
			sprintf(str, "RH click select sender");
			ofDrawBitmapString(str, 15, ofGetHeight()-20);
		}
		else {
			// A texture read failure might happen if the sender
			// is closed. Release the receiver and start again.
			spoutreceiver.ReleaseReceiver();
			bInitialized = false;
			return;
		}
	}

	// Show fps
	sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
	ofSetColor(255);
	ofDrawBitmapString(str, ofGetWidth()-120, 20);
    // ===================

}

//--------------------------------------------------------------
void ofApp::exit() {

    // ====== SPOUT =====
	if(bInitialized) 
		spoutreceiver.ReleaseReceiver(); // Release the receiver

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	// ====== SPOUT =====
	if(button == 2) { // rh button
		// Open the sender selection panel
		// Spout must have been installed
		spoutreceiver.SelectSenderPanel();
	}
}

