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

#include <XRMirror_GL.h>

XRMirror_GL::XRMirror_GL( SandboxCommon *pCommon, int nWidth, int nHeight, const char *pTitle, std::wstring sCurrentPath, const char *sLogFile )
	: m_pCommon( pCommon )
	, m_nScreenWidth( nWidth )
	, m_nScreenHeight( nHeight )
	, m_sCurrentPath( sCurrentPath )
{	
	// Initialize logger
	std::vector< spdlog::sink_ptr > vLogSinks;
	vLogSinks.push_back( std::make_shared< spdlog::sinks::stdout_color_sink_st >() );
	vLogSinks.push_back( std::make_shared< spdlog::sinks::basic_file_sink_st >( sLogFile ) );
	m_pLogger = std::make_shared< spdlog::logger >( "XRMirror - OpenGL", begin( vLogSinks ), end( vLogSinks ) );

	m_pLogger->set_level( spdlog::level::trace );
	m_pLogger->set_pattern( "[%Y-%b-%d %a] [%T %z] [%^%L%$] [%n] %v" );
	m_pLogger->info( "G'day from {}! Logging to: {}", "XRMirror - OpenGL", sLogFile );

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
		m_pLogger->error( "{}", eMessage );

		throw eMessage;
	}

	// Create context
	m_pXRMirrorContext = SDL_GL_CreateContext( m_pXRMirror );
	if ( !m_pXRMirrorContext )
	{
		std::string eMessage = "Failed to create SDL window context";
		m_pLogger->error( "{}", eMessage );

		throw eMessage;
	}

	// Make context current
	SDL_GL_MakeCurrent( m_pXRMirror, m_pXRMirrorContext );
	m_pLogger->info( "Window for OpenXR Mirror context created" );

	// Disable vsync
	SDL_GL_SetSwapInterval( 0 );

	// Check glad can be loaded before we use any OpenGL calls
	if ( !gladLoadGLLoader( ( GLADloadproc )SDL_GL_GetProcAddress ) )
	{
		std::string eMessage = "Failed to initialize GLAD";
		m_pLogger->error( "{}", eMessage );
		throw eMessage;
	}

	// Create mirror
	glViewport( 0, 0, nWidth, nHeight );
	m_pLogger->info( "Mirror created {}x{}", nWidth, nHeight );
	m_pLogger->info( "OpenGL ver {}.{} with shading language ver {} ", GLVersion.major, GLVersion.minor, ( char * )glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	m_pLogger->info( "OpenGL vendor: {}, renderer {}", ( char * )glGetString( GL_VENDOR ), ( char * )glGetString( GL_RENDERER ) );
}

XRMirror_GL::~XRMirror_GL()
{
	// Delete logger
	if ( m_pLogger )
		spdlog::drop( "XRMirror - OpenGL" );

	// Delete context
	if ( m_pXRMirrorContext )
		SDL_GL_DeleteContext( m_pXRMirrorContext );

	// Destroy window
	if ( m_pXRMirror )
		SDL_DestroyWindow( m_pXRMirror );

	SDL_Quit();
}

unsigned int XRMirror_GL::LoadTexture( const wchar_t* pTextureFile, GLuint nShader, const char *pSamplerParam, GLint nMinFilter, GLint nMagnitudeFilter, GLint nWrapS, GLint nWrapT )
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
		m_pLogger->warn( "Unable to load cube texture from disk ({})", pTexture );

	stbi_image_free( textureData );

	// Set the 2d texture sampler param in the shader
	glUseProgram( nShader );
	glUniform1i( glGetUniformLocation( nShader, pSamplerParam ), 0 );

	return nTexture;
}

