/*

	Spout OpenFrameworks Graphics Sender example

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

	Copyright (C) 2022-2023 Lynn Jarvis.

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

 	strcpy_s(sendername, 256, "Spout Graphics Sender"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// ----------------------------------------------

	//
	// Options
	//

	//
	// Logs
	// Logging functions are in a namespace and can be called directly
	// Refer to the "SpoutUtils.cpp" source code for further details.
	//
	// OpenSpoutConsole(); // Empty console for debugging
	// EnableSpoutLog(); // Enable console logging to detect Spout warnings and errors

	// The Spout SDK version number e.g. "2.007.000"
	SpoutLog("Spout version : %s", GetSDKversion().c_str());

	// Find out whether the computer is a laptop or desktop system
	char computername[16]{};
	DWORD nchars = 16;
	GetComputerNameA(computername, &nchars);
	if (IsLaptop()) SpoutLog("Laptop system (%s)", computername);
	else SpoutLog("Desktop system (%s)", computername);

	//
	// Other options
	// Refer to the "SpoutUtils.cpp" source code for documentation.
	//     Spout SDK version number
	//     Console
	//     Spout logs
	//     SpoutMessageBox dialog
	//     Registry utilities
	//     Computer information
	//     Timing utilities

	//
	// Sharing mode
	//
	// Use of texture sharing by GPU or CPU depends on user settings (SpoutSettings)
	// and compatibility with the OpenGL/DirectX interop functions, which is tested.
	//
	// User settings can be retrieved
	//     bool GetAutoShare();
	//     bool GetCPUshare();
	// As well as the mode being used by the application
	//     bool GetGLDX();
	//     bool GetCPU();
	// Hardware compatibility can be retrieved
	//     bool IsGLDXready();
	// And also re-tested
	//     bool GetGLDXready();
	//
	// The auto detect and CPU share options set by the user are global
	// but can be enabled or disabled for individual applications
	// without affecting the global settings.
	//     sender.SetAutoShare(true/false);
	//     sender.SetCPU(true/false);

	//
	// Graphics preference
	//
	// Windows Graphics performance preferences are available from Windows
	// Version 1803 (build 17134) and later.
	// "Settings > System > Display > Graphics settings".
	// These settings apply laptops with multiple graphics that allow power saving.
	// Spout texture sharing requires "High performance" peference.
	// Both Sender and Receiver must be set to the same preference.
	// Spout functions allow programmer control over the settings.
	// Refer to "SpoutSettings" help or "Spout.cpp/.h" source code for further details.
	//

	//
	// Frame counting
	//
	// Frame counting is enabled by default.
	// Status can be queried with IsFrameCountEnabled();
	// Frame counting can be independently disabled for this application
	//     sender.DisableFrameCount();
	//
	// Set the frame rate of the application.
	// In this example, the frame rate can be set if necessary
	// with ofSetFrameRate. Applications without such frame rate
	// control can use Spout "HoldFps" (see Draw())

	//
	// Set sender application OpenGL format
	//
	//       OpenGL                             Compatible DX11 format
	//       GL_RGBA16    16 bit				(DXGI_FORMAT_R16G16B16A16_UNORM)			
	//       GL_RGBA16F   16 bit float			(DXGI_FORMAT_R16G16B16A16_FLOAT)
	//       GL_RGBA32F   32 bit float			(DXGI_FORMAT_R32G32B32A32_FLOAT)
	//       GL_RGB10_A2  10 bit 2 bit alpha	(DXGI_FORMAT_R10G10B10A2_UNORM)
	//       GL_RGBA       8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//
	// glFormat = GL_RGB16; // Example 16 bit rgba
	//
	// Set a compatible DirectX 11 shared texture format for the sender
	// so that receivers get a texture with the same format.
	// Note that some applications may not receive other formats.
	// sender.SetSenderFormat(sender.DX11format(glFormat));

	// ----------------------------------------------

	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// This example uses the window size to demonstrate sender re-sizing.
	// If the application renders to an FBO, the sender size can be
	// independent of the window and would need to be defined and managed.

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Create an image for optional pixel transfer
	myPixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR_ALPHA);

	// Give the sender a name
	// If no name is specified, the executable name is used.
	sender.SetSenderName(sendername);

	// Starting value for sender fps display
	g_SenderFps = GetRefreshRate();

	// Update caption with the produced name
	// in case of multiples of the same sender
	// (see SetSenderName)
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

	// Clear the fbo to reset the background and depth buffer
	// Background alpha should be opaque for the receiver
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate(myFbo.getWidth() / 2.0f, myFbo.getHeight() / 2.0f, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4f*myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6f;
	rotY += 0.6f;
	
	//
	// Option 1 : Send fbo
	//
	//   The fbo must be bound for read.
	//   The invert option is false because in this case 
	//   the texture attached to the fbo is already flipped in y.
	//   See also Option 4 to send the default framebuffer.
	sender.SendFbo(myFbo.getId(), ofGetWidth(), ofGetHeight(), false);

	myFbo.end();
	// - - - - - - - - - - - - - - - - 

	//
	// Option 2 : Send texture
	//
	// sender.SendTexture(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget,
		// ofGetWidth(), ofGetHeight(), false);

	//
	// Option 3 : Send image pixels
	//
	// myFbo.readToPixels(myPixels); // readToPixels is slow - but this is just an example
	// sender.SendImage(myPixels.getData(), ofGetWidth(), ofGetHeight(), GL_RGBA, false);

	// Show the result sized to the application window
	myFbo.draw(0.0f, 0.0f, (float)ofGetWidth(), (float)ofGetHeight());

	//
	// Option 4 : Send default OpenGL framebuffer
	//
	// If width and height are 0, the viewport size is used.
	//
	// sender.SendFbo(0, 0, 0);
	//
	// If performance is affected by repeated calls to glGetIntegerv,
	// the viewport size can be retrieved only when necessary to manage
	// size changes (see "windowResized"). Then the width and height 
	// values must be passed in. See also code comments in Spout.cpp.
	//
	// sender.SendFbo(0, ofGetWidth(), ofGetHeight());
	//

	// Show what it's sending
	if (sender.IsInitialized()) {
		ofSetColor(255);
		std::string str = "Sending as : ";
		str += sender.GetName(); str += " (";
		str += ofToString(sender.GetWidth()); str += "x";
		str += ofToString(sender.GetHeight()); str += ") ";
		// Sender OpenGL texture format description
		// for 16 bit and floating point types
		GLint glformat = sender.GLDXformat();
		if (glformat != GL_RGBA)
			str += sender.GLformatName(sender.GLDXformat());
		// Show sender fps and framecount if available
		if (sender.GetFrame() > 0) {
			str += " fps ";
			// Average to stabilise fps display
			g_SenderFps = g_SenderFps*.85 + 0.15*sender.GetFps();
			// Round first or integer cast will truncate to the whole part
			str += ofToString((int)(round(g_SenderFps)));
			str += " : frame  ";
			str += ofToString(sender.GetFrame());
		}
		ofDrawBitmapString(str, 10, 20);

		// Show more details if graphics is not texture share compatible
		if (!sender.IsGLDXready()) {
			// If Auto switching is allowed
			if (sender.GetAutoShare())
				str = "CPU share mode";
			else
				str = "Graphics not texture share compatible";
			ofDrawBitmapString(str, 10, 35);
			// Show the graphics adapter currently being used
			ofDrawBitmapString(sender.AdapterName(), 10, 50);
		}
	}

	//
	// Applications without frame rate control can call this
	// function to introduce the required delay between frames.
	//
	// Note : For this example, if you change to a lower fps,
	// the cube will rotate more slowly so increase RotX and RotY.
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
		myFbo.allocate(w, h, GL_RGBA);
		myPixels.allocate(w, h, GL_RGBA);
	}
}

