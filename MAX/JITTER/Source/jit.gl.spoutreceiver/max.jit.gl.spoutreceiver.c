/*

    max.jit.gl.spoutreceiver.c

	----------------------------------------------------
	09-10-14 - Added console post of version at start
	24.07.15 - Revised for Max 7 and 64 bit
	25.07.15 - Added first sender detection on startup
	27.07.15 - Used Max 6 main function
			 - Version 2.007
	01.08.15 - Recompiled for Spout 2.004 - 32 bit VS2010 - Version 2.007.10
	01.08.15 - Recompiled for Spout 2.004 - 64bit VS2012 - Version 2.007.12
	29.10.15 - Removed all Max 6 references
			 - Debug frame number output
	01.04.16 - Included detection of 64bit for post of version number
			 - Recompiled /MT Spout 2.005 - 64bit VS2012 - Version 2.009.12
			 - Recompiled /MT Spout 2.005 - 32bit VS2012 - Version 2.009.12
	16.05.16 - Changed Version numbering to allow the Max Package manager
			   to show 2.0.4 -> 2.0.5 for the package, VS2010 option removed.
	20.06.16 - Removed frame numbering testing
			 - Recompiled /MT Spout 2.005 - 64bit and 32bit VS2012 - Version 2.0.5.10
	----------------------------------------------------
		
	Based on :
		max.jit.gl.syphonclient
		Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).


		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Copyright (c) 2015, Lynn Jarvis. All rights reserved.

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
#include "ext_obex.h"
#include "../../SpoutSDK/Spout.h"

// Check for 64bit compile
#ifdef _WIN64
     #define ENV64BIT
#else
    #define ENV32BIT
#endif

typedef struct _max_jit_gl_spout_receiver 
{
	t_object		ob;
	void			*obex;

	// output texture outlet
	void			*texout;
    void            *dumpout;
	bool			b_Started;

} t_max_jit_gl_spout_receiver;

t_jit_err jit_gl_spout_receiver_init(void);
void *max_jit_gl_spout_receiver_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_gl_spout_receiver_free(t_max_jit_gl_spout_receiver *x);

// custom draw
void max_jit_gl_spout_receiver_bang(t_max_jit_gl_spout_receiver *x); // TODO - necessary ?
void max_jit_gl_spout_receiver_draw(t_max_jit_gl_spout_receiver *x, t_symbol *s, long argc, t_atom *argv);

//custom list outof available Senders via the dumpout outlet.
void max_jit_gl_spout_receiver_getavailablesenders(t_max_jit_gl_spout_receiver *x);


t_class *max_jit_gl_spout_receiver_class;

t_symbol *ps_out_texture, *ps_maxdraw, *ps_out_name, *ps_spoutsendername, *ps_clear;

// MAX 7
void ext_main(void *r)
{
	UNREFERENCED_PARAMETER(r);

	t_class *maxclass, *jitclass;

	/*
	// Debug console window so printf works
	FILE* pCout; // should really be freed on exit 
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout); 
	printf("jit_gl_spout_receiver - Vers 2.0.5.10\n");
	*/

	// Show the version for reference
	#ifdef ENV64BIT
	post("jit_gl_spout_receiver - Vers 2.0.5.10 (64 bit)");
	#else
	post("jit_gl_spout_receiver - Vers 2.0.5.10 (32 bit)");
	#endif

	// initialize our Jitter class
	jit_gl_spout_receiver_init();	
	
	// create our Max class
	maxclass = class_new("jit.gl.spoutreceiver", 
						(method)max_jit_gl_spout_receiver_new,
						(method)max_jit_gl_spout_receiver_free, 
						sizeof(t_max_jit_gl_spout_receiver),
						NULL, A_GIMME, 0);

	// specify a byte offset to keep additional information about our object
	max_jit_class_obex_setup(maxclass, calcoffset(t_max_jit_gl_spout_receiver, obex));

	// look up our Jitter class in the class registry
	jitclass = (t_class *)jit_class_findbyname(gensym("jit_gl_spout_receiver"));	
	
	// wrap our Jitter class with the standard methods for Jitter objects
    max_jit_class_wrap_standard(maxclass, jitclass, 0);
	
	// custom draw handler so we can output our texture.
	// override default ob3d bang/draw methods
	class_addmethod(maxclass, (method)max_jit_gl_spout_receiver_bang, "bang", 0); // TODO - necessary ?
	class_addmethod(maxclass, (method)max_jit_gl_spout_receiver_draw, "draw", 0);
	class_addmethod(maxclass, (method)max_jit_gl_spout_receiver_getavailablesenders, "getavailablesenders", 0);

   	// use standard ob3d assist method
    class_addmethod(maxclass, (method)max_jit_ob3d_assist, "assist", A_CANT, 0); 

	
	// add methods for 3d drawing
	max_jit_class_ob3d_wrap(maxclass);

	ps_out_texture = gensym("jit_gl_texture");
	ps_maxdraw = gensym("maxdraw");
	ps_out_name = gensym("out_name");
	ps_spoutsendername = gensym("SpoutSender");
    ps_clear = gensym("clear");

	// register our class with max
	class_register(CLASS_BOX, maxclass);
	max_jit_gl_spout_receiver_class = maxclass;

}