int XRMirror_GL::Init( OpenXRProvider::XRRender *pRender )
{
	assert( pRender );
	m_pXRRender = pRender;

	// Enable depth buffer testing
	glFrontFace( GL_CW );
	glEnable( GL_DEPTH_TEST );

	// Setup vertex buffer object (cube)
	glGenBuffers( 1, &cubeVBO );

	glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_pCommon->vCube ), m_pCommon->vCube, GL_STATIC_DRAW );

	// Setup element buffer object (cube)
	// glGenBuffers(1, &cubeEBO);

	// Setup vertex array object (cube)
	glGenVertexArrays( 1, &cubeVAO );
	glBindVertexArray( cubeVAO );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )0 );
	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );

	// Setup instanced data for sea of cubes
	glGenBuffers( 1, &cubeInstanceDataVBO );
	glBindBuffer( GL_ARRAY_BUFFER, cubeInstanceDataVBO );

	for ( int i = 0; i < 4; ++i )
	{
		glEnableVertexAttribArray( 2 + i );
		glVertexAttribPointer( 2 + i, 4, GL_FLOAT, GL_FALSE, sizeof( glm::mat4 ), ( const GLvoid * )( sizeof( GLfloat ) * i * 4 ) );
		glVertexAttribDivisor( 2 + i, 1 );
	}
	glBindVertexArray( 0 );

	// Setup vertex buffer object (controller)
	glGenBuffers( 1, &controllerVBO );

	glBindBuffer( GL_ARRAY_BUFFER, controllerVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_pCommon->vControllerMesh ), m_pCommon->vControllerMesh, GL_STATIC_DRAW );

	// Setup vertex array object (controller)
	glGenVertexArrays( 1, &controllerVAO );
	glBindVertexArray( controllerVAO );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )0 );
	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );

	// Setup instanced data for the controller meshes
	glGenBuffers( 1, &controllerInstanceDataVBO );
	glBindBuffer( GL_ARRAY_BUFFER, controllerInstanceDataVBO );

	for ( int i = 0; i < 4; ++i )
	{
		glEnableVertexAttribArray( 2 + i );
		glVertexAttribPointer( 2 + i, 4, GL_FLOAT, GL_FALSE, sizeof( glm::mat4 ), ( const GLvoid * )( sizeof( GLfloat ) * i * 4 ) );
		glVertexAttribDivisor( 2 + i, 1 );
	}
	glBindVertexArray( 0 );

	// Setup vertex buffer object (joint)
	glGenBuffers( 1, &jointVBO );
	glBindBuffer( GL_ARRAY_BUFFER, jointVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_pCommon->vJointMesh ), m_pCommon->vJointMesh, GL_STATIC_DRAW );

	// Setup vertex array object (joint)
	glGenVertexArrays( 1, &jointVAO );
	glBindVertexArray( jointVAO );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )0 );
	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );

	// Setup instanced data for the hand joints
	glGenBuffers( 1, &jointInstanceDataVBO );
	glBindBuffer( GL_ARRAY_BUFFER, jointInstanceDataVBO );

	for ( int i = 0; i < 4; ++i )
	{
		glEnableVertexAttribArray( 2 + i );
		glVertexAttribPointer( 2 + i, 4, GL_FLOAT, GL_FALSE, sizeof( glm::mat4 ), ( const GLvoid * )( sizeof( GLfloat ) * i * 4 ) );
		glVertexAttribDivisor( 2 + i, 1 );
	}
	glBindVertexArray( 0 );

	// Setup frame buffer object
	glGenFramebuffers( 1, &FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, FBO );

	// Create shader programs
	nShaderVisMask = CreateShaderProgram( ( m_sCurrentPath + VIS_MASK_VERTEX_SHADER ).c_str(), ( m_sCurrentPath + VIS_MASK_FRAGMENT_SHADER ).c_str() );
	nShaderLit = CreateShaderProgram( ( m_sCurrentPath + LIT_VERTEX_SHADER ).c_str(), ( m_sCurrentPath + LIT_FRAGMENT_SHADER ).c_str() );
	nShaderUnlit =CreateShaderProgram( ( m_sCurrentPath + UNLIT_VERTEX_SHADER ).c_str(), ( m_sCurrentPath + UNLIT_FRAGMENT_SHADER ).c_str() );
	nShaderTextured = CreateShaderProgram( ( m_sCurrentPath + TEXTURED_VERTEX_SHADER ).c_str(), ( m_sCurrentPath + TEXTURED_FRAGMENT_SHADER ).c_str() );

	if ( nShaderVisMask == 0 || nShaderLit == 0 || nShaderUnlit == 0 || nShaderTextured == 0 )
		return -1;

	// Set fragment shader colors
	glUseProgram( nShaderLit );
	glUniform3f( glGetUniformLocation( nShaderLit, "surfaceColor" ), 1.0f, 1.0f, 0.0f );
	glUniform3f( glGetUniformLocation( nShaderLit, "lightColor" ), 1.0f, 1.0f, 1.0f );

	glUseProgram( nShaderUnlit );
	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 1.0f, 1.0f, 1.0f );

	// Load textures for sea of cubes
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_bellevue_valve.png" ).c_str(), nShaderTextured, "texSample" ) );
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_munich_mein_schatz.png" ).c_str(), nShaderTextured, "texSample" ) );
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_hobart_mein_heim.png" ).c_str(), nShaderTextured, "texSample" ) );
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_hobart_rose.png" ).c_str(), nShaderTextured, "texSample" ) );
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_hobart_mein_kochen.png" ).c_str(), nShaderTextured, "texSample" ) );
	m_vCubeTextures.push_back( LoadTexture( ( m_sCurrentPath + L"\\img\\t_hobart_sunset.png" ).c_str(), nShaderTextured, "texSample" ) );

	return 0;
}

