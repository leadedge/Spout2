/*

    jit.gl.spoutreceiver.c

	Based on :
		jit.gl.simple by Cycling74
		and jit.gl.syphonSender.m
	    Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).

	=================== SPOUT 2 ===================
	03-07-14 - major change to use Spout SDK
	02-08-14 - compiled /MT
			 - Fixed dest_changed error
			 - enabled memoryshare for receiver creation
			 - TODO : memoryshare needs a local texture for it to be returned into
	04-08-14 - Compiled for DX9
			 - Included local OpenGL texture for memoryshare mode
	05-08-14 - Memoryshare working
	10-08-14 - Updated for testing - DX9 mode
	13-08-14 - corrected context change texture handle leak
	14-08-14 - interop class corrected texture delete without context
	24.08.14 - recompiled with MB sendernames class revision
	01.09.14 - changes to Interop class to ensure texture and fbo are deleted and set to zero
	03.09.14 - dest changed cycle delay removed
			 - error due to not setting texture and fbo ids to zero after release
			 - subsequent release failed and caused Jitter errors.
	30.09.14 - Updated for DirectX 11 and revised SDK
	09.10.14 - Cleanup for release
	12.10.14 - Recompile for release - Version 2.001
	20.10.14 - Recompile for update V 2.001 beta
			 - Version 2.002
	04.02.15 - Compile for DX9 and DX11 for SDK update
			 - Vers 2.003
	14.02.15 - added auto detection in SpoutGLDXinterop so can leave as DX11 default
			 - added Optimus enablement export
			 - Vers 2.004
	25.04.15 - Changed from graphics auto detection to set DirectX mode to optional installer
			   Version 2.005
	26.05.15 - Recompile for revised SpoutPanel registry write of sender name
			   Version 2.006

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Copyright (c) 2014, Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 */

// Temporary debugging define for ableton test 
// patch needing servers instead of senders
// ** Must be changed in Max file as well **
// #define UseServers

// Compile for DX11 instead of DX9 (default)
// A DX11 receiver can receive from both DX9 and DX11 senders
// so compiling for DX9 is not necessary dependent on NVIDIA driver bug (10-08-14)
// 14.02.15 - added auto detection in SpoutGLDXinterop
// 25.04.15 - changed to optional installation rather than auto-detect
// #define UseD3D9

#include "jit.common.h"
#include "jit.gl.h"
#include "jit.gl.ob3d.h"
#include "ext_obex.h"
#include "string"
#include "../../SpoutSDK/Spout.h"

// This allows the Optimus global 3d setting to be "adapt" instead of "high performance"
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv);

typedef struct _jit_gl_spout_receiver 
{
	// Max object
	t_object ob;	

	// 3d object extension.  This is what all objects in the GL group have in common.
	void *ob3d;
		
	// attributes
	#ifdef UseServers
	t_symbol *servername; // Use for sharing name
	#else
	t_symbol *sendername; // Use for sharing name
	#endif

	t_symbol *texturename;	

	long update; // update to the active sender
	long aspect; // retain aspect ratio of input texture
	long memoryshare; // memory share instead of interop directx texture share
	long dim[2]; // output dim

	// Our Spout receiver object
	SpoutReceiver * myReceiver;

	unsigned int g_Width, g_Height;
	char         g_SenderName[256];
	GLuint       g_GLtexture; // local utility texture
	bool         bInitialized;

	// internal jit.gl.texture object
	t_jit_object *output;
	
} t_jit_gl_spout_receiver;

void *_jit_gl_spout_receiver_class;

//
// Function Declarations
//

// init/constructor/free
t_jit_err jit_gl_spout_receiver_init(void);
t_jit_gl_spout_receiver *jit_gl_spout_receiver_new(t_symbol * dest_name);
void jit_gl_spout_receiver_free(t_jit_gl_spout_receiver *x);

