/*

	Spout OpenFrameworks Graphics Sender example
	using the SpoutLibrary C-compatible dll

	1) Copy SpoutLibrary.h to the source files "src" folder
	
	2) Copy SpoutLibrary.lib to any convenient folder e.g. "libs"

	3) Tell the linker to input SpoutLibrary.lib to your project
	   For Visual Studio this will be : Project > Properties > Linker > Input

	4) Tell the linker where to find it
	   For Visual Studio this will be :
	   Project > Properties > Linker > General > Aditional library directories

	5) Copy SpoutLibrary.dll to the executable folder e.g. "bin" in this case

   	To use :

	1) Include SpoutLibrary.h in your application header file
	   #include "SpoutLibrary.h"

	2) create a spout sender object pointer
	    SPOUTLIBRARY * spoutsender;

	3) Create an instance of the library
	    spoutsender = GetSpout(); 

	4) Use the object as usual :
	    spoutsender->SetupSender etc.

	Compare with the graphics sender example using the Spout SDK source files.

	Spout 2.007
	OpenFrameworks 10
	Visual Studio 2017
	Apr 03 2019

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

 	strcpy_s(senderName, 256, "SpoutLibrary Sender"); // We need a sender name for Spout
	ofSetWindowTitle(senderName); // show it on the title bar

	spoutsender = GetSpout(); // Create an instance of the Spout library
	if (!spoutsender) {
		MessageBoxA(NULL, "Load Spout library failed", "Spout Sender", MB_ICONERROR);
		exit();
	}

	//
	// Optional : enable Spout logging to detect warnings and errors
	    spoutsender->EnableSpoutLog();
	//
	// Output is to a console window.
	//
	// You can set the level above which the logs are shown
	// (0) SPOUT_LOG_SILENT  : (1) SPOUT_LOG_VERBOSE : (2) SPOUT_LOG_NOTICE (default)
	// (3) SPOUT_LOG_WARNING : (4) SPOUT_LOG_ERROR   : (5) SPOUT_LOG_FATAL
	// For example, to show only warnings and errors (you shouldn't see any)
	// or leave set to Notice to see more information.
	//    spoutsender->SetSpoutLogLevel(SPOUT_LOG_WARNING);
	//
	// You can instead, or also, specify output to a text file
	// with the extension of your choice
	//    spoutsender->EnableSpoutLogFile("SpoutLibrary Sender.log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    spoutsender->EnableSpoutLogFile("SpoutLibrary Sender.log", true);
	//
	// The file is saved in the %AppData% folder 
	//    C:>Users>username>AppData>Roaming>Spout
	// unless you specify the full path.
	// After the application has run you can find and examine the log file
	//
	// This folder can also be shown in Windows Explorer directly from the application.
	//	spoutsender->ShowSpoutLogs();
	//
	// Or the entire log can be returned as a string
	//	std::string logstring = spoutsender->GetSpoutLog();
	//
	// You can create your own logs
	// For example :
	//	spoutsender->SpoutLog("SpoutLog test");
	//
	// You can also specify the logging level :
	// For example :
	//    spoutsender->SpoutLogNotice("Important notice");
	// or :
	//    spoutsender->SpoutLogFatal("This should not happen");
	// or :
	//    spoutsender->SetSpoutLogLevel(1);
	//    spoutsender->SpoutLogVerbose("Message");
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
	spoutsender->SetupSender(senderName, senderwidth, senderheight, false);

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
	if (!spoutsender->IsInitialized())
		return;

	// Check for sender size change
	// See windowResized() for more detail
	// This could be done in Update but here we are sure
	// changes are made before rendering
	if (bResized) {
		myFbo.allocate(senderwidth, senderheight, GL_RGBA);
		myPixels.allocate(senderwidth, senderheight, GL_RGBA);
		spoutsender->UpdateSender(senderName, senderwidth, senderheight);
		bResized = false;
	}

	// Draw 3D graphics demo into the fbo
	// This could be anything for your application
	myFbo.begin();
	// Clear to reset the background and depth buffer
	// Clear background alpha to opaque for the receiver
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate((float)ofGetWidth() / 2.0, (float)ofGetHeight() / 2.0, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4*(float)ofGetHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();

	// Option 1 : Send an fbo while the fbo is bound
	spoutsender->SendFboData(myFbo.getId());

	myFbo.end();
	rotX += 0.6;
	rotY += 0.6;

	// Show the result
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Option 2 : Send a texture
	// spoutsender->SendTextureData(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget);

	// Option 3 : Send pixels
	// myFbo.readToPixels(myPixels);
	// spoutsender->SendImageData(myPixels.getData());

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += senderName; str += " (";
	str += ofToString(spoutsender->GetWidth()); str += "x";
	str += ofToString(spoutsender->GetHeight()); str += ")";

	// Show sender fps and framecount if selected
	if (spoutsender->GetFrame() > 0) {
		str += " fps ";
		str += ofToString((int)roundf(spoutsender->GetFps())); str += " : frame  ";
		str += ofToString(spoutsender->GetFrame());
	}
	else {
		// Show Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 20, 30);

	// Applications without frame rate control can call this
	// function to introduce the required delay between frames.
	// spoutsender->HoldFps(30);


}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender on exit
	spoutsender->ReleaseSender();
	// Release the library
	spoutsender->Release();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

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
		if (w != (int)spoutsender->GetWidth() || h != (int)spoutsender->GetHeight()) {
			// Change the sender dimensions here
			// the changes will take effect in Draw()
			senderwidth = (unsigned int)ofGetWidth();
			senderheight = (unsigned int)ofGetHeight();
			bResized = true;
		}
	}

}

