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

*/
#define GL_BGRA_EXT 0x80E1

#include "malloc.h"
#include <direct.h>
#include "JSpout.h"
#include "../../SpoutSDK/Spout.h"

char   g_SenderName[256];
int	   g_Width		= 0;
int    g_Height		= 0;
bool   bInitialized	= false;
bool   bMemoryMode	= false;

SpoutSender sender;		// Spout sender object
SpoutReceiver receiver;	// Spout receiver object

void CheckGLerror();

JNIEXPORT jint JNICALL Java_JSpout_InitSender (JNIEnv *env, jclass c, jstring name, jint width, jint height, jint memorymode) {

	bool bRet = false;
	jint sharing_mode = 0; // 1 - memory 0 - texture : dependent on compatibility

	const char *nativeString = env->GetStringUTFChars(name, 0);

	// Set the global sender name, width and height
	strcpy_s(g_SenderName, 256, nativeString);
	g_Width = width;
	g_Height = height;

	if(memorymode == 1) { // user has selected memoryshare mode
		sender.SetMemoryShareMode(true);
	}
	else {
		// TODO - check DX11
		// Create a DX9 sender for initial release
		sender.SetDX9(true);
	}

	// SPOUT CreateSender
	// name						- name of this sender
	// width, height			- width and height of this sender
	// dwFormat					- optional DX11 texture format
	// DXGI_FORMAT_R8G8B8A8_UNORM - DX11 < > DX11
	// DXGI_FORMAT_B8G8R8A8_UNORM - DX11 < > DX9 (default)
	// Also sender.SetDX9compatible(true / false);
	// Returns true for success or false for initialisation failure.
	bRet = sender.CreateSender(g_SenderName, g_Width, g_Height);
	if(bRet) {
		// Check compatibility
		if(sender.GetMemoryShareMode()) {
			bMemoryMode = true;
			sharing_mode = 1;
		}
		else {
			bMemoryMode = false;
			sharing_mode = 0;
		}
		bInitialized = true;
	}
	else {
		bInitialized = false;
	}

	// release the input string
	env->ReleaseStringUTFChars(name, nativeString);

	if(bRet) 
		return sharing_mode;
	else
		return -1;

}

JNIEXPORT jint JNICALL Java_JSpout_InitReceiver (JNIEnv *env, jclass c, jstring name, jintArray dimarray, jint memorymode) {

	bool bRet = false;
	unsigned int width, height;
	char Sendername[256]; // user entered Sender name
	jint sharing_mode = 1; // 0 - memory 1 - texture : dependent on compatibility

	jboolean isCopy = JNI_FALSE;
	jint *dim = env->GetIntArrayElements(dimarray, &isCopy);
	isCopy = JNI_FALSE;

	width  = (unsigned int)dim[0];	// whatever was passed
	height = (unsigned int)dim[1];

	const char *nativestring = env->GetStringUTFChars(name, &isCopy);
	if(nativestring[0])
		strcpy_s(Sendername, 256, nativestring);
	else
		Sendername[0] = 0;

	if(memorymode == 1) { // user has selected memoryshare mode
		receiver.SetMemoryShareMode(true);
	}
	// TODO - test DX11
	// create a DX9 receiver for initial release - 
	else {
		receiver.SetDX9(true);
	}

	//
	// SPOUT CreateReceiver
	//
	// Returns :
	// name						- name of the sender found if the name passed was null
	// width, height			- width and height of the sender
	// Returns true for success or false for initialisation failure.
	bRet = receiver.CreateReceiver(Sendername, width, height);

	if(bRet) {
		// Set globals
		g_Width  = width;
		g_Height = height;
		strcpy_s(g_SenderName, 256, Sendername);

		// Check compatibility
		if(sender.GetMemoryShareMode()) {
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
		dim[0] = (jint)width;
		dim[1] = (jint)height;
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

	sender.ReleaseSender();
	bInitialized = false;
	return true;

}

JNIEXPORT jboolean JNICALL Java_JSpout_ReleaseReceiver (JNIEnv *env, jclass c) {

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

	if(receiver.ReceiveTexture(g_SenderName, width, height)) {

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

		// ---------------------------------------------------------------
		// retrieve opengl texture data directly to image pixels rather than via an fbo and texture
		// Java always operates on big-endian format so if C handles
		// data in little-endian format, bytes need to be reversed.
		// 16-03-14 - transfer directly using a different GL format
		// https://www.opengl.org/registry/specs/EXT/bgra.txt
		// EXT_bgra extends the list of host-memory color formats.
		// Specifically, it provides formats which match the memory layout of
		// Windows DIBs so that applications can use the same data in both
		// Windows API calls and OpenGL pixel API calls.
		receiver.BindSharedTexture();
		glGetTexImage(GL_TEXTURE_2D, 0,  GL_BGRA_EXT,  GL_UNSIGNED_BYTE, pix);
		receiver.UnBindSharedTexture();
		// --------------------------------------------
	}
	else {
		bRet = false;
	}

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
	if(receiver.GetDX9()) {
		return(receiver.SelectSenderPanel("/DX9"));
	}
	else {
		return(receiver.SelectSenderPanel("/DX11"));
	}
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
