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
	m_pUtils = new Utils( "XRMirror - OpenGL", sLogFile );

	// Initialize sdl
	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	// Create sdl window
	m_pXRMirror = SDL_CreateWindow( pTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nWidth, nHeight, SDL_WINDOW_OPENGL );
	if ( !m_pXRMirror )
	{
		std::string eMessage = "Failed to create SDL window";
		m_pUtils->GetLogger()->error( "{}", eMessage );

		throw eMessage;
	}

	// Create context
	m_pXRMirrorContext = SDL_GL_CreateContext( m_pXRMirror );
	if ( !m_pXRMirrorContext )
	{
		std::string eMessage = "Failed to create SDL window context";
		m_pUtils->GetLogger()->error( "{}", eMessage );

		throw eMessage;
	}

	// Make context current
	SDL_GL_MakeCurrent( m_pXRMirror, m_pXRMirrorContext );
	m_pUtils->GetLogger()->info( "Window for OpenXR Mirror context created" );

	// Disable vsync
	SDL_GL_SetSwapInterval( 0 );

	// Check glad can be loaded before we use any OpenGL calls
	if ( !gladLoadGLLoader( ( GLADloadproc )SDL_GL_GetProcAddress ) )
	{
		std::string eMessage = "Failed to initialize GLAD";
		m_pUtils->GetLogger()->error( "{}", eMessage );
		throw eMessage;
	}

	// Create mirror
	glViewport( 0, 0, nWidth, nHeight );
	m_pUtils->GetLogger()->info( "Mirror created {}x{}", nWidth, nHeight );
	m_pUtils->GetLogger()->info( "OpenGL ver {}.{} with shading language ver {} ", GLVersion.major, GLVersion.minor, ( char * )glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	m_pUtils->GetLogger()->info( "OpenGL vendor: {}, renderer {}", ( char * )glGetString( GL_VENDOR ), ( char * )glGetString( GL_RENDERER ) );
}

XRMirror::~XRMirror()
{
	delete m_pUtils;
	
	// Delete context
	if ( m_pXRMirrorContext )
		SDL_GL_DeleteContext( m_pXRMirrorContext );

	// Destroy window
	if ( m_pXRMirror )
		SDL_DestroyWindow( m_pXRMirror );

	SDL_Quit();
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
