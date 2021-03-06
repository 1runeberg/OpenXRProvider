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

#include <XRCore.h>

// Supported Extensions
#include <extensions/XRExtVisibilityMask.h>

namespace OpenXRProvider
{
	class XRRender
	{
		// OpenXR view, in VR we have one for each eye
		static const uint32_t k_nVRViewCount = 2;

		static const float k_fMedianIPD;
		static const float k_fMinIPD;
		static const float k_fTau;

	  public:
		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		/// @param[in] pXRCore			Pointer to the XR Core system
		/// @param[in] xrRenderInfo		Information needed to create swapchain textures that the app will render to
		XRRender( XRCore *pXRCore, XRRenderInfo xrRenderInfo );

		/// Class Destructor
		~XRRender();

		/// Getter for the OpenXR view configurations (in VR, this is one for each eye)
		/// @return		The OpenXR view configuration
		std::vector< XrViewConfigurationView > GetConfigViews() { return m_vXRViewConfigs; }

		/// Getter for depth handling indicator
		/// @return		If depth textures are also generated/handled
		bool GetDepthHandling() { return m_bDepthHandling; }

		/// Retrieve the graphics api dependent class that's maintained by the XR Provider and created during OpenXR initialization
		/// @return		The GraphicsAPI object that can deal with graphics api dependent transactions (e.g OpenGL, DirectX, Vulkan)
		XRGraphicsAPI *GetGraphicsAPI();

		/// Retrieve the current IPD of the user's HMD (this can change at any time depending on the hardware. IPD is calculated when this call is made)
		/// @return		The user's IPD
		float GetCurrentIPD();

		/// Get the projection matrix for the given eye
		/// @param[in]	eEye				Eye
		/// @param[in]	bInvert				(Optional: false) Whether to invert the matrix or not, set to true for OpenGL
		/// @param[out]	mProjectionMatrix	The projection matrix to write to
		void GetEyeProjection( EXREye eEye, std::vector<float> *mProjectionMatrix, bool bInvert = false );
		
		/// Retrieve the current position and orientation of the headset
		/// @return		The current orientation and position of the user's HMD in their tracked space
		XrPosef GetHMDPose();

		/// Getter for the current HMD State (eye poses, fov, if hmd is tracked, etc)
		/// @return		The eye poses, fov, tracking status, etc of the user's HMD
		XRHMDState *GetHMDState() { return m_pXRHMDState; }

		/// Getter for the swapchain images (color texture2d) that the application must use to render frames to and bound for the hmd
		/// @return		Array of image textures (color texture2d) that is used by the OpenXR runtime to render images to the user's hmd
		std::vector< XrSwapchain > GetSwapchainColor() { return m_vXRSwapChainsColor; }

		/// Getter for the swapchain images (depth texture2d) that the application must use to render frames to and bound for the hmd
		/// @return		Array of image textures (depth texture2d) that is used by the OpenXR runtime to render images to the user's hmd 
		std::vector< XrSwapchain > GetSwapchainDepth() { return m_vXRSwapChainsDepth; }
		
		/// Getter for the current frame's predicted display time
		/// @return		The predicted display time in nanoseconds
		XrTime GetPredictedDisplayTime() const { return m_xrPredictedDisplayTime; }

		/// Getter for the current frame's predicted display period
		/// @return		The predicted display period in nanoseconds for predicting display times beyond the next m_xrPredictedDisplayTime
		XrDuration GetPredictedDisplayPeriod() const { return m_xrPredictedDisplayPeriod; }

		/// Getter for the current texture array size for each image in the swapchain
		/// @return		The size of the the texture array for each image in the swapchain
		uint32_t GetTextureArraySize() { return m_nTextureArraySize; }

		/// Getter for the current graphics api dependent texture format for images in the swapchain
		/// @return		The graphics api dependent format in int64_t (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		int64_t GetTextureFormat() { return m_nTextureFormat; }

		/// Getter for the texture height for images in the swapchain
		/// @return		Texture height in pixels
		uint32_t GetTextureHeight() { return m_nTextureHeight; }

		/// Getter for the texture Mip count for images in the swapchain
		/// @return		The swapchain textures' Mip count
		uint32_t GetTextureMipCount() { return m_nTextureMipCount; }

