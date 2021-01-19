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

#include <main.h>

#define APP_PROJECT_NAME	"OpenXR Sandbox"
#define APP_PROJECT_VER		1
#define APP_ENGINE_NAME		"Custom"
#define APP_ENGINE_VER		1
#define APP_MIRROR_WIDTH	800
#define APP_MIRROR_HEIGHT	600

#define APP_LOG_TITLE		"Sandbox"
#define APP_LOG_FILE		L"\\logs\\openxr-provider-sandbox-log.txt"

#define VIS_MASK_VERTEX_SHADER		L"\\shaders\\vert-vismask.glsl"
#define VIS_MASK_FRAGMENT_SHADER	L"\\shaders\\frag-vismask.glsl"

#define LIT_VERTEX_SHADER		L"\\shaders\\vert-lit.glsl"
#define LIT_FRAGMENT_SHADER		L"\\shaders\\frag-lit.glsl"

#define UNLIT_VERTEX_SHADER		L"\\shaders\\vert-unlit.glsl"
#define UNLIT_FRAGMENT_SHADER	L"\\shaders\\frag-unlit.glsl"

#define TEXTURED_VERTEX_SHADER		L"\\shaders\\vert-textured.glsl"
#define TEXTURED_FRAGMENT_SHADER	L"\\shaders\\frag-textured.glsl"