void XRMirror_GL::BlitToWindow()
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	glViewport( 0, 0, m_pXRRender->GetTextureWidth(), m_pXRRender->GetTextureHeight() );
	glBlitFramebuffer( 0, 0, m_pXRRender->GetTextureWidth(), m_pXRRender->GetTextureHeight(), 0, 0, m_nScreenWidth, m_nScreenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
}

void XRMirror_GL::Clear( glm::vec4 v4ClearColor ) 
{
	glClearColor( v4ClearColor.r, v4ClearColor.g, v4ClearColor.b, v4ClearColor.a );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

void XRMirror_GL::DrawFrame( SandboxCommon::ESandboxScene eCurrentScene, OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex )
{
	glBindFramebuffer( GL_FRAMEBUFFER, FBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pXRRender->GetGraphicsAPI()->GetTexture2D( eEye, nSwapchainIndex ), 0 );

	Clear( m_v4ClearColor );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	// Check if hmd is tracking
	if ( !m_pXRRender->GetHMDState()->IsPositionTracked || !m_pXRRender->GetHMDState()->IsPositionTracked )
		return;

	// Draw vismask on XR Mirror
	uint32_t nVertCount = ( eEye == OpenXRProvider::EYE_LEFT ) ? ( uint32_t )m_pCommon->MaskVertices_Left()->size() : ( uint32_t )m_pCommon->MaskVertices_Right()->size();
	float *vMask = ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->MaskVertices_Left()->data() : m_pCommon->MaskVertices_Left()->data();

	if ( nVertCount > 0 )
	{
		// TO DO: Apply vismask, set clip values
	}

	// Draw current active scene
	switch ( eCurrentScene )
	{
		case SandboxCommon::SANDBOX_SCENE_HAND_TRACKING:
			DrawHandTrackingScene( eEye, nSwapchainIndex );
			break;

		case SandboxCommon::SANDBOX_SCENE_SEA_OF_CUBES:
			glm::vec3 cubeScale = glm::vec3( 0.5f, 0.5f, 0.5f );
			DrawSeaOfCubesScene( eEye, nSwapchainIndex, cubeScale, 1.5f, 1.5f );
		default:
			break;
	}
}

void XRMirror_GL::DrawSeaOfCubesScene( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex, glm::vec3 cubeScale, float fSpacingPlane, float fSpacingHeight )
{
	assert( m_pXRRender );
	assert( m_vCubeTextures.size() > 0 );

	// Set eye projections if we haven't already
	if ( !m_pCommon->EyeProjectionsSet() )
	{
		m_pCommon->EyeProjectionLeft( m_pCommon->GetEyeProjection( m_pXRRender->GetHMDState()->LeftEye.FoV, 0.1f, 100.f ) );
		m_pCommon->EyeProjectionRight( m_pCommon->GetEyeProjection( m_pXRRender->GetHMDState()->RightEye.FoV, 0.1f, 100.f ) );
		m_pCommon->EyeProjectionsSet( true );
	}

	// Get eye view for this frame
	XrPosef eyePose = ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pXRRender->GetHMDState()->LeftEye.Pose : m_pXRRender->GetHMDState()->RightEye.Pose;

	XrMatrix4x4f xrEyeView;
	XrVector3f xrScale { 1.0, 1.0, 1.0f };
	XrMatrix4x4f_CreateTranslationRotationScale( &xrEyeView, &eyePose.position, &eyePose.orientation, &xrScale );

	glm::mat4 eyeView = glm::make_mat4( xrEyeView.m );
	glm::mat4 eyeViewInverted = m_pCommon->InvertMatrix( eyeView );

	// Generate sea of cubes
	uint32_t nTextureCount = ( uint32_t )m_vCubeTextures.size();
	float nStartXZ = ( nTextureCount / 2 ) * fSpacingPlane;
	glm::vec3 startPosition = glm::vec3( -nStartXZ, fSpacingHeight / 4, nStartXZ );

	// Bottom to top
	uint32_t nCubeIndex = 0;
	glm::mat4 *vEyeProjections = new glm::mat4[ 36 ]; // max number of cubes in a single plane

	for ( int i = 0; i < nTextureCount; ++i )
	{
		// Back to front
		float z = startPosition.z;
		for ( int j = 0; j < nTextureCount; ++j )
		{
			float x = startPosition.x;
			z -= fSpacingPlane;

			// Left to Right
			for ( int k = 0; k < nTextureCount; ++k )
			{
				x += fSpacingPlane;

				FillEyeMVP( vEyeProjections, eyeViewInverted, eEye, nCubeIndex, glm::vec3( x, startPosition.y, z ), cubeScale );

				++nCubeIndex;
			}
		}

		// Set depth texture
		const uint32_t depthTexture = GetDepth( m_pXRRender->GetGraphicsAPI()->GetTexture2D( eEye, nSwapchainIndex ) );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 );

		// Set shader
		glUseProgram( nShaderTextured );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, m_vCubeTextures[ i ] );

		// Draw cube instance in a single plane at once
		glBindBuffer( GL_ARRAY_BUFFER, cubeInstanceDataVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ) * 36, vEyeProjections, GL_STATIC_DRAW );

		glBindVertexArray( cubeVAO );
		glDrawArraysInstanced( GL_TRIANGLES, 0, 36, 36 );

		// Go to the next cube plane up
		startPosition.y += fSpacingHeight;
		nCubeIndex = 0;

		// Draw Controllers
		DrawControllers( eEye, eyeViewInverted );

		// Draw hand joints
		DrawHandJoints( eEye, eyeViewInverted );
	}

	delete[] vEyeProjections;
}

