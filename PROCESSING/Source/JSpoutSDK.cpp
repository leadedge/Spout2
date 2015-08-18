/*

        JSpout.cpp : Defines the functions for the JNI dll

		- - - - - - - - - - - - - - - - - - - - - - - - - - 
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
		- - - - - - - - - - - - - - - - - - - - - - - - - - 


		06-08-14 - conversion to Spout SDK
				 - some simplification of spout.pde
		14-08-14 - TODO - test DX11 - create a DX9 sender and receiver for initial release
				 - finally JSpout should have an optional DirectX mode flag
		16.08.14 - Tested endian for Receivetexture into Processing pixels (JAVA is big endian)
		18.08.14 - Set to DX9 for a receiver for testing
		20.08.14 - Changed back to pixel method for ReadTexture for testing
		24.08.14 - recompiled with MB sendernames class revision
		05.09.14 - compiled with revised SDK + cleanup
		30.09.14 - Java_JSpout_ReadTexture uses "ReceiveImage"
				 - to transfer shared texture to Processing image pixels
				 - DirectX 11
		04.02.15 - Fixed error in InitReceiver. Sender object used instead of receiver object for 
		           memoryshare test prevented sketch to stop and java was not terminated on close.
		05.02.15 - changed InitSender to protect against null string copy
				 - included FBO in call to SendTexture
		06.02.15 - SDK recompile - added UseD3D9 define to compile for both DirectX 9 or DirectX 11
		14.02.15 - SDK recompile for auto detection of DirectX compatibiliy (see SpoutGLDXinterop.cpp).
		21.04.15 - SDK recompile for both DX9 and DX11 versions for optional installer
		26.05.15 - Recompile for revised SpoutPanel registry write of sender name
		29.06.15 - Added Spout Controls
		03.07.15 - Checked for null name and text for release env in Java_JSpout_CheckControls
		07.07.15 - Converted VS2012 project to VS2010
				 - Compiled with JDK 8.45
		17.07.15 - Recomplile Win32 with SpoutControls class revisions. No changes made to JSpout functions.
		22.07.15 - Included "CreateControl"
		27.07.15 - Used pixel transfer method for Java_JSpout_ReadTexture due to problems with Intel 4400 graphics
				 - recompiled Win32 and x64 after work on Max externals
		01.08.15 - Recompile Win32 for 2.004 release VS2010 /MT UseD3D9 undefined
		01.08.15 - Recompile x64 for 2.004 release VS2012 /MT UseD3D9 undefined

*/
#define GL_BGRA_EXT 0x80E1

// Compile for DX9 instead of DX11
// Now not used - set by setup program instead
// #define UseD3D9

#include "malloc.h"
#include <direct.h>
#include "JSpout.h"
#include "../../SpoutSDK/Spout.h"
#include "../../SpoutSDK/SpoutControls.h"

char   g_SenderName[256];
int	   g_Width		= 0;
int    g_Height		= 0;
bool   bInitialized	= false;
bool   bMemoryMode	= false;

SpoutSender sender;		// Spout sender object
SpoutReceiver receiver;	// Spout receiver object
SpoutControls spoutcontrols; // Spout controls object

void CheckGLerror();