int main()
{
	// Setup our sandbox application, which will be used for testing
	if ( AppSetup() != 0 )
		return -1;

	#pragma region OPENXR_PROVIDER_SETUP

	// SETUP OPENXR PROVIDER
	// OpenXR Provider is a wrapper library to the official OpenXR loader.
	//
	// This library provides classes that when instantiated, exposes to an app simpler access
	// to any currently running OpenXR runtime (e.g SteamVR, OculusVR, etc) by abstracting away most
	// of the necessary OpenXR calls and their strict call sequence

	// (1) Optional: Create Extension class(es) for the OpenXR extensions the app wants to activate.
	//     The graphics api which is also an extension in OpenXR, will be activated for the app
	//	   as appropriate (see XRGraphicsAPI class in XRGraphicsAwareTypes.h)

	pXRVisibilityMask = new OpenXRProvider::XRExtVisibilityMask( pUtils->GetLogger() );
	std::vector< void * > RequestExtensions{pXRVisibilityMask};

	// (2) Create OpenXR Provider - this creates an OpenXR instance and session.
	//     Provide application metadata here which will be used by the OpenXR Provider
	//     library to setup an instance and session with the OpenXR loader

	OpenXRProvider::XRAppInfo xrAppInfo(
		APP_PROJECT_NAME,
		APP_PROJECT_VER,
		APP_ENGINE_NAME,
		APP_ENGINE_VER,
		OpenXRProvider::ROOMSCALE,
		RequestExtensions,						// optional: see step (1), otherwise an empty vector
		false,									// optional: depth texture support if the active runtime supports it
		pAppLogFile								// optional: log file logging for the library
		);

	// App graphics info is a struct that holds graphics-api dependent values that'll be used
	// to access the user's hardware and app context
	OpenXRProvider::XRAppGraphicsInfo xrAppGraphicsInfo( wglGetCurrentDC(), wglGetCurrentContext() );

	// Finally, create an XRProvider class
	try
	{
		pXRProvider = new OpenXRProvider::XRProvider( xrAppInfo, xrAppGraphicsInfo );
	}
	catch ( const std::exception &e )
	{
		pUtils->GetLogger()->info( "Unable to create OpenXR Provider. {}", e.what() );
		pUtils->GetLogger()->info( "OpenXR Instance and Session can't be established with the active OpenXR runtime" );
		return -1;
	}

	// (3) Create OpenXR Render manager - this handles all the OpenXR rendering calls.
	//	   The OpenXR session created by the XRProvider class will be started and
	//     textures to render to will be created in an accessible image swapchain
	//     using the render information provided here

	OpenXRProvider::XRRenderInfo xrRenderInfo(
		{ GL_SRGB8_ALPHA8 },	// Request texture formats here in order of preference.
								// These are uint64_t nums that's defined by the graphics API
								// (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
								// Otherwise, put 0 in the array to let the runtime decide the format

		{0},			// Request depth texture formats here in order of preference.
						// These are uint64_t nums that's defined by the graphics API
						// (e.g. GL_DEPTH_COMPONENT, DXGI_FORMAT_D16_UNORM, etc)
						// Otherwise, put 0 in the array to get a default depth format

		1,				// Texture array size. 1 if not an array.
		1				// Mip count
		);

	try
	{
		pXRRenderManager = new OpenXRProvider::XRRenderManager( pXRProvider, xrRenderInfo );
	}
	catch ( const std::exception &e )
	{
		pUtils->GetLogger()->info( "Unable to create the XR Render Manager. {}", e.what() );
		pUtils->GetLogger()->info( "OpenXR Session can't be started. Tracking or rendering operations can't begin" );
		return -1;
	}

	// Get swapchain capacity
	nSwapchainCapacity = pXRRenderManager->GetGraphicsAPI()->GetSwapchainImageCount( OpenXRProvider::LEFT );

	if ( nSwapchainCapacity < 1 )
	{
		// This shouldn't really happen with conformant OpenXR runtimes
		pUtils->GetLogger()->info( "Not enough swapchain capacity ({}) to do any rendering work", nSwapchainCapacity );
		return -1;
	}

	// (4) Optional: Register for OpenXR events
	OpenXRProvider::XRCallback xrCallback = {OpenXRProvider::ALL};
	OpenXRProvider::XRCallback *pXRCallback = &xrCallback;
	pXRCallback->type = OpenXRProvider::EXREventType::SESSION_STATE_CHANGED;
	pXRCallback->callback = Callback_XR_Session_Changed;
	pXRProvider->GetXREventHandler()->RegisterCallback( pXRCallback );

	// (5) Optional: Use any pre-render loop extensions

	// Retrieve visibility mask from runtime if available
	OpenXRProvider::XRExtVisibilityMask *pXRVisibilityMask = pXRRenderManager->GetXRVisibilityMask();
	if ( pXRVisibilityMask )
	{
		pXRVisibilityMask->GetVisibilityMask( OpenXRProvider::LEFT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN, vMaskVertices_L, vMaskIndices_L );
		pXRVisibilityMask->GetVisibilityMask( OpenXRProvider::RIGHT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN, vMaskVertices_R, vMaskIndices_R );
	}

	pUtils->GetLogger()->info(
		"Runtime returned a visibility mask with {} verts and {} indices for the left eye (0)",
		vMaskVertices_L.size() / 2,
		vMaskIndices_L.size() );
	pUtils->GetLogger()->info(
		"Runtime returned a visibility mask with {} verts and {} indices for the right eye (1)",
		vMaskVertices_R.size() / 2,
		vMaskIndices_R.size() );

	#pragma endregion OPENXR_PROVIDER_SETUP

	// Setup any graphics api objects needed by the sandbox.
	// OpenXR provides the textures to render to, so simply
	// retrieve them from the XR Provider's Render Manager swapchain
	if ( GraphicsAPIObjectsSetup() != 0 )
		return -1;

	#pragma region SANDBOX_FRAME_LOOP

	// FRAME LOOP
	while ( !glfwWindowShouldClose( pXRMirror->GetWindow() ) )
	{
		// (1) Check for openxr events
		//     a chance for us to run any of our registered callbacks
		//     and track the openxr session state so we can act accordingly
		pXRProvider->PollXREvents();

		// Check if runtime wants to close the app
		if ( xrCurrentSessionState == OpenXRProvider::EXREventData::SESSION_STATE_EXITING ||
		     xrCurrentSessionState == OpenXRProvider::EXREventData::SESSION_STATE_LOSS_PENDING ||
		     xrCurrentSessionState == OpenXRProvider::EXREventData::SESSION_STATE_STOPPING )
		{
			break;
		}

		// (2) Render pass
		nSwapchainIndex = nSwapchainIndex > nSwapchainCapacity - 1 ? 0 : nSwapchainIndex;

		DrawFrame( pXRRenderManager, OpenXRProvider::LEFT, nSwapchainIndex, FBO, nShaderVisMask );
		DrawFrame( pXRRenderManager, OpenXRProvider::RIGHT, nSwapchainIndex, FBO, nShaderVisMask );

		// [DEBUG] pUtils->GetLogger()->info("Processing frame {} swapchain index {} / {}", nFrameNumber, nSwapchainIndex, nSwapchainCapacity);

		// (3) Render frame - call ProcessXRFrame from the render manager after rendering to the appropriate swapchain image
		if ( pXRRenderManager && pXRRenderManager->ProcessXRFrame() )
		{
			// Blit (copy) texture to XR Mirror
			BlitToWindow();

			// [DEBUG] pUtils->GetLogger()->info("HMD IPD is currently set to: {}", xrRenderManager->GetCurrentIPD());
		}

		// (4) Update app frame state
		++nFrameNumber;
		++nSwapchainIndex;

		// (5) glfw render and input events
		glfwSwapBuffers( pXRMirror->GetWindow() );
		glfwPollEvents();
	}

	#pragma endregion SANDBOX_FRAME_LOOP

	// CLEANUP
	delete pXRRenderManager;
	delete pXRProvider;
	delete pXRMirror;
	delete pUtils;

	return 0;
}

