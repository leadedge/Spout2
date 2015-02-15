/*

    jit.gl.freeframe.c

	Based on : jit.gl.spoutreceiver and jit.gl.spoutsender
	With much help from Syphon Server and Client externals. 

	attributes :

	// @loadeffect, load the given plugin name
	// @param, parameter name an number e.g. param heat 0.5 or param 0 0.5
	// @reload, to rescan for new effects
	// @dim, set output dimensions (only for a source plugin)
	// @paramlist, output lists of paramters for the loaded plugin
	// @effectlist, output a list of all plugins in the plugin folder
	// @bypass, bypass effect and hand on input texture
	// @paramdialog, activate a parameter dialog

	=======================================================================================
	13-11-14 - started project
	18-11-14 - first working version
	19.11.14 - FFGLParamInfo parameters rearranged due to alignment compiler warning
			   remove case sensitivity for effects and params
	20.11.14 - Fixed alpha problem and crash if more than one input texture
			   Needs two input textures to the external for mixing to work.
	21.11.14 - Two sources working for dual texture plugins
			   Append "2" to the texture or matrix output and link to the RH inlet
			   i.e. the message ends up as "jit_matrix 2" or "jit_gl_texture 2"
			   and the two input textures can be updated as required.
			   Can be linked to the LH inlet as well, but using the RH inlet makes it clear.
			   Plugin capabilities show whether it accepts more than one texture.
			   Only two input textures are provided for.
	23.11.14 - Allowed for a source texture which does not need an inpu texture
			   but does need dimension input because there is no input to base it on
	24.11.14 - fixed jitter texture initialization for a source plugin
			   Added search of effects in the patch path
	26.11.14 - added "getparamlist", "getnumparams", "getparameter"
			   Updated general example patch.
	27.11.14 - added number and type to "getparameter"	
	29.11.14 - fixed name compare in plugin selection
			   added an additonal output texture and render step so that blending would work. 
			   Added function to render a texture to the jitter texture
			   Enabled blend in final output texture render.
			   Created enhanced example patch with plugin selection.
	30.11.14 - Added parameter dialog
	01.12.14 - max.jit.gl.freeframe - added call to draw before parameter retrieval to make sure plugin is intialized
	02.12.14 - max.jit.gl.freeframe - changed paramlist output format
			   max.jit.gl.freeframe - changed "parameter" to "param" for dump output for "getparameter"
	03.12.14 - some corrections to checkbox parameter passing to the plugin
			   correction to parameter dialog for up to 30 parameters
			   changes to ensure no more that 16 chars copied from the plugin where necessary
	10.12.14 - clean up parameter dialog to save and restore parameters properly
	10.01.15 - look for FreeFrame folder under x86 and 64bit "Program Files" folder
			   Vers 1.002
	12.01.15 - Added FFGL time support
			   Vers 1.003 - for testing
	30.01.15 - Changed param attribute function for checkbox toggle
			   Vers 1.004
	03.02.15 - Cleanup
			   Vers 1.005
	11.02.15 - Check for GL context on jit_gl_freeframe_free before unloading plugin and free GL resources
			 - added Optimus enablement export
			   Vers 1.006

	=========================================================================================

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

	=========================================================================================


*/
#include "jit.common.h"
#include "jit.gl.h"
#include "jit.gl.ob3d.h"
#include "ext_obex.h"
#include "string"
#include "vector"

// FFGL
#include "FFGL.h"
#include "FFGLPluginInstance.h"
#include "WinPluginInstance.h"
#include "Timer.h"

#include "FFGLPluginSDK.h"
#include "resource.h" // Our resources
#include <Shlobj.h> // to get the program folder path
#include <comutil.h> // for _bstr_t (used in the string conversion)
#include <direct.h> // for directories
#include <io.h> // for file existence check

using namespace std;
#pragma comment(lib, "comsuppw") // for _bstr_t

#define IDM_EFFECT 50000	// for parameter dialog resoucres

// For dialog box to get the dll hModule since there is no main
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

// This allows the Optimus global 3d setting to be "adapt" instead of "high performance"
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

struct FFGLParamInfo {
	float value;      // float value (default)
	DWORD type;       // 0-checkbox, 1-button, 100-text, 10 and default - float
	bool state;       // checkbox or button state
	char text[256];   // text input
	char name[16];    // Parameter name
	char display[16]; // alternate parameter display value (other than actual value)
};

static int ScrollBarPos[30]; // for dialog controls
static vector<FFGLParamInfo> InfoList; // for dialog controls
static vector<FFGLParamInfo> infoBackup; // for dialog cancel
static HWND ParamWnd = NULL;
static t_object *xs;

t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv);

typedef struct _jit_gl_freeframe
{
	// Max object
	t_object ob;	

	// 3d object extension.  This is what all objects in the GL group have in common.
	void *ob3d;
	
	 // The plugin name to be loaded
	t_symbol *effectname;

	// internal jit.gl.texture object, which we use to handle matrix input.
	t_symbol *texture;
	t_symbol *texture2;
	
	// the name of the jitter texture source we should draw from - our internal one (for matrix input) or an external one
	t_symbol *textureSource;
	t_symbol *textureSource2; // the name of the jitter texture source from the second inlet

	// internal jit.gl.texture object that is filled, processed by the Freeframe plugin and sent out
	t_symbol *texturename;
	t_jit_object *output;

	// attributes
	long param;  // Plugin parameter
	long dim[2]; // Output dim
	long bypass; // Bypass the effect and just hand on the incoming texture
	long paramdialog;

	WinPluginInstance *plugin;           // Our Freeframe plugin object
	Timer *time;                         // Freeframe time class  
	double curFrameTime;                 // Freeframe time
	vector<string> EffectList;           // list of effect files - full path
	vector<FFGLParamInfo> ParamInfoList; // List of parameters for the loaded plugin
	FFGLTextureStruct ffglTexture;       // The texture which is passed to the plugin
	FFGLTextureStruct ffglTexture2;      // A second texture that can be passed to the plugin
	FFGLTextureStruct ffglTexture3;      // A third texture which is rendered into by the plugin via an fbo

	GLuint       g_texId;                // jitter texture ID
	unsigned int g_Width;                // Width
	unsigned int g_Height;			     // Height
	int          g_numInputsMax;         // Maximum number of inputs the plugin has (up to 2 allowed)
	int          g_numInputsMin;         // Minimum number of inputs the plugin has
	char         g_PluginName[MAX_PATH]; // Plugin name
	char         g_PluginPath[MAX_PATH]; // Plugin file path
	GLuint       g_fbo1;                 // Internal fbo used for texture drawing
	GLuint       g_fbo2;                 // Internal fbo used for texture drawing
	bool         bPluginLoaded;          // A plugin is loaded
	bool         bPluginInitialized;     // The plugin is initialized
	bool         bPluginSelected;        // The user has selected a plugin
	bool         bInitialized;           // The plugin initialized OK
	bool         bDestChanged;           // dest_changed has occurred

} t_jit_gl_freeframe;

void *_jit_gl_freeframe_class;


//
// Function Declarations
//


// init/constructor/free
t_jit_err jit_gl_freeframe_init(void);
t_jit_gl_freeframe *jit_gl_freeframe_new(t_symbol * dest_name);
void jit_gl_freeframe_free(t_jit_gl_freeframe *x);

// handle context changes - need to rebuild textures here.
t_jit_err jit_gl_freeframe_dest_closing(t_jit_gl_freeframe *x);
t_jit_err jit_gl_freeframe_dest_changed(t_jit_gl_freeframe *x);

// draw;
t_jit_err jit_gl_freeframe_draw(t_jit_gl_freeframe *x);
t_jit_err jit_gl_freeframe_draw2(t_jit_gl_freeframe *x);
t_jit_err jit_gl_freeframe_drawto(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv);

// handle input texture
t_jit_err jit_gl_freeframe_jit_gl_texture(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv);

// handle input matrix
t_jit_err jit_gl_freeframe_jit_matrix(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv);


//
// attributes
//


// @loadeffect, for plugin name
t_jit_err jit_gl_freeframe_loadeffect(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv);

// @param Plugin parameter name an number
t_jit_err jit_gl_freeframe_param(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv); 

// @reload to rescan for new effects
t_jit_err jit_gl_freeframe_reload(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv); 

// @dim - dimensions for a source plugin (sets the output texture dimensions)
t_jit_err jit_gl_freeframe_setattr_dim(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv);

// @bypass
t_jit_err jit_gl_freeframe_bypass(t_jit_gl_freeframe *x);

// @paramdialog
t_jit_err jit_gl_freeframe_paramdialog(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv);

// @paramlist for output by max...
t_jit_err jit_gl_freeframe_getattr_out_param(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av);

// @numparams for output by max...
// t_jit_err jit_gl_freeframe_getattr_out_numparams(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av);

// @effectlist for output by max...
t_jit_err jit_gl_freeframe_getattr_out_effect(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av);

// @out_name - output texture name for output by max...
t_jit_err jit_gl_freeframe_getattr_out_name(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av);

//
// symbols
//
t_symbol *ps_effectname;
t_symbol *ps_target;
t_symbol *ps_automatic;
t_symbol *ps_flip;

t_symbol *ps_texture;
t_symbol *ps_texid;
t_symbol *ps_width;
t_symbol *ps_height;
t_symbol *ps_glid;
t_symbol *ps_drawto;

// our internal texture
extern t_symbol *ps_jit_gl_texture; 

// For retrieving parameter and plugin list information for Max
extern t_symbol *ps_paramlist;
extern t_symbol *ps_effectlist;


//
// Function implementations
//
bool FindEffects(t_jit_gl_freeframe *x);
bool LoadFFGLplugin(t_jit_gl_freeframe *x, const char *pluginfile);
bool InitializeFFGLplugin(t_jit_gl_freeframe *x);
bool UnInitializeFFGLplugin(t_jit_gl_freeframe *x);
bool UnloadFFGLplugin(t_jit_gl_freeframe *x);
FFGLTextureStruct CreateFFGLtexture(GLuint glTextureHandle, int textureWidth, int textureHeight);
bool DrawGLTexture(GLuint sourceTexture, GLuint sourceTarget);
bool DrawJitterTexture(t_jit_gl_freeframe *x);
bool DrawToGLtexture(t_jit_gl_freeframe *x, t_symbol *texture, GLuint fbo, GLuint TextureID, GLuint TextureTarget);
bool DrawToJitterTexture(t_jit_gl_freeframe *x, GLuint destTexture, GLuint destTarget, GLuint FBO, GLuint sourceTexture, GLuint sourceTarget);
void ToLowerCase(char *str);
void CopyString(char *dest, char *source, int maxChars);
void UnloadFree(t_jit_gl_freeframe *x);
void GLerror();
int FindPixelType();

