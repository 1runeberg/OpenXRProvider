/* Copyright 2021 Rune Berg (GitHub: https://github.com/1runeberg, Twitter: https://twitter.com/1runeberg)
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 *
 */

#pragma once

#include <map>
#include <Utils.h>

// OpenGL includes
#include <glad/glad.h>

// Windowing includes
#include <glfw3.h>

#include <XRRenderManager.h>

class XRMirror
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] nWidth		The width of the desktop window (XR Mirror)
	/// @param[in] nHeight		The height of the desktop window (XR Mirror)
	/// @param[in] pTitle		The title text that will be displayed on the console window
	/// @param[in] sLogFile		The absolute path and filename of the log file
	XRMirror( int nWidth, int nHeight, const char *pTitle, const char* sLogFile );

	/// Class Destructor
	~XRMirror();

	/// Getter for the GLFW window
	/// @return				The current GLFW desktop window (XR Mirror)
	GLFWwindow* GetWindow() { return m_pXRMirror; }

	/// Load a texture file from disk
	/// @param[in]	pTextureFile		The absolute file to the texture on disk
	/// @param[in]	nShader				The program id for the shader
	/// @param[in]	pSamplerParam		The texture sampler2d parameter name
	/// @param[in]	nMinFilter			(optional: GL_LINEAR)
	/// @param[in]	nMagnitudeFilter	(optional: GL_NEAREST)
	/// @param[in]	nWrapS				(optional: GL_REPEAT)
	/// @param[in]	nWrapT				(optional: GL_REPEAT)
	/// @return		unsigned int		The texture id of the loaded texture
	unsigned int LoadTexture(
		const wchar_t* pTextureFile, 
		GLuint nShader,
		const char* pSamplerParam,
		GLint nMinFilter = GL_LINEAR,
		GLint nMagnitudeFilter = GL_NEAREST,
		GLint nWrapS = GL_REPEAT,
		GLint nWrapT = GL_REPEAT);

private:
	// ** MEMBER VARIABLES (PRIVATE) **/

	/// Pointer to the helper utilities log (logger lives here)
	Utils *m_pUtils;

	/// Pointer to the GLFW desktop window (XR Mirror)
	GLFWwindow *m_pXRMirror;

};