// handle context changes - need to rebuild IOSurface + textures here.
t_jit_err jit_gl_spout_receiver_dest_closing(t_jit_gl_spout_receiver *x);
t_jit_err jit_gl_spout_receiver_dest_changed(t_jit_gl_spout_receiver *x);

// draw;
t_jit_err jit_gl_spout_receiver_draw(t_jit_gl_spout_receiver *x);
t_jit_err jit_gl_spout_receiver_draw2(t_jit_gl_spout_receiver *x);
t_jit_err jit_gl_spout_receiver_drawto(t_jit_gl_spout_receiver *x, t_symbol *s, int argc, t_atom *argv);

// attributes
// @sendername, for Sender name
#ifdef UseServers
t_jit_err jit_gl_spout_receiver_servername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);
#else
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);
#endif


// @update  update to active Sender
t_jit_err jit_gl_spout_receiver_update(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// @aspect to retain shared texture aspect ratio
t_jit_err jit_gl_spout_receiver_aspect(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// @memoryshare 0 / 1 force memoryshare
t_jit_err jit_gl_spout_receiver_memoryshare(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// @texturename to read a named texture.
t_jit_err jit_gl_spout_receiver_texturename(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// @out_name for output...
t_jit_err jit_gl_spout_receiver_getattr_out_name(t_jit_gl_spout_receiver *x, void *attr, long *ac, t_atom **av);

// @dim - dimension output
t_jit_err jit_gl_spout_receiver_setattr_dim(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);


// symbols
#ifdef UseServers
t_symbol *ps_servername;
#else
t_symbol *ps_sendername;
#endif
t_symbol *ps_texture;
t_symbol *ps_width;
t_symbol *ps_height;
t_symbol *ps_glid;
t_symbol *ps_target;
t_symbol *ps_automatic;
t_symbol *ps_flip;
t_symbol *ps_drawto;
t_symbol *ps_draw;

// for our internal texture
extern t_symbol *ps_jit_gl_texture;

//
// Function implementations
//
bool InitTexture(t_jit_gl_spout_receiver *x);

//
// Init, New, Cleanup, Context changes
//

t_jit_err jit_gl_spout_receiver_init(void) 
{

	// setup our OB3D flags to indicate our capabilities.
	long ob3d_flags = JIT_OB3D_NO_MATRIXOUTPUT; // no matrix output
	ob3d_flags |= JIT_OB3D_NO_ROTATION_SCALE;
	ob3d_flags |= JIT_OB3D_NO_POLY_VARS;
	ob3d_flags |= JIT_OB3D_NO_FOG;
	ob3d_flags |= JIT_OB3D_NO_MATRIXOUTPUT;
	ob3d_flags |= JIT_OB3D_NO_LIGHTING_MATERIAL;
	ob3d_flags |= JIT_OB3D_NO_DEPTH;
	ob3d_flags |= JIT_OB3D_NO_COLOR;

	_jit_gl_spout_receiver_class = jit_class_new("jit_gl_spout_receiver", 
										 (method)jit_gl_spout_receiver_new, (method)jit_gl_spout_receiver_free,
										 sizeof(t_jit_gl_spout_receiver),A_DEFSYM,0L);
	
	// set up object extension for 3d object, customized with flags
	void *ob3d;
	ob3d = jit_ob3d_setup(_jit_gl_spout_receiver_class, 
						  calcoffset(t_jit_gl_spout_receiver, ob3d), 
						  ob3d_flags);


	
	// OB3D methods

	// define the dest_closing and dest_changed methods. 
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_dest_closing, "dest_closing", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_dest_changed, "dest_changed", A_CANT, 0L);

	// define our OB3D draw method.
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_draw, 
						"ob3d_draw", A_CANT, 0L);


	// must register for ob3d use
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_object_register, "register", A_CANT, 0L);
	
	// add attributes

	// INPUTS
	long attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;

	t_jit_object *attr;
    
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset_array,
											"dim",
											_jit_sym_long,
											2,
											attrflags,
											(method)0L,
											(method)jit_gl_spout_receiver_setattr_dim,
											0/*fix*/,
											calcoffset(t_jit_gl_spout_receiver,dim));
    jit_class_addattr(_jit_gl_spout_receiver_class,attr);	
	
	// Enter a sender name
	#ifdef UseServers
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"servername",
										  _jit_sym_symbol,attrflags,
										   (method)0L,
										   jit_gl_spout_receiver_servername, 
										   calcoffset(t_jit_gl_spout_receiver, servername));
	#else
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"sendername",
										  _jit_sym_symbol,attrflags,
										   (method)0L,
										   jit_gl_spout_receiver_sendername, 
										   calcoffset(t_jit_gl_spout_receiver, sendername));
	#endif

	jit_class_addattr(_jit_gl_spout_receiver_class, attr);
	
	// Update to latest sender
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, "update", 
										_jit_sym_long, attrflags, 
										 (method)0L, 
										 (method)jit_gl_spout_receiver_update, 
										 calcoffset(t_jit_gl_spout_receiver, update));

	jit_attr_addfilterset_clip(attr, 0, 1, TRUE, TRUE);	// Must be 0 to 1
	jit_class_addattr(_jit_gl_spout_receiver_class, attr);

	// Retain aspect of shared texture
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, "aspect", 
										_jit_sym_long, attrflags, 
										 (method)0L, 
										 (method)jit_gl_spout_receiver_aspect, 
										 calcoffset(t_jit_gl_spout_receiver, aspect));
	jit_attr_addfilterset_clip(attr, 0, 1, TRUE, TRUE);	// Must be 0 to 1
	jit_class_addattr(_jit_gl_spout_receiver_class, attr);

	// force memory share
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"memoryshare", _jit_sym_long, attrflags,
		(method)0L, (method)jit_gl_spout_receiver_memoryshare, calcoffset(_jit_gl_spout_receiver, memoryshare));	
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);


	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"texturename",_jit_sym_symbol,attrflags,
						  (method)0L,(method)jit_gl_spout_receiver_texturename,calcoffset(t_jit_gl_spout_receiver, texturename));		
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);	

	
	// OUTPUT
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_OPAQUE_USER;

	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"out_name",_jit_sym_symbol, attrflags,
						  (method)jit_gl_spout_receiver_getattr_out_name,(method)0L,0);	
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);

	//symbols
	#ifdef UseServers
	ps_servername = gensym("servername");
	#else
	ps_sendername = gensym("sendername");
	#endif

	ps_texture = gensym("texture");
	ps_width = gensym("width");
	ps_height = gensym("height");
	ps_glid = gensym("glid");
	ps_target = gensym("target");
	ps_automatic = gensym("automatic");
	ps_flip = gensym("flip");
	ps_drawto = gensym("drawto");
	ps_draw = gensym("draw");
	
	jit_class_register(_jit_gl_spout_receiver_class);

	return JIT_ERR_NONE;
}