void XRMirror_GL::DrawHandTrackingScene( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex )
{
	assert( m_pXRRender );
	assert( m_vCubeTextures.size() > 3 ); // We'll draw four large cubes in the scene

	// Set eye projections if we haven't already
	if ( !m_pCommon->EyeProjectionsSet() )
	{
		m_pCommon->EyeProjectionLeft( m_pCommon->GetEyeProjection( m_pXRRender->GetHMDState()->LeftEye.FoV, 0.1f, 100.f ) );
		m_pCommon->EyeProjectionRight( m_pCommon->GetEyeProjection( m_pXRRender->GetHMDState()->RightEye.FoV, 0.1f, 100.f ) );
		m_pCommon->EyeProjectionsSet( true );
	}

	// Get eye view for this frame
	XrPosef eyePose = ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pXRRender->GetHMDState()->LeftEye.Pose : m_pXRRender->GetHMDState()->RightEye.Pose;

	glm::mat4 eyeView( 1.0f );
	glm::quat eyeRotation( eyePose.orientation.w, eyePose.orientation.x, eyePose.orientation.y, eyePose.orientation.z );
	eyeView = glm::translate( eyeView, glm::vec3( eyePose.position.x, eyePose.position.y, eyePose.position.z ) );
	eyeView = glm::rotate( eyeView, angle( eyeRotation ), axis( eyeRotation ) );

	glm::mat4 eyeViewInverted = m_pCommon->InvertMatrix( eyeView );

	// Generate four rotating cubes in scene
	glm::vec3 vFourCubePositions[ 4 ] = {
		// OpenXR Coords: +x Right, +y Up, +z Back
		glm::vec3( 0.f, 1.5f, 3.0f ),  // North	(t_bellevue_valve.png)
		glm::vec3( 0.f, 1.5f, -3.0f ), // South	(t_munich_mein_schatz.png)
		glm::vec3( 3.f, 1.5f, 0.f ),   // East		(t_hobart_mein_heim.png)
		glm::vec3( -3.f, 1.5f, 0.f )   // West		(t_hobart_rose.png)
	};

	for ( int i = 0; i < 4; ++i )
	{
		DrawCube( eEye, nSwapchainIndex, eyeViewInverted, m_vCubeTextures[ i ], vFourCubePositions[ i ], glm::vec3( 1.0f ), glm::vec3( 0.5f, 1.0f, 0.0f ) );
	}

	// Draw controller meshes
	DrawControllers( eEye, eyeViewInverted );

	// Generate joint meshes for both hands
	DrawHandJoints( eEye, eyeViewInverted );
}

