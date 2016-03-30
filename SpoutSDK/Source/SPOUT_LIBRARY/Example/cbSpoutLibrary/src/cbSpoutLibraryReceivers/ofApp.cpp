/*

	Spout OpenFrameworks Multiple Receiver example
	using the SpoutLibrary C-compatible dll

    Include SpoutLibrary.h in the "src" source files folder
    Include SpoutLibrary.lib where the linker can find it
    Include SpoutLibrary.dll in the "bin" executable folder

	Copyright (C) 2016 Lynn Jarvis.

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

	// Set the window title to show that it is a Spout Receiver
	ofSetWindowTitle("Spout Receivers");

    // ====== SPOUT =====
    receiver1 = GetSpout(); // Create an instance of the Spout library
    receiver2 = GetSpout(); // Create another instance for the second receiver
  	if(!receiver1 || !receiver2) {
        MessageBoxA(NULL, "Load Spout library failed", "Spout Receivers", MB_ICONERROR);
        exit();
  	}
	bInitialized1	= false; // Spout receiver 1 initialization
	bInitialized2	= false; // Spout receiver 2 initialization
	SenderName1[0]	= 0;     // the name will be filled when receiver 1 connects to a sender
	SenderName2[0]	= 0;

	g_Width1  = ofGetWidth();
	g_Height1 = ofGetHeight();
	myTexture1.allocate(g_Width1, g_Height1, GL_RGBA);

	g_Width2  = ofGetWidth();
	g_Height2 = ofGetHeight();
	myTexture2.allocate(g_Width2, g_Height2, GL_RGBA);

} // end setup

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	char str[256];
	ofSetColor(255);
	unsigned int width, height;

	if(!bInitialized1) {
		if(receiver1->CreateReceiver(SenderName1, width, height, true)) {
			if(width != g_Width1 || height != g_Height1 ) {
				g_Width1  = width;
				g_Height1 = height;
				myTexture1.allocate(g_Width1, g_Height1, GL_RGBA);
			}
			bInitialized1 = true;
			return;
		} // receiver was initialized
	} // end initialization1


	if(!bInitialized2) {
		if(receiver2->CreateReceiver(SenderName2, width, height, true)) {
			if(width != g_Width2 || height != g_Height2 ) {
				g_Width2  = width;
				g_Height2 = height;
				myTexture2.allocate(g_Width2, g_Height2, GL_RGBA);
			}
			bInitialized2 = true;
			return;
		} // receiver was initialized
	} // end initialization1


	// The receiver has initialized so OK to draw
	if(bInitialized1) {
		width  = g_Width1;
		height = g_Height1;
		if(receiver1->ReceiveTexture(SenderName1, width, height, myTexture1.getTextureData().textureID, myTexture1.getTextureData().textureTarget)) {
			if(width != g_Width1 || height != g_Height1 ) {
				g_Width1  = width;
				g_Height1 = height;
				myTexture1.allocate(g_Width1, g_Height1, GL_RGBA);
				return;
			}
			myTexture1.draw(0, 0, ofGetWidth()/2, ofGetHeight());

			// Show what it is receiving
			sprintf(str, "Receiving from : [%s]", SenderName1);
			ofDrawBitmapString(str, 20, 20);
		}
		else {
			receiver1->ReleaseReceiver();
			bInitialized1 = false;
			return;
		}
	}

	if(bInitialized2) {
		width  = g_Width2;
		height = g_Height2;
		if(receiver2->ReceiveTexture(SenderName2, width, height, myTexture2.getTextureData().textureID, myTexture2.getTextureData().textureTarget)) {
			if(width != g_Width2 || height != g_Height2 ) {
				g_Width2  = width;
				g_Height2 = height;
				myTexture2.allocate(g_Width2, g_Height2, GL_RGBA);
				return;
			}
			myTexture2.draw(ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight());

			// Show what it is receiving
			sprintf(str, "Receiving from : [%s]", SenderName2);
			ofDrawBitmapString(str, ofGetWidth()/2 + 20, 20);
		}
		else {
			receiver2->ReleaseReceiver();
			bInitialized2 = false;
			return;
		}
	}

	sprintf(str, "RH click to select sender 1");
	ofDrawBitmapString(str, 15, ofGetHeight()-20);

	sprintf(str, "RH click to select sender 2");
	ofDrawBitmapString(str, ofGetWidth()/2 + 20, ofGetHeight()-20);

}


//--------------------------------------------------------------
void ofApp::exit() {

    if(receiver1) {
        receiver1->ReleaseReceiver(); // Release the receiver
        receiver1->Release(); // Release the Spout SDK library instance
    }
    if(receiver2) {
        receiver2->ReleaseReceiver(); // Release the receiver
        receiver2->Release(); // Release the Spout SDK library instance
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	if(button == 2) {
        if(x < ofGetWidth()/2)
            receiver1->SelectSenderPanel();
        else
            receiver2->SelectSenderPanel();
	}
}

