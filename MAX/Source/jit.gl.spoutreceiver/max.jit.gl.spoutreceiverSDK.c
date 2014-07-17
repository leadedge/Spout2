/*
    max.jit.gl.spoutreceiver.c

	Based on :
		max.jit.gl.syphonclient- Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).

		TODO : change getavailableservers to getavailablesenders

 */
#include "jit.common.h"
#include "jit.gl.h"
#include "ext_obex.h"
#include "../../SpoutSDK/Spout.h"
//
// NVidia Optimus force to high performance graphics
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
// A value of 0x00000001 indicates that rendering should be performed using High Performance Graphics.
// Doesn't seem to work
//
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

typedef struct _max_jit_gl_spout_receiver 
{
	t_object		ob;
	void			*obex;

	// output texture outlet
	void			*texout;
    void            *dumpout;

} t_max_jit_gl_spout_receiver;

t_jit_err jit_gl_spout_receiver_init(void);
void *max_jit_gl_spout_receiver_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_gl_spout_receiver_free(t_max_jit_gl_spout_receiver *x);

// custom draw
void max_jit_gl_spout_receiver_bang(t_max_jit_gl_spout_receiver *x);
void max_jit_gl_spout_receiver_draw(t_max_jit_gl_spout_receiver *x, t_symbol *s, long argc, t_atom *argv);

//custom list outof available Senders via the dumpout outlet.
void max_jit_gl_spout_receiver_getavailableservers(t_max_jit_gl_spout_receiver *x);

t_class *max_jit_gl_spout_receiver_class;

// LJ - changed names -  t_symbol *ps_jit_gl_texture, *ps_draw, *ps_out_name, *ps_appname, *ps_Sendername, *ps_clear;
t_symbol *ps_jit_gl_texture, *ps_maxdraw, *ps_out_name, *ps_spoutsendername, *ps_clear;

void main(void)
{	
	void *classex, *jitclass;
	
	/*
	// LJ debug console window so printf works
	AllocConsole();
	freopen("CONIN$",  "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	printf("jit_gl_spout_receiver\n");
	// =================================
	*/
	
	// initialize our Jitter class
	jit_gl_spout_receiver_init();	
	
	// create our Max class
	setup((t_messlist **)&max_jit_gl_spout_receiver_class, 
		  (method)max_jit_gl_spout_receiver_new, 
		  (method)max_jit_gl_spout_receiver_free, 
		  (short)sizeof(t_max_jit_gl_spout_receiver), 
		  0L, A_GIMME, 0);
	
	// specify a byte offset to keep additional information about our object
	classex = max_jit_classex_setup(calcoffset(t_max_jit_gl_spout_receiver, obex));
	
	// look up our Jitter class in the class registry
	jitclass = jit_class_findbyname(gensym("jit_gl_spout_receiver"));	
	
	// wrap our Jitter class with the standard methods for Jitter objects
    max_jit_classex_standard_wrap(classex, jitclass, 0); 	
	
	// custom draw handler so we can output our texture.
	// override default ob3d bang/draw methods
	addbang((method)max_jit_gl_spout_receiver_bang);
	max_addmethod_defer_low((method)max_jit_gl_spout_receiver_draw, "draw");  
	
    max_addmethod_defer_low((method)max_jit_gl_spout_receiver_getavailableservers, "getavailableservers");
    
   	// use standard ob3d assist method
    addmess((method)max_jit_ob3d_assist, "assist", A_CANT,0);  
	
	// add methods for 3d drawing
    max_ob3d_setup();

	ps_jit_gl_texture = gensym("jit_gl_texture");
	ps_maxdraw = gensym("maxdraw");
	ps_out_name = gensym("out_name");
	ps_spoutsendername = gensym("SpoutSender");
    ps_clear = gensym("clear");

}

void max_jit_gl_spout_receiver_free(t_max_jit_gl_spout_receiver *x)
{

	max_jit_ob3d_detach(x);

	// lookup our internal Jitter object instance and free
	jit_object_free(max_jit_obex_jitob_get(x));
	
	// free resources associated with our obex entry
	max_jit_obex_free(x);
}

void max_jit_gl_spout_receiver_bang(t_max_jit_gl_spout_receiver *x)
{
	// post("max_jit_gl_spout_receiver_bang");
	// typedmess((t_object *)x, ps_draw, 0, NULL);
	max_jit_gl_spout_receiver_draw(x, ps_maxdraw, 0, NULL);

}