t_jit_gl_spout_receiver *jit_gl_spout_receiver_new(t_symbol * dest_name)
{
	t_jit_gl_spout_receiver *x;

	// make jit object
	if ((x = (t_jit_gl_spout_receiver *)jit_object_alloc(_jit_gl_spout_receiver_class))) {
		
		// Initialize variables
		x->update          = 0;     // update to active Sender
		x->aspect          = 0;     // preserve aspect ratio of incoming texture
		x->memoryshare     = false; // user memory mode flag
		x->g_Width         = 320;   // give it an initial image size
		x->g_Height        = 240;
		x->g_SenderName[0] = 0;     // means it will try to find the active sender when it starts
		x->g_GLtexture     = NULL;  // local OpenGL texture for memoryshare mode
		x->bInitialized    = false; // not initialized yet

		// Create a new Spout receiver
		x->myReceiver      = new SpoutReceiver;

		#ifdef UseD3D9
		x->myReceiver->SetDX9(true); // Set to DX9 for compatibility with Version 1 apps
		#else
		x->myReceiver->SetDX9(false);
		#endif

		// Syphon comment : TODO : is this right ?  LJ not sure
		// set up attributes
		#ifdef UseServers
		jit_attr_setsym(x->servername, _jit_sym_name, gensym("servername"));
		#else
		jit_attr_setsym(x->sendername, _jit_sym_name, gensym("sendername"));
		#endif

		// instantiate a single internal jit.gl.texture for output
		x->output = (t_jit_object *)jit_object_new(ps_jit_gl_texture, dest_name);

		if (x->output) {
			x->texturename = jit_symbol_unique();
			// set texture attributes.
			jit_attr_setsym(x->output,  _jit_sym_name, x->texturename);
			jit_attr_setsym(x->output,  gensym("defaultimage"), gensym("black"));
			jit_attr_setlong(x->output, gensym("rectangle"), 1);
			jit_attr_setlong(x->output, gensym("flip"), 0);
			x->dim[0] = 320;
			x->dim[1] = 240;
			// Set texture size
			jit_attr_setlong_array(x->output, _jit_sym_dim, 2, x->dim);
        } 
		else {
			jit_object_error((t_object *)x,"jit.gl.spoutReceiver: could not create texture");
			x->texturename = _jit_sym_nothing;		
		}
		
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);


	} 
	else {
		x = NULL;
	}

	return x;
}