// For parameter dialog
HMODULE GetCurrentModule();
LRESULT CALLBACK PluginParameters(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
bool EnterPluginParameters(t_jit_gl_freeframe *x);
void UpdateParameterList(t_object *x);

//
// Init, New, Cleanup, Context changes
//

t_jit_err jit_gl_freeframe_init(void) 
{

	// create our class
	_jit_gl_freeframe_class = jit_class_new("jit_gl_freeframe", 
										 (method)jit_gl_freeframe_new, (method)jit_gl_freeframe_free,
										 sizeof(t_jit_gl_freeframe),A_DEFSYM,0L);

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
	ob3d = jit_ob3d_setup(_jit_gl_freeframe_class, 
						  calcoffset(t_jit_gl_freeframe, ob3d), 
						  ob3d_flags);

	
	// define our OB3D draw method.
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_gl_freeframe_draw, 
						"ob3d_draw", A_CANT, 0L);


	// define the dest_closing and dest_changed methods. 
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_gl_freeframe_dest_closing, "dest_closing", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_gl_freeframe_dest_changed, "dest_changed", A_CANT, 0L);

	// must register for ob3d use
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_object_register, "register", A_CANT, 0L);

	// handle texture input
	// we need to explictly handle jit_gl_texture messages so we can set our internal texture
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_gl_freeframe_jit_gl_texture, 
						"jit_gl_texture", A_GIMME, 0L);

	// handle matrix inputs
	jit_class_addmethod(_jit_gl_freeframe_class, 
						(method)jit_gl_freeframe_jit_matrix,
						"jit_matrix", A_USURP_LOW, 0);

	// add attributes

	// INPUTS
	long attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;

	t_jit_object *attr;

    // Set output dimensions for a source plugin
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset_array,
											"dim",
											_jit_sym_long,
											2,
											attrflags,
											(method)0L,
											(method)jit_gl_freeframe_setattr_dim,
											0,
											calcoffset(t_jit_gl_freeframe,dim));
    jit_class_addattr(_jit_gl_freeframe_class,attr);


	// Enter a plugin effect name
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"loadeffect",
										  _jit_sym_symbol,attrflags,
										   (method)0L,
										   jit_gl_freeframe_loadeffect, 
										   calcoffset(t_jit_gl_freeframe, effectname));

	jit_class_addattr(_jit_gl_freeframe_class, attr);


	// Enter and set a plugin Parameter
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, "param", 
										_jit_sym_long, attrflags, 
										 (method)0L, 
										 (method)jit_gl_freeframe_param, 
										 calcoffset(t_jit_gl_freeframe, param));
	jit_class_addattr(_jit_gl_freeframe_class, attr);


	// Reload all effects and scan for new ones (redundant)
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"reload",
										  _jit_sym_symbol, attrflags,
										  (method)0L, 
										  (method)jit_gl_freeframe_reload,
										  (method)0L, 0);	
	jit_class_addattr(_jit_gl_freeframe_class, attr);

	
	// Bypass the effect
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"bypass", _jit_sym_long, attrflags,
		(method)0L, (method)jit_gl_freeframe_bypass, calcoffset(_jit_gl_freeframe, bypass));	
	jit_class_addattr(_jit_gl_freeframe_class,attr);

	// activate a parameter dialog
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"paramdialog", _jit_sym_long, attrflags,
		(method)0L, (method)jit_gl_freeframe_paramdialog, calcoffset(_jit_gl_freeframe, paramdialog));	
	jit_class_addattr(_jit_gl_freeframe_class,attr);

	// OUTPUT
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_OPAQUE_USER;


	// Texture
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset,"out_name",_jit_sym_symbol, attrflags,
						  (method)jit_gl_freeframe_getattr_out_name, (method)0L, 0);	
	jit_class_addattr(_jit_gl_freeframe_class,attr);


	// Parameter list
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset, "out_param", _jit_sym_symbol, attrflags,
						  (method)jit_gl_freeframe_getattr_out_param, (method)0L, 0);	
	jit_class_addattr(_jit_gl_freeframe_class, attr);


	// Effect list
	attr = (t_jit_object *)jit_object_new(_jit_sym_jit_attr_offset, "out_effect", _jit_sym_symbol, attrflags,
						  (method)jit_gl_freeframe_getattr_out_effect, (method)0L, 0);	
	jit_class_addattr(_jit_gl_freeframe_class, attr);


	//symbols
	ps_effectname = gensym("effectname");
	ps_target     = gensym("target"); // TODO
	ps_automatic  = gensym("automatic");
	ps_flip       = gensym("flip");
	ps_drawto     = gensym("drawto");

	ps_texture    = gensym("texture");
	ps_texid      = gensym("texid");
	ps_width      = gensym("width");
	ps_height     = gensym("height");
	ps_glid       = gensym("glid");

	ps_paramlist  = gensym("paramlist");
	ps_effectlist = gensym("effectlist");

	ps_jit_gl_texture = gensym("jit_gl_texture");
	
	jit_class_register(_jit_gl_freeframe_class);

	return JIT_ERR_NONE;
}

t_jit_gl_freeframe *jit_gl_freeframe_new(t_symbol * dest_name)
{
	t_jit_gl_freeframe *x = NULL;

	// make jit object
	if ((x = (t_jit_gl_freeframe *)jit_object_alloc(_jit_gl_freeframe_class)))	{
		
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);

		// Initialize variables
		x->g_texId             = 0;
		x->param               = 0;     // 
		x->g_Width             = 640;   // give it an initial image size
		x->g_Height            = 480;
		x->g_numInputsMin      = 0;     // In case the plugin is an effect without any inputs
		x->g_numInputsMax      = 2;     // default is 2 maximum
		x->g_PluginName[0]     = 0;     // name only 
		x->g_PluginPath[0]     = 0;     // full file path
		x->g_fbo1              = 0;     // Internal fbo used for texture drawing
		x->bInitialized        = false; // not initialized yet
		x->bDestChanged        = false; // to avoid context change problem
		x->bypass              = 0;     // Bypass the effect

		// ======= FFGL =======
		x->bPluginLoaded       = false;
		x->bPluginInitialized  = false;
		x->bPluginSelected     = true;	// User plugin selected - set true the first time round so the plugin is loaded
		x->ffglTexture.Handle  = 0;		// First source texture
		x->ffglTexture2.Handle = 0;		// Second source texture
		x->ffglTexture3.Handle = 0;		// Intermediate output texture for source plugins
		x->curFrameTime        = 0;		// FFGL time

		// instantiate an internal jit.gl.texture for input
		x->texture = (t_symbol *)jit_object_new(ps_jit_gl_texture, jit_attr_getsym(x, ps_drawto) );
		if (x->texture)	{
			// set texture attributes.
			t_symbol *name =  jit_symbol_unique();
			jit_attr_setsym(x->texture, _jit_sym_name, name);
			jit_attr_setsym(x->texture, gensym("defaultimage"), gensym("black"));
			jit_attr_setlong(x->texture, gensym("rectangle"), 1);
			jit_attr_setsym(x, ps_texture, name);
			x->textureSource = name;

			t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource);
			GLuint texname	= jit_attr_getlong(texture, ps_glid);
			GLuint inwidth	= jit_attr_getlong(texture, ps_width);
			GLuint inheight	= jit_attr_getlong(texture, ps_height);
		} 
		else {
			jit_object_error((t_object *)x,"jit.gl.freeframe : could not create input texture");
			x->textureSource = _jit_sym_nothing;		
		}

		// instantiate a second internal jit.gl.texture in case it is needed
		x->texture2 = (t_symbol *)jit_object_new(ps_jit_gl_texture, jit_attr_getsym(x, ps_drawto) );
		if (x->texture2)	{
			// set texture attributes.
			t_symbol *name =  jit_symbol_unique();
			jit_attr_setsym(x->texture2, _jit_sym_name, name);
			jit_attr_setsym(x->texture2, gensym("defaultimage"), gensym("black"));
			jit_attr_setlong(x->texture2, gensym("rectangle"), 1);
			// jit_attr_setlong(x->texture2, gensym("argb"), 1); //  LJ DEBUG default is auto ?
			// jit_attr_setlong(x->texture, gensym("blend"), 1); // LJ DEBUG
			jit_attr_setsym(x, ps_texture, name);
			x->textureSource2 = name;

			t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource2);
			GLuint texname	= jit_attr_getlong(texture, ps_glid);
			GLuint inwidth	= jit_attr_getlong(texture, ps_width);
			GLuint inheight	= jit_attr_getlong(texture, ps_height);
		} 
		else {
			jit_object_error((t_object *)x,"jit.gl.freeframe : could not create input texture");
			x->textureSource2 = _jit_sym_nothing;		
		}

		// instantiate a single internal jit.gl.texture for output
		x->output = (t_jit_object *)jit_object_new(ps_jit_gl_texture, dest_name);
		if (x->output) {
			x->texturename = jit_symbol_unique();
			// set texture attributes.
			jit_attr_setsym(x->output,  _jit_sym_name, x->texturename);
			jit_attr_setsym(x->output,  gensym("defaultimage"), gensym("black"));
			jit_attr_setlong(x->output, gensym("rectangle"), 1); // rectangle is default
			jit_attr_setlong(x->output, gensym("flip"), 0); // flip is default on (TODO - performance ?)
			// Give it a reasonable initial size (this can be adjusted with the dim attribute)
			x->dim[0] = 640;
			x->dim[1] = 480;
			// Set the intial global width and height here too
			x->g_Width  = x->dim[0];
			x->g_Height = x->dim[1];
			jit_attr_setlong_array(x->output, _jit_sym_dim, 2, x->dim);
        } 
		else {
			jit_object_error((t_object *)x,"jit.gl.freeframe: could not create output texture");
			x->texturename = _jit_sym_nothing;		
		}
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);

		// Create a new FFGL plugin object - does not instantiate
		x->plugin = new WinPluginInstance;

		//start the timer
		x->time = Timer::New();

	} 
	else {
		x = NULL;
	}

	return x;
}


void jit_gl_freeframe_free(t_jit_gl_freeframe *x)
{

	// Free FFGL textures
	// Check for OpenGL context first
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();
	
	// If there is a context, unload plugin and free all resources
	if(jit_ctx) {

		if(x->bPluginLoaded) UnloadFFGLplugin(x);

		if(x->ffglTexture.Handle != 0) glDeleteTextures(1, &x->ffglTexture.Handle);
		if(x->ffglTexture2.Handle != 0) glDeleteTextures(1, &x->ffglTexture2.Handle);
		if(x->ffglTexture3.Handle != 0) glDeleteTextures(1, &x->ffglTexture3.Handle);

		if(x->g_fbo1 > 0) glDeleteFramebuffersEXT(1, &x->g_fbo1);
	}

	x->ffglTexture.Handle = 0;
	x->ffglTexture2.Handle = 0;
	x->ffglTexture3.Handle = 0;
	x->g_fbo1 = 0;

	// Delete the FFGL plugin object
	if(x->plugin) delete x->plugin;
	x->plugin = NULL;

	// release the FFGL timer
	if(x->time) delete x->time;

	// free our output texture
	if(x->output) jit_object_free(x->output);

	// free our ob3d data
	if(x) jit_ob3d_free(x);

}

t_jit_err jit_gl_freeframe_dest_closing(t_jit_gl_freeframe *x)
{
	// The context is about to close but has not changed yet,
	// so delete OpenGL objects while the context still exists

	// try and find a context.
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();
	
	// If there is a context, unload plugin and free all resources
	if(jit_ctx) 
		UnloadFree(x);

	x->bDestChanged = true; // Let the plugin reload function know it is a context change

	return JIT_ERR_NONE;
}