static void Callback_XR_Session_Changed( const OpenXRProvider::EXREventType xrEventType, const OpenXRProvider::EXREventData xrEventData )
{
	xrCurrentSessionState = xrEventData;

	// if (helperUtils && helperUtils->GetLogger())
	//	pUtils->GetLogger()->info("OpenXR Event Session State Changed to {}", std::to_string(xrCurrentSessionState));
}

int AppSetup()
{
	// Get executable directory
	wchar_t exePath[ MAX_PATH ];
	int nExePathLength = GetModuleFileNameW( NULL, exePath, MAX_PATH );

	if ( nExePathLength < 1 )
	{
		std::cout << "Fatal error! Unable to get current executable directory from operating system \n";
		return -1;
	}

	std::wstring::size_type nLastSlashIndex = std::wstring( exePath ).find_last_of( L"\\/" );
	sCurrentPath = std::wstring( exePath ).substr( 0, nLastSlashIndex );

	// Set the log file location for the OpenXR Library to use
	std::wcstombs( pAppLogFile, ( sCurrentPath + APP_LOG_FILE ).c_str(), MAX_PATH );

	// Setup helper utilities class
	pUtils = new Utils( APP_LOG_TITLE, pAppLogFile );

	// Create XR Mirror (Window)
	nScreenWidth = APP_MIRROR_WIDTH;
	nScreenHeight = APP_MIRROR_HEIGHT;

	pXRMirror = new XRMirror( nScreenWidth, nScreenHeight, APP_PROJECT_NAME, pAppLogFile );
	glfwMakeContextCurrent( pXRMirror->GetWindow() );

	return 0;
}

int GraphicsAPIObjectsSetup()
{
	// Enable depth buffer testing
	glFrontFace( GL_CW );
	glEnable( GL_DEPTH_TEST );

	// Setup vertex buffer object (cube)
	glGenBuffers( 1, &cubeVBO );

	glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vCube ), vCube, GL_STATIC_DRAW );

	// Setup element buffer object (cube)
	//glGenBuffers(1, &cubeEBO);

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

	// Setup frame buffer object
	glGenFramebuffers( 1, &FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, FBO );

	// Create shader programs
	nShaderVisMask = pUtils->CreateShaderProgram( ( sCurrentPath + VIS_MASK_VERTEX_SHADER ).c_str(), ( sCurrentPath + VIS_MASK_FRAGMENT_SHADER ).c_str() );
	nShaderLit = pUtils->CreateShaderProgram( ( sCurrentPath + LIT_VERTEX_SHADER ).c_str(), ( sCurrentPath + LIT_FRAGMENT_SHADER ).c_str() );
	nShaderUnlit = pUtils->CreateShaderProgram( ( sCurrentPath + UNLIT_VERTEX_SHADER ).c_str(), ( sCurrentPath + UNLIT_FRAGMENT_SHADER ).c_str() );
	nShaderTextured = pUtils->CreateShaderProgram( ( sCurrentPath + TEXTURED_VERTEX_SHADER ).c_str(), ( sCurrentPath + TEXTURED_FRAGMENT_SHADER ).c_str() );

	if ( nShaderVisMask == 0 || nShaderLit == 0 || nShaderUnlit == 0 || nShaderTextured == 0 )
		return -1;

	// Set fragment shader colors
	glUseProgram( nShaderLit );
	glUniform3f( glGetUniformLocation( nShaderLit, "surfaceColor" ), 1.0f, 1.0f, 0.0f );
	glUniform3f( glGetUniformLocation( nShaderLit, "lightColor" ), 1.0f, 1.0f, 1.0f );

	glUseProgram( nShaderUnlit );
	glUniform3f( glGetUniformLocation( nShaderUnlit, "surfaceColor" ), 1.0f, 1.0f, 1.0f );

	// Load textures for sea of cubes
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_bellevue_valve.png" ).c_str(), nShaderTextured, "texSample" ) );
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_munich_mein_schatz.png" ).c_str(), nShaderTextured, "texSample" ) );
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_hobart_mein_heim.png" ).c_str(), nShaderTextured, "texSample" ) );
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_hobart_rose.png" ).c_str(), nShaderTextured, "texSample" ) );
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_hobart_mein_kochen.png" ).c_str(), nShaderTextured, "texSample" ) );
	vSeaOfCubesTextures.push_back( pXRMirror->LoadTexture( ( sCurrentPath + L"\\img\\t_hobart_sunset.png" ).c_str(), nShaderTextured, "texSample" ) );

	return 0;
}

