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

#define _CRT_SECURE_NO_WARNINGS 1

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX_STRING_LEN 512

// Logger includes
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// OpenGL includes
#include <glad/glad.h>

// Third party includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb/stb_image.h>

class Utils
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] pLogTitle	The title text that will be displayed on the logs and console window
	/// @param[in] pLogFile		The absolute path and filename of the log file
	Utils( const char *pLogTitle, const char *pLogFile );

	/// Class Destructor
	~Utils();

	/// Creates a shader program from provider vertex and fragment shader files on disk
	/// @param[in] pVertexShaderFile			The absolute path and filename of the vertex shader file (glsl)
	/// @param[in] pFragmentShaderFile			The absolute path and filename of the fragment shader file (glsl)
	/// @return									The OpenGL id of the generated shader program
	GLuint CreateShaderProgram( const wchar_t *pVertexShaderFile, const wchar_t *pFragmentShaderFile );

	// Getter for the logger
	/// @return									The shared pointer to the logger object
	std::shared_ptr< spdlog::logger > GetLogger() { return m_pLogger; }

  private:
	// ** FUNCTIONS (PRIVATE) **/

	/// Loads and compiles a GLSL shader file from the disk
	/// @param[in] eShaderType	Fragment or Vertex shader
	/// @param[in] pFilePath	The absolute path and filename of the shader file to load and compile
	/// @return					The shader id of the shader file. 0 if any errors were encountered
	GLuint LoadShaderFromDisk( GLenum eShaderType, const char *pFilePath );


	// ** MEMBER VARIABLES (PRIVATE) **/

	/// The logger object
	std::shared_ptr< spdlog::logger > m_pLogger;
};
