/*

		Spout example for Cinder
		Based on the FboBasic example
		Search on SPOUT for details.
		
		2021-2023 Lynn Jarvis https://spout.zeal.co/

*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

// SPOUT
#include "..\SpoutGL\SpoutSender.h"
#include "..\SpoutGL\SpoutReceiver.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// SPOUT
// Enable this define to create a receiver
// Disable it to create a sender
// #define _receiver

// This sample shows a very basic use case for FBOs - it renders a spinning colored cube
// into an FBO, and uses that as a Texture onto the sides of a blue cube.
class FboBasicApp : public App {
  public:

	void	setup() override;
	void	update() override;
	void	draw() override;

// SPOUT
#ifdef _receiver
	void    mouseDown(MouseEvent event) override;
#endif

  private:
	void			renderSceneToFbo();
	
	gl::FboRef			mFbo;
	mat4				mRotation;
	static const int	FBO_WIDTH = 256, FBO_HEIGHT = 256;

	// SPOUT
	SpoutSender sender;
#ifdef _receiver
	SpoutReceiver receiver;
	gl::Texture2dRef spoutTexture; // texture used for receiving
#endif


};

void FboBasicApp::setup()
{
	gl::Fbo::Format format;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	mFbo = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, format.depthTexture() );

	gl::enableDepthRead();
	gl::enableDepthWrite();

	// SPOUT
	// Optional console or logging
	// OpenSpoutConsole();
	// EnableSpoutLog();

#ifdef _receiver
	// Allocate a receiving texture of any size
	spoutTexture = gl::Texture2d::create(getWindowWidth(), getWindowHeight());
#endif

}

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
}

void FboBasicApp::update()
{
	// Rotate the torus by .06 radians around an arbitrary axis
	mRotation *= rotate( 0.06f, normalize( vec3( 0.16666f, 0.333333f, 0.666666f ) ) );
	
	// SPOUT
#ifdef _receiver
	// If Updated() returns true, the sender size has changed.
	// Resize the receiving texture.
	if (receiver.IsUpdated()) {
		spoutTexture.reset();
		spoutTexture = gl::Texture2d::create(receiver.GetSenderWidth(), receiver.GetSenderHeight());
	}
#else
	// render into our FBO
	renderSceneToFbo();
#endif


}

void FboBasicApp::draw()
{
	// clear the window to gray
	gl::clear(Color(0.35f, 0.35f, 0.35f));

	// SPOUT
#ifdef _receiver
	// ReceiveTexture connects to and receives from a sender
	// Optionally include the ID of an fbo if one is currently bound
	receiver.ReceiveTexture(spoutTexture->getId(), spoutTexture->getTarget(), true);
	// Draw the texture and fill the screen if connected to a sender
	if(receiver.IsConnected())
		gl::draw(spoutTexture, getWindowBounds());
#else
	// setup our camera to render the cube
	CameraPersp cam( getWindowWidth(), getWindowHeight(), 60.0f );
	cam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	cam.lookAt( vec3( 2.6f, 1.6f, -2.6f ), vec3( 0 ) );
	gl::setMatrices( cam );

	// use the scene we rendered into the FBO as a texture
	mFbo->bindTexture();

	// draw a cube textured with the FBO
	{
		gl::ScopedGlslProg shaderScp( gl::getStockShader( gl::ShaderDef().texture() ) );
		gl::drawCube( vec3( 0 ), vec3( 2.2f ) );
	}

	// show the FBO color texture in the upper left corner
	gl::setMatricesWindow( toPixels( getWindowSize() ) );
	gl::draw( mFbo->getColorTexture(), Rectf( 0, 0, 128, 128 ) );
	// and draw the depth texture adjacent
	gl::draw( mFbo->getDepthTexture(), Rectf( 128, 0, 256, 128 ) );

	// SPOUT

	// Sending options
	sender.SendFbo(mFbo->getId(), getWindowWidth(), getWindowHeight());

	// sender.SendTexture(mFbo->getColorTexture()->getId(),
		// mFbo->getColorTexture()->getTarget(),
		// mFbo->getColorTexture()->getWidth(),
		// mFbo->getColorTexture()->getHeight());

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


CINDER_APP( FboBasicApp, RendererGl )
