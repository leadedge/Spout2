/*

	Spout OpenFrameworks dynamic Load example

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

// SpoutLibrary dll dynamic loader
// Includes a header for SpoutLibrary
#include "SpoutLibraryLoader.h"


class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();

		SpoutLibraryLoader spoutloader; // SpoutLibrary dynamic loader
		SPOUTLIBRARY * sender; // A sender object pointer
		char sendername[256];  // Sender name
		ofImage myBoxImage;    // Image for the 3D demo
		float rotX, rotY;

};
