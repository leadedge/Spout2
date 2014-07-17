/*
    jit.gl.spoutreceiver.c

	Based on :
		jit.gl.simple by Cycling74
		and jit.gl.syphonSender.m
	    Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).

	// =================== SPOUT 2 ===================
	//	03-07-14 - major change to use Spout SDK
	//
 
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
	t_object	ob;	

	// 3d object extension.  This is what all objects in the GL group have in common.
	void		*ob3d;
		
	// attributes
	t_symbol	*sendername;	// Use for sharing name
	t_symbol	*texturename;	

	long		update;			// update to the active sender
	long		aspect;			// retain aspect ratio of input texture
	long		dim[2];			// output dim

	// Our Spout receiver object
	SpoutReceiver * myReceiver;

	unsigned int	g_Width, g_Height;
	char			g_SenderName[256];
	bool			bInitialized;
	// bool			bMemoryMode; // force memory share mode - needs an attribute

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
// Senderuuid, @sendername, for Sender human readable name
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// NOT USED - @texturename to read a named texture.
t_jit_err jit_gl_spout_receiver_texturename(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// NOT USED - @out_name for output...
t_jit_err jit_gl_spout_receiver_getattr_out_name(t_jit_gl_spout_receiver *x, void *attr, long *ac, t_atom **av);

// @dim - dimension output
t_jit_err jit_gl_spout_receiver_setattr_dim(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv);

// @update  update to active Sender
t_jit_err jit_gl_spout_receiver_update(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// @aspect to retain shared texture aspect ratio
t_jit_err jit_gl_spout_receiver_aspect(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv); 

// symbols
t_symbol *ps_sendername;
t_symbol *ps_texture;
t_symbol *ps_width;
t_symbol *ps_height;
t_symbol *ps_glid;
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

// #pragma mark -
// #pragma mark Init, New, Cleanup, Context changes

t_jit_err jit_gl_spout_receiver_init(void) 
{
	/*
	AllocConsole();
	freopen("CONIN$",  "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("\njit_gl_spout_receiver\n");
	*/

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
	// these methods are called by jit.gl.render when the 
	// destination context closes or changes: for example, when 
	// the user moves the window from one monitor to another. Any 
	// resources your object keeps in the OpenGL machine 
	// (e.g. textures, display lists, vertex shaders, etc.) 
	// will need to be freed when closing, and rebuilt when it has 
	// changed.
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_dest_closing, "dest_closing", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_dest_changed, "dest_changed", A_CANT, 0L);

	// ========================================================

	// All Jitter OB3Ds must define a method bound to the symbol ob3d_draw. 
	// This method takes no arguments in addition to the object struct,
	// and should be defined with the private A_CANT type signature.
	// The private ob3d_draw method will be called by the standard draw,
	// and drawraw methods that are added to every OB3D. The draw method 
	// will set up OpenGL state associated with the default OB3D attributes 
	// before calling ob3d_draw, while the drawraw method will not.

	// define our OB3D draw method.  called in automatic mode by 
	// jit.gl.render or otherwise through ob3d when banged. this 
	// method is A_CANT because our draw setup needs to happen 
	// in the ob3d beforehand to initialize OpenGL state 

	// Your OB3D draw method, bound to the ob3d_draw symbol, 
	// is where all of your drawing code takes place. 
	// It is called automatically when your associated jit.gl.render
	// object receives a bang, if your automatic and enabled attributes
	// are turned on, as they are by default.
	// It is also called if your Max wrapper object receives a bang

	jit_class_addmethod(_jit_gl_spout_receiver_class, 
						(method)jit_gl_spout_receiver_draw, 
						"ob3d_draw", A_CANT, 0L);
	// ========================================================

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

	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"sendername",_jit_sym_symbol,attrflags,
						  (method)0L, jit_gl_spout_receiver_sendername, calcoffset(t_jit_gl_spout_receiver, sendername));	
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);	

	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"texturename",_jit_sym_symbol,attrflags,
						  (method)0L,(method)jit_gl_spout_receiver_texturename,calcoffset(t_jit_gl_spout_receiver, texturename));		
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);	

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

	
	// OUTPUT
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_OPAQUE_USER;

	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"out_name",_jit_sym_symbol, attrflags,
						  (method)jit_gl_spout_receiver_getattr_out_name,(method)0L,0);	
	jit_class_addattr(_jit_gl_spout_receiver_class,attr);

	//symbols
	ps_sendername = gensym("sendername");
	ps_texture = gensym("texture");
	ps_width = gensym("width");
	ps_height = gensym("height");
	ps_glid = gensym("glid");
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
		
		// Syphon comment : TODO : is this right ?  LJ not sure
		// set up attributes
		jit_attr_setsym(x->sendername, _jit_sym_name, gensym("sendername"));

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
			jit_object_error((t_object *)x,"jit.gl.syphonSender: could not create texture");
			x->texturename = _jit_sym_nothing;		
		}
		
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);

		x->update = 0; // update to active Sender
		x->aspect = 0; // preserve aspect ratio of incoming texture

		// ======================= Initialize sharing vars =====================
		x->g_Width			= 320;
		x->g_Height			= 240;		// give it an initial image size
		x->g_SenderName[0]	= 0;		// means it will try to find the active sender when it starts
		x->bInitialized		= false;
		// x->bMemoryMode	= false;	// memory mode flag
		x->bInitialized		= false;	// not initialized yet
		x->myReceiver		= new SpoutReceiver;	// Create a new Spout receiver object
		// ======================================================================
	} 
	else {
		x = NULL;
	}

	return x;
}

