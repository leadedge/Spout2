//
//			jit.gl.windowpos.c
//
//	Simple app to control the visual property of the render window
//
//	12.01.14	-	Version 1.00
//	16.03.14	-	Added fullscreen - Version 1.01
//	28.03.14	-	Added fullscreen menubar - Version 1.02
//	30.03.14	-	Change fullscreen menubar to show/hide the taskbar
//					Moved Hide/Show to a separate attribute "hide"
//	30.03.14	-	Changed taskbar to be topmost for fsmenubar to avoid repaint problems
//					No action on mode or hide if fullscreen - Version 1.03
//	16.04.14	-	Changed fullscreen to acount for caption and edges - workaround for openGl Viewport fitted to client rect
//					Allowed 1 pixel in Y in case the taskbar is set to autohide
//	21.04.14	-	Changed fullscreen to acount for caption and edges - workaround for openGl Viewport fitted to client rect
//					Vers 1.04
//	21.04.14	-	Fixed bug where topmost was lost after fullscreen
//					Vers 1.05
//
//
//		mode - Mode
//			0 - top
//			1 - bottom
//
//		fullscreen - Fullscreen
//			0 - normal
//			1 - fullscreen
//
//		fsmenubar - fullscreen taskbar
//			0 - no taskbar fullscreen
//			1 - taskbar fullscreen
//
//		hide
//			0 - show
//			1 - hide

#include "jit.common.h"
#include "jit.gl.h"

typedef struct _jit_gl_windowpos 
{
	t_object	ob;					// Max object
	void		*ob3d;				// 3d object extension
	long		mode;				// topmost mode (0 or 1)
	long		fullscreen;			// fullscreen (0 normal or 1 fullscreen)
	long		fsmenubar;			// menubar fullscreen (0 normal or 1 menubar fullscreen)
	long		hide;				// hide render window (0 normal or 1 hide)
	HWND		window_hwnd;		// window handle
	char		window_name[256];	// global window name based on the context name

	// For full screen mode
	DWORD		dwStyle;
	RECT		windowRect;
	RECT		clientRect;
	RECT		rectWorkArea;

} t_jit_gl_windowpos;

void *_jit_gl_windowpos_class;

t_jit_err jit_gl_windowpos_init(void);
t_jit_gl_windowpos *jit_gl_windowpos_new(t_symbol * dest_name);
void jit_gl_windowpos_free(t_jit_gl_windowpos *x);
t_jit_err jit_gl_windowpos_draw(t_jit_gl_windowpos *x);
t_jit_err jit_gl_windowpos_dest_closing(t_jit_gl_windowpos *x);
t_jit_err jit_gl_windowpos_dest_changed(t_jit_gl_windowpos *x);

// @mode 0/1/2/3 - Topmost / Normal
t_jit_err jit_gl_windowpos_mode(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv); 

// @fullscreen 0/1 - Normal / Fullscreen
t_jit_err jit_gl_windowpos_fullscreen(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv); 

// @fsmenubar 0/1 - Normal / Menubar fullscreen
t_jit_err jit_gl_windowpos_fsmenubar(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv); 

// @hide 0/1 - Show / Hide window
t_jit_err jit_gl_windowpos_hide(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv); 

// LJ DEBUG
t_jit_err jit_ob3d_dest_name_set(t_jit_object *x, void *attr, long argc, t_atom *argv);


// --------------------------------------------------------------------------------

