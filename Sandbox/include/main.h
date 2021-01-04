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

// Sandbox includes
#include <XRMirror.h>

// OpenXR Provider includes
#include <OpenXRProvider.h>
#include <XRRenderManager.h>


// ** FUNCTIONS (GLOBAL) **//

/// Setup the sandbox application - windowing and opengl support
int AppSetup();

/// Setup the OpenGL objects (VAO, VBO, FBO) needed for rendering to textures from the OpenXR runtime
int GraphicsAPIObjectsSetup();


/// Blit (copy) swapchain image that's rendered to the HMD to the desktop screem (XR Mirror)
void BlitToWindow();

/// Render to the swapchain image (texture) that'll be rendered to the user's HMD and blitted (copied) to
/// the Sandbox's window (XR Mirror)
/// @param[in] pXRRenderManager		Pointer to the XR Render Manager object
/// @param[in] eEye					The eye (left/right) texture that will be rendered on
/// @param[in] nSwapchainIndex		The index of the swapchain image (texture)
/// @param[in] FBO					The OpenGL Frame Buffer Object needed to draw to a Texture2D
/// @param[in] nShaderVisMask		The OpenGL shader program that will be used for the Visibility Mask
void DrawFrame
(
	OpenXRProvider::XRRenderManager *pXRRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	unsigned int FBO,
	GLuint nShaderVisMask
);

/// Submit a texture to the compositor
/// @param[in] xrEventType	The type of event that triggered the callback (e.g. Session state change, reference space changed, etc)
/// @param[in] xrEventData	The data payload of the event (e.g. The new session state, etc)
static void Callback_XR_Session_Changed( const OpenXRProvider::EXREventType xrEventType, const OpenXRProvider::EXREventData xrEventData );


// ** MEMBER VARIABLES (GLOBAL) **//

/// The OpenGL Frame Buffer Object used in rendering processes
unsigned int FBO;

/// The OpenGL Vertex Array Object used in rendering processes
unsigned int VAO;

/// The OpenGL Vertex Buffer Object used in rendering processes
unsigned int VBO;

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

/// The shader program ID for visibility mask that will be applied on all textures
GLuint nShaderVisMask;

/// The absolute path to the built Sandbox executable
std::wstring sCurrentPath;

/// The path and filename to write the OpenXR Provider library file to
char pAppLogFile [ MAX_PATH ] = "";

/// Indices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
std::vector< uint32_t > vMaskIndices_L;

/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
std::vector< uint32_t > vMaskIndices_R;

/// Vertices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
std::vector< float > vMaskVertices_L;

/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
std::vector< float > vMaskVertices_R;

/// Pointer to the Utilities (Utils) class instantiated and used by the sandbox app (logging utility lives here)
Utils *pUtils = nullptr;

/// Pointer to the XRMirror class that handles the desktop window where textures sent to the HMD are blitted (copied) to
XRMirror *pXRMirror = nullptr;

/// Stores the current OpenXR session state
OpenXRProvider::EXREventData xrCurrentSessionState;

/// Pointer to the XRProvider class of the OpenXR Provider library which handles all state, system and generic calls to the OpenXR runtime
OpenXRProvider::XRProvider *pXRProvider = nullptr;

/// Pointer to the XRRenderManager class of the OpenXR Provider library which handles all OpenXR rendering
OpenXRProvider::XRRenderManager *pXRRenderManager = nullptr;

/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
OpenXRProvider::XRExtVisibilityMask *pXRVisibilityMask = nullptr;
