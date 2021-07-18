/* Copyright 2021 Rune Berg (GitHub: https://github.com/1runeberg, Twitter: https://twitter.com/1runeberg)
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *	 https://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

// OpenXR Provider includes
#include <OpenXRProvider.h>
#include <Utils.h>

/// Sandbox scenes
enum ESandboxScene
{
	SANDBOX_SCENE_SEA_OF_CUBES = 0,
	SANDBOX_SCENE_HAND_TRACKING = 1
};


// ** FUNCTIONS (GLOBAL) **//

/// Setup the sandbox application - windowing
int AppSetup();

// ** MEMBER VARIABLES (GLOBAL) **//

/// If hand joints should be rendered (hand tracking extension required)
bool bDrawHandJoints = false;

/// Current active scene
ESandboxScene eCurrentScene = SANDBOX_SCENE_SEA_OF_CUBES;

/// The width of the desktop window (XR Mirror)
int nScreenWidth = 1920;

/// The height of the desktop window (XR Mirror)
int nScreenHeight = 1080;

/// The number of images (Texture2D) that are in the swapchain created by the OpenXR runtime
uint32_t nSwapchainCapacity = 0;

/// The current index of the swapchain array
uint32_t nSwapchainIndex = 0;

/// The current frame number (internal to the sandbox application)
uint64_t nFrameNumber = 1;

/// The absolute path to the built Sandbox executable
std::wstring sCurrentPath;

/// The path and filename to write the OpenXR Provider library file to
char pAppLogFile[ MAX_PATH ] = "";

/// Pointer to the Utilities (Utils) class instantiated and used by the sandbox app (logging utility lives here)
Utils *pUtils = nullptr;

/// Stores the current OpenXR session state
XrSessionState xrCurrentSessionState = XR_SESSION_STATE_UNKNOWN;

/// Pointer to the XRProvider class of the OpenXR Provider library which handles all state, system and generic calls to the OpenXR runtime
OpenXRProvider::XRProvider *pXRProvider = nullptr;

/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
OpenXRProvider::XRExtVisibilityMask *pXRVisibilityMask = nullptr;

/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
OpenXRProvider::XRExtHandTracking *pXRHandTracking = nullptr;

/// Pointer to the XrExtHandJointsMotionRange class of the OpenXR Provider library which handles specifying motion ranges for the hand joints for runtimes that support it
OpenXRProvider::XRExtHandJointsMotionRange *pXRHandJointsMotionRange = nullptr;