void max_jit_gl_spout_receiver_free(t_max_jit_gl_spout_receiver *x)
{

	// lookup our internal Jitter object instance and free
	jit_object_free(max_jit_obex_jitob_get(x));
	
	// free resources associated with our obex entry
	max_jit_object_free(x);
}



// TODO - necessary ?
void max_jit_gl_spout_receiver_bang(t_max_jit_gl_spout_receiver *x)
{
	UNREFERENCED_PARAMETER(x);

	// printf("max_jit_gl_spout_receiver_bang\n");
	// max_jit_gl_spout_receiver_draw(x, ps_maxdraw, 0, NULL);


}

void max_jit_gl_spout_receiver_draw(t_max_jit_gl_spout_receiver *x, t_symbol *s, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(s);
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	t_atom a;

	// get the jitter object
	t_jit_object *jitob = (t_jit_object*)max_jit_obex_jitob_get(x);
	
	// Call the jitter object's draw method (from Syphon code)

	// LJ - This causes an error with corrupted texture received in draw
	// but does not affect the output. Seem to be not needed - needs tracing
	// t_symbol *attr = gensym("draw");
	// jit_object_method(jitob, attr, s, argc, argv);
	
	// query the texture name and send out the texture output 
	jit_atom_setsym(&a, jit_attr_getsym(jitob, ps_out_name));
	outlet_anything(x->texout, ps_out_texture, 1, &a);


}


void *max_jit_gl_spout_receiver_new(t_symbol *s, long argc, t_atom *argv)
{
	UNREFERENCED_PARAMETER(s);

	t_max_jit_gl_spout_receiver *x;
	void *jit_ob;
	long attrstart;
	t_symbol *dest_name_sym = _jit_sym_nothing;

	if ((x = (t_max_jit_gl_spout_receiver *)max_jit_object_alloc(max_jit_gl_spout_receiver_class, gensym("jit_gl_spout_receiver"))))
	{
		// get first normal arg, the destination name
		attrstart = max_jit_attr_args_offset((short)argc,argv);
		if (attrstart&&argv) {
			jit_atom_arg_getsym(&dest_name_sym, 0, attrstart, argv);

		}
		
		// instantiate Jitter object with dest_name arg
		if ((jit_ob = jit_object_new(gensym("jit_gl_spout_receiver"), dest_name_sym))) {

			// set internal jitter object instance
			max_jit_obex_jitob_set(x, jit_ob);
			
			// process attribute arguments 
			max_jit_attr_args(x, (short)argc, argv);		

			// add a general purpose outlet (rightmost)
			x->dumpout = outlet_new(x,NULL);
			max_jit_obex_dumpout_set(x, x->dumpout);

			// Texture outlet
			x->texout = outlet_new(x, "jit_gl_texture");

			// For first sender detection
			x->b_Started = false;

		} 
		else {
			error("jit_gl_spout_receiver : could not allocate object");
			freeobject((t_object *)x);
			x = NULL;
		}
	}
	return (x);
}


void max_jit_gl_spout_receiver_getavailablesenders(t_max_jit_gl_spout_receiver *x)
{
	int nSenders;
	t_atom atomName; // to send out
	string namestring; // sender name string in the list of names
	char sendername[256]; // array to clip a passed name if > 256 bytes

	SpoutReceiver * myReceiver;
	myReceiver = new SpoutReceiver;

	// post("max_jit_gl_spout_receiver_getavailablesenders");
	// printf("max_jit_gl_spout_receiver_getavailablesenders\n");

	nSenders = myReceiver->GetSenderCount();
	if(nSenders > 0) {
	    // send a clear first.
		outlet_anything(max_jit_obex_dumpout_get(x), ps_clear, 0, 0); 
		for(int i=0; i<nSenders; i++) {
			myReceiver->GetSenderName(i, sendername);
			// post("    %s", sendername);
			// printf("    %s\n", sendername);
			atom_setsym(&atomName, gensym((char*)sendername));
			outlet_anything(x->dumpout, ps_spoutsendername, 1, &atomName); 
		}
	}

	delete myReceiver;
	myReceiver = NULL;

}


