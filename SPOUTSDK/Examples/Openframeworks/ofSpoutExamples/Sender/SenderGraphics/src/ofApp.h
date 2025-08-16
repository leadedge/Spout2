/*

	Spout OpenFrameworks Graphics Sender example

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
#include "../../../../apps/SpoutGL/SpoutSender.h"

// disable warning conversion from 'size_t' to 'int' for ofxXmlSettings
#pragma warning(disable:4267)

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void mousePressed(int x, int y, int button);
		void keyPressed(int key);
		void windowResized(int w, int h);

		// For Truetype font
		ofTrueTypeFont myFont;
		bool LoadWindowsFont(ofTrueTypeFont& font, std::string name, int size);
		void DrawString(std::string str, int posx, int posy);
		void SetSenderName();
		void SelectSenderFormat();

		// For sender
		SpoutSender sender;              // Spout sender object
		char m_sendername[256]{};        // Sender name
		unsigned int m_senderwidth = 0;
		unsigned int m_senderheight = 0;
		double m_senderfps = 0.0;       // For fps display averaging
		GLint m_glformat = GL_RGBA;      // Default OpenGL texture format
		bool bShowInfo = true;           // Show on-screen information

		// For graphics
		ofImage myBoxImage;            // Image for the 3D demo
		ofFbo myFbo;                   // For texture send example
		ofPixels myPixels;             // For pixel send example
		float rotX = 0.0f;             // Box rotation
		float rotY = 0.0f;

};
