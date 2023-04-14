/*

	Spout OpenFrameworks Sender Sync example

	Copyright (C) 2023 Lynn Jarvis.

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

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void windowResized(int w, int h);

		SpoutSender sender;    // Spout sender object
		char sendername[256];  // Sender name

		ofImage myBoxImage;    // Image for the 3D demo
		ofFbo myFbo;           // For texture send example
		ofPixels myPixels;     // For pixel send example
		float rotX, rotY;

		void showInfo();

};