void jit_gl_spout_receiver_free(t_jit_gl_spout_receiver *x)
{

	// free our ob3d data 
	if(x) jit_ob3d_free(x);
	
	// free our internal texture
	if(x->output) jit_object_free(x->output);

	// Release the receiver
	x->myReceiver->ReleaseReceiver();

	// Delete the Receiver object last.
	if(x->myReceiver) delete x->myReceiver;
	x->myReceiver = NULL;
}

t_jit_err jit_gl_spout_receiver_dest_closing(t_jit_gl_spout_receiver *x)
{
	return JIT_ERR_NONE;
}

t_jit_err jit_gl_spout_receiver_dest_changed(t_jit_gl_spout_receiver *x)
{	
	// try and find a context.
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();
	t_symbol *context = jit_attr_getsym(x, ps_drawto);

	if(jit_ctx) {

		if (x->output) {

			jit_attr_setsym(x->output, ps_drawto, context);
		
			// Syphon comment : our texture has to be bound in the new context before we can use it
			// http://cycling74.com/forums/topic.php?id=29197
			t_jit_gl_drawinfo drawInfo;
			t_symbol *texName = jit_attr_getsym(x->output, gensym("name"));
			jit_gl_drawinfo_setup(x, &drawInfo);
			jit_gl_bindtexture(&drawInfo, texName, 0);
			jit_gl_unbindtexture(&drawInfo, texName, 0);

			// Release Receiver if initialized
			x->myReceiver->ReleaseReceiver();
			x->bInitialized = false; // Initialize again in draw

		}
	}

	return JIT_ERR_NONE;
}

