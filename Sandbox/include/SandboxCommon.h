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

#define _CRT_SECURE_NO_WARNINGS 1

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX_STRING_LEN 512

// Logger includes
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// OpenGL includes
#include <glad/glad.h>

// Third party includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb/stb_image.h>

// OpenXR Provider includes
#include <OpenXRProvider.h>

class SandboxCommon
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] pLogTitle	The title text that will be displayed on the logs and console window
	/// @param[in] pLogFile		The absolute path and filename of the log file
	SandboxCommon( const char *pLogTitle, const char *pLogFile );

	/// Class Destructor
	~SandboxCommon();

	/// Sandbox scenes
	enum ESandboxScene
	{
		SANDBOX_SCENE_SEA_OF_CUBES = 0,
		SANDBOX_SCENE_HAND_TRACKING = 1
	};

	// Getter for the logger
	/// @return		The shared pointer to the logger object
	std::shared_ptr< spdlog::logger > GetLogger() { return m_pLogger; }

	/// Calculate the eye projection matrix for this OpenXR session.
	/// This can be calculated only once per application run
	/// @param[in]	pRenderManager		Pointer to OpenXR Provider library's Render manager
	/// @param[in]	eyeFoV				The eye's field of view angles in radians
	/// @param[in]  fNear				Near plane distance (must be greater than 0 to use depth testing in OpenGL)
	/// @param[in]	fFar				Far plane distance
	glm::mat4 GetEyeProjection( XrFovf eyeFoV, float fNear = 0.1, float fFar = 1000.f );

	/// Get the inverse of a given projection matrix
	/// @param[in]	pProjectionMatrix	The matrix to invert
	/// @param[out]	glm::mat4			The inverted matrix
	glm::mat4 InvertMatrix( const glm::mat4 pProjectionMatrix );

	/// Checks if hand joints should be drawn (handtracking extension required)
	/// @return		bool	If hand joints should be drawn
	bool ShouldDrawHandJoints() const { return m_bDrawHandJoints; }

	/// Sets if hand joints should be drawn (handtracking extension required)
	/// @param[in]	val		If hand joints should be drawn
	void ShouldDrawHandJoints( bool val ) { m_bDrawHandJoints = val; }

	/// Checks if the eye projections for both left and right eyes have been set, this saves perf as they only need to be calculated once per frame
	/// @return		glm::m_bEyeProjectionsSet	If the eye projections for both left and right eyes have been set
	bool EyeProjectionsSet() const { return m_bEyeProjectionsSet; }

	/// Checks if the eye projections for both left and right eyes have been set, this saves perf as they only need to be calculated once per frame
	/// @param[in]		val		Boolean to indicate if eye projections for both left and right eyes have been set
	void EyeProjectionsSet( bool val ) { m_bEyeProjectionsSet = val; }

	/// Getter for the left eye projection 
	/// @return		glm::mat4		The left eye view (camera) projection matrix for this frame
	glm::mat4 EyeProjectionLeft() const { return m_EyeProjectionLeft; }

	/// Setter for the left eye projection
	/// @param[in]		glm::mat4		The left eye view (camera) projection matrix for this frame
	void EyeProjectionLeft( glm::mat4 val ) { m_EyeProjectionLeft = val; }

	/// Getter for the right eye projection 
	/// @return		glm::mat4		The right eye view (camera) projection matrix for this frame
	glm::mat4 EyeProjectionRight() const { return m_EyeProjectionRight; }

	/// Setter for the right eye projection
	/// @param[in]		glm::mat4		The right eye view (camera) projection matrix for this frame
	void EyeProjectionRight( glm::mat4 val ) { m_EyeProjectionRight = val; }

	/// Getter for the pointer to the visibility mask extension object
	/// @return		XRExtVisibilityMask		Pointer to the visibility mask extension object
	OpenXRProvider::XRExtVisibilityMask *XRVisibilityMask() const { return m_pXRVisibilityMask; }

	/// Setter for the pointer to the visibility mask extension object
	/// @param[in]	val		Pointer to the visibility mask extension object
	void XRVisibilityMask( OpenXRProvider::XRExtVisibilityMask *val ) { m_pXRVisibilityMask = val; }

	/// Getter for the pointer to the hand tracking extension object
	/// @return		XRExtVisibilityMask		Pointer to the hand tracking extension object
	OpenXRProvider::XRExtHandTracking *XRHandTracking() const { return m_pXRHandTracking; }

	/// Setter for the pointer to the hand tracking extension object
	/// @param[in]	val		Pointer to the hand tracking extension object
	void XRHandTracking( OpenXRProvider::XRExtHandTracking *val ) { m_pXRHandTracking = val; }

	/// Pointer of the left controller's current space location
	/// @return		XrSpaceLocation		Pointer of the left controller's current space location
	XrSpaceLocation *XrLocation_Left() { return &xrLocation_Left; }

	/// Pointer of the right controller's current space location
	/// @return		XrSpaceLocation		Pointer of the right controller's current space location
	XrSpaceLocation *XrLocation_Right() { return &xrLocation_Right; }

	/// Pointer of the left controller's current space velocity
	/// @return		XrSpaceVelocity		Pointer of the left controller's current space velocity
	XrSpaceVelocity *XrVelocity_Left() { return &xrVelocity_Left; }

	/// Pointer of the right controller's current space velocity
	/// @return		XrSpaceVelocity		Pointer of the right controller's current space velocity
	XrSpaceVelocity *XrVelocity_Right() { return &xrVelocity_Right; }

	/// Visibility mask vertices (left)
	/// @return		*MaskIndices_Left		Array of mask vertices (left eye)
	std::vector< float > *MaskVertices_Left() { return &vMaskVertices_L; }

	/// Visibility mask vertices (right)
	/// @return		*MaskIndices_Right		Array of mask vertices (right eye)
	std::vector< float > *MaskVertices_Right() { return &vMaskVertices_R; }


	/// Visibility mask indices (left)
	/// @return		*MaskIndices_Left		Array of mask indices (left eye)
	std::vector< uint32_t > *MaskIndices_Left() { return &vMaskIndices_L; }

	/// Visibility mask indices (right)
	/// @return		*MaskIndices_Right		Array of mask indices (right eye)
	std::vector< uint32_t > *MaskIndices_Right() { return &vMaskIndices_R; }


	// ** SIMPLE MESHES ** //

	/// Controller mesh vertices and colors (x, y, z, r, g, b)
	float vControllerMesh[144] =
	{
		0.0f, 0.0f, -0.4f,		0.0f, 1.0f, 0.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.0f,  0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -0.4f,		0.0f, 1.0f, 0.0f,

		0.0f, 0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.4f,		0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.4f,		0.0f, 1.0f, 0.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f, 
		0.0f, 0.2f,	0.0f,		1.0f, 1.0f, 1.0f,

		0.0f, 0.0f, -0.4f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		1.0f, 1.0f, 1.0f,
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.2f, 0.0f, 0.1f,		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, -0.4f,		1.0f, 1.0f, 1.0f,

		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.4f,		0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.4f,		0.0f, 1.0f, 0.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f, 
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f
	};

	/// Joint mesh vertices and colors (x, y, z, r, g, b)
	float vJointMesh[144] 
	{
		0.0f, 0.0f, -0.4f,		0.0f, 1.0f, 0.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.0f,  0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -0.4f,		0.0f, 1.0f, 0.0f,

		0.0f, 0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.2f,		0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.2f,		0.0f, 1.0f, 0.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f, 
		0.0f, 0.2f,	0.0f,		1.0f, 1.0f, 1.0f,

		0.0f, 0.0f, -0.4f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		1.0f, 1.0f, 1.0f,
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.2f, 0.0f, 0.1f,		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, -0.4f,		1.0f, 1.0f, 1.0f,

		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.2f,		0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.2f,		0.0f, 1.0f, 0.0f,
		-0.2f, 0.0f, 0.1f,		0.0f, 1.0f, 0.0f, 
		0.0f, -0.2f, 0.0f,		1.0f, 1.0f, 1.0f
	};


	/// Cube vertices and texture coordinates (x, y, z, s ,t)
	float vCube[180] =
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

  private:
	// ** MEMBER VARIABLES (PRIVATE) **/

	/// The log title
	const char *m_pLogTitle;

	/// The logger object
	std::shared_ptr< spdlog::logger > m_pLogger;

	/// If hand joints should be rendered (hand tracking extension required)
	bool m_bDrawHandJoints = false;

	/// Keeps track of whether both left and right eye projection matrices
	/// have been set. Ensure we calculate these only once per frame
	bool m_bEyeProjectionsSet = false;

	/// The left eye view (camera) projection matrix
	glm::mat4 m_EyeProjectionLeft;

	/// The right eye view (camera) projection matrix
	glm::mat4 m_EyeProjectionRight;

	/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
	OpenXRProvider::XRExtVisibilityMask *m_pXRVisibilityMask = nullptr;

	/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
	OpenXRProvider::XRExtHandTracking *m_pXRHandTracking = nullptr;

	// Space location of the left controller
	XrSpaceLocation xrLocation_Left { XR_TYPE_SPACE_LOCATION };
	
	// Space location of the right controller
	XrSpaceLocation xrLocation_Right { XR_TYPE_SPACE_LOCATION };

	// Space velocity of the left controller
	XrSpaceVelocity xrVelocity_Left { XR_TYPE_SPACE_VELOCITY };
	
	// Space velocity of the left controller
	XrSpaceVelocity xrVelocity_Right { XR_TYPE_SPACE_VELOCITY };

	// ** Visibility Mask **/

	/// Indices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< uint32_t > vMaskIndices_L;

	/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< uint32_t > vMaskIndices_R;

	/// Vertices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< float > vMaskVertices_L;

	/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< float > vMaskVertices_R;


	// ** FUNCTIONS (PRIVATE) **/

};
