/*

	Spout OpenFrameworks SpoutLibrary dll dynamic Load example

	The SpoutLibrary.lib file does not need to be linked during build

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2017

	Copyright (C) 2015-2022 Lynn Jarvis.

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
    along with this program. If not, see <http://www.gnu.org/licenses/>.
	=========================================================================
*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(10, 100, 140);
	
	// Try to load SpoutLibrary.dll
	// The dll file must be in the same folder as the executable.
	// The dll type (Win32 or x64) must match that of the executable.
	sender = spoutloader.LoadSpoutLibrary();
	if (!sender) {
		MessageBoxA(NULL, "SpoutLibrary dll not found", "Error", MB_OK);
		ofExit();
	}

	// Now use the library functions as usual
	sender->EnableSpoutLog();
	// sender->SetSpoutLogLevel(SPOUT_LOG_WARNING); // For only warning messages

	// Give the sender a name
	sender->SetSenderName("OF Spout Dynamic Load Sender");

	// Also show it on the title bar
	ofSetWindowTitle("OF Spout Dynamic Load Sender");
	
	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// Framerate is default 60fps for driver vsync enabled

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	// Draw graphics
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2.0, ofGetHeight() / 2.0, 0); // centre the cube
	ofRotateYDeg(rotX); // rotate the cube
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4*ofGetHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6;
	rotY += 0.6;
	
	// Send the default framebuffer (SendFbo handles re-sizing)
	sender->SendFbo(0, ofGetWidth(), ofGetHeight());

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender->GetName(); str += " (";
	str += ofToString(sender->GetWidth()); str += "x";
	str += ofToString(sender->GetHeight()); str += ")";
	if (sender->GetFrame() > 0) {
		str += " fps ";
		str += ofToString((int)roundf(sender->GetFps()));
		str += " : frame  ";
		str += ofToString(sender->GetFrame());
	}
	else {
		// Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 10, 20);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	sender->ReleaseSender();
	// Release SpoutLibrary
	sender->Release();
	// Free the loaded dll
	spoutloader.FreeSpoutLibrary();
}
