/*

	Spout OpenFrameworks Webcam Sender example
    With modifications for SpoutCam

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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================
*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);

	// Open a console to show webcam details
	OpenSpoutConsole();
	// Option - show Spout logs
	// EnableSpoutLog();

	camsendername = "Spout Webcam Sender"; // Set the sender name
	ofSetWindowTitle(camsendername); // show it on the title bar

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// Load a Windows truetype font to avoid dependency on a font file.
	// Arial, Verdana, Tahoma etc. in C:\Windows\Fonts
	LoadWindowsFont(myFont, "Verdana", 12);

	// Get the webcam list into a vector so that SpoutCam
	// can be identified by name and detected later
	camdevices = vidGrabber.listDevices();

	// Use the default webcam (0) or change as required
	camindex = 0;
	vidGrabber.setDeviceID(camindex);

	// Try to set this frame rate
	// (for SpoutCam use SpoutCamSettings)
	vidGrabber.setDesiredFrameRate(30);

	// Give the sender the same name as the window title
	// If none is specified, the executable name is used
	camsender.SetSenderName(camsendername.c_str());

	// Start the webcam
	if (camdevices.size() > 0)
		SetWebcam(camindex);

}


//--------------------------------------------------------------
void ofApp::update() {

	if (!vidGrabber.isInitialized())
		return;

	vidGrabber.update();

	// SpoutCam is a receiver and connects to the active sender when it starts.
	// To avoid feedback from this application, wait until another sender is running.
	bool bSendCam = true;
	if (camdevices[camindex].deviceName == "SpoutCam") {
		if(camsender.GetSenderCount() == 0) {
			// Nothing running - do not send
			bSendCam = false;
		}
		else if (camsender.GetSenderCount() == 1) {
			// Only one sender left
			if (camsender.IsInitialized()) {
				camsender.ReleaseSender();
				bSendCam = false;
			}
			// Give Spoutcam a frame time to refresh buffers
			Sleep(ofGetFrameRate());
		}
	}

	if (bSendCam) {
		// If the webcam texture is sent when the frame is new
		// and Frame count is enabled in SpoutSettings,
		// receivers will detect the webcam fps.
		if (vidGrabber.isFrameNew()) {
			// Send the webcam texture
			// A sender is created and updated for size changes
			camsender.SendTexture(vidGrabber.getTexture().getTextureData().textureID,
				vidGrabber.getTexture().getTextureData().textureTarget,
				(unsigned int)vidGrabber.getWidth(),
				(unsigned int)vidGrabber.getHeight(), false);
		}
	}

}


//--------------------------------------------------------------
void ofApp::draw() {

	std::string str;
	ofSetColor(255);

	if (vidGrabber.isInitialized())
		vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Show what it's sending
	if (camsender.IsInitialized()) {
		str = "Sending as : ";
		str += camsender.GetName(); str += " (";
		str += ofToString(camsender.GetWidth()); str += "x";
		str += ofToString(camsender.GetHeight()); str += ")";
		// Show sender fps and framecount if they are selected in SpoutSettings
		if (camsender.GetFrame() > 0) {
			str += " fps: ";
			str += ofToString((int)roundf(camsender.GetFps())); str += " frame : ";
			str += ofToString(camsender.GetFrame());
		}
		else {
			// Show Openframeworks fps
			str += "fps : " + ofToString((int)roundf(ofGetFrameRate()));
		}
		DrawString(str, 20, 30);
	}

	// Show the webcam list for selection
	int ypos = 60;
	for (int i=0; i<(int)camdevices.size(); i++) {
		str = "("; str+= ofToString(i); str += ") ";
		str += camdevices[i].deviceName;
		DrawString(str, 40, ypos);
		ypos += 22;
	}

	if(camdevices.size() > 1) {
		str = "Select a webcam  - right click or press 0 to ";
		str += ofToString(camdevices.size() - 1);
	}
	else if (camdevices.size() == 1) {
		str = "Select a webcam  - right click or press 0";
	}
	else {
		str = "Mo webcams available";
	}
	DrawString(str, 40, ofGetHeight()-30);

	// Spoutcam properties
	if (camdevices[camindex].deviceName == "SpoutCam") {
		str = "Middle click for SpoutCam properties";
		DrawString(str, 40, ofGetHeight() - 10);
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	camsender.ReleaseSender(); 
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	int i = key - 48; // Decimal number
	if (key == 32) {
		camdevices = vidGrabber.listDevices();
		printf("Select a webcam by it's index (0-%d)\n", (int)camdevices.size() - 1);
	}
	else if (i >= 0 && i < (int)camdevices.size()) {
		// Change if the user selected a different webcam
		if (i != camindex)
			SetWebcam(i);
	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	if (button == 2) { // Right button
		if (!camdevices.empty()) {
			// List the webcams for combo box selection
			std::vector<std::string> cams;
			for (size_t i = 0; i < camdevices.size(); i++) {
				cams.push_back(camdevices[i].deviceName);
			}
			int index = camindex;
			if (SpoutMessageBox(NULL, "", "Select a webcam", MB_OKCANCEL, cams, index) == IDOK) {
				// Change if the user selected a different webcam
				if (index != camindex)
					SetWebcam(index);
			}
		}
	}

	// Middle button for SpoutCam properties
	if (button == 1) {
		if (!camdevices.empty() && camdevices[camindex].deviceName == "SpoutCam") {
			// Bring up the SpoutCam property page
			// system("C:/Windows/System32/rundll32.exe SpoutCam64.ax, Configure");
			char path[MAX_PATH]{};
			if (ReadPathFromRegistry(HKEY_CLASSES_ROOT, "CLSID\\{8E14549A-DB61-4309-AFA1-3578E927E933}\\InprocServer32", "", path)) {
				std::string str = "C:/Windows/System32/rundll32.exe ";
				str += path;
				str += ", Configure";

				// The dialog will block
				system(str.c_str());

				// Restart SpoutCam with the changed settings
				SetWebcam(camindex);

			}
		}
	}

}

//--------------------------------------------------------------
// Set up a webcam
bool ofApp::SetWebcam(int index)
{
	if (index > camdevices.size())
		return false;

	// Release the sender before changing webcams or a
	// switch to SpoutCam will pick up this sender application
	camsender.ReleaseSender();
	// Set the sender name again
	camsender.SetSenderName(camsendername.c_str());

	// Set the webcam from the index
	camindex = index;
	vidGrabber.close();
	camindex = index;
	vidGrabber.setDeviceID(camindex);
	vidGrabber.setDesiredFrameRate(30); // Default fps
	vidGrabber.setUseTexture(true);

	// Default resolution
	unsigned int camwidth  = 640;
	unsigned int camheight = 480;

	// Look in the registry for SpoutCam resolutions
	if (camdevices[camindex].deviceName == "SpoutCam") {
		// Active sender 0 (default)
		DWORD mode = 0;
		if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "resolution", &mode)) {
			// Possible resolutions set by SpoutCamSettings
			std::vector<unsigned int> widths  = { 0, 320, 640, 640, 800, 1024, 1024, 1280, 1280, 1280, 1920 };
			std::vector<unsigned int> heights = { 0, 240, 360, 480, 600, 720, 768, 720, 960, 1024, 1080 };
			// 0 means the active sender resolution
			if (widths[mode] == 0) {
				char sendername[256] {};
				if (camsender.GetActiveSender(sendername)) {
					// Set camwidth/camheight only if the sender is found
					HANDLE dxShareHandle = nullptr;
					DWORD dwFormat = 0;
					camsender.GetSenderInfo(sendername, camwidth, camheight, dxShareHandle, dwFormat);
				}
			}
			// Get SpoutCam fps
			DWORD fpsMode = 0;
			if(ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "fps", &fpsMode)) {
				std::vector<int> fps = { 10, 15, 25, 30, 50, 60 };
				int camfps = fps[fpsMode];
				vidGrabber.setDesiredFrameRate(camfps); // Spoutcam fps
			}
		}
	}

	// Set up the grabber at default or SpoutCam resolution
	if (vidGrabber.setup(camwidth, camheight)) {

		// Set window maximum width to 800
		int width = vidGrabber.getWidth();
		if (width > 800) width = 800;

		// Adjust window height to the aspect ratio of the webcam
		int height = width * camheight / camwidth;
		ofSetWindowShape(width, height);

		// Centre on the desktop
		int xpos = (ofGetScreenWidth() - width) / 2;
		int ypos = (ofGetScreenHeight() - height) / 2;
		ofSetWindowPosition(xpos, ypos);

		printf("Initialized webcam [%s] (%d x %d)",
			camdevices[camindex].deviceName.c_str(),
			(int)vidGrabber.getWidth(), (int)vidGrabber.getHeight());

	}
	else {
		printf("Webcam setup error. Try a different one.\n");
		return false;
	}

	return true;
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
