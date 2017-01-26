/*
				
		Basic Spout receiver for Cinder

		Uses the Spout SDK

		Based on the RotatingBox CINDER example without much modification
		Nothing fancy about this, just the basics.

		Search for "SPOUT" to see what is required

	==========================================================================
	Copyright (C) 2014-2017 Lynn Jarvis.

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
	==========================================================================

	11.05.14 - used updated Spout Dll with host fbo option and rgba
	04.06.14 - used updated Spout Dll 04/06 with host fbo option removed
			 - added Update function
			 - moved receiver initialization from Setup to Update for sender detection
	11.07.14 - changed to Spout SDK instead of the dll
	29.09.14 - update with with SDK revision
	12.10.14 - recompiled for release
	03.01.15 - SDK recompile - SpoutPanel detected from registry install path
	04.02.15 - SDK recompile for default DX9 (see SpoutGLDXinterop.h)
	14.02.15 - SDK recompile for default DX11 and auto compatibility detection (see SpoutGLDXinterop.cpp)
	21.05.15 - Added optional SetDX9 call
			 - Recompiled for both DX9 and DX11 for new installer
	26.05.15 - Recompile for revised SpoutPanel registry write of sender name
	01.07.15 - Convert project to VS2012
			 - add a window title
	30.03.16 - Rebuild for 2.005 release - VS2012 /MT
	17.01.16 - Rebuild for 2.006 release - VS2012 /MT

*/

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"

// -------- SPOUT -------------
#include "..\..\..\SpoutSDK\Spout.h"
// ----------------------------

using namespace ci;
using namespace ci::app;

class SpoutBoxApp : public AppBasic {

  public:

	void setup();
	void draw();
	void update();
	void mouseDown(MouseEvent event);

	// -------- SPOUT -------------
	SpoutReceiver spoutreceiver;				// Create a Spout receiver object
	void prepareSettings(Settings *settings);
	void shutdown();
	bool bInitialized;							// true if a sender initializes OK
	bool bDoneOnce;								// only try to initialize once
	bool bMemoryMode;							// tells us if texture share compatible
	unsigned int g_Width, g_Height;				// size of the texture being sent out
	char SenderName[256];						// sender name 
	gl::Texture spoutTexture;					// Local Cinder texture used for sharing
	// ----------------------------

};

// -------- SPOUT -------------
void SpoutBoxApp::prepareSettings(Settings *settings)
{
	g_Width = 320; // set global width and height to something
	g_Height = 240; // they need to be reset when the receiver connects to a sender
	settings->setTitle("CinderSpoutReceiver");
	settings->setWindowSize( g_Width, g_Height );
	settings->setFullScreen( false );
	settings->setResizable( true ); // allowed for a receiver
	settings->setFrameRate( 60.0f );

}
// ----------------------------


void SpoutBoxApp::setup()
{

}

void SpoutBoxApp::update()
{
	unsigned int width, height;

	// -------- SPOUT -------------
	if(!bInitialized) {

		// This is a receiver, so the initialization is a little more complex than a sender
		// The receiver will attempt to connect to the name it is sent.
		// Alternatively set the optional bUseActive flag to attempt to connect to the active sender. 
		// If the sender name is not initialized it will attempt to find the active sender
		// If the receiver does not find any senders the initialization will fail
		// and "CreateReceiver" can be called repeatedly until a sender is found.
		// "CreateReceiver" will update the passed name, and dimensions.
		SenderName[0] = NULL; // the name will be filled when the receiver connects to a sender
		width  = g_Width; // pass the initial width and height (they will be adjusted if necessary)
		height = g_Height;

		// Initialize a receiver
		if(spoutreceiver.CreateReceiver(SenderName, width, height, true)) { // true to find the active sender
			// Optionally test for texture share compatibility
			// bMemoryMode informs us whether Spout initialized for texture share or memory share
			bMemoryMode = spoutreceiver.GetMemoryShareMode();

			// Is the size of the detected sender different from the current texture size ?
			// This is detected for both texture share and memoryshare
			if(width != g_Width || height != g_Height) {
				// Reset the global width and height
				g_Width = width;
				g_Height = height;
				// Reset the local receiving texture size
				spoutTexture =  gl::Texture(g_Width, g_Height);
				// reset render window
				setWindowSize(g_Width, g_Height);
			} 
			bInitialized = true;
		}
		else {
			// Receiver initialization will fail if no senders are running
			// Keep trying until one starts
		}
	} // endif not initialized
	// ----------------------------

}
// -------- SPOUT -------------
void SpoutBoxApp::shutdown()
{
	spoutreceiver.ReleaseReceiver();
}

void SpoutBoxApp::mouseDown(MouseEvent event)
{
	// Select a sender
	if( event.isRightDown() ) { 
		spoutreceiver.SelectSenderPanel();
	}
}
// ----------------------------


void SpoutBoxApp::draw()
{
	
	unsigned int width, height;
	char txt[256];

	gl::setMatricesWindow( getWindowSize() );
	gl::clear();
	gl::color( Color( 1, 1, 1 ) );	

	// Save current global width and height - they will be changed
	// by receivetexture if the sender changes dimensions
	width  = g_Width;
	height = g_Height;
	
	//
	// Try to receive the texture at the current size 
	//
	// NOTE : if ReceiveTexture is called with a framebuffer object bound, 
	// include the FBO id as an argument so that the binding is restored afterwards
	// because Spout uses an fbo for intermediate rendering
	if(bInitialized) {
		if(spoutreceiver.ReceiveTexture(SenderName, width, height, spoutTexture.getId(), spoutTexture.getTarget())) {
			//	Width and height are changed for sender change so the local texture has to be resized.
			if(width != g_Width || height != g_Height ) {
				// The sender dimensions have changed - update the global width and height
				g_Width  = width;
				g_Height = height;
				// Update the local texture to receive the new dimensions
				spoutTexture =  gl::Texture(g_Width, g_Height);
				// reset render window
				setWindowSize(g_Width, g_Height);
				return; // quit for next round
			}

			// Otherwise draw the texture and fill the screen
			gl::draw(spoutTexture, getWindowBounds());

			// Show the user what it is receiving
			gl::enableAlphaBlending();
			sprintf_s(txt, "Receiving from [%s]", SenderName);
			gl::drawString( txt, Vec2f( toPixels( 20 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
			sprintf_s(txt, "fps : %2.2d", (int)getAverageFps());
			gl::drawString( txt, Vec2f(getWindowWidth() - toPixels( 100 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
			gl::drawString( "RH click to select a sender", Vec2f( toPixels( 20 ), getWindowHeight() - toPixels( 40 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
			gl::disableAlphaBlending();
			return; // received OK
		}
	}

	gl::enableAlphaBlending();
	gl::drawString( "No sender detected", Vec2f( toPixels( 20 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
	gl::disableAlphaBlending();
	// ----------------------------
}


CINDER_APP_BASIC( SpoutBoxApp, RendererGl )