void jit_gl_spout_receiver_free(t_jit_gl_spout_receiver *x)
{

	// Free the memoryshare input texture if there is one
	if(x->g_GLtexture) glDeleteTextures(1, &x->g_GLtexture);

	// free our internal texture
	if(x->output) jit_object_free(x->output);

	// free our ob3d data 
	if(x) jit_ob3d_free(x);
	
	// Release the receiver
	x->myReceiver->ReleaseReceiver();

	// Delete the Receiver object last.
	if(x->myReceiver) delete x->myReceiver;
	x->myReceiver = NULL;
}

t_jit_err jit_gl_spout_receiver_dest_closing(t_jit_gl_spout_receiver *x)
{
	// In case we have dest-closing without dest_changed
	// or dest_changed without dest_closing - allow for both.
	// Release sender if initialized
	if(x->bInitialized)	x->myReceiver->ReleaseReceiver();
	x->bInitialized = false; // Initialize again in draw

	return JIT_ERR_NONE;
}

t_jit_err jit_gl_spout_receiver_dest_changed(t_jit_gl_spout_receiver *x)
{	

	//
	// Release receiver if initialized
	//
	// IMPORTANT : do not re-initialize here but do so next round in draw
	//
	if(x->bInitialized) x->myReceiver->ReleaseReceiver();
	x->bInitialized = false; // Initialize again in draw

	// Syphon comment : our texture has to be bound in the new context before we can use it
	// http://cycling74.com/forums/topic.php?id=29197
	// Result otherwise is a white screen
	if (x->output) {
		t_jit_gl_context ctx = jit_gl_get_context();
		t_symbol *context = jit_attr_getsym(x, ps_drawto);
		jit_attr_setsym(x->output, ps_drawto, context);
		t_jit_gl_drawinfo drawInfo;
		t_symbol *texName = jit_attr_getsym(x->output, gensym("name"));
		jit_gl_drawinfo_setup(x, &drawInfo);
		jit_gl_bindtexture(&drawInfo, texName, 0);
		jit_gl_unbindtexture(&drawInfo, texName, 0);
	}

	return JIT_ERR_NONE;
}

// 
// Draw
//

