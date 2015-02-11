/*

    max.jit.gl.freeframe.c
		
	Based on : max.jit.gl.receiver

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
#include <string>
#include <vector>
#include <Shlobj.h> // to get the program folder path

#include "jit.common.h"
#include "jit.gl.h"
#include "ext_obex.h"

#include "SpoutGLextensions.h"

using namespace std;

typedef struct _max_jit_gl_freeframe 
{
	t_object		ob;
	void			*obex;
	
	// outlet
	void			*texout; // texture output
    void            *dumpout; // information out

	// inlet
	long m_in; // space for the inlet number used by all the proxies
	void *m_proxy;

} t_max_jit_gl_freeframe;


struct FFGLParamInfo {
	float value; // float value (default)
	DWORD type; // 0-checkbox, 1-button, 100-text, 10 and default - float
	bool state; // checkbox or button state
	char text[256]; // text input
	char name[16]; // Parameter name
	char display[16]; // alternate parameter display value (other than actual value)
};


t_jit_err jit_gl_freeframe_init(void);
void *max_jit_gl_freeframe_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_gl_freeframe_free(t_max_jit_gl_freeframe *x);

// custom draw
void max_jit_gl_freeframe_bang(t_max_jit_gl_freeframe *x);
void max_jit_gl_freeframe_draw(t_max_jit_gl_freeframe *x, t_symbol *s, long argc, t_atom *argv);
void max_jit_gl_freeframe_geteffectlist(t_max_jit_gl_freeframe *x);
void max_jit_gl_freeframe_getnumparams(t_max_jit_gl_freeframe *x);
void max_jit_gl_freeframe_getparamlist(t_max_jit_gl_freeframe *x);
void max_jit_gl_freeframe_getparameter(t_max_jit_gl_freeframe *x, t_symbol *s, long argc, t_atom *argv);

// Utility
void ToLowerCase(char *str);

t_class *max_jit_gl_freeframe_class;

t_symbol *ps_jit_gl_texture, *ps_out_name, *ps_maxdraw, *ps_clear;
t_symbol *ps_parameter, *ps_numparams;
t_symbol *ps_effectlist, *ps_out_effect;
t_symbol *ps_paramlist, *ps_out_param, *ps_out_numparams;

void main(void)
{	
	void *classex, *jitclass;
	
	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("jit_gl_freeframe - Vers 1.006\n");
	*/

	post("jit_gl_freeframe - Vers 1.006");

	// initialize our Jitter class
	jit_gl_freeframe_init();	
	
	// create our Max class
	setup((t_messlist **)&max_jit_gl_freeframe_class, 
		  (method)max_jit_gl_freeframe_new, 
		  (method)max_jit_gl_freeframe_free, 
		  (short)sizeof(t_max_jit_gl_freeframe), 
		  0L, A_GIMME, 0);
	
	// specify a byte offset to keep additional information about our object
	classex = max_jit_classex_setup(calcoffset(t_max_jit_gl_freeframe, obex));
	
	// look up our Jitter class in the class registry
	jitclass = jit_class_findbyname(gensym("jit_gl_freeframe"));	
	
	// wrap our Jitter class with the standard methods for Jitter objects
    max_jit_classex_standard_wrap(classex, jitclass, 0); 

	// custom draw handler so we can output our texture.
	// override default ob3d bang/draw methods
	addbang((method)max_jit_gl_freeframe_bang);
	max_addmethod_defer_low((method)max_jit_gl_freeframe_draw, "draw");  

	// To activate attribute output functions in jitter
	max_addmethod_defer_low((method)max_jit_gl_freeframe_geteffectlist, "geteffectlist");
	max_addmethod_defer_low((method)max_jit_gl_freeframe_getparamlist,  "getparamlist");
	max_addmethod_defer_low((method)max_jit_gl_freeframe_getparameter,  "getparameter");
	max_addmethod_defer_low((method)max_jit_gl_freeframe_getnumparams,  "getnumparams");

   	// use standard ob3d assist method
    addmess((method)max_jit_ob3d_assist, "assist", A_CANT,0);  
	
	// add methods for 3d drawing
    max_ob3d_setup();

	ps_jit_gl_texture = gensym("jit_gl_texture");
	ps_out_name       = gensym("out_name");
	ps_numparams      = gensym("numparams");
	ps_parameter      = gensym("param");
	ps_paramlist      = gensym("paramlist");
	ps_out_param      = gensym("out_param");
	ps_out_numparams  = gensym("out_numparams");
	ps_effectlist     = gensym("effectlist");
	ps_out_effect     = gensym("out_effect");
	ps_maxdraw        = gensym("maxdraw");
    ps_clear          = gensym("clear");

}

