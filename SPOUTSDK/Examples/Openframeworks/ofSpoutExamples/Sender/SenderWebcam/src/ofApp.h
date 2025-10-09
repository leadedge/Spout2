/*

	Spout OpenFrameworks Webcam Sender example

	Copyright (C) 2022-2025 Lynn Jarvis.

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
#include "../../../apps/SpoutGL/SpoutSender.h" // Spout Sender

class ofApp : public ofBaseApp {
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);
		void mousePressed(int x, int y, int button);

		ofTrueTypeFont myFont; // For Truetype font
		bool LoadWindowsFont(ofTrueTypeFont& font, std::string name, int size);
		void DrawString(std::string str, int posx, int posy);
		bool SetWebcam(int index); // Set up a webcam

		ofVideoGrabber vidGrabber; // Webcam
		std::vector <ofVideoDevice> camdevices; // Webcams available
		SpoutSender camsender; // A Spout sender object
		std::string camsendername; // Sender name
		int camindex = 0; // Selected webcam in the device list

};
