/*

	Spout OpenFrameworks Graphics Sender example

	Spout 2.007
	OpenFrameworks 10
	Visual Studio 2017

	Copyright (C) 2019 Lynn Jarvis.

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
	
	// Optional : enable Spout logging to detect warnings and errors
	// Logging functions are in the "spoututils" namespace so they can be called directly.
	EnableSpoutLog();
	//
	// Output is to a console window.
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
	//    EnableSpoutLogFile("OF Spout Graphics Sender.log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    EnableSpoutLogFile("OF Spout Graphics Sender.log", true);
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

	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// Set the sender size here 
	// This example uses an fbo which can be different from the window size
	senderwidth = ofGetWidth();
	senderheight = ofGetHeight();

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(senderwidth, senderheight, GL_RGBA);

	// Create an image for optional pixel transfer
	myPixels.allocate(senderwidth, senderheight, GL_RGBA);

	// Set up the Spout sender with it's name and size
	// Optional : set whether to invert the outgoing texture
	// Default is true because the shared DirectX texture 
	// and OpenGL textures have different origins
	// In this example, the data from the fbo is already inverted so set false
	sender.SetupSender(sendername, senderwidth, senderheight, false);

	// Optional : set the frame rate of the application.
	// If the user has selected "Frame count" in SpoutSettings
	// a receiver will detect this rate.
	// Applications without frame rate control can use 
	// a Spout function "HoldFps" to control frame rate (see Draw())
	// ofSetFrameRate(30);

	// Local update flag for window size change
	bResized = false;
	

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	// Quit if the sender is not initialized
	if (!sender.IsInitialized())
		return;

	// Check for sender size change
	// See windowResized() for more detail
	// This could be done in Update but here we are sure
	// changes are made before rendering
	if (bResized) {
		myFbo.allocate(senderwidth, senderheight, GL_RGBA);
		myPixels.allocate(senderwidth, senderheight, GL_RGBA);
		sender.UpdateSender(sendername, senderwidth, senderheight);
		bResized = false;
	}
	
	// Draw 3D graphics demo into the fbo
	// This could be anything for your application
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
	
	// Option 1 : Send fbo data while the fbo is bound
	// with a texture attached to point 0
	sender.SendFboData(myFbo.getId());

	myFbo.end();
	// - - - - - - - - - - - - - - - - 

	// Show the result sized to the application window
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Option 2 : Send texture data
	// sender.SendTextureData(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget);

	// Option 3 : Send pixel data
	// myFbo.readToPixels(myPixels);
	// sender.SendImageData(myPixels.getData());

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sendername; str += " (";
	str += ofToString(sender.GetWidth()); str += "x";
	str += ofToString(sender.GetHeight()); str += ")";

	// Show sender fps and framecount if selected
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
	ofDrawBitmapString(str, 20, 30);

	//
	// Applications without frame rate control can call this
	// function to introduce the required delay between frames.
	//
	// Note : if you change to HoldFps(30) fps in this example,
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
	// "bResized" is a local flag to signal that update of
	// sender dimensions is necessary to retain the size of the screen.
	// The sending fbo and pixel buffer must also be re-sized
	//
	// A flag is used because Update() and Draw() are not called while
	// the window is being resized, so texture or image re-allocation
	// can be done after the mouse button is released.
	// This prevents multiple re-allocation as the window is stretched.
	// If Openframeworks is not used, a Windows application with 
	// a message loop can monitor "WM_EXITSIZEMOVE".
	//
	// Re-allocation is not actually necessary for this application
	// because the fbo size is independent of the window, but it shows
	// how sender size change can be managed if necessary.
	//
	if (w > 0 && h > 0) {
		if (w != (int)sender.GetWidth() || h != (int)sender.GetHeight()) {
			// Change the sender dimensions here
			// the changes will take effect in Draw()
			senderwidth = (unsigned int)ofGetWidth();
			senderheight = (unsigned int)ofGetHeight();
			bResized = true;
		}
	}

}

