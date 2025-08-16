/*

	Spout OpenFrameworks Multiple Receiver example

	- Opens two receivers, side by side in the window
	- Each receiver is independent and can receive from different senders
	- Mouse right click in each half of the window to select a sender 

	Visual Studio using the Spout SDK

	Copyright (C) 2015-2025 Lynn Jarvis.

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

	ofBackground(0, 0, 0);
	ofSetWindowTitle("Multiple Receiver Example");

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma
	LoadWindowsFont(myFont, "Verdana", 12);

	// Option - enable spout logging
	EnableSpoutLog();

	// Allocate a texture for the firt receiver
	myTexture1.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	// Option : Receive only from "Spout Demo Sender"
	// Any other sender name can be specified.
	// receiver1->SetReceiverName("Spout Demo Sender");

	// Second receiver
	myTexture2.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	// Option : Receive only from "Spout Demo Sender_1"
	// (Open the demo sender again to create it)
	// receiver2->SetReceiverName("Spout Demo Sender_1");

	// Make the window double width for the two receivers
	ofSetWindowShape(ofGetWidth()*2, ofGetHeight());

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);


} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}


//--------------------------------------------------------------
void ofApp::draw() {

	// Connect to and receive from the active sender
	// (For the SetReceiverName option, "Spout Demo Sender")
	if (receiver1.ReceiveTexture(myTexture1.getTextureData().textureID, myTexture1.getTextureData().textureTarget)) {
		// If IsUpdated() returns true, the sender size has changed
		// Re-allocate the receiving texture
		if (receiver1.IsUpdated())
			myTexture1.allocate(receiver1.GetSenderWidth(), receiver1.GetSenderHeight(), GL_RGBA);
		myTexture1.draw(0, 0, ofGetWidth()/2, ofGetHeight());
	}

	// Connect to and receive from the active sender
	// (For the SetReceiverName option, "Spout Demo Sender_1")
	if (receiver2.ReceiveTexture(myTexture2.getTextureData().textureID, myTexture2.getTextureData().textureTarget)) {
		if (receiver2.IsUpdated())
			myTexture2.allocate(receiver2.GetSenderWidth(), receiver2.GetSenderHeight(), GL_RGBA);
		myTexture2.draw(ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight());
	}

	// On-screen display
	showInfo();

}

//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255);

	if (receiver1.IsConnected()) {
		str = receiver1.GetSenderName(); // sender name
		str += " (";
		str += std::to_string(receiver1.GetSenderWidth()); // width
		str += "x";
		str += std::to_string(receiver1.GetSenderHeight()); // height 
		str += ") ";
		DrawString(str, 10, 20);
		DrawString("Right click to select sender 1", 15, ofGetHeight() - 20);
	}
	else {
		str = "No sender detected";
		DrawString(str, 10, 20);
	}

	if (receiver2.IsConnected()) {
		str = receiver2.GetSenderName(); // sender name
		str += " (";
		str += std::to_string(receiver2.GetSenderWidth()); // width
		str += "x";
		str += std::to_string(receiver2.GetSenderHeight()); // height 
		str += ") ";
		DrawString(str, ofGetWidth()/2+10, 20);
		DrawString("Right click to select sender 2", ofGetWidth() / 2 + 20, ofGetHeight() - 20);
	}
	else {
		str = "No sender detected";
		DrawString(str, ofGetWidth()/2+10, 20);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {

	// Release the receivers
	receiver1.ReleaseReceiver();
	receiver2.ReleaseReceiver();

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	
	if (button == 2) {
		if (x < ofGetWidth() / 2)
			receiver1.SelectSender();
		else
			receiver2.SelectSender();
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

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