void DrawFrame(
	OpenXRProvider::XRRenderManager *pXRRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	unsigned int FBO,
	GLuint nShaderVisMask )
{
	glBindFramebuffer( GL_FRAMEBUFFER, FBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pXRRenderManager->GetGraphicsAPI()->GetTexture2D( eEye, nSwapchainIndex ), 0 );

	glClearColor( 0.5f, 0.9f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	// Check if hmd is tracking
	if ( !pXRRenderManager->GetHMDState()->IsPositionTracked || !pXRRenderManager->GetHMDState()->IsPositionTracked )
		return;

	// Draw vismask on XR Mirror
	uint32_t nVertCount = ( eEye == OpenXRProvider::LEFT ) ? ( uint32_t )vMaskVertices_L.size() : ( uint32_t )vMaskVertices_L.size();
	float *vMask = ( eEye == OpenXRProvider::LEFT ) ? vMaskVertices_L.data() : vMaskVertices_R.data();

	if ( nVertCount > 0 )
	{
		// TO DO: Apply vismask, set clip values
	}

	// Draw cube
	glm::vec3 cubeScale = glm::vec3( 0.5f, 0.5f, 0.5f );
	DrawSeaOfCubes(
		pXRRenderManager,
		eEye,
		nSwapchainIndex,
		nShaderTextured,
		cubeInstanceDataVBO,
		cubeVAO,
		cubeScale,
		vSeaOfCubesTextures,
		1.5f,
		1.5f );
}

void BlitToWindow()
{
	glfwGetWindowSize( pXRMirror->GetWindow(), &nScreenWidth, &nScreenHeight );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	glViewport( 0, 0, pXRRenderManager->GetTextureWidth(), pXRRenderManager->GetTextureHeight() );
	glBlitFramebuffer(
		0,
		0,
		pXRRenderManager->GetTextureWidth(),
		pXRRenderManager->GetTextureHeight(),
		0,
		0,
		nScreenWidth,
		nScreenHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR );
}


void DrawCube(
	OpenXRProvider::XRRenderManager *pRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	GLuint nShader,
	unsigned int nTexture,
	unsigned int VAO,
	glm::vec3 cubePosition )
{
	assert( pRenderManager );

	glUseProgram( nShader );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, nTexture );

	glm::mat4 cubeModel = glm::mat4( 1.0f );
	cubeModel = glm::translate( cubeModel, cubePosition );
	//cubeModel = glm::rotate(cubeModel, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	cubeModel = glm::scale( cubeModel, glm::vec3( 0.8f, 0.8f, 0.8f ) );

	// Get eye pose
	OpenXRProvider::XRPose eyePose = ( eEye == OpenXRProvider::LEFT ) ?
		                                 pRenderManager->GetHMDState()->LeftEye.Pose :
		                                 pRenderManager->GetHMDState()->RightEye.Pose;

	// Get the eye view and apply translation and rotation
	glm::mat4 eyeView( 1.0f );
	glm::quat eyeRotation( eyePose.Orientation.w, eyePose.Orientation.x, eyePose.Orientation.y, eyePose.Orientation.z );
	eyeView = glm::translate( eyeView, glm::vec3( eyePose.Position.x, eyePose.Position.y, eyePose.Position.z ) );
	eyeView = glm::rotate( eyeView, angle( eyeRotation ), axis( eyeRotation ) );

	glm::mat4 cubeMVP = ( ( eEye == OpenXRProvider::LEFT ) ? GetEyeProjectionLeft() : GetEyeProjectionRight() ) * InvertMatrix( eyeView ) * cubeModel;
	glUniformMatrix4fv( glGetUniformLocation( nShader, "mvp" ), 1, GL_FALSE, &cubeMVP[ 0 ][ 0 ] );

	glBindVertexArray( VAO );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
}

