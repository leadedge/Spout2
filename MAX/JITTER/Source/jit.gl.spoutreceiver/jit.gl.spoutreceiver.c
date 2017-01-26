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
	24.07.15 - Revised for Max 7 and 64 bit
	25.07.15 - Changed SpoutGLDXinterop so that the OpenGL texture and fbo are not re-created
	26.07.15 - Created utilities for Save and restore OpenGL state
			 - Save/restore state in dest_closing for release of receiver
			 - Introduced flags for closing and changed to ensure that all is done before creating the receiver again
			 - Moved ReleaseReceiver from jit_gl_spout_receiver_sendername to draw
	01.08.15 - Recompiled for Spout 2.004 - 32 bit VS2010 - Version 2.007.10
	01.08.15 - Recompiled for Spout 2.004 - 64bit VS2012 - Version 2.007.12
	29.10.15 - Testing of frame number output
	07.11.15 - Frame numbering completed.
	11.11.15 - Recompiled with corrections to fbo extensions in SpoutGLextensions.cpp
			 - cleanup for 2.005 testing 
	30.03.16 - rebuild for 2.005 release with Max 7.1 SDK
			 - 64bit VS2012 - Version 2.008.12
			 - 32bit VS2012 - Version 2.008.12
	01.04.16 - Recompile VS2012 /MT - needed removal of "libcmt.lib"
			 - from Linker "ignore specific libraries".
			 - 64bit VS2012 - Spout 2.005 - Version 2.009.12
			 - 32bit VS2012 - Spout 2.005 - Version 2.009.12
	16.05.16 - Changed Version numbering to allow the Max Package manager
			   to show 2.0.4 -> 2.0.5 for the package, VS2010 option removed.
	20.06.16 - Removed frame number testing
			 - Added check for jitter texture in draw
			 - Added check for zero dimensions in SaveOpenGLstate
	21.06.16 - Recompiled /MT Spout 2.005 - 64bit and 32bit VS2012 - Version 2.0.5.10
	23.06.16 - change back to 2.004 logic for access locks for texture read/write
	26.01.17 - Rebuild for Spout 2.006 VS2012 /MT - Vers 2.006.0

			 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Copyright (c) 2016-2017, Lynn Jarvis. All rights reserved.

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

#include "jit.common.h"
#include "jit.gl.h"
#include "jit.gl.ob3d.h"
#include "ext_obex.h"
#include "string"
#include "../../SpoutSDK/Spout.h"


t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv);

