/*

	Spout OpenFrameworks Spout Utilities example

	Copyright (C) 2015-2025 Lynn Jarvis.

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
#include "../../../apps/SpoutGL/SpoutSender.h"

// disable warning conversion from 'size_t' to 'int' for ofxXmlSettings
// Addon is in the common project file and used for the sync example
#pragma warning(disable:4267)

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);
		void windowResized(int w, int h);
		void mouseMoved(int x, int y);
		void mousePressed(int x, int y, int button);

		SpoutSender sender;            // Spout sender object
		char sendername[256]{};        // Sender name
		unsigned int senderwidth = 0;
		unsigned int senderheight = 0;
		double g_SenderFps = 0.0;      // For fps display averaging
		GLint glFormat = GL_RGBA;      // Default OpenGL texture format
		bool bShowInfo = true;         // Show on-screen information
		bool bMessagebox = false;      // Prevent mouse item selection while messagebox is open
		void doMessagebox(std::string title);   // SpoutMessageBox examples

		ofImage myBoxImage;            // Image for the 3D demo
		ofFbo myFbo;                   // For texture send
		float rotX = 0.0f;             // Box rotation
		float rotY = 0.0f;
		int mousex = 0;
		int mousey = 0;

		// Button
		struct button {
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			std::string title="";
			ofColor backcol = 192;
			ofColor textcol = 0;
		};

		std::vector<button> buttons;
		void AddButton(int x, int y, int w, int h,
			std::string title, ofColor backcol = 192, ofColor textcol = 0);
		void AddButtons();
		void DrawButtons();


		// Openframeworks truetype font for larger on-screen text
		ofTrueTypeFont myFont{};
		bool LoadWindowsFont(ofTrueTypeFont& font, std::string name, int size);
		void DrawString(std::string str, int posx, int posy, ofColor oldcolor = 255);

};