t_jit_err jit_gl_freeframe_dest_changed(t_jit_gl_freeframe *x)
{	
	// try and find a context.
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();

	if(jit_ctx) {

		// Unload plugin and free all resources
		UnloadFree(x);

		// Syphon comment : our texture has to be bound in the new context before we can use it
		// http://cycling74.com/forums/topic.php?id=29197
		// The output texture must be bound at *least once* while its parent context is valid
		// in order to generate the gpu texture resource (and hence id). 
		// Result otherwise is a white screen
		if (x->output) {
			t_symbol *context = jit_attr_getsym(x, ps_drawto);
			jit_attr_setsym(x->output, ps_drawto, context);
			t_jit_gl_drawinfo drawInfo;
			t_symbol *texName = jit_attr_getsym(x->output, gensym("name"));
			jit_gl_drawinfo_setup(x, &drawInfo);
			jit_gl_bindtexture(&drawInfo, texName, 0);
			jit_gl_unbindtexture(&drawInfo, texName, 0);
		}
	}

	x->bDestChanged = true; // Let thhe plugin reload function know

	return JIT_ERR_NONE;
}

//
// Input Imagery, Texture/ Matrix
//

//
// Handle matrix input
//
// If anything is appended to the message, e.g. "jit_matrix 2" 
// the second texture is updated instead of the first
// This allows for two inputs from either the main inlet or a RH proxy inlet
//
t_jit_err jit_gl_freeframe_jit_matrix(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv)
{
	t_symbol *name;
	void *m;

	if ((name=jit_atom_getsym(argv)) != _jit_sym_nothing) {
		m = jit_object_findregistered(name);
		if (!m)	{
			jit_object_error((t_object *)x,"jit_gl_freeframe: couldn't get matrix object!");
			return JIT_ERR_GENERIC;
		}
	}

	if(argc == 1) {
		if (x->texture)	{				
			jit_object_method(x->texture, s ,s, argc, argv);
			t_symbol *texName = jit_attr_getsym(x->texture, _jit_sym_name);
			// add texture to ob3d texture list
			jit_attr_setsym(x, ps_texture, texName);
			x->textureSource = texName; // first input texture is now the matrix input
		}
	}
	else {
		if (x->texture2)	{				
			jit_object_method(x->texture2, s ,s, argc, argv);
			t_symbol *texName = jit_attr_getsym(x->texture2, _jit_sym_name);
			// add texture to ob3d texture list
			jit_attr_setsym(x, ps_texture, texName);
			x->textureSource2 = texName; // second input texture is now the matrix input
		}
	}

	return JIT_ERR_NONE;
}


// handle texture input 
// This happens all the time. Changes are handled in draw
// Same arrangement as for matrix input - check for an additional arg
// e,g "jit_gl_texture 2" 
t_jit_err jit_gl_freeframe_jit_gl_texture(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv)
{
	t_symbol *name  = jit_atom_getsym(argv);

	if (name->s_name[0]) {
		// add texture to ob3d texture list
		jit_attr_setsym(x, ps_texture, name);
		if(argc == 1) x->textureSource = name; // First input texture
		else          x->textureSource2 = name; // Second if another arg is appended to the message
	}

	return JIT_ERR_NONE;
}


// Set destination for draw
t_jit_err jit_gl_freeframe_drawto(t_jit_gl_freeframe *x, t_symbol *s, int argc, t_atom *argv)
{
	object_attr_setvalueof(x->output, s, argc, argv);
	jit_ob3d_dest_name_set((t_jit_object *)x, NULL, argc, argv);
	return JIT_ERR_NONE;
}


t_jit_err jit_gl_freeframe_draw(t_jit_gl_freeframe *x)
{
	t_jit_err result = JIT_ERR_NONE;
	DWORD dwRet = FF_FAIL;
	bool bRet = false;
	float vpdim[4]; // for saving the viewport dimensions
	GLint previousFBO = 0;      
	GLint previousMatrixMode = 0;
	GLint previousActiveTexture = 0;
	t_jit_gl_drawinfo drawInfo;
	bool bSourcePlugin = false; // source

	if (!x) return JIT_ERR_INVALID_PTR;


	// try and find a context.
	t_jit_gl_context jit_ctx = 0;
	jit_ctx = jit_gl_get_context();
	if(!jit_ctx) return JIT_ERR_NONE;

	jit_gl_drawinfo_setup(x, &drawInfo);


	// We need the Jitter input texture info
	t_jit_object *texture = (t_jit_object*)jit_object_findregistered(x->textureSource);
	GLuint texname	= jit_attr_getlong(texture, ps_glid);
	GLuint inwidth	= jit_attr_getlong(texture, ps_width);
	GLuint inheight	= jit_attr_getlong(texture, ps_height);

	// Is there a second input texture to use ?
	t_jit_object *texture2 = (t_jit_object*)jit_object_findregistered(x->textureSource2);
	GLuint texname2 = jit_attr_getlong(texture2, ps_glid);

	// Get the ID of the output texture object
	GLuint outname  = jit_attr_getlong(x->output, ps_glid);

	// Load a plugin if it is not initialized
	if(!x->bInitialized) {

		// Has a new plugin been selected ?
		if(x->bPluginSelected) {
			if(x->bPluginLoaded) {
				UnloadFFGLplugin(x);
			}

			// Now load the one selected (x->g_PluginPath)
			x->bPluginLoaded = LoadFFGLplugin(x, x->g_PluginPath);
			x->bPluginSelected = false; // wait for the user to select another one

			// Free any existing textures
			if(x->ffglTexture.Handle != 0) glDeleteTextures(1, &x->ffglTexture.Handle);
			if(x->ffglTexture2.Handle != 0) glDeleteTextures(1, &x->ffglTexture2.Handle);
			if(x->ffglTexture3.Handle != 0) glDeleteTextures(1, &x->ffglTexture3.Handle);
			x->ffglTexture.Handle = 0;
			x->ffglTexture2.Handle = 0;
			x->ffglTexture3.Handle = 0;

		}
		
		// Allow for resizing without unloading the plugin
		if(x->bPluginLoaded && !x->bPluginInitialized) {
			x->bPluginInitialized = InitializeFFGLplugin(x);
		}

		x->bInitialized = true;

		return JIT_ERR_NONE;

	} // endif not initialized	

	// Quit now if the plugin requires a texture but there is none attached 
	// If the plugin can do without one (min-inputs = 0) it is OK to continue
	if(x->g_numInputsMin > 0 && texname <= 0)
		return JIT_ERR_NONE;

	// -------------------------- Test plugin requirements --------------------------------
	// A source plugin will always have max-inputs = 0 and max-inputs = 0
	// An effect plugin will always have min-inputs > 0 and max-inputs > 0
	// An effect plugin not requiring a texture will have min-inputs = 0 and max-inputs > 0
	// ------------------------------------------------------------------------------------

	// Pure source
	if(x->g_numInputsMin == 0 && x->g_numInputsMax == 0) {
		bSourcePlugin = true;
	}
	// Pure effect
	else if(x->g_numInputsMin >  0 && x->g_numInputsMax >  0) {
		bSourcePlugin = false;
	}
	// Source and effect plugin, depends on whether there is an input texture available
	else {
		// Is there an incoming texture ?
		if(texname > 0) 
			bSourcePlugin = false; // assume it requires an input
		else 
			bSourcePlugin = true;
	}

	// For an effect plugin all of these must be > 0, so quit now if not
	if(!bSourcePlugin && !(texname && inwidth > 0 && inheight > 0))	{
		return JIT_ERR_NONE;
	}

	// If there is an input texture, the output texture size is linked to the input texture size
	// so check for an input dimension change - TODO - different size of second input texture ?
	if(texname > 0) {
		if(x->dim[0] != inwidth || x->dim[1] != inheight) {
			x->dim[0] = inwidth;
			x->dim[1] = inheight;
			x->g_Width  = inwidth;
			x->g_Height = inheight;
			jit_attr_setlong_array(x->output, _jit_sym_dim, 2, x->dim);

			// Reset the FFGL textures
			if(x->ffglTexture.Handle != 0) glDeleteTextures(1, &x->ffglTexture.Handle);
			if(x->ffglTexture2.Handle != 0) glDeleteTextures(1, &x->ffglTexture2.Handle);
			if(x->ffglTexture3.Handle != 0) glDeleteTextures(1, &x->ffglTexture3.Handle);
			x->ffglTexture.Handle = 0;
			x->ffglTexture2.Handle = 0;
			x->ffglTexture3.Handle = 0;

			return JIT_ERR_NONE;
		}
	}
	else if(x->g_Width != x->dim[0] || x->g_Height != x->dim[1]) {
		// Otherwise for a source plugin, or an effect plugin without any 
		// inputs and no input texture, the ffgl texture size is linked
		// to the user defined output dimension, so check it here
		x->g_Width  = x->dim[0];
		x->g_Height = x->dim[1];

		// Reset the FFGL textures
		if(x->ffglTexture.Handle != 0) glDeleteTextures(1, &x->ffglTexture.Handle);
		if(x->ffglTexture2.Handle != 0) glDeleteTextures(1, &x->ffglTexture2.Handle);
		if(x->ffglTexture3.Handle != 0) glDeleteTextures(1, &x->ffglTexture3.Handle);
		x->ffglTexture.Handle = 0;
		x->ffglTexture2.Handle = 0;
		x->ffglTexture3.Handle = 0;

		return JIT_ERR_NONE;
	}

	// Create an FFGL texture for the plugin to render into via an fbo
	if(x->ffglTexture.Handle == 0) {
		x->ffglTexture = CreateFFGLtexture(x->ffglTexture.Handle, x->g_Width, x->g_Height);
	}

	// For a source plugin create an FFGL texture for output
	if(bSourcePlugin) {
		if(x->ffglTexture3.Handle == 0) {
			x->ffglTexture3 = CreateFFGLtexture(x->ffglTexture3.Handle, x->g_Width, x->g_Height);
		}
	}
	else {
		// If the plugin requires a second input texture, create that too
		// We only allow for two input textures
		if(x->g_numInputsMax > 1 && x->ffglTexture2.Handle == 0) {
			x->ffglTexture2 = CreateFFGLtexture(x->ffglTexture2.Handle, x->g_Width, x->g_Height);
		}
	}

	
	// Save FBO etc
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
	
	// save the viewport size
	glGetFloatv(GL_VIEWPORT, vpdim);

	// Save texture state, client state, etc.
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// Syphon note :
	// Jitter uses multiple texture coordinate arrays on different units
	// We need to ensure we set this before changing our texture matrix
	// glActiveTexture selects which texture unit subsequent texture state calls will affect.
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0); // ensure we act on the proper client texture as well

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	// find the viewport size in order to save it
	glGetFloatv(GL_VIEWPORT, vpdim);

	glViewport(0, 0, x->g_Width, x->g_Height);

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0f, 1.0f);
                               
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	if(x->g_fbo1 == 0) glGenFramebuffersEXT(1, &x->g_fbo1); // FBO for the plugin to draw into

	// Otherwise, did it initialize ?
	if(x->bInitialized) {

		//
		// Render the incoming jitter textures into our local FFGL textures via fbo
		//

		// For a source plugin
		if(bSourcePlugin) {

			// this is necessary here but only for a source plugin
			// and only if there is an incoming jitter texture
			// Possibly a similar problem as referred to in Dest_Changed
			// Bind and unbind of the output works as well
			// but the output ID is already valid
			if(texname > 0) {
				jit_gl_bindtexture(&drawInfo, x->textureSource, 0);
				jit_gl_unbindtexture(&drawInfo, x->textureSource, 0);
			}

			// Clear the fbo from the previous frame
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, x->g_fbo1); 
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

			bRet = true;
		}
		else {
			// For an effect plugin, render from the first jitter texture to the first FFGL texture
			bRet = DrawToGLtexture(x, x->textureSource, x->g_fbo1, x->ffglTexture.Handle, GL_TEXTURE_2D);

			// We now have a local FFGL texture which can be passed to the FFGL plugin
			// and at the same time, render into the output texture via an fbo

			// If the plugin requires a second input texture, a second FFGL texture will have been created
			// so render the second Jitter input texture to the second FFGL texture
			if(x->g_numInputsMax > 1 && x->ffglTexture2.Handle > 0)
				bRet = DrawToGLtexture(x, x->textureSource2, x->g_fbo1, x->ffglTexture2.Handle, GL_TEXTURE_2D);

		}

		// We have a source texture to work with
		if(bRet) {

			// Pass the FFGL textures to the effect
			// The plugin will render into our fbo with the output texture attached
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, x->g_fbo1);

			//
			// Attach the output texture to the color buffer in our fbo
			//
			// As per Resolume communication with Edwin : 
			//     for a pure source plugin, render into an intermediate texture
			//     and then draw that into the destination with blending enabled.
			//
			// Otherwise let the plugin draw straight into the jitter texture
			//
			if(bSourcePlugin)
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, x->ffglTexture3.Handle, 0);
			else
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, outname, 0);

			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

			// clear the color buffer - otherwise Resolume plugins with "Black BG" show a white screen
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			if(x->bPluginLoaded && !x->bypass) {
	
				// prepare the structure used to call the plugin's ProcessOpenGL method
				ProcessOpenGLStructTag processStruct;
				processStruct.HostFBO = x->g_fbo1; // inform the plugin it is drawing into an fbo
				
				// Pass parameters to the plugin in case the user has changed them
				// TODO - only do this if changed ?
				for(int i=0; i<(int)x->ParamInfoList.size(); i++) {
					if(x->ParamInfoList.at(i).type == 0) { // checkbox
						// Can't use SetFloatParameter for state because the size
						// of boolean does not match cast from float (see FFGL code)
						x->plugin->SetFloatParameter(i, (float)x->ParamInfoList.at(i).value);
					}
					else if(x->ParamInfoList.at(i).type == 1) { // Momentary push button
						// Only act if it has been clicked in the dialog
						if(x->ParamInfoList.at(i).state == true) {
							x->plugin->SetFloatParameter(i, (float)x->ParamInfoList.at(i).value);
						}
						// Reset straight away
						x->ParamInfoList.at(i).state = false;
						x->ParamInfoList.at(i).value = 0.0;
					}
					else if(x->ParamInfoList.at(i).type == 100) { // Text
						// pass the address of the string (can be NULL)
						DWORD dwParam = (DWORD)x->ParamInfoList.at(i).text; 
						x->plugin->SetParameter(i, dwParam);
					}
					else { // all others are scrollbars
						x->plugin->SetFloatParameter(i, (float)x->ParamInfoList.at(i).value);
					}
				} // end all parameters

				//
				// FFGL time
				//
				// Tell the plugin the time if it supports time
				if(x->plugin->getPluginCaps(FF_CAP_SETTIME) == 1) {
					// whats the current time on the timer?
					x->curFrameTime = x->time->GetElapsedTime();
					// Tell the plugin
					x->plugin->SetTime(x->curFrameTime);
				}

				// Create the array of OpenGLTextureStruct * to be passed to the plugin
				// We allow for two input textures. The second will only be created if the plugin needs it
				// and there is a second Jitter input texture to use

				// A source plugin, or an effect plugin that does not need a texture, does not
				// have a texture to be passed because the result is generated by the plugin
				if(bSourcePlugin) {
					FFGLTextureStruct *inputTextures[1];
					inputTextures[0] = 0;
					processStruct.numInputTextures = 0;
					processStruct.inputTextures = inputTextures;
					dwRet = x->plugin->CallProcessOpenGL(processStruct); // call the plugin's ProcessOpenGL
				}
				else if(x->g_numInputsMax > 1) {
					// An effect can require two textures for combinations
					FFGLTextureStruct *inputTextures[2];
					inputTextures[0] = &x->ffglTexture; // provide our input textures
					inputTextures[1] = &x->ffglTexture2;
					processStruct.numInputTextures = 2;
					processStruct.inputTextures = inputTextures;
					dwRet = x->plugin->CallProcessOpenGL(processStruct);
				}
				else {
					// A typical effect needs only one texture
					FFGLTextureStruct *inputTextures[1];
					inputTextures[0] = &x->ffglTexture; // This is our local texture which is a copy of the jitter one
					processStruct.numInputTextures = 1;
					processStruct.inputTextures = inputTextures;
					dwRet = x->plugin->CallProcessOpenGL(processStruct);
				}
				if(dwRet != FF_SUCCESS) {
					// The plugin's CallProcessOpenGL failed
					glClearColor(0.0, 0.0, 0.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
				}
			} // endif plugin loaded
			else {
				// No plugin available, so just render the incoming Jitter texture into the output texture as-is
				if(texname) DrawJitterTexture(x);
			}

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

			// An effect plugin has rendered directly to the Jitter output texture.
			// But a source plugin has drawn to an OpenGL output texture, 
			// so now draw that into the jitter output texture.
			if(bSourcePlugin)
				DrawToJitterTexture(x, outname, GL_TEXTURE_RECTANGLE_EXT, x->g_fbo1, x->ffglTexture3.Handle, GL_TEXTURE_2D);

			// The Jitter output texture is finally output by the max draw function when it gets a bang

		} // end DrawToGLtexture or source OK
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
	glViewport((int)vpdim[0], (int)vpdim[1], (int)vpdim[2], (int)vpdim[3]);

	return JIT_ERR_NONE;

}
	
