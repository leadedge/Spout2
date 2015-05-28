/*

	Spout Example - main.cpp

	Example openFrameworks application for Spout inter-process communication

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
#include "testApp.h"
#include "ofAppGlutWindow.h"

//--------------------------------------------------------------
// to change options for console window (Visual Studio)
//
// Properties > Linker > System > Subsystem
//
// for console : Windows (/SUBSYSTEM:CONSOLE)
//
// for Window : Windows (/SUBSYSTEM:WINDOWS)
//
// Click APPLY (important) and OK. Then make changes to Main as below
//--------------------------------------------------------------

// for console default
int main() {
//

// for window without console
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	ofAppGlutWindow window; // create a window
	ofSetupOpenGL(&window, 256, 256, OF_WINDOW);

	ofRunApp(new testApp()); // start the app

}
