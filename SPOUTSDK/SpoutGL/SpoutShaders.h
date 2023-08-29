/*

				SpoutShaders.h

		Functions to manage compute shaders

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Copyright (c) 2016-2023, Lynn Jarvis. All rights reserved.

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
#pragma once
#ifndef __spoutShaders__
#define __spoutShaders__

#include <windows.h>
#include "SpoutGLextensions.h"
#include "SpoutCommon.h"
#include "SpoutUtils.h"

using namespace spoututils;

class SPOUT_DLLEXP spoutShaders {

	public:

		spoutShaders();
		~spoutShaders();

		// Texture copy
		bool Copy(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height,
			bool bInvert = false, bool swap = false);

		// Flip image in place
		bool Flip(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap = false);

		// Mirror image in place
		bool Mirror(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap = false);

		// Swap RGBA <> BGRA
		bool Swap(GLuint SourceID, unsigned int width, unsigned int height);

		// Image adjust - brightness, contrast, saturation, gamma
		bool Adjust(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float brightness, float contrast, 
			float saturation, float gamma);

		// Unsharp mask sharpen
		bool Sharpen(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float sharpenWidth, float sharpenStrength);

		// Gaussian blur
		bool Blur(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height, float amount);

		// Kuwahara
		bool Kuwahara(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height, float amount);

		GLuint m_copyProgram    = 0;
		GLuint m_flipProgram    = 0;
		GLuint m_mirrorProgram  = 0;
		GLuint m_swapProgram    = 0;

		GLuint m_brcosaProgram  = 0;
		float m_brightness      = 0.0f; // -1 > 1
		float m_contrast        = 1.0f; //  0 > 1
		float m_saturation      = 1.0f; //  0 > 1
		float m_gamma           = 1.0f; //  0 > 1

		GLuint m_sharpenProgram = 0;
		float m_sharpWidth      = 1.0f; // 1=3x3, 2=5x5, 3=7x7
		float m_sharpStrength   = 1.0f; // 1 > 3 typical

		GLuint m_hBlurProgram   = 0;
		GLuint m_vBlurProgram   = 0;
		float m_blurAmount      = 0.0f; // 1 > 4 typical

		GLuint m_kuwaharaProgram = 0;
		float m_kuwaharaAmount   = 0.0f; // 1 > 4 typical

	protected :

		bool ComputeShader(std::string shader, GLuint &program, 
			GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float uniform0 = -1.0, float uniform1 = -1.0,
			float uniform2 = -1.0, float uniform3 = -1.0);
		GLuint CreateComputeShader(std::string shader, unsigned int nWgX, unsigned int nWgY);
		std::string GetFileString(const char* filepath);

		//
		// Shader source
		//

		//
		// Texture copy
		//
		std::string m_copystr = "layout(rgba8, binding=0) uniform readonly image2D src;\n"
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
			"layout (location = 0) uniform bool flip;\n"
			"layout (location = 1) uniform bool swap;\n"
		"void main() {\n"
			"vec4 c = imageLoad(src, ivec2(gl_GlobalInvocationID.xy));\n"
			"uint ypos = gl_GlobalInvocationID.y;\n"
			"if(flip) ypos = imageSize(src).y-ypos;\n" // Flip image option
			// Texture copy with output alpha = 1
			"if(swap) {\n" // Swap RGBA<>BGRA option
			"    imageStore(dst, ivec2(gl_GlobalInvocationID.x, ypos), vec4(c.b,c.g,c.r,c.a));\n"
			"}\n"
			"else {\n"
			"    imageStore(dst, ivec2(gl_GlobalInvocationID.x, ypos), vec4(c.r,c.g,c.b,c.a));\n"
			"}\n"
		"}";

		//
		// Flip in place
		//
		std::string m_flipstr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout (location = 0) uniform bool swap;\n"
		"void main() {\n"
			"if(gl_GlobalInvocationID.y > imageSize(src).y/2)\n" // Half image
			"    return;\n"
			"uint ypos = imageSize(src).y-gl_GlobalInvocationID.y;\n" // Flip y position
			"vec4 c0 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy));\n" // This pixel
			"vec4 c1 = imageLoad(src, ivec2(gl_GlobalInvocationID.x, ypos));\n" // Flip pixel
			"if (swap) {\n" // Swap RGBA<>BGRA option
				"c0 = vec4(c0.b, c0.g, c0.r, c0.a);\n"
				"c1 = vec4(c1.b, c1.g, c1.r, c1.a);\n"
			"}\n"
			"imageStore(src, ivec2(gl_GlobalInvocationID.x, ypos), c0);\n" // Move this pixel to flip position
			"imageStore(src, ivec2(gl_GlobalInvocationID.xy), c1);\n"  // Move flip pixel to this position
		"}";

		//
		// Mirror in place
		//
		std::string m_mirrorstr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout (location = 0) uniform bool swap;\n"
		"void main() {\n"
			"if(gl_GlobalInvocationID.x > imageSize(src).x/2)\n"
			"    return;\n"
			"uint xpos = imageSize(src).x-gl_GlobalInvocationID.x;\n"
			"vec4 c0 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy));\n"
			"vec4 c1 = imageLoad(src, ivec2(xpos, gl_GlobalInvocationID.y));\n"
			"if (swap) {\n"
				"c0 = vec4(c0.b, c0.g, c0.r, c0.a);\n"
				"c1 = vec4(c1.b, c1.g, c1.r, c1.a);\n"
			"}\n"
			"imageStore(src, ivec2(xpos, gl_GlobalInvocationID.y), c0);\n"
			"imageStore(src, ivec2(gl_GlobalInvocationID.xy), c1);\n"
		"}";

		//
		// Swap RGBA <> BGRA
		//
		std::string m_swapstr = "layout(rgba8, binding=0) uniform image2D src;\n"
		"void main() {\n"
			"vec4 c0 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy));\n"
			"imageStore(src, ivec2(gl_GlobalInvocationID.xy), vec4(c0.b, c0.g, c0.r, c0.a));\n" 
		"}";

		//
		// Adjust - brightness, contrast, saturation, gamma
		//
		std::string m_brcosastr = "layout(rgba8, binding=0) uniform image2D src;\n" // Read/Write
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n" // Write only
			"layout(location = 0) uniform float brightness;\n"
			"layout(location = 1) uniform float contrast;\n"
			"layout(location = 2) uniform float saturation;\n"
			"layout(location = 3) uniform float gamma;\n"
			"\n"
		"void main() {\n"
			"\n"
			"vec4 c1 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy)); // rgba\n"
			"\n"
			// Gamma (0 > 10) default 1
			"vec3 c2 = pow(c1.rgb, vec3(1.0 / gamma)); // rgb\n"
			"\n"
			// Saturation (0 > 3) default 1
			"float luminance = dot(c2, vec3(0.2125, 0.7154, 0.0721)); // weights sum to 1\n"
			"c2 = mix(vec3(luminance), c2, vec3(saturation));\n"
			"\n"
			// Contrast (0 > 2) default
			"c2 = (c2 - 0.5) * contrast + 0.5;\n"
			"\n"
			// Brightness (-1 > 1) default 0
			"c2 += brightness;\n"
			"\n"
			// Output with original alpha
			"imageStore(dst, ivec2(gl_GlobalInvocationID.xy), vec4(c2, c1.a)); \n"
		"}\n";

		//
		// Sharpen - unsharp mask
		//
		std::string m_sharpenstr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
			"layout(location = 0) uniform float width;\n"
			"layout(location = 1) uniform float strength;\n"
			"\n"
		"void main() {\n"
			"\n"
			// Original pixel
			"vec4 orig = imageLoad(src, ivec2(gl_GlobalInvocationID.xy)); // rgba\n"
			"\n"
			// Get the blur neighbourhood
			"float dx = width;\n"
			"float dy = width;\n"
			"\n"
			"vec4 c1 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(-dx, -dy));\n"
			"vec4 c2 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, -dy));\n"
			"vec4 c3 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(dx, -dy));\n"
			"vec4 c4 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(-dx, 0.0));\n"
			"vec4 c5 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(dx, 0.0));\n"
			"vec4 c6 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(-dx, dy));\n"
			"vec4 c7 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, dy));\n"
			"vec4 c8 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(dx, dy));\n"
			"\n"
			// Gaussian blur filter
			// [ 1, 2, 1 ]
			// [ 2, 4, 2 ]
			// [ 1, 2, 1 ]
			//  c1 c2 c3
			//  c4    c5
			//  c6 c7 c8
			"vec4 blur = ((c1 + c3 + c6 + c8) + 2.0 * (c2 + c4 + c5 + c7) + 4.0 * orig) / 16.0;\n"
			// Subtract the blurred image from the original image
			"vec4 coeff_blur = vec4(strength);\n"
			"vec4 coeff_orig = vec4(1.0) + coeff_blur;\n"
			"vec4 c9 = coeff_orig * orig - coeff_blur * blur;\n"
			"\n"
			// Output
			"imageStore(dst, ivec2(gl_GlobalInvocationID.xy), c9);\n"
		"}\n";

		//
		// Gaussian blur
		// Adapted from Openframeworks "09_gaussianBlurFilter" example
		// https://openframeworks.cc/
		//

		//
		// Horizontal Gaussian blur
		//
		std::string m_hblurstr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n" 
			"layout(location = 0) uniform float amount;\n"
			"\n"
		"void main() {\n"
			"\n"
			"vec4 c1 = 0.000229 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*-4.0, 0.0));\n"
			"vec4 c2 = 0.005977 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*-3.0, 0.0));\n"
			"vec4 c3 = 0.060598 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*-2.0, 0.0));\n"
			"vec4 c4 = 0.241732 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*-1.0, 0.0));\n"
			"vec4 c5 = 0.382928 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, 0.0));\n"
			"vec4 c6 = 0.241732 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*1.0, 0.0));\n"
			"vec4 c7 = 0.060598 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*2.0, 0.0));\n"
			"vec4 c8 = 0.005977 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*3.0, 0.0));\n"
			"vec4 c9 = 0.000229 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(amount*4.0, 0.0));\n"
			"\n"
			// Output
			"imageStore(dst, ivec2(gl_GlobalInvocationID.xy), (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9));\n"
			"\n"
		"}\n";

		//
		// Vertical Gaussian blur
		//
		std::string m_vblurstr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
			"layout(location = 0) uniform float amount;\n"
			"\n"
		"void main() {\n"
			"\n"
			"vec4 c1 = 0.000229 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*-4.0));\n"
			"vec4 c2 = 0.005977 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*-3.0));\n"
			"vec4 c3 = 0.060598 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*-2.0));\n"
			"vec4 c4 = 0.241732 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*-1.0));\n"
			"vec4 c5 = 0.382928 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, 0.0));\n"
			"vec4 c6 = 0.241732 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*1.0));\n"
			"vec4 c7 = 0.060598 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*2.0));\n"
			"vec4 c8 = 0.005977 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*3.0));\n"
			"vec4 c9 = 0.000229 * imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(0.0, amount*4.0));\n"
			"\n"
			// Output
			"imageStore(dst, ivec2(gl_GlobalInvocationID.xy), (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9));\n"
			"\n"
		"}\n";


		//
		// Kuwahara effect
		// Adapted from : Jan Eric Kyprianidis (http://www.kyprianidis.com/)
		//
		std::string m_kuwaharastr = "layout(rgba8, binding=0) uniform image2D src;\n"
			"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
			"layout(location = 0) uniform float radius;\n"
			"\n"
		"void main() {\n"
			"\n"
			"	vec3 m[4];\n"
			"	vec3 s[4];\n"
			"	for (int j = 0; j < 4; ++j) {\n"
			"		m[j] = vec3(0.0);\n"
			"		s[j] = vec3(0.0);\n"
			"	}\n"
			"\n"
			"	vec3 c;\n"
			"	int ir = int(floor(radius));\n"
			"	for (int j = -ir; j <= 0; ++j) {\n"
			"		for (int i = -ir; i <= 0; ++i) {\n"
			"			c = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(i, j)).rgb;\n"
			"			m[0] += c;\n"
			"			s[0] += c * c;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	for (int j = -ir; j <= 0; ++j) {\n"
			"		for (int i = 0; i <= ir; ++i) {\n"
			"			c = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(i, j)).rgb;\n"
			"			m[1] += c;\n"
			"			s[1] += c * c;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	for (int j = 0; j <= ir; ++j) {\n"
			"		for (int i = 0; i <= ir; ++i) {\n"
			"			c = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(i, j)).rgb;\n"
			"			m[2] += c;\n"
			"			s[2] += c * c;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	for (int j = 0; j <= ir; ++j) {\n"
			"		for (int i = -ir; i <= 0; ++i) {\n"
			"			c = imageLoad(src, ivec2(gl_GlobalInvocationID.xy) + ivec2(i, j)).rgb;\n"
			"			m[3] += c;\n"
			"			s[3] += c * c;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	float min_sigma2 = 1e+2;\n"
			"	float n = float((radius+1)*(radius+1));\n"
			"	for (int k = 0; k < 4; ++k) {\n"
			"		m[k] /= n;\n"
			"		s[k] = abs(s[k] / n - m[k] * m[k]);\n"
			"		float sigma2 = s[k].r + s[k].g + s[k].b;\n"
			"		if (sigma2 < min_sigma2) {\n"
			"			min_sigma2 = sigma2;\n"
			"			imageStore(dst, ivec2(gl_GlobalInvocationID.xy), vec4(m[k], 1.0));\n"
			"		}\n"
			"	}\n"
		"}\n";

	// ============================================================

};

#endif