t_jit_err jit_gl_windowpos_init(void) 
{
	long attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;
	t_jit_object *attr;
	long ob3d_flags = JIT_OB3D_NO_MATRIXOUTPUT; // no matrix output
	void *ob3d;
	
	_jit_gl_windowpos_class = jit_class_new("jit_gl_windowpos", 
		(method)jit_gl_windowpos_new, (method)jit_gl_windowpos_free,
		sizeof(t_jit_gl_windowpos),A_DEFSYM,0L);
	
	// set up object extension for 3d object, customized with flags
	ob3d = jit_ob3d_setup(_jit_gl_windowpos_class, 
				calcoffset(t_jit_gl_windowpos, ob3d), 
				ob3d_flags);
	
	// define our OB3D draw method.  called in automatic mode by 
	// jit.gl.render or otherwise through ob3d when banged. this 
	// method is A_CANT because our draw setup needs to happen 
	// in the ob3d beforehand to initialize OpenGL state 
	jit_class_addmethod(_jit_gl_windowpos_class, 
		(method)jit_gl_windowpos_draw, "ob3d_draw", A_CANT, 0L);
	
	// define our dest_closing and dest_changed methods. 
	// In this object, these functions do nothing, and could be omitted.
	jit_class_addmethod(_jit_gl_windowpos_class, (method)jit_gl_windowpos_dest_closing, "dest_closing", A_CANT, 0L);
	jit_class_addmethod(_jit_gl_windowpos_class, (method)jit_gl_windowpos_dest_changed, "dest_changed", A_CANT, 0L);


	// add attribute(s)

	// Mode
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, 
										 "mode", 
										 _jit_sym_long, 
										 attrflags, 
										 (method)NULL, (method)jit_gl_windowpos_mode, 
										 calcoffset(t_jit_gl_windowpos, mode));
	jit_attr_addfilterset_clip(attr, 0, 3, TRUE, TRUE);	// Must be 0 to 3
	jit_class_addattr(_jit_gl_windowpos_class, attr);

	// Fullscreen
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, 
										 "fullscreen", 
										 _jit_sym_long, 
										 attrflags, 
										 (method)NULL, (method)jit_gl_windowpos_fullscreen, 
										 calcoffset(t_jit_gl_windowpos, fullscreen));
	jit_attr_addfilterset_clip(attr, 0, 1, TRUE, TRUE);	// Must be 0 to 1
	jit_class_addattr(_jit_gl_windowpos_class, attr);

	// Fullscreen menubar
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, 
										 "fsmenubar", 
										 _jit_sym_long, 
										 attrflags, 
										 (method)NULL, (method)jit_gl_windowpos_fsmenubar, 
										 calcoffset(t_jit_gl_windowpos, fsmenubar));
	jit_attr_addfilterset_clip(attr, 0, 1, TRUE, TRUE);	// Must be 0 to 1
	jit_class_addattr(_jit_gl_windowpos_class, attr);


	// Hide / Show window
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset, 
										 "hide", 
										 _jit_sym_long, 
										 attrflags, 
										 (method)NULL, (method)jit_gl_windowpos_hide, 
										 calcoffset(t_jit_gl_windowpos, hide));
	jit_attr_addfilterset_clip(attr, 0, 1, TRUE, TRUE);	// Must be 0 to 1
	jit_class_addattr(_jit_gl_windowpos_class, attr);


	// must register for ob3d use
	jit_class_addmethod(_jit_gl_windowpos_class, (method)jit_object_register, "register", A_CANT, 0L);

	jit_class_register(_jit_gl_windowpos_class);

	return JIT_ERR_NONE;
}


t_jit_gl_windowpos *jit_gl_windowpos_new(t_symbol * dest_name)
{
	t_jit_gl_windowpos *x;

	// make jit object
	if (x = (t_jit_gl_windowpos *)jit_object_alloc(_jit_gl_windowpos_class)) 	{
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);

		x->mode = 0;			// topmost mode (top or normal)
		x->fullscreen = 0;		// fullscreen
		x->fsmenubar = 0;		// fullscreen menubar
		x->hide = 0;			// Hide window
		x->window_hwnd = NULL;	// window

		sprintf(x->window_name, "%s", dest_name->s_name);
		// possibly can't find the window here or it might be the wrong one so check it in mode change

	} 
	else 
	{
		x = NULL;
	}	
	return x;
}


void jit_gl_windowpos_free(t_jit_gl_windowpos *x)
{
	// free our ob3d data 
	jit_ob3d_free(x);
}


t_jit_err jit_gl_windowpos_draw(t_jit_gl_windowpos *x)
{
	return JIT_ERR_NONE;
}

