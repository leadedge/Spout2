/*

	Spout OpenFrameworks Webcam Sender example
    With modifications for SpoutCam

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

	Copyright (C) 2022-2023 Lynn Jarvis.

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

	ofBackground(0);

	// Option - show Spout logs
	// EnableSpoutLog();
	// Or a console to show webcam details
	OpenSpoutConsole();

	camsendername = "Spout Webcam Sender"; // Set the sender name
	ofSetWindowTitle(camsendername); // show it on the title bar

	// Get the webcam list into a vector so we can
	// identify by name and detect SpoutCam later
	camdevices = vidGrabber.listDevices();
	// printf("Select a webcam by it's index (0-%d)\n\n", (int)camdevices.size() - 1);

	// Use the default webcam (0) or change as required
	camindex = 0;
	vidGrabber.setDeviceID(camindex);
	// Try to set this frame rate
	// (For SpoutCam use SpoutCamSettings instead)
	vidGrabber.setDesiredFrameRate(30);
	vidGrabber.setup(640, 480); // try to grab at this size. 
	if (vidGrabber.isInitialized())
		printf("Initialized webcam (%d x %d)\n", (int)vidGrabber.getWidth(), (int)vidGrabber.getHeight());
	else
		printf("Select a webcam by it's index (0-%d)\n\n", (int)camdevices.size() - 1);

	// Give the sender the same name as the window title
	// If none is specified, the executable name is used
	camsender.SetSenderName(camsendername.c_str());

}


//--------------------------------------------------------------
void ofApp::update() {

	if (!vidGrabber.isInitialized()) {
		return;
	}

	vidGrabber.update();

	// SpoutCam is a receiver and connects to the active sender when it starts.
	// To avoid feedback from this application, wait until another sender is running.
	bSendCam = true;
	if (camdevices[camindex].deviceName == "SpoutCam") {
		if(camsender.GetSenderCount() == 0) {
			// Nothing running - do not send
			bSendCam = false;
		}
		else if (camsender.GetSenderCount() == 1) {
			// Only one sender left
			if (camsender.IsInitialized()) {
				camsender.ReleaseSender();
				bSendCam = false;
			}
			// Give Spoutcam a frame time to refresh buffers
			Sleep(ofGetFrameRate());
		}
	}

	if (bSendCam) {
		// If you send the webcam texture when the frame is new
		// and Frame count is enabled in SpoutSettings,
		// receivers will detect the webcam fps.
		if (vidGrabber.isFrameNew()) {
			// Send the webcam texture
			// A sender is created and updated for size changes
			camsender.SendTexture(vidGrabber.getTexture().getTextureData().textureID,
				vidGrabber.getTexture().getTextureData().textureTarget,
				(unsigned int)vidGrabber.getWidth(),
				(unsigned int)vidGrabber.getHeight(), false);
		}
	}

}


//--------------------------------------------------------------
void ofApp::draw() {

	std::string str;
	ofSetColor(255);

	if (vidGrabber.isInitialized())
		vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Show what it's sending
	if (camsender.IsInitialized()) {
		str = "Sending as : ";
		str += camsender.GetName(); str += " (";
		str += ofToString(camsender.GetWidth()); str += "x";
		str += ofToString(camsender.GetHeight()); str += ")";
		// Show sender fps and framecount if they are selected in SpoutSettings
		if (camsender.GetFrame() > 0) {
			str += " fps: ";
			str += ofToString((int)roundf(camsender.GetFps())); str += " frame : ";
			str += ofToString(camsender.GetFrame());
		}
		else {
			// Show Openframeworks fps
			str += "fps : " + ofToString((int)roundf(ofGetFrameRate()));
		}
	}
	else {
		str += "Select a webcam by it's index";
	}
	ofDrawBitmapString(str, 20, 30);

	// Show the webcam list for selection
	int ypos = 50;
	for (int i=0; i<(int)camdevices.size(); i++) {
		str = "("; str+= ofToString(i); str += ") ";
		str += camdevices[i].deviceName;
		ofDrawBitmapString(str, 40, ypos);
		ypos += 15;
	}
	str = "Press 0 to ";
	str += ofToString(camdevices.size()-1);
	str += " to select a webcam";
	ofDrawBitmapString(str, 40, ypos);


}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	camsender.ReleaseSender(); 
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	int i = key - 48; // Decimal number
	if (key == 32) {
		camdevices = vidGrabber.listDevices();
		printf("Select a webcam by it's index (0-%d)\n", (int)camdevices.size() - 1);
	}
	else if (i >= 0 && i < (int)camdevices.size()) {
		// Change if the user selected a different webcam
		if (i != camindex) {
			// Release our sender before changing webcams or a
			// switch to SpoutCam will pick up this application
			camsender.ReleaseSender();
			camindex = i;
			vidGrabber.close();
			vidGrabber.setDeviceID(camindex);
			vidGrabber.setDesiredFrameRate(30);
			vidGrabber.setUseTexture(true);
			if (vidGrabber.setup(640, 480)) {
				ofSetWindowShape(vidGrabber.getWidth(), vidGrabber.getHeight());
				cout << "Initialized webcam [" << camdevices[camindex].deviceName << "] (" << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;
			}
			else {
				printf("Webcam setup error. Try a different one.\n");
			}
		}
	}

}

