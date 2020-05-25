/*
				
		Spout receiver for Cinder

	==========================================================================
	Copyright (C) 2014-2020 Lynn Jarvis.

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
	11.05.19 - Revise and rebuild for 2.007 release - VS2012 /MT
	22.01.20 - Update for latest 2.007 changes - VS2012 /MT
	25.05.20 - Update for latest 2.007 changes - VS2012 /MT

*/

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "..\..\..\SpoutSDK\Spout.h"

using namespace ci;
using namespace ci::app;

class SpoutBoxApp : public AppBasic {

  public:

	void setup();
	void draw();
	void update();
	void mouseDown(MouseEvent event);

	SpoutReceiver spoutreceiver;				// Create a Spout receiver object
	gl::Texture spoutTexture;					// Local Cinder texture used for sharing
	void prepareSettings(Settings *settings);
	void shutdown();
	void showInfo();

};


void SpoutBoxApp::prepareSettings(Settings *settings)
{
	settings->setTitle("CinderSpoutReceiver");
	settings->setWindowSize( 640, 360); // Window can be adjusted to the sender size
	settings->setResizable( true ); // allowed for a receiver
	settings->setFullScreen( false );
	settings->setFrameRate( 60.0f );
}


void SpoutBoxApp::setup()
{

	// Allocate a texture of any size
	// It will be resized to match the sender when the receiver connects
	spoutTexture =  gl::Texture(getWindowWidth(), getWindowHeight());

	// Optional : specify the sender to connect to.
	// The receiver will not connect to any other unless the user selects one.
	// If that sender closes, the application will wait for the nominated sender to open.
	// spoutreceiver.SetReceiverName("Spout DX11 Sender");

}


void SpoutBoxApp::update()
{
	// If Updated() returns true, the sender size has changed
	// and the receiving texture must be re-sized
	if (spoutreceiver.IsUpdated()) {
		// Resize the receiving texture to the new sender size		
		spoutTexture.reset();
		spoutTexture =  gl::Texture(spoutreceiver.GetSenderWidth(), spoutreceiver.GetSenderHeight());
		// Optionally resize the window
		setWindowSize(spoutreceiver.GetSenderWidth(), spoutreceiver.GetSenderHeight());
		// and centre on the desktop
		setWindowPos((GetSystemMetrics(SM_CXSCREEN)-getWindowWidth())/2, (GetSystemMetrics(SM_CYSCREEN)-getWindowHeight())/2); 
	}
}


void SpoutBoxApp::shutdown()
{
	spoutreceiver.ReleaseReceiver();
}


void SpoutBoxApp::mouseDown(MouseEvent event)
{
	// Select a sender
	if( event.isRightDown() ) { 
		spoutreceiver.SelectSender();
	}
}


void SpoutBoxApp::draw()
{
	
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();
	gl::color( Color( 1, 1, 1 ) );

	// ReceiveTexture connects to and receives from a sender
	// Optionally include the ID of an fbo if one is currently bound
	spoutreceiver.ReceiveTexture(spoutTexture.getId(), spoutTexture.getTarget());

	// Draw the texture and fill the screen
	gl::draw(spoutTexture, getWindowBounds());

	// Show the user what it is receiving
	showInfo();

}

void SpoutBoxApp::showInfo()
{
	char str[256];

	if(spoutreceiver.IsConnected()) {
		// Applications < 2.007 will return no frame count information
		// Frame counting can also be disabled in SpoutSettings
		if (spoutreceiver.GetSenderFrame() > 0) {
			sprintf_s(str, 256, "Receiving : [%s] (%dx%d : fps %2.0f : frame %d)",
				spoutreceiver.GetSenderName(), // sender name
				spoutreceiver.GetSenderWidth(), // width
				spoutreceiver.GetSenderHeight(), // height 
				spoutreceiver.GetSenderFps(), // fps
				spoutreceiver.GetSenderFrame()); // frame since the sender started
		}
		else {
			sprintf_s(str, 256, "Receiving : [%s] (%dx%d)",
				spoutreceiver.GetSenderName(),
				spoutreceiver.GetSenderWidth(),
				spoutreceiver.GetSenderHeight());
		}
		gl::enableAlphaBlending();
		gl::drawString( str, Vec2f( toPixels( 10 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 20 ) ) );
		gl::disableAlphaBlending();
	}
	else {
		gl::enableAlphaBlending();
		gl::drawString( "No sender detected", Vec2f( toPixels( 10 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 20 ) ) );
		gl::disableAlphaBlending();
	}
}


CINDER_APP_BASIC( SpoutBoxApp, RendererGl )