t_jit_err jit_gl_spout_receiver_drawto(t_jit_gl_spout_receiver *x, t_symbol *s, int argc, t_atom *argv)
{
	object_attr_setvalueof(x->output, s, argc, argv);	
	jit_ob3d_dest_name_set((t_jit_object *)x, NULL, argc, argv);
	return JIT_ERR_NONE;
}


t_jit_err jit_gl_spout_receiver_draw(t_jit_gl_spout_receiver *x)
{
	t_jit_err result = JIT_ERR_NONE;
	float vpdim[4]; // for saving the viewport dimensions
	float fx, fy, as, vpScaleX, vpScaleY, vpWidth, vpHeight;
	int vpx, vpy;
	long newdim[2];	// output dim
	unsigned int senderWidth = 0;
	unsigned int senderHeight = 0;
	GLint previousFBO = 0;      
	GLint previousMatrixMode = 0;
	GLint previousActiveTexture = 0;

	if (!x) {
		return JIT_ERR_INVALID_PTR;
	}

	// We need the Jitter texture ID, width and height.
	GLuint texname	= jit_attr_getlong(x->output, ps_glid);
	GLuint width	= jit_attr_getlong(x->output, ps_width);
	GLuint height	= jit_attr_getlong(x->output, ps_height);

	// TODO: necessary ? JKC says no unless context changed above? should be set during draw for you. 
	// LJ - seems necessary
	t_jit_gl_context ctx = jit_gl_get_context();
	jit_ob3d_set_context(x);

	// Save FBO etc
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
	
	// find the viewport size in order to scale to the aspect ratio
	glGetFloatv(GL_VIEWPORT, vpdim);

	// Save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// Syphon note :
	// Jitter uses multiple texture coordinate arrays on different units
	// http://s-musiclab.jp/mmj_docs/max5/develop/MaxSDK-5.1.1_J/html/jit_8gl_8texture_8c_source.html
	// We need to ensure we set this before changing our texture matrix
	// glActiveTexture selects which texture unit subsequent texture state calls will affect.
	glActiveTexture(GL_TEXTURE0);

	// ensure we act on the proper client texture as well
	glClientActiveTexture(GL_TEXTURE0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	// find the viewport size in order to scale to the aspect ratio
	glGetFloatv(GL_VIEWPORT, vpdim);

	// Scale width and height to the current viewport size
	vpScaleX = width/(float)x->g_Width;			// vpdim[2]/(float)x->g_Width;
	vpScaleY = height/(float)x->g_Height;		// vpdim[3]/(float)x->g_Height;
	vpWidth  = (float)x->g_Width  * vpScaleX;
	vpHeight = (float)x->g_Height * vpScaleY;
	vpx = vpy = 0.0;

	// User selection flag to preserve aspect ratio or not
	if(x->aspect == 1) {
		// back to original aspect ratio
		as = (float)x->g_Width/(float)x->g_Height;
		if(x->g_Width > x->g_Height) {
			fy = vpWidth/as;
			vpy = (int)(vpHeight-fy)/2;
			vpHeight = fy;
		}
		else {
			fx = vpHeight/as;
			vpx = (int)(vpWidth-fx)/2;
			vpWidth = fx;
		}
	}
	glViewport((int)vpx, (int)vpy, (int)vpWidth, (int)vpHeight);

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);
                               
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	// Try to create a receiver if not initialized
	// If sendername is null it will pick up the active sender
	// Otherwise it will wait for the correct name to be entered
	if(!x->bInitialized) {

		// Set memoryshare mode if the user requested it
		// Needs a local texture to receive the memoryshare result
		if(x->memoryshare == 1) x->myReceiver->SetMemoryShareMode(true);

		if(x->myReceiver->CreateReceiver(x->g_SenderName, senderWidth, senderHeight)) {

			x->g_Width	= senderWidth;
			x->g_Height	= senderHeight;
			
			// For memoryshare create a local OpenGL texture of the same size
			if(x->memoryshare == 1) InitTexture(x);
			
			// Update output texture dim to the new size
			newdim[0] = x->g_Width;
			newdim[1] = x->g_Height;
			jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim);  // LJ DEBUG - should be x-output ?
			
			x->bInitialized = true;
		}
	} // endif not initialized

	// Did it initialize ?
	if(x->bInitialized) {

		// Receive a shared texture and return it's width and height
		// For MemoryShare there will be a valid texture, otherwise it will be NULL

		// Necessary for memoryshare size change check
		senderWidth = x->g_Width;
		senderHeight = x->g_Height;
	
		if(x->myReceiver->ReceiveTexture(x->g_SenderName, senderWidth, senderHeight, x->g_GLtexture, GL_TEXTURE_2D)) {

			// Test for change of texture size
			if(senderWidth != x->g_Width || senderHeight != x->g_Height) {

				// Set global width and height
				x->g_Width	= senderWidth;
				x->g_Height	= senderHeight;
			
				// For memoryshare create a local OpenGL texture of the same size
				if(x->memoryshare == 1) InitTexture(x);

				// Update output dim to the new size
				newdim[0] = x->g_Width;
				newdim[1] = x->g_Height;
				jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim);  // LJ DEBUG - should be x-output ?

			}
			else {

				// We have a shared texture and can render into the jitter texture

				// An FBO for render to texture
				GLuint tempFBO;
				glGenFramebuffersEXT(1, &tempFBO);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO); 

				// Attach the jitter texture (destination) to the color buffer in our frame buffer  
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, texname, 0);
				if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {

					// If memoryshare, draw the local OpenGL texture into it
					if(x->memoryshare == 1) {
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, x->g_GLtexture);
						glColor3f(1.0,  1.0, 1.0);
						glBegin(GL_QUADS);
						glTexCoord2d(0, 0);
						glVertex2d(-1,  1); // lower left
						glTexCoord2d(0, 1);	
						glVertex2d(-1, -1); // upper left
						glTexCoord2d(1, 1);	
						glVertex2d( 1, -1); // upper right
						glTexCoord2d(1, 0);	
						glVertex2d( 1,  1); // lower right
						glEnd();
						glBindTexture(GL_TEXTURE_2D, 0);
						glDisable(GL_TEXTURE_2D);
					}
					else {
						// Otherwise draw the shared texture straight into it
						x->myReceiver->DrawSharedTexture();
					}
				}
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
				glDeleteFramebuffersEXT(1, &tempFBO);
				tempFBO = 0;
			}
		} // Received texture OK
		else {
			// If it doesn't get a texture, the sender is closed
			x->myReceiver->ReleaseReceiver();
			x->bInitialized = false; // Try to find another
		}
	} // endif intialized

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(previousMatrixMode);

	glActiveTexture(previousActiveTexture);

	// ensure we act on the proper client texture as well
	glPopClientAttrib();
	glClientActiveTexture(previousActiveTexture);

	glPopAttrib();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);

	// Restore the viewport
	glViewport(vpdim[0], vpdim[1], vpdim[2], vpdim[3]);

	// Restore context
	jit_gl_set_context(ctx);

	return JIT_ERR_NONE;

}

		
//
// attributes
//

