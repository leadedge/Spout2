/*

	Spout OpenFrameworks Sender example
	using the SpoutLibrary C-compatible dll

	Include SpoutLibrary.h in the "src" source files folder
	Include SpoutLibrary.lib where the linker can find it
	Include SpoutLibrary.dll in the "bin" executable folder

	Search for SPOUT for additions to a typical Openframeworks application

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

	//
	// Options
	//

	// OpenSpoutConsole(); // Empty console for debugging
	// Enable Spout logging to detect warnings and errors
	sender->EnableSpoutLog(); // Output is to a console window.
	// sender->SetSpoutLogLevel(SpoutLibLogLevel::SPOUT_LOG_WARNING); // For only warnings

	//
	// You can set the level above which the logs are shown
	// SPOUT_LOG_SILENT  : SPOUT_LOG_VERBOSE : SPOUT_LOG_NOTICE (default)
	// SPOUT_LOG_WARNING : SPOUT_LOG_ERROR   : SPOUT_LOG_FATAL
	// For example, to show only warnings and errors (you shouldn't see any)
	// or leave set to default Notice to see more information.
	//    sender->SetSpoutLogLevel(SpoutLibLogLevel::SPOUT_LOG_WARNING);
	//
	// You can instead, or additionally, specify output to a text file
	// with the extension of your choice
	//    sender->EnableSpoutLogFile("Spout Library Sender.log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    sender->EnableSpoutLogFile("Spout Library Sender.log", true);
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
	//    sender->SetSpoutLogLevel(SpoutLibLogLevel::SPOUT_LOG_VERBOSE);
	//    sender->SpoutLogVerbose("Message");
	//


	//
	// Other options
	//

	// Find out whether the computer is a laptop or desktop system
	char computername[16];
	DWORD nchars = 16;
	GetComputerNameA(computername, &nchars);
	if (sender->IsLaptop())
		sender->SpoutLog("Laptop system (%s)", computername);
	else
		sender->SpoutLog("Desktop system (%s)", computername);

	//
	// Sharing mode
	//
	// By default, graphics is tested for OpenGL/DirectX compatibility
	// and, if not compatible, textures are shared using system memory
	// This can be disabled if necessary as follows :
	//     sender->SetAutoShare(false); // Disable auto sharing for this application

	//
	// Graphics adapter
	//
	// If there are multiple graphics cards in the system,
	// you may wish to use a particular one for texture sharing
	// Sender and Receiver must use the same adapter.
	//
	// The number of adapters available can be queried :
	int nAdapters = sender->GetNumAdapters();
	sender->SpoutLog("Number of adapters = %d", nAdapters);
	//
	// The names can be retrieved :
	//     bool GetAdapterName(int index, char *adaptername, int maxchars = 256);
	char adaptername[256];
	for (int i=0; i<nAdapters; i++) {
		sender->GetAdapterName(i, adaptername, 256);
		sender->SpoutLog("  %d - %s", i, adaptername);
	}

	//
	// Set a specific adapter from it's index.
	// The selected adapter must be connected to a monitor.
	// For example :
	//     sender->SetAdapter(1); // (0, 1, 2 etc.)

	//
	// Graphics preference
	//
	// Note that Windows Graphics performance preferences are only available 
	// from Windows 10 April 2018 update (Version 1803, build 17134) and later.
	//
	// For a laptop with multiple graphics, you can set an application preference for
	// Windows to use the most suitable adapter for battery life or high performance.
	//
	// Many power saving graphics adapters do not support texture sharing.
	// The application should then be registered to prefer "High performance". 
	//
	// Sender and Receiver applications must have same preference to use the same adapter.
	// Once set, the preference is retained by Windows and it can be checked at any time.
	//
	// Get or Set Windows graphics performance
	//	     -1 - Not registered
	//	      0 - Let Windows decide  DXGI_GPU_PREFERENCE_UNSPECIFIED
	//	      1 - Power saving        DXGI_GPU_PREFERENCE_MINIMUM_POWER
	//	      2 - High performance    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
	//     int GetPerformancePreference(const char* path)
	//     bool SetPerformancePreference(const char* path)
	//
	// For example, GetPerformancePreference for this executable (ofSpoutExample.exe)
	// will report unregistered at first. Use SetPerformancePreference as required
	// or register the executable using Windows graphics settings.
	// "Settings > System > Display > Graphics settings" and observe the result.
	//
	// Get the graphics adapter name for a Windows preference. Useful for diagnostics.
	//     bool GetPreferredAdapterName(int preference, char* adaptername, int maxchars)
	//
	// You can set the adapter index for a performance preference without registering the
	// preference with Windows. This index is then used by CreateDX11device when DirectX
	// is intitialized. The function should be called before Spout is initialized.
	//     bool SetPreferredAdapter(int preference)
	//
	// For example :
	//     sender->SetPreferredAdapter(DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);
	// Take note of the Spout log console. CreateDX11device reports a specified adapter.
	//
	// Note that Windows Graphics performance preferences are available from Windows
	// Version 1803 (build 17134) and later.
	//

	//
	// Frame counting
	//
	// Frame counting is enabled by default.
	// Status can be queried with IsFrameCountEnabled();
	// Frame counting can be independently disabled for this application
	//     sender->DisableFrameCount();

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
	// sender->SendFbo(myFbo.getId(), senderwidth, senderheight, false);

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
	sender->SendFbo(0, senderwidth, senderheight);

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

