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

#include "Utils.h"

Utils::Utils( const char *pLogTitle, const char *pLogFile )
{
	// Initialize logger
	std::vector< spdlog::sink_ptr > vLogSinks;
	vLogSinks.push_back( std::make_shared< spdlog::sinks::stdout_color_sink_st >() );
	vLogSinks.push_back( std::make_shared< spdlog::sinks::basic_file_sink_st >( pLogFile ) );
	m_pLogger = std::make_shared< spdlog::logger >( pLogTitle, begin( vLogSinks ), end( vLogSinks ) );

	m_pLogger->set_level( spdlog::level::trace );
	m_pLogger->set_pattern( "[%Y-%b-%d %a] [%T %z] [%^%L%$] [%n] %v" );
	m_pLogger->info( "G'day from {}! Logging to: {}", pLogTitle, pLogFile );
}

Utils::~Utils() {}

GLuint Utils::CreateShaderProgram( const wchar_t *pVertexShaderFile, const wchar_t *pFragmentShaderFile )
{
	// Set vertex shader path
	char pathVertexShader[ MAX_PATH ];
	std::wcstombs( pathVertexShader, pVertexShaderFile, MAX_PATH );

	// Set fragment shader path
	char pathFragmentShader[ MAX_PATH ];
	std::wcstombs( pathFragmentShader, pFragmentShaderFile, MAX_PATH );

	// Load shader files from disk
	GLuint nVertexShaderID = LoadShaderFromDisk( GL_VERTEX_SHADER, pathVertexShader );
	GLuint nFragmentShaderID = LoadShaderFromDisk( GL_FRAGMENT_SHADER, pathFragmentShader );

	if ( nVertexShaderID == 0 || nFragmentShaderID == 0 )
		return 0;

	// Create a shader program
	GLuint nShaderVisMask = glCreateProgram();
	m_pLogger->info( "Shader program created ({})", nShaderVisMask );

	// Link shader to the program
	m_pLogger->info( "Linking vertex ({}) and fragment ({}) shaders to shader program ({})", nVertexShaderID, nFragmentShaderID, nShaderVisMask );

	glAttachShader( nShaderVisMask, nVertexShaderID );
	glAttachShader( nShaderVisMask, nFragmentShaderID );
	glLinkProgram( nShaderVisMask );

	// Verify shader program
	GLint nGLCallResult = GL_FALSE;
	int nReturnLogLength;

	glGetProgramiv( nShaderVisMask, GL_LINK_STATUS, &nGLCallResult );
	glGetProgramiv( nShaderVisMask, GL_INFO_LOG_LENGTH, &nReturnLogLength );

	if ( nReturnLogLength > 0 )
	{
		char eMessage[ MAX_STRING_LEN ];
		glGetProgramInfoLog( nShaderVisMask, nReturnLogLength, NULL, &eMessage[ 0 ] );
		m_pLogger->error(
			"Unable to link shader program ({}) to vertex ({}) and fragment ({}) shaders. {}", nShaderVisMask, nVertexShaderID, nFragmentShaderID, eMessage );

		return 0;
	}

	// Cleanup
	glDetachShader( nShaderVisMask, nVertexShaderID );
	glDetachShader( nShaderVisMask, nFragmentShaderID );

	glDeleteShader( nVertexShaderID );
	glDeleteShader( nFragmentShaderID );

	// Return shader program id
	m_pLogger->info(
		"Shader program ({}) succesfully created and linked to the vertex ({}) and fragment ({}) shaders", nShaderVisMask, nVertexShaderID, nFragmentShaderID );
	return nShaderVisMask;
}

GLuint Utils::LoadShaderFromDisk( GLenum eShaderType, const char *pFilePath )
{
	GLuint nShaderID = glCreateShader( eShaderType );

	// Read shader from disk
	m_pLogger->info( "Reading shader file {}", pFilePath );

	std::string sShaderCode;
	std::ifstream inShaderStream( pFilePath, std::ios::in );

	if ( !inShaderStream.is_open() )
	{
		m_pLogger->error( "Can't open shader file {}", pFilePath );
		return 0;
	}

	std::stringstream sstr;
	sstr << inShaderStream.rdbuf();
	sShaderCode = sstr.str();
	inShaderStream.close();

	m_pLogger->info( "Shader file retrieved from disk ({})", pFilePath );

	// Compile shader
	m_pLogger->info( "Compiling shader" );

	char const *pShaderSrcPtr = sShaderCode.c_str();
	glShaderSource( nShaderID, 1, &pShaderSrcPtr, NULL );
	glCompileShader( nShaderID );

	// Verify shader
	GLint nGLCallResult = GL_FALSE;
	int nReturnLogLength;

	glGetShaderiv( nShaderID, GL_COMPILE_STATUS, &nGLCallResult );
	glGetShaderiv( nShaderID, GL_INFO_LOG_LENGTH, &nReturnLogLength );

	if ( nReturnLogLength > 0 )
	{
		char eMessage[ MAX_STRING_LEN ];
		glGetShaderInfoLog( nShaderID, nReturnLogLength, NULL, &eMessage[ 0 ] );
		m_pLogger->error( "Unable to compile shader. {}", eMessage );

		return 0;
	}

	m_pLogger->info( "Shader compiled successfully" );
	return nShaderID;
}
