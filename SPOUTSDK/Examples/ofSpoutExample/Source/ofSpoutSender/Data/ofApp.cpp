/*

	Spout OpenFrameworks Data Sender example

	Example of sending per-frame data

	Spout 2.007
	OpenFrameworks 11
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

	EnableSpoutLog();

 	strcpy_s(sendername, 256, "OF Spout Data Sender"); // The sender name
	ofSetWindowTitle(sendername); // show it on the title bar


	
	// 3D drawing setup for the demo 
	ofDisableArbTex(); // Needed for ofBox texturing
	ofEnableDepthTest(); // enable depth comparisons for the cube
	myBoxImage.load("SpoutBox1.png"); // image for the cube texture
 	rotX = 0.0f;
	rotY = 0.0f;

	// Set the sender size
	senderwidth = ofGetWidth();
	senderheight = ofGetHeight();

	// Create an RGBA fbo for texture transfers
	myFbo.allocate(senderwidth, senderheight, GL_RGBA);

	// Give the sender a name
	sender.SetSenderName(sendername);

	// Mouse data to send to receiver
	mousex = 0;
	mousey = 0;
	mousebutton = 0;
	mousepressed = 0;
	mousedragged = 0;

	//
	// Create shared memory for data exchange
	//
	// The size of the shared memory map is fixed after it is created.
	//
	// If the sending data length is also fixed, WriteMemoryBuffer can be used
	// alone and a map of that size is created on the first function call.
	//
	// If the sending data length varies, the memory must be created in advance
	// large enough to contain at least the maximum number of bytes required.
	//
	// For example, the xml method below requires between 109 and 114 bytes,
	// so the memory should be created at least 114 bytes in size. 
	//
	// If the sending data size is not precisely known, the shared memory can be 
	// created larger. For example, a 256 byte buffer is plenty for this example.
	//
	sender.spout.CreateMemoryBuffer(sendername, 256);

	// Framerate is default 60fps for driver vsync enabled

} // end setup


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	//
	// Option - frame synchronisation
	//
	// To avoid the receiver from missing frames, the sender can wait
	// until the receiver signals that it is ready to read more data.
	// Use a timeout greater than the expected delay. 
	//
	// Sync is necessary if the sending and receiving applications require
	// frame accuracy and missed frames are not acceptable. 
	//
	// The sender will then run at the same framerate as the receiver.
	// For example, if connected to the data receiver example, this will
	// be 30fps instead of the 60fps established in Setup().
	//
	// Sync is not required for this simple example but is included to
	// demonstrate the functions (Refer to the data receiver example).
	//
	sender.WaitFrameSync(sender.GetName(), 67);

	// Draw 3D graphics into the fbo
	myFbo.begin();
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
	sender.SendFbo(myFbo.getId(), senderwidth, senderheight, false);

	myFbo.end();

	// Draw the result
	myFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// ====================================
	
	//
	// Send data
	// Often known as "per-frame metadata".
	//
	// Data to write should be char type.
	// In this example, send mouse coordinates and button status.
	// The receiver should expect the same data format.	
	// (Refer to the data receiver example).

	//
	// Method 1 (char array)
	//
	// Here the coordinates are converted to a char array.
	// The array length can vary depending on mouse coordinate values.
	// A memory buffer of sufficient size has been created in advance (see Setup)
	//
	sprintf_s(senderdata, 256, "%d %d %d %d %d", mousex, mousey, mousebutton, mousepressed, mousedragged);
	sender.WriteMemoryBuffer(sender.GetName(), senderdata, strlen(senderdata));
	
	/*
	//
	// Method 2 - XML
	//
	// XML might be appropriate for some applications.
	// The receiver can parse the data.
	//
	// In this example we create :
	//
	// <mouse>
	//	  <mousex>000</mousex>
	// 	  <mousey>000</mousey>
	//	  <button>0</button>
	//	  <pressed>0</pressed>
	//	  <dragged>0</dragged>
	// </mouse>
	//

	// Set the mouse data to xml
	xml.setValue("mouse:mousex", std::to_string(mousex));
	xml.setValue("mouse:mousey", std::to_string(mousey));
	xml.setValue("mouse:button", std::to_string(mousebutton));
	xml.setValue("mouse:pressed", std::to_string(mousepressed));
	xml.setValue("mouse:dragged", std::to_string(mousedragged));

	// Save the xml data as a string
	std::string xmlText;
	xml.copyXmlToString(xmlText);

	// Write the string data to shared memory.
	// A memory buffer of sufficient size has been created in advance (see Setup).
	// The receiver can parse the xml data, expecting the format sent.
	sender.WriteMemoryBuffer(sender.GetName(), xmlText.c_str(), xmlText.size());
	*/

	/*
	//
	// Method 3 - casting
	//
	// WriteMemoryBuffer requires char data.
	// If we have an array of different type, the pointer can be cast to char.
	// For example, here we create an int array "mousedata".
	//
	int mousedata[5];
	mousedata[0] = mousex;
	mousedata[1] = mousey;
	mousedata[2] = mousebutton;
	mousedata[3] = mousepressed;
	mousedata[4] = mousedragged;

	// Cast to char
	// The receiver can cast the char pointer back to int.
	char * mousechars = reinterpret_cast<char *>(mousedata);

	// Write the data to shared memory.
	// Because the data is fixed in size (5 integers), shared memory of that size
	// can simply be created by WriteMemoryBuffer on the first function call
	// instead of creating it in advance (see Setup).
	sender.WriteMemoryBuffer(sender.GetName(), mousechars, 5*sizeof(int));
	*/

	//
	// These are just examples. There are sure to be other applications.
	//

	// ====================================


	// Show what it is sending
	ofSetColor(255);
	std::string str = "Sending as : ";
	str += sender.GetName(); str += " (";
	str += ofToString(sender.GetWidth()); str += "x";
	str += ofToString(sender.GetHeight()); str += ")";
	if (sender.GetFrame() > 0) {
		str += " fps ";
		str += ofToString((int)roundf(sender.GetFps()));
		str += " : frame  ";
		str += ofToString(sender.GetFrame());
	}
	else {
		// Openframeworks fps
		str += " fps : " + ofToString((int)roundf(ofGetFrameRate()));
	}
	ofDrawBitmapString(str, 10, 20);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Close the sender on exit
	sender.ReleaseSender();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) 
{
	// The sending size matches the window size, update the fbo
	if (w > 0 && h > 0) {
		senderwidth = w;
		senderheight = h;
		myFbo.allocate(senderwidth, senderheight, GL_RGBA);
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	mousex = x;
	mousey = y;
	mousebutton = button;
	mousepressed = 1;
	mousedragged = 0;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	mousex = x;
	mousey = y;
	mousebutton = button;
	mousepressed = 0;
	mousedragged = 0;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
	mousex = x;
	mousey = y;
	mousepressed = 0;
	mousedragged = 0;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	mousex = x;
	mousey = y;
	mousebutton = button;
	mousepressed = 1;
	mousedragged = 1;
}

