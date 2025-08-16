/*

	Spout OpenFrameworks Spout Utilities example

	Examples of utilities in the Spout library "SpoutUtils"
	The number of functions is too great to include
	documentation in the other sender examples

	'F1' - sender name, 'F2' - sender format and are practical examples

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

	// OpenSpoutConsole(); // Empty console for debugging

 	strcpy_s(sendername, 256, "Spout Utilities"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar

	// Don't centre on the screen to demonstrate messagebox
	// window centre postioning using spoutMessageBoxWindow

	// Load an Information icon from imageres.dll
	// https://renenyffenegger.ch/development/Windows/PowerShell/examples/WinAPI/ExtractIconEx/imageres.html
	HICON hIconBig = nullptr;
	HICON hIconSmall = nullptr;
	ExtractIconExA("%SystemRoot%\\system32\\imageres.dll", 76, &hIconBig, &hIconSmall, 1);
	HWND hwnd = ofGetWin32Window();
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

	// Load a Windows truetype font for larger on-screen display
	// Arial 13, Verdana 12, Tahoma 12
	LoadWindowsFont(myFont, "Tahoma", 12);

	// ==========================================================
	//
	// SpoutMessageBox
	//
	// SpoutMessageBox is an enhanced MessageBox using "TaskDialogIndirect"
	// that can be used as a direct replacement for the Win32 MessageBox function
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
	//   Custom icon
	//    Icons can be specifed using MessageBox definitions MB_ICONWARNING, MB_ICONINFORMATION etc
	//    A custom icon can also be specified using :
	//      SpoutMessageBoxIcon(HICON hIcon)
	//      SpoutMessageBoxIcon(std::string iconfile)
	//    Icon files must be of the type ".ico"
	//    A full path to the icon file is required.
	// std::string iconfile = GetExePath() + "data\\Spout.ico";
	// SpoutMessageBoxIcon(iconfile);
	//
	//   Modeless mode
	//     A typical MessageBox or TaskDialog is modal and stops the appication until return.
	//     This is undesirable if the messagebox originates from a dll.
	//     Modeless mode forwards the information on to "SpoutPanel" which is
	//     an independent process and does not affect the host application.
	//     In this example, the application window shows and the cube continues to rotate.
	//     SpoutPanel version 2.017 or greater is required.
	//     Spout must have been configured using SpoutSettings.
	//       SpoutMessageBoxModeless(bool bMode = true)
	// SpoutMessageBoxModeless();
	//
	//   Centre on the application window or the desktop
	//    o Pass the window handle of the current application to centre on the window.
	//    o Pass NULL as the window handle to centre on the desktop
	//      SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption, UINT uType, DWORD dwMilliseconds = 0)
	//    Specify the window handle for SpoutMessageBox functions that do not include one
	//      HWND hWnd = ofGetWin32Window();
	//      SpoutMessageBoxWindow(hWnd);
	//
	//   Message with variable arguments
	//    Presents text equivalent to that of "printf"
	//      SpoutMessageBox(const char * caption, const char* format, ...)
	// SpoutMessageBox("ofExample", "Spout SDK version %s", GetSDKversion().c_str());
	//
	// MessageBox dialog with an edit control for text input
	// Can be used in place of a specific application resource dialog
	//   o For message content, the control is in the footer area
	//   o If no message, the control is in the main content area
	//   o All SpoutMessageBox functions such as user icon and buttons are available
	// std::string text;
	// if (SpoutMessageBox(NULL, NULL, "Simple text entry", MB_OKCANCEL, text) == IDOK) {
	// 	SpoutMessageBox("Text entry", "%s\n", text.c_str());
	// }
	// if (SpoutMessageBox(NULL, "This is a text entry dialog\nEnter the text in the edit\ncontrol in the footer area\n", "Message text entry", MB_ICONINFORMATION | MB_OKCANCEL, text) == IDOK) {
	// 	SpoutMessageBox("Message text entry", "%s\n", text.c_str());
	// }
	//
	// MessageBox dialog with a combobox control for item selection
	// Properties the same as the edit control
	// std::vector<std::string>items;
	// items.push_back("Item 1");
	// items.push_back("Item 2");
	// items.push_back("Item 3");
	// items.push_back("Item 4");
	// int index = 0;
	// if (SpoutMessageBox(NULL, NULL, "Select item", MB_OKCANCEL, items, index) == IDOK) {
	// 	SpoutMessageBox("Item selection", "%d (%s)\n", index, items[index].c_str());
	// }
	// if (SpoutMessageBox(NULL, "This is an item selection dialog\nSelect an item from the combobox\ncontrol in the footer area\n", "Message item selection", MB_ICONINFORMATION | MB_OKCANCEL, items, index) == IDOK) {
	// 	SpoutMessageBox("Message item selection", "%d (%s)\n", index, items[index].c_str());
	// }
	//
	//   Timeout option
	//    The messagebox closes after the specified interval in millseconds.
	//	    SpoutMessageBox(const char* message, DWORD dwMilliseconds = 0)
	//      SpoutMessageBox(HWND hWnd, const char* message, const char* caption, UINT uType, DWORD dwMilliseconds = 0)
	// SpoutMessageBox("message", 2000);
	//
	//   Multiple buttons
	//    Any number of buttons can be specified.
	//    The SpoutMessageBox function returns the ID of the chosen button.
	//    The messagebox is modal to allow return of the user choice.
	//      void SpoutMessageBoxButton(int ID, std::wstring title)
	// SpoutMessageBoxButton(1000, L"Button 1");
	// SpoutMessageBoxButton(2000, L"Button 2");
	// SpoutMessageBoxButton(3000, L"Button 3");
	// int buttonid = SpoutMessageBox(NULL, "Select a button", "ofExample", MB_OK);
	// if (buttonid == 1000) SpoutMessageBox("Button 1");
	// if (buttonid == 2000) SpoutMessageBox("Button 2\n");
	// if (buttonid == 3000) SpoutMessageBox("Button 3\n");
	// if (buttonid == 1) SpoutMessageBox("OK pressed\n");
	//
	//   Hyperlinks anywhere in the message text
	//    Urls can be included in the content using HTML format.
	//    Only double quotes are supported and must be escaped.
	// SpoutMessageBox("<a href=\"https://spout.zeal.co/\">Spout home page</a>");
	//
	// This application shows practical examples of each function
	// Source code can be found in "doMessagebox"
//
	// ==========================================================

	//
	// Other options
	//
	// Refer to the "SpoutUtils.cpp" source code for documentation.
	//     Spout SDK version number
	//     Registry utilities
	//     Computer information
	//     Timing utilities

	// ---------------------------------------------------------------------------
	//
	// Sender format
	//
	// Set the sender application starting OpenGL format
	//
	// Default DirectX format is DXGI_FORMAT_B8G8R8A8_UNORM
	// Default OpenGL format is GL_RGBA
	glFormat = GL_RGBA;
	//
	//
	//       OpenGL                             Compatible DX11 format
	//       GL_RGBA16    16 bit				(DXGI_FORMAT_R16G16B16A16_UNORM)			
	//       GL_RGBA16F   16 bit float			(DXGI_FORMAT_R16G16B16A16_FLOAT)
	//       GL_RGBA32F   32 bit float			(DXGI_FORMAT_R32G32B32A32_FLOAT)
	//       GL_RGB10_A2  10 bit 2 bit alpha	(DXGI_FORMAT_R10G10B10A2_UNORM)
	//       GL_RGBA8      8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//       GL_RGBA       8 bit                (DXGI_FORMAT_R8G8B8A8_UNORM)
	//
	// The starting format can be changed here or later by using the 'F1' key
	// Note that some applications may not receive other formats.
	// Only RGB or RGBA is supported for SendImage and for Openframeworks pixels.
	//
	// glFormat = GL_RGBA16; // Example 16 bit rgba
	//
	// A compatible DirectX 11 shared texture format must be set
	// so that receivers get a texture with the same format.
	//
	// sender.SetSenderFormat(sender.DX11format(glFormat));
	//
	// See also OpenGL format the graphics receiver exapmle
	//
	// ----------------------------------------------

	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox.jpg"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// Create an fbo for texture transfers
	senderwidth = ofGetWidth();
	senderheight = ofGetHeight();
	myFbo.allocate(senderwidth, senderheight, glFormat);

	// Give the sender a name
	sender.SetSenderName(sendername);

	// Update caption with the produced name
	// in case of multiples of the same sender
	// (see SetSenderName)
	ofSetWindowTitle(sender.GetName());

	// Starting value for sender fps display
	g_SenderFps = GetRefreshRate();

	// Add menu buttons
	AddButtons();

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0);

	// Draw 3D graphics demo into the fbo
	myFbo.begin();

	// Clear the fbo to reset the background and depth buffer
	// Background alpha should be opaque for the receiver
	ofClear(10, 100, 140, 255);

	ofEnableDepthTest(); // enable depth comparisons
	ofPushMatrix();
	ofTranslate(myFbo.getWidth()/2.0f, myFbo.getHeight()/2.0f, 0);
	ofRotateYDeg(rotX); // rotate
	ofRotateXDeg(rotY);
	myBoxImage.bind(); // bind our box face image
	ofDrawBox(0.45f*myFbo.getHeight()); // draw the box
	myBoxImage.unbind();
	ofPopMatrix();
	rotX += 0.6f;
	rotY += 0.6f;

	// This is necessary or Truetype font draw fails
	ofDisableDepthTest();
	
	// Send fbo
	sender.SendFbo(myFbo.getId(), senderwidth, senderheight, false);

	myFbo.end();

	// Show the result sized to the application window
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	if (bShowInfo) {
		// Show what it's sending
		std::string str;
		if (sender.IsInitialized()) {
			ofSetColor(255);
			str = "Sending as : ";
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
				str += "  fps ";
				// Average to stabilise fps display
				g_SenderFps = g_SenderFps*.85 + 0.15*sender.GetFps();
				// Round first or integer cast will truncate to the whole part
				str += ofToString((int)(round(g_SenderFps)));
				str += " : frame  ";
				str += ofToString(sender.GetFrame());
			}
			DrawString(str, 10, 20);

			// Show options to change sender name and format at the bottom of the window
			str = "'F1' - sender name : 'F2' - sender format : Space - hide info";
			DrawString(str, 170, ofGetHeight()-14);

			//
			// Options for SpoutMessageBox examples
			// See "doMessageBox" for source code.
			//

			// Draw menu buttons
			DrawButtons();
		}
		else {
			str = "Sender not initialized\n";
			DrawString(str, 10, 20);
		}
	}

}

void ofApp::mouseMoved(int x, int y)
{
	mousex = x;
	mousey = y;
}

void ofApp::mousePressed(int x, int y, int button)
{
	if (!bMessagebox) {
		// Which button was pressed
		for (size_t i = 0; i < buttons.size(); i++) {
			if (x > buttons[i].x
				&& x < (buttons[i].x + buttons[i].width)
				&& y > buttons[i].y
				&& y < (buttons[i].y + buttons[i].height)) {
				doMessagebox(buttons[i].title);
			}
		}
	}

}

//--------------------------------------------------------------
void ofApp::exit() {

	// Close the sender on exit
	sender.ReleaseSender();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	// Show/Hide on-screen information
	if (key == ' ') {
		bShowInfo = !bShowInfo;
	}

	// Change sender name
	if (key == OF_KEY_F1) {
		// Spout icon
		std::string iconfile = GetExePath() + "data\\Spout.ico";
		SpoutMessageBoxIcon(iconfile);
		// Centre dialog on the window instead of the desktop
		SpoutMessageBoxWindow(ofGetWin32Window());
		// SpoutMessageBox with edit control
		std::string strname = sendername; // existing entry
		if (SpoutMessageBox(NULL, NULL, "Enter sender name", MB_USERICON | MB_OKCANCEL, strname) == IDOK) {
			if (strcmp(sendername, strname.c_str()) != 0) {
				// Change to the user entered name
				strcpy_s(sendername, 256, strname.c_str());
				// Release the current sender
				// SendTexture looks after sender creation for the new name
				sender.ReleaseSender();
				// SetSenderName handles duplicate names with "_1", "_2" etc. appended.
				sender.SetSenderName(sendername);
			}
		}
		// Default
		SpoutMessageBoxWindow(NULL);
	}

	// Change sender format
	if (key == OF_KEY_F2) {
		std::vector<std::string> items;
		// Items for the comobo box
		// "Default", "8 bit RGBA", "16 bit RGBA", "16 bit RGBA float", "32 bit RGBA float"
		items.push_back("Default"); // DXGI_FORMAT_B8G8R8A8_UNORM
		items.push_back("8 bit RGBA"); // DXGI_FORMAT_R8G8B8A8_UNORM
		items.push_back("16 bit RGBA");
		items.push_back("16 bit RGBA float");
		items.push_back("32 bit RGBA float");
		int selected = 0;
		// Selected index to show in the comobo box
		if (sender.GetDX11format() == DXGI_FORMAT_B8G8R8A8_UNORM) { // default
			selected = 0;
		}
		else {
			switch (glFormat) {
				case GL_RGBA:    selected = 1;	break;
				case GL_RGBA16:	 selected = 2; break;
				case GL_RGBA16F: selected = 3; break;
				case GL_RGBA32F: selected = 4; break;
				default: selected = 0; break;
			}
		}
		SpoutMessageBoxWindow(ofGetWin32Window());
		// SpoutMessageBox with combobox returns the item index
		if (SpoutMessageBox(NULL, NULL, "Sender format", MB_ICONINFORMATION | MB_OKCANCEL, items, selected) == IDOK) {
			switch (selected) {
				// Default DirectX output format
				case 1:	glFormat = GL_RGBA;	break;
				case 2:	glFormat = GL_RGBA16; break;
				case 3:	glFormat = GL_RGBA16F; break;
				case 4:	glFormat = GL_RGBA32F; break;
				case 0: default: glFormat = GL_RGBA; break; // Changed below
			}
			// Release sendeer to re-start with the new format
			sender.ReleaseSender();
			// Set sender DirectX texture format
			if (selected == 0)
				sender.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM); // Default
			else
				sender.SetSenderFormat(sender.DX11format(glFormat)); // Selected
			sender.SetSenderName(sendername); // Keep the same name
			// Re-allocate fbo for texture transfer
			myFbo.allocate(senderwidth, senderheight, glFormat);
		}
		SpoutMessageBoxWindow(NULL);
	}

}

//
// SpoutMessageBox examples
//
void ofApp::doMessagebox(std::string title)
{
	// Prevent mouse item selection while a messagebox is open
	bMessagebox = true;

	//
	// " About
	//
	// Practical example of an About box.
	// Typically created using resources for Windows programs
	//
	if (title == "About") {
		std::string str = "Utility functions of the Spout library\n";
		str += "Examples can be tested here and the source provides example code.\n\n";
		str += "spoutMessageBox\n";
		str += "    An enhanced ";
		str += "<a href=\"https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox\">MessageBox</a>";
		str += " using ";
		str += "<a href=\"https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-taskdialogindirect\">TaskDialogIndirect</a>\n";
		str += "    with additional options for position, modeless, timeout,\n";
		str += "    hyperlinks, instruction, user icon, user buttons, text entry\n";
		str += "    and combobox list selection controls.\n\n";
		str += "Overload functions\n";
		str += "  o MessageBox with standard arguments with optional timeout\n";
		str += "    replaces an existing MessageBox call\n";
		str += "        SpoutMessageBox(HWND hwnd, LPCSTR message, LPCSTR caption,\n";
		str += "            UINT uType, DWORD dwTimeout = 0)\n";
		str += "  o MessageBox with standard arguments\n";
		str += "    including taskdialog main instruction large text\n";
		str += "        SpoutMessageBox(hwnd, message, caption, type, instruction, timeout = 0)\n";
		str += "  o MessageBox with optional timeout\n";
		str += "        SpoutMessageBox(message, timeout = 0)\n";
		str += "  o MessageBox with variable arguments\n";
		str += "        SpoutMessageBox(caption, const char* format, ...)\n";
		str += "  o Edit control\n";
		str += "    An edit control within the MessageBox to enter and return text.\n";
		str += "    If the edit string is not empty, it is shown highlighted in the edit control.\n";
		str += "        SpoutMessageBox(hwnd, message, caption, type, std::string& text)\n";
		str += "  o Combo box control\n";
		str += "    A combo box control within the MessageBox\n";
		str += "    to select from a list and return the selection index.\n";
		str += "        SpoutMessageBox(hwnd, message, caption, type,\n";
		str += "            std::vector<std::string> items, int& selected)\n\n";
		str += "Other functions\n";
		str += "  o Clipboard\n";
		str += "    Utility function to copy text to the clipboard\n";
		str += "        bool CopyToClipBoard(HWND hwnd, const char* caps);\n\n";
		str += "Refer to SpoutUtils.cpp for more utilites\n";
		str += "Information, Console, Logging, Registry, Timing\n\n";
		str += "                                    <a href=\"https://spout.zeal.co\">https://spout.zeal.co</a>\n\n";
		std::string iconfile = GetExePath() + "data\\Spout.ico";
		SpoutMessageBoxIcon(iconfile);
		SpoutMessageBox(NULL, str.c_str(), "About", MB_TOPMOST | MB_OK);
	}

	//
	// Window position
	//
	// SpoutMessageBox opens centred on the desktop by default.
	// Some functions have a window handle argument which can be 
	// used to centre on the application window.
	// SpoutMessageBoxWindow can be used to provide a window handle
	// for those functions that do not include one. This setting also
	// applies for all subsequent SpoutMessageBox functions.
	//     spoutMessageBoxWindow(HWND hwnd)
	// A null handle returns to default centre on the desktop
	//
	if (title == "Position") {
		std::string str = "SpoutMessageBox opens centred on the application window\n";
		str += "if a handle argument is passed in, or on the desktop for a null\n";
		str += "window handle.\n\n";
		str += "\"SpoutMessageBoxWindow\" provides a window handle\n";
		str += "in place of the window handle argument, and all functions\n";
		str += "centre on that window thereafter. A null handle returns to\n";
		str += "the position depending on the window handle passed in.\n\n";
		str += "    spoutMessageBoxWindow(HWND hwnd)\n\n";
		str += "Click \"Window\" to center messages on the window\n";
		str += "Click \"Desktop\" to center on the desktop\n";
		str += "Click \"OK\" for no change\n\n";
		str += "If the application window is centred on the desktop now,\n";
		str += "move it to one side so the effect can be more easily seen.\n";
		SpoutMessageBoxButton(1000, L"Window");
		SpoutMessageBoxButton(2000, L"Desktop");
		int iret = SpoutMessageBox(NULL, str.c_str(), "Position", MB_TOPMOST | MB_OK);
		if (iret == 1000) { // Window centre
			SpoutMessageBoxWindow(ofGetWin32Window());
			SpoutMessageBox("Messages will be centred on the application window");
		}
		else if (iret == 2000) { // Desktop centre
			SpoutMessageBoxWindow(NULL);
			SpoutMessageBox("Messages will be centred on the  desktop");
		}
	}

	//
	// Simple messagebox with message and optional timeout
	// The dialog closes itself if a timeout is specified.
	//
	if (title == "Simple") {
		std::string str = "Simple messagebox with message and optional millisecond timeout\n";
		str += "The dialog closes itself if a timeout is specified\n\n";
		str += "  SpoutMessageBox(const char * message, DWORD dwTimeout = 0)\n\n";
		str += " For example  :  SpoutMessageBox(\"Simple messagebox\")\n\n";
		str += "Click OK to show this example\n";
		if(SpoutMessageBox(NULL, str.c_str(), "Simple messagebox", MB_OKCANCEL) == IDOK)
			SpoutMessageBox("Simple messagebox");
	}

	//
	// MessageBox with variable arguments
	//
	if (title == "Variable") {
		std::string str = "Variable arguments provide a replacement for \"printf\"\n";
		str += "which avoids having to open a console.\n";
		str += "Useful for debugging and tracing errors\n";
		str += "Icon and buttons can also be specified\n\n";
		str += "  int SpoutMessageBox(const char * caption, const char* format, ...);\n";
		str += "  int SpoutMessageBox(const char * caption,  UINT uType, const char* format, ...);\n\n";
		str += "For example :\n";
		str += "    float result = 123.0;\n";
		str += "    int error = 2;\n";
		str += "    char *errorstring = \"wrong\";\n";
		str += "    SpoutMessageBox(\"Error\", \"result %4.3f\\nerror = %d (%s)\", result, error, errorstring);\n\n";
		str += "Click OK to show this example\n";
		if (SpoutMessageBox(NULL, str.c_str(), "Variable arguments", MB_OKCANCEL) == IDOK) {
			float result = 123.0;
			int error = 2;
			const char* errorstring = "wrong";
			SpoutMessageBox("Something went wrong", "result = %4.3f\nerror = %d (%s)", result, error, errorstring);
		}
	}

	//
	// Messagebox with message, caption, and milliseconds timeout
	//
	if (title == "Timeout") {
		std::string str = "A timeout can be added to any of the spoutMessageBox functions\n";
		str += "except for a MessageBox with variable arguments\n\n";
		str += "Timeout is in millseconds and disabled for any MessageBox requiring user input.\n";
		str += "For this example, wait 10 seconds and the message will close.\n";
		SpoutMessageBox(NULL, str.c_str(), "Timeout after 10 seconds", MB_OK, "WAIT FOR 10 SECONDS", 10000);
	}

	//
	// MessageBox options
	//
	if (title == "Options") {
		std::string str = "Optional arguments are the same as for Windows ";
		str += "<a href=\"https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox\">MessageBox</a>\n";
		str += "but limited by translation to ";
		str += "<a href=\"https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-taskdialogindirect/\">TaskDialogIndirect</a>\n\n";
		str += "\"Type\" is buttons and icon combined such as, in this case\n";
		str += "   \"MB_TOPMOST | MB_ICONWARNING | MB_YESNO\"\n\n";
		str += "   o Buttons\n";
		str += "       MB_OK\n";
		str += "       MB_OKCANCEL\n";
		str += "       MB_YESNOCANCEL\n";
		str += "       MB_YESNO\n\n";
		str += "   o Topmost\n";
		str += "       MB_TOPMOST\n\n";
		str += "   o Icon\n";
		str += "       MB_ICONERROR\n";
		str += "       MB_ICONWARNING\n";
		str += "       MB_ICONINFORMATION\n\n";
		str += "   o Return (button pressed)\n";
		str += "       IDOK\n";
		str += "       IDCANCEL\n";
		str += "       IDYES\n";
		str += "       IDNO\n\n";
		str += "In this example return is either IDYES, IDNO or IDCANCEL\n";
		str += "Click Yes or No to see the result.\n\n";
		int iRet = SpoutMessageBox(NULL, str.c_str(), "Caption, message icon and type", MB_TOPMOST | MB_ICONWARNING | MB_YESNOCANCEL);
		if (iRet == IDYES) SpoutMessageBox("IDYES");
		if (iRet == IDNO) SpoutMessageBox("IDNO");
	}

	//
	// MessageBox with message, caption, type, instruction
	//
	if (title == "Instruction") {
		std::string str = "MessageBox with message, caption, type and instruction\n\n";
		str += "The main instruction is a special heading in large blue font\n";
		str += "above the message and is useful to draw attention to the content\n";
		str += "Use the long form of the MessageBox and include an instruction\n\n";
		str += "spoutMessageBox(NULL, \"Message\", \"Caption\", MB_OK, \"Instruction\")\n\n";
		SpoutMessageBox(NULL, str.c_str(), "Main instruction", MB_ICONINFORMATION | MB_OK, "Instruction");
	}

	//
	// Custom icon
	//
	// A full path to an icon file (.ico) is required
	//
	if (title == "Icon") {
		std::string iconfile = GetExePath() + "data\\Spout.ico";
		SpoutMessageBoxIcon(iconfile);
		std::string str = "A custom icon can be loaded from an icon \".ico\" image file:\n";
		str += "A full path to the icon file is required.\n\n";
		str += "    std::string iconfile = GetExePath() + \"data\\Spout.ico\"\n";
		str += "    SpoutMessageBoxIcon(iconfile)\n\n";
		str += "An icon can also be loaded using an icon handle.\n";
		str += "The handle can be obtained from resources\n";
		str += "if the icon is defined in a resource file (IDI_ICON1)\n\n";
		str += "    LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1));\n";
		SpoutMessageBox(NULL, str.c_str(), "Custom Icon", MB_OK);
	}

	//
	// Custom buttons
	//
	// The button ID is returned by the messagebox
	//
	if (title == "Buttons") {
		std::string str = "As well as OK / CANCEL etc, multiple user buttons can be added.\n";
		str += "Each button has a unique ID and button text. The text is a wide string.\n";
		str += "The ID is returned and can be used to choose the required action.\n\n";
		str += "      SpoutMessageBoxButton(1000, L\"Button 1\");\n";
		str += "      SpoutMessageBoxButton(2000, L\"Button 2\");\n";
		str += "      SpoutMessageBoxButton(3000, L\"Button 3\");\n";
		str += "      int iret = SpoutMessageBox(\"User buttons\", \"Choose a button\");\n";
		str += "      if(iret == 1000) SpoutMessageBox(\"Button 1 pressed\");\n";
		str += "      if(iret == 2000) SpoutMessageBox(\"Button 2 pressed\");\n";
		str += "      if(iret == 3000) SpoutMessageBox(\"Button 3 pressed\");\n\n";
		str += "Choose a button\n";
		SpoutMessageBoxButton(1000, L"Button 1");
		SpoutMessageBoxButton(2000, L"Button 2");
		SpoutMessageBoxButton(3000, L"Button 3");
		int iret = SpoutMessageBox(NULL, str.c_str(), "User buttons", MB_OK);
		if (iret == 1000) SpoutMessageBox("Button 1 pressed");
		if (iret == 2000) SpoutMessageBox("Button 2 pressed");
		if (iret == 3000) SpoutMessageBox("Button 3 pressed");
	}

	//
	// Edit control
	//
	if (title == "Edit") {
		std::string str = "An edit control within the MessageBox can be used to return a string.\n\n";
		str += "Can be used in place of a specific application resource dialog\n";
		str += "If the edit string is not empty, it is shown highlighted in the edit control.\n";
		str += "All SpoutMessageBox functions such as user icon and buttons are available.\n\n";
		str += "   o Caption only\n";
		str += "      The edit control is in the main content area\n";
		str += "         std::string editstring;\n";
		str += "         SpoutMessageBox(NULL, NULL, \"Text entry with caption\", MB_OK, editstring)\n\n";
		str += "   o Caption with message\n";
		str += "      The edit control is in the footer area\n";
		str += "      This example shows an exiting text entry.\n";
		str += "      The same applies for the caption only option.\n";
		str += "         std::string editstring = \"Existing text\";\n";
		str += "         SpoutMessageBox(NULL, \"Enter new text\", \"Caption\", MB_OK, editstring)\n\n";
		str += "   o Icon\n";
		str += "      Custom icons can be used with both caption and message with caption\n";
		str += "         std::string icofile = GetExePath()+\"data/Spout.ico\", true);\n";
		str += "         SpoutMessageBoxIcon(icofile);\n\n";
		str += "Select an option and repeat for more options\n";
		SpoutMessageBoxButton(1000, L"Caption");
		SpoutMessageBoxButton(2000, L"Message");
		SpoutMessageBoxButton(3000, L"Icon");
		int iret = SpoutMessageBox(NULL, str.c_str(), "Edit control", MB_OK);
		std::string editstring;
		if (iret == 1000) {
			//                     hwnd  message  caption                    buttons      edit string
			iret = SpoutMessageBox(NULL, NULL,    "Text entry with caption", MB_OKCANCEL, editstring);
			if (iret != 0 && !editstring.empty())
				SpoutMessageBox(NULL, editstring.c_str(), "Text entered", MB_OK);
		}
		if (iret == 2000) {
			editstring = "existing entry";
			if (SpoutMessageBox(NULL, "This example also shows an existing entry.\nEnter new text in the edit control in the footer area.\n", "Message text entry", MB_OKCANCEL, editstring) == IDOK) {
				if (!editstring.empty())
					SpoutMessageBox(NULL, editstring.c_str(), "Text entered", MB_OK);
			}
		}
		if (iret == 3000) {
			std::string icofile = GetExePath()+"data/Spout.ico";
			SpoutMessageBoxIcon(icofile);
			if (SpoutMessageBox(NULL, "Example of using a custom icon with text entry\n",
				"Text entry with icon", MB_OKCANCEL, editstring) == IDOK) {
				if (!editstring.empty())
					SpoutMessageBox(NULL, editstring.c_str(), "Text entered", MB_OK);
			}
		}
	}

	//
	// MessageBox with a combo box control for item selection,
	// the selected item index is returned
	//
	if (title == "Combobox") {
		std::string str = "A combo box control within the MessageBox can be used\nto select an item from a list and return the selection index.\n\n";
		str += "A vector of strings is passed to fill the item list. The index value\n";
		str += "passed in is used to show an initial item in the combobox and is\n";
		str += "is returned as the selected item index\n\n";
		str += "    std::vector<std::string>items;\n";
		str += "    items.push_back(\"Item 1\");\n";
		str += "    items.push_back(\"Item 2\");\n";
		str += "    items.push_back(\"Item 3\");\n";
		str += "    items.push_back(\"Item 4\");\n";
		str += "    int index = 0;\n";
		str += "    if (SpoutMessageBox(NULL, NULL, \"Select item\", MB_OKCANCEL, items, index) == IDOK) {\n";
		str += "        SpoutMessageBox(\"Item selection\", \"%d (%s)\", index, items[index].c_str());\n";
		str += "    }\n\n";
		str += "Caption, Message or Icon options are the same as the edit control\n";
		str += "Select an option and repeat for more\n";
		SpoutMessageBoxButton(1000, L"Caption");
		SpoutMessageBoxButton(2000, L"Message");
		SpoutMessageBoxButton(3000, L"Icon");
		int buttonid = SpoutMessageBox(NULL, str.c_str(), "Combo box control", MB_OK);
		std::vector<std::string>items;
		items.push_back("Item 0");
		items.push_back("Item 1");
		items.push_back("Item 2");
		items.push_back("Item 3");
		int index = 1; // starting index (0-3)
		if (buttonid == 1000) {
			// Caption only
			if (SpoutMessageBox(NULL, NULL, "Select item", MB_OKCANCEL, items, index) == IDOK) {
				SpoutMessageBox("Item selection", "%d (%s)", index, items[index].c_str());
			}
		}
		if (buttonid == 2000) {
			index = 2; // starting index (0-3)
			// Caption with message
			if (SpoutMessageBox(NULL, "This is a list of items\nSelect an item\nThe item index will be returned", "Select item", MB_OKCANCEL, items, index) == IDOK) {
				SpoutMessageBox("Item selection", "%d (%s)", index, items[index].c_str());
			}
		}
		if (buttonid == 3000) {
			index = 3; // starting index (0-3)
			// Caption with message and icon
			std::string icofile = GetExePath()+"data/Spout.ico";
			SpoutMessageBoxIcon(icofile);
			if (SpoutMessageBox(NULL, "Combo box with user icon\nThis is a list of items\nSelect an item\nThe item index will be returned", "Select item", MB_OKCANCEL, items, index) == IDOK) {
				SpoutMessageBox("Item selection", "%d (%s)", index, items[index].c_str());
			}
		}
	}

	//
	// Modeless mode
	//
	// This transfers the message to another program
	// "SpoutPanel.exe" so the dialog does not stop the application's
	// operation which continues after the messagebox is displayed
	// and the messagebox remains until closed even if the application
	// is closed first. 
	// Spout must have been installed and SpoutPanel or SpoutSettings run at least once.
	// Modeless is disabled for any dialog requiring user input.
	if (title == "Modeless") {
		std::string str = "A MessageBox is normally modal and stops the application until closed.\n";
		str += "\"Modeless\" mode transfers the message to another program\n";
		str += "\"SpoutPanel.exe\" so the dialog does not stop the application's operation.\n";
		str += "The MessageBox remains open even if the applicaion is closed first.\n";
		str += "\"Modeless\" is disabled for any dialog requiring user input.\n\n";
		str += "       SpoutMessageBoxModeless(true);\n";
		str += "       SpoutMessageBoxModeless(false);\n\n";
		str += "This Message is modeless. Move it to one side and notice that the cube\n";
		str += "is still spinning. All other controls can also be used and the message will\n";
		str += "remain open even if the program is closed.\n\n";
		str += "For this to work, you must have downloaded a <a href=\"https://github.com/leadedge/Spout2/releases/\">Spout release</a>\n";
		str += "and run SpoutPanel or SpoutSettings at least once.\n\n";
		SpoutMessageBoxModeless(true);
		// Make topmost so it does not get lost
		SpoutMessageBox(NULL, str.c_str(), "Modeless message", MB_TOPMOST | MB_OK);
		SpoutMessageBoxModeless(false);
	}


	//
	// Hyperlink
	//
	// Hyperlinks can be included in the content using HTML format.
	// For example : <a href=\"https://spout.zeal.co/\">Spout home page</a>
	// Only double quotes are supported and must be escaped.
	//
	if (title == "Hyperlink") {
		std::string str = "Hyperlinks can be included in the content using HTML format.\n";
		str += "Only double quotes are supported and must be escaped.\n";
		str += "The MessageBox closes if the link is selected.\n\n";
		str += "For example : \n\n";
		// Don't use this, it's modified to show up in the MessageBox as code
		str += "    <a href=\\\"https://spout.zeal.co/\\\">Spout home page</a>\n\n";
		// This is the required format
		str += "This appears as  <a href=\"https://spout.zeal.co/\">Spout home page</a>.\n";
		str += "Refer to the sketch source code for details.\n";
		SpoutMessageBox(NULL, str.c_str(), "Hyperlink", MB_OK);
	}

	//
	// Clipboard
	//
	// Copy text to the clipboard
	//
	if (title == "Clipboard") {
		std::string str = "Text can be copied to the clipboard by using \n\n";
		str += "    bool CopyToClipBoard(HWND hwnd, const char* text);\n\n";
		str += "\"hwnd\" is the window handle used by ";
		str += "<a href=\"https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard\">OpenClipboard</a>\n";
		str += "to indicate the window to be associated with the open clipboard.\n";
		str += "It can be NULL and the open clipboard is associated with the current task.\n\n";
		str += "OK to copy the source of this message to the clipboard, CANCEL to skip.\n";
		if (SpoutMessageBox(NULL, str.c_str(), "Clipboard", MB_ICONINFORMATION | MB_OKCANCEL) == IDOK) {
			CopyToClipBoard(NULL, str.c_str());
			SpoutMessageBox("Text copied to the clipboard\nCheck by using a text editor and \"Ctrl-V\" or \"Edit > Paste\"");
		}
	}

	// Allow mouse item selection
	bMessagebox = false;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	// If the sending size matches the window size,
	// the fbo, texture or image should be updated here.
	if (w > 0 && h > 0) {
		senderwidth = w;
		senderheight = h;
		myFbo.allocate(senderwidth, senderheight, glFormat);
	}

}

//--------------------------------------------------------------
// Add a button to the buttons vector
void ofApp::AddButton(int x, int y, int w, int h,
	std::string title, ofColor backcol, ofColor textcol)
{
	button btn;
	btn.x = x;
	btn.y = y;
	btn.width = w;
	btn.height = h;
	btn.title = title;
	btn.backcol = backcol;
	btn.textcol = textcol;
	buttons.push_back(btn);
}

void ofApp::AddButtons()
{
	int x = 10;
	int y = 30;
	int width = 140;
	int height = 22;
	int next = 23;

	// Red
	AddButton(x, y, width, height, "About", ofColor(128, 32, 0), ofColor(255));
	y += next;
	// Blue
	AddButton(x, y, width, height, "Position", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Simple", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Variable", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Timeout", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Options", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Instruction", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Icon", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Buttons", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Edit", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Combobox", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Modeless", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Hyperlink", ofColor(0, 45, 90), ofColor(255));
	y += next;
	AddButton(x, y, width, height, "Clipboard", ofColor(0, 45, 90), ofColor(255));

}

void ofApp::DrawButtons()
{
	ofFill();
	int xpos = 0;
	int ypos = 0;
	ofRectangle r;
	for (size_t i = 0; i < buttons.size(); i++) {
		// Button background colour
		ofSetColor(buttons[i].backcol);
		// Highlight item for mouse hover
		if (mousex > buttons[i].x
			&& mousex < (buttons[i].x + buttons[i].width)
			&& mousey > buttons[i].y
			&& mousey < (buttons[i].y + buttons[i].height)) {
			ofSetColor(0, 128, 128); // cyan highlight
		}
		ofDrawRectangle(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);
		ofSetColor(buttons[i].textcol);
		r = myFont.getStringBoundingBox(buttons[i].title, 0, 0);
		xpos = buttons[i].x + (buttons[i].width - (int)r.getWidth()) / 2;
		DrawString(buttons[i].title, xpos, buttons[i].y + 18);
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
void ofApp::DrawString(std::string str, int posx, int posy, ofColor oldcolor)
{
	// White text
	ofSetColor(255);

	if (myFont.isLoaded()) {
		myFont.drawString(str, posx, posy);
	}
	else {
		// Allow for the Windows font not found
		// Quick fix because the default font is wider
		int x = posx-20;
		if (x <= 0) x = 10;
		ofDrawBitmapString(str, x, posy);
	}
	// Original colour
	ofSetColor(oldcolor);
}