void DrawSeaOfCubes(
	OpenXRProvider::XRRenderManager *pRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	GLuint nShader,
	unsigned int VBO,
	unsigned VAO,
	glm::vec3 cubeScale,
	std::vector< unsigned > vTextures,
	float fSpacingPlane,
	float fSpacingHeight )
{
	// Set eye projections if we haven't already
	if ( !m_bEyeProjectionsSet )
	{
		m_EyeProjectionLeft = GetEyeProjection( pRenderManager, pRenderManager->GetHMDState()->LeftEye.FoV, 0.1f, 100.f );
		m_EyeProjectionRight = GetEyeProjection( pRenderManager, pRenderManager->GetHMDState()->RightEye.FoV, 0.1f, 100.f );
		m_bEyeProjectionsSet = true;
	}

	// Get eye view for this frame
	OpenXRProvider::XRPose eyePose = ( eEye == OpenXRProvider::LEFT ) ?
		                                 pRenderManager->GetHMDState()->LeftEye.Pose :
		                                 pRenderManager->GetHMDState()->RightEye.Pose;

	glm::mat4 eyeView( 1.0f );
	glm::quat eyeRotation( eyePose.Orientation.w, eyePose.Orientation.x, eyePose.Orientation.y, eyePose.Orientation.z );
	eyeView = glm::translate( eyeView, glm::vec3( eyePose.Position.x, eyePose.Position.y, eyePose.Position.z ) );
	eyeView = glm::rotate( eyeView, angle( eyeRotation ), axis( eyeRotation ) );

	glm::mat4 eyeViewInverted = InvertMatrix( eyeView );

	// Generate sea of cubes
	uint32_t nTextureCount = vTextures.size();
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
				//DrawCube(pRenderManager, eEye, nSwapchainIndex, nShader, vTextures[i], VAO, glm::vec3(x, startPosition.y, z));
				FillEyeMVP(
					pRenderManager,
					vEyeProjections,
					eyeViewInverted,
					eEye,
					nCubeIndex,
					nShader,
					vTextures[ i ],
					glm::vec3( x, startPosition.y, z ),
					cubeScale );
				++nCubeIndex;
			}
		}

		const uint32_t depthTexture = GetDepth( pRenderManager->GetGraphicsAPI()->GetTexture2D( eEye, nSwapchainIndex ) );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 );

		// Draw cube instance in a single plane at once
		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ) * 36, vEyeProjections, GL_STATIC_DRAW );

		glBindVertexArray( VAO );
		glDrawArraysInstanced( GL_TRIANGLES, 0, 36, 216 );

		// Go to the next cube plane up
		startPosition.y += fSpacingHeight;
		nCubeIndex = 0;
	}

	delete[] vEyeProjections;
}

void FillEyeMVP(
	OpenXRProvider::XRRenderManager *pRenderManager,
	glm::mat4 *vEyeProjections,
	glm::mat4 eyeView,
	OpenXRProvider::EXREye eEye,
	uint32_t nCubeIndex,
	GLuint nShader,
	unsigned nTexture,
	glm::vec3 cubePosition,
	glm::vec3 cubeScale )
{
	assert( pRenderManager );

	glUseProgram( nShader );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, nTexture );

	glm::mat4 cubeModel = glm::mat4( 1.0f );
	cubeModel = glm::translate( cubeModel, cubePosition );
	//cubeModel = glm::rotate(cubeModel, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	cubeModel = glm::scale( cubeModel, cubeScale );

	vEyeProjections[ nCubeIndex ] = ( ( ( eEye == OpenXRProvider::LEFT ) ? GetEyeProjectionLeft() : GetEyeProjectionRight() ) * eyeView * cubeModel );
}