t_jit_err jit_gl_windowpos_dest_closing(t_jit_gl_windowpos *x)
{
	// nothing in this object to free. 
	return JIT_ERR_NONE;
}

t_jit_err jit_gl_windowpos_dest_changed(t_jit_gl_windowpos *x)
{
	// nothing in this object to update. 
	return JIT_ERR_NONE;
}

// top window mode (default is 0 - off - the render window is normal)
t_jit_err jit_gl_windowpos_mode(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv)
{
	char title[256];
	char c = jit_atom_getlong(argv);
	HWND hWndTaskBar;

	// post("Vers 1.04 - jjit_gl_windowpos_mode");

	// Quit if fullscreen
	if(x->fullscreen) return JIT_ERR_NONE;

	x->mode = c;

	//		Topmost Mode
	//			0 - top
	//			1 - bottom

	// First time ?
	if(x->window_hwnd == NULL) {
		// find the OpenGL render window
		HDC	GLhdc = wglGetCurrentDC();
		x->window_hwnd = WindowFromDC(GLhdc); 
	}

	// x->fsmenubar = 1;


	if(x->window_hwnd != NULL) {

		switch (x->mode) {

			case 0 : // bottom (will show the window if hidden)
				post("Vers 1.04 - jjit_gl_windowpos_mode  1 : NOT TOPMOST");
				SetWindowPos(x->window_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				ShowWindow(x->window_hwnd, SW_SHOW);
				break;

			case 1 : // (Default on start) on top (will show the window if hidden)
				post("Vers 1.04 - jjit_gl_windowpos_mode : case 0 TOPMOST");
				SetWindowPos(x->window_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
				ShowWindow(x->window_hwnd, SW_SHOW);
				break;

			default :
				break;

		}

	}

	return JIT_ERR_NONE;
}

// fsmenubar - fullscreen menubar flag (default is 0 - off)
t_jit_err jit_gl_windowpos_fsmenubar(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv)
{
	char title[256];
	char c = jit_atom_getlong(argv);

	x->fsmenubar = c;

	// post("Vers 1.04 - jjit_gl_windowpos_fsmenubar : FSMENUBAR = %d", (int)c);

	return JIT_ERR_NONE;
}

// hide - Show (0) or Hide window (default is 0 - Show)
t_jit_err jit_gl_windowpos_hide(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv)
{
	char title[256];
	char c = jit_atom_getlong(argv);

	// Quit if fullscreen
	if(x->fullscreen) return JIT_ERR_NONE;

	x->hide = c;
	// post("Vers 1.04 - jjit_gl_windowpos_hide : HIDE = %d", (int)c);

	//			0 - show
	//			1 - hide

	if(x->window_hwnd == NULL) {
		// find the OpenGL render window
		HDC	GLhdc = wglGetCurrentDC();
		x->window_hwnd = WindowFromDC(GLhdc); 
	}

	if(x->window_hwnd != NULL) {

		switch (x->hide) {

			case 0 : // visible (does not change top or bottom)
				// post("Vers 1.04 - jjit_gl_windowpos_hide : SHOW");
				ShowWindow(x->window_hwnd, SW_SHOW);
				break;


			case  1 : // hidden (does not change top or bottom)
				// post("Vers 1.04 - jjit_gl_windowpos_hide : HIDE");
				ShowWindow(x->window_hwnd, SW_HIDE);
				// SetWindowPos(x->window_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
				break;

			default :
				break;

		}

	}


	return JIT_ERR_NONE;
}

// fullscreen flag (default is 0 - off)
t_jit_err jit_gl_windowpos_fullscreen(t_jit_gl_windowpos *x, void *attr, long argc, t_atom *argv)
{
	char title[256];
	char c = jit_atom_getlong(argv);
	DWORD FullScreenStyle;
	RECT rectWorkArea;
	RECT rectTaskBar;
	HWND hWndTaskBar;
	HWND hWndMode;
	int addX, addY;

	x->fullscreen = c;

	//		Fullscreen
	//			0 - normal
	//			1 - fullscreen

	if(x->window_hwnd == NULL) {
		// find the OpenGL render window
		HDC	GLhdc = wglGetCurrentDC();
		x->window_hwnd = WindowFromDC(GLhdc); 
		if(!x->fullscreen) {
			GetWindowRect(x->window_hwnd, &x->windowRect);
			GetClientRect(x->window_hwnd, &x->clientRect);
			x->dwStyle = GetWindowLongPtrA(x->window_hwnd, GWL_STYLE);
			GetWindowTextA(x->window_hwnd, (LPSTR)title, 256);
		}
	}

	if(x->window_hwnd != NULL) {

		switch (x->fullscreen) {

			case 0 : // normal
				// post("Vers 1.04 - jjit_gl_windowpos_fullscreen : NORMAL");
				SetWindowLongPtrA(x->window_hwnd, GWL_STYLE, x->dwStyle); // restore original style
				
				if(x->mode == 0) 
					hWndMode = HWND_TOP; 
				else
					hWndMode = HWND_TOPMOST; // topmost was selected

				SetWindowPos(x->window_hwnd, hWndMode, 
							 x->windowRect.left-x->clientRect.left, 
							 x->windowRect.top-x->clientRect.top, 
							 x->windowRect.right-x->windowRect.left, 
							 x->windowRect.bottom-x->windowRect.top, SWP_SHOWWINDOW);
				ShowCursor(true);
				break;

			case 1 : // Fullscreen - take note of fsMenuBar flag - which shows the taskbar or not

				// post("getting window style and dimensions");
				GetWindowRect(x->window_hwnd, &x->windowRect);
				GetClientRect(x->window_hwnd, &x->clientRect);
				hWndTaskBar = FindWindowA("Shell_TrayWnd", "");
				GetClientRect(hWndTaskBar, &rectTaskBar);
				x->dwStyle = GetWindowLongPtrA(x->window_hwnd, GWL_STYLE);
				GetWindowTextA(x->window_hwnd, (LPSTR)title, 256);
				SetWindowLongPtrA(x->window_hwnd, GWL_STYLE, WS_VISIBLE); // no other styles but visible

				addX = 0;
				addY = 0;
				if(x->dwStyle & WS_CAPTION) {
					addY  = GetSystemMetrics(SM_CYCAPTION);
					addY += GetSystemMetrics(SM_CYFRAME)*2;
					addX  += GetSystemMetrics(SM_CXFRAME)*2;
				}
				
				// Full screen with taskbar visible
				if(x->fsmenubar == 1) {
					// post("Vers 1.04 - jit_gl_windowpos_fullscreen : FULL SCREEN with taskbar");
					SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rectWorkArea, 0);
					// -1 to Y is needed to unhide the taskbar if it is set to autohide and to cover the empty area is it is hidden
					SetWindowPos(x->window_hwnd, HWND_NOTOPMOST, 0, -addY, (rectWorkArea.right-rectWorkArea.left)+addX, (rectWorkArea.bottom-rectWorkArea.top)+addY-1, SWP_SHOWWINDOW);
					// SWP_FRAMECHANGED is important or the taskbar doesn't get repainted
					SetWindowPos(hWndTaskBar, HWND_TOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE  | SWP_FRAMECHANGED);
					SetFocus(x->window_hwnd); // otherwise esc is lost
					ShowCursor(true);
				}
				else {
					// post("Vers 1.04 - jit_gl_windowpos_fullscreen : FULL SCREEN without taskbar");
					SetWindowPos(hWndTaskBar, HWND_NOTOPMOST, 0, 0, (rectTaskBar.right-rectTaskBar.left), (rectTaskBar.bottom-rectTaskBar.top), SWP_NOMOVE | SWP_NOSIZE);
					SetWindowPos(x->window_hwnd, HWND_TOPMOST, 0, -addY, GetSystemMetrics(SM_CXSCREEN)+addX, GetSystemMetrics(SM_CYSCREEN)+addY, SWP_SHOWWINDOW);
					SetFocus(x->window_hwnd);
					ShowCursor(false);
				}
				break;

			default :
				break;

		}
	}

	return JIT_ERR_NONE;
}