//
// attributes
//

// @loadeffect
t_jit_err jit_gl_freeframe_loadeffect(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv)
{
	t_symbol *name;
	char *effect;
	char thename[MAX_PATH];
	char effectpath[MAX_PATH];
	char effectname[MAX_PATH];

	if(x) {	

		if (argc && argv) {
			
			name = jit_atom_getsym(argv);
			strcpy_s(effectname, MAX_PATH, name->s_name);
			ToLowerCase(effectname);

			FindEffects(x); // refresh the effect list
			if(x->EffectList.size() > 0) {
				// Find a match with the given effect name
				for(int i=0; i<(int)x->EffectList.size(); i++) {
					
					strcpy_s(effectpath, MAX_PATH, x->EffectList.at(i).c_str());
					ToLowerCase(effectpath);
					effect = (char *)strstr(effectpath, effectname);

					// Split the file name out from the full path 
					_splitpath_s(effectpath, NULL, NULL, NULL, NULL, thename, MAX_PATH, NULL, 0);

					// Compare the effect name with the one being entered
					// Both are now lower case
					if(strcmp(thename, effectname) == 0) {

						// Found the effect name in the path string
						x->effectname = name; // record the name entered

						// Save the plugin name and path
						strcpy_s(x->g_PluginName, MAX_PATH, effect);
						strcpy_s(x->g_PluginPath, MAX_PATH, x->EffectList.at(i).c_str());

						// Unload the effect that is already loaded
						if(x->bPluginLoaded) UnloadFFGLplugin(x);

						// Now load the one selected (x->g_PluginPath)
						x->bPluginLoaded = LoadFFGLplugin(x, x->g_PluginPath);

						x->bInitialized = false; // set up again when there is an opengl context
					}
				}
			} 

			// Call draw to initialize
			jit_gl_freeframe_draw(x);

		}
		else {
			// no args, set to zero
			x->effectname = _jit_sym_nothing;
			x->g_PluginName[0] = 0;
		}
	}
	return JIT_ERR_NONE;
}


// @param - plugin parameter
// Set a parameter of a plugin
// Options :
//		Param name : value
//		Param number : value (default) : boolean (types 0/1) text (type 100)
//
t_jit_err jit_gl_freeframe_param(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv)
{
	t_symbol *name;
	t_symbol *char_array;
	char paramname[MAX_PATH];
	char paramstring[MAX_PATH];
	float fValue;
	int i, index;

	// If there is no plugin loaded don't do anything
	if(!x->bPluginLoaded) {
		return JIT_ERR_NONE;
	}
	
	if(x) {	
		if (argc && argv) {

			// The first arg will be a name or a number
			name = jit_atom_getsym(&argv[0]);
			strcpy_s(paramname, MAX_PATH, name->s_name);
			ToLowerCase(paramname); // not case sensitive

			index = jit_atom_getlong(&argv[0]);

			if(argc > 1) {
				// More than one arg means a parameter with a value
				// i.e. a scroll bar and not a button, but a checkbox can be on or off
				if(x->ParamInfoList.size() > 0) {
					// If a parameter name is supplied, look for this name in the paramstruct for this plugin
					if(strlen(paramname) > 0) {
						index = -1;
						for(i=0; i <(int)x->ParamInfoList.size(); i++) {
							strcpy_s(paramstring, MAX_PATH, x->ParamInfoList.at(i).name);
							ToLowerCase(paramstring);
							if(strstr(paramstring, paramname) != 0) {
								// There is a match
								index = i;
								break;
							}
						}
					}
					// Now index is either the one passed, or the name matched
					// The index has to be in the range of parameters available
					if(index >= 0 && index <(int)x->ParamInfoList.size()) {
						strcpy_s(paramname, MAX_PATH, x->ParamInfoList.at(index).name); // use again
						// What is the type of this parameter
						// It can be 0, 1 - checkbox
						// 100 = text
						// or all others are float
						if(x->ParamInfoList.at(index).type == 0) {
							// 0 - a checkbox
							fValue = (float)jit_atom_getfloat(&argv[1]);
							x->ParamInfoList.at(index).value = fValue;
							if(fValue >= 0.5)
								x->ParamInfoList.at(index).state = true;
							else
								x->ParamInfoList.at(index).state = false;
						}
						else if(x->ParamInfoList.at(index).type == 100) {
							// 100 - text
							char_array = jit_atom_getsym(&argv[1]);
							strcpy_s(paramstring, MAX_PATH, char_array->s_name);
							strcpy_s(x->ParamInfoList.at(index).text, 256, paramstring);

						}
						else {
							// default float slider control
							fValue = (float)jit_atom_getfloat(&argv[1]);
							x->ParamInfoList.at(index).value = fValue;
						}
					} // endif valid index
				} // endif there are parameters for this plugin
			} // endif parameter value was passed
			else {
				// Only one arg means it could be a button or checkbox toggle
				if(strlen(paramname) > 0) {
					index = -1;
					for(i=0; i <(int)x->ParamInfoList.size(); i++) {
						strcpy_s(paramstring, MAX_PATH, x->ParamInfoList.at(i).name);
						ToLowerCase(paramstring);
						if(strstr(paramstring, paramname) != 0) {
							// There is a match
							index = i;
							break;
						}
					}
				}
				// Now index is either the one passed, or the name matched
				// The index has to be in the range of parameters available
				if(index >= 0 && index <(int)x->ParamInfoList.size()) {
					strcpy_s(paramname, MAX_PATH, x->ParamInfoList.at(index).name); // use again
					// What is the type of this parameter
					// It can be 0, 1 - checkbox or button
					// 1 - a button
					if(x->ParamInfoList.at(index).type == 1) {
						// A button is a message that it was pressed
						// We just need to know that it was pressed
						// set state to true and reset to false again after sending it to the plugin
						x->ParamInfoList.at(index).state = true;
						x->ParamInfoList.at(index).value = 1.0;
					}
					else if(x->ParamInfoList.at(index).type == 0) {
						// 0 - a checkbox toggle
						fValue = x->ParamInfoList.at(index).value;
						// Toggle 
						if(fValue >= 0.5) {
							x->ParamInfoList.at(index).state = false;
							x->ParamInfoList.at(index).value = 0.0;
						}
						else {
							x->ParamInfoList.at(index).state = true;
							x->ParamInfoList.at(index).value = 1.0;
						}
					}
				} // endif valid index
			} // endif no parameter value
		} // endif parameters were passed
	} // endif valid x

	return JIT_ERR_NONE;
}