typedef struct _jit_gl_spout_receiver 
{
	// Max object
	t_object ob;	

	// 3d object extension.  This is what all objects in the GL group have in common.
	void *ob3d;
		
	// attributes
	t_symbol *sendername; // Use for sharing name
	t_symbol *texturename;	

	long update; // update to the active sender
	long aspect; // retain aspect ratio of input texture
	long memoryshare; // memory share instead of interop directx texture share
	t_atom_long  dim[2]; // dimension input

	// Our Spout receiver object
	SpoutReceiver * myReceiver;
	unsigned int g_Width, g_Height;
	char         g_SenderName[256];
	GLuint       g_GLtexture; // local utility texture

	bool         bInitialized;
	bool         bDestClosing;
	bool         bDestChanged;
	bool         bNameChanged;

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

// handle context changes - need to rebuild textures here.
t_jit_err jit_gl_spout_receiver_dest_closing(t_jit_gl_spout_receiver *x);
t_jit_err jit_gl_spout_receiver_dest_changed(t_jit_gl_spout_receiver *x);

// draw;
t_jit_err jit_gl_spout_receiver_draw(t_jit_gl_spout_receiver *x);
t_jit_err jit_gl_spout_receiver_drawto(t_jit_gl_spout_receiver *x, t_symbol *s, int argc, t_atom *argv);

// attributes
// @sendername, for Sender name
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// @update  update to active Sender
t_jit_err jit_gl_spout_receiver_update(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// @aspect to retain shared texture aspect ratio
t_jit_err jit_gl_spout_receiver_aspect(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// @texturename to read a named texture.
t_jit_err jit_gl_spout_receiver_texturename(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// @out_name for texture output...
t_jit_err jit_gl_spout_receiver_getattr_out_name(t_jit_gl_spout_receiver *x, void *attr, long *ac, t_atom **av);

// @dim - dimension input
t_jit_err jit_gl_spout_receiver_setattr_dim(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// Utility
void SaveOpenGLstate(t_jit_gl_spout_receiver *x,  GLuint width, GLuint height, GLint &previousFBO, GLint &previousMatrixMode,  GLint &previousActiveTexture, float *vpdim);
void RestoreOpenGLstate(t_jit_gl_spout_receiver *x, GLint previousFBO, GLint previousMatrixMode,  GLint previousActiveTexture, float *vpdim);

// symbols
t_symbol *ps_sendername;
t_symbol *ps_texture;
t_symbol *ps_width;
t_symbol *ps_height;
t_symbol *ps_glid;
t_symbol *ps_target;
t_symbol *ps_automatic;
t_symbol *ps_flip;
t_symbol *ps_drawto;
t_symbol *ps_draw;

// To give back to Max
extern t_symbol *ps_out_texture; // for our internal texture

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
						(method)jit_gl_spout_receiver_draw,	"ob3d_draw", A_CANT, 0L);

	// must register for ob3d use
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_object_register, "register", A_CANT, 0L);
	
	//
	// add attributes
	//

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
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"sendername",
										  _jit_sym_symbol,attrflags,
										   (method)0L,
										   jit_gl_spout_receiver_sendername, 
										   calcoffset(t_jit_gl_spout_receiver, sendername));

	jit_class_addattr(_jit_gl_spout_receiver_class, attr);
	
	// Update - open SpoutPanel to select a sender (currently not used)
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


	// Use a named texture
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"texturename",_jit_sym_symbol,attrflags,
						  (method)0L,(method)jit_gl_spout_receiver_texturename,calcoffset(t_jit_gl_spout_receiver, texturename));		
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);	

	//
	// OUTPUT
	//
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_OPAQUE_USER;

	// Texture output
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"out_name",_jit_sym_symbol, attrflags,
						  (method)jit_gl_spout_receiver_getattr_out_name,(method)0L,0);	
	jit_class_addattr(_jit_gl_spout_receiver_class, attr);

	//
	// symbols
	//
	ps_sendername = gensym("sendername");
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
		x->memoryshare     = 0;     // user memory mode flag
		x->g_Width         = 320;   // give it an initial image size
		x->g_Height        = 240;
		x->g_SenderName[0] = 0;     // initial NULL means it will try to find the active sender when it starts
		x->g_GLtexture     = NULL;  // local OpenGL texture for memoryshare mode
		x->bInitialized    = false; // not initialized yet
		x->bDestClosing    = false;
		x->bDestChanged    = false;
		x->bNameChanged    = false;

		// Create a new Spout receiver
		x->myReceiver      = new SpoutReceiver;

		// Syphon comment : TODO : is this right ?  LJ not sure
		// set up attributes
		jit_attr_setsym(x->sendername, _jit_sym_name, gensym("sendername"));

		// instantiate a single internal jit.gl.texture for output
		x->output = (t_jit_object *)jit_object_new(ps_out_texture, dest_name);

		if (x->output) {
			x->texturename = jit_symbol_unique();
			// set texture attributes.
			jit_attr_setsym(x->output,  _jit_sym_name, x->texturename);
			jit_attr_setsym(x->output,  gensym("defaultimage"), gensym("black"));
			jit_attr_setlong(x->output, gensym("rectangle"), 1);
			jit_attr_setlong(x->output, gensym("flip"), 0);
			// Set dimension output
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
	if(x->bInitialized)	x->myReceiver->ReleaseReceiver();

	// Delete the Receiver object last.
	if(x->myReceiver) delete x->myReceiver;
	x->myReceiver = NULL;
}

t_jit_err jit_gl_spout_receiver_dest_closing(t_jit_gl_spout_receiver *x)
{

	float vpdim[4];
	GLint previousFBO;
    GLint previousMatrixMode;
	GLint previousActiveTexture;
	GLuint width  = (GLuint)jit_attr_getlong(x->output, ps_width);
	GLuint height = (GLuint)jit_attr_getlong(x->output, ps_height);

	// Release receiver if initialized
	if(x->bInitialized)	{
		SaveOpenGLstate(x, width, height, previousFBO, previousMatrixMode, previousActiveTexture, vpdim);
		x->myReceiver->ReleaseReceiver();
		RestoreOpenGLstate(x, previousFBO, previousMatrixMode, previousActiveTexture, vpdim);
	}

	x->bInitialized = false; // Initialize again in draw
	x->bDestClosing = true;

	return JIT_ERR_NONE;
}

