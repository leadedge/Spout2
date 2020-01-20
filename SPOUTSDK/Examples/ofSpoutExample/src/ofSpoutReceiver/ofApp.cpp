/*

	Spout OpenFrameworks Receiver example

    Visual Studio using the Spout SDK

	Copyright (C) 2020 Lynn Jarvis.

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

	ofBackground(0, 0, 0);

	// Allocate an RGBA texture to receive from the sender
	// It can be resized later to match the sender - see Update()
	myTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Also allocate an RGB image for this example
	// it can also be RGBA, BGRA or BGR
	myImage.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);

	// Optionally specify the sender to connect to.
	// The application will not connect to any other unless the user selects one.
	// If that sender closes, the application will wait for the nominated sender to open.
	// receiver.SetReceiverName("Spout DX11 Sender");

} // end setup


//--------------------------------------------------------------
void ofApp::update() {
	// If IsUpdated() returns true, the sender size has changed
	// and the receiving texture or pixel buffer must be re-sized.
	if (receiver.IsUpdated()) {
		myTexture.allocate(receiver.GetSenderWidth(), receiver.GetSenderHeight(), GL_RGBA);
		// Also resize the image for this example
		myImage.resize((int)receiver.GetSenderWidth(), (int)receiver.GetSenderHeight());
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	// ReceiveTextureData or ReceiveImageData connect to and receive from a sender
	// Optionally include the ID of an fbo if one is currently bound

	// Option 1 : Receive texture data
	receiver.ReceiveTextureData(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget);
	myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

	/*
	// Option 2 : Receive pixel data
	// Specify RGB for this example. Default is RGBA.
	if (receiver.ReceiveImageData(myImage.getPixels().getData(), GL_RGB)) {
		// ofImage update is necessary because the pixels have been changed externally
		myImage.update();
	}
	myImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	*/

	/*
	// Option 3 : Receive a shared texture and use locally
	if(receiver.ReceiveTextureData()) {
		// Get the shared texture ID
		GLuint texID = receiver.GetSharedTextureID();
		// Bind to get access to the shared texture
		receiver.BindSharedTexture();
		// Do something with it
		// For this example, copy the shared texture to the local texture
		receiver.CopyTexture(texID, GL_TEXTURE_2D,
			myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget,
			receiver.GetSenderWidth(), receiver.GetSenderHeight());
		receiver.UnBindSharedTexture();
	}
	myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
	*/


	// On-screen display
	showInfo();

}


//--------------------------------------------------------------
void ofApp::showInfo() {

	char str[256];
	ofSetColor(255);

	if(receiver.IsConnected()) {

		// Applications < 2.007 will return no frame count information
		// Frame counting can also be disabled in SpoutSettings
		if (receiver.GetSenderFrame() > 0) {
			sprintf_s(str, 256, "Receiving : [%s] (%dx%d : fps %2.0f : frame %d)",
				receiver.GetSenderName(), // sender name
				receiver.GetSenderWidth(), // width
				receiver.GetSenderHeight(), // height 
				receiver.GetSenderFps(), // fps
				receiver.GetSenderFrame()); // frame since the sender started
		}
		else {
			sprintf_s(str, 256, "Receiving : [%s] (%dx%d)",
				receiver.GetSenderName(),
				receiver.GetSenderWidth(),
				receiver.GetSenderHeight());
		}
		ofDrawBitmapString(str, 10, 20);
		sprintf_s(str, 256, "RH click select sender");
		ofDrawBitmapString(str, 10, ofGetHeight() - 20);

	}
	else {
		sprintf_s(str, 256, "No sender detected");
		ofDrawBitmapString(str, 20, 20);
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

