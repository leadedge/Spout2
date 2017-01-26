/*

	Spout OpenFrameworks Video Sender example

    Search for SPOUT for additions to a typical Openframeworks application

	Copyright (C) 2017 Lynn Jarvis.

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

	ofBackground(255,255,255);
	ofSetVerticalSync(true);

    // SPOUT
	bInitialized = false;						// Spout sender initialization
	strcpy(sendername, "OF Spout Video Sender");	// Set the sender name
	ofSetWindowTitle(sendername);					// show it on the title bar

    // Video setup for a sender
	fingerMovie.loadMovie("movies/fingers.mov");
	fingerMovie.play();

}


//--------------------------------------------------------------
void ofApp::update() {
    fingerMovie.update();
}


//--------------------------------------------------------------
void ofApp::draw() {

	char str[256];
	ofSetHexColor(0xFFFFFF);

	fingerMovie.draw(0, 0, ofGetWidth(), ofGetHeight());

	// SPOUT
	if(!bInitialized) {
		// Create a Spout sender the same size as the video
		bInitialized = spoutsender.CreateSender(sendername, fingerMovie.getWidth(), fingerMovie.getHeight()); 
	}
 
	if(bInitialized) {
		// Send the video texture out for all receivers to use
		spoutsender.SendTexture(fingerMovie.getTextureReference().getTextureData().textureID,
				fingerMovie.getTextureReference().getTextureData().textureTarget,
				fingerMovie.getWidth(), fingerMovie.getHeight(), false);

		// Show what it is sending
		ofSetColor(255);
		sprintf(str, "Sending as : [%s]", sendername);
		ofDrawBitmapString(str, 20, 20);

		// Show fps
		sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
		ofDrawBitmapString(str, 20, 40);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	// SPOUT
	// Release the sender on exit
	spoutsender.ReleaseSender(); 
}

