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

#include <XRMirror.h>

XRMirror::XRMirror( int nWidth, int nHeight, const char *pTitle, const char* sLogFile )
{
	// Setup helper utilities class
	m_pUtils = new Utils( "XRMirror", sLogFile );

	// Initialize glfw
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create glfw window
	m_pXRMirror = glfwCreateWindow( nWidth, nHeight, pTitle, NULL, NULL );

	if ( m_pXRMirror == NULL )
	{
		std::string eMessage = "Failed to create GLFW window";
		m_pUtils->GetLogger()->error( "{}", eMessage );
		glfwTerminate();
		throw eMessage;
	}

	glfwMakeContextCurrent( m_pXRMirror );
	m_pUtils->GetLogger()->info( "Window for OpenXR Mirror context created" );

	// Check glad can be loaded before we use any OpenGL calls
	if ( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
	{
		std::string eMessage = "Failed to initialize GLAD";
		m_pUtils->GetLogger()->error( "{}", eMessage );
		glfwTerminate();
		throw eMessage;
	}

	// Create mirror
	glViewport( 0, 0, nWidth, nHeight );
	m_pUtils->GetLogger()->info( "Mirror created {}x{}", nWidth, nHeight );

	// Set callback for window resizing
	glfwSetFramebufferSizeCallback( m_pXRMirror, []( GLFWwindow *mirror, int width, int height ) { glViewport( 0, 0, width, height ); } );
}

XRMirror::~XRMirror()
{
	delete m_pUtils;
	glfwDestroyWindow( GetWindow() );
	glfwTerminate();
}


unsigned int XRMirror::LoadTexture( const wchar_t* pTextureFile, GLuint nShader, const char *pSamplerParam, GLint nMinFilter, GLint nMagnitudeFilter, GLint nWrapS, GLint nWrapT )
{
	unsigned int nTexture = 0;

	glGenTextures( 1, &nTexture );
	glBindTexture( GL_TEXTURE_2D, nTexture );

	// Set the texture parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// Load image from disk
	int nWidth, nHeight, nChannels;
	char pTexture[ MAX_PATH ] = "";
	std::wcstombs( pTexture, pTextureFile, MAX_PATH );

	stbi_set_flip_vertically_on_load( true );

	unsigned char *textureData = stbi_load( pTexture, &nWidth, &nHeight, &nChannels, 0 );
	if ( textureData )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData );
	else
		m_pUtils->GetLogger()->warn( "Unable to load cube texture from disk ({})", pTexture );

	stbi_image_free( textureData );

	// Set the 2d texture sampler param in the shader
	glUseProgram( nShader );
	glUniform1i( glGetUniformLocation( nShader, pSamplerParam ), 0 );

	return nTexture;
}
