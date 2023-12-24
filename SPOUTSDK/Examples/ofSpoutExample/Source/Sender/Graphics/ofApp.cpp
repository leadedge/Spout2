/*

	Spout OpenFrameworks Graphics Sender example

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

	Copyright (C) 2022-2024 Lynn Jarvis.

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
	//
	// Logging functions are in a namespace and can be called directly
	// OpenSpoutConsole(); // Empty console for debugging
	EnableSpoutLog(); // Enable console logging to detect Spout warnings and errors
	//
	// Logs report information, warnings and errors as well as user logs
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
	// Refer to the "SpoutUtils.cpp" source code for further details.
	//

	//
	// SpoutMessageBox
	//
	// SpoutMessageBox is an enhanced MessageBox using "TaskDialogIndirect"
	// and is useful to present user notices with options that are not
	// available for a standard MessageBox.
	//
	// Enhancements include :
	//   Timeout option
	//   Message with variable arguments
	//   Centre on the application window or desktop
	//   Multiple buttons
	//   Hyperlinks anywhere in the message text
	//   Custom icon
	//   Modeless mode
	//
	//   Timeout option
	//    The messagebox closes after the specified interval in millseconds.
	//	    SpoutMessageBox(const char * message, DWORD dwMilliseconds = 0)
	//
	//   Message with variable arguments
	//    Presents text equivalent to that of "printf"
	//      SpoutMessageBox(const char * caption, const char* format, ...)
	//
	//   Centre on the application window or desktop
	//    Pass the window handle of the current application as for a standard MessageBox
	//      SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0)
	//    Specify the window handle for SpoutMessageBox functions that do not include one
	//      SpoutMessageBoxWindow(HWND hWnd)
	//    Pass NULL as the window handle to centre on the desktop
	//
	//   Multiple buttons
	//    Any number of buttons can be specified. The function returns the ID of the chosen button.
	//      int SpoutMessageBoxButton(int ID, std::wstring title)
	//
	//   Hyperlinks anywhere in the message text
	//    Urls can be included in the content using HTML format.
	//    For example : <a href=\"https://spout.zeal.co/\">Spout home page</a>
	//    Only double quotes are supported and must be escaped.
	//
	//   Custom icon
	//    Icons can be specifed using MessageBox definitions MB_ICONWARNING, MB_ICONINFORMATION etc
	//    A custom icon can also be specified using :
	//      SpoutMessageBoxIcon(HICON hIcon)
	//      SpoutMessageBoxIcon(std::string iconfile)
	//
	//   Modeless mode
	//     A normal MessageBox or TaskDialog is modal and stops the appication until return.
	//     This is undesirable if the messagebox originates from a dll.
	//     Modeless mode forwards the information on to "SpoutPanel" which is
	//     an independent process and does not affect the host application.
	//     Spout must have been configured using SpoutSettings.
	//       SpoutMessageBoxModeless(bool bMode = true)
	//

	//
	// Other options
	//
	// Refer to the "SpoutUtils.cpp" source code for documentation.
	//     Spout SDK version number
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

	// ---------------------------------------------------------------------------
	//
	// Sender format
	//
	// Set the sender application starting OpenGL format
	// Default DirectX format is DXGI_FORMAT_B8G8R8A8_UNORM
	//       OpenGL                             Compatible DX11 format
	//       GL_RGBA16    16 bit				(DXGI_FORMAT_R16G16B16A16_UNORM)			
	//       GL_RGBA16F   16 bit float			(DXGI_FORMAT_R16G16B16A16_FLOAT)
	//       GL_RGBA32F   32 bit float			(DXGI_FORMAT_R32G32B32A32_FLOAT)
	//       GL_RGB10_A2  10 bit 2 bit alpha	(DXGI_FORMAT_R10G10B10A2_UNORM)
	//       GL_RGBA8      8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//       GL_RGBA       8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//
	// glFormat = GL_RGBA16; // Example 16 bit rgba
	// A compatible DirectX 11 shared texture format must be set
	// so that receivers get a texture with the same format.
	// Note that some applications may not receive other formats.
	//     sender.SetSenderFormat(sender.DX11format(glFormat));
	// See the corresponding received format in the graphics receiver
	//
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
		// If the sender format has been set as above
		if (sender.GetDX11format() != DXGI_FORMAT_B8G8R8A8_UNORM) { // default
			str += sender.GLformatName(sender.GLDXformat());
		}
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

