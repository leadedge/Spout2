/*

	Spout - Sender example

    CodeBlocks with GCC MINGW compiler using the Spout2.dll

	Copyright (C) 2015 Lynn Jarvis.

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
#include "resource.h" // for custom icon
#include "SpoutDLL.h" // for Spout2 dll


class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void windowResized(int w, int h);
		void exit();

		bool InitGLtexture(GLuint &texID, unsigned int width, unsigned int height);
		char sendername[256];			// Shared memory name
		GLuint sendertexture;			// Local OpenGL texture used for sharing
		bool bInitialized;				// Initialization result
		ofImage myTextureImage;			// Texture image for the 3D demo
		float rotX, rotY;

};