// @Senderuuid
#ifdef UseServers
t_jit_err jit_gl_spout_receiver_servername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
#else
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
#endif
{
	t_symbol *srvname;
	char name[256];

	if(x) {	
		if (argc && argv) {
			srvname = jit_atom_getsym(argv);
			#ifdef UseServers
			x->servername = srvname;
			strcpy_s(name, 256, x->servername->s_name);
			#else
			x->sendername = srvname;
			strcpy_s(name, 256, x->sendername->s_name);
			#endif
			if(strcmp(x->g_SenderName, name) != 0) { // different name
				strcpy_s(x->g_SenderName, 256, name);
				x->myReceiver->ReleaseReceiver();
				x->bInitialized = false;
				return JIT_ERR_NONE;
			}
		} 
		else {
			// no args, set to zero
			#ifdef UseServers
			x->servername = _jit_sym_nothing;
			#else
			x->sendername = _jit_sym_nothing;
			#endif
			x->g_SenderName[0] = 0;
		}
	}

	return JIT_ERR_NONE;
}

// #texturename
t_jit_err jit_gl_spout_receiver_texturename(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	t_symbol *s=jit_atom_getsym(argv);

	if(s->s_name) {
		x->texturename = s;
		if (x->output) jit_attr_setsym(x->output, _jit_sym_name, s);
		jit_attr_setsym(x,ps_texture,s);
	}

	return JIT_ERR_NONE;
}