void max_jit_gl_freeframe_free(t_max_jit_gl_freeframe *x)
{

	max_jit_ob3d_detach(x);

	// lookup our internal Jitter object instance and free
	jit_object_free(max_jit_obex_jitob_get(x));
	
	// free resources associated with our obex entry
	max_jit_obex_free(x);
}



void max_jit_gl_freeframe_bang(t_max_jit_gl_freeframe *x)
{
	/*
	// The proxy inlet needs a bang to before this will work
	switch (proxy_getinlet((t_object *)x)) {
           case 0:
              post("bang received in left inlet");
              break;
          case 1:
              post("bang received in right inlet");
              break;
    }
	*/
	max_jit_gl_freeframe_draw(x, ps_maxdraw, 0, NULL);
}


void max_jit_gl_freeframe_draw(t_max_jit_gl_freeframe *x, t_symbol *s, long argc, t_atom *argv)
{

	t_atom a;

	// get the jitter object
	t_jit_object *jitob = (t_jit_object*)max_jit_obex_jitob_get(x);
	
	// call the jitter object's draw method
	jit_object_method(jitob, s, s, argc, argv);
	
	// query the texture name and send out the texture output 
	jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_name));

	outlet_anything(x->texout, ps_jit_gl_texture, 1, &a);

}


// Get the number of parameters for a plugin
void max_jit_gl_freeframe_getnumparams(t_max_jit_gl_freeframe *x)
{
	t_atom a;
	t_atom atomNum; // number to send out
	vector<FFGLParamInfo> * ParamInfoList;
	DWORD pParamList; // pointer retrieved from jitter
	t_jit_object *jitob;

	// printf("max_jit_gl_freeframe_getnumparams\n");

	// get the jitter object
	jitob = (t_jit_object*)max_jit_obex_jitob_get(x);

	// query the parameter list output attribute function in jitter
	jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_param));
	
	// retrieve the paramlist address from jitter
	pParamList = (DWORD)atom_getsym(&a)->s_thing; 

	// Convert the address to a ParamInfoList vector pointer
	ParamInfoList = (vector<FFGLParamInfo> *)pParamList;

	// All we want to send out is the size
	atom_setlong(&atomNum, (long)ParamInfoList->size());
	outlet_anything(x->dumpout, ps_numparams, 1, &atomNum);

}


