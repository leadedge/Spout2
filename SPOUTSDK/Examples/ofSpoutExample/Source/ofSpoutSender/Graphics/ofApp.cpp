/*

	Spout OpenFrameworks Graphics Sender example

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2017

	Copyright (C) 2015-2022 Lynn Jarvis.

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

 	strcpy_s(sendername, 256, "OF Spout Graphics Sender"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	//
	// Options
	//

	// OpenSpoutConsole(); // Empty console for debugging
	// Enable Spout logging to detect warnings and errors
	// (Logging functions are in the "spoututils" namespace so they can be called directly.)
	EnableSpoutLog(); // Output is to a console window.
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // For only warnings

	//
	// You can set the level above which the logs are shown
	// SPOUT_LOG_SILENT  : SPOUT_LOG_VERBOSE : SPOUT_LOG_NOTICE (default)
	// SPOUT_LOG_WARNING : SPOUT_LOG_ERROR   : SPOUT_LOG_FATAL
	// For example, to show only warnings and errors (you shouldn't see any)
	// or leave set to default Notice to see more information.
	//    SetSpoutLogLevel(SPOUT_LOG_WARNING);
	//
	// You can instead, or additionally, specify output to a text file
	// with the extension of your choice
	//    EnableSpoutLogFile("OF Spout Graphics sender.log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    EnableSpoutLogFile("OF Spout Graphics sender.log", true);
	//
	// The file is saved in the %AppData% folder 
	//    C:>Users>username>AppData>Roaming>Spout
	// unless you specify the full path.
	// After the application has run you can find and examine the log file
	//
	// This folder can also be shown in Windows Explorer directly from the application.
	//    ShowSpoutLogs();
	//
	// Or the entire log can be returned as a string
	//    std::string logstring = GetSpoutLog();
	//
	// You can also create your own logs
	// For example :
	//    SpoutLog("SpoutLog test");
	//
	// Or specify the logging level :
	// For example :
	//    SpoutLogNotice("Important notice");
	// or :
	//    SpoutLogFatal("This should not happen");
	// or :
	//    SetSpoutLogLevel(SPOUT_LOG_VERBOSE);
	//    SpoutLogVerbose("Message");
	//

	//
	// Other options
	//

	//
	// Sharing mode
	//
	// By default, graphics is tested for OpenGL/DirectX compatibility
	// and, if not compatible, textures are shared using system memory
	// This can be disabled if necessary
	// sender.SetAutoShare(false); // Disable auto sharing for this application

	//
	// Graphics adapter
	//
	// If there are multiple graphics cards in the system,
	// you may wish to use a particular one for texture sharing
	// Sender and Receiver must use the same adapter.
	//
	// The number of adapters available can be queried :
	// int nAdapters = sender.GetNumAdapters();
	// SpoutLog("Number of adapters = %d", nAdapters);
	//
	// The names can be retrieved :
	// bool GetAdapterName(int index, char *adaptername, int maxchars = 256);
	//
	// Set a specific adapter from it's index :
	// sender.SetAdapter(1); // use the second in the list (0, 1, 2 etc.)

	// Frame counting is enabled by default.
	// Status can be queried with IsFrameCountEnabled();
	// Frame counting can be independently disabled for this application
	// sender.DisableFrameCount();

	// Set the frame rate of the application.
	// In this example, the frame rate can be set with : ofSetFrameRate
	// but applications without frame rate control can use "HoldFps" (see Draw())

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

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(senderwidth, senderheight, GL_RGBA);

	// Create an image for optional pixel transfer
	myPixels.allocate(senderwidth, senderheight, OF_IMAGE_COLOR_ALPHA);

	// Give the sender a name
	// If no name is specified, the executable name is used.
	sender.SetSenderName(sendername);

	// Update caption in case of multiples of the same sender
	ofSetWindowTitle(sender.GetName());

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
	// sender.SendFbo(myFbo.getId(), senderwidth, senderheight, false);

	myFbo.end();
	// - - - - - - - - - - - - - - - - 

	// Option 2 : Send texture
	// sender.SendTexture(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget,
		// senderwidth, senderheight, false);

	// Option 3 : Send image pixels
	// myFbo.readToPixels(myPixels); // readToPixels is slow - but this is just an example
	// sender.SendImage(myPixels.getData(),senderwidth, senderheight, GL_RGBA, false);

	// Show the result sized to the application window
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Option 4 : Send default framebuffer
	// The invert option is default true in this case
	sender.SendFbo(0, senderwidth, senderheight);

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender.GetName(); str += " (";
	str += ofToString(sender.GetWidth()); str += "x";
	str += ofToString(sender.GetHeight()); str += ")";

	// Show sender fps and framecount if available
	if (sender.GetFrame() > 0) {
		str += " fps ";
		str += ofToString((int)roundf(sender.GetFps()));
		str += " : frame  ";
		str += ofToString(sender.GetFrame());
	}
	else {
		// Show Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 10, 20);

	// Show more details if graphics is not texture share compatible
	if (!sender.IsGLDXready()) {
		// If Auto switching is allowed
		if(sender.GetAutoShare())
			str = "CPU share mode";
		else
			str = "Graphics not texture share compatible";
		ofDrawBitmapString(str, 10, 35);
		// Show the graphics adapter currently being used
		ofDrawBitmapString(sender.AdapterName(), 10, 50);
	}

	//
	// Applications without frame rate control can call this
	// function to introduce the required delay between frames.
	//
	// Note : If you change to a lower fps in this example,
	// the cube will rotate more slowly (increase RotX and RotY).
	//
	// sender.HoldFps(30);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender on exit
	sender.ReleaseSender();
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