void XRMirror_GL::DrawCube(
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	glm::mat4 eyeView,
	unsigned int nTexture,
	glm::vec3 cubePosition,
	glm::vec3 cubeScale,
	glm::vec3 cubeRotationOverTime )
{
	assert( m_pXRRender );

	// Set cube texture
	glUseProgram( nShaderTextured );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, nTexture );

	// Calculate mvp
	glm::mat4 *vEyeProjection = new glm::mat4[ 1 ]; // just one instance of this cube

	glm::mat4 cubeModel = glm::mat4( 1.0f );
	cubeModel = glm::translate( cubeModel, cubePosition );
	cubeModel = glm::rotate( cubeModel, ( float )SDL_GetTicks() / 1000, cubeRotationOverTime );
	cubeModel = glm::scale( cubeModel, cubeScale );

	// Get eye pose
	XrPosef eyePose = ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pXRRender->GetHMDState()->LeftEye.Pose : m_pXRRender->GetHMDState()->RightEye.Pose;

	vEyeProjection[ 0 ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * cubeModel );

	// Draw cube
	glBindBuffer( GL_ARRAY_BUFFER, cubeInstanceDataVBO );
	glBindVertexArray( cubeVAO );

	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ) * 1, vEyeProjection, GL_STATIC_DRAW );
	glDrawArraysInstanced( GL_TRIANGLES, 0, 36, 1 );

	// Clean up
	delete[] vEyeProjection;
}

