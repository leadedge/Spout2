/*

	Spout OpenFrameworks Webcam Sender example

	Spout 2.007
	OpenFrameworks 10
	Visual Studio 2017

	Copyright (C) 2020 Lynn Jarvis.

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

	sendername = "OF Spout Webcam Sender"; // Set the sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// vidGrabber.setDeviceID(0); // Use the default webcam or change as required
	// Try to set this frame rate
	// (For SpoutCam use SpoutCamSettings instead)
	vidGrabber.setDesiredFrameRate(30);
	vidGrabber.setup(640, 480); // try to grab at this size. 
	cout << "Initialized webcam (" << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;

	// Create the Spout sender with it's name
	// and the size of the the webcam texture
	// which can be different to the setup size
	sender.CreateSender(sendername.c_str(),
		(unsigned int)vidGrabber.getWidth(),
		(unsigned int)vidGrabber.getHeight());

}


//--------------------------------------------------------------
void ofApp::update() {
	vidGrabber.update();
	// If you send the webcam texture if the frame is new
	// and Frame count is enabled in SpoutSettings,
	// receivers will detect the webcam fps.
	if (vidGrabber.isFrameNew()) {
		// Send the webcam texture
		// The sender is updated for size changes
		sender.SendTexture(vidGrabber.getTexture().getTextureData().textureID,
			vidGrabber.getTexture().getTextureData().textureTarget,
			(unsigned int)vidGrabber.getWidth(),
			(unsigned int)vidGrabber.getHeight(), false);
	}
}


//--------------------------------------------------------------
void ofApp::draw() {

	ofSetColor(255);

	vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Show what it is sending
	if (sender.IsInitialized()) {
		std::string str = "Sending as : ";
		str += sendername; str += " (";
		str += ofToString(sender.GetWidth()); str += "x";
		str += ofToString(sender.GetHeight()); str += ")";
		// Show sender fps and framecount if selected in SpoutSettings
		if (sender.GetFrame() > 0) {
			str += " fps: ";
			str += ofToString((int)roundf(sender.GetFps())); str += " frame : ";
			str += ofToString(sender.GetFrame());
		}
		else {
			// Show Openframeworks fps
			str = "fps : " + ofToString((int)roundf(ofGetFrameRate()));
		}
		ofDrawBitmapString(str, 20, 40);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	sender.ReleaseSender(); 
}