// Get the parameters for a plugin
void max_jit_gl_freeframe_getparameter(t_max_jit_gl_freeframe *x, t_symbol *s, long argc, t_atom *argv)
{
	t_symbol *name;
	char paramname[MAX_PATH];
	char paramstring[MAX_PATH];
	char display[256];
	char text[256];
	int i, index;
	float fValue;
	t_atom a;
	t_atom atomList; // parameter list to send out
	char list[256]; // list string
	vector<FFGLParamInfo> * ParamInfoList;
	FFGLParamInfo ParamInfo;
	DWORD pParamList; // pointer retrieved from jitter
	t_jit_object *jitob;

	if(argc > 0) {
		
		// get the jitter object
		jitob = (t_jit_object*)max_jit_obex_jitob_get(x);

		// query the parameter list output attribute function in jitter
		jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_param));
	
		// retrieve the paramlist address from jitter
		pParamList = (DWORD)atom_getsym(&a)->s_thing; 

		// Convert the address to a ParamInfoList vector pointer
		ParamInfoList = (vector<FFGLParamInfo> *)pParamList;

		if(ParamInfoList->size() > 0) {

			// The first arg will be a name or a number
			name = jit_atom_getsym(&argv[0]);
			strcpy_s(paramname, MAX_PATH, name->s_name);
			index = jit_atom_getlong(&argv[0]);

			// If a parameter name is supplied, look for this name in the paramstruct for this plugin
			if(strlen(paramname) > 0) {
				index = -1;
				ToLowerCase(paramname);
				for(i=0; i <(int)ParamInfoList->size(); i++) {
					strcpy_s(paramstring, MAX_PATH, ParamInfoList->at(i).name);
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
			if(index >= 0 && index <(int)ParamInfoList->size()) {
				// Get the type of the parameter to format it correctly
				// Send out the parameter details
				// The list is formatted as "param param param-val param-display".
				// The param argument is the name of the parameters, the param-val the value,
				// and param-display is an alternate display value, for parameters whose "real value",
				// as understood by the parameter, lies outside of the range 0-1. 
				if(!ParamInfoList->at(index).display[0])
					sprintf_s(display, 256, "null");
				else
					strcpy_s(display, 256, ParamInfoList->at(index).display);

				if(!ParamInfoList->at(index).text[0])
					sprintf_s(text, 256, "null");
				else
					strcpy_s(text, 256, ParamInfoList->at(index).display);

				switch (ParamInfoList->at(index).type) {
					case 0 :
					case 1 :
						sprintf_s(list, 256, "%d %d %d \"%s\" \"%s\"", index, ParamInfoList->at(index).type, display, ParamInfoList->at(index).name);
						break;
					case 100 :
						sprintf_s(list, 256, "%d %d \"%s\" \"%s\" \"%s\"", index, ParamInfoList->at(index).type, text, display, ParamInfoList->at(index).name);
						break;
					default :
						sprintf_s(list, 256, "%d %d %f \"%s\" \"%s\"", index, ParamInfoList->at(index).type, ParamInfoList->at(index).value, display, ParamInfoList->at(index).name);
						break;
				}

				atom_setsym(&atomList, gensym(list));
				outlet_anything(x->dumpout, ps_parameter, 1, &atomList);
			} // endif valid index

		} // endif there are parameters for this plugin
	} // end args passed
}


// Get the parameter list for a plugin

//
// parameters done 0
// parameters numparams 5
// parameters param 0 1000 0.5 "My Float Slider"
// parameters param 1 1000 0.625 "My Float Slider 2"
// parameters param 2 1 0 "My Button"
// parameters param 3 100 "default string" "My String"
// parameters param 4 1000 3.0 "Num Slices"
// parameters done 1

void max_jit_gl_freeframe_getparamlist(t_max_jit_gl_freeframe *x)
{
	t_atom a;
	t_atom atomList; // parameter list to send out
	char list[256]; // list string
	char display[256];
	char text[256];
	vector<FFGLParamInfo> * ParamInfoList;
	DWORD pParamList; // pointer retrieved from jitter

	// get the jitter object
	t_jit_object *jitob = (t_jit_object*)max_jit_obex_jitob_get(x);

	// query the parameter list output attribute function in jitter
	jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_param));
	
	// retrieve the paramlist address from jitter
	pParamList = (DWORD)atom_getsym(&a)->s_thing; 

	// Convert the address to a ParamInfoList vector pointer
	ParamInfoList = (vector<FFGLParamInfo> *)pParamList;

	// Output : paramlist param-name param-type
	sprintf_s(list, 256, "done 0");
	atom_setsym(&atomList, gensym(list));
	outlet_anything(x->dumpout, ps_paramlist, 1, &atomList);

	sprintf_s(list, 256, "numparams %d", ParamInfoList->size());
	atom_setsym(&atomList, gensym(list));
	outlet_anything(x->dumpout, ps_paramlist, 1, &atomList);

	if(ParamInfoList->size() > 0) {

		for(int i=0; i<(int)ParamInfoList->size(); i++) {

			if(!ParamInfoList->at(i).display[0])
				sprintf_s(display, 256, "null");
			else
				strcpy_s(display, 256, ParamInfoList->at(i).display);
			
			if(!ParamInfoList->at(i).text[0])
				sprintf_s(text, 256, "null");
			else
				strcpy_s(text, 256, ParamInfoList->at(i).display);

			switch (ParamInfoList->at(i).type) {
				case 0 :
				case 1 :
					sprintf_s(list, 256, "param %d %d %d \"%s\" \"%s\"", i, ParamInfoList->at(i).type, ParamInfoList->at(i).state, display, ParamInfoList->at(i).name);
					break;
				case 100 :
					sprintf_s(list, 256, "param %d %d \"%s\" \"%s\" \"%s\"", i, ParamInfoList->at(i).type, text, display, ParamInfoList->at(i).name);
					break;
				default :
					sprintf_s(list, 256, "param %d %d %f \"%s\" \"%s\"", i, ParamInfoList->at(i).type, ParamInfoList->at(i).value, display, ParamInfoList->at(i).name);
					break;
			}
			atom_setsym(&atomList, gensym(list));
			outlet_anything(x->dumpout, ps_paramlist, 1, &atomList);
		}
	}
	
	sprintf_s(list, 256, "done 1");
	atom_setsym(&atomList, gensym(list));
	outlet_anything(x->dumpout, ps_paramlist, 1, &atomList);

}


