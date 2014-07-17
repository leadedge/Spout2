/*

    jit.gl.spoutsender.c
    
	Based on :
		jit.gl.simple by Cycling74
		and jit.gl.syphonSender.m
	    Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).

	// =================== SPOUT 2 ===================
	//	28-06-14 - major change to use Spout SDK
	//			 - cleanup - debug comments saved in backup 28-06-14
	//	03-07-14 - saved OpenGL debugging in backup - cleanup
	//			 - used jitter texture directly for send - no difference to errors
	//	08.07-14 - cleanup
	//
 */

#include "jit.common.h"
#include "jit.gl.h"
#include "jit.gl.ob3d.h"
#include "ext_obex.h"
#include "string"

#include "../../SpoutSDK/Spout.h"


t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv);

typedef struct _jit_gl_spout_sender 
{
	// Max object
	t_object	ob;

	// 3d object extension.  This is what all objects in the GL group have in common.
	void		*ob3d;
		
	// internal jit.gl.texture object, which we use to handle matrix input.
	t_symbol	*texture;
	
	// the name of the texture we should draw from - our internal one (for matrix input) or an external one
	t_symbol	*textureSource;
	
	// attributes
	t_symbol	*sendername;	// Used for input of the sender name
	long		memoryshare;	// force memory share instead of interop directx texture share

	// Our Spout sender object
	SpoutSender * mySender;

	bool bInitialized;
	GLuint g_texId;			// jitter texture ID
	GLuint g_texTarget;		// jitter texture target
	GLuint m_fbo;			// local fbo used for texture transfers
	GLuint m_fbo_texture;	// local texture used for transfers
	int g_Width;			// width
	int g_Height;			// height
	char g_SenderName[256]; // sender name - link with sendername
	float dim[4];			// for saving viewport dimensions

} t_jit_gl_spout_sender;


void *_jit_gl_spout_sender_class;

#pragma mark -
#pragma mark Function Declarations

// init/constructor/free
t_jit_err jit_gl_spout_sender_init(void);

t_jit_gl_spout_sender *jit_gl_spout_sender_new(t_symbol * dest_name);

void jit_gl_spout_sender_free(t_jit_gl_spout_sender *x);

// handle context changes - need to rebuild spoutsender & textures here.
t_jit_err jit_gl_spout_sender_dest_closing(t_jit_gl_spout_sender *x);
t_jit_err jit_gl_spout_sender_dest_changed(t_jit_gl_spout_sender *x);

// draw
t_jit_err jit_gl_spout_sender_draw(t_jit_gl_spout_sender *x);

// handle input texture
t_jit_err jit_gl_spout_sender_jit_gl_texture(t_jit_gl_spout_sender *x, t_symbol *s, int argc, t_atom *argv);

// handle input matrix
t_jit_err jit_gl_spout_sender_jit_matrix(t_jit_gl_spout_sender *x, t_symbol *s, int argc, t_atom *argv);

// Utility
void SendTexture(t_jit_gl_spout_sender *x);
void glErrorText(char *intext);

//attributes
// @sendername, for sender human readable name
t_jit_err jit_gl_spout_sender_sendername(t_jit_gl_spout_sender *x, void *attr, long argc, t_atom *argv);

// @memoryshare 0 / 1 force memoryshare
t_jit_err jit_gl_spout_sender_memoryshare(t_jit_gl_spout_sender *x, void *attr, long argc, t_atom *argv); 

// symbols
t_symbol *ps_sendername;
t_symbol *ps_texture;
t_symbol *ps_width;
t_symbol *ps_height;
t_symbol *ps_glid;
t_symbol *ps_gltarget;
t_symbol *ps_drawto;
t_symbol *ps_jit_gl_texture; // our internal texture for matrix input

//
// Function implementations
//

#pragma mark -
#pragma mark Init, New, Cleanup, Context changes