// @reload - rescan for new plugins
t_jit_err jit_gl_freeframe_reload(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv)
{
	FindEffects(x); // refresh the effect list
	return JIT_ERR_NONE;
}


// @dim - Set the output dimension - only used for source plugins
t_jit_err jit_gl_freeframe_setattr_dim(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv)
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


// @bypass
// Bypass the effect (default is 0 off) - toggles on bang (checkbox)
t_jit_err jit_gl_freeframe_bypass(t_jit_gl_freeframe *x)
{
	x->bypass = !x->bypass; // 0 off or 1 on

	return JIT_ERR_NONE;

}
// @paramdialog
// Activate the parameter dialog
t_jit_err jit_gl_freeframe_paramdialog(t_jit_gl_freeframe *x, void *attr, long argc, t_atom *argv)
{
	// If the dialog is open, close it. Otherwise open it.
	if(ParamWnd) {
		DestroyWindow(ParamWnd);
		ParamWnd = NULL;
	}
	else {
		if(x->bPluginLoaded && x->ParamInfoList.size() >0 )	EnterPluginParameters(x);
	}

	return JIT_ERR_NONE;
}


// Used to send the texture name to Max for texture output
t_jit_err jit_gl_freeframe_getattr_out_name(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av)
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


// Used to send the address of the plugin parameter list to Max for output
// TODO : disable or send NULL for no plugin loaded, what to send for no parameters ?
t_jit_err jit_gl_freeframe_getattr_out_param(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av)
{
	DWORD pParamList;
	DWORD dwRetValue;
	char *pDisplay;
	int i;

	// If there is no plugin loaded don't do anything
	if(!x->bPluginLoaded) {
		return JIT_ERR_NONE;
	}

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

	if(x->bPluginLoaded && x->bPluginInitialized && x->ParamInfoList.size() > 0) {
		// Find the alternate parameter value display for all the parameters
		for(i=0; i<(int)x->ParamInfoList.size(); i++) {
			dwRetValue = x->plugin->getParameterDisplay(i);
			if(dwRetValue != FF_FAIL) {
				pDisplay = (char *)dwRetValue;
				if(pDisplay) {
					//FreeFrame spec defines parameter names to be 16 characters long MAX
					// Up to 16 chars not null terminated
					// CopyString adds the null
					CopyString(x->ParamInfoList.at(i).display, pDisplay, 16);
				}
			}
		}
	}

	// ParamInfoList can be empty but pass it back anyway
	pParamList = (DWORD)&x->ParamInfoList;
	ps_paramlist->s_thing = (t_object *)pParamList; 
	jit_atom_setsym(*av, ps_paramlist);

	return JIT_ERR_NONE;
}											  

// Used to send the address of the effect list to Max for output
t_jit_err jit_gl_freeframe_getattr_out_effect(t_jit_gl_freeframe *x, void *attr, long *ac, t_atom **av)
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

	// vector<string> EffectList;
	FindEffects(x); // refresh the list

	DWORD pEffectList = (DWORD)&x->EffectList; // pointer to the effect list vector
    ps_effectlist->s_thing = (t_object *)pEffectList; 
	jit_atom_setsym(*av, ps_effectlist);


	return JIT_ERR_NONE;
}											  


// ===================================
t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv)
{
	return JIT_ERR_NONE;

}


// Find freeframe plugins
bool FindEffects(t_jit_gl_freeframe *x)
{
	// Get all the effects in the plugin folder
	char PluginPath[MAX_PATH];
	char FFGLfile[MAX_PATH];
	char FilePath[MAX_PATH];
	char path[MAX_PATH];
	char name[MAX_PATH];
	char pf[MAX_PATH]; // Program files path
	HANDLE hFind = NULL;
	typedef vector<string>::iterator Iterator;
	string effectstring;
	WIN32_FIND_DATA fd; 
	int i, j;
	bool bFound;
	char *whereis;
	short appPath;
	short sresult;

	// We have to load plugins to find out what type they are
	WinPluginInstance *plugin;
	plugin = new WinPluginInstance;

	if(x->EffectList.size() > 0) x->EffectList.erase (x->EffectList.begin(), x->EffectList.end() );

	//
	// MAX SDK path functions
	//
	// short path_getdefault(void) 
	// Retrieve the Path ID of the default search path. 
	// By default, Max searches the patches/ and examples/ folders in the main application folder,
	// including all subdirectories.
	// This should be the patch folder.
	//
	// short path_topotentialname(C74_CONST short path, C74_CONST char *file, char *name, short check)
 	// Create a fully qualified file name from a Path ID/file name combination, 
	// regardless of whether or not the file exists on disk. 
	// Seems to work OK with NULL file name to get just the path

	//
	// First find all the effects in the patch folder
	//

	// Find the patch path - if the MAX SDK method does not work, use getcwd
	appPath = path_getdefault();
	sresult = path_topotentialname(appPath, NULL, PluginPath, 0);
	if(sresult != 0) {
		_getcwd(PluginPath, MAX_PATH);
	}

	// Find all the dll files in that folder
	sprintf_s(FilePath, MAX_PATH, "%s\\*.dll", PluginPath);
	hFind = FindFirstFile(FilePath, &fd); 
	if(hFind != INVALID_HANDLE_VALUE) { 
		i = 0;
		do  { 
            // read all dll files in current folder
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
				strcpy_s(FFGLfile, fd.cFileName);
				whereis = strchr(FFGLfile, '.'); *whereis = 0;
				// Create the plugin full path
				sprintf_s(FilePath, MAX_PATH, "%s\\%s", PluginPath, fd.cFileName);
				// We need to check whether it is an FFGL plugin so have to load it to find out
				if(plugin->Load(FilePath) != FF_FAIL) {
					if(plugin->getPluginCaps(FF_CAP_PROCESSOPENGL) == 1) { // Only select FFGL plugins
						effectstring = FilePath;
						x->EffectList.push_back(effectstring); // the list of effects
					}
					plugin->Unload();
				}
				i++;
            }
        } while(FindNextFile(hFind, &fd) != 0); 
        FindClose(hFind); 
    }

	// Next find all the effects in the Program Files\Common Files\Freeframe
	// but exclude any already loaded from the patch path
	//
	// SHGetKnownFolderPath ?
	// Registry ?
	// http://stackoverflow.com/questions/445139/how-to-get-program-files-folder-path-not-program-files-x86-from-32bit-wow-pr
	//
	// SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILES, FALSE); 
	// PWSTR wpath;
	// SHGetKnownFolderPath((REFKNOWNFOLDERID)CSIDL_PROGRAM_FILES, KF_FLAG_DEFAULT_PATH, NULL, &wpath);
	LPWSTR wszPath = NULL;
	HRESULT hr;
	PluginPath[0] = 0;

	// "FOLDERID_ProgramFilesX64" is not supported for 32-bit applications running on 64-bit operating systems.
	// We can look for "Program Files X86" and if it exists, it will be "Program Files" for a 32bit system.
	// For a 64bit system find out whether the user has created a Freeframe fiolder under the x86 folder first
	// If not, look at the "Program Files" folder which will exist for both 32bit and 64bit.
	hr = SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, KF_FLAG_DEFAULT_PATH, NULL, &wszPath);
	if (SUCCEEDED(hr)) {
		_bstr_t bstrPath(wszPath);
		string strPath((char*)bstrPath);
		sprintf_s(PluginPath, MAX_PATH, "%s\\Common Files\\Freeframe", strPath.c_str()); // the freeframe folder path
		// Does the folder exist ?
		if(_access(PluginPath, 0) == -1) { // Mode 0 - existence check
			// It does not, so look for the "Program Files Folder"
			hr = SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT_PATH, NULL, &wszPath);
			if (SUCCEEDED(hr)) {
				_bstr_t bstrPath(wszPath);
				string strPath((char*)bstrPath);
				sprintf_s(PluginPath, MAX_PATH, "%s\\Common Files\\Freeframe", strPath.c_str()); // the freeframe path
			}
		}
	}
	else {
		// Try this last
		SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILES, FALSE); 
		sprintf_s(PluginPath, MAX_PATH, "%s\\Common Files\\Freeframe", pf); // the freeframe path
	}
	
	// Did we get a plugin folder path  and does the folder exist ?
	if(PluginPath[0] == 0 || _access(PluginPath, 0) == -1) {
		// just quit if it does not
		delete(plugin);
		return false;
	}

	// Find all the dll files in that folder
	sprintf_s(FilePath, MAX_PATH, "%s\\*.dll", PluginPath); // all dll files in the freeframe path
	hFind = FindFirstFile(FilePath, &fd); 
	if(hFind != INVALID_HANDLE_VALUE) { 
		i = 0;
		do  { 
            // read all dll files in current folder
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
				strcpy_s(FFGLfile, fd.cFileName);
				whereis = strchr(FFGLfile, '.'); *whereis = 0;
				// Create the plugin full path
				sprintf_s(FilePath, MAX_PATH, "%s\\%s", PluginPath, fd.cFileName);
				// Is it already in the effect list ?
				bFound = false;
				if(x->EffectList.size() > 0) {
					for(j=0; j<(int)x->EffectList.size(); j++) {
						sprintf_s(path, MAX_PATH, "%s", x->EffectList.at(j).c_str());
						_splitpath_s(path, NULL, NULL, NULL, NULL, name, MAX_PATH, NULL, 0);
						strcat(name, ".dll"); // restore the extension for the match
						// Compare the effect name with the one being entered
						if(strcmp(name, fd.cFileName) == 0)	{
							bFound = true;
						}
					}
				}
				if(!bFound) {
					// We need to check whether it is an FFGL plugin so have to load it to find out
					if(plugin->Load(FilePath) != FF_FAIL) {
						if(plugin->getPluginCaps(FF_CAP_PROCESSOPENGL) == 1) { // Only select FFGL plugins
							effectstring = FilePath;
							x->EffectList.push_back(effectstring); // the list of effects
						}
						plugin->Unload();
					}
				}
				i++;
            }
        } while(FindNextFile(hFind, &fd) != 0); 
        FindClose(hFind); 
    }
	else {
		delete(plugin);
		return false;
	}

	delete(plugin);

	return true;

}


