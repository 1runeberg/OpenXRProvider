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

int main( int argc, char *argv[] )
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
	//     There is no need to specify the graphics api extension as it will be automatically activated

	pCommon->XRVisibilityMask( new OpenXRProvider::XRExtVisibilityMask( pCommon->GetLogger() ) );
	pCommon->XRHandTracking ( new OpenXRProvider::XRExtHandTracking( pCommon->GetLogger() ) );

	std::vector< void * > RequestExtensions { pCommon->XRVisibilityMask(), pCommon->XRHandTracking() };

	// (2) Setup the application metadata which will be used by the OpenXR Provider
	//     library to setup an instance and session with the OpenXR loader

	OpenXRProvider::XRAppInfo xrAppInfo
	(
		APP_PROJECT_NAME,
		APP_PROJECT_VER,
		APP_ENGINE_NAME,
		APP_ENGINE_VER,
		OpenXRProvider::TRACKING_ROOMSCALE,
		RequestExtensions,						// optional: see step (1), otherwise an empty vector
		pAppLogFile								// optional: log file logging for the library
	);

	// (3) Setup currently active graphics api info that'll be used to access the user's hardware 
	//     and app context
	#ifdef XR_USE_GRAPHICS_API_OPENGL
		OpenXRProvider::XRAppGraphicsInfo xrAppGraphicsInfo( wglGetCurrentDC(), wglGetCurrentContext() );
	#endif

	// (4) Setup required render info. Texture/images in the swapchain will be generated using the 
	//     information provided here
	OpenXRProvider::XRRenderInfo xrRenderInfo
	(
		{ GL_SRGB8_ALPHA8 }, // Request texture formats here in order of preference.
							 // These are uint64_t nums that's defined by the graphics API
							 // (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
							 // Otherwise, put 0 in the array to let the runtime decide the format

		{ 0 },				// Request depth texture formats here in order of preference.
							// These are uint64_t nums that's defined by the graphics API
							// (e.g. GL_DEPTH_COMPONENT, DXGI_FORMAT_D16_UNORM, etc)
							// Otherwise, put 0 in the array to get a default depth format

		1,					// Texture array size. 1 if not an array.
		1,					// Mip count
		false				// optional: depth texture support if the active runtime supports it
	);

	// (5) Finally, create an XRProvider class
	try
	{
		pXRProvider = new OpenXRProvider::XRProvider( xrAppInfo, xrAppGraphicsInfo, xrRenderInfo );
	}
	catch ( const std::exception &e )
	{
		pCommon->GetLogger()->info( "Unable to create OpenXR Provider. {}", e.what() );
		pCommon->GetLogger()->info( "OpenXR Instance and Session can't be established with the active OpenXR runtime" );
		return -1;
	}
	
	// (6) Create input bindings (if any)

	// 6.1 Setup space locations that will hold the controller poses per frame. Optionally add velocity in the queries
	pCommon->XrLocation_Left()->next = pCommon->XrVelocity_Left();
	pCommon->XrLocation_Right()->next = pCommon->XrVelocity_Right();

	// 6.2 Create action set(s)
	xrActionSet_Main = pXRProvider->Input()->CreateActionSet( "main", "main", 0 );

	// 6.3 Create actions mapped to specific action set(s)
	xrActionState_PoseLeft.type = XR_TYPE_ACTION_STATE_POSE;
	xrAction_PoseLeft = pXRProvider->Input()->CreateAction( xrActionSet_Main, "pose_left", "Pose (Left)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );

	xrActionState_PoseRight.type = XR_TYPE_ACTION_STATE_POSE;
	xrAction_PoseRight = pXRProvider->Input()->CreateAction( xrActionSet_Main, "pose_right", "Pose (Right)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );

	xrActionState_SwitchScene.type = XR_TYPE_ACTION_STATE_BOOLEAN;
	xrAction_SwitchScene = pXRProvider->Input()->CreateAction( xrActionSet_Main, "switch_scene", "Switch Scenes", XR_ACTION_TYPE_BOOLEAN_INPUT, 0, NULL );
	
	xrAction_Haptic = pXRProvider->Input()->CreateAction( xrActionSet_Main, "haptic", "Haptic Feedback", XR_ACTION_TYPE_VIBRATION_OUTPUT, 0, NULL );

	// 6.4 Create action bindings for every controller this app supports
	CreateInputActionBindings();

	// 6.5 Suggest controller-specific action bindings to the runtime (one call for each controller this app supports)
	pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->ValveIndex()->ActionBindings(), pXRProvider->Input()->ValveIndex()->GetInputProfile() );
	pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->HTCVive()->ActionBindings(), pXRProvider->Input()->HTCVive()->GetInputProfile() );
	pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->OculusTouch()->ActionBindings(), pXRProvider->Input()->OculusTouch()->GetInputProfile() );

	// 6.6 Activate all action sets that we want to update per frame (this can also be changed per frame or anytime app wants to sync a different action set data)
	pXRProvider->Input()->ActivateActionSet( xrActionSet_Main );


	// (7) Optional: Cache anything your app needs in the frame loop

	// 7.1 Test for activated instance extensions
	pCommon->ShouldDrawHandJoints( pCommon->XRHandTracking()->IsActive() );

	// 7.2 Get swapchain capacity
	nSwapchainCapacity = pXRProvider->Render()->GetGraphicsAPI()->GetSwapchainImageCount( OpenXRProvider::EYE_LEFT );

	if ( nSwapchainCapacity < 1 )
	{
		// This shouldn't really happen with conformant OpenXR runtimes
		pCommon->GetLogger()->info( "Not enough swapchain capacity ({}) to do any rendering work", nSwapchainCapacity );
		return -1;
	}

	// (8) Optional: Register for OpenXR events
	OpenXRProvider::XRCallback xrCallback = { XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED };	// XR_TYPE_EVENT_DATA_BUFFER = Register for all events
	OpenXRProvider::XRCallback *pXRCallback = &xrCallback;
	pXRCallback->callback = Callback_XR_Event;
	pXRProvider->Core()->GetXREventHandler()->RegisterCallback( pXRCallback );

	// (9) Optional: Use any pre-render loop extensions

	// Retrieve visibility mask from runtime if available
	OpenXRProvider::XRExtVisibilityMask *m_pXRVisibilityMask = pXRProvider->Render()->GetXRVisibilityMask();
	if ( m_pXRVisibilityMask )
	{
		m_pXRVisibilityMask->GetVisibilityMask( 
			OpenXRProvider::EYE_LEFT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN,
			*pCommon->MaskVertices_Left(), *pCommon->MaskIndices_Left() );
		m_pXRVisibilityMask->GetVisibilityMask( 
			OpenXRProvider::EYE_RIGHT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN, 
			*pCommon->MaskVertices_Right(), *pCommon->MaskIndices_Right() );
	}

	pCommon->GetLogger()->info(
		"Runtime returned a visibility mask with {} verts and {} indices for the left eye (0)",
		pCommon->MaskVertices_Left()->size() / 2,
		pCommon->MaskIndices_Left()->size() );
	pCommon->GetLogger()->info(
		"Runtime returned a visibility mask with {} verts and {} indices for the right eye (1)",
		pCommon->MaskVertices_Right()->size() / 2,
		pCommon->MaskIndices_Right()->size() );

	#pragma endregion OPENXR_PROVIDER_SETUP

	// Setup any graphics api objects needed by the sandbox.
	// OpenXR provides the textures to render to, so simply
	// retrieve them from the XR Provider's Render Manager swapchain
	if ( pXRMirror->Init( pXRProvider->Render() ) != 0 )
		return -1;

	#pragma region SANDBOX_FRAME_LOOP

	// FRAME LOOP
	while ( !bExitApp )
	{
		// (1) Check for openxr events
		//     a chance for us to run any of our registered callbacks
		//     and track the openxr session state so we can act accordingly
		pXRProvider->Core()->PollXREvents();

		// Check if runtime wants to close the app
		if ( xrCurrentSessionState == XR_SESSION_STATE_EXITING )
			break;	

		if ( xrCurrentSessionState == XR_SESSION_STATE_IDLE )
		{
			// HMD is not ready or inactive, clear window with clear color
			pXRMirror->Clear( glm::vec4 { 0.5f, 0.9f, 1.0f, 1.0f } );
			pXRMirror->BlitToWindow();
		}
		else if ( xrCurrentSessionState == XR_SESSION_STATE_READY )
		{
			// Begin the session - e.g. coming back from stopping state
			XrResult xrResult = pXRProvider->Core()->XRBeginSession();
			bool bResult = pXRProvider->Render()->ProcessXRFrame();

			pCommon->GetLogger()->info( "OpenXR Session started ({}) and initial frame processed ({})", OpenXRProvider::XrEnumToString( xrResult ), bResult );
		}
		else if (  xrCurrentSessionState == XR_SESSION_STATE_STOPPING )
		{
			// End session so runtime can safely transition back to idle
			XrResult xrResult = pXRProvider->Core()->XREndSession();

			pCommon->GetLogger()->info( "OpenXR Session ended ({})", OpenXRProvider::XrEnumToString( xrResult ) );
		}
		else if ( xrCurrentSessionState > XR_SESSION_STATE_IDLE )
		{
			// (2) Process frame - call ProcessXRFrame from the render manager 
			if ( pXRProvider->Render() && pXRProvider->Render()->ProcessXRFrame() )
			{
				// 2.1 Render to swapchain image
				nSwapchainIndex = nSwapchainIndex > nSwapchainCapacity - 1 ? 0 : nSwapchainIndex;

				pXRMirror->DrawFrame( eCurrentScene, OpenXRProvider::EYE_LEFT, nSwapchainIndex );
				pXRMirror->DrawFrame( eCurrentScene, OpenXRProvider::EYE_RIGHT, nSwapchainIndex );

				// Blit (copy) texture to XR Mirror
				pXRMirror->BlitToWindow();
		
				// Update app frame state
				++nFrameNumber;
				++nSwapchainIndex;
				
				// [DEBUG] pCommon->GetLogger()->info("HMD IPD is currently set to: {}", pXRProvider->Render()->GetCurrentIPD());
			}

			// (3) Process input
			if ( xrCurrentSessionState == XR_SESSION_STATE_FOCUSED && pXRProvider->Input() )
			{
				// 3.1 Sync data with runtime for all active action sets
				pXRProvider->Input()->SyncActiveActionSetsData();

				// 3.2 Process all received input states from previous sync call
				ProcessInputStates();

				// 3.3 Update controller and/or other action poses
				//     as we are not a pipelined app (single threaded), we're using one time period ahead of the current frame pose
				uint64_t nPredictedTime = pXRProvider->Render()->GetPredictedDisplayTime() + pXRProvider->Render()->GetPredictedDisplayPeriod();

				pXRProvider->Input()->GetActionPose( xrAction_PoseLeft, nPredictedTime, pCommon->XrLocation_Left() );
				pXRProvider->Input()->GetActionPose( xrAction_PoseRight, nPredictedTime, pCommon->XrLocation_Right() );

				// 3.4 Update any other input dependent poses (e.g. handtracking extension)
				if ( pCommon->ShouldDrawHandJoints() )
				{
					pXRProvider->Core()->GetExtHandTracking()->LocateHandJoints( XR_HAND_LEFT_EXT, pXRProvider->Core()->GetXRSpace(), nPredictedTime );
					pXRProvider->Core()->GetExtHandTracking()->LocateHandJoints( XR_HAND_RIGHT_EXT, pXRProvider->Core()->GetXRSpace(), nPredictedTime );
				}
			}
		}

		// SDL: present the back buffer
		SDL_GL_SwapWindow( pXRMirror->SDLWindow() );

		// SDL: Poll events
		PollSDLEvents();
	} 

	#pragma endregion SANDBOX_FRAME_LOOP

	// CLEANUP
	delete pXRMirror;
	delete pXRProvider;
	delete pCommon;

	return 0;
}