t_jit_err jit_gl_spout_sender_init(void) 
{

	/*
	// Debug console window so printf works
	AllocConsole();
	freopen("CONIN$",  "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("\njit_gl_spout_sender\n");
	*/

	// create our class
	_jit_gl_spout_sender_class = jit_class_new("jit_gl_spout_sender", 
												(method)jit_gl_spout_sender_new, (method)jit_gl_spout_sender_free,
												sizeof(t_jit_gl_spout_sender),A_DEFSYM,0L);
	
	// setup our OB3D flags to indicate our capabilities.
	long ob3d_flags = JIT_OB3D_NO_MATRIXOUTPUT; // no matrix output
	ob3d_flags |= JIT_OB3D_NO_ROTATION_SCALE;
	ob3d_flags |= JIT_OB3D_NO_POLY_VARS;
	ob3d_flags |= JIT_OB3D_NO_FOG;
	ob3d_flags |= JIT_OB3D_NO_LIGHTING_MATERIAL;
	ob3d_flags |= JIT_OB3D_NO_DEPTH;
	ob3d_flags |= JIT_OB3D_NO_COLOR;
	
	// set up object extension for 3d object, customized with flags
	void *ob3d;
	ob3d = jit_ob3d_setup(_jit_gl_spout_sender_class, calcoffset(t_jit_gl_spout_sender, ob3d), ob3d_flags);
		
	// add attributes
	long attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;
	t_jit_object *attr;
	
	// sender name
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"sendername",_jit_sym_symbol,attrflags,
						  (method)0L, jit_gl_spout_sender_sendername, calcoffset(t_jit_gl_spout_sender, sendername));	
	jit_class_addattr(_jit_gl_spout_sender_class,attr);
	
	// force memory share
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"memoryshare", _jit_sym_long, attrflags,
		(method)0L, (method)jit_gl_spout_sender_memoryshare, calcoffset(_jit_gl_spout_sender, memoryshare));	
	jit_class_addattr(_jit_gl_spout_sender_class,attr);	
	
	// define our dest_closing and dest_changed methods. 
	// these methods are called by jit.gl.render when the 
	// destination context closes or changes: for example, when 
	// the user moves the window from one monitor to another. Any 
	// resources your object keeps in the OpenGL machine 
	// (e.g. textures, display lists, vertex shaders, etc.) 
	// will need to be freed when closing, and rebuilt when it has changed.
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_gl_spout_sender_dest_closing, "dest_closing", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_gl_spout_sender_dest_changed, "dest_changed", A_CANT, 0L);
	
	// OB3D methods
	// must register for ob3d use
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_object_register, "register", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_gl_spout_sender_draw, "ob3d_draw", A_CANT, 0L);
	
	// handle texture input - we need to explictly handle jit_gl_texture messages
	// so we can set our internal texture reference
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_gl_spout_sender_jit_gl_texture, "jit_gl_texture", A_GIMME, 0L);

	// handle matrix inputs
	jit_class_addmethod(_jit_gl_spout_sender_class, (method)jit_gl_spout_sender_jit_matrix, "jit_matrix", A_USURP_LOW, 0);	
	
	//symbols
	ps_sendername = gensym("sendername");
	ps_texture = gensym("texture");
	ps_width = gensym("width");
	ps_height = gensym("height");
	ps_glid = gensym("glid");
	ps_gltarget = gensym("gltarget");
	ps_jit_gl_texture = gensym("jit_gl_texture");
	ps_drawto = gensym("drawto");

	jit_class_register(_jit_gl_spout_sender_class);

	return JIT_ERR_NONE;
}

t_jit_gl_spout_sender *jit_gl_spout_sender_new(t_symbol * dest_name)
{
	t_jit_gl_spout_sender *x = NULL;

	// make jit object
	if ((x = (t_jit_gl_spout_sender *)jit_object_alloc(_jit_gl_spout_sender_class)))	{
		
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);

		// Initialize variables
		x->bInitialized			= false;
		x->g_Width				= 0;
		x->g_Height				= 0;
		x->g_texId				= 0;
		x->g_texTarget			= 0;
		x->m_fbo				= 0;
		x->m_fbo_texture		= 0;
		x->mySender				= NULL;

		// Create a new Spout sender
		x->mySender = new SpoutSender;

		// set up attributes
		x->memoryshare = 0; // default is texture share

		// Syphon comment - TODO : is this right ? LJ - not sure
		jit_attr_setsym(x->sendername, _jit_sym_name, gensym("sendername"));

		// instantiate a single internal jit.gl.texture for matrix input
		x->texture = (t_symbol *)jit_object_new(ps_jit_gl_texture, jit_attr_getsym(x,ps_drawto) );
		if (x->texture)	{
			// set texture attributes.
			t_symbol *name =  jit_symbol_unique();
			jit_attr_setsym(x->texture, _jit_sym_name, name);
			jit_attr_setsym(x->texture,gensym("defaultimage"),gensym("white"));
			jit_attr_setlong(x->texture,gensym("rectangle"), 1);
			jit_attr_setsym(x->texture, gensym("mode"),gensym("dynamic"));	
			jit_attr_setsym(x, ps_texture, name);
			x->textureSource = name;
		} 
		else {
			jit_object_error((t_object *)x,"jit.gl.spoutsender: could not create texture");
			x->textureSource = _jit_sym_nothing;		
		}
	} 
	else {
		x = NULL;
	}

	return x;
}

