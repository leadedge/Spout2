/*

	Spout OpenFrameworks Video Sender example

	Copyright (C) 2017-2022 Lynn Jarvis.

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

	strcpy_s(sendername, 256, "OF Spout Video Sender");	// Set the sender name
	ofSetWindowTitle(sendername);					// show it on the title bar

    // Video setup
	fingerMovie.load("movies/fingers.mov");
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

	// Send the video texture out for all receivers to use
	// SendTexture manages sender creation and re-sizing
	if (fingerMovie.isFrameNew()) {
		// Receivers will detect the video frame rate (30 fps in this case)
		spoutsender.SendTexture(fingerMovie.getTexture().getTextureData().textureID,
			fingerMovie.getTexture().getTextureData().textureTarget,
			fingerMovie.getWidth(), fingerMovie.getHeight(), false);
	}
	
	// Show what it is sending
	ofSetColor(255);
	sprintf_s(str, 256, "Sending as : [%s]", sendername);
	ofDrawBitmapString(str, 20, 20);

	// Show fps
	sprintf_s(str, 256, "fps: %3.3d", (int)ofGetFrameRate());
	ofDrawBitmapString(str, 20, 40);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	spoutsender.ReleaseSender(); 
}

