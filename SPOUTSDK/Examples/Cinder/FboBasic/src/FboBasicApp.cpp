/*

		Spout example for Cinder
		Based on the FboBasic example
		Search on SPOUT for details.

		2021-2022 Lynn Jarvis https://spout.zeal.co/

		Revised 18.11.21

*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

// SPOUT
// Enable this define to create a receiver
// Disable to create a sender
// #define _receiver

#ifdef _receiver
#include "..\SpoutGL\SpoutReceiver.h"
#else
#include "..\SpoutGL\SpoutSender.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

// This sample shows a very basic use case for FBOs - it renders a spinning colored cube
// into an FBO, and uses that as a Texture onto the sides of a blue cube.
class FboBasicApp : public App {
  public:

	void	setup() override;
	void	update() override;
	void	draw() override;

	// SPOUT
	void	quit() override;
#ifdef _receiver
	void    mouseDown(MouseEvent event) override;
#endif

  private:

	// SPOUT
#ifdef _receiver
	SpoutReceiver receiver;
	gl::Texture2dRef spoutTexture; // texture used for receiving
#else
	SpoutSender sender;
	void				renderSceneToFbo();
	gl::FboRef			mFbo;
	mat4				mRotation;
	static const int	FBO_WIDTH = 256, FBO_HEIGHT = 256;
#endif

	void showInfo();

};

void FboBasicApp::setup()
{
	// SPOUT
	// Optional console or logging
	// OpenSpoutConsole();
	// EnableSpoutLog();
	
	// Match window size to Spout demo programs
	setWindowSize(640, 360);

#ifdef _receiver
	// Allocate a receiving texture of any size
	spoutTexture = gl::Texture2d::create(getWindowWidth(), getWindowHeight());
#else
	gl::Fbo::Format format;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	mFbo = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format.depthTexture());
	gl::enableDepthRead();
	gl::enableDepthWrite();
	// Option : set the frame rate of the sender.
	// A receiver will detect this rate.
	// setFrameRate(30.0);
#endif

}

#ifndef _receiver
// Render the color cube into the FBO
void FboBasicApp::renderSceneToFbo()
{
	// this will restore the old framebuffer binding when we leave this function
	// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
	// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
	gl::ScopedFramebuffer fbScp( mFbo );

	// clear out the FBO with blue
	gl::clear( Color( 0.25, 0.5f, 1.0f ) );

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );

	// setup our camera to render the torus scene
	CameraPersp cam( mFbo->getWidth(), mFbo->getHeight(), 60.0f );
	cam.setPerspective( 60, mFbo->getAspectRatio(), 1, 1000 );
	cam.lookAt( vec3( 2.8f, 1.8f, -2.8f ), vec3( 0 ));
	gl::setMatrices( cam );

	// set the modelview matrix to reflect our current rotation
	gl::setModelMatrix( mRotation );
	
	// render the color cube
	gl::ScopedGlslProg shaderScp( gl::getStockShader( gl::ShaderDef().color() ) );
	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	gl::drawColorCube( vec3( 0 ), vec3( 2.2f ) );
	gl::color( Color::white() );

	//
	// SPOUT - Send fbo
	//

	//   Send the fbo used to texture the cube while it is bound for read.
	//   The default "screen" framebuffer can also be used to send the whole screen.
	//   See Update().
	// sender.SendFbo(mFbo->getId(), mFbo->getWidth(), mFbo->getHeight());

}
#endif

void FboBasicApp::update()
{
	// SPOUT
#ifdef _receiver
	// If Updated() returns true, the sender size has changed.
	// Resize the receiving texture.
	if (receiver.IsUpdated()) {
		spoutTexture.reset();
		spoutTexture = gl::Texture2d::create(receiver.GetSenderWidth(), receiver.GetSenderHeight());
	}
#else
	
	// Rotate the torus by .06 radians around an arbitrary axis
	mRotation *= rotate(0.06f, normalize(vec3(0.16666f, 0.333333f, 0.666666f)));

	// render into our FBO
	renderSceneToFbo();

	//
	// SPOUT
	//

	//
	// Send the "screen" FBO
	//
	// The default framebuffer can be used to send the whole screen.
	// Call this in "update" rather than "draw" to prevent multiple sender
	// updates when the window is stretched by the user and re-sized.
	// Update() is not called during this process, but Draw() is.
	//

	sender.SendFbo(0, getWindowWidth(), getWindowHeight());

	// On-screen text for what it's sending (see Draw) is not done
	// in this case or it would be included in the sending frame.

	// An fbo can also be used for send while it is bound.
	// See renderSceneToFbo().

	//
	// Send texture
	//

	// Send the texture attached to the fbo used for the cube
	// Include the ID of the active framebuffer if one is currently bound.
	// sender.SendTexture(mFbo->getColorTexture()->getId(),
		// mFbo->getColorTexture()->getTarget(),
		// mFbo->getColorTexture()->getWidth(),
		// mFbo->getColorTexture()->getHeight());
	// Option : show on-screen sender details (see showInfo() in Draw)

	// Note that only one fbo or texture can be used at the same time.


#endif

}

void FboBasicApp::draw()
{
	// clear the window to gray
	gl::clear(Color(0.35f, 0.35f, 0.35f));

	// SPOUT
#ifdef _receiver
	// ReceiveTexture connects to and receives from a sender
	// Flip vertically to compensate for coordinates of the DirectX shared texture
	// Include the ID of the active framebuffer if one is currently bound.
	receiver.ReceiveTexture(spoutTexture->getId(), spoutTexture->getTarget(), true);
	
	// Draw the texture and fill the screen if connected to a sender
	if (receiver.IsConnected()) {
		gl::color(Color::white());
		gl::draw(spoutTexture, getWindowBounds());
	}

	// Show what it is receiving
	showInfo();

#else
	
	// setup our camera to render the cube
	CameraPersp cam(getWindowWidth(), getWindowHeight(), 60.0f);
	cam.setPerspective(60, getWindowAspectRatio(), 1, 1000);
	cam.lookAt(vec3(2.6f, 1.6f, -2.6f), vec3(0));
	gl::setMatrices(cam);

	// use the scene we rendered into the FBO as a texture
	mFbo->bindTexture();

	// draw a cube textured with the FBO
	{
		gl::ScopedGlslProg shaderScp(gl::getStockShader(gl::ShaderDef().texture()));
		gl::drawCube(vec3(0), vec3(2.2f));
	}

	// show the FBO color texture in the upper left corner
	// gl::setMatricesWindow(toPixels(getWindowSize()));
	// gl::draw(mFbo->getColorTexture(), Rectf(0, 0, 128, 128));
	// and draw the depth texture adjacent
	// gl::draw(mFbo->getDepthTexture(), Rectf(128, 0, 256, 128));
	
	//
	// SPOUT
	//
	// Show sender details unless sending the screen fbo
	// showInfo();


#endif

}

// SPOUT
#ifdef _receiver
	void FboBasicApp::mouseDown(MouseEvent event)
	{
		// Select a sender
		// SpoutPanel.exe is used to select senders
		// and is detected after SpoutSettings has been run once.
		if (event.isRightDown()) {
			receiver.SelectSender();
		}
	}
#endif

// Close the sender or receiver on exit
void FboBasicApp::quit()
{
#ifdef _receiver
	receiver.ReleaseReceiver();
#else
	sender.ReleaseSender();
#endif
}

// Show sender or receiver information
void FboBasicApp::showInfo()
{
	std::string str;
	gl::color(Color::white());
	gl::setMatricesWindow(getWindowSize());
	gl::enableAlphaBlending();

#ifdef _receiver
	if (receiver.IsConnected()) {
		str = "Receiving : [";
		str += receiver.GetSenderName(); str += "] (";
		str += std::to_string(receiver.GetSenderWidth()); str += "x";
		str += std::to_string(receiver.GetSenderHeight()); str += ") ";
		// Show sender fps and framecount
		// Applications < 2.007 will return no frame count information
		str += "fps : ";
		if (receiver.GetSenderFrame() > 0) {
			str += std::to_string((int)receiver.GetSenderFps());
			str += " frame : ";
			str += std::to_string(receiver.GetSenderFrame());
		}
		else {
			str += std::to_string((int)getAverageFps());
		}
		gl::drawString(str, vec2(toPixels(10), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(20)));
		gl::drawString("RH click - select sender", vec2(toPixels(10), toPixels((float)getWindowHeight()-30)), Color(1, 1, 1), Font("Verdana", toPixels(20)));
	}
	else {
		gl::drawString("No sender detected", vec2(toPixels(10), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(20)));
	}

#else
	str = "Sending as [";
	str += sender.GetName(); str += "] (";
	str += std::to_string(sender.GetWidth()); str += "x";
	str += std::to_string(sender.GetHeight()); str += ") ";
	str += "fps : ";
	if (sender.GetFrame() > 0) {
		str += std::to_string((int)sender.GetFps());
		str += " frame : ";
		str += std::to_string(sender.GetFrame());
	}
	else {
		str += std::to_string((int)getAverageFps());
	}
	gl::drawString(str, vec2(toPixels(20), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(20)));
#endif
	gl::disableAlphaBlending();

}


CINDER_APP( FboBasicApp, RendererGl )