JNIEXPORT jint JNICALL Java_JSpout_InitSender (JNIEnv *env, jclass c, jstring name, jint width, jint height, jint memorymode) {

	bool bRet = false;
	char Sendername[256]; // user entered Sender name
	unsigned int uWidth, uHeight;
	jint sharing_mode = 0; // 1 - memory 0 - texture : dependent on compatibility
	jboolean isCopy = JNI_FALSE;

	UNREFERENCED_PARAMETER(c);

	try {
		
		// const char *nativestring = env->GetStringUTFChars(name, 0);
		const char *nativestring = env->GetStringUTFChars(name, &isCopy);

		// Set the sender name, width and height
		if(nativestring[0])
			strcpy_s(Sendername, 256, nativestring);
		else
			Sendername[0] = 0;

		uWidth = (unsigned int)width;
		uHeight = (unsigned int)height;


		/*
		// Set to DX9 for compatibility with Version 1 apps
		#ifdef UseD3D9
		sender.SetDX9(true);
		#else
		sender.SetDX9(false);
		#endif
		*/

		// user has selected memoryshare mode
		if(memorymode == 1) sender.SetMemoryShareMode(true);

		//
		// Default settings are :
		//		DirectX 11
		//		Compatible texture format DXGI_FORMAT_B8G8R8A8_UNORM
		//

		// SPOUT CreateSender
		// name						- name of this sender
		// width, height			- width and height of this sender
		// dwFormat					- optional DX11 texture format
		//		DXGI_FORMAT_R8G8B8A8_UNORM - DX11 < > DX11
		//		DXGI_FORMAT_B8G8R8A8_UNORM - DX11 < > DX9 (default)
		// Also sender.SetDX9compatible(true / false); (true default)
		// Returns true for success or false for initialisation failure.
		bRet = sender.CreateSender(Sendername, uWidth, uHeight);
		if(bRet) {

			// Set globals
			g_Width  = (int)uWidth;
			g_Height = (int)uHeight;
			strcpy_s(g_SenderName, 256, Sendername);

			// Check compatibility
			if(sender.GetMemoryShareMode()) {
				bMemoryMode = true;
				sharing_mode = 1; // memory mode
			}
			else {
				bMemoryMode = false;
				sharing_mode = 0; // texture mode
			}
			bInitialized = true;
		}
		else {
			bInitialized = false;
		}

		// release the input string
		env->ReleaseStringUTFChars(name, nativestring);

		if(bRet) 
			return sharing_mode;
		else
			return -1; // error
	}
	catch (...) {
		MessageBoxA(NULL, "Exception in InitSender", "JSpout", MB_OK);
		return -1;
	}

}

JNIEXPORT jint JNICALL Java_JSpout_InitReceiver (JNIEnv *env, jclass c, jstring name, jintArray dimarray, jint memorymode) {

	bool bRet = false;
	unsigned int uWidth, uHeight;
	char Sendername[256]; // user entered Sender name
	jint sharing_mode = 1; // 0 - memory 1 - texture : dependent on compatibility

	UNREFERENCED_PARAMETER(c);

	jboolean isCopy = JNI_FALSE;
	jint *dim = env->GetIntArrayElements(dimarray, &isCopy);
	isCopy = JNI_FALSE;

	uWidth  = (unsigned int)dim[0];	// whatever was passed
	uHeight = (unsigned int)dim[1];

	const char *nativestring = env->GetStringUTFChars(name, &isCopy);
	if(nativestring[0])
		strcpy_s(Sendername, 256, nativestring);
	else
		Sendername[0] = 0;

	// Set to DX9 for compatibility with Version 1 apps
	#ifdef UseD3D9
	receiver.SetDX9(true);
	#else
	receiver.SetDX9(false);
	#endif


	// user has selected memoryshare mode
	if(memorymode == 1) receiver.SetMemoryShareMode(true);
	
	//
	// Default settings are	DirectX 11
	//

	//
	// SPOUT CreateReceiver
	//
	// Returns :
	// name						- name of the sender found if the name passed was null
	// width, height			- width and height of the sender
	// Returns true for success or false for initialisation failure.
	bRet = receiver.CreateReceiver(Sendername, uWidth, uHeight);

	if(bRet) {
		// Set globals
		g_Width  = (int)uWidth;
		g_Height = (int)uHeight;
		strcpy_s(g_SenderName, 256, Sendername);

		// Check compatibility
		if(receiver.GetMemoryShareMode()) {
			bMemoryMode = true;
			sharing_mode = 1;
		}
		else {
			bMemoryMode = false;
			sharing_mode = 0;
		}
	}

	if(bRet) {
		// Pass back the width and height of the sender it connected to
		// The new sender name can be retreived with GetSenderName
		dim[0] = (jint)uWidth;
		dim[1] = (jint)uHeight;
		bInitialized = true;
	}
	else {
		bInitialized = false;
	}

	// JNI_ABORT - done with the data which is then freed and not passed back
	// JNI_COMMIT - done with the data which is passed back but not freed
	// 0 - done with the data which is passed back and freed
	env->ReleaseIntArrayElements(dimarray, dim, 0);
	env->DeleteLocalRef(dimarray);
	env->ReleaseStringUTFChars(name, nativestring);

	if(bRet) {
		return sharing_mode;
	}
	else {
		return -1;
	}

}


