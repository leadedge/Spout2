/*
				
		Spout sender for Cinder

		Based on the RotatingBox CINDER example without much modification
		Nothing fancy about this, just the basics.

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
	04.06.14 - updated Spout dll 04/06 - host fbo option removed
	11.07.14 - changed to Spout SDK instead of the dll
	29.09.14 - updated to revised SDK
	12.10.14 - recompiled for release
	03.01.15 - SDK recompile
	04.02.15 - SDK recompile for default DX9 (see SpoutGLDXinterop.h)
	14.02.15 - SDK recompile for default DX11 and auto compatibility detection (see SpoutGLDXinterop.cpp)
	21.05.15 - Added optional SetDX9 call
			 - Recompiled for both DX9 and DX11 for new installer
	26.05.15 - Recompile for revised SpoutPanel registry write of sender name
	01.07.15 - Convert project to VS2012
			 - add a window title
	30-03-16 - Rebuild for Spout 2.005 release - VS2012 /MT
	17-01-17 - Rebuild for Spout 2.006 release - VS2012 /MT
	11-05-19 - Revise and rebuild for Spout 2.007 release - VS2012 /MT
	18-06-19 - Revise sender Update function and rebuild for Spout 2.007 release - VS2012 /MT
	22.01.10 - Update for latest 2.007 changes - VS2012 /MT

	
*/
#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h" 
#include "cinder/Camera.h"
#include <vector>
#include <math.h>
#include "..\..\..\SpoutSDK\Spout.h"

using namespace ci;
using namespace ci::app;

class SpoutBoxApp : public AppBasic {


  public:

	void setup();
	void update();
	void draw();
	void resize();
	void prepareSettings(Settings *settings);
	void shutdown();
	void showInfo();
	
	// still using the cam vars from the cinder demo
	CameraPersp	mCam;
	gl::Texture	cubeTexture;
	Matrix44f mCubeRotation;

	SpoutSender spoutsender; // Create a Spout sender object
	std::string SenderName;	// sender name 
	gl::Texture spoutTexture; // Cinder texture used for sharing
	gl::Fbo myFbo; // Fbo used for data collection and transfer

};


void SpoutBoxApp::prepareSettings(Settings *settings)
{
		settings->setTitle("CinderSpoutSender");
		settings->setWindowSize( 640, 360 );
		settings->setFullScreen( false );
		settings->setResizable( true ); // see rezize() to update the sender
		settings->setFrameRate( 30.0f ); // default frame rate
		// A receiver will detect this fps if the user
		// has selected "Frame count" in SpoutSettings
}


void SpoutBoxApp::setup()
{

	// Option : enable Spout logging to detect warnings and errors
	// Logging functions are in the "spoututils" namespace so they can be called directly.
	//    EnableSpoutLog();
	//
	// Output is to a console window.
	//
	// You can set the level above which the logs are shown
	// SPOUT_LOG_SILENT  : SPOUT_LOG_VERBOSE : SPOUT_LOG_NOTICE (default)
	// SPOUT_LOG_WARNING : SPOUT_LOG_ERROR   : SPOUT_LOG_FATAL
	// For example, to show only warnings and errors (you shouldn't see any)
	// or leave set to default Notice to see more information.
	//    SetSpoutLogLevel(SPOUT_LOG_WARNING);
	//
	// You can instead, or additionally, specify output to a text file
	// with the extension of your choice
	//    EnableSpoutLogFile("SpoutLibrary Sender.log");
	//
	// The log file is re-created every time the application starts
	// unless you specify to append to the existing one :
	//    EnableSpoutLogFile("SpoutLibrary Sender.log", true);
	//
	// The file is saved in the %AppData% folder 
	//    C:>Users>username>AppData>Roaming>Spout
	// unless you specify the full path.
	// After the application has run you can find and examine the log file
	//
	// This folder can also be shown in Windows Explorer directly from the application.
	//    ShowSpoutLogs();
	//
	// Or the entire log can be returned as a string
	//    std::string logstring = GetSpoutLog();
	//
	// You can also create your own logs
	// For example :
	//    SpoutLog("SpoutLog test");
	//
	// Or specify the logging level :
	// For example :
	//    SpoutLogNotice("Important notice");
	// or :
	//    SpoutLogFatal("This should not happen");
	// or :
	//    SetSpoutLogLevel(SPOUT_LOG_VERBOSE);
	//    SpoutLogVerbose("Message");
	//

	// Image to texture the demo cube
	cubeTexture = loadImage( "../data/SpoutLogo512.jpg");

	// Set up for the cube draw
	mCam.lookAt( Vec3f( 3, 2, -3 ), Vec3f::zero() );
	mCubeRotation.setToIdentity();
	glEnable( GL_TEXTURE_2D );
	gl::enableDepthRead();
	gl::enableDepthWrite();	

	// Create an fbo for texture transfers
	// Default format is RGBA which is what we want
	myFbo = gl::Fbo(getWindowWidth(), getWindowHeight());

	// Set a sender name
	SenderName = "CINDER Spout Sender"; 

	// Create a Spout sender with that name and the fbo dimensions
	spoutsender.CreateSender(SenderName.c_str(), myFbo.getWidth(), myFbo.getHeight());

	// Option : set the frame rate of the application.
	// If the user has selected "Frame count" in SpoutSettings
	// a receiver will detect this rate.
	// Applications without frame rate control can use 
	// a Spout function HoldFps to control frame rate (see Update())
	// If the default Cinder frame rate is 30 we will boost it a bit
	// so we see the Spout HoldFps function working.
	setFrameRate(60.0);

}