void jit_gl_spout_sender_free(t_jit_gl_spout_sender *x)
{
	// free our internal texture
	if(x->texture) jit_object_free(x->texture);

	// free our ob3d data
	if(x) jit_ob3d_free(x);

	// Release the sender
	x->mySender->ReleaseSender();

	// Delete the sender object last.
	if(x->mySender) delete x->mySender;
	x->mySender = NULL;

}

t_jit_err jit_gl_spout_sender_dest_closing(t_jit_gl_spout_sender *x)
{
	// Don't seem to need to do anything here
	// because dest-changed happens as well
	return JIT_ERR_NONE;
}


t_jit_err jit_gl_spout_sender_dest_changed(t_jit_gl_spout_sender *x)
{	
	// try and find a context.
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();

	if(jit_ctx) {
		if(x->textureSource) {
			// get our latest texture info.
			t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource);
			GLuint texId = jit_attr_getlong(texture, ps_glid);			
			// Syphon comment : our texture has to be bound in the new context before we can use it
			// http://cycling74.com/forums/topic.php?id=29197
			t_jit_gl_drawinfo drawInfo;
			jit_gl_drawinfo_setup(x, &drawInfo);
			jit_gl_bindtexture(&drawInfo, x->textureSource, 0); // bind the jitter texure
			jit_gl_unbindtexture(&drawInfo, x->textureSource, 0); // unbind it
		}
	}

	// Release sender if initialized
	x->mySender->ReleaseSender();
	x->bInitialized = false; // Initialize again in draw

	// Tried releaseing and re-creating the entire sender object
	// to avoid jitter OpenGL errors with fullscreen but no effect

	return JIT_ERR_NONE;
}

#pragma mark -
#pragma mark Input Imagery, Texture/ Matrix


// handle matrix input
t_jit_err jit_gl_spout_sender_jit_matrix(t_jit_gl_spout_sender *x, t_symbol *s, int argc, t_atom *argv)
{
	
	t_symbol *name;
	void *m;

	if ((name=jit_atom_getsym(argv)) != _jit_sym_nothing) {
		m = jit_object_findregistered(name);
		if (!m)	{
			jit_object_error((t_object *)x,"jit.gl.spoutsender: couldn't get matrix object!");
			return JIT_ERR_GENERIC;
		}
	}
	
	if (x->texture)	{				
		jit_object_method(x->texture,s,s,argc,argv);
		// add texture to ob3d texture list
		t_symbol *texName = jit_attr_getsym(x->texture, _jit_sym_name);
		jit_attr_setsym(x,ps_texture,texName);
		x->textureSource = texName;
	}

	return JIT_ERR_NONE;
}

// handle texture input 
// This happens all the time. Changes are handled in draw
t_jit_err jit_gl_spout_sender_jit_gl_texture(t_jit_gl_spout_sender *x, t_symbol *s, int argc, t_atom *argv)
{

	t_symbol *name = jit_atom_getsym(argv);
	
	if (name)   {
		// add texture to ob3d texture list
		jit_attr_setsym(x, ps_texture, name);
		x->textureSource = name;
	}

	return JIT_ERR_NONE;
}

#pragma mark -
#pragma mark Draw


t_jit_err jit_gl_spout_sender_draw(t_jit_gl_spout_sender *x)
{
	if (!x)	return JIT_ERR_INVALID_PTR;

	if(x->textureSource) {

		// get our latest texture info.
		t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource);
		GLuint texId		= jit_attr_getlong(texture, ps_glid);
		GLuint texWidth		= jit_attr_getlong(texture, ps_width);
		GLuint texHeight	= jit_attr_getlong(texture, ps_height);
		GLuint texTarget	= jit_attr_getlong(texture, ps_gltarget);

		// all of these must be > 0
		if(texId > 0 && texWidth > 0 && texHeight > 0)	{
			// --- Create a sender if it is not initialized ---
			if(!x->bInitialized) {
				x->g_texId	= texId;
				x->g_Width	= texWidth;
				x->g_Height	= texHeight;
				x->mySender->CreateSender(x->g_SenderName, x->g_Width, x->g_Height);
				x->bInitialized = true;
			}
			// --- Check for change of width and height or texture ID ---
			else if(texWidth != x->g_Width || texHeight != x->g_Height || texId != x->g_texId) {
				// Set globals for sender create next round
				x->g_texId	= texId;
				x->g_Width	= texWidth;
				x->g_Height	= texHeight;
				x->mySender->ReleaseSender(0);
				x->bInitialized = false; // Initialize again the next time round
			} // endif texture has changed
			// --- otherwise it is initialized OK so send the frame out ---
			else {
				// No actual rendering is done here, this is just sending out a texture
				SendTexture(x); // Saves OpenGL state - no apparent difference
			} // end if size was OK and normal draw
		} // endif texId && width > 0 && height > 0
	} // endif x->textureSource
	return JIT_ERR_NONE;
}