void XRMirror_GL::DrawControllers( OpenXRProvider::EXREye eEye, glm::mat4 eyeView )
{
	assert( m_pXRRender );

	// Setup model views
	XrVector3f xrScale { 0.15f, 0.15f, 0.15f };

	XrMatrix4x4f xrModelView_L;
	XrMatrix4x4f_CreateTranslationRotationScale( &xrModelView_L, &m_pCommon->XrLocation_Left()->pose.position, &m_pCommon->XrLocation_Left()->pose.orientation, &xrScale );

	XrMatrix4x4f xrModelView_R;
	XrMatrix4x4f_CreateTranslationRotationScale( &xrModelView_R, &m_pCommon->XrLocation_Right()->pose.position, &m_pCommon->XrLocation_Right()->pose.orientation, &xrScale );

	glm::mat4 controllerModel_L = glm::make_mat4( xrModelView_L.m );
	glm::mat4 controllerModel_R = glm::make_mat4( xrModelView_R.m );

	// Setup eye projections
	glm::mat4 *vEyeProjections_LeftHand = new glm::mat4[ 1 ];  // max number of meshes in a single hand
	glm::mat4 *vEyeProjections_RightHand = new glm::mat4[ 1 ]; // max number of meshes in a single hand

	vEyeProjections_LeftHand[ 0 ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * controllerModel_L );
	vEyeProjections_RightHand[ 0 ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * controllerModel_R );

	// Set shader
	glUseProgram( nShaderUnlit );

	// Draw controller mesh
	glBindBuffer( GL_ARRAY_BUFFER, controllerInstanceDataVBO );
	glBindVertexArray( controllerVAO );

	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 0.1f, 0.1f, 1.0f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ), vEyeProjections_LeftHand, GL_STATIC_DRAW );
	glDrawArraysInstanced( GL_TRIANGLES, 0, 24, 1 );

	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 1.0f, 0.1f, 0.1f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ), vEyeProjections_RightHand, GL_STATIC_DRAW );
	glDrawArraysInstanced( GL_TRIANGLES, 0, 24, 1 );

	// Clean up
	delete[] vEyeProjections_LeftHand;
	delete[] vEyeProjections_RightHand;
}

void XRMirror_GL::DrawHandJoints( OpenXRProvider::EXREye eEye, glm::mat4 eyeView )
{
	if ( !m_pCommon->ShouldDrawHandJoints() )
		return;

	XrHandJointLocationsEXT *xrHandJoints_L = m_pCommon->XRHandTracking()->GetHandJointLocations( XR_HAND_LEFT_EXT );
	XrHandJointLocationsEXT *xrHandJoints_R = m_pCommon->XRHandTracking()->GetHandJointLocations( XR_HAND_RIGHT_EXT );

	uint32_t nMeshIndex = 0;
	glm::mat4 *vEyeProjections_LeftHand = new glm::mat4[ 26 ];	// max number of joint meshes in a single hand
	glm::mat4 *vEyeProjections_RightHand = new glm::mat4[ 26 ]; // max number of joint meshes in a single hand

	for ( int i = 0; i < XR_HAND_JOINT_COUNT_EXT; ++i )
	{
		// Fill eye mvp for left hand joint meshes
		XrMatrix4x4f xrModelView_L;
		float fScale = xrHandJoints_L->jointLocations[ i ].radius * 1.5f;
		XrVector3f xrScale_L { fScale, fScale, fScale };
		XrMatrix4x4f_CreateTranslationRotationScale( &xrModelView_L, &xrHandJoints_L->jointLocations[ i ].pose.position, &xrHandJoints_L->jointLocations[ i ].pose.orientation, &xrScale_L );

		glm::mat4 jointModel_L = glm::make_mat4( xrModelView_L.m );

		vEyeProjections_LeftHand[ nMeshIndex ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * jointModel_L );

		// Fill eye mvp for right hand joint meshes
		XrMatrix4x4f xrModelView_R;
		fScale = xrHandJoints_R->jointLocations[ i ].radius * 1.5f;
		XrVector3f xrScale_R { fScale, fScale, fScale };
		XrMatrix4x4f_CreateTranslationRotationScale( &xrModelView_R, &xrHandJoints_R->jointLocations[ i ].pose.position, &xrHandJoints_R->jointLocations[ i ].pose.orientation, &xrScale_R );

		glm::mat4 jointModel_R = glm::make_mat4( xrModelView_R.m );

		vEyeProjections_RightHand[ nMeshIndex ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * jointModel_R );

		++nMeshIndex;
	}

	// Set shader
	glUseProgram( nShaderUnlit );

	// Draw joint mesh instances
	glBindBuffer( GL_ARRAY_BUFFER, jointInstanceDataVBO );
	glBindVertexArray( jointVAO );

	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 0.1f, 0.1f, 1.0f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ) * 26, vEyeProjections_LeftHand, GL_STATIC_DRAW );
	glDrawArraysInstanced( GL_TRIANGLES, 0, 24, 26 );

	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 1.0f, 0.1f, 0.1f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ) * 26, vEyeProjections_RightHand, GL_STATIC_DRAW );
	glDrawArraysInstanced( GL_TRIANGLES, 0, 24, 26 );

	// Clean up
	delete[] vEyeProjections_LeftHand;
	delete[] vEyeProjections_RightHand;
}

