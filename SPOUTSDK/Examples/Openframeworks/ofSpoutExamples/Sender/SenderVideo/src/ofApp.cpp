/*

	Spout OpenFrameworks Video Sender example

	A simple example of a sender

	Copyright (C) 2017-2025 Lynn Jarvis.

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

	// OpenSpoutConsole(); // for debugging
	ofBackground(255,255,255);
	ofSetVerticalSync(true);

	strcpy_s(sendername, 256, "OF Spout Video Sender");	// Set the sender name
	ofSetWindowTitle(sendername);					// show it on the title bar

    // Video setup
	if (!fingerMovie.load("fingers.mp4")) {
		SpoutMessageBox("Example movie file \"fingers.mp4\" not found\n");
		ofExit();
	}


	// Set the window height to match the movie aspect ratio.
	// The movie texture size is not affected.
	// The receiver is responsible for setting it's own window shape.
	fingerMovie.update();
	float height = (float)ofGetWidth()*fingerMovie.getHeight()/fingerMovie.getWidth();
	ofSetWindowShape(ofGetWidth(), (int)height);

	// Centre the window on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	fingerMovie.play();


}


//--------------------------------------------------------------
void ofApp::update() {
    fingerMovie.update();

}


//--------------------------------------------------------------
void ofApp::draw() {

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
	
	char str[256]; // For formatting

	// Show what it is sending and fps 
	ofSetColor(255);
	sprintf_s(str, 256, "Sending as : [%s] (fps: %3d)", sendername, (int)ofGetFrameRate());
	ofDrawBitmapString(str, 20, 20);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	spoutsender.ReleaseSender(); 
}