// #pragma mark -
// #pragma mark Draw

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
	char sendername[256];
	unsigned int senderWidth, senderHeight;

	if (!x) {
		return JIT_ERR_INVALID_PTR;
	}
	
	// TODO: necessary ? JKC says no unless context changed above? should be set during draw for you. 
	// LJ seems un-necessary but does not hurt if it stays
	t_jit_gl_context ctx = jit_gl_get_context();
	jit_ob3d_set_context(x);

	// Try to create a receiver if not initialized
	// If sendername is null it will pick up the active sender
	// Otherwise it will wait for the correct name to be entered
	if(!x->bInitialized) {
		if(x->myReceiver->CreateReceiver(x->g_SenderName, senderWidth, senderHeight)) {
			x->g_Width	= senderWidth;
			x->g_Height	= senderHeight;
			// Update output texture dim to the new size
			newdim[0] = x->g_Width;
			newdim[1] = x->g_Height;
			jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim);
			x->bInitialized = true;
		}
		return JIT_ERR_NONE;
	}
	
	//
	// Now it is initialized so try to receive a texture
	//
	
	// make sure we pop out to the right FBO
	// LJ perormance issue for get and restore read and draw
	GLint previousFBO;      
	// GLint previousReadFBO;
	// GLint previousDrawFBO;
    GLint previousMatrixMode;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO); // 8CA6
	// glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO); // 8CAA
	// glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO); // 8CA6
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	
	// save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// We need the texture ID, width/height.
	GLuint texname	= jit_attr_getlong(x->output, ps_glid);
	GLuint width	= jit_attr_getlong(x->output, ps_width);
	GLuint height	= jit_attr_getlong(x->output, ps_height);

	// An FBO for render to texture
	GLuint tempFBO;
	glGenFramebuffersEXT(1, &tempFBO);

	// Syphon note :
	// Jitter uses multiple texture coordinate arrays on different units, and we (Syphon) erronously do not re-set  
	// our internal Client Active Texture in the framework to GL_TEXTURE0, thus our texture coord array is not set.
	// glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	// glClientActiveTexture(GL_TEXTURE0);
	// http://s-musiclab.jp/mmj_docs/max5/develop/MaxSDK-5.1.1_J/html/jit_8gl_8texture_8c_source.html
	//
	// LJ DEBUG - needs checking (restore ?)
	//
	// We need to ensure we set this before changing our texture matrix
	// glActiveTexture selects which texture unit subsequent texture state calls will affect.
	glActiveTexture(GL_TEXTURE0);

	// ensure we act on the proper client texture as well
	glClientActiveTexture(GL_TEXTURE0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	// =========================================================
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
	// =========================================================

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
	glLoadIdentity();

	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);
                               
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	// Receive a shared texture and return it's width and height
	if(x->myReceiver->ReceiveTexture(x->g_SenderName, senderWidth, senderHeight)) {
		// Test for change of texture size
		if(senderWidth != x->g_Width || senderHeight != x->g_Height) {
			// Set global width and height
			x->g_Width	= senderWidth;
			x->g_Height	= senderHeight;
			// Update output dim to the new size
			newdim[0] = x->g_Width;
			newdim[1] = x->g_Height;
			jit_attr_setlong_array(x, _jit_sym_dim, 2, newdim);
		}
		else {
			// We have a shared texture and can render into the jitter texture
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO); 
			// Attach the jitter texture (destination) to the color buffer in our frame buffer  
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, texname, 0);
			if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
				// Draw the shared texture into it
				x->myReceiver->DrawSharedTexture();
			}
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(previousMatrixMode);

		glPopClientAttrib();  
		glPopAttrib();

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
		// glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
		// glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO); 

	    // clean up
		glDeleteFramebuffersEXT(1, &tempFBO);
		tempFBO = 0;

	} // Received texture OK

	return JIT_ERR_NONE;

}

		
// #pragma mark -
// #pragma mark Attributes

// attributes

// @Senderuuid
t_jit_err jit_gl_spout_receiver_sendername(t_jit_gl_spout_receiver *x, void *attr, long argc, t_atom *argv)
{
	t_symbol *srvname;
	char name[256];
	
	if(x) {	
		if (argc && argv) {
			srvname = jit_atom_getsym(argv);
			x->sendername = srvname;
			strcpy_s(name, 256, x->sendername->s_name);
			if(strcmp(x->g_SenderName, name) != 0) { // different name
				strcpy_s(x->g_SenderName, 256, name);
				x->myReceiver->ReleaseReceiver();
				x->bInitialized = false;
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

	t_symbol *s=jit_atom_getsym(argv);

	if(s->s_name) {
		x->texturename = s;
		if (x->output) jit_attr_setsym(x->output, _jit_sym_name, s);
		jit_attr_setsym(x,ps_texture,s);
	}

	return JIT_ERR_NONE;
}

// @update - to get active Sender
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


