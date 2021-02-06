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

#include <IXRMirror.h>

// OpenGL includes
#include <glad/glad.h>


#define VIS_MASK_VERTEX_SHADER L"\\shaders\\vert-vismask.glsl"
#define VIS_MASK_FRAGMENT_SHADER L"\\shaders\\frag-vismask.glsl"

#define LIT_VERTEX_SHADER L"\\shaders\\vert-lit.glsl"
#define LIT_FRAGMENT_SHADER L"\\shaders\\frag-lit.glsl"

#define UNLIT_VERTEX_SHADER L"\\shaders\\vert-unlit.glsl"
#define UNLIT_FRAGMENT_SHADER L"\\shaders\\frag-unlit.glsl"

#define TEXTURED_VERTEX_SHADER L"\\shaders\\vert-textured.glsl"
#define TEXTURED_FRAGMENT_SHADER L"\\shaders\\frag-textured.glsl"

class XRMirror_GL : public IXRMirror
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] pCommon		Pointer to the sandbox common object
	/// @param[in] nWidth		The width of the desktop window (XR Mirror)
	/// @param[in] nHeight		The height of the desktop window (XR Mirror)
	/// @param[in] pTitle		The title text that will be displayed on the console window
	/// @param[in] sCurrentPath	The absolute path to where the application executable is
	/// @param[in] sLogFile		The absolute path and filename of the log file
	XRMirror_GL( SandboxCommon *pCommon, int nWidth, int nHeight, const char *pTitle, std::wstring sCurrentPath, const char *sLogFile );

	/// Class Destructor
	~XRMirror_GL();

	/// Setup the graphics api objects needed for rendering to this xr mirror
	/// @param[in] pRender		Pointer to the OpenXRProvider Library Render Manager
	const int Init( OpenXRProvider::XRRender *pRender ) override;

	/// Blit (copy) swapchain image that's rendered to the HMD to the desktop screen (XR Mirror)
	void BlitToWindow() override;

	/// Clear texture
	/// @param[in]	v4ClearColor	Color (r,g,b,a)
	void Clear( glm::vec4 v4ClearColor ) override;

	/// Render to the swapchain image (texture) that'll be rendered to the user's HMD and blitted (copied) to
	/// the Sandbox's window (XR Mirror)
	/// @param[in]	eCurrentScene		The current sandbox scene/level to be rendered
	/// @param[in] eEye					The eye (left/right) texture that will be rendered on
	/// @param[in] nSwapchainIndex		The index of the swapchain image (texture)
	void DrawFrame( SandboxCommon::ESandboxScene eCurrentScene, OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex ) override;

	/// Load a texture file from disk
	/// @param[in]	pTextureFile		The absolute file to the texture on disk
	/// @param[in]	nShader				The program id for the shader
	/// @param[in]	pSamplerParam		The texture sampler2d parameter name
	/// @param[in]	nMinFilter			(optional: GL_LINEAR)
	/// @param[in]	nMagnitudeFilter	(optional: GL_NEAREST)
	/// @param[in]	nWrapS				(optional: GL_REPEAT)
	/// @param[in]	nWrapT				(optional: GL_REPEAT)
	/// @return		unsigned int		The texture id of the loaded texture
	unsigned int LoadTexture(
		const wchar_t* pTextureFile, 
		GLuint nShader,
		const char* pSamplerParam,
		GLint nMinFilter = GL_LINEAR,
		GLint nMagnitudeFilter = GL_NEAREST,
		GLint nWrapS = GL_REPEAT,
		GLint nWrapT = GL_REPEAT);

	/// Draw a scene with a sea of instanced textured cubes in the scene based on the number of textures provided
	/// @param[in]	eEye				Current eye to render to
	/// @param[in]	nSwapchainIndex		Texture in the swapchain to render to
	/// @param[in]	cubeScale			Scale to apply to the cubes
	/// @param[in]	fSpacingPlane		The amount of spacing in between cubes in the x,z axis
	/// @param[in]	fSpacingHeight		The amount of spacing in between cubes in the y axis
	void DrawSeaOfCubesScene( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex, glm::vec3 cubeScale, float fSpacingPlane, float fSpacingHeight );

	/// Draw a scene with hand tracked joints and four large rotating cubes around the center of the playspace
	/// @param[in]	eEye				Current eye to render to
	/// @param[in]	nSwapchainIndex		Texture in the swapchain to render to
	void DrawHandTrackingScene( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex );

private:
	// ** MEMBER VARIABLES (PRIVATE) **/

	/// Pointer to the SDL context
	SDL_GLContext m_pGLWindowContext;

	/// The OpenGL Vertex Buffer Object (cube) used in rendering processes
	unsigned int cubeVBO;

	/// The OpenGL Vertex Buffer Object (controller) used in rendering processes
	unsigned int controllerVBO;

	/// The OpenGL Vertex Buffer Object (joint) used in rendering processes
	unsigned int jointVBO;

	/// Instanced cube data for its projection matrices
	unsigned int cubeInstanceDataVBO;

	/// Instanced controller mesh data for its projection matrices
	unsigned int controllerInstanceDataVBO;

	/// Instanced joint mesh data for its projection matrices
	unsigned int jointInstanceDataVBO;

	/// The OpenGL Vertex Array Object (cube) used in rendering processes
	unsigned int cubeVAO;

	/// The OpenGL Vertex Array Object (controller) used in rendering processes
	unsigned int controllerVAO;

	/// The OpenGL Vertex Array Object (joint) used in rendering processes
	unsigned int jointVAO;

	/// The OpenGL Vertex Array Object (light source) used in rendering processes
	unsigned int lightVAO;

	/// The OpenGL Element Buffer Object (cube) used in rendering processes
	unsigned int cubeEBO;

	/// The OpenGL Frame Buffer Object (hmd texture) used in rendering processes
	unsigned int FBO;

	/// The shader program ID for visibility mask that will be applied on all textures
	GLuint nShaderVisMask;

	/// The shader program ID for lit objects
	GLuint nShaderLit;

	/// The shader program ID for unlit
	GLuint nShaderUnlit;

	/// The shader program ID for textured
	GLuint nShaderTextured;

	/// OpenGL Sea of Cubes textures (texture2d)
	std::vector< unsigned int > m_vCubeTextures;

	/// A OpenGL map of color texture id to depth texture id
	std::map< uint32_t, uint32_t > m_mapColorDepth;


	// ** FUNCTIONS (PRIVATE) **/

	/// Draw the controller meshes for each hand
	/// @param[in]	eEye				Current eye to render to
	/// @param[in]	nSwapchainIndex		Texture in the swapchain to render to
	void DrawControllers( OpenXRProvider::EXREye eEye, glm::mat4 eyeView );

	/// Draw the hand joints (hand tracking runtime support required)
	/// @param[in]	eEye				Current eye to render to
	/// @param[in]	nSwapchainIndex		Texture in the swapchain to render to
	void DrawHandJoints( OpenXRProvider::EXREye eEye, glm::mat4 eyeView );

	/// Draw a single cube
	/// @param[in]	eEye					Current eye to render to
	/// @param[in]  nSwapchainIndex			Texture in the swapchain to render to
	/// @param[in]  eyeView					The eye view matrix
	/// @param[in]	nTexture				The texture id to use for the cube
	/// @param[in]	cubePosition			The position of the cube in world space
	/// @param[in]	cubeScale				The scale of the cube in world space
	/// @param[in]	cubeRotationOverTime	Amount of rotation of the cube over time
	void DrawCube( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex, glm::mat4 eyeView, unsigned int nTexture, glm::vec3 cubePosition, glm::vec3 cubeScale, glm::vec3 cubeRotationOverTime );

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
	void FillEyeMVP( glm::mat4 *vEyeProjections, glm::mat4 eyeView, OpenXRProvider::EXREye eEye, uint32_t nCubeIndex, glm::vec3 cubePosition, glm::vec3 cubeScale = glm::vec3( 0.f ) );

	/// Fill the array of model-view-projection matrices that will be applied on a  rotating cube in a single frame and eye.
	/// This is filled in as instanced variables to the shader
	/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
	/// @param[out] vEyeProjections		Array of eye view projection matrices that will be applied to each cube model vertex
	/// @param[in]	eyeView				The current eye view (camera) projection matrix
	/// @param[in]	nCubeIndex			Index of the current cube being rendered (within the "sea")
	/// @param[in]	nShader				Shader program id to use for the cube
	/// @param[in]	nTexture			The texture id to use for the cube
	/// @param[in]	cubePosition		The position of the cube
	/// @param[in]	cubeRotation		The amount of rotation applied to the cube over time
	/// @param[in]	cubeScale			The scale of the cube
	void FillEyeMVP_RotateOverTime(
		glm::mat4 *vEyeProjections,
		glm::mat4 eyeView,
		OpenXRProvider::EXREye eEye,
		uint32_t nCubeIndex,
		unsigned nTexture,
		glm::vec3 cubePosition,
		glm::vec3 cubeRotation,
		glm::vec3 cubeScale );

	/// Get the corresponding depth texture for a given color texture, create one if it doesn't exist
	/// If the runtime supports the XR_KHR_composition_layer_depth extension, the runtime provided
	/// depth texture will be used instead
	/// @param[in]	nTexture			Color texture to match with a depth texture to return
	/// @param[in]	nMinFilter			(Optional: GL_LINEAR)
	/// @param[in]	nMagnitudeFilter	(Optional: GL_NEAREST)
	/// @param[in]	nWrapS				(Optional: GL_CLAMP_TO_EDGE)
	/// @param[in]	nWrapT				(Optional: GL_CLAMP_TO_EDGE)
	/// @param[in]	nDepthFormat		(Optional: GL_DEPTH_COMPONENT24)
	/// @return		uin32_t			Depth texture OpenGL id
	uint32_t GetDepth( uint32_t nTexture, GLint nMinFilter = GL_LINEAR, GLint nMagnitudeFilter = GL_NEAREST, GLint nWrapS = GL_REPEAT, GLint nWrapT = GL_REPEAT, GLint nDepthFormat = GL_DEPTH_COMPONENT24 );

	
	/// Loads and compiles a GLSL shader file from the disk
	/// @param[in] eShaderType	Fragment or Vertex shader
	/// @param[in] pFilePath	The absolute path and filename of the shader file to load and compile
	/// @return					The shader id of the shader file. 0 if any errors were encountered
	GLuint LoadShaderFromDisk( GLenum eShaderType, const char *pFilePath );

	/// Creates a shader program from provider vertex and fragment shader files on disk
	/// @param[in] pVertexShaderFile			The absolute path and filename of the vertex shader file (glsl)
	/// @param[in] pFragmentShaderFile			The absolute path and filename of the fragment shader file (glsl)
	/// @return									The OpenGL id of the generated shader program
	GLuint CreateShaderProgram( const wchar_t *pVertexShaderFile, const wchar_t *pFragmentShaderFile );
};