t_jit_err jit_gl_spout_receiver_dest_changed(t_jit_gl_spout_receiver *x)
{	
	// Syphon comment : our texture has to be bound in the new context before we can use it
	// http://cycling74.com/forums/topic.php?id=29197
	// Result otherwise is a white screen
	if (x->output) {
		// t_jit_gl_context ctx = jit_gl_get_context();
		t_symbol *context = jit_attr_getsym(x, ps_drawto);
		jit_attr_setsym(x->output, ps_drawto, context);
		t_jit_gl_drawinfo drawInfo;
		t_symbol *texName = jit_attr_getsym(x->output, gensym("name"));
		jit_gl_drawinfo_setup(x, &drawInfo);
		jit_gl_bindtexture(&drawInfo, texName, 0);
		jit_gl_unbindtexture(&drawInfo, texName, 0);
	}

	x->bDestChanged = true;

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

	// t_jit_err result = JIT_ERR_NONE;
	t_atom_long newdim[2]; // new output dimensions
	unsigned int senderWidth = 0;
	unsigned int senderHeight = 0;
	// bool bRet = false;

	float vpdim[4]; // for saving the viewport dimensions
	GLint previousFBO = 0;      
	GLint previousMatrixMode = 0;
	GLint previousActiveTexture = 0;

	if (!x) {
		return JIT_ERR_INVALID_PTR;
	}

	if(x->bDestClosing || x->bDestChanged) {
		// skip a frame so that the receiver misses a frame
		if(x->bDestClosing) x->bDestClosing = false;
		if(x->bDestChanged) x->bDestChanged = false;
		return JIT_ERR_NONE;
	}

	// We need the Jitter texture ID, width and height.
	GLuint texname	= (GLuint)jit_attr_getlong(x->output, ps_glid);
	GLuint width	= (GLuint)jit_attr_getlong(x->output, ps_width);
	GLuint height	= (GLuint)jit_attr_getlong(x->output, ps_height);

	if(texname == 0 || width == 0 || height == 0) {
		return JIT_ERR_NONE;
	}

	// TODO: necessary ? 
	// Syphon comment : JKC says no unless context changed above? should be set during draw for you. 
	// LJ - seems necessary - needs checking
	t_jit_gl_context ctx = jit_gl_get_context();
	jit_ob3d_set_context(x);

	// Save OpenGL state
	SaveOpenGLstate(x, width, height, previousFBO, previousMatrixMode, previousActiveTexture, vpdim);

	// First check if a new sender name has been selected
	if(x->bNameChanged) {
		if(x->bInitialized) x->myReceiver->ReleaseReceiver();
		x->bInitialized = false;
		x->bNameChanged = false;
	}

	// Try to create a receiver if not initialized
	// If sendername is null it will pick up the active sender
	// Otherwise it will wait for the correct name to be entered
	if(!x->bInitialized) {

		// Set memoryshare mode if the user requested it
		// Needs a local texture to receive the memoryshare result
		// if(x->memoryshare == 1) x->myReceiver->SetMemoryShareMode(true);

		if(x->myReceiver->CreateReceiver(x->g_SenderName, senderWidth, senderHeight)) {

			x->g_Width	= senderWidth;
			x->g_Height	= senderHeight;
			// printf("Created receiver [%s] (%dx%d)\n", x->g_SenderName, senderWidth, senderHeight);
			
			// For 2.005 check for memoryshare here
			if(x->myReceiver->GetMemoryShareMode()) x->memoryshare = 1;

			// For memoryshare create a local OpenGL texture of the same size
			if(x->memoryshare == 1)	InitTexture(x);

			// Update output texture to the new size
			newdim[0] = x->g_Width;
			newdim[1] = x->g_Height;
			jit_attr_setlong_array(x->output, _jit_sym_dim, 2, newdim); 

			// Set output dimension as well
			jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim);
	
			x->bInitialized = true;

		}

	} // endif not initialized

	// Is it initialized
	if(x->bInitialized) {

		// Receive a shared texture and return it's width and height
		// For MemoryShare there will be a valid texture, otherwise it will be NULL

		// Necessary for memoryshare size change check
		senderWidth = x->g_Width;
		senderHeight = x->g_Height;
	
		//
		// Call Receivetexture here whether memoryshare or not
		//
		// If memoryshare, the shared texture is returned in x->g_GLtexture
		// If not, all the checks for sender name and size are done anyway
		// and the shared texture can be used directly with DrawSharedTexture 
		// into the Jitter texture because the sender will have updated it
		//
		if(x->myReceiver->ReceiveTexture(x->g_SenderName, senderWidth, senderHeight, x->g_GLtexture, GL_TEXTURE_2D)) {

			// Test for change of texture size
			if(senderWidth != x->g_Width || senderHeight != x->g_Height) {
				// printf("Size change from (%dx%d) to (%dx%d)\n", x->g_Width, x->g_Height, senderWidth, senderHeight);

				// Set global width and height
				x->g_Width	= senderWidth;
				x->g_Height	= senderHeight;
			
				// For memoryshare create a local OpenGL texture of the same size
				if(x->memoryshare == 1) InitTexture(x);

				// Update output dim to the new size
				newdim[0] = x->g_Width;
				newdim[1] = x->g_Height;
				jit_attr_setlong_array(x->output, _jit_sym_dim, 2, newdim); // texture
				jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim); // dimensions

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
					// Not needed for texture share
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

	// Restore everything
	RestoreOpenGLstate(x, previousFBO, previousMatrixMode, previousActiveTexture, vpdim);

	// Restore context
	jit_gl_set_context(ctx);

	return JIT_ERR_NONE;

}

void SaveOpenGLstate(t_jit_gl_spout_receiver *x, GLuint width, GLuint height, GLint &previousFBO, GLint &previousMatrixMode,  GLint &previousActiveTexture, float *vpdim)
{

	float fx, fy, as, vpScaleX, vpScaleY, vpWidth, vpHeight;
	int vpx, vpy;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
	
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
	if(width > 0 && x->g_Width > 0 && height > 0 && x->g_Height > 0) {
		vpScaleX = width/(float)x->g_Width;			// vpdim[2]/(float)x->g_Width;
		vpScaleY = height/(float)x->g_Height;		// vpdim[3]/(float)x->g_Height;
		vpWidth  = (float)x->g_Width  * vpScaleX;
		vpHeight = (float)x->g_Height * vpScaleY;
		vpx = vpy = 0;

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
	}

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);
                               
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

}

