/*

	Spout OpenFrameworks Receiver Sync example

    Example of synchronizing a receiver with a sender

	Synchronization is necessary if the sending and receiving applications 
	require	frame accuracy and missed or duplicated frames are not acceptable.

	Copyright (C) 2023 Lynn Jarvis.

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

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

	ofSetWindowTitle("Receiver Sync Example");
	ofBackground(0, 0, 0);

	// Optional logs
	// EnableSpoutLog();
	// Optional console for windowed application (see main.cpp)
	// OpenSpoutConsole();

	// Allocate an RGBA texture to receive from the sender
	// It is resized later to match the sender - see Update()
	myTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	
} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	//
	// Receiver waits on the sender
	//
	// If the receiver cycles faster than the sender, there will be duplicated frames.
	// BEFORE processing, the receiver can wait until the sender signals
	// that it has sent a new frame. Use a timeout greater than the expected delay. 
	//
	receiver.WaitFrameSync(receiver.GetSenderName(), 67);
	//
	// To show the effect of sync functions, reduce the sender frame rate.
	// (See the sender sync example).
	// The receiver will synchronize with the sender frame rate. 
	//
	// The on-screen display will show the actual received frame rate.
	// Build without WaitFrameSync or with no timeout and observe the difference.

	// Receive texture
	if (receiver.ReceiveTexture(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {
		if (receiver.IsUpdated()) {
			myTexture.allocate(receiver.GetSenderWidth(), receiver.GetSenderHeight(), GL_RGBA);
			return;
		}
		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
	}

	// On-screen display
	showInfo();

	//
	// Sender waits on the receiver
	//
	// If the receiver cycles slowerthan the sender, there will be missed frames.
	// AFTER processing, the receiver can signal when it is ready to receive the next frame.
	//
	// receiver.SetFrameSync(receiver.GetSenderName());
	//
	// To demonstrate the effect of sync functions, reduce the receiver frame rate.
	// The sender will synchronize with the receiver frame rate.
	// receiver.HoldFps(30);

}


//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255, 255, 255);

	if(receiver.IsConnected()) {
		str = receiver.GetSenderName(); // sender name
		str += " (";
		str += to_string(receiver.GetSenderWidth()); // width
		str += "x";
		str += to_string(receiver.GetSenderHeight()); // height 
		str += ") ";
		if (receiver.GetSenderFrame() > 0) {
			str += " : fps ";
			str += to_string((int)(roundf(ofGetFrameRate()))); // the actual received frame rate
			str += " : frame ";
			str += to_string(receiver.GetSenderFrame()); // frame since the sender started
		}
		ofDrawBitmapString(str, 10, 20);
	}
	else {
		str = "No sender detected";
		ofDrawBitmapString(str, 10, 20);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	receiver.ReleaseReceiver();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if(button == 2) { // rh button
		// Open the sender selection panel
		// Spout must have been installed
		receiver.SelectSender();
	}
}