JNIEXPORT jboolean JNICALL Java_JSpout_ReleaseSender (JNIEnv *env, jclass c) {

	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	sender.ReleaseSender();
	bInitialized = false;
	return true;

}

JNIEXPORT jboolean JNICALL Java_JSpout_ReleaseReceiver (JNIEnv *env, jclass c) {

	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	receiver.ReleaseReceiver();
	bInitialized = false;
	return true;

}

// Not used
JNIEXPORT jboolean JNICALL Java_JSpout_ReadFromSharedMemory (JNIEnv *env, jclass c, jintArray dimarray, jintArray  pixarray)
{
	bool bRet = true;
	unsigned int width, height;
	unsigned char *rgbBuffer = NULL;

	UNREFERENCED_PARAMETER(c);

	// Quit if memoryshare has not been initialized
	if(!bMemoryMode) return false;

	// should check isCopy here
	jboolean isCopy = JNI_FALSE;
	jint *dim = env->GetIntArrayElements(dimarray, &isCopy);
	isCopy = JNI_FALSE;
	jint *pix = env->GetIntArrayElements(pixarray, &isCopy);

	width  = (unsigned int)dim[0];
	height = (unsigned int)dim[1];

	if(width == 0 || height == 0) {
		bRet = false;
		goto exit;
	}

	rgbBuffer = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));

	// Memorymode is RGB. Format argument is disregarded
	if(receiver.ReceiveImage(g_SenderName, width, height, rgbBuffer)) {
		// If the image size has changed, return now with the changed size and true to show a 
		// or the buffer sizes will not match. Return true to show a successful read and 
		// resize the image in the calling program
		if(dim[0] != (jint)width || dim[1] != (jint)height) {
			dim[0] = (jint)width;
			dim[1] = (jint)height;
			free((void*)rgbBuffer);
			bRet = true;
			goto exit;
		}
		else {
			// otherwise transfer the image data to our image pixels
			unsigned char *pBits = rgbBuffer;
			jint *pixels = pix;
			unsigned int red, grn, blu, alf;
			alf = (255U << 24);
			for(unsigned int i = 0; i < width*height; i++) {
				red = ((unsigned int)*pBits++ << 16);
				grn = ((unsigned int)*pBits++ <<  8);
				blu = ((unsigned int)*pBits++      );
				pixels[i] = alf | red | grn | blu;
			}
		}
	}

	free((void*)rgbBuffer);

exit :

	// JNI_ABORT - done with the data which is then freed and not passed back
	// JNI_COMMIT - done with the data which is passed back but not freed
	// 0 - done with the data which is passed back and freed
	env->ReleaseIntArrayElements(dimarray, dim, 0);
	env->DeleteLocalRef(dimarray);
	env->ReleaseIntArrayElements(pixarray, pix, 0);
	env->DeleteLocalRef(pixarray);

	return bRet;

}

// Not used
JNIEXPORT jboolean JNICALL Java_JSpout_WriteToSharedMemory (JNIEnv *env, jclass c, jint width, jint height, jintArray  pixarray)
{
	unsigned char* rgbBuffer;
	unsigned int pixel;
	unsigned int red, grn, blu;	

	UNREFERENCED_PARAMETER(c);
	
	jboolean isCopy = JNI_FALSE;

	jint *pix = env->GetIntArrayElements(pixarray, &isCopy);

	if(pix == NULL || width == 0 || height == 0)
		return false;

	// Quit if memoryshare has not been initialized
	if(!bMemoryMode) return false;

	rgbBuffer = (unsigned char *)malloc(width*height*3*sizeof(unsigned char) + sizeof(BITMAPINFOHEADER));

	// Copy the image data to the local rgb buffer - pBits
	jint *pixels = pix;
	for(int i = 0; i < width*height; i++) {
		pixel = (unsigned int)pixels[i];
		red = (pixel >> 16) & 0xff;
		grn = (pixel >>  8) & 0xff;
		blu = (pixel      ) & 0xff;
		*rgbBuffer++ = (unsigned char)red;
		*rgbBuffer++ = (unsigned char)grn;
		*rgbBuffer++ = (unsigned char)blu;
	}

	// Memorymode is RGB. Format, alignment and invert arguments are disregarded
	sender.SendImage(rgbBuffer, width, height);

	free((void *)rgbBuffer);

	// Tell the environment to write back the changes (if it's a copy)
	// or release the data (if it's not a copy)
	env->ReleaseIntArrayElements(pixarray, (jint *)pix, JNI_ABORT); // done with the data because we are just reading it
	env->DeleteLocalRef(pixarray);

	return true;
}