void SendTexture(t_jit_gl_spout_sender *x)
{
	// get our latest texture info.
	t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource);
	GLuint texId		= jit_attr_getlong(texture, ps_glid);
	GLuint texWidth		= jit_attr_getlong(texture, ps_width);
	GLuint texHeight	= jit_attr_getlong(texture, ps_height);
	GLuint texTarget	= jit_attr_getlong(texture, ps_gltarget);

	// Syphon note :
	// Jitter uses multuple texture coordinate arrays on different units, and we (Syphon) erronously do not re-set  
	// our internal Client Active Texture in the framework to GL_TEXTURE0, thus our texture coord array is not set.
	// glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	// glClientActiveTexture(GL_TEXTURE0);
	// http://s-musiclab.jp/mmj_docs/max5/develop/MaxSDK-5.1.1_J/html/jit_8gl_8texture_8c_source.html
	//
	// make sure we pop out to the right FBO
	// LJ performance penalty for getting an restoring read and draw fbo's
	GLint previousFBO;
	// GLint previousReadFBO;
	// GLint previousDrawFBO;
    GLint previousMatrixMode;
	GLint previousActiveTexture;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO); // 8CA6
	// glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO); // 8CAA
	// glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO); // 8CA6
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);

	// save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// We need to ensure we set this before changing our texture matrix
	// glActiveTexture selects which texture unit subsequent texture state calls will affect.
	glActiveTexture(GL_TEXTURE0);

	// ensure we act on the proper client texture as well
	glClientActiveTexture(GL_TEXTURE0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
                               
    glViewport(0, 0,  x->g_Width, x->g_Height);
	glMatrixMode(GL_PROJECTION);

    glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, x->g_Width,  0.0,  x->g_Height, -1, 1);              
                               
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	// Do stuff
	x->mySender->SendTexture(texId, texTarget,  texWidth, texHeight);

	glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(previousMatrixMode);
						
	glPopAttrib();
    glPopClientAttrib();  

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
	// glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
    // glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO); 

	// RestoreOpenGLstate(x);
	glActiveTexture(previousActiveTexture);
	// ensure we act on the proper client texture as well
	glClientActiveTexture(previousActiveTexture);

}


#pragma mark -
#pragma mark Attributes


// attributes
// @sendername
t_jit_err jit_gl_spout_sender_sendername(t_jit_gl_spout_sender *x, void *attr, long argc, t_atom *argv)
{
	t_symbol *srvname;

	// Use this for user input of sharing name
	if(x) {	
		if (argc && argv) {
			srvname = jit_atom_getsym(argv);
			x->sendername = srvname;
		} 
		else {
			// no args, set to zero
			x->sendername = gensym("jit.gl.spoutSender");
		}
		// set the name for this sender
		strcpy(x->g_SenderName, x->sendername->s_name);
	}

	return JIT_ERR_NONE;
}


// force memory share flag (default is off - then is automatic dependent on hardware)
t_jit_err jit_gl_spout_sender_memoryshare(t_jit_gl_spout_sender *x, void *attr, long argc, t_atom *argv)
{
	long c = jit_atom_getlong(argv);

	x->memoryshare = c; // 0 off or 1 on
	x->mySender->SetMemoryShareMode((bool)c); // LJ DEBUG Memoryshare on or off

	return JIT_ERR_NONE;
}


void glErrorText(char *intext)
{
	GLenum myError;
	do {
		myError = glGetError(); // GetLastError();
		if(myError != 0) post("    %s - error : [%d][0x%x][%s]", intext, myError, myError, gluErrorString( myError ) );
	} while(myError != 0);

}