void max_jit_gl_spout_receiver_draw(t_max_jit_gl_spout_receiver *x, t_symbol *s, long argc, t_atom *argv)
{

	t_atom a;

	// post("max_jit_gl_spout_receiver_draw");

	// get the jitter object
	t_jit_object *jitob = (t_jit_object*)max_jit_obex_jitob_get(x);
	
	// call the jitter object's draw method
	jit_object_method(jitob, s, s, argc, argv);
	
	// query the texture name and send out the texture output 
	jit_atom_setsym(&a,jit_attr_getsym(jitob, ps_out_name));

	outlet_anything(x->texout, ps_jit_gl_texture, 1, &a);


}

void max_jit_gl_spout_receiver_getavailableservers(t_max_jit_gl_spout_receiver *x)
{
	int nSenders;
	t_atom atomName; // to send out
	string namestring; // sender name string in the list of names
	char sendername[256]; // array to clip a passed name if > 256 bytes

	// post("max_jit_gl_spout_receiver_getavailableservers");
	
	SpoutReceiver * myReceiver;
	myReceiver = new SpoutReceiver;

	nSenders = myReceiver->GetSenderCount();
	if(nSenders > 0) {
	    // send a clear first.
		outlet_anything(max_jit_obex_dumpout_get(x), ps_clear, 0, 0); 
		for(int i=0; i<nSenders; i++) {
			myReceiver->GetSenderName(i, sendername);
			atom_setsym(&atomName, gensym((char*)sendername));
			outlet_anything(x->dumpout, ps_spoutsendername, 1, &atomName); 
		}
	}

	delete myReceiver;
	myReceiver = NULL;

	/*
	Syphon code

	t_atom atomName;
    t_atom atomHostName;
    
    // send a clear first.
    outlet_anything(max_jit_obex_dumpout_get(x), ps_clear, 0, 0); 

    for(NSDictionary* SenderDict in [[SyphonSenderDirectory sharedDirectory] Senders])
    {
        NSString* SenderName = [SenderDict valueForKey:SyphonSenderDescriptionNameKey];
        NSString* SenderAppName = [SenderDict valueForKey:SyphonSenderDescriptionAppNameKey];
        
        const char* name = [SenderName cStringUsingEncoding:NSUTF8StringEncoding];
        const char* hostName = [SenderAppName cStringUsingEncoding:NSUTF8StringEncoding];
                
        atom_setsym(&atomName, gensym((char*)name));
        atom_setsym(&atomHostName, gensym((char*)hostName));

        outlet_anything(x->dumpout, ps_Sendername, 1, &atomName); 
        outlet_anything(x->dumpout, ps_appname, 1, &atomHostName); 
    }  
	*/
}

void *max_jit_gl_spout_receiver_new(t_symbol *s, long argc, t_atom *argv)
{
	t_max_jit_gl_spout_receiver *x;
	void *jit_ob;
	long attrstart;
	t_symbol *dest_name_sym = _jit_sym_nothing;
	
	if ((x = (t_max_jit_gl_spout_receiver *) max_jit_obex_new(max_jit_gl_spout_receiver_class, gensym("jit_gl_spout_receiver")))) 
	{
		// get first normal arg, the destination name
		attrstart = max_jit_attr_args_offset(argc,argv);
		if (attrstart&&argv) {
			jit_atom_arg_getsym(&dest_name_sym, 0, attrstart, argv);

		}
		
		// instantiate Jitter object with dest_name arg
		if ((jit_ob = jit_object_new(gensym("jit_gl_spout_receiver"), dest_name_sym))) {

			// set internal jitter object instance
			max_jit_obex_jitob_set(x, jit_ob);
			
			// process attribute arguments 
			max_jit_attr_args(x, argc, argv);		

			// add a general purpose outlet (rightmost)
			x->dumpout = outlet_new(x,NULL);
			max_jit_obex_dumpout_set(x, x->dumpout);

			// Syphon comment !!! - this outlet is used to shit out textures! yay!
			x->texout = outlet_new(x, "jit_gl_texture");
			
			// LJ DEBUG - create new proxy inlet for update bang.
			// max_jit_obex_proxy_new(x, 0);


		} 
		else {
			error("jit_gl_spout_receiver : could not allocate object");
			freeobject((t_object *)x);
			x = NULL;
		}
	}
	return (x);
}


