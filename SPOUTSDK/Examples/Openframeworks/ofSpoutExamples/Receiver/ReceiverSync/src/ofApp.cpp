/*

	Spout OpenFrameworks Receiver Sync example

    Example of synchronizing a receiver and sender

	Synchronization is necessary if the sending and receiving applications 
	require	frame accuracy and missed or duplicated frames are not acceptable.

	See also the sender sync example.

	The code option in this example is set initially
	for the Sender to wait on the receiver

	The receiver produces a sync signal when ready for the sender to produce a new frame

	1) Open the sender sync example and leave sync enabled
	   On open, the sender will cycle at 60 fps
	2) Run this receiver example
	   On open, the receiver will cycle at 30 fps
	3) The sender will synchronise to the receiver at 30fps
	4) Disable receiver Sync while this example is running
	5) The receiver receives from the sender every frame and cycles at 60fps

	Applications are accumulated in the project "Binaries" folder.

	Consult the code comments for the reverse (receiver waits on the sender)

	Copyright (C) 2025 Lynn Jarvis.

	Spout 2.007
	OpenFrameworks 12
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

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma
	LoadWindowsFont(myFont, "Verdana", 12);

	// Optional console for windowed application (see main.cpp)
	// OpenSpoutConsole();
	// Option - enanble console logs
	// EnableSpoutLog();

	// Allocate an RGBA texture to receive from the sender
	// It is resized later to match the sender - see Update()
	myTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Enable sync events
	bSync = true;
	receiver.EnableFrameSync(bSync);

	//
	// The sender waits on the receiver
	//
	// Change this for both sender and receiver
	// demonstration applications as required
	bSenderWait = true;

	
} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	if (!bSenderWait) {
		// =======================================================================
		//
		// Receiver waits on the sender
		//
		// If the receiver cycles faster than the sender, there will be duplicated frames.
		// BEFORE processing, the receiver can wait until the sender signals
		// that it has sent a new frame. Use a timeout greater than the expected delay.
		// The sender signals that it is ready after processing the last frame.
		// (See the sender sync example)
		//
		receiver.WaitFrameSync(receiver.GetSenderName(), 67);
		//
		// To show the effect of sync functions, reduce the sender frame rate.
		// (See the sender sync example).
		// The receiver will synchronize with the sender frame rate.
		//
		// The on-screen display will show the actual received frame rate.
		// Build without WaitFrameSync or with no timeout and observe the difference.
		// Or press the space bar to disable/enable sync.
		//
		// =======================================================================
	}

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

	if (bSenderWait) {
		// =======================================================================
		//
		// Sender waits on the receiver
		//
		// To demonstrate the effect of sync functions, reduce the receiver frame rate.
		// The sender will synchronize with the receiver frame rate.
		// Space bar to disable/enable sync
		if (bSync) receiver.HoldFps(30);
		//
		// If the receiver cycles slower than the sender, there will be missed frames.
		// AFTER processing, the receiver can signal when it is ready to receive the next frame.
		// Before sending, the sender waits for the receiver signal.
		// (See the sender sync example)
		//
		receiver.SetFrameSync(receiver.GetSenderName());
		//
		// =======================================================================
	}
}


//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255, 255, 255);

	if(receiver.IsConnected()) {
		str = receiver.GetSenderName(); // sender name
		str += " (";
		str += std::to_string(receiver.GetSenderWidth()); // width
		str += "x";
		str += std::to_string(receiver.GetSenderHeight()); // height 
		str += ") ";
		if (receiver.GetSenderFrame() > 0) {
			str += " : fps ";
			str += std::to_string((int)(roundf(ofGetFrameRate()))); // the actual received frame rate
			str += " : frame ";
			str += std::to_string(receiver.GetSenderFrame()); // frame since the sender started
		}
		DrawString(str, 10, 20);

		if (bSync) {
			if (bSenderWait)
				str = "Sender waits for the receiver ready to receive a frame";
			else
				str = "Receiver waits for the sender to produce a frame";
			DrawString(str, (ofGetWidth()-(int)str.length()*8)/2, ofGetHeight()-40);
			str = "  SPACE to disable sync";
		}
		else {
			str = "SPACE to enable sync";
		}
		DrawString(str, (ofGetWidth()-(int)str.length()*8)/2, ofGetHeight()-20);
	}
	else {
		str = "No sender detected";
		DrawString(str, 10, 20);
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

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	// Space bar - enable/disable or open/close sync
	if (key == ' ') {
		bSync = !bSync;
		if (bSenderWait) {
			// If the sender waits on the receiver
			if (!bSync) {
				// Close the sync event
				// So that the sender has no event to wait on
				receiver.CloseFrameSync();
			}
			else {
				// This creates the sync event again
				receiver.EnableFrameSync(bSync);
			}
		}
		else {
			// If the receiver waits on the sender
			// Enable or disable waiting on the sync event
			// so that the receiver does not wait on the sender event
			receiver.EnableFrameSync(bSync);
		}
	}
}


//--------------------------------------------------------------
// Load a Windows truetype font
bool ofApp::LoadWindowsFont(ofTrueTypeFont& font, std::string name, int size)
{
	std::string fontfolder;
	char* path = nullptr;
	errno_t err = _dupenv_s(&path, NULL, "WINDIR");
	if (err == 0 && path) {
		fontfolder = path;
		fontfolder += "\\Fonts\\";
		fontfolder += name;
		fontfolder += ".ttf";
		if (_access(fontfolder.c_str(), 0) != -1) {
			return font.load(fontfolder, size, true, true);
		}
	}
	return false;
}

//--------------------------------------------------------------
void ofApp::DrawString(std::string str, int posx, int posy)
{
	if (myFont.isLoaded()) {
		myFont.drawString(str, posx, posy);
	}
	else {
		// This will only happen if the Windows font is not foud
		// Quick fix because the default font is wider
		int x = posx-20;
		if (x <= 0) x = 10;
		ofDrawBitmapString(str, x, posy);
	}
}