JNIEXPORT jboolean JNICALL Java_JSpout_WriteTexture (JNIEnv *env, jclass c, jint width, jint height, jint texID, jint texTarget, jboolean bInvert)
{
	bool bInv;
	
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	// Only use textureshare if it has initialized
	if(!bInitialized) return false;

	// To quiet the compiler performance warning, use if/else rather than a cast
	if(bInvert) bInv = true;
	else bInv = false;

	return(sender.SendTexture(texID, texTarget, width, height, bInv));

}


// COPY THE SHARED TEXTURE TO IMAGE PIXELS
JNIEXPORT jboolean JNICALL Java_JSpout_ReadTexture (JNIEnv *env, jclass c, jintArray dimarray, jintArray pixarray)
{
	bool bRet = false;
	jboolean isCopy = FALSE;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned char *rgbBuffer = NULL; // LJ DEBUG

	UNREFERENCED_PARAMETER(c);

	// Quit if texture share has not been initialized
	if(!bInitialized) return false;

	// should check isCopy here
	isCopy = JNI_FALSE;
	jint *dim = env->GetIntArrayElements(dimarray, &isCopy);

	isCopy = JNI_FALSE;
	jint *pix = env->GetIntArrayElements(pixarray, &isCopy);
	
	width  = (unsigned int)dim[0];
	height = (unsigned int)dim[1];

	if(width == 0 || height == 0) {
		bRet = false;
		goto exit;
	}


	// LJ DEBUG
    rgbBuffer = (unsigned char *)malloc(width*height*4*sizeof(unsigned char));
	GLint previousFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);

    // if(receiver.ReceiveImage(g_SenderName, width, height, rgbBuffer, GL_RGBA)) { // default format is RGBA
	if(receiver.ReceiveImage(g_SenderName, width, height, rgbBuffer, GL_RGBA, previousFBO)) {
        
		bRet = true;

        // If the image size has changed, return now with the changed size and true to show a
        // or the buffer sizes will not match. Return true to show a successful read and
        // resize the image in the calling program
        if(dim[0] != (jint)width || dim[1] != (jint)height) {
            dim[0] = (jint)width;
            dim[1] = (jint)height;
            free((void*)rgbBuffer);
            goto exit;
        }
        else {
            // Data appears to come out is BGRA format, possibly endian issue as below
            // so read in that order and assemble Processing pixel as ABGR (reverse RGBA)
            // Clues http://www.felixgers.de/teaching/jogl/imagingProg.html
            unsigned char *pBits = rgbBuffer;
            unsigned __int32 *pixels = (unsigned __int32 *)pix;
            unsigned __int32 red, grn, blu, alf;
            for(unsigned int i = 0; i < width*height; i++) {
                // Read pixels in BGRA order
                blu = ((unsigned __int32)*pBits++ << 16);
                grn = ((unsigned __int32)*pBits++ <<  8);
                red = ((unsigned __int32)*pBits++      );
                alf = ((unsigned __int32)*pBits++ << 24);
                // Assemble in ABGR order
                pixels[i] = (alf & 0xFF000000) | (blu & 0x00FF0000) | (grn & 0x0000FF00) | (red & 0x000000FF);
            }
			
			// make sure we are current
			width  = dim[0];
			height = dim[1];

        }
    }
	else {
		bRet = false;
	}

    free((void*)rgbBuffer);

	/*
	// ---------------------------------------------------------------
	// retrieve opengl texture data directly to image pixels
	// bytes need to be reversed.
	// Transfer directly using a different GL format
	// https://www.opengl.org/registry/specs/EXT/bgra.txt
	// 
	// retrieve the current fbo
	GLint previousFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);

	if(receiver.ReceiveImage(g_SenderName, width, height, (unsigned char *)pix, GL_BGRA_EXT, previousFBO)) {

		bRet = true;

		// If the image size has changed, return now with the changed size
		// or the buffer sizes will not match. Resize the image in the calling program
		if(dim[0] != (jint)width || dim[1] != (jint)height) { // check passed values
			dim[0] = (jint)width;
			dim[1] = (jint)height;
			goto exit;
		}
	
		// make sure we are current
		width  = dim[0];
		height = dim[1];

	}
	else {
		bRet = false;
	}
	*/


