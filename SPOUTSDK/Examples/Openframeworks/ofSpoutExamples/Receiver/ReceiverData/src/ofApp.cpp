/*

	Spout OpenFrameworks Data Receiver example

    Example of receiving per-frame data

	1) Run this example (bin\ReceiverData.exe)
	2) Run the Data Sender example
	3) Move the mouse over the sender window :
	    the cursor position is mirrored by a yellow ball in the receiver window
	4) Press and draw a line :
	    a yellow line is drawn in the receiver window

	Applications are accumulated in the project "Binaries" folder.

	Copyright (C) 2022-2025 Lynn Jarvis.

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

	ofSetWindowTitle("Spout Data Receiver");

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma
	LoadWindowsFont(myFont, "Verdana", 12);

	// Optional logs
	// EnableSpoutLog();

	ofBackground(0, 0, 0);

	// Allocate an RGBA texture to receive from the sender
	// It is resized later to match the sender - see Update()
	myTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	
	// Variables for data exchange
	// Refer to the data sender example.
	sendermousex = 0;
	sendermousey = 0;
	senderbutton = 0;
	senderpressed = 0;
	senderdragged = 0;
	senderbytes = 0;
	
} // end setup


//--------------------------------------------------------------
void ofApp::update() {
	// If IsUpdated() returns true, the sender size has changed
	// and the receiving texture or pixel buffer must be re-sized.
	if (receiver.IsUpdated()) {
		myTexture.allocate(receiver.GetSenderWidth(), receiver.GetSenderHeight(), GL_RGBA);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	// Receive texture
	if (receiver.ReceiveTexture(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {

		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

		//
		// RECEIVE DATA
		//

		// Data is received to a char array.
		// The content is expected from the sender.
		// This example receives mouse coordinates and button status.
		// (Refer to the data sender example for more detail).
		//

		// Read the data that the sender has produced.
		// Specify the maximum number of bytes to read.
		senderbytes = receiver.ReadMemoryBuffer(receiver.GetSenderName(), senderdata, 256);
		if (senderbytes > 0) {

			//
			// Method 1 (char array)
			//
			// The sender has produced the data with sprintf.
			// The receiver can decode the data with sscanf.
			//
			sscanf_s(senderdata, "%d %d %d %d %d", 
				&sendermousex, &sendermousey, &senderbutton,
				&senderpressed, &senderdragged);

			/*
			//
			// Method 2 - XML
			// The received data is in XML format.
			//
			// In this example we will receive :
			// <mouse>
			//	  <mousex>000</mousex>
			// 	  <mousey>000</mousey>
			//	  <button>0</button>
			//	  <pressed>0</pressed>
			//	  <dragged>0</dragged>
			// </mouse>
			//
			// Load the xml data from the received char buffer
			xml.loadFromBuffer(senderdata);
			// Parse the mouse coords and button status
			std::string value;
			value = xml.getValue("mouse:mousex", "0");
			sendermousex = atoi(value.c_str());
			value = xml.getValue("mouse:mousey", "0");
			sendermousey = atoi(value.c_str());
			value = xml.getValue("mouse:button", "0");
			senderbutton = atoi(value.c_str());
			value = xml.getValue("mouse:pressed", "0");
			senderpressed = atoi(value.c_str());
			value = xml.getValue("mouse:dragged", "0");
			senderdragged = atoi(value.c_str());
			*/

			/*
			//
			// Method 3 - casting
			//
			// ReadMemoryBuffer reads data to a char array. If the sender data
			// has been cast to char from a different type, it can be cast back
			// to that type here.
			//
			// For this example, the sender has cast to char from an int array.
			// (Refer to the sender example for details).
			//
			int * mouseint = reinterpret_cast<int *>(senderdata);

			// Check that we have received the whole array as expected
			if (senderbytes/sizeof(int) >= 5) {
				sendermousex = mouseint[0];
				sendermousey = mouseint[1];
				senderbutton = mouseint[2];
				senderpressed = mouseint[3];
				senderdragged = mouseint[4];
			}
			*/

			//
			// These are just examples. There are sure to be other applications.
			//

		} // endif received data
		else {
			sendermousex = 0;
			sendermousey = 0;
			senderbutton = 0;
			senderpressed = 0;
			senderdragged = 0;
		}
	} // Endif received texture

	// Draw the sender mouse position if data has been received.
	if (receiver.IsConnected() && senderbytes > 0) {

		// Accumulate a line if the sender mouse is dragged
		if (senderdragged)
			senderpoints.push_back(ofVec2f(sendermousex, sendermousey));

		// Draw accumulated points in yellow
		if (senderpoints.size() > 1) {
			ofSetLineWidth(2.0f);
			ofSetColor(255, 255, 0);
			for (int i = 0; i < (int)senderpoints.size() - 1; i++)
				ofDrawLine(senderpoints[i], senderpoints[i + 1]);
		}

		// Clear points on sender RH button click
		if (senderbutton == 2 && senderpoints.size() > 0)
			senderpoints.clear();

		// Draw a ball at the sender mouse position
		if (senderpressed == 1 && senderbutton == 0) // Yellow if the sender mouse LH button is pressed
			ofSetColor(255, 255, 0);
		else
			ofSetColor(255, 0, 0); // Red if released
		ofDrawCircle(sendermousex, sendermousey, 0, 16);

	}

	// On-screen display
	showInfo();

}


//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255, 255, 255);

	if(receiver.IsConnected()) {
		str = "Receiving [";
		str += receiver.GetSenderName(); // sender name
		str += "] (";
		if (receiver.GetSenderCPU())
			str += "CPU share : "; 
		str += to_string(receiver.GetSenderWidth()); // width
		str += "x";
		str += to_string(receiver.GetSenderHeight()); // height 
		if (receiver.GetSenderFrame() > 0) {
			str += " : fps ";
			str += to_string((int)(round(receiver.GetSenderFps()))); // frames per second
			str += " : frame ";
			str += to_string(receiver.GetSenderFrame()); // frame since the sender started
		}
		str += ") ";
		
		DrawString(str, 10, 20);
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