// Create a freeframe texture
// LJ - disable power of 2
FFGLTextureStruct CreateFFGLtexture(GLuint glTextureHandle, int textureWidth, int textureHeight)
{
  // note - there must be an active opengl context when this is called
  // ie, wglMakeCurrent(someHDC, someHGLRC)
  /*
  //find smallest power of two sized texture that can contain the texture  
  int glTextureWidth = 1;
  while (glTextureWidth<textureWidth) glTextureWidth *= 2;

  int glTextureHeight = 1;
  while (glTextureHeight<textureHeight) glTextureHeight *= 2;
  */
  int glTextureWidth = textureWidth;
  int glTextureHeight = textureHeight;

  if(glTextureHandle != 0) glDeleteTextures(1, &glTextureHandle);
  glTextureHandle = 0;

  //create and setup the gl texture
  // GLuint glTextureHandle = 0;
  glGenTextures(1, &glTextureHandle);

  // bind this texture so that glTex* calls apply to it
  glBindTexture(GL_TEXTURE_2D, glTextureHandle);
  
  // use bilinear interpolation when the texture is scaled larger than its true size
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // no mipmapping (for when the texture is scaled smaller than its true size)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  //no wrapping (for when texture coordinates reference outside the bounds of the texture)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // allocate room for the gl texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glTextureWidth, glTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  //unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  
  //fill the OpenGLTextureStruct
  FFGLTextureStruct t;

  t.Handle = glTextureHandle;

  t.Width = textureWidth;
  t.Height = textureHeight;
  
  t.HardwareWidth = glTextureWidth;
  t.HardwareHeight = glTextureHeight;

  return t;

}

// Draw a texture - test function
bool DrawGLTexture(GLuint sourceTexture, GLuint sourceTarget)
{

	/*
	glColor4f(1.f, 0.f, 0.f, 1.f);
	glBegin(GL_QUADS);
	glVertex3f(-1,-1,0);
	glVertex3f(-1,1,0);
	glVertex3f(1,1,0);
	glVertex3f(1,-1,0);
	glEnd();
	*/

	glEnable(sourceTarget);
	glBindTexture(sourceTarget, sourceTexture); // bind source texture

	// glEnable (GL_BLEND); 
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.f, 0.f, 0.f, 1.f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex2f(-1.0,-1.0); // lower left
	glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0, 1.0); // upper left
	glTexCoord2f(1.0, 1.0);	glVertex2f( 1.0, 1.0); // upper right
	glTexCoord2f(1.0, 0.0);	glVertex2f( 1.0,-1.0); // lower right
	glEnd();

	// glDisable(GL_BLEND); 

	glBindTexture(sourceTarget, 0); // unbind source texture
	glDisable(sourceTarget);
	

	return true;

}


// Draw the Jitter texture, usually into an fbo
bool DrawJitterTexture(t_jit_gl_freeframe *x)
{
	// our texture has to be bound using jitter functions
	t_jit_gl_drawinfo drawInfo;
	jit_gl_drawinfo_setup(x, &drawInfo);

	glEnable(GL_TEXTURE_RECTANGLE_EXT);
	jit_gl_bindtexture(&drawInfo, x->textureSource, 0); // bind the the jitter texure and draw it
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	jit_gl_texcoord2f(&drawInfo, 0.0, 0.0);
	glVertex2f(-1, -1); // lower left

	jit_gl_texcoord2f(&drawInfo, 0.0, 1.0);
	glVertex2f(-1,  1); // upper left

	jit_gl_texcoord2f(&drawInfo, 1.0, 1.0);
	glVertex2f( 1,  1); // upper right

	jit_gl_texcoord2f(&drawInfo, 1.0, 0.0);
	glVertex2f( 1, -1); // lower right

	glEnd();

	jit_gl_unbindtexture(&drawInfo, x->textureSource, 0);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);

	return true;

}

// Draw the Jitter texture into another texture via an fbo
bool DrawToGLtexture(t_jit_gl_freeframe *x, t_symbol *texture, GLuint FBO, GLuint TextureID, GLuint TextureTarget)
{
	bool bRet = false;
	t_jit_gl_drawinfo drawInfo;
	jit_gl_drawinfo_setup(x, &drawInfo);


	// our texture has to be bound using jitter functions
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO); 

	// Don't clear here for combination plugins
	// glClearColor(0, 0, 0, 0.0);
	// glClear(GL_COLOR_BUFFER_BIT);

	// Attach the texture desination to the color buffer in our frame buffer 
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureTarget, TextureID, 0); 

	if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {

		// draw into it with the jitter texture
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		jit_gl_bindtexture(&drawInfo, texture, 0); // bind the the jitter texure and draw it
		
		// QUAD draw
		glBegin(GL_QUADS);
		glColor4f(1.0, 1.0, 1.0, 1.0);

		jit_gl_texcoord2f(&drawInfo, 0.0, 0.0);
		glVertex2f(-1, -1); // lower left

		jit_gl_texcoord2f(&drawInfo, 0.0, 1.0);
		glVertex2f(-1,  1); // upper left

		jit_gl_texcoord2f(&drawInfo, 1.0, 1.0);
		glVertex2f( 1,  1); // upper right

		jit_gl_texcoord2f(&drawInfo, 1.0, 0.0);
		glVertex2f( 1, -1); // lower right

		glEnd();

		jit_gl_unbindtexture(&drawInfo, texture, 0);
		glDisable(GL_TEXTURE_RECTANGLE_EXT);

		bRet = true;
	}
	else {
		bRet = false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 


	return bRet;

}

// Draw a texture into the jitter texture via an fbo
bool DrawToJitterTexture(t_jit_gl_freeframe *x, GLuint destTexture, GLuint destTarget, GLuint FBO, GLuint sourceTexture, GLuint sourceTarget)
{
	bool bRet = false;
	t_jit_gl_drawinfo drawInfo;
	jit_gl_drawinfo_setup(x, &drawInfo);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO); 

	// Attach the texture desination to the color buffer in our frame buffer 
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, destTarget, destTexture, 0);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {

		glEnable (GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.f, 1.f, 1.f, 1.f);

		// draw into it with the source texture
		glEnable(sourceTarget);
		glBindTexture(sourceTarget, sourceTexture); // bind source texture

		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);	glVertex2f(-1.0,-1.0); // lower left
		glTexCoord2f(0.0, 1.0);	glVertex2f(-1.0, 1.0); // upper left
		glTexCoord2f(1.0, 1.0);	glVertex2f( 1.0, 1.0); // upper right
		glTexCoord2f(1.0, 0.0);	glVertex2f( 1.0,-1.0); // lower right
		glEnd();

		glBindTexture(sourceTarget, 0); // unbind source texture
		glDisable(sourceTarget);

		glDisable(GL_BLEND); 

		bRet = true;
	}
	else {
		bRet = false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 

	return bRet;

}


// Load a plugin given the file path
bool LoadFFGLplugin(t_jit_gl_freeframe *x, const char *pluginfile)
{

	DWORD nParams;
	DWORD dwRetValue, dwType;
	// char *pDisplay;
	char *pName;
	float fValue;

	// This just loads a plugin and does not instantiate it
	// so an opengl context is not needed
	if (x->plugin->Load(pluginfile) != FF_FAIL) {

		// We need to know the number of inputs to set up the necessary textures
		x->g_numInputsMin = x->plugin->getPluginCaps(FF_CAP_MINIMUMINPUTFRAMES);
		x->g_numInputsMax = x->plugin->getPluginCaps(FF_CAP_MAXIMUMINPUTFRAMES);

		// If it is not a context change, it is a different plugin, so load the default parameters
		if(!x->bDestChanged) {

			// Find the type and name of the parameters for this plugin
			dwType = x->plugin->getType();
			nParams = x->plugin->getNumParameters();

			// Get and save default plugin parameters
			if(x->ParamInfoList.size() > 0) x->ParamInfoList.erase (x->ParamInfoList.begin(), x->ParamInfoList.end() );
			if(nParams > 0) {
				for(int i=0; i<(int)nParams; i++) {

					FFGLParamInfo info;

					// info.value (0 - 11)
					// info.state - (0 and 1)
					// info.type 0(checkbox) 1(button)
					// info.text (only for type 100)
					// info.name; // parameter name
					// info.display; // alternate parameter display value (other than actual value)

					// Set defaults
					info.type = 10; // can be -1 perhaps
					info.value = 0.5;
					info.state = false;
					info.text[0] = 0;
					info.name[0] = 0;
					info.display[0] = 0;
				
					// Get the plugin parameter type and name
					info.type = x->plugin->getParameterType(i);
					pName = (char *)x->plugin->GetParameterName(i);
					if(pName) {
						// Up to 16 chars not null terminated
						// CopyString adds the null
						CopyString(info.name, pName, 16);
					}
					// Find the values the plugin has set as defaults
					dwRetValue = x->plugin->getParameterDefault(i);
					if(dwRetValue != FF_FAIL) {
						fValue = *((float *)(unsigned)&dwRetValue);
						info.value = fValue;
						if(info.type == 0) { // checkbox
							if(fValue >= 0.5)
								info.state = true;
							else
								info.state = false;
						}
						else if(info.type == 1) { // button
								info.state = false;
								info.value = 0.0; // alwasy default unclicked
						}
						else if(info.type == 100) { // text
							char *text = (char *)dwRetValue;
							if(text != NULL) {
								strcpy_s(info.text, 256, text);
							}
							else {
								info.text[0] = 0;
							}
						}
					} // end get default for this param

					// Save the info for the loaded plugin
					x->ParamInfoList.push_back(info);
				} // end all params
				// Now we have a list of parameter types and names
			} // end nparams > 0

			// Reset the timer for the plugin if it supports time
			if(x->plugin->getPluginCaps(FF_CAP_SETTIME) == 1) {
				x->time->Reset();
			}

		} // end a different plugin
		else {
			x->bDestChanged = false;
		}

		return true;
	} // end plugin loaded OK

	return false;	
}

bool UnloadFFGLplugin(t_jit_gl_freeframe *x)
{
	// If one is already loaded and initialized, de-initialize and unload it

	// de-instantiate
	if(x->bPluginInitialized) 
		x->plugin->DeInstantiateGL();

	// Unload the plugin
	if(x->bPluginLoaded) x->plugin->Unload();

	x->g_numInputsMin = 0; // default for the next plugin to be loaded
	x->g_numInputsMax = 2;
	x->bPluginInitialized = false;
	x->bPluginLoaded = false;
	
	return true;
}

// Initialization separate because it needs an openGL context
// and loading / unloading may be done without one
// must be called after loading and before initialization
bool InitializeFFGLplugin(t_jit_gl_freeframe *x)
{
	if(x->bPluginLoaded) {

		// Create the output window viewport
		FFGLViewportStruct windowViewport;
		windowViewport.x = 0;
		windowViewport.y = 0;
		windowViewport.width =  x->g_Width;
		windowViewport.height = x->g_Height;

		// instantiate the FFGL plugin with the viewport
		if (x->plugin->InstantiateGL(&windowViewport) == FF_SUCCESS) {
			return true;
		}
		else {
			x->plugin->Unload();
			x->bPluginInitialized = false;
			x->bPluginLoaded = false;
		}
	} // plugin was loaded

	return false;
}

