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
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
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
	glfwTerminate();
}
