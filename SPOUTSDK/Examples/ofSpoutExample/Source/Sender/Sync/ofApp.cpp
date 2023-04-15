/*

	Spout OpenFrameworks Sender Sync example

    Example of synchronizing a sender with a receiver

	Synchronization is necessary if the sending and receiving applications
	require	frame accuracy and missed or duplicated frames are not acceptable.

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

	Copyright (C) 2023 Lynn Jarvis.

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

	// Optional logs
	// EnableSpoutLog();
	// Optional console for windowed application (see main.cpp)
	// OpenSpoutConsole();

 	strcpy_s(sendername, 256, "Sender Sync Example"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// Give the sender a name
	sender.SetSenderName(sendername);
	
	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = rotY = 0.0f;

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	//
	// Sender waits on the receiver
	//
	// If the sender cycle is faster, thereceiver will miss frames.
	// BEFORE processing, the sender can wait until the receiver
	// signals that it is ready to receive a frame.
	// Use a timeout greater than the expected delay. 
	//
	// sender.WaitFrameSync(sender.GetName(), 67);
	//
	// To demonstrate the effect of sync functions, reduce the receiver frame rate.
	// (See the receiver sync example).
	// The sender will synchronize with the receiver frame rate. 
	//
	// The on-screen display will show the actual sender frame rate.
	// Build without WaitFrameSync or with no timeout and observe the difference.

	// Draw 3D graphics into the fbo
	myFbo.begin();
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate(myFbo.getWidth() / 2.0, myFbo.getHeight() / 2.0, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4*myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6;
	rotY += 0.6;
	
	// Send fbo
	sender.SendFbo(myFbo.getId(), ofGetWidth(), ofGetHeight(), false);

	myFbo.end();

	// Draw the result
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// On-screen display
	showInfo();

	//
	// Receiver waits on the sender
	//
	// If the sender cycle is slower, the receiver will duplicate frames.
	// AFTER processing, the sender can signal that a new frame has been produced.
	//
	sender.SetFrameSync(sender.GetName());
	//
	// To demonstrate the effect of sync functions, reduce the sender frame rate.
	// The receiver will synchronize with the sender frame rate.
	sender.HoldFps(30);

}

//--------------------------------------------------------------
void ofApp::showInfo() {

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender.GetName(); str += " (";
	str += ofToString(sender.GetWidth()); str += "x";
	str += ofToString(sender.GetHeight()); str += ")";
	if (sender.GetFrame() > 0) {
		// Sender fps
		str += " fps ";
		str += to_string((int)(roundf(ofGetFrameRate()))); // the sending frame rate
		str += " : frame  ";
		str += ofToString(sender.GetFrame());
	}
	ofDrawBitmapString(str, 10, 20);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender on exit
	sender.ReleaseSender();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	// The sending size matches the window size, update the fbo
	if (w > 0 && h > 0) {
		myFbo.allocate(w, h, GL_RGBA);
	}
}