static void Callback_XR_Event( XrEventDataBuffer xrEvent )
{
	assert( pCommon && pCommon->GetLogger() );

	switch ( xrEvent.type )
	{
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			const XrEventDataSessionStateChanged &xrEventDataSessionStateChanged = *reinterpret_cast< XrEventDataSessionStateChanged * >( &xrEvent );
			
			pCommon->GetLogger()->info( "Session State changing from {} to {}", 
				OpenXRProvider::XrEnumToString( xrCurrentSessionState ), OpenXRProvider::XrEnumToString( xrEventDataSessionStateChanged.state ) );
			
			xrCurrentSessionState = xrEventDataSessionStateChanged.state;
			
			break;
	}
}

void PollSDLEvents() 
{
	while ( SDL_PollEvent( &sdlEvent ) )
	{
		switch ( sdlEvent.type )
		{
			case SDL_QUIT:
				bExitApp = true;
				break;
			case SDL_KEYDOWN:
				switch ( sdlEvent.key.keysym.sym )
				{
					case SDLK_1:
						pCommon->ShouldDrawHandJoints( pCommon->XRHandTracking()->IsActive() );
						eCurrentScene = SandboxCommon::SANDBOX_SCENE_SEA_OF_CUBES;

						pCommon->GetLogger()->info( "Switched to scene: Sea of Cubes" );
						break;

					case SDLK_2:
						pCommon->ShouldDrawHandJoints( pCommon->XRHandTracking()->IsActive() );
						eCurrentScene = SandboxCommon::SANDBOX_SCENE_HAND_TRACKING;

						pCommon->GetLogger()->info( "Switched to scene: Hand Tracking" );
						break;

					case SDLK_SPACE:
						// See if hand tracking is enabled
						if ( pCommon->XRHandTracking()->IsActive() )
							pCommon->ShouldDrawHandJoints( !pCommon->ShouldDrawHandJoints() );
						else
							pCommon->ShouldDrawHandJoints( false );

						pCommon->GetLogger()->info( "Hand joints will be rendered ({})", pCommon->ShouldDrawHandJoints() );
						break;

					case SDLK_ESCAPE:
						pCommon->GetLogger()->info( "Escape key pressed. Quitting Sandbox" );
						bExitApp = true;
						break;
				}
				break;
		}
	}
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
	pCommon = new SandboxCommon( APP_LOG_TITLE, pAppLogFile );

	// Create helpful title
	std::string sWindowTitle = APP_PROJECT_NAME;
	sWindowTitle += ". Press: [1] Sea of Cubes (default), [2] Hand Tracking, [SPACEBAR] Toggle hands [ESC] Quit";

	pXRMirror = new XRMirror_GL( pCommon, APP_MIRROR_WIDTH, APP_MIRROR_HEIGHT, sWindowTitle.c_str(), sCurrentPath, pAppLogFile );

	return 0;
}

