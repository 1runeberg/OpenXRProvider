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
void DrawFrame(
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

unsigned int cubeInstanceDataVBO;

/// The OpenGL Vertex Buffer Object (cube) used in rendering processes
unsigned int cubeVBO;

/// The OpenGL Vertex Array Object (cube) used in rendering processes
unsigned int cubeVAO;

/// The OpenGL Vertex Array Object (light source) used in rendering processes
unsigned int lightVAO;

/// The OpenGL Element Buffer Object (cube) used in rendering processes
unsigned int cubeEBO;

/// The OpenGL Frame Buffer Object (hmd texture) used in rendering processes
unsigned int FBO;

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

/// The shader program ID for lit objects
GLuint nShaderLit;

/// The shader program ID for unlit
GLuint nShaderUnlit;

/// The shader program ID for textured
GLuint nShaderTextured;

/// The absolute path to the built Sandbox executable
std::wstring sCurrentPath;

/// The path and filename to write the OpenXR Provider library file to
char pAppLogFile[ MAX_PATH ] = "";

/// Sea of Cubes textures
std::vector< unsigned int > vSeaOfCubesTextures;

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


/// -------------------------------
/// SEA OF CUBES
/// -------------------------------

/// Cube vertices and texture coordinates (x, y, z, s ,t)
float vCube[ ] =
{
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f
};

/// Keeps track of whether both left and right eye projection matrices
/// have been set. Ensure we calculate these only once per frame
bool m_bEyeProjectionsSet = false;

/// The left eye view (camera) projection matrix
glm::mat4 m_EyeProjectionLeft;

/// The right eye view (camera) projection matrix
glm::mat4 m_EyeProjectionRight;

/// A OpenGL map of color texture id to depth texture id
std::map< uint32_t, uint32_t > m_mapColorDepth;

/// Draw a sea of instanced textured cubes in the scene based on the number of textures provided
/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
/// @param[in]	eEye				Current eye to render to
/// @param[in]	nSwapchainIndex		Texture in the swapchain to render to
/// @param[in]	VBO					Vertex buffer object that contains instanced data for the projection matrices
/// @param[in]	VAO					Vertex Array Object containing the cube vertices
/// @param[in]	cubeScale			Scale to apply to the cubes
/// @param[in]	vTextures			Array of textures that will be applied on the cubes
/// @param[in]	fSpacingPlane		The amount of spacing in between cubes in the x,z axis
/// @param[in]	fSpacingHeight		The amount of spacing in between cubes in the y axis
void DrawSeaOfCubes(
	OpenXRProvider::XRRenderManager *pRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	GLuint nShader,
	unsigned int VBO,
	unsigned int VAO,
	glm::vec3 cubeScale,
	std::vector< unsigned int > vTextures,
	float fSpacingPlane,
	float fSpacingHeight );

/// Fill the array of model-view-projection matrices that will be applied on a cube in a single frame and eye.
/// This is filled in as instanced variables to the shader
/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
/// @param[out] vEyeProjections		Array of eye view projection matrices that will be applied to each cube model vertex
/// @param[in]	eyeView				The current eye view (camera) projection matrix
/// @param[in]	nCubeIndex			Index of the current cube being rendered (within the "sea")
/// @param[in]	nShader				Shader program id to use for the cube
/// @param[in]	nTexture			The texture id to use for the cube
/// @param[in]	cubePosition		The position of the cube
/// @param[in]	cubeScale			The scale of the cube
void FillEyeMVP(
	OpenXRProvider::XRRenderManager *pRenderManager,
	glm::mat4 *vEyeProjections,
	glm::mat4 eyeView,
	OpenXRProvider::EXREye eEye,
	uint32_t nCubeIndex,
	GLuint nShader,
	unsigned int nTexture,
	glm::vec3 cubePosition,
	glm::vec3 cubeScale );

/// Draw a single cube (when not using instancing, will require a different shader)
/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
/// @param[in]	eEye				Current eye to render to
/// @param[in]  nSwapchainIndex		Texture in the swapchain to render to
/// @param[in]	nShader				Shader program id to use for the cube
/// @param[in]	nTexture			The texture id to use for the cube
/// @param[in]	VAO					Vertex Array Object containing the cube vertices
void DrawCube(
	OpenXRProvider::XRRenderManager* pRenderManager,
	OpenXRProvider::EXREye eEye,
	uint32_t nSwapchainIndex,
	GLuint nShader,
	unsigned int nTexture,
	unsigned int VAO,
	glm::vec3 cubePosition);

/// Calculate the eye projection matrix for this OpenXR session.
/// This can be calculated only once per application run
/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
/// @param[in]	eyeFoV				The eye's field of view angles in radians 
/// @param[in]  fNear				Near plane distance (must be greater than 0 to use depth testing in OpenGL)
/// @param[in]	fFar				Far plane distance
glm::mat4 GetEyeProjection( 
	OpenXRProvider::XRRenderManager *pRenderManager, 
	OpenXRProvider::XRFoV eyeFoV, 
	float fNear = 0.1, 
	float fFar = 1000.f );

/// Getter for the left eye projection (for use in future refactoring)
/// @return		glm::mat4		The left eye view (camera) projection matrix for this frame
glm::mat4 GetEyeProjectionLeft() { return m_EyeProjectionLeft; }

/// Getter for the right eye projection (for use in future refactoring)
/// @return		glm::mat4		The right eye view (camera) projection matrix for this frame
glm::mat4 GetEyeProjectionRight() { return m_EyeProjectionRight; }


/// Get the corresponding depth texture for a give color texture, create one if it doesn't exist
/// If the runtime supports the XR_KHR_composition_layer_depth extension, the runtime provided
/// depth texture will be used instead
/// @param[in]	nTexture			Color texture to match with a depth texture to return
/// @param[in]	nMinFilter			(Optional: GL_LINEAR)
/// @param[in]	nMagnitudeFilter	(Optional: GL_NEAREST)
/// @param[in]	nWrapS				(Optional: GL_CLAMP_TO_EDGE)
/// @param[in]	nWrapT				(Optional: GL_CLAMP_TO_EDGE)
/// @param[in]	nDepthFormat		(Optional: GL_DEPTH_COMPONENT24)
/// @return		uin32_t			Depth texture OpenGL id
uint32_t GetDepth(
	uint32_t nTexture,
	GLint nMinFilter = GL_LINEAR,
	GLint nMagnitudeFilter = GL_NEAREST,
	GLint nWrapS = GL_REPEAT,
	GLint nWrapT = GL_REPEAT,
	GLint nDepthFormat = GL_DEPTH_COMPONENT24 );


/// Get the inverse of a given projection matrix
/// @param[in]	pProjectionMatrix	The matrix to invert
/// @param[out]	glm::mat4			The inverted matrix
glm::mat4 InvertMatrix( const glm::mat4 pProjectionMatrix );
