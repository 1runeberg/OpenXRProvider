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
	std::vector< void * > RequestExtensions { pXRVisibilityMask };

	// (2) Create OpenXR Provider - this creates an OpenXR instance and session.
	//     Provide application metadata here which will be used by the OpenXR Provider
	//     library to setup an instance and session with the OpenXR loader

	OpenXRProvider::XRAppInfo xrAppInfo(
		APP_PROJECT_NAME,
		APP_PROJECT_VER,
		APP_ENGINE_NAME,
		APP_ENGINE_VER,
		OpenXRProvider::ROOMSCALE,
		RequestExtensions				// See optional step (1), otherwise an empty vector
	);

	xrAppInfo.LogFile = pAppLogFile;	// optional log file logging for the library

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
		pUtils->GetLogger()->info( "Unable to create OpenXR Provider. {}",  e.what());
		pUtils->GetLogger()->info( "OpenXR Instance and Session can't be established with the active OpenXR runtime" );
		return -1;
	}
		
	// (3) Create OpenXR Render manager - this handles all the OpenXR rendering calls.
	//	   The OpenXR session created by the XRProvider class will be started and
	//     textures to render to will be created in an accessible image swapchain
	//     using the render information provided here

	OpenXRProvider::XRRenderInfo xrRenderInfo(
		{ GL_SRGB8_ALPHA8 }, // Request texture formats here in order of preference.
							 // These are uint64_t nums that's defined by the graphics API
							 // (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
							 // Otherwise, put 0 in the array to let the runtime decide the format

		false, // Set to true if you want/need depth textures generated as well
			   // along with the color textures that we will render to

		1,	  // Texture array size. 1 if not an array.
		1	  // Mip count
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
	OpenXRProvider::XRCallback xrCallback = { OpenXRProvider::ALL };
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
		"Runtime returned a visibility mask with {} verts and {} indices for the left eye (0)", vMaskVertices_L.size()/2, vMaskIndices_L.size() );
	pUtils->GetLogger()->info(
		"Runtime returned a visibility mask with {} verts and {} indices for the right eye (1)", vMaskVertices_R.size()/2, vMaskIndices_R.size() );

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
	// Setup vertex array object
	glGenVertexArrays( 1, &VAO );
	glBindVertexArray( VAO );

	// Setup vertex buffer object
	glGenBuffers( 1, &VBO );

	// Setup frame buffer object
	glGenFramebuffers( 1, &FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, FBO );

	// Create shader program for the visibility masks
	nShaderVisMask = pUtils->CreateShaderProgram( ( sCurrentPath + VIS_MASK_VERTEX_SHADER ).c_str(), ( sCurrentPath + VIS_MASK_FRAGMENT_SHADER ).c_str() );

	if ( nShaderVisMask == 0 )
		return -1;

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
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Draw shape on XR Mirror
	uint32_t nVertCount = ( eEye == OpenXRProvider::LEFT ) ? ( uint32_t )vMaskVertices_L.size() : ( uint32_t )vMaskVertices_L.size();

	float *vMask = ( eEye == OpenXRProvider::LEFT ) ? vMaskVertices_L.data() : vMaskVertices_R.data();

	if ( nVertCount > 0 )
	{
		// Send shape vertices to vbo
		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vMask ), vMask, GL_STATIC_DRAW );

		// Setup the vertex attributes
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), ( void * )0 );
		glEnableVertexAttribArray( 0 );

		// Activate shader program
		glUseProgram( nShaderVisMask );

		// Draw the mask
		glDrawArrays( GL_TRIANGLES, 0, nVertCount );
	}
}

void BlitToWindow()
{
	glfwGetWindowSize( pXRMirror->GetWindow(), &nScreenWidth, &nScreenHeight );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	glViewport( 0, 0, nScreenWidth, nScreenHeight );
	glBlitFramebuffer( 0, 0, nScreenWidth, nScreenHeight, 0, 0, nScreenWidth, nScreenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );
}
