/*

	Spout OpenFrameworks Sender example
	using the SpoutLibrary C-compatible dll

	Include SpoutLibrary.h in the "src" source files folder
	Include SpoutLibrary.lib where the linker can find it
	Include SpoutLibrary.dll in the "bin" executable folder

	Search for SPOUT for additions to a typical Openframeworks application

	Copyright (C) 2015-2024 Lynn Jarvis.

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

	ofBackground(10, 100, 140);

	// Create a Spout sender object from the SpoutLibary dll
	sender = GetSpout();
	if (!sender) {
		MessageBoxA(NULL, "Load Spout library failed", "Spout Sender", MB_ICONERROR);
		exit();
	}

	strcpy_s(sendername, 256, "Spout Library Sender");	// Set the sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// ----------------------------------------------
	//
	// Options
	//
	// Logging functions
	// Refer to the "SpoutUtils.cpp" source code for further details.
	//
	// sender->OpenSpoutConsole(); // Empty console for debugging
	sender->EnableSpoutLog(); // Enable console logging to detect Spout warnings and errors
	// The Spout SDK version number e.g. "2.007.000"
	sender->SpoutLog("Spout version : %s", sender->GetSDKversion().c_str());
	//
	// Many other options are available but are not repeated in this example.
	// Refer to the "ofApp.cpp" source code for the graphics sender example
	// that uses the SpoutGL source files directly.
	//
	// ----------------------------------------------
	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// Set the sender size here 
	// This example uses the window size to demonstrate sender re-sizing (see "windowResized").
	// However, this application renders to an FBO, so the sender size can be independent 
	// of the window if you wish.
	senderwidth = ofGetWidth();
	senderheight = ofGetHeight();

	// Starting value for sender fps
	g_SenderFps = sender->GetRefreshRate();

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(senderwidth, senderheight, GL_RGBA);

	// Create an image for optional pixel transfer
	myPixels.allocate(senderwidth, senderheight, OF_IMAGE_COLOR_ALPHA);

	// Give the sender a name
	// If no name is specified, the executable name is used.
	sender->SetSenderName(sendername);

	// Update caption in case of multiples of the same sender
	ofSetWindowTitle(sender->GetName());

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	// All sending functions check the sender name and dimensions
	// and create or update the sender as necessary

	// For all sending functions other than SendFbo, include the ID of
	// the active framebuffer if one is currently bound.

	// Draw 3D graphics demo into the fbo
	// This could be anything
	// - - - - - - - - - - - - - - - - 
	myFbo.begin();

	// Clear to reset the background and depth buffer
	// Clear background alpha to opaque for the receiver
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate(myFbo.getWidth() / 2.0, myFbo.getHeight() / 2.0, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4*myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6;
	rotY += 0.6;
	
	// Option 1 : Send fbo
	//   The fbo must be bound for read.
	//   The invert option is false because the texture attached
	//   to the fbo is already flipped in y.
	sender->SendFbo(myFbo.getId(), senderwidth, senderheight, false);

	myFbo.end();
	// - - - - - - - - - - - - - - - - 

	// Option 2 : Send texture
	// sender->SendTexture(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget,
		// senderwidth, senderheight, false);

	// Option 3 : Send image pixels
	// myFbo.readToPixels(myPixels); // readToPixels is slow - but this is just an example
	// sender->SendImage(myPixels.getData(),senderwidth, senderheight, GL_RGBA, false);

	// Show the result sized to the application window
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Option 4 : Send default framebuffer
	// The invert option is default true in this case
	// sender->SendFbo(0, senderwidth, senderheight);

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender->GetName(); str += " (";
	str += ofToString(sender->GetWidth()); str += "x";
	str += ofToString(sender->GetHeight()); str += ")";

	// Show sender fps and framecount if available
	if (sender->GetFrame() > 0) {
		str += " fps ";
		// Average to stabilise fps display
		g_SenderFps = 0.95*g_SenderFps + 0.05*sender->GetFps();
		// Round first or integer cast will truncate to the whole part
		str += ofToString((int)(round(g_SenderFps)));
		str += " : frame  ";
		str += ofToString(sender->GetFrame());
	}
	else {
		// Show Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 10, 20);

	// Show more details if graphics is not texture share compatible
	if (!sender->IsGLDXready()) {
		// If Auto switching is allowed
		if(sender->GetAutoShare())
			str = "CPU share mode";
		else
			str = "Graphics not texture share compatible";
		ofDrawBitmapString(str, 10, 35);
		// Show the graphics adapter currently being used
		ofDrawBitmapString(sender->AdapterName(), 10, 50);
	}

	//
	// Applications without frame rate control can call this
	// function to introduce the required delay between frames.
	//
	// Note : If you change to a lower fps in this example,
	// the cube will rotate more slowly (increase RotX and RotY).
	//
	// sender->HoldFps(30);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender
	sender->ReleaseSender();
	// Release the library on exit
	sender->Release();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	// If the sending size matches the window size,
	// the fbo, texture or image should be updated here.
	if (w > 0 && h > 0) {
		senderwidth = w;
		senderheight = h;
		myFbo.allocate(senderwidth, senderheight, GL_RGBA);
		myPixels.allocate(senderwidth, senderheight, GL_RGBA);
	}
}

