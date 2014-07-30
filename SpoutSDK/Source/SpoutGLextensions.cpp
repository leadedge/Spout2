/*
//
//
//			spoutGLextensions.cpp
//
//			Used for load of openGL extensions with option
//			to use Glew or disable dynamic load of specific extensions
//			See spoutGLext.h
//

		Copyright (c) 2014>, Lynn Jarvis. All rights reserved.

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
*/

#include "spoutGLextensions.h"

#ifndef USE_GLEW

// GL/DX extensions
PFNWGLDXOPENDEVICENVPROC				wglDXOpenDeviceNV				= NULL;
PFNWGLDXREGISTEROBJECTNVPROC			wglDXRegisterObjectNV			= NULL;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC	wglDXSetResourceShareHandleNV	= NULL;
PFNWGLDXLOCKOBJECTSNVPROC				wglDXLockObjectsNV				= NULL;
PFNWGLDXUNLOCKOBJECTSNVPROC				wglDXUnlockObjectsNV			= NULL;
PFNWGLDXCLOSEDEVICENVPROC				wglDXCloseDeviceNV				= NULL;
PFNWGLDXUNREGISTEROBJECTNVPROC			wglDXUnregisterObjectNV			= NULL;

// FBO extensions
#ifdef USE_FBO_EXTENSIONS
glBindFramebufferEXTPROC				glBindFramebufferEXT			= NULL;
glBindRenderbufferEXTPROC				glBindRenderbufferEXT			= NULL;
glCheckFramebufferStatusEXTPROC			glCheckFramebufferStatusEXT		= NULL;
glDeleteFramebuffersEXTPROC				glDeleteFramebuffersEXT			= NULL;
glDeleteRenderBuffersEXTPROC			glDeleteRenderBuffersEXT		= NULL;
glFramebufferRenderbufferEXTPROC		glFramebufferRenderbufferEXT	= NULL;
glFramebufferTexture1DEXTPROC			glFramebufferTexture1DEXT		= NULL;
glFramebufferTexture2DEXTPROC			glFramebufferTexture2DEXT		= NULL;
glFramebufferTexture3DEXTPROC			glFramebufferTexture3DEXT		= NULL;
glGenFramebuffersEXTPROC				glGenFramebuffersEXT			= NULL;
glGenRenderbuffersEXTPROC				glGenRenderbuffersEXT			= NULL;
glGenerateMipmapEXTPROC					glGenerateMipmapEXT				= NULL;
glGetFramebufferAttachmentParameterivEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
glGetRenderbufferParameterivEXTPROC		glGetRenderbufferParameterivEXT	= NULL;
glIsFramebufferEXTPROC					glIsFramebufferEXT				= NULL;
glIsRenderbufferEXTPROC					glIsRenderbufferEXT				= NULL;
glRenderbufferStorageEXTPROC			glRenderbufferStorageEXT		= NULL;
#endif

// FBO blit extensions
glBlitFramebufferEXTPROC				glBlitFramebufferEXT			= NULL;

#ifdef USE_FBO_EXTENSIONS
// OpenGL sync control extensions
PFNWGLSWAPINTERVALEXTPROC				wglSwapIntervalEXT				= NULL;
PFNWGLGETSWAPINTERVALEXTPROC			wglGetSwapIntervalEXT			= NULL;
#endif

// PBO extensions
#ifdef USE_PBO_EXTENSIONS
glGenBuffersPROC						glGenBuffers					= NULL;
glDeleteBuffersPROC						glDeleteBuffers					= NULL;
glBindBufferPROC						glBindBuffer					= NULL;
glBufferDataPROC						glBufferData					= NULL;
glMapBufferPROC							glMapBuffer						= NULL;
glUnmapBufferPROC						glUnmapBuffer					= NULL;
#endif

#endif