// @update - activate SpoutPanel to get the active Sender
t_jit_err jit_gl_spout_receiver_update(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	long c = jit_atom_getlong(argv);

	x->update = c; // Button - check down or up ?

	x->myReceiver->SelectSenderPanel();

	return JIT_ERR_NONE;
}


// @aspect - retain aspect ratio of shared texture
t_jit_err jit_gl_spout_receiver_aspect(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	long c = jit_atom_getlong(argv);

	x->aspect = c;

	return JIT_ERR_NONE;
}


// @memoryshare
// force memory share flag (default is 0 off - then is automatic dependent on hardware)
t_jit_err jit_gl_spout_receiver_memoryshare(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	long c = jit_atom_getlong(argv);

	// Bypass compiler warning
	bool bC = true;
	if(c == 0) bC = false;

	x->memoryshare = c; // 0 off or 1 on
	x->myReceiver->SetMemoryShareMode(bC); // Memoryshare on or off

	// Set requested memoryshare mode and start again
	if(c == 0) {
		// If turning off, make sure the local OpenGL texture is erased
		if(x->g_GLtexture) glDeleteTextures(1, &x->g_GLtexture);
		x->g_GLtexture = NULL;
	}
	
	x->myReceiver->ReleaseReceiver();
	x->bInitialized = false;

	return JIT_ERR_NONE;
}


t_jit_err jit_gl_spout_receiver_getattr_out_name(t_jit_gl_spout_receiver *x, void *attr, long *ac, t_atom **av)
{

	if ((*ac)&&(*av)) {
		//memory passed in, use it
	} else {
		//otherwise allocate memory
		*ac = 1;
		if (!(*av = (t_atom *)jit_getbytes(sizeof(t_atom)*(*ac)))) {
			*ac = 0;
			return JIT_ERR_OUT_OF_MEM;
		}
	}
	
	jit_atom_setsym(*av, jit_attr_getsym(x->output, _jit_sym_name));

	return JIT_ERR_NONE;
}											  


t_jit_err jit_gl_spout_receiver_setattr_dim(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
    long i;
	long v;

	if (x) 	{

		for(i = 0; i < JIT_MATH_MIN(argc, 2); i++) {
			v = jit_atom_getlong(argv+i);
			if (x->dim[i] != JIT_MATH_MIN(v,1))	{
				x->dim[i] = v;
			}
		}

        // update internal output texture size
        jit_attr_setlong_array(x->output, _jit_sym_dim, 2, x->dim);

		return JIT_ERR_NONE;
	}
	return JIT_ERR_INVALID_PTR;

}


// ===================================
t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv)
{
	return JIT_ERR_NONE;

}

// Create a local RGB texture for memoryshare transfers
bool InitTexture(t_jit_gl_spout_receiver *x)
{
	if(x->g_Width == 0 || x->g_Height == 0) {
		return JIT_ERR_INVALID_PTR;
	}

	if(x->g_GLtexture) glDeleteTextures(1, &x->g_GLtexture);
	glGenTextures(1, &x->g_GLtexture);

	glBindTexture(GL_TEXTURE_2D, x->g_GLtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x->g_Width, x->g_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return JIT_ERR_NONE;
}
