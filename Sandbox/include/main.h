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

//#define XR_GRAPHICS_API_OPENGL 1
#define XR_GRAPHICS_API_DX11 1

#ifdef XR_GRAPHICS_API_OPENGL
// Sandbox includes
#include <XRMirror_GL.h>

#elif XR_GRAPHICS_API_DX11
#include <XRMirror_DX11.h>
#endif

// OpenXR Provider includes
#include <OpenXRProvider.h>

// ** FUNCTIONS (GLOBAL) **//

/// Setup the sandbox application - windowing and opengl support
int AppSetup();

/// Callback for session state changes TODO: Use native OpenXR type
/// @param[in] xrEventType	The type of event that triggered the callback (e.g. Session state change, reference space changed, etc)
/// @param[in] xrEventData	The data payload of the event (e.g. The new session state, etc)
static void Callback_XR_Event( XrEventDataBuffer xrEvent );

// Poll for SDL events
static void PollSDLEvents();


// ** MEMBER VARIABLES (GLOBAL) **//

/// If app should exit gracefully
bool bExitApp = false;

/// Current active scene
SandboxCommon::ESandboxScene eCurrentScene = SandboxCommon::SANDBOX_SCENE_SEA_OF_CUBES;

/// The number of images (Texture2D) that are in the swapchain created by the OpenXR runtime
uint32_t nSwapchainCapacity = 0;

/// The current index of the swapchain array 
uint32_t nSwapchainIndex = 0;

/// The current frame number (internal to the sandbox application)
uint64_t nFrameNumber = 1;

/// Sdl event returned by polling sdl
SDL_Event sdlEvent;

/// The absolute path to the built Sandbox executable
std::wstring sCurrentPath;

/// The path and filename to write the OpenXR Provider library file to
char pAppLogFile[ MAX_PATH ] = "";

/// Pointer to the Utilities (SandboxCommon) class instantiated and used by the sandbox app (logging utility lives here)
SandboxCommon *pCommon = nullptr;

#ifdef XR_GRAPHICS_API_OPENGL
/// Pointer to the XRMirror_GL class that handles the desktop window where textures sent to the HMD are blitted (copied) to
XRMirror_GL *pXRMirror = nullptr;

#elif XR_GRAPHICS_API_DX11
/// Pointer to the XRMirror_DX11 class that handles the desktop window where textures sent to the HMD are blitted (copied) to
XRMirror_DX11 *pXRMirror = nullptr;

#endif

/// Stores the current OpenXR session state
XrSessionState xrCurrentSessionState = XR_SESSION_STATE_UNKNOWN;

/// Pointer to the XRProvider class of the OpenXR Provider library which handles all state, system and generic calls to the OpenXR runtime
OpenXRProvider::XRProvider *pXRProvider = nullptr;

/// -------------------------------
/// INPUTS
/// -------------------------------

/// Action set to use for this sandbox
XrActionSet xrActionSet_Main;

// Action states
XrActionStateBoolean xrActionState_SwitchScene;
XrActionStatePose xrActionState_PoseLeft, xrActionState_PoseRight;

// Actions
XrAction xrAction_SwitchScene, xrAction_Haptic;
XrAction xrAction_PoseLeft, xrAction_PoseRight;


/// Generate all input action bindings to multiple controllers
void CreateInputActionBindings();

/// Process all the input states after syncing with runtime
void ProcessInputStates();