void max_jit_gl_freeframe_geteffectlist(t_max_jit_gl_freeframe *x)
{
	int nSenders;
	t_atom a;
	t_atom atomName; // to send out
	vector<string> * EffectList;
	DWORD pEffectList; // pointer retrieved from jitter
	char PluginPath[MAX_PATH]; // file path saved in the effectlist vector
	char effectname[MAX_PATH]; // file name for output

	// get the jitter object
	t_jit_object *jitob = (t_jit_object*)max_jit_obex_jitob_get(x);

	// query the parameter list output attribute function in jitter
	jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_effect));

	// retrieve the effectlist address from jitter
	pEffectList = (DWORD)atom_getsym(&a)->s_thing; 

	// Convert the address to a EffectList vector pointer
	EffectList = (vector<string> *)pEffectList;

	// Output : effectlist effect-name
	if(EffectList->size() > 0) {
		for(int i=0; i<(int)EffectList->size(); i++) {
			sprintf_s(PluginPath, MAX_PATH, "%s", EffectList->at(i).c_str());
			_splitpath_s(PluginPath, NULL, NULL, NULL, NULL, effectname, MAX_PATH, NULL, 0);
			atom_setsym(&atomName, gensym(effectname));
			outlet_anything(x->dumpout, ps_effectlist, 1, &atomName);
		}
		atom_setsym(&atomName, gensym("end"));
		outlet_anything(x->dumpout, ps_effectlist, 1, &atomName);
	}

}


void *max_jit_gl_freeframe_new(t_symbol *s, long argc, t_atom *argv)
{
	t_max_jit_gl_freeframe *x;
	void *jit_ob;
	long attrstart;
	t_symbol *dest_name_sym = _jit_sym_nothing;
	
	if ((x = (t_max_jit_gl_freeframe *) max_jit_obex_new(max_jit_gl_freeframe_class, gensym("jit_gl_freeframe") ))) {
		
		// get first normal arg, the destination name
		attrstart = max_jit_attr_args_offset(argc,argv);
		if (attrstart && argv) {
			jit_atom_arg_getsym(&dest_name_sym, 0, attrstart, argv);
		}
		
		// instantiate Jitter object with dest_name arg
		if ((jit_ob = jit_object_new(gensym("jit_gl_freeframe"), dest_name_sym))) {

			// set internal jitter object instance
			max_jit_obex_jitob_set(x, jit_ob);
			
			// process attribute arguments 
			max_jit_attr_args(x, argc, argv);

			// add a general purpose outlet (rightmost)
			x->dumpout = outlet_new(x,NULL);
			max_jit_obex_dumpout_set(x, x->dumpout);

			// Texture outlet (leftmost)
			x->texout = outlet_new(x, "jit_gl_texture");

			// Add a second texture inlet

			// Notes from SDK docs
			//
			// When it is necessary to have multiple input matrices, this is typically
			// managed by either declaring alternately named methods for each input, 
			// or exposing an attribute that specifies which input the jit_matrix method 
			// assumes it is being called with. Note that this requires additional
			// logic within the Max wrapper class to map to inlets, as it is not handled automatically.
			
			// Messages are received equally from the LH inlet or the RH proxy inlet
			// The RH inlet is there just for convenience and clarity
			x->m_proxy = proxy_new((t_object *)x, 1, &x->m_in);
			
		} 
		else {
			error("jit_gl_freeframe : could not allocate object");
			freeobject((t_object *)x);
			x = NULL;
		}
	}
	return (x);
}
