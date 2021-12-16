/*

	Spout OpenFrameworks Webcam Sender example

    With modifications for SpoutCam

	Copyright (C) 2022 Lynn Jarvis.

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

	bInitialized = false; // Spout sender initialization
	strcpy(sendername, "OF Spout Webcam Sender"); // Set the sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// Get the webcam list into a vector so we can
	// identify by name and detect SpoutCam later
	camdevices = vidGrabber.listDevices();

	// Select a webcam from the console listing and enter it here
	camindex = 0;

    // Webcam setup
	vidGrabber.setDeviceID(camindex); // set this webcam
	vidGrabber.setDesiredFrameRate(30); // try to set this frame rate
	vidGrabber.setUseTexture(true); // we need a texture to send
	vidGrabber.setup(640, 360); // try to grab at this size. 
	ofSetWindowShape(vidGrabber.getWidth(), vidGrabber.getHeight());
	cout << "Initialized webcam [" << camdevices[camindex].deviceName << "] (" << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;
	
	
}


//--------------------------------------------------------------
void ofApp::update() {
	vidGrabber.update();
}


//--------------------------------------------------------------
void ofApp::draw() {

	char str[256]; 
	str[0] = 0;
	ofSetColor(255);

	// SpoutCam is a Spout receiver and connects to the active sender when it starts.
	// To avoid feedback from this application, wait until another sender is running.

	if (camdevices[camindex].deviceName == "SpoutCam") {
		// Release if we are the only sender left
		// so that SpoutCam does not receive
		if (spoutsender.spout.GetActiveSender(str)) {
			if (strcmp(str, sendername) == 0) {
				spoutsender.ReleaseSender();
				bInitialized = false;
				return; // Skip a frame
			}
		}
		// Quit if no senders for SpoutCam
		if (spoutsender.spout.GetSenderCount() == 0) {
			// Show SpoutCam static
			vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());
			return;
		}

	}

	// The current webcam image
	vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());

	if(!bInitialized) {

		// Create a Spout sender the same size as the webcam video
		bInitialized = spoutsender.CreateSender(sendername, vidGrabber.getWidth(), vidGrabber.getHeight()); 

		// If the active sender name has been retrieved above for SpoutCam,
		// re-set the active sender so that SpoutCam still receives from it
		// and not from this application.
		if (str[0] && strcmp(str, sendername) != 0)
			spoutsender.spout.SetActiveSender(str);

	}
 
	if(bInitialized && vidGrabber.isInitialized()) {

		if (vidGrabber.isFrameNew()) {
			// Send the video texture out for all receivers to use
			spoutsender.SendTexture(vidGrabber.getTexture().getTextureData().textureID,
				vidGrabber.getTexture().getTextureData().textureTarget,
				vidGrabber.getWidth(), vidGrabber.getHeight(), false);
		}

		// Show the webcam it is receiving from
		sprintf_s(str, 256, "Receiving  : [%s]", camdevices[camindex].deviceName.c_str());
		ofDrawBitmapString(str, 20, 20);

		// Show what it is sending
		sprintf_s(str, 256, "Sending as : [%s]", sendername);
		ofDrawBitmapString(str, 20, 35);

	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	int i = key - 48;
	if(i >= 0 && i < (int)camdevices.size()) {
		camindex = i;
		vidGrabber.close();
		vidGrabber.setDeviceID(camindex);
		vidGrabber.setDesiredFrameRate(30);
		vidGrabber.setUseTexture(true);
		if (vidGrabber.setup(640, 360)) {
			ofSetWindowShape(vidGrabber.getWidth(), vidGrabber.getHeight());
				cout << "Initialized webcam [" << camdevices[camindex].deviceName << "] (" << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;
		}
		else {
			printf("Webcam setup error. Try a different one.\n");
		}


	}

	
	// Release the sender on exit
	spoutsender.ReleaseSender();
}


//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	spoutsender.ReleaseSender(); 
}