void CreateInputActionBindings()
{
	// Valve Index
	OpenXRProvider::XRInputProfile_ValveIndex *pValveIndex = pXRProvider->Input()->ValveIndex();
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseLeft, pValveIndex->Hand_Left, pValveIndex->Pose_Grip, pValveIndex->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseRight, pValveIndex->Hand_Right, pValveIndex->Pose_Grip, pValveIndex->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pValveIndex->Hand_Left, pValveIndex->Button_Trigger_Click, pValveIndex->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pValveIndex->Hand_Right, pValveIndex->Button_Trigger_Click, pValveIndex->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pValveIndex->Hand_Left, pValveIndex->Output_Haptic, pValveIndex->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pValveIndex->Hand_Right, pValveIndex->Output_Haptic, pValveIndex->ActionBindings() );

	// Vive
	OpenXRProvider::XRInputProfile_HTCVive *pHTCVive = pXRProvider->Input()->HTCVive();
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseLeft, pHTCVive->Hand_Left, pHTCVive->Pose_Grip, pHTCVive->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseRight, pHTCVive->Hand_Right, pHTCVive->Pose_Grip, pHTCVive->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pHTCVive->Hand_Left, pHTCVive->Button_Trigger_Click, pHTCVive->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pHTCVive->Hand_Right, pHTCVive->Button_Trigger_Click, pHTCVive->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pHTCVive->Hand_Left, pHTCVive->Output_Haptic, pHTCVive->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pHTCVive->Hand_Right, pHTCVive->Output_Haptic, pHTCVive->ActionBindings() );

	// Oculus Touch
	OpenXRProvider::XRInputProfile_OculusTouch *pOculus = pXRProvider->Input()->OculusTouch();
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseLeft, pOculus->Hand_Left, pOculus->Pose_Grip, pOculus->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_PoseRight, pOculus->Hand_Right, pOculus->Pose_Grip, pOculus->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pOculus->Hand_Left, pOculus->Button_Trigger_Value, pOculus->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_SwitchScene, pOculus->Hand_Right, pOculus->Button_Trigger_Value, pOculus->ActionBindings() );

	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pOculus->Hand_Left, pOculus->Output_Haptic, pOculus->ActionBindings() );
	pXRProvider->Input()->CreateActionBinding( xrAction_Haptic, pOculus->Hand_Right, pOculus->Output_Haptic, pOculus->ActionBindings() );
}

void ProcessInputStates() 
{
	// Check if we need to respond to an action
	XrResult xrResult = pXRProvider->Input()->GetActionStateBoolean( xrAction_SwitchScene, &xrActionState_SwitchScene );
	if ( xrResult == XR_SUCCESS && xrActionState_SwitchScene.changedSinceLastSync && xrActionState_SwitchScene.currentState )
	{
		// Switch active scene
		eCurrentScene = eCurrentScene == SandboxCommon::SANDBOX_SCENE_HAND_TRACKING ? 
			SandboxCommon::SANDBOX_SCENE_SEA_OF_CUBES : SandboxCommon::SANDBOX_SCENE_HAND_TRACKING;

		// Apply haptic
		pXRProvider->Input()->GenerateHaptic( xrAction_Haptic, XR_MIN_HAPTIC_DURATION, 0.5f, XR_FREQUENCY_UNSPECIFIED );
		pCommon->GetLogger()->info( "Input Detected: Action Switch Scene ({}) last changed on ({}) nanoseconds", 
			( bool )xrActionState_SwitchScene.currentState, ( uint64_t )xrActionState_SwitchScene.lastChangeTime );
	}
}
