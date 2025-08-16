/*

	Spout OpenFrameworks Sender Sync example

    Example of synchronizing sender and receiver

	Synchronization is necessary if the sending and receiving applications
	require	frame accuracy and missed or duplicated frames are not acceptable.

	See also the receiver sync example.

	The code option in this example is set initially
	for the Sender to wait on the receiver

	The sender waits for a sync signal from the receiver before producing a frame

	1) Run this sender example
	2) On open, the sender will cycle at 60 fps
	3) Open the receiver sync example and leave sync enabled
	   On open, the receiver will cycle at 30 fps
	4) The sender will synchronise with the receiver at 30fps
	5) Disable sender Sync while this example is running
	6) The sender does not wait for the receiver and will cycle at 60fps

	Applications are accumulated in the project "Binaries" folder.

	Consult the code comments for the reverse (receiver waits on the sender)

	Spout 2.007
	OpenFrameworks 12
	Visual Studio 2022

	Copyright (C) 2025 Lynn Jarvis.

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

	// Optional console for windowed application (see main.cpp)
	// OpenSpoutConsole();
	// Optional logs
	// EnableSpoutLog();

 	strcpy_s(sendername, 256, "Sender Sync Example"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// Load a Windows truetype font to avoid dependency on a font file.
	LoadWindowsFont(myFont, "Verdana", 12);

	// Give the sender a name
	sender.SetSenderName(sendername);
	
	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	myBoxImage.load("SpoutBox.jpg"); // image for the cube texture
 	rotX = rotY = 0.0f;

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Enable sync events
	bSync = true;
	sender.EnableFrameSync(bSync);

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

	if (bSenderWait) {
		// =======================================================================
		//
		// Sender waits on the receiver
		//
		// If the sender cycle is faster, the receiver will miss frames.
		//
		// BEFORE processing, the sender can wait until the receiver
		// signals that it is ready to receive a frame.
		// Use a timeout greater than the expected delay.
		// The receiver signals that it is ready after processing the last frame.
		// (See the receiver sync example)
		//
		sender.WaitFrameSync(sender.GetName(), 67);
		//
		// To demonstrate the effect of sync functions, reduce the receiver frame rate.
		// (See the receiver sync example).
		// The sender will synchronize with the receiver frame rate.
		//
		// The on-screen display will show the actual sender frame rate.
		// Build without WaitFrameSync or with no timeout to observe the difference.
		// Or press the space bar to disable/enable sync.
		//
		// =======================================================================
	}


	// Draw 3D graphics into the fbo
	myFbo.begin();
	ofClear(10, 100, 140, 255);
	ofEnableDepthTest(); // enable depth comparisons for the cube
	ofPushMatrix();
	ofTranslate(myFbo.getWidth() / 2.0, myFbo.getHeight() / 2.0, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4 * myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6;
	rotY += 0.6;
	ofDisableDepthTest(); // Or graphics draw fails

	// Send fbo
	sender.SendFbo(myFbo.getId(), ofGetWidth(), ofGetHeight(), false);

	myFbo.end();

	// Draw the result
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// On-screen display
	showInfo();

	if(!bSenderWait) {
		// =======================================================================
		//
		// Receiver waits on the sender
		//
		// If the sender cycle is slower, the receiver will duplicate frames.
		// AFTER processing, the sender can signal that a new frame has been produced.
		// Before receiving, the receiver waits for the sender signal.
		// (See the receiver sync example)
		sender.SetFrameSync(sender.GetName());
		//
		// To demonstrate the effect of sync functions, reduce the sender frame rate.
		// The receiver will synchronize with the sender frame rate.
		// Space bar to disable/enable sync
		if (bSync) sender.HoldFps(30);
		//
		// =======================================================================
	}

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
		str += std::to_string((int)(roundf(ofGetFrameRate()))); // the sending frame rate
		str += " : frame  ";
		str += ofToString(sender.GetFrame());
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

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	// Space bar - enable/disable or open/close sync
	if (key == ' ') {
		bSync = !bSync;
		if (bSenderWait) {
			// If the sender waits on the receiver
			// Enable or disable waiting on the sync event
			// so that the sender does not wait on the receiver event
			sender.EnableFrameSync(bSync);
		}
		else {
			// If the receiver waits on the sender
			if (!bSync) {
				// Close the sync event so that
				// the receiver has no event to wait on
				sender.CloseFrameSync();
			}
			else {
				// This creates the sync event again
				sender.EnableFrameSync(bSync);
			}
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
