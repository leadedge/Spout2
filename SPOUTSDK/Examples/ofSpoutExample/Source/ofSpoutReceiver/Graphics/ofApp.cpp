/*

	Spout OpenFrameworks Receiver example

    Visual Studio using the Spout SDK

	Copyright (C) 2015-2022 Lynn Jarvis.

	Spout 2.007
	OpenFrameworks 10
	Visual Studio 2017

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

	//
	// Options
	//

	// Logging (see sender example)
	// OpenSpoutConsole(); // for debugging when a console is not availlable
	EnableSpoutLog(); // Spout logging to console

	// Specify the sender to connect to.
	// The application will not connect to any other unless the user selects one.
	// If that sender closes, the application will wait for the nominated sender to open.
	// receiver.SetReceiverName("Spout Demo Sender");

	// Disable CPU sharing backup
	// If the graphics is not compatible for OpenGL/DirectX texture sharing,
	// CPU backup methods with system memory and DirectX textures are used.
	// In most cases it is satisfactory to leave auto-detection enabled,
	// but sometimes it may be preferable to simply fail if incompatible
	// so that it is clear whether high speed texture sharing is being used.
	// receiver.SetAutoShare(false);

	ofSetWindowTitle("OpenGL Receiver Example");

	ofBackground(0, 0, 0);

	// Allocate an RGBA texture to receive from the sender
	// It is resized later to match the sender - see Update()
	myTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Allocate an RGB image for this example
	// it can also be RGBA, BGRA or BGR
	myImage.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);

	// If Wait For Vertical Sync is applied by the driver, 
	// Performance can be improved if disabled here.
	ofSetVerticalSync(false);

	// Limit frame rate using timing instead
	ofSetFrameRate(60);

} // end setup


//--------------------------------------------------------------
void ofApp::update() {
	// If IsUpdated() returns true, the sender size has changed
	// and the receiving texture or pixel buffer must be re-sized.
	if (receiver.IsUpdated()) {
		myTexture.allocate(receiver.GetSenderWidth(), receiver.GetSenderHeight(), GL_RGBA);
		// Also resize the image for this example
		myImage.resize(receiver.GetSenderWidth(), receiver.GetSenderHeight());
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	//
	// ReceiveTexture or ReceiveImage connect to and receive from a sender
	// Optionally include the ID of an fbo if one is currently bound
	//
	// For successful receive, sender details can be retrieved with
	//		const char * GetSenderName();
	//		unsigned int GetSenderWidth();
	//		unsigned int GetSenderHeight();
	//		DWORD GetSenderFormat();
	//		double GetSenderFps();
	//		long GetSenderFrame();
	//
	// If receive fails, the sender has closed
	// Connection can be tested at any time with 
	//		bool IsConnected();
	//

	// Option 1 : Receive texture
	if (receiver.ReceiveTexture(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {
		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
	}

	// Option 2 : Receive pixel data
	// Specify RGB for this example. Default is RGBA.
	/*
	if (receiver.ReceiveImage(myImage.getPixels().getData(), GL_RGB)) {
		// ofImage update is necessary because the pixels have been changed
		myImage.update();
		myImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	*/
	
	/*
	// Option 3 : Receive an OpenGL shared texture to access directly
	// Only if compatible for GL/DX interop or else BindSharedTexture fails
	if(receiver.ReceiveTexture()) {
		// Bind to get access to the shared texture
		if (receiver.BindSharedTexture()) {
			// Get the shared texture ID and do something with it
			GLuint texID = receiver.GetSharedTextureID();
			// For this example, copy from the shared texture 
			// if the local texture has been updated in ofApp::update()
			if ((int)myTexture.getWidth() == receiver.GetSenderWidth()
				&& (int)myTexture.getHeight() == receiver.GetSenderHeight()) {
				receiver.CopyTexture(texID, GL_TEXTURE_2D,
					myTexture.getTextureData().textureID,
					myTexture.getTextureData().textureTarget,
					receiver.GetSenderWidth(), receiver.GetSenderHeight());
			}
			// Un-bind to release access to the shared texture
			receiver.UnBindSharedTexture();
			myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
		}
	}
	*/

	// On-screen display
	showInfo();
	
}


//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255, 255, 255);

	if(receiver.IsConnected()) {

		// Show sender details
		str = receiver.GetSenderName(); // sender name
		str += " (";

		// Show sender sharing mode if not OpenGL compatible
		if (receiver.GetSenderCPU())
			str += "CPU share : "; 

		// Show sender size
		str += to_string(receiver.GetSenderWidth()); // width
		str += "x";
		str += to_string(receiver.GetSenderHeight()); // height 

		// Applications < 2.007 will return no frame count information
		// Frame counting can also be disabled in SpoutSettings
		if (receiver.GetSenderFrame() > 0) {
			str += " : fps ";
			str += to_string((int)(round(receiver.GetSenderFps()))); // frames per second
			str += " : frame ";
			str += to_string(receiver.GetSenderFrame()); // frame since the sender started
		}
		str += ") ";
		
		ofDrawBitmapString(str, 10, 20);
	}
	else {
		str = "No sender detected";
		ofDrawBitmapString(str, 10, 20);
	}

	// Show more details if not OpenGL/DirectX compatible
	if (!receiver.IsGLDXready()) {
		if (receiver.GetAutoShare()) {
			// CPU share allowed (default)
			str = "CPU share receiver";
		}
		else {
			// CPU share disabled (program setting)
			str = "Graphics not texture share compatible";
		}
		ofDrawBitmapString(str, 10, 35);

		// Show current graphics adapter
		str = "Graphics adapter ";
		str += to_string(receiver.GetAdapter());
		str += " : ";
		str += receiver.AdapterName();
		ofDrawBitmapString(str, 10, 50);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	receiver.ReleaseReceiver();
}

//--------------------------------------------------------------
// RH mouse click to open SpoutPanel for sender selection
void ofApp::mousePressed(int x, int y, int button){
	
	// RH button for "SpoutPanel" sender selection dialog
	// Spout must have been installed and SpoutPanel
	// or SpoutSettings run at least once
	if(button == 2) {
		receiver.SelectSender();
	}

	// LH button to show a sender list in the console
	// This could be used to construct a dialog list for user selection
	if (button == 0) {
		// Show the user the current sender list
		int nSenders = receiver.GetSenderCount();
		if (nSenders > 0) {
			printf("\n");
			char SenderName[256];
			for (int i = 0; i < nSenders; i++) {
				receiver.GetSender(i, SenderName);
				printf("(%d) [%s]\n", i, SenderName);
			}
		}
		printf("Press number to detect\n");
	}

}

//--------------------------------------------------------------
// Keypress for sender selection from a list
void ofApp::keyPressed(int key) {

	// Convert ASCII to number
	int index = key - 48;
	// Single key selection (0-9)
	if (index >= 0 && index <= 9) {
		char SenderName[256];
		// Check if the sender exists
		if (receiver.GetSender(index, SenderName)) {
			printf("\n");
			// Set as active
			receiver.SetActiveSender(SenderName);
			// Change to the active sender
			receiver.SetReceiverName();
			// Change to it and lock to that sender
			// receiver.SetReceiverName(SenderName);
		}
		else {
			printf("sender index [%d] not found\n", index);
		}
	}
}