bool UnInitializeFFGLplugin(t_jit_gl_freeframe *x)
{
	if(x->bPluginLoaded) {
		// de-instantiate
		if(x->bPluginInitialized) {
			x->plugin->DeInstantiateGL();
			x->bPluginInitialized = false;
		}
	}

	return false;
}

// Utility
void ToLowerCase(char *str)
{
	for(int j=0; j<(int)strlen(str); j++) {	if(str[j] >= 65 && str[j] <= 90) str[j] = str[j]+32; }
}

// Limited to 512 chars
// Needed for un-terminated strings in FFGL
void CopyString(char *dest, char *source, int maxChars)
{
	int i = 0;
	char *src = source;
	char *dst = dest;
	
	while (*src && i < maxChars && i < 512) {
		*dest++ = *src++;
		i++;
	}
    // make sure the last char is a null
	*dest = 0;
}



bool EnterPluginParameters(t_jit_gl_freeframe *x)
{
	int i;
	// INT_PTR nRet;
	HMODULE hModule;
	DWORD dwRetValue;
	float fValue;
	FFGLParamInfo info;
	// char *pName;

	// Preserve the class pointer in a static pointer
	xs = (t_object *)x;
	
	// Convert scrollbar values to static ints
	for(i=0; i<30; i++) ScrollBarPos[i] = 0;
	if(InfoList.size() > 0) InfoList.erase (InfoList.begin(), InfoList.end() );
	if(infoBackup.size() > 0) infoBackup.erase (infoBackup.begin(), infoBackup.end() );
	for(i=0; i<(int)x->ParamInfoList.size(); i++) {

		// Parameter name for debug
		// pName = (char *)x->plugin->GetParameterName(i);
		// if(pName) CopyString(info.name, pName, 16);

		// Get the current parameter from the plugin
		dwRetValue = x->plugin->getParameter(i);
		if(dwRetValue != FF_FAIL) {
			fValue = *((float *)(unsigned)&dwRetValue);
			info.value = fValue;
			if(x->ParamInfoList.at(i).type == 0) { // Checkbox
				if(fValue >= 0.5) {
					info.state = true;
					info.value = 1.0;
				}
				else {
					info.state = false;
					info.value = 1.0;
				}
			}
			else if(x->ParamInfoList.at(i).type == 0) { // always turn buttons off
					info.state = false;
					info.value = 0.0;
			}
			else if(x->ParamInfoList.at(i).type == 100) { // text
				char *text = (char *)dwRetValue;
				if(text != NULL) {
					strcpy_s(info.text, 256, text);
				}
				else {
					info.text[0] = 0;
				}
			}
		}

		InfoList.push_back(x->ParamInfoList.at(i)); // fill static vector
		infoBackup.push_back(x->ParamInfoList.at(i)); // fill static backup vector for cancel
		if(x->ParamInfoList.at(i).type == 10)
			ScrollBarPos[i] = (int)(x->ParamInfoList.at(i).value*100.0+0.5); // The 0.5 avoids integer truncation
	}

	// The hInst parameter is the handle to the module that contains the dialog resource. 
	// If you want to get the dialog from the DL's resources, then pass the handle to the DLL
	// rather than the handle to the main application
	hModule = GetCurrentModule();

	// Create the modeless dialog
	if(!ParamWnd) ParamWnd = CreateDialog(hModule, MAKEINTRESOURCEA(IDD_PARAMETERS), NULL, (DLGPROC)PluginParameters);

	return true;
}


// Message handler for plugin parameters
LRESULT CALLBACK PluginParameters(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning
	static int iPos, dn;
	int i, j, n, width, height;
	float fValue;
	char value[16];
	static HWND hBar, hBox;
	DWORD dwParam;
	char text[256];
	RECT rc;

	switch (message) {
		
		case WM_INITDIALOG:

			// Keep it on top
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 

			// How big is the dialog - allow for a max of 30 parameters at 32 Y each
			i = (int)InfoList.size();
			GetWindowRect(hDlg, &rc);
			width = rc.right-rc.left;
			height = 100+(i-1)*32;
			SetWindowPos(hDlg, NULL, rc.left, rc.top, width, height, SWP_NOZORDER | SWP_NOMOVE);

			for(i=0; i<30; i++) {
				n = i*10;
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMTEXT0+n),   SW_HIDE); // text input
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMNAME0+n),   SW_HIDE); // parameter name
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMCLICK0+n),  SW_HIDE); // button
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMSLIDER0+n), SW_HIDE); // slider
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMVALUE0+n),  SW_HIDE); // slider value
				ShowWindow(GetDlgItem(hDlg, IDC_PARAMSTATE0+n),  SW_HIDE); // check box
			}

			// Hide or show dialog options depending on parameter type
			if(InfoList.size() > 0) {
				for(i=0; i<(int)InfoList.size(); i++) {
					n = i*10;

					hBar = GetDlgItem(hDlg, IDC_PARAMSLIDER0+n); // slider
					hBox = GetDlgItem(hDlg, IDC_PARAMSTATE0+n); // check box

					ShowWindow(GetDlgItem(hDlg, IDC_PARAMNAME0+n),   SW_HIDE); // parameter name
					ShowWindow(GetDlgItem(hDlg, IDC_PARAMTEXT0+n),   SW_HIDE); // text input
					ShowWindow(GetDlgItem(hDlg, IDC_PARAMSLIDER0+n), SW_HIDE); // slider
					ShowWindow(GetDlgItem(hDlg, IDC_PARAMCLICK0+n),  SW_HIDE); // button
					ShowWindow(GetDlgItem(hDlg, IDC_PARAMSTATE0+n),  SW_HIDE); // check box

					// Set the name in the text field
					SetDlgItemTextA(hDlg, IDC_PARAMNAME0+n, (LPCSTR)InfoList.at(i).name);
					ShowWindow(GetDlgItem(hDlg, IDC_PARAMNAME0+n),  SW_SHOW);

					// Check the check boxes and set scroll positions

					// What is the type
					switch(InfoList.at(i).type) {
						// 0 - state
						// 1 - button
						// 100 - text
						// all others are sliders

						case 0 :
							ShowWindow(GetDlgItem(hDlg, IDC_PARAMSTATE0+n), SW_SHOW); // check box
							// Check the box depending on the saved state
							if(InfoList.at(i).state) {
								CheckDlgButton(hDlg, IDC_PARAMSTATE0+n, BST_CHECKED);
							}
							else {
								CheckDlgButton(hDlg, IDC_PARAMSTATE0+n, BST_UNCHECKED);
							}
							break;

						case 1 :
							ShowWindow(GetDlgItem(hDlg, IDC_PARAMNAME0+n),  SW_HIDE); // parameter name
							ShowWindow(GetDlgItem(hDlg, IDC_PARAMCLICK0+n), SW_SHOW); // button
							// Print the name on the button
							SetDlgItemTextA(hDlg, IDC_PARAMCLICK0+n, (LPCSTR)InfoList.at(i).name);
							// Deactivate the button
							InfoList.at(i).state = false;
							InfoList.at(i).value = 0.0;
							break;

						case 100 :
							// Set the text in the text field
							SetDlgItemTextA(hDlg, IDC_PARAMTEXT0+n, (LPCSTR)InfoList.at(i).text);
							ShowWindow(GetDlgItem(hDlg, IDC_PARAMTEXT0+n),  SW_SHOW); // text input
							// Select all text in the edit field
							SendDlgItemMessage (hDlg, IDC_PARAMTEXT0+n, EM_SETSEL, 0, 0x7FFF0000L);
							break;

						default :
							hBar = GetDlgItem(hDlg, IDC_PARAMSLIDER0+n);
							// Set the scroll bar limits and text
							SetScrollRange(hBar, SB_CTL, 0, 100, FALSE);
							SetScrollPos(hBar, SB_CTL, ScrollBarPos[i], TRUE);
							sprintf_s(value, 16, "%4.2f", (float)ScrollBarPos[i]/100.0);
							SetDlgItemTextA(hDlg, IDC_PARAMVALUE0+n, (LPCSTR)value);

							ShowWindow(GetDlgItem(hDlg, IDC_PARAMSLIDER0+n), SW_SHOW); // slider
							ShowWindow(GetDlgItem(hDlg, IDC_PARAMVALUE0+n), SW_SHOW); // slider value
							break;

					}
				} // end for all parameters
			} // there were parameters
			return TRUE;

       case WM_HSCROLL:
			hBar = (HWND)lParam;
            iPos = GetScrollPos(hBar, SB_CTL);
            // Calculate new horizontal scroll position
            switch (LOWORD(wParam)) {
                case SB_LINEDOWN:
                        dn =  1;
                        break;
                case SB_LINEUP:
                        dn = -1;
                        break;
                case SB_PAGEDOWN:
                        dn =  128;
                        break;
                case SB_PAGEUP:
                        dn = -128;
                        break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                        dn = HIWORD(wParam) - iPos;
                        break;
                default:
                    dn = 0;
            }
            iPos = iPos+dn;

			// Reset text & values and change only scrollbar values for the plugin
			for(i=0; i<(int)InfoList.size(); i++) {
				n = i*10; // Each parameter's ID's are separated by 10
				if(InfoList.at(i).type == 10) { // scrollbars
					if(hBar == GetDlgItem(hDlg, IDC_PARAMSLIDER0+n)) {
						// Save the value in the static scroll bar pos
						ScrollBarPos[i] = iPos;
						SetScrollPos (hBar, SB_CTL, iPos, TRUE);
						// Safety
						fValue = ((float)iPos)/100.0;
						if(fValue < 0.0) fValue = 0.0;
						if(fValue > 1.0) fValue = 1.0;
						sprintf_s(value, 16, "%4.2f", fValue);
						SetDlgItemTextA(hDlg, IDC_PARAMVALUE0+n, (LPCSTR)value);
						InfoList.at(i).value = fValue;
					}
				}
				else if(InfoList.at(i).type == 1) { // buttons
					InfoList.at(i).state = false;
					InfoList.at(i).value = 0.0;
				}
			}
			UpdateParameterList(xs);
			break;

	   case WM_CLOSE :
		   // Will not save or restore
		   if(ParamWnd) DestroyWindow(ParamWnd);
		   DestroyWindow(hDlg);
		   ParamWnd = NULL;
		   return FALSE;
	   
	   case WM_COMMAND:

			switch(LOWORD(wParam)) {

				// CHECKBOXES - 30 possibilities
				// These change the parameter which is picked up in Draw straight away.
				case IDC_PARAMSTATE0:
				case IDC_PARAMSTATE1:
				case IDC_PARAMSTATE2:
				case IDC_PARAMSTATE3:
				case IDC_PARAMSTATE4:
				case IDC_PARAMSTATE5:
				case IDC_PARAMSTATE6:
				case IDC_PARAMSTATE7:
				case IDC_PARAMSTATE8:
				case IDC_PARAMSTATE9:
				case IDC_PARAMSTATE10:
				case IDC_PARAMSTATE11:
				case IDC_PARAMSTATE12:
				case IDC_PARAMSTATE13:
				case IDC_PARAMSTATE14:
				case IDC_PARAMSTATE15:
				case IDC_PARAMSTATE16:
				case IDC_PARAMSTATE17:
				case IDC_PARAMSTATE18:
				case IDC_PARAMSTATE19:
				case IDC_PARAMSTATE20:
				case IDC_PARAMSTATE21:
				case IDC_PARAMSTATE22:
				case IDC_PARAMSTATE23:
				case IDC_PARAMSTATE24:
				case IDC_PARAMSTATE25:
				case IDC_PARAMSTATE26:
				case IDC_PARAMSTATE27:
				case IDC_PARAMSTATE28:
				case IDC_PARAMSTATE29:
					n = (int)(LOWORD(wParam));
					i = (n - (int)IDC_PARAMSTATE0)/10;
					if(IsDlgButtonChecked(hDlg, n) == BST_CHECKED) {
						InfoList.at(i).state = true;
						InfoList.at(i).value = 1.0;
					}
					else {
						InfoList.at(i).state = false;
						InfoList.at(i).value = 0.0;
					}
					// pass the other possible fields to the plugin as well
					for(i=0; i<(int)InfoList.size(); i++) {
						n = i*10;
						if(InfoList.at(i).type == 10) { // Scrollbars
							InfoList.at(i).value = (float)(ScrollBarPos[i])/100;
						}
						else if(InfoList.at(i).type == 1) { // buttons
							InfoList.at(i).state = false;
							InfoList.at(i).value = 0.0;
						}
						else if(InfoList.at(i).type == 100) { // Text
							// Get contents of the edit field
							GetDlgItemTextA(hDlg, IDC_PARAMTEXT0+n, (LPSTR)text, 256);
							strcpy_s(InfoList.at(i).text, 256, text);
						}
					}
					UpdateParameterList(xs);
					break;

				// BUTTONS - 30 possibilities
				case IDC_PARAMCLICK0 :
				case IDC_PARAMCLICK1 :
				case IDC_PARAMCLICK2 :
				case IDC_PARAMCLICK3 :
				case IDC_PARAMCLICK4 :
				case IDC_PARAMCLICK5 :
				case IDC_PARAMCLICK6 :
				case IDC_PARAMCLICK7 :
				case IDC_PARAMCLICK8 :
				case IDC_PARAMCLICK9 :
				case IDC_PARAMCLICK10 :
				case IDC_PARAMCLICK11 :
				case IDC_PARAMCLICK12 :
				case IDC_PARAMCLICK13 :
				case IDC_PARAMCLICK14 :
				case IDC_PARAMCLICK15 :
				case IDC_PARAMCLICK16 :
				case IDC_PARAMCLICK17 :
				case IDC_PARAMCLICK18 :
				case IDC_PARAMCLICK19 :
				case IDC_PARAMCLICK20 :
				case IDC_PARAMCLICK21 :
				case IDC_PARAMCLICK22 :
				case IDC_PARAMCLICK23 :
				case IDC_PARAMCLICK24 :
				case IDC_PARAMCLICK25 :
				case IDC_PARAMCLICK26 :
				case IDC_PARAMCLICK27 :
				case IDC_PARAMCLICK28 :
				case IDC_PARAMCLICK29 :
					// Now set this particular button
					n = (int)(LOWORD(wParam));
					j = (n - (int)IDC_PARAMCLICK0)/10;
					// Reset the other things too
					for(i=0; i<(int)InfoList.size(); i++) {
						n = i*10;
						// Checkboxes
						if(InfoList.at(i).type == 0) { // checkbox
							if(IsDlgButtonChecked(hDlg, IDC_PARAMSTATE0+n) == BST_CHECKED) {
								InfoList.at(i).state = true;
								InfoList.at(i).value = 1.0;
							}
							else {
								InfoList.at(i).state = false;
								InfoList.at(i).value = 0.0;
							}
						}
						else if(InfoList.at(i).type == 1) { // buttons
							InfoList.at(i).state = false;
							InfoList.at(i).value = 0.0;
							// If it is the selected button, set state true
							if(i == j) {
								InfoList.at(i).state = true;
								InfoList.at(i).value = 1.0;
							}
						}
						else if(InfoList.at(i).type == 100) { // Text
							// Get contents of the edit field
							GetDlgItemTextA(hDlg, IDC_PARAMTEXT0+n, (LPSTR)text, 256);
							strcpy_s(InfoList.at(i).text, 256, text);
						}
						else if(InfoList.at(i).type == 10) { // Scrollbars
							InfoList.at(i).value = (float)(ScrollBarPos[i])/100;
						}
					}
					UpdateParameterList(xs);
					break;
				break;

				case IDOK :
					// pass all fields to the plugin
					for(i=0; i<(int)InfoList.size(); i++) {
						n = i*10;
						// Checkboxes
						if(InfoList.at(i).type == 0) { // checkbox
							if(IsDlgButtonChecked(hDlg, IDC_PARAMSTATE0+n) == BST_CHECKED) {
								InfoList.at(i).state = true;
								InfoList.at(i).value = 1.0;
							}
							else {
								InfoList.at(i).state = false;
								InfoList.at(i).value = 0.0;
							}
						}
						else if(InfoList.at(i).type == 1) { // buttons
							InfoList.at(i).state = false;
							InfoList.at(i).value = 0.0;
						}
						else if(InfoList.at(i).type == 10) { // Scrollbars
							InfoList.at(i).value = (float)(ScrollBarPos[i])/100;
						}
						else if(InfoList.at(i).type == 100) { // Text
							// Get contents of the edit field
							GetDlgItemTextA(hDlg, IDC_PARAMTEXT0+n, (LPSTR)text, 256);
							strcpy_s(InfoList.at(i).text, 256, text);
						}
					}
					UpdateParameterList(xs);
					DestroyWindow(hDlg);
					ParamWnd = NULL;
                    break;

				case IDCANCEL :
                    // User pressed cancel.  
					// Take down the dialog box and restore previous parameter values.
					if(InfoList.size() > 0) InfoList.erase (InfoList.begin(), InfoList.end() );
					for(i=0; i<(int)infoBackup.size(); i++) {
						InfoList.push_back(infoBackup[i]);
					}
					UpdateParameterList(xs);
					DestroyWindow(hDlg);
					ParamWnd = NULL;
					return TRUE;

				default:
					return FALSE;
			}
			break;
	}

	return FALSE;
}

