/*

	Spout OpenFrameworks Multiple Receiver example
	using 2.007 SpoutLibrary

	Copyright (C) 2015-2022 Lynn Jarvis.

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
#include "SpoutLibrary.h" // for the SpoutLibrary dll

class ofApp : public ofBaseApp{
	public:

		void setup();
		void update();
		void draw();
		void exit();
		void mousePressed(int x, int y, int button);
		void keyPressed(int key);

		SPOUTLIBRARY * receiver1; // A receiver object pointer
		ofTexture myTexture1; // Receiving texture
		
		SPOUTLIBRARY * receiver2; // Second receiver
		ofTexture myTexture2; // Second receiving texture

		void showInfo();

};
