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

#include <XRCommon.h>

namespace OpenXRProvider
{
	/// Forward declaration of the XR Provider
	class XRProvider;

#ifdef XR_USE_GRAPHICS_API_OPENGL
	/// OPENGL: Official OpenXR name of the extension
	static const char *s_GraphicsExtensionName = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;

	/// OPENGL: Graphics info (hdc and hglrc). Every supported graphics API will have their own version of this struct
	struct XRAppGraphicsInfo
	{
		HDC hdc;
		HGLRC hglrc;

		XRAppGraphicsInfo( HDC hDC, HGLRC hGLRC )
			: hdc( hDC )
			, hglrc( hGLRC )
		{
		}
	};
#endif

	/// Class that handles graphics api dependent state and transactions. This class is meant to be handled completely internally by the OpenXR Provider
	class XRGraphicsAPI
	{
	  public:
		// ** FUNCTIONS (PUBLIC) **/

		/// Class constructor 
		/// @param[in]	xrInstance			The active OpenXR instance
		/// @param[in]	xrSystemId			The id of the active OpenXR system
		/// @param[in]	xrAppGraphicsInfo	Information payload to create graphics api dependent variables and transactions
		/// @param[out]	xrSession			The created OpenXR session
		/// @param[out]	xrResult			Result of negotiation of the graphics api with the current OpenXR runtime
		XRGraphicsAPI( XrInstance *xrInstance, XrSystemId *xrSystemId, XRAppGraphicsInfo *xrAppGraphicsInfo, XrSession *xrSession, XrResult *xrResult );

		/// Class Destructor
		~XRGraphicsAPI();

		/// Create swapchain images in the correct active graphics api format (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		/// @param[in]	xrSwapChain		The OpenXR swapchain generated by the XR Render Manager
		/// @param[in]	nEye			Eye to generate the texture for
		/// @param[in]	bIsDepth		If a depth texture should be generated as well
		XrResult GenerateSwapchainImages( const XrSwapchain &xrSwapChain, const uint32_t nEye, const bool bIsDepth = false );

		/// Getter for the OpenXR Graphics Binding (graphics api dependent)
		/// @return OpenXR Graphics Binding
		void *GetGraphicsBinding() { return m_xrGraphicsBinding; }

		/// Retrieve the number of swapchain images
		/// @param[in]	eEye		Which eye texture
		/// @param[in]	bIsDepth	Get the depth texture
		/// @return					Number of images (depth/color) in the swapchain for the requested eye
		uint32_t GetSwapchainImageCount( const EXREye eEye, const bool bIsDepth = false );

		/// Retrieve the name of the graphics api dependent format (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		/// @param[in]	nTextureFormat		The texture format represented in int64_t (usually macro definitions from the graphics api library)
		/// @return							The name of the texture format in string
		std::string GetTextureFormatName( int64_t nTextureFormat );

#ifdef XR_USE_GRAPHICS_API_OPENGL
		/// OPENGL: Retrieve the graphics api dependent texture2d that the app should render to
		/// @param[in]	eEye				Which eye texture
		/// @param[in]  nSwapchainIndex		The index of the image in the swapchain to retrieve
		/// @param[in]	bIsDepth			If this is a depth texture
		/// @return							The graphics api dependent texture (a uint32_t id in OpenGL)
		uint32_t GetTexture2D( const EXREye eEye, uint32_t nSwapchainIndex, const bool bGetDepth = false );

		/// OPENGL: Retrieve all the graphics api dependent texture2d that the app should render to
		/// @param[in]	eEye				Which eye texture
		/// @param[in]	bGetDepth			Whether the textures to retrieve are depth textures
		/// @return							Array of graphics api dependent textures that the app should render to
		std::vector< XrSwapchainImageOpenGLKHR > GetTextures2D( const EXREye eEye, const bool bGetDepth = false );
#endif

	  private:
		// ** MEMBER VARIABLES (PRIVATE) **/

		/// The active OpenXR Graphics binding
		void *m_xrGraphicsBinding = nullptr;

		/// The OpenXR Session that was created by this class
		XrSession *m_xrSession = XR_NULL_HANDLE;

#if XR_USE_GRAPHICS_API_OPENGL
		/// OPENGL: Swapchain color images for the left eye that the app should render to
		std::vector< XrSwapchainImageOpenGLKHR > m_xrSwapchainImages_Color_L;

		/// OPENGL: Swapchain color images for the right eye that the app should render to
		std::vector< XrSwapchainImageOpenGLKHR > m_xrSwapchainImages_Color_R;

		/// OPENGL: Swapchain depth images for the left eye that the app should render to
		std::vector< XrSwapchainImageOpenGLKHR > m_xrSwapchainImages_Depth_L;

		/// OPENGL: Swapchain depth images for the right eye that the app should render to
		std::vector< XrSwapchainImageOpenGLKHR > m_xrSwapchainImages_Depth_R;
#endif
	};
}
