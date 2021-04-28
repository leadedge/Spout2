/*

	Spout OpenFrameworks Graphics Sender example
	using the 2.007 SpoutLibrary C-compatible dll

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
	    SPOUTLIBRARY * sender;

	3) Create an instance of the library
	    sender = GetSpout(); 

	4) Use the object as usual :
	    sender->SendTexture(... ) etc.

	Compare with the graphics sender example using the Spout SDK source files.

	Spout 2.007
	OpenFrameworks 10
	Visual Studio 2017

	Copyright (C) 2021 Lynn Jarvis.

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

 	strcpy_s(sendername, 256, "SpoutLibrary Sender"); // We need a sender name for Spout
	ofSetWindowTitle(sendername); // show it on the title bar

	sender = GetSpout(); // Create an instance of the Spout library
	if (!sender) {
		MessageBoxA(NULL, "Load Spout library failed", "Spout Sender", MB_ICONERROR);
		exit();
	}

	//
	// Options
	//

	// sender->OpenSpoutConsole(); // Empty console for debugging
	// Enable Spout logging to detect warnings and errors
	// (Logging functions are in the "spoututils" namespace so they can be called directly.)
	sender->EnableSpoutLog(); // Output is to a console window.
	//
	// You can set the level above which the logs are shown
	// SPOUT_LOG_SILENT  : SPOUT_LOG_VERBOSE : SPOUT_LOG_NOTICE (default)
	// SPOUT_LOG_WARNING : SPOUT_LOG_ERROR   : SPOUT_LOG_FATAL
	// For example, to show only warnings and errors (you shouldn't see any)
	// or leave set to default Notice to see more information.
	//    sender->SetSpoutLogLevel(SPOUT_LOG_WARNING);
	//
	// You can instead, or additionally, specify output to a text file
	// with the extension of your choice
	//    sender->EnableSpoutLogFile("OF Spout Graphics sender->log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    sender->EnableSpoutLogFile("OF Spout Graphics sender->log", true);
	//
	// The file is saved in the %AppData% folder 
	//    C:>Users>username>AppData>Roaming>Spout
	// unless you specify the full path.
	// After the application has run you can find and examine the log file
	//
	// This folder can also be shown in Windows Explorer directly from the application.
	//    sender->ShowSpoutLogs();
	//
	// Or the entire log can be returned as a string
	//    std::string logstring = sender->GetSpoutLog();
	//
	// You can also create your own logs
	// For example :
	//    sender->SpoutLog("SpoutLog test");
	//
	// Or specify the logging level :
	// For example :
	//    sender->SpoutLogNotice("Important notice");
	// or :
	//    sender->SpoutLogFatal("This should not happen");
	// or :
	//    sender->SetSpoutLogLevel(SPOUT_LOG_VERBOSE);
	//    sender->SpoutLogVerbose("Message");
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
	// sender->SetAutoShare(false); // Disable auto sharing for this application

	//
	// Graphics adapter
	//
	// If there are multiple graphics cards in the system,
	// you may wish to use a particular one for texture sharing
	//
	// The number of adapters available can be queried :
	// int nAdapters = sender->GetNumAdapters();
	// printf("Number of adapters = %d\n", nAdapters);
	//
	// The names can be retrieved :
	// bool GetAdapterName(int index, char *adaptername, int maxchars = 256);
	//
	// Set a specific adapter from it's index :
	// sender->SetAdapter(1); // use the second in the list (0, 1, 2 etc.)

	// Frame counting is enabled by default
	// Status can be queried with IsFrameCountEnabled();
	// Frame counting can be independently disabled for this application
	// sender->DisableFrameCount();

	// Set the frame rate of the application.
	// In this example, the frame rate can be set with : ofSetFrameRate(30)
	// but applications without frame rate control can use "HoldFps" (see Draw())

	// ----------------------------------------------


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

	// Give the sender a name
	// If no name is specified, the executable name is used
	sender->SetSenderName(sendername);
	// Update caption in case of multiples of the same sender
	ofSetWindowTitle(sender->GetName());

	// Optional : set the frame rate of the application.
	// If the user has selected "Frame count" in SpoutSettings
	// a receiver will detect this rate.
	// Applications without frame rate control can use 
	// a Spout function "HoldFps" to control frame rate (see Draw())
	// ofSetFrameRate(30);

	// Mouse coordinates to send to receiver
	mousex = 0;
	mousey = 0;

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	// For applications requiring frame accuracy between 
	// sender and receiver, wait for a ready signal from 
	// the receiver before rendering to synchronise with
	// the receiver fps. Use a timeout greater than the
	// expected delay. Refer to the receiver example.
	// sender.WaitFrameSync(sender.GetName(), 67);

	// All sending functions check the sender name and dimensions
	// and create or update the sender as necessary

	// In this example, the fbo texture is already inverted
	// so set the invert option false for all sending functions


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

	// Send fbo
	//   The fbo must be bound for read.
	//   The invert option is false because the fbo is already flipped in y.
	sender->SendFbo(myFbo.getId(), senderwidth, senderheight, false);

	myFbo.end();
	// - - - - - - - - - - - - - - - - 

	// Option 2 : Send texture
	// sender->SendTexture(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget,
		// senderwidth, senderheight, false);

	// Option 3 : Send image pixels
	// myFbo.readToPixels(myPixels);
	// sender->SendImage(myPixels.getData(),senderwidth, senderheight, GL_RGBA, false);

	// Show the result sized to the application window
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Option : send a data buffer.
	// Send mouse coordinates to the receiver.
	// Refer to the receiver example.
	sprintf_s(senderdata, 256, "%d %d", mousex, mousey);
	sender->WriteMemoryBuffer(sender->GetName(), senderdata, 256);

	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender->GetName(); str += " (";
	str += ofToString(sender->GetWidth()); str += "x";
	str += ofToString(sender->GetHeight()); str += ")";

	// Show sender fps and framecount if available
	if (sender->GetFrame() > 0) {
		str += " fps ";
		str += ofToString((int)roundf(sender->GetFps()));
		str += " : frame  ";
		str += ofToString(sender->GetFrame());
	}
	else {
		// Show Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 10, 30);

	// Show more details if graphics is not texture share compatible
	if (!sender->IsGLDXready()) {
		// If Auto switching is allowed
		if (sender->GetAutoShare())
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
	// Note : if you change to HoldFps(30) fps in this example,
	// the cube will rotate more slowly (increase RotX and RotY).
	//
	// sender->HoldFps(30);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender on exit
	sender->ReleaseSender();
	// Release the library
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

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
	mousex = x;
	mousey = y;
}

