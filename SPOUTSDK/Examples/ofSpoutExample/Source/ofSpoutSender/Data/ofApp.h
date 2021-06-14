/*

	Spout OpenFrameworks Data Sender example

	Copyright (C) 2021 Lynn Jarvis.

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

*/
#pragma once

#include "ofMain.h"
#include "..\..\..\SpoutGL\SpoutSender.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void windowResized(int w, int h);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);

		SpoutSender sender;    // Spout sender object

		char sendername[256];  // Sender name
		unsigned int senderwidth;  // Dimensions of the sender can be independent
		unsigned int senderheight; // of the application window if using an fbo
		ofImage myBoxImage;    // Image for the 3D demo
		ofFbo myFbo;           // For texture send example
		ofPixels myPixels;     // For pixel send example
		float rotX, rotY;

		// For sender data
		int mousex;
		int mousey;
		int mousebutton;  // 0 - left, 1 - centre, 2 - right
		int mousepressed; // 1 - pressed, 0 - released
		int mousedragged; // 1 - moved with button down, 0 - released or moved with button up

		char senderdata[256];
		ofxXmlSettings xml;

};