		/// Getter for the texture width for images in the swapchain
		/// @return		Texture width in pixels
		uint32_t GetTextureWidth() { return m_nTextureWidth; }

		/// Getter for the visibility mask extension object (if supported and activated by the runtime
		/// @return		The visibility mask extension object generated during OpenXR initialization if the currently active runtime supports it
		XRExtVisibilityMask *GetXRVisibilityMask() { return m_pXRVisibilityMask; }

		/// Process frame, call after app has rendered to the appropriate swapchain image(s)
		bool ProcessXRFrame();

	  private:
		// ** FUNCTIONS (PRIVATE) **/

		/// Create the swapchain(s) that will hold OpenXR runtime created images (texture2d)
		/// @param[in] bIsDepth		If depth textures need to be created along with the color textures
		void GenerateSwapchains( bool bIsDepth = false );

		/// Destroy swapchain(s) that holds OpenXR runtime created images (texture2d)
		/// @param[in] vXRSwapchains	Array that holds the active swapchains
		void DestroySwapchains( std::vector< XrSwapchain > &vXRSwapchains );
		
		/// Reset all values in the hmd state (e.g. eye poses, fov, tracking status, etc)
		void ResetHMDState();

		/// Set values in the hmd state (e.g. eye poses, fov, tracking status, etc)
		/// @param[in] eEye			Which eye status to update
		/// @param[in] pEyeState	Pointer to the eye state data from OpenXR
		void SetHMDState( EXREye eEye, XREyeState *pEyeState );

		/// Set graphics api dependent format that will be used for creating the swapchain images (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		/// @param[in] vAppTextureFormats	Array of texture color formats requested by the app in order of preference
		/// @param[in] vAppDepthFormats		Array of texture depth formats requested by the app in order of preference
		void SetSwapchainFormat( std::vector< int64_t > vAppTextureFormats, std::vector< int64_t > vAppDepthFormats);


		// ** MEMBER VARIABLES (PRIVATE) **/

		/// If depth textures are active/being handled
		bool m_bDepthHandling = false;

		/// The size of the the texture array for each image in the swapchain
		uint32_t m_nTextureArraySize = 1;

		/// The swapchain textures' Mip count
		uint32_t m_nTextureMipCount = 1;

		/// Texture width of swapchain images in pixels
		uint32_t m_nTextureWidth = 1920;

		/// Texture height of swapchain images in pixels
		uint32_t m_nTextureHeight = 1080;

		/// The graphics api dependent color format in int64_t (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		int64_t m_nTextureFormat = 0;

		/// The graphics api dependent depth format in int64_t (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		int64_t m_nDepthFormat = 0;
		
		/// The logger object
		std::shared_ptr< spdlog::logger > m_pXRLogger;

		/// Array of image textures (color texture2d) that is used by the OpenXR runtime to render images to the user's hmd
		std::vector< XrSwapchain > m_vXRSwapChainsColor;

		/// Array of image textures (depth texture2d) that is used by the OpenXR runtime to render images to the user's hmd 
		std::vector< XrSwapchain > m_vXRSwapChainsDepth;

		/// Array of OpenXR views (for VR, there is one for each eye)
		std::vector< XrView > m_vXRViews;

		/// Array of OpenXR views (for VR, there is one for each eye)
		std::vector< XrViewConfigurationView > m_vXRViewConfigs;

		///  The visibility mask extension object generated during OpenXR initialization if the currently active runtime supports it
		XRExtVisibilityMask *m_pXRVisibilityMask = nullptr;

		/// Pointer to the hmd state which contains info on eye poses, fov, tracking status, etc of the user's HMD
		XRHMDState *m_pXRHMDState = nullptr;

		/// Pointer to the XR core system object
		XRCore *m_pXRCore = nullptr; 

		/// Results of the last call to the OpenXR api
		XrResult m_xrLastCallResult = XR_SUCCESS;

		/// The current predicted display time
		XrTime m_xrPredictedDisplayTime;

		/// The current predicted display period for predicting display times beyond the next m_xrPredictedDisplayTime
		XrDuration m_xrPredictedDisplayPeriod;
	};
} // namespace OpenXRProvider