void XRMirror_GL::FillEyeMVP( glm::mat4 *vEyeProjections, glm::mat4 eyeView, OpenXRProvider::EXREye eEye, uint32_t nCubeIndex, glm::vec3 cubePosition, glm::vec3 cubeScale )
{
	glm::mat4 cubeModel = glm::mat4( 1.0f );
	cubeModel = glm::translate( cubeModel, cubePosition );
	cubeModel = glm::scale( cubeModel, cubeScale );

	vEyeProjections[ nCubeIndex ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * cubeModel );
}

void XRMirror_GL::FillEyeMVP_RotateOverTime(
	glm::mat4 *vEyeProjections,
	glm::mat4 eyeView,
	OpenXRProvider::EXREye eEye,
	uint32_t nCubeIndex,
	unsigned nTexture,
	glm::vec3 cubePosition,
	glm::vec3 cubeRotation,
	glm::vec3 cubeScale )
{
	if ( nTexture > 0 )
	{
		glUseProgram( nShaderTextured );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, nTexture );
	}

	glm::mat4 cubeModel = glm::mat4( 1.0f );
	cubeModel = glm::translate( cubeModel, cubePosition );
	cubeModel = glm::rotate( cubeModel, ( float )SDL_GetTicks() / 1000, cubeRotation );
	cubeModel = glm::scale( cubeModel, cubeScale );

	vEyeProjections[ nCubeIndex ] = ( ( ( eEye == OpenXRProvider::EYE_LEFT ) ? m_pCommon->EyeProjectionLeft() : m_pCommon->EyeProjectionRight() ) * eyeView * cubeModel );
}

uint32_t XRMirror_GL::GetDepth( uint32_t nTexture, GLint nMinFilter, GLint nMagnitudeFilter, GLint nWrapS, GLint nWrapT, GLint nDepthFormat )
{
	// Check if we already generated a depth texture for this
	std::map< uint32_t, uint32_t >::iterator const it = m_mapColorDepth.find( nTexture );
	if ( it != m_mapColorDepth.end() )
		return it->second;

	// Create a new depth buffer
	uint32_t nDepth;
	GLint nWidth, nHeight;

	glBindTexture( GL_TEXTURE_2D, nTexture );

	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &nWidth );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &nHeight );

	glGenTextures( 1, &nDepth );
	glBindTexture( GL_TEXTURE_2D, nDepth );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nMinFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nMagnitudeFilter );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, nWrapS );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, nWrapT );

	glTexImage2D( GL_TEXTURE_2D, 0, nDepthFormat, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );

	m_mapColorDepth.insert( std::make_pair( nTexture, nDepth ) );

	return nDepth;
}

GLuint XRMirror_GL::CreateShaderProgram( const wchar_t *pVertexShaderFile, const wchar_t *pFragmentShaderFile )
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
		m_pLogger->error( "Unable to link shader program ({}) to vertex ({}) and fragment ({}) shaders. {}", nShaderVisMask, nVertexShaderID, nFragmentShaderID, eMessage );

		return 0;
	}

	// Cleanup
	glDetachShader( nShaderVisMask, nVertexShaderID );
	glDetachShader( nShaderVisMask, nFragmentShaderID );

	glDeleteShader( nVertexShaderID );
	glDeleteShader( nFragmentShaderID );

	// Return shader program id
	m_pLogger->info( "Shader program ({}) succesfully created and linked to the vertex ({}) and fragment ({}) shaders", nShaderVisMask, nVertexShaderID, nFragmentShaderID );
	return nShaderVisMask;
}

GLuint XRMirror_GL::LoadShaderFromDisk( GLenum eShaderType, const char *pFilePath )
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