glm::mat4 GetEyeProjection( OpenXRProvider::XRRenderManager *pRenderManager, OpenXRProvider::XRFoV eyeFoV, float fNear, float fFar )
{
	assert( pRenderManager );

	// Convert angles and fovs
	float fovLeft = std::tanf( eyeFoV.LeftAngle );
	float fovRight = std::tanf( eyeFoV.RightAngle );
	float fovUp = std::tanf( eyeFoV.UpAngle );
	float fovDown = std::tanf( eyeFoV.DownAngle );

	float fovWidth = fovRight - fovLeft;
	float fovHeight = fovUp - fovDown;

	float fOffset = fNear;
	float fLength = fFar - fNear;

	// Calculate eye projection matrix
	glm::mat4 mProjection = glm::mat4( 1.0f );

	mProjection[ 0 ][ 0 ] = 2.0f / fovWidth;
	mProjection[ 0 ][ 1 ] = 0.0f;
	mProjection[ 0 ][ 2 ] = 0.0f;
	mProjection[ 0 ][ 3 ] = 0.0f;

	mProjection[ 1 ][ 0 ] = 0.0f;
	mProjection[ 1 ][ 1 ] = 2.0f / fovHeight;
	mProjection[ 1 ][ 2 ] = 0.0f;
	mProjection[ 1 ][ 3 ] = 0.0f;

	mProjection[ 2 ][ 0 ] = ( fovRight + fovLeft ) / fovWidth;
	mProjection[ 2 ][ 1 ] = ( fovUp + fovDown ) / fovHeight;
	mProjection[ 2 ][ 2 ] = -( fFar + fOffset ) / fLength;
	mProjection[ 2 ][ 3 ] = -1.0f;

	mProjection[ 3 ][ 0 ] = 0.0f;
	mProjection[ 3 ][ 1 ] = 0.0f;
	mProjection[ 3 ][ 2 ] = -( fFar * ( fNear + fOffset ) ) / fLength;
	mProjection[ 3 ][ 3 ] = 0.0f;

	return mProjection;
}

glm::mat4 InvertMatrix( const glm::mat4 mMatrix )
{
	glm::mat4 mInvertedMatrix( 1.0f );

	mInvertedMatrix[ 0 ][ 0 ] = mMatrix[ 0 ][ 0 ];
	mInvertedMatrix[ 0 ][ 1 ] = mMatrix[ 1 ][ 0 ];
	mInvertedMatrix[ 0 ][ 2 ] = mMatrix[ 2 ][ 0 ];
	mInvertedMatrix[ 0 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 1 ][ 0 ] = mMatrix[ 0 ][ 1 ];
	mInvertedMatrix[ 1 ][ 1 ] = mMatrix[ 1 ][ 1 ];
	mInvertedMatrix[ 1 ][ 2 ] = mMatrix[ 2 ][ 1 ];
	mInvertedMatrix[ 1 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 2 ][ 0 ] = mMatrix[ 0 ][ 2 ];
	mInvertedMatrix[ 2 ][ 1 ] = mMatrix[ 1 ][ 2 ];
	mInvertedMatrix[ 2 ][ 2 ] = mMatrix[ 2 ][ 2 ];
	mInvertedMatrix[ 2 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 3 ][ 0 ] = -( mMatrix[ 0 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 0 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 0 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 1 ] = -( mMatrix[ 1 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 1 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 1 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 2 ] = -( mMatrix[ 2 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 2 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 2 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 3 ] = 1.0f;

	return mInvertedMatrix;
}

uint32_t GetDepth( uint32_t nTexture, GLint nMinFilter, GLint nMagnitudeFilter, GLint nWrapS, GLint nWrapT, GLint nDepthFormat )
{
	// Check if we already generated a depth texture for this
	std::map< uint32_t, uint32_t >::iterator const it = m_mapColorDepth.find( nTexture );
	if ( it != m_mapColorDepth.end() )
		return it->second;

	// Create a new depth buffer
	// TODO: Pickup from depth swapchain if supported
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