void SpoutBoxApp::shutdown()
{
	spoutsender.ReleaseSender();
}


void SpoutBoxApp::update()
{
	// Rotate the cube around an arbitrary axis
	mCubeRotation.rotate( Vec3f( 1, 1, 1 ), 0.02f );
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	gl::setMatrices( mCam );

	// Optionally control frame rate
	// This must be called every frame
	spoutsender.HoldFps(30);

}


void SpoutBoxApp::resize()
{
	// Update the fbo and sender dimensions if necessary
	// This is not actually necessary because the fbo size
	// is independent of the window. This just illustrates
	// what you need to do for sender size updates.
	if(spoutsender.GetWidth() != getWindowWidth() || spoutsender.GetHeight() != getWindowHeight()) {
		myFbo.reset();
		myFbo = gl::Fbo(getWindowWidth(), getWindowHeight());
		spoutsender.UpdateSender(SenderName.c_str(), getWindowWidth(), getWindowHeight());
	}
}


void SpoutBoxApp::draw()
{
	
	// Clear the screen first
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );

	// Quit if the sender is not initialized
	if (!spoutsender.IsInitialized())
		return;

	// Draw 3D graphics demo into the fbo
	// This could be anything for your application
	myFbo.bindFramebuffer();
	gl::clear( Color( 0.05f, 0.10f, 0.30f ) ); // dark blue
	cubeTexture.bind();
	glPushMatrix();
		gl::multModelView( mCubeRotation );
		gl::drawCube( Vec3f::zero(), Vec3f( 2.5f, 2.5f, 2.5f ) );
	glPopMatrix();
	cubeTexture.unbind();

	// Option : Send an fbo while the fbo is bound
	spoutsender.SendFbo(myFbo.getId(), getWindowWidth(), getWindowHeight());

	myFbo.unbindFramebuffer();

	// Draw the Fbo
	myFbo.blitToScreen(myFbo.getBounds(), getWindowBounds());

	// Option : send a texture
	// spoutsender.SendTexture(myFbo.getTexture().getId(),
		// myFbo.getTexture().getTarget(),
		// getWindowWidth(), getWindowHeight());

	// Option : send pixels
	// Invert because they come from the fbo bottom up
	// Surface mySurface(myFbo.getTexture());
	// spoutsender.SendImage(mySurface.getData(), myFbo.getWidth(), myFbo.getHeight(), GL_RGBA, true);

	// Show what it is sending
	showInfo();

}


void SpoutBoxApp::showInfo()
{
	char txt[256];
	glColor3f( 1.0f, 1.0f, 1.0f );
	sprintf_s(txt, "Sending as [%s] (%dx%d)", SenderName.c_str(), spoutsender.GetWidth(), spoutsender.GetHeight());
	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	gl::drawString( txt, Vec2f( toPixels( 20 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 20 ) ) );
	// Show sender fps and framecount if selected
	if (spoutsender.GetFrame() > 0)
		sprintf_s(txt, "fps : %2.0f  frame : %d", floor(spoutsender.GetFps()+0.5), spoutsender.GetFrame()); 
	else
		sprintf_s(txt, "fps : %2.0f", getAverageFps());
	gl::drawString( txt, Vec2f(toPixels( 20 ), toPixels( 40 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 20 ) ) );
	gl::disableAlphaBlending();
}

CINDER_APP_BASIC( SpoutBoxApp, RendererGl )