void RestoreOpenGLstate(t_jit_gl_spout_receiver *x, GLint previousFBO, GLint previousMatrixMode,  GLint previousActiveTexture, float vpdim[4])
{
	UNREFERENCED_PARAMETER(x);

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
	glViewport((int)vpdim[0], (int)vpdim[1], (GLsizei)vpdim[2], (GLsizei)vpdim[3]);
}


//
// attributes
//

// @Senderuuid
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(attr);

	t_symbol *srvname;
	char name[256];

	if(x) {	
		if (argc && argv) {
			srvname = jit_atom_getsym(argv);
			x->sendername = srvname;
			strcpy_s(name, 256, x->sendername->s_name);
			if(strcmp(x->g_SenderName, name) != 0) { // different name
				strcpy_s(x->g_SenderName, 256, name);
				// Can't release the receiver here because the OpenGL context
				// is not guaranteed, so set a name changed flag and do it in Draw
				x->bNameChanged = true;
				return JIT_ERR_NONE;
			}
		} 
		else {
			// no args, set to zero
			x->sendername = _jit_sym_nothing;
			x->g_SenderName[0] = 0;
		}
	}

	return JIT_ERR_NONE;
}


// #texturename
t_jit_err jit_gl_spout_receiver_texturename(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(attr);
	UNREFERENCED_PARAMETER(argc);

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
	UNREFERENCED_PARAMETER(attr);
	UNREFERENCED_PARAMETER(argc);

	long c = (long)jit_atom_getlong(argv);

	x->update = c; // Button - check down or up ?

	x->myReceiver->SelectSenderPanel();

	return JIT_ERR_NONE;
}


// @aspect - retain aspect ratio of shared texture
t_jit_err jit_gl_spout_receiver_aspect(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(attr);
	UNREFERENCED_PARAMETER(argc);

	long c = (long)jit_atom_getlong(argv);
	x->aspect = c;

	return JIT_ERR_NONE;
}

t_jit_err jit_gl_spout_receiver_getattr_out_name(t_jit_gl_spout_receiver *x, void *attr, long *ac, t_atom **av)
{
	UNREFERENCED_PARAMETER(attr);

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
	UNREFERENCED_PARAMETER(attr);

 	long i;
	t_atom_long v;

	if (x) 	{

		for(i = 0; i < JIT_MATH_MIN(argc, 2); i++) {
			v = jit_atom_getlong(argv+i);
			if (x->dim[i] != JIT_MATH_MIN(v,1))	{
				x->dim[i] = v; // Update dimension
			}
		}

        // update internal output texture size as well
        jit_attr_setlong_array(x->output, _jit_sym_dim, 2, x->dim);

		return JIT_ERR_NONE;
	}
	return JIT_ERR_INVALID_PTR;

}


// ===================================
// TODO ??
t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(attr);
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	return JIT_ERR_NONE;

}

// Create a local RGBA OpenGL texture for memoryshare transfers
bool InitTexture(t_jit_gl_spout_receiver *x)
{
	if(x->g_Width == 0 || x->g_Height == 0) {
		return false;
	}

	if(x->g_GLtexture) glDeleteTextures(1, &x->g_GLtexture);
	glGenTextures(1, &x->g_GLtexture);

	glBindTexture(GL_TEXTURE_2D, x->g_GLtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x->g_Width, x->g_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