//
//	This is critical or the dialog's resources are not found
//
// http://www.codeguru.com/cpp/w-p/dll/tips/article.php/c3635/Tip-Detecting-a-HMODULEHINSTANCE-Handle-Within-the-Module-Youre-Running-In.htm
//
HMODULE GetCurrentModule()
{
	// from ATL 7.0 sources
	return reinterpret_cast<HMODULE>(&__ImageBase);
}

void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}


// Update the parameter ist from the static info list used by the parameter dialog
void UpdateParameterList(t_object *xs) 
{
	// The static info list has been updated, so update the paramlist
	t_jit_gl_freeframe *x;
	x = (t_jit_gl_freeframe *)xs;
	for(int i=0; i<(int)x->ParamInfoList.size(); i++) x->ParamInfoList.at(i) = InfoList.at(i);
}


// Unload plugin and free all resources
void UnloadFree(t_jit_gl_freeframe *x) {

	// Unload the plugin
	// sets plugin initialization and loading flags to false
	if(x->bPluginSelected) UnloadFFGLplugin(x); 

	// Delete all FFGL textures
	if(x->ffglTexture.Handle != 0) glDeleteTextures(1, &x->ffglTexture.Handle);
	x->ffglTexture.Handle = 0;
	if(x->ffglTexture2.Handle != 0) glDeleteTextures(1, &x->ffglTexture2.Handle);
	x->ffglTexture2.Handle = 0;
	if(x->ffglTexture3.Handle != 0) glDeleteTextures(1, &x->ffglTexture3.Handle);
	x->ffglTexture3.Handle = 0;
	// Free the fbo
	if(x->g_fbo1 > 0) glDeleteFramebuffersEXT(1, &x->g_fbo1);
	x->g_fbo1 = 0;
		
	x->bPluginSelected = true; // Means load the plugin again in draw
	x->bInitialized = false; // Means that the plugin has been de-instantiated

}

// For debugging
void GLerror() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("GL error = %d (0x%x) %s\n", err, err, gluErrorString(err));
	}
}	

/*
For debugging
// Pixel format descriptor
typedef struct tagPIXELFORMATDESCRIPTOR
{
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, FAR *LPPIXELFORMATDESCRIPTOR;
*/
int FindPixelType() {

	PIXELFORMATDESCRIPTOR  pfd;
	HDC  hdc;
	int  iPixelFormat;

	
	// hdc=GetDC(hWnd);

	// HGLRC hrc = wglGetCurrentContext();
	hdc = wglGetCurrentDC();
	if (!hdc)
		return -1;

	// get the current pixel format index  
	iPixelFormat = GetPixelFormat(hdc); 
 
	// obtain a detailed description of that pixel format  
	DescribePixelFormat(hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	
	printf("iPixelFormat    = %d\n", iPixelFormat);
    printf("nSize           = %d\n", pfd.nSize);
    printf("nVersion        = %d\n", pfd.nVersion);
    printf("dwFlags         = %d\n", pfd.dwFlags);
	printf("iPixelType      = %d\n", pfd.iPixelType);
	printf("cColorBits      = %d\n", pfd.cColorBits);
	printf("cRedBits        = %d\n", pfd.cRedBits);
	printf("cRedShift       = %d\n", pfd.cRedShift);
	printf("cGreenBits      = %d\n", pfd.cGreenBits);
	printf("cGreenShift     = %d\n", pfd.cGreenShift);
	printf("cBlueBits       = %d\n", pfd.cBlueBits);
	printf("cBlueShift      = %d\n", pfd.cBlueShift);
	printf("cAlphaBits      = %d\n", pfd.cAlphaBits);
	printf("cAlphaShift     = %d\n", pfd.cAlphaShift);
	printf("cAccumBits      = %d\n", pfd.cAccumBits);
	printf("cAccumRedBits   = %d\n", pfd.cAccumRedBits);
	printf("cAccumGreenBits = %d\n", pfd.cAccumGreenBits);
	printf("cAccumBlueBits  = %d\n", pfd.cAccumBlueBits);
	printf("cAccumAlphaBits = %d\n", pfd.cAccumAlphaBits);
	printf("cDepthBits      = %d\n", pfd.cDepthBits);
	printf("cStencilBits    = %d\n", pfd.cStencilBits);
	printf("cAuxBuffers     = %d\n", pfd.cAuxBuffers);
	printf("iLayerType      = %d\n", pfd.iLayerType);
	printf("bReserved       = %d\n", pfd.bReserved);
	printf("dwLayerMask     = %d\n", pfd.dwLayerMask);
	printf("dwVisibleMask   = %d\n", pfd.dwVisibleMask);
	printf("dwDamageMask    = %d\n", pfd.dwDamageMask);
	return(pfd.iPixelType);
}