//
// Load the Nvidia-Extensions dynamically
//
bool loadInteropExtensions() {

	#ifdef USE_GLEW
	if(WGLEW_NV_DX_interop)
		return true;
	else
		return false;
	#else
	wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
	if(!wglDXOpenDeviceNV) {
		return false;
	}
	wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	if(!wglDXRegisterObjectNV) {
		return false;
	}
	wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	if(!wglDXUnregisterObjectNV) {
		return false;
	}
	wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)wglGetProcAddress("wglDXSetResourceShareHandleNV");
	if(!wglDXSetResourceShareHandleNV) {
		return false;
	}
	wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	if(!wglDXLockObjectsNV)	{
		return false;
	}
	wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");
	if(!wglDXUnlockObjectsNV) {
		return false;
	}
	wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");
	if(!wglDXUnlockObjectsNV) {
		return false;
	}

	return true;
	#endif

}

bool loadFBOextensions() {

#ifdef USE_FBO_EXTENSIONS

	#ifdef USE_GLEW
	if(GLEW_EXT_framebuffer_object)
		return true;
	else
		return false;
	#else
	try { // load extensions for FBO
		glBindFramebufferEXT						= (glBindFramebufferEXTPROC)(unsigned)wglGetProcAddress("glBindFramebufferEXT");
		glBindRenderbufferEXT						= (glBindRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glBindRenderbufferEXT");
		glCheckFramebufferStatusEXT					= (glCheckFramebufferStatusEXTPROC)(unsigned)wglGetProcAddress("glCheckFramebufferStatusEXT");
		glDeleteFramebuffersEXT						= (glDeleteFramebuffersEXTPROC)(unsigned)wglGetProcAddress("glDeleteFramebuffersEXT");
		glDeleteRenderBuffersEXT					= (glDeleteRenderBuffersEXTPROC)(unsigned)wglGetProcAddress("glDeleteRenderbuffersEXT");
		glFramebufferRenderbufferEXT				= (glFramebufferRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glFramebufferRenderbufferEXT");
		glFramebufferTexture1DEXT					= (glFramebufferTexture1DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture1DEXT");
		glFramebufferTexture2DEXT					= (glFramebufferTexture2DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture2DEXT");
		glFramebufferTexture3DEXT					= (glFramebufferTexture3DEXTPROC)(unsigned)wglGetProcAddress("glFramebufferTexture3DEXT");
		glGenFramebuffersEXT						= (glGenFramebuffersEXTPROC)(unsigned)wglGetProcAddress("glGenFramebuffersEXT");
		glGenRenderbuffersEXT						= (glGenRenderbuffersEXTPROC)(unsigned)wglGetProcAddress("glGenRenderbuffersEXT");
		glGenerateMipmapEXT							= (glGenerateMipmapEXTPROC)(unsigned)wglGetProcAddress("glGenerateMipmapEXT");
		glGetFramebufferAttachmentParameterivEXT	= (glGetFramebufferAttachmentParameterivEXTPROC)(unsigned)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
		glGetRenderbufferParameterivEXT				= (glGetRenderbufferParameterivEXTPROC)(unsigned)wglGetProcAddress("glGetRenderbufferParameterivEXT");
		glIsFramebufferEXT							= (glIsFramebufferEXTPROC)(unsigned)wglGetProcAddress("glIsFramebufferEXT");
		glIsRenderbufferEXT							= (glIsRenderbufferEXTPROC)(unsigned)wglGetProcAddress("glIsRenderbufferEXT");
		glRenderbufferStorageEXT					= (glRenderbufferStorageEXTPROC)(unsigned)wglGetProcAddress("glRenderbufferStorageEXT");
	}
	catch (...) {
		return false;
	}
	if	  ( glBindFramebufferEXT						!= NULL && 
			glBindRenderbufferEXT						!= NULL && 
			glCheckFramebufferStatusEXT					!= NULL && 
			glDeleteFramebuffersEXT						!= NULL && 
			glDeleteRenderBuffersEXT					!= NULL &&
			glFramebufferRenderbufferEXT				!= NULL && 
			glFramebufferTexture1DEXT					!= NULL && 
			glFramebufferTexture2DEXT					!= NULL && 
			glFramebufferTexture3DEXT					!= NULL && 
			glGenFramebuffersEXT						!= NULL &&
			glGenRenderbuffersEXT						!= NULL && 
			glGenerateMipmapEXT							!= NULL && 
			glGetFramebufferAttachmentParameterivEXT	!= NULL && 
			glGetRenderbufferParameterivEXT				!= NULL && 
			glIsFramebufferEXT							!= NULL &&
			glIsRenderbufferEXT							!= NULL && 
			glRenderbufferStorageEXT					!= NULL) {
		return true;
	}
	else {
		return false;
	}


	#endif

#endif

	return true;
}

bool loadBLITextension() {

	#ifdef USE_GLEW
	if(GLEW_EXT_framebuffer_blit)
		return true;
	else
		return false;
	#else
	try { // load additional function for advanced FBO buffer copying
		glBlitFramebufferEXT = (glBlitFramebufferEXTPROC) wglGetProcAddress("glBlitFramebufferEXT");
	}
	catch (...)	{
		return false;
	}
	return glBlitFramebufferEXT!=NULL;
	#endif

}

bool loadSwapExtensions()
{
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	if(wglSwapIntervalEXT == NULL || wglGetSwapIntervalEXT == NULL) {
		return false;
	}

	return true;

}


// =================== LJ - PBO support 18.01.14 ==================
bool loadPBOextensions() 
{

#ifdef USE_PBO_EXTENSIONS

	#ifdef USE_GLEW
	if(glGenBuffersARB)
		return true;
	else
		return false;
	#else
	try { // load extensions for PBO
		glGenBuffers	= (glGenBuffersPROC)(unsigned)wglGetProcAddress("glGenBuffers");
		glDeleteBuffers = (glDeleteBuffersPROC)(unsigned)wglGetProcAddress("glDeleteBuffers");
		glBindBuffer	= (glBindBufferPROC)(unsigned)wglGetProcAddress("glBindBuffer");
		glBufferData	= (glBufferDataPROC)(unsigned)wglGetProcAddress("glBufferData");
		glMapBuffer		= (glMapBufferPROC)(unsigned)wglGetProcAddress("glMapBuffer");
		glUnmapBuffer	= (glUnmapBufferPROC)(unsigned)wglGetProcAddress("glUnmapBuffer");
	}
	catch (...) {
		return false;
	}
	if(glGenBuffers != NULL && glDeleteBuffers != NULL
	&& glBindBuffer != NULL && glBufferData    != NULL
	&& glMapBuffer  != NULL && glUnmapBuffer   != NULL) {
		return true;
	}
	else {
		return false;
	}
	#endif

#endif

	return true;

}

bool InitializeGlew()
{
	#ifdef USE_GLEW
	HGLRC glContext;
	GLenum glew_error;

	// Opengl context is necessary
	glContext = wglGetCurrentContext();
	if(glContext == NULL) {
		return false;
	}
	//
	// Note from Glew : GLEW obtains information on the supported extensions from the graphics driver.
	// Experimental or pre-release drivers, however, might not report every available extension through
	// the standard mechanism, in which case GLEW will report it unsupported. To circumvent this situation,
	// the glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(),
	// which ensures that all extensions with valid entry points will be exposed. 
	//
	glewExperimental = GL_TRUE;
	glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		return false;
	}
	//
	// Glew should have loaded all the extensions and we can check for them
	//
	// http://glew.sourceforge.net/basic.html
	//
	return true;
	#else
	return false;
	#endif
}

//
// Load GL extensions
//
unsigned int loadGLextensions() {
	
	unsigned int caps = 0; // as per elio glextensions

	// printf("spoutGLextensions - loadGLextensions()\n");

	#ifdef USE_GLEW
	InitializeGlew(); // probably needs failure check
	#endif

	// Check for FBO extensions first - no use continuing without them
	if(!loadFBOextensions()) {
		// printf("    loadFBOextensions fail\n");
		return 0;
	}

	// Load wgl interop extensions - return if fail
	if (!loadInteropExtensions()) {
		// printf("    loadInteropExtensions fail\n");
		return 0;
	}

	caps |= GLEXT_SUPPORT_NVINTEROP;
	caps |= GLEXT_SUPPORT_FBO;

	// Load PBO extension and FBO blit extension
	if(loadBLITextension()) {
		caps |= GLEXT_SUPPORT_FBO_BLIT;
	}

	if(loadSwapExtensions()) {
		caps |= GLEXT_SUPPORT_SWAP;
	}

	if(loadPBOextensions()) {
		caps |= GLEXT_SUPPORT_PBO;
	}

	return caps;

}