exit :

	// JNI_ABORT - done with the data which is then freed and not passed back
	// JNI_COMMIT - done with the data which is passed back but not freed
	// 0 - done with the data which is passed back and freed
	env->ReleaseIntArrayElements(dimarray, dim, 0);
	env->DeleteLocalRef(dimarray);
	env->ReleaseIntArrayElements(pixarray, pix, 0);
	env->DeleteLocalRef(pixarray);

	return bRet;

}


// Used by a receiver
JNIEXPORT jstring JNICALL Java_JSpout_GetSenderName (JNIEnv *env, jclass c)
{
	char Sendername[256];
	unsigned int width, height;
	DWORD dwFormat;
	HANDLE dxShareHandle;

	UNREFERENCED_PARAMETER(c);

	if(receiver.GetMemoryShareMode()) {
		strcpy_s(Sendername, 256, "Memory share");
	}
	else if (receiver.GetSenderCount() == 0) {
		strcpy_s(Sendername, 256, "No senders");
	}
	else {
		// Get the current sender name
		strcpy_s(Sendername, 256, "Sender closed");
		// Does it still exist ?
		if(g_SenderName[0]) {
			if(receiver.GetSenderInfo(g_SenderName, width, height, dxShareHandle, dwFormat))
				strcpy_s(Sendername, 256, g_SenderName);
		}
		// just leave the name because the sender might reopen
	}

	jstring jstrSenderName = env->NewStringUTF(Sendername);

	return jstrSenderName;
}


// Function to return a selected Sender name from a dialog showing the list in shared memory
JNIEXPORT jboolean JNICALL Java_JSpout_SenderDialog (JNIEnv *env, jclass c)
{
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	if(receiver.GetDX9()) {
		return(receiver.SelectSenderPanel("/DX9"));
	}
	else {
		return(receiver.SelectSenderPanel("/DX11"));
	}
}

//
// Spout controls
//

//
// CreateControl(name, type, minimum, maximum, value, text)
//
JNIEXPORT jboolean JNICALL Java_JSpout_CreateControl
  (JNIEnv *env, jclass c, jstring name, jstring type, jfloat minimum, jfloat maximum, jfloat value, jstring text)
{

	bool bRet = false;
	char ControlName[256]; // control name
	char ControlType[256]; // control type
	char ControlText[256]; // control text
	float fMin;
	float fMax;
	float fValue;
	jboolean isCopy = JNI_FALSE;

	UNREFERENCED_PARAMETER(c);

	try {
		const char *nativestring1 = env->GetStringUTFChars(name, &isCopy);
		if(nativestring1[0])
			strcpy_s(ControlName, 256, nativestring1);
		else
			ControlName[0] = 0;

		const char *nativestring2 = env->GetStringUTFChars(type, &isCopy);
		if(nativestring2[0])
			strcpy_s(ControlType, 256, nativestring2);
		else
			ControlType[0] = 0;

		const char *nativestring3 = env->GetStringUTFChars(text, &isCopy);
		if(nativestring3[0])
			strcpy_s(ControlText, 256, nativestring3);
		else
			ControlText[0] = 0;

		fMin   = (float)minimum;
		fMax   = (float)maximum;
		fValue = (float)value;

		if(ControlName[0] && ControlType[0]) {

			// event
			// bool
			// float
			// text

			if(strcmp(ControlType, "event") == 0) // button
				bRet = spoutcontrols.CreateControl(ControlName, ControlType, fValue);
			else if(strcmp(ControlType, "bool") == 0) // checkbox
				bRet = spoutcontrols.CreateControl(ControlName, ControlType, fValue);
			else if(strcmp(ControlType, "text") == 0) {	// text
				if(ControlText[0])
					bRet = spoutcontrols.CreateControl(ControlName, ControlType, ControlText);
				else
					bRet = spoutcontrols.CreateControl(ControlName, ControlType);
			}
			else { // float
				bRet = spoutcontrols.CreateControl(ControlName, ControlType, fMin, fMax, fValue);
			}

		}

		// release the input strings
		env->ReleaseStringUTFChars(name, nativestring1);
		env->ReleaseStringUTFChars(name, nativestring2);
		env->ReleaseStringUTFChars(name, nativestring3);

		return (jboolean)bRet;

	}
	catch (...) {
		MessageBoxA(NULL, "Exception in CreateControl(name, type, min, max, value, text)", "JSpout", MB_OK);
		return false;
	}

}



