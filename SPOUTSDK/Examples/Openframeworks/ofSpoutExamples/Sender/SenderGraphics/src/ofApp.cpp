/*

	Spout OpenFrameworks Graphics Sender example

	Spout 2.007
	OpenFrameworks 12
	Visual Studio 2022

	Copyright (C) 2022-2025 Lynn Jarvis.

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

	ofBackground(0, 0, 0);

	// Give the sender a name
	// If no name is specified, the executable name is used.
	strcpy_s(m_sendername, 256, "Spout Graphics Sender");
	sender.SetSenderName(m_sendername);

	// Show it on the title bar
	ofSetWindowTitle(m_sendername);

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// Set a custom icon
	std::string icopath = GetExePath() + "data\\Spout.ico";
	SetClassLongPtrA(ofGetWin32Window(), GCLP_HICON, (LONG_PTR)LoadImageA(nullptr, icopath.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE));

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma
	LoadWindowsFont(myFont, "Verdana", 12);


	// ----------------------------------------------
	//
	// Options
	//

	// Empty console for debugging if linker options are
	// "Windows (/SUBSYSTEM:WINDOWS)". See Main.cpp.
	// A title for the console window can be specified.
	// Default is the executable name.
	// OpenSpoutConsole(m_sendername);

	//
	// Logs
	//
	// Logging functions are in a namespace and can be called directly
	// Enable console logging to detect Spout warnings and errors
	// A title for the console window can be specified.
	// Default is executable name.log
	// EnableSpoutLog(); 
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
	// available with a standard MessageBox.
	//
	// Enhancements include :
	//   Custom icon
	//   Modeless mode
	//   Centre on the application window or desktop
	//   Message with variable arguments
	//   Text entry edit control
	//   Combobox item selection control
	//   Timeout option
	//   Multiple buttons
	//   Hyperlinks anywhere in the message text
	//
	// See details in the Sender > Utilities example.
	// Practical examples can be found here for sender name and format.
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
	//
	// Sender format
	//
	// Set the sender application starting OpenGL format
	//
	// Default DirectX format is DXGI_FORMAT_B8G8R8A8_UNORM
	// Default OpenGL format is GL_RGBA
	//
	//       OpenGL                             Compatible DX11 format
	//       GL_RGBA       8 bit                (DXGI_FORMAT_B8G8R8A8_UNORM) (default)
	//       GL_RGBA8      8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//       GL_RGB10_A2  10 bit 2 bit alpha	(DXGI_FORMAT_R10G10B10A2_UNORM)
	//       GL_RGBA16    16 bit				(DXGI_FORMAT_R16G16B16A16_UNORM)			
	//       GL_RGBA16F   16 bit float			(DXGI_FORMAT_R16G16B16A16_FLOAT)
	//       GL_RGBA32F   32 bit float			(DXGI_FORMAT_R32G32B32A32_FLOAT)
	//
	// The starting texture format can be set here and changed while 
	// the program is running with right mouse click or the 'F2' key. 
	// Note that some applications may not receive formats other than RGBA.
	//
	// OpenGL fornat             Value
	// m_glformat = GL_RGBA;     // 0x1908
	// m_glformat = GL_RGBA8;    // 0x8058
	// m_glformat = GL_RGB10_A2; // 0x8059
	// m_glformat = GL_RGBA16;   // 0x805B
	// m_glformat = GL_RGBA16F;  // 0x881A
	// m_glformat = GL_RGBA32F;  // 0x8814
	// 
	// RGB can be specified in this example and is used for for SendImage.
	// It is accepted by SendFbo and SendTexture and is translated to 
	// default DirectX format. See: spoutGL::DX11format(GLint glformat)
	// m_glformat = GL_RGB; // 0x1907 (DXGI_FORMAT_B8G8R8A8_UNORM)
	//
	// If the texture format is changed from default (GL_RGBA)
	// a compatible DirectX 11 shared texture format must be set
	// so that receivers get a texture with the same format.
	// sender.SetSenderFormat(sender.DX11format(m_glformat));
	//
	// See also OpenGL format in the graphics receiver exapmle
	//
	// ---------------------------------------------------------------------------

	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox.jpg"); // image for the cube texture

 	rotX = 0.0f;
	rotY = 0.0f;

	// This example uses the window size to demonstrate sender re-sizing.
	// If the application renders to an FBO, the sender size can be
	// independent of the window and would need to be defined and managed.

	// Create an fbo for texture transfers
	m_senderwidth  = ofGetWidth();
	m_senderheight = ofGetHeight();

	// Use fbo settings to specify the internal format set as above
	// See also keypress 'F2' for re-allocation with the selected format
	ofFboSettings settings;
	settings.textureTarget = GL_TEXTURE_2D;
	settings.width = m_senderwidth;
	settings.height = m_senderheight;
	settings.internalformat = m_glformat;
	settings.useDepth = true;
	myFbo.allocate(settings);

	// Create pixels optional SendImage
	myPixels.allocate(m_senderwidth, m_senderheight, OF_IMAGE_COLOR_ALPHA);

	// Starting value for sender fps display
	m_senderfps = GetRefreshRate();

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0);

	// All sending functions check the sender name and dimensions
	// and create or update the sender as necessary

	// For all sending functions other than SendFbo, include the ID of
	// the active framebuffer if one is currently bound.

	// For this example, draw 3D graphics demo into an fbo
	// - - - - - - - - - - - - - - - - - - - - - - - - - -
	myFbo.begin();
	// Clear the fbo to reset the background and depth buffer
	// Background alpha should be opaque for the receiver
	ofClear(10, 100, 140, 255);
	ofEnableDepthTest(); // enable depth comparisons
	ofPushMatrix();
	ofTranslate(myFbo.getWidth() / 2.0f, myFbo.getHeight() / 2.0f, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.4f * myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6f;
	rotY += 0.6f;
	ofDisableDepthTest(); // Or graphics draw fails

	//
	// Option 1 : Send fbo
	//
	//   o The fbo must be bound for read.
	//   o The invert option is false in this case because the 
	//     texture attached to the fbo is already flipped in y.
	//   o The fbo is allocated sender width, height and format
	//     A smaller part of the fbo can be used if necessary.
	//   See also Option 4 to send the default framebuffer.
	//
	sender.SendFbo(myFbo.getId(), m_senderwidth, m_senderheight, false);

	myFbo.end();
	// - - - - - - - - - - - - - - - - - - - - - - - - - -

	//
	// Option 2 : Send texture
	//
	// sender.SendTexture(myFbo.getTexture().getTextureData().textureID,
		// myFbo.getTexture().getTextureData().textureTarget,
		// m_senderwidth, m_senderheight, false);

	//
	// Option 3 : Send image pixels
	// RGBA, BGRA, RGB, BGR formats and unsigned char pixel data type are supported.
	//
	// Typically an application pixel buffer will be used.
	// For this example, the fbo texture is read to pixel data
	// and formats are limited to GL_RGBA or GL_RGB
	// myFbo.readToPixels(myPixels);
	// sender.SendImage(myPixels.getData(), m_senderwidth, m_senderheight, m_glformat);

	// - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Show the fbo result sized to the application window
	//
	myFbo.draw(0.0f, 0.0f, (float)ofGetWidth(), (float)ofGetHeight());
	//
	// - - - - - - - - - - - - - - - - - - - - - - - - - -


	//
	// Option 4 : Send default OpenGL framebuffer
	//
	// If width and height are 0, the viewport size is used.
	//
	// sender.SendFbo(0, 0, 0);
	//
	// If performance is affected by repeated calls to glGetIntegerv
	// to retrieve the viewport size, width and height can be passed in.
	// See also code comments in Spout::SendFbo
	//
	// sender.SendFbo(0, ofGetWidth(), ofGetHeight());
	//

	if (bShowInfo) {
		// Show what it's sending
		std::string str;
		if (sender.IsInitialized()) {
			ofSetColor(255);
			str = "Sending as : ";
			str += sender.GetName(); str += " (";
			str += ofToString(sender.GetWidth()); str += "x";
			str += ofToString(sender.GetHeight()); str += ")";
			// Sender OpenGL texture format description
			// Sender format is selected with F2
			// Default DirectX format is DXGI_FORMAT_B8G8R8A8_UNORM.
			// Corresponding OpenGL format is GL_RGBA.
			// If OpenGL name is GL_RGBA. Do not show.
			if (sender.GLDXformat() != GL_RGBA) {
				str += " - ";
				str += sender.GLformatName(sender.GLDXformat());
			}
			// str += sender.GLformatName(m_glformat);
			DrawString(str, 10, 20);
			// Show sender fps and framecount if available
			if (sender.GetFrame() > 0) {
				str = "fps ";
				// Average to stabilise fps display
				m_senderfps = m_senderfps*.85 + 0.15*sender.GetFps();
				// Round first or integer cast will truncate to the whole part
				str += ofToString((int)(round(m_senderfps)));
				str += " : frame  ";
				str += ofToString(sender.GetFrame());
				DrawString(str, 10, 40);
			}
			// Show more details if graphics is not texture share compatible
			if (!sender.IsGLDXready()) {
				// If Auto switching is allowed
				if (sender.GetAutoShare())
					str = "CPU share mode";
				else
					str = "Graphics not texture share compatible";
				DrawString(str, 10, 50);
				// Show the graphics adapter currently being used
				DrawString(sender.AdapterName(), 10, 65);
			}
		}
		else {
			str = "Sender not initialized";
			if (m_glformat != GL_RGBA)
				str += " - Format not compatible\n";
			DrawString(str, 10, 20);
		}

		// Keypress options
		str = "F1 - sender name : F2/Right click - sender format : Space - hide info";
		DrawString(str, 34, ofGetHeight()-16);

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
// Right mouse click for sender format selection
void ofApp::mousePressed(int x, int y, int button) {
	if (button == 2)
		SelectSenderFormat();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == ' ') // On-screen information
		bShowInfo = !bShowInfo;
	else if (key == OF_KEY_F1) // Sender name
		SetSenderName();
	else if (key == OF_KEY_F2) 	// Sender format
		SelectSenderFormat();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	// If the sending size matches the window size,
	// the fbo, texture or image should be updated here.
	if (w > 0 && h > 0) {
		m_senderwidth = w;
		m_senderheight = h;
		// Use settings to specify the internal format
		ofFboSettings settings;
		settings.textureTarget = GL_TEXTURE_2D;
		settings.width = m_senderwidth;
		settings.height = m_senderheight;
		settings.internalformat = m_glformat;
		settings.useDepth = true;
		myFbo.allocate(settings);
		myPixels.allocate(m_senderwidth, m_senderheight, OF_IMAGE_COLOR_ALPHA);
	}

}

//--------------------------------------------------------------
// Sender name entry dialog
void ofApp::SetSenderName()
{
	std::string strname = m_sendername;
	// SpoutMessageBox with edit control
	if (SpoutMessageBox(ofGetWin32Window(), NULL, "Enter a new sender name", MB_ICONINFORMATION | MB_OKCANCEL, strname) == IDOK) {
		if (strcmp(m_sendername, strname.c_str()) != 0) {
			// Release the current sender first
			sender.ReleaseSender();
			// Change to the user entered name
			strcpy_s(m_sendername, 256, strname.c_str());
			// SetSenderName handles duplicate names with "_1", "_2" etc. appended.
			sender.SetSenderName(m_sendername);
			// SendTexture looks after sender creation for the new name
		}
	}
}

//--------------------------------------------------------------
// Sender format selection list
void ofApp::SelectSenderFormat()
{
	// Items for the comobo box
	std::vector<std::string> items;
	items.push_back("Default");           // DXGI_FORMAT_B8G8R8A8_UNORM
	items.push_back("8 bit RGBA8");       // DXGI_FORMAT_R8G8B8A8_UNORM
	items.push_back("10 bit RGB A2");     // DXGI_FORMAT_R10G10B10A2_UNORM
	items.push_back("16 bit RGBA");       // DXGI_FORMAT_R16G16B16A16_UNORM
	items.push_back("16 bit RGBA float"); // DXGI_FORMAT_R16G16B16A16_FLOAT
	items.push_back("32 bit RGBA float"); // DXGI_FORMAT_R32G32B32A32_FLOAT
	items.push_back("8 bit RGB");         // DXGI_FORMAT_B8G8BRA8_UNORM
	int selected = 0;

	// Selected index to show in the comobo box
	switch (m_glformat) {
		case GL_RGBA:     selected = 0;	break;
		case GL_RGBA8:    selected = 1;	break;
		case GL_RGB10_A2: selected = 2; break;
		case GL_RGBA16:	  selected = 3; break;
		case GL_RGBA16F:  selected = 4; break;
		case GL_RGBA32F:  selected = 5; break;
		case GL_RGB:      selected = 6; break;
		default:          selected = 0; break;
	}

	// SpoutMessageBox with combobox returns the item index
	if (SpoutMessageBox(ofGetWin32Window(), NULL, "Select sender format", MB_ICONINFORMATION | MB_OKCANCEL, items, selected) == IDOK) {
		// Default output format
		m_glformat = GL_RGBA;
		switch (selected) {
			case 0: default: break;
			case 1:	m_glformat = GL_RGBA8;	  break;
			case 2:	m_glformat = GL_RGB10_A2; break;
			case 3:	m_glformat = GL_RGBA16;   break;
			case 4:	m_glformat = GL_RGBA16F;  break;
			case 5:	m_glformat = GL_RGBA32F;  break;
			case 6:	m_glformat = GL_RGB;      break;
		}

		// Release sender to re-start with the new format
		sender.ReleaseSender();

		// Set sender DirectX texture format
		sender.SetSenderFormat(sender.DX11format(m_glformat)); // Selected

		sender.SetSenderName(m_sendername); // Keep the same name

		// Re-allocate fbo for texture transfer.
		// Use settings here to specify the internal format
		// or the default is used 0x1908 (GL_RGBA)
		ofFboSettings settings;
		settings.textureTarget = GL_TEXTURE_2D;
		settings.width = m_senderwidth;
		settings.height = m_senderheight;
		settings.internalformat = m_glformat;
		settings.useDepth = true; // for cube draw
		myFbo.allocate(settings);
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
