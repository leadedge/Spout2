/*

	Spout Example Receiver

	Copyright (C) 2014 Lynn Jarvis.

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

#include "ofMain.h"						// The usual
#include "..\..\..\SpoutSDK\Spout.h"	// Spout SDK

class testApp : public ofBaseApp {

	public:

		void setup();
		void update();
		void draw();
		
		void mousePressed(int x, int y, int button);
		void exit(); 

		SpoutReceiver *spoutreceiver;	// A receiver object
		bool bInitialized;				// Initialization result
		bool bMemoryShare;				// Texture share compatibility
		ofTexture myTexture;			// Texture used for texture share transfers
		unsigned int g_Width, g_Height;	// Global width and height
		char SenderName[256];			// Sender name used by a receiver

};