JNIEXPORT jboolean JNICALL Java_JSpout_OpenControls (JNIEnv *env, jclass c, jstring name) 
{

	bool bRet = false;
	char Sendername[256]; // user entered Sender name
	jboolean isCopy = JNI_FALSE;

	UNREFERENCED_PARAMETER(c);

	try {
		
		const char *nativestring = env->GetStringUTFChars(name, &isCopy);

		// Set the sender name, width and height
		if(nativestring[0])
			strcpy_s(Sendername, 256, nativestring);
		else
			Sendername[0] = 0;

		bRet = spoutcontrols.OpenControls(Sendername);

		// release the input string
		env->ReleaseStringUTFChars(name, nativestring);

		return (jboolean)bRet;

	}
	catch (...) {
		MessageBoxA(NULL, "Exception in OpenControls", "JSpout", MB_OK);
		return false;
	}

}

JNIEXPORT jint JNICALL Java_JSpout_CheckControls (JNIEnv * env, jclass c, jobjectArray controlName, jintArray controlType, jfloatArray controlValue, jobjectArray controlText)
{
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	jint nControls = 0;
	jint *Type;
	jfloat *Value;
	jboolean isCopy = JNI_FALSE;
	bool bRet = false;
	bool bText = false;
	bool bName = false;


	// Spout controls vector
	vector<control> Controls;

	if(spoutcontrols.CheckControls(Controls)) {

		if(Controls.size() > 0) {

			//
			// Initialization
			//

			// All the arrays are the same size
			// jsize stringCount = env->GetArrayLength(controlName);
			jsize stringCount = env->GetArrayLength(controlType);

			// Control Types
			Type = env->GetIntArrayElements(controlType, &isCopy);

			// Control Values
			Value = env->GetFloatArrayElements(controlValue, &isCopy);

			// Return names and text
			for(int i=0; i<stringCount, i<(int)Controls.size(); i++) {
				// Return the names
				if(!Controls.at(i).name.empty()) {
					jstring string = env->NewStringUTF(Controls.at(i).name.c_str());
					env->SetObjectArrayElement(controlName, i, string);
					bName = true;
				}

				// Return the text
				if(!Controls.at(i).text.empty()) {
					jstring string = env->NewStringUTF(Controls.at(i).text.c_str());
					env->SetObjectArrayElement(controlText, i, string);
					bText = true;
				}

				// Return the type
				Type[i] = Controls.at(i).type;

				// Return the value
				Value[i] = Controls.at(i).value;

			}
		}

		env->ReleaseIntArrayElements(controlType, Type, 0);
		env->ReleaseFloatArrayElements(controlValue, Value, 0);

		if(bName)
			env->DeleteLocalRef(controlName);

		if(bText)
			env->DeleteLocalRef(controlText);

		env->DeleteLocalRef(controlType);
		env->DeleteLocalRef(controlValue);

		return (jint)Controls.size();
	}

	return 0;
}

JNIEXPORT jboolean JNICALL Java_JSpout_CloseControls (JNIEnv *env, jclass c)
{
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(env);

	return(spoutcontrols.CloseControls());


}


// check OpenGL error
void CheckGLerror()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
		string error;
        switch(err) {
			case GL_INVALID_OPERATION: error="INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:      error="INVALID_ENUM";      break;
            case GL_INVALID_VALUE:     error="INVALID_VALUE";     break;
            case GL_OUT_OF_MEMORY:     error="OUT_OF_MEMORY";     break;
            // case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
    }
}
