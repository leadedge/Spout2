/*

    max.jit.gl.spoutsender.c

	----------------------------------------------------
	09-10-14 - added console post of version at start
	24.07.15 - Revised for Max 7 and 64 bit
	27.07.15 - Used Max 6 main function
			   Version 2.007
	01.08.15 - Recompiled for Spout 2.004 - 32bit VS2010 - Version 2.007.10
	01.08.15 - Recompiled for Spout 2.004 - 64bit VS2012 - Version 2.007.12
			 - Recompiled /MD Spout 2.005 - 64bit VS2012 - Version 2.008.12
			 - Recompiled /MD Spout 2.005 - 64bit VS2012 - Version 2.008.12
	26.03.16 - Recompiled /MD Spout 2.005 - 64bit VS2012 - Version 2.009.12
			 - Recompiled /MD Spout 2.005 - 64bit VS2012 - Version 2.009.12
	01.04.16 - Included detection of 64bit for post of version number
			 - Recompiled /MT Spout 2.005 - 64bit VS2012 - Version 2.010.12
			 - Recompiled /MT Spout 2.005 - 32bit VS2012 - Version 2.010.12
	16.05.16 - Changed Version numbering to allow the Max Package manager
			   to show 2.0.4 -> 2.0.5 for the package, VS2010 option removed.
	02.06.16 - Recompiled /MT Spout 2.005 - 64bit and 32bit VS2012 - Version 2.0.5.10

	----------------------------------------------------
		
	Based on :
		max.jit.gl.syphonclient
		Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).



		- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Copyright (c) 2015-2016 - Lynn Jarvis. All rights reserved.

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

// Check for 64bit compile
#ifdef _WIN64
     #define ENV64BIT
#else
    #define ENV32BIT
#endif

typedef struct _max_jit_gl_spoutsender
{
	t_object		ob;
	void			*obex;
} t_max_jit_gl_spoutsender;

t_jit_err jit_gl_spoutsender_init(void);

void *max_jit_gl_spoutsender_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_gl_spoutsender_free(t_max_jit_gl_spoutsender *x);
t_class *max_jit_gl_spoutsender_class;

// MAX 7
void ext_main(void *r)
{

/*
// MAX 6
int C74_EXPORT main(void)
{	
*/

	t_class *maxclass, *jitclass;

	#ifdef ENV64BIT
	post("jit_gl_spout_sender - Vers 2.0.5.10 (64 bit)");
	#else
	post("jit_gl_spout_sender - Vers 2.0.5.10 (32 bit)");
	#endif


	// initialize our Jitter class
	jit_gl_spoutsender_init();	
	
	// create our Max class
	maxclass = class_new("jit.gl.spoutsender", 
						(method)max_jit_gl_spoutsender_new,
						(method)max_jit_gl_spoutsender_free, 
						sizeof(t_max_jit_gl_spoutsender),
						NULL, A_GIMME, 0);

	// specify a byte offset to keep additional information about our object
	max_jit_class_obex_setup(maxclass, calcoffset(t_max_jit_gl_spoutsender, obex));

	// look up our Jitter class in the class registry
	jitclass = (t_class *)jit_class_findbyname(gensym("jit_gl_spoutsender"));

	// wrap our Jitter class with the standard methods for Jitter objects
	max_jit_class_wrap_standard(maxclass, jitclass, 0);
	
   	// use standard ob3d assist method
	class_addmethod(maxclass, (method)max_jit_ob3d_assist, "assist", A_CANT, 0);

	// add methods for 3d drawing
	max_jit_class_ob3d_wrap(maxclass);

	// register our class with max
	class_register(CLASS_BOX, maxclass);
	max_jit_gl_spoutsender_class = maxclass;

}

void max_jit_gl_spoutsender_free(t_max_jit_gl_spoutsender *x)
{
	// lookup our internal Jitter object instance and free
	jit_object_free(max_jit_obex_jitob_get(x));

	// free resources associated with our obex entry
	max_jit_object_free(x);
}

void *max_jit_gl_spoutsender_new(t_symbol *s, long argc, t_atom *argv)
{
	// Sender
	t_max_jit_gl_spoutsender *x;
	void *jit_ob;
	long attrstart;
	t_symbol *dest_name_sym = _jit_sym_nothing;

	if ((x = (t_max_jit_gl_spoutsender *)max_jit_object_alloc(max_jit_gl_spoutsender_class, gensym("jit_gl_spoutsender"))))
	{
		// get first normal arg, the destination name
		attrstart = max_jit_attr_args_offset((SHORT)argc, argv);
		if (attrstart&&argv) 
		{
			jit_atom_arg_getsym(&dest_name_sym, 0, attrstart, argv);
		}
		
		// instantiate Jitter object with dest_name arg
		if ((jit_ob = jit_object_new(gensym("jit_gl_spoutsender"), dest_name_sym)))
		{
			// set internal jitter object instance
			max_jit_obex_jitob_set(x, jit_ob);
			
			// add a general purpose outlet (rightmost)
			max_jit_obex_dumpout_set(x, outlet_new(x,NULL));
			
			// process attribute arguments 
			max_jit_attr_args(x, (SHORT)argc, argv);
			
			// create new proxy inlet.
			max_jit_obex_proxy_new(x, 0);
		} 
		else {
			error("jit.gl.spout_sender: could not allocate object");
			freeobject((t_object *)x);
			x = NULL;
		}
	}


	return (x);
}


