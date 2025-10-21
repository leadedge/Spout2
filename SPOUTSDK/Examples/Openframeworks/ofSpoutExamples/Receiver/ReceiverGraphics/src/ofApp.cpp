/*

	Spout OpenFrameworks Receiver example

    Visual Studio using the Spout SDK

	Copyright (C) 2015-2025 Lynn Jarvis.

	Spout 2.007
	OpenFrameworks 11
	Visual Studio 2022

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

	//
	// ================ Options =================
	//

	// Console window for when a console is not available (see main.cpp)
	// In this example it is used to show a list of senders with left mouse click
	OpenSpoutConsole();

	// Logging (see sender example)
	// EnableSpoutLog(); // Spout logging to console

	// Optional - specify the sender to connect to.
	// The application will not connect to any other unless the user selects one.
	// If that sender closes, the application will wait for the nominated sender to open.
	// receiver.SetReceiverName("Spout Sender");

	//
	// Receiver texture format
	//
	// The receiving texture format can be set to that of the sender
	// after a texture has been received.
	// See receiver.IsUpdated() in "Draw()" below.
	//    receiver.GLDXformat();
	//

	//
	// Other options
	// Refer to the graphics sender example
	//

	// Pre-allocate a receiving texture for "Option 3"
	// This can be any size or format
	g_SenderWidth = 1280;
	g_SenderHeight = 720;
	myTexture.allocate(g_SenderWidth, g_SenderHeight, GL_RGBA);

	// ==========================================


	ofSetWindowTitle("Spout Graphics Receiver");
	ofBackground(0, 0, 0);

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// Set a custom icon
	std::string icopath = ofToDataPath("Spout.ico", true);
	HICON hIcon = reinterpret_cast<HICON>(LoadImageA(nullptr, icopath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
	SendMessage(ofGetWin32Window(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(ofGetWin32Window(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma
	LoadWindowsFont(myFont, "Verdana", 12);

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	//
	// ReceiveTexture or ReceiveImage connect to and receive from a sender
	// Optionally include the ID of an fbo if one is currently bound
	//
	// For successful receive, sender details can be retrieved with
	//		const char * GetSenderName();
	//		unsigned int GetSenderWidth();
	//		unsigned int GetSenderHeight();
	//		DWORD GetSenderFormat();
	//		double GetSenderFps();
	//		long GetSenderFrame();
	//
	// If receive succeeds, a sender is connected.
	// IsUpdated() returns true if the sender changes
	// and the receiving texture can be allocated to match.
	//
	// If receive fails, the sender has closed
	// Connection can be tested at any time with 
	//		bool IsConnected();
	//

	/*
	//
	// Option 1 : Receive pixel data
	//
	// Received pixel format can be GL_RGBA, GL_BGRA, GL_RGB or GL_BGR
	// Change it here for testing
	GLint glformat = GL_RGBA;
	if (receiver.ReceiveImage(myImage.getPixels().getData(), glformat)) {
		// Update the receiving image if the received size has changed
		if (receiver.IsUpdated()) {
			g_SenderWidth = receiver.GetSenderWidth();
			g_SenderHeight = receiver.GetSenderHeight();
			// Re-allocated myImage to a compatible format
			if (glformat == GL_RGB || glformat == GL_BGR)
				myImage.allocate(g_SenderWidth, g_SenderHeight, OF_IMAGE_COLOR);
			else
				myImage.allocate(g_SenderWidth, g_SenderHeight, OF_IMAGE_COLOR_ALPHA);
			return; // Return now because the image will empty
		}
		// ofImage update is necessary because the pixels have been changed
		myImage.update();
		myImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	*/

	//
	// Option 2 : Receive texture
	//
	if (receiver.ReceiveTexture(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {
		// Update the receiving texture if the sender name, size or format is different
		if (receiver.IsUpdated()) {
			
			//
			// Allocate or re-allocate the receiving texture
			//
			// o The width and height can be any value
			//   and the texture can be pre-allocated.
			//   Typically the sender dimensions are used
			//   and the texture is allocated here.
			// o The texture OpenGL format must be compatible with the
			//   sender DirectX shared texture format. It can be
			//   GL_RGBA, GL_RGBA16, GL_RGBA16F or GL_RGBA32F.
			//
			g_SenderWidth = receiver.GetSenderWidth();
			g_SenderHeight = receiver.GetSenderHeight();
			myTexture.allocate(g_SenderWidth, g_SenderHeight, receiver.GLDXformat());
			return; // Return now because the texture will empty
		}
		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
	}

	/*
	//
	// Option 3 : Receive to a pre-allocated OpenGL texture
	//
	// The sender texture is fitted to the receiving texture size
	//
	if (receiver.ReceiveTexture(myTexture.getTextureData().textureID, myTexture.getTextureData().textureTarget)) {
		//
		// IsUpdated() returns true if the sender has changed
		// It is optional for a pre-allocated texture.
		//
		myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	*/

	/*
	//
	// Option 4 : Receive an OpenGL shared texture to access directly.
	//
	// Only if compatible for GL/DX interop or else BindSharedTexture fails.
	// For this example, copy from the shared texture. For other applications
	// the texture binding may be used directly for rendering.
	//
	if (receiver.ReceiveTexture()) {

		// IsUpdated() returns true if the sender has changed
		// It is optional if accessing the sender texture directly
		// Here the receiving texture size is retrieved for on-screen display
		if (receiver.IsUpdated()) {
			g_SenderWidth  = receiver.GetSenderWidth();
			g_SenderHeight = receiver.GetSenderHeight();
		}
		// Bind to get access to the shared texture
		if (receiver.BindSharedTexture()) {
			// Copy from the shared texture 
			receiver.CopyTexture(receiver.GetSharedTextureID(), GL_TEXTURE_2D,
				myTexture.getTextureData().textureID,
				myTexture.getTextureData().textureTarget,
				g_SenderWidth, g_SenderHeight);
			// Draw the Openframeworks texture
			myTexture.draw(0, 0, ofGetWidth(), ofGetHeight());
			// Un-bind to release access to the shared texture
			receiver.UnBindSharedTexture();
		}
	}
	*/

	// On-screen display
	// Space bar to disable
	if(bShowInfo) showInfo();
	
}


//--------------------------------------------------------------
void ofApp::showInfo() {

	std::string str;
	ofSetColor(255, 255, 255);

	if(receiver.IsConnected()) {

		// Show sender details
		str = "Receiving [";
		str += receiver.GetSenderName(); // sender name
		str += "]  (";
		str += ofToString(receiver.GetSenderWidth()); str += "x";
		str += ofToString(receiver.GetSenderHeight());

		// Received texture OpenGL format
		// Default DirectX format is DXGI_FORMAT_B8G8R8A8_UNORM.
		// Corresponding OpenGL format is GL_RGBA.
		// If OpenGL name is GL_RGBA. Do not show.
		if (receiver.GLDXformat() != GL_RGBA) {
			str += " - ";
			str += receiver.GLformatName(receiver.GLDXformat());
		}
		str += ")";
		DrawString(str, 10, 20);

		// Is the receiver using CPU sharing ?
		if (receiver.GetCPUshare()) {
			str = "CPU share";
		}
		else {
			str = "Texture share";
			// Graphics can still be incompatible if the user
			// did not select "Auto" or "CPU" in SpoutSettings
			if (!receiver.IsGLDXready())
				str = "Graphics not compatible";
		}

		// Applications < 2.007 will return no frame count information
		// Frame counting can also be disabled in SpoutSettings
		if (receiver.GetSenderFrame() > 0) {
			str += " - fps ";
			str += ofToString((int)roundf(receiver.GetSenderFps()));
			str += " frame  ";
			str += ofToString(receiver.GetSenderFrame());
		}
		else {
			// Show Openframeworks fps
			str += " - fps : " + ofToString((int)roundf(ofGetFrameRate()));
		}
		DrawString(str, 10, 40);

		// Show the receiving resolution if different
		if (g_SenderWidth != receiver.GetSenderWidth()
			|| g_SenderHeight != receiver.GetSenderHeight()) {
				str = "Receiving resolution :  ";
				str += std::to_string(g_SenderWidth);
				str += "x";
				str += std::to_string(g_SenderHeight);
				DrawString(str, 10, 60);
		}
		str = "Left or Right click - select sender : Space - hide info";
		DrawString(str, ofGetWidth()/2 - 225, ofGetHeight() - 14);

	}
	else {
		str = "No sender detected";
		DrawString(str, 10, 20);
	}

	// Show more details if not OpenGL/DirectX compatible
	if (!receiver.IsGLDXready()) {
		if (receiver.GetAutoShare()) {
			// CPU share allowed (default)
			str = "CPU share receiver";
		}
		else {
			// CPU share disabled (program setting)
			str = "Graphics not texture share compatible";
		}
		DrawString(str, 10, 35);

		// Show current graphics adapter
		str = "Graphics adapter ";
		str += std::to_string(receiver.GetAdapter());
		str += " : ";
		str += receiver.AdapterName();
		DrawString(str, 10, 50);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	receiver.ReleaseReceiver();
}

//--------------------------------------------------------------
// Right mouse click for sender selection
void ofApp::mousePressed(int x, int y, int button){
	
	//
	// Select a sender
	//
	if(button == 2) {
		//
		// Option 1
		//
		// Activate the sender selection dialog
		//
		// "SpoutPanel" is used if previously opened or if
		// SpoutSettings has been run to establish it's path.
		// If not available, a SpoutMessageBox is used (see further below).
		//
		// SpoutPanel is centred at the cursor position or on the application 
		// window if the handle is passed in "SelectSender(ofGetWin32Window())".
		// Centre on the cursor is best for mouse button press. 
		// Centre on the window is more appropriate for a menu selection or key press.
		//
		receiver.SelectSender();
		return;


		//
		// Option 2
		//
		// Show a list for user selection.
		// This can be managed by the application.
		// In this example, a SpoutMessageBox is used.
		//
		std::vector<std::string> senderlist = receiver.GetSenderList();
		
		// Open a SpoutMessageBox for sender selection.
		// Show the MessageBox even if the list is empty.
		// The index "selected" can be passed in to SpoutMessageBox
		// and is used as the current combobox item.

		// First get the active sender index to pass in as currently selected
		int selected = 0;
		char sendername[256]{};
		if (receiver.GetActiveSender(sendername)) {
			selected = receiver.spout.sendernames.GetSenderIndex(sendername);
		}

		// Centre on the mouse click cursor position.
		// (for this function, x and y are client coordinates)
		POINT pt={};
		GetCursorPos(&pt);
		SpoutMessageBoxPosition(pt);
		
		// If a point is not specified, SpoutMessageBox is centred on :
		//   - the application window if the handle is passed in
		//   - the monitor for no window handle
		if (SpoutMessageBox(NULL, NULL, "Select sender", MB_OKCANCEL, senderlist, selected) == IDOK && !senderlist.empty()) {

			// Make the selected sender active
			// Receivers then detect this sender on first opening
			receiver.SetActiveSender(senderlist[selected].c_str());

			//
			// If SetReceiverName has been used to nominate the sender to receive from :
			//
			// Either :
			//     Re-set the receiving name for change to the active sender (the one just set as active).
			//         receiver.SetReceiverName();
			//
			// Or :
			//     Nominate the new selected sender for the receiver to connect to.
			//     If that sender closes, the application will wait for the same sender to open again.
			//     To reset the behaviour at some other point, call SetReceiverName() as above.
			//         receiver.SetReceiverName(senderlist[selected].c_str());
			//

			// Release the current receiver
			// Next time it connects with the active sender
			// or the sender nominated by SetReceiverName
			receiver.ReleaseReceiver();
		}
	}

	// Left mouse button to show a sender list in the console.
	if (button == 0) {
		// Clear the console
		system("cls");
		// Show the user the current sender list
		std::vector<std::string> senderlist = receiver.GetSenderList();
		if (!senderlist.empty()) {
			printf("\n");
			for (int i = 0; i <(int)senderlist.size(); i++) {
				printf("(%d) [%s]\n", i, senderlist[i].c_str());
			}
			printf("Press number to detect\n");
		}
		else {
			printf("No senders\n");
		}
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	// Show on-screen info
	if (key == ' ') {
		bShowInfo = !bShowInfo;
	}

	// Keypress for sender selection from a list
	// Convert ASCII to number
	int index = key - 48;
	// Single key selection (0-9)
	if (index >= 0 && index <= 9) {
		char SenderName[256];
		// Check if the sender exists
		if (receiver.GetSender(index, SenderName)) {
			// Set as active
			receiver.SetActiveSender(SenderName);
			// Change to the active sender on the next receive
			receiver.ReleaseReceiver();
		}
		else {
			printf("sender index [%d] not found\n", index);
		}
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
