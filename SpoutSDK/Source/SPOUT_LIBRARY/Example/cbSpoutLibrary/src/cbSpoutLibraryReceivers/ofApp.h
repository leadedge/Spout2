/*

	Spout OpenFrameworks Multiple Receiver example

	Copyright (C) 2016 Lynn Jarvis.

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
#include "SpoutLibrary.h" // for the Spout SDK dll


class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void mousePressed(int x, int y, int button);
		void exit();

        SPOUTLIBRARY * receiver1; // Spout library pointer for receiver 1
        SPOUTLIBRARY * receiver2; // Spout library pointer for receiver 2

		bool bInitialized1;		// Initialization result
		ofTexture myTexture1;	// Texture used for texture share transfers
		char SenderName1[256];	// Sender name used by a receiver
		int g_Width1, g_Height1;  // Used for checking sender size change

		bool bInitialized2;		// Initialization result
		ofTexture myTexture2;	// Texture used for texture share transfers
		char SenderName2[256];	// Sender name used by a receiver
		int g_Width2, g_Height2;  // Used for checking sender size change

};
