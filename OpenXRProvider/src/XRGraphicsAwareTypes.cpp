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

#include "../../Sandbox/third_party/glad/include/glad/glad.h"

#include <XRGraphicsAwareTypes.h>

namespace OpenXRProvider
{
	XRGraphicsAPI::XRGraphicsAPI(
		XrInstance *xrInstance,
		XrSystemId *xrSystemId,
		XRAppGraphicsInfo *xrAppGraphicsInfo,
		XrSession *xrSession,
		XrResult *xrResult )
	{
		assert( xrInstance );
		assert( xrSystemId );
		assert( xrAppGraphicsInfo );
		assert( xrSession );
		assert( xrResult );

		m_xrGraphicsBinding = nullptr;

#ifdef XR_USE_GRAPHICS_API_OPENGL

		XrGraphicsRequirementsOpenGLKHR xrRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
		PFN_xrGetOpenGLGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR = nullptr;
		xrGetInstanceProcAddr( *xrInstance, "xrGetOpenGLGraphicsRequirementsKHR", ( PFN_xrVoidFunction * )&xrGetOpenGLGraphicsRequirementsKHR );

		// Setup Graphics bindings
		XrGraphicsBindingOpenGLWin32KHR xrGraphicsBinding = { XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
		xrGraphicsBinding.next = nullptr;
		xrGraphicsBinding.hDC = xrAppGraphicsInfo->hdc;
		xrGraphicsBinding.hGLRC = xrAppGraphicsInfo->hglrc;

		*xrResult = xrGetOpenGLGraphicsRequirementsKHR( *xrInstance, *xrSystemId, &xrRequirements );

		if ( *xrResult != XR_SUCCESS )
			return; // Not throwing a runtime error here as we want to report why the create session failed.

		// Create Session
		XrSessionCreateInfo xrSessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
		xrSessionCreateInfo.next = &xrGraphicsBinding;
		xrSessionCreateInfo.systemId = *xrSystemId;

		*xrResult = xrCreateSession( *xrInstance, &xrSessionCreateInfo, xrSession );

		if ( *xrResult != XR_SUCCESS )
			return; // Not throwing a runtime error here as we want to report why the create session failed.

		m_xrGraphicsBinding = &xrGraphicsBinding;
		return;
#endif
	}

	XRGraphicsAPI::~XRGraphicsAPI() {}

	XrResult XRGraphicsAPI::GenerateSwapchainImages( const XrSwapchain &xrSwapChain, const uint32_t nEye, const bool bIsDepth )
	{
		// Check how many images are in this swapchain from the runtime
		uint32_t nNumOfSwapchainImages;
		XrResult xrResult = xrEnumerateSwapchainImages( xrSwapChain, 0, &nNumOfSwapchainImages, nullptr );
		if ( xrResult != XR_SUCCESS )
			return xrResult;

		// Generate swapchain image holders based on retrieved count from the runtime
#if XR_USE_GRAPHICS_API_OPENGL
		std::vector< XrSwapchainImageOpenGLKHR > xrSwapchainImages;
		xrSwapchainImages.resize( nNumOfSwapchainImages );

		for ( uint32_t i = 0; i < nNumOfSwapchainImages; i++ )
		{
			xrSwapchainImages[ i ] = { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR };
		}
#endif

		// Retrieve swapchain images from the runtime
		xrResult = xrEnumerateSwapchainImages(
			xrSwapChain, nNumOfSwapchainImages, &nNumOfSwapchainImages, reinterpret_cast< XrSwapchainImageBaseHeader * >( xrSwapchainImages.data() ) );
		if ( xrResult != XR_SUCCESS )
			return xrResult;

		// Add the images to our swapchain image cache for drawing to later
		for ( uint32_t i = 0; i < ( uint32_t )nNumOfSwapchainImages; i++ )
		{
			if ( nEye == 0 )
			{
				if ( bIsDepth )
					m_xrSwapchainImages_Depth_L.push_back( xrSwapchainImages[ i ] );
				else
					m_xrSwapchainImages_Color_L.push_back( xrSwapchainImages[ i ] );
			}
			else
			{
				if ( bIsDepth )
					m_xrSwapchainImages_Depth_R.push_back( xrSwapchainImages[ i ] );
				else
					m_xrSwapchainImages_Color_R.push_back( xrSwapchainImages[ i ] );
			}
		}

		return XR_SUCCESS;
	}

	uint32_t XRGraphicsAPI::GetSwapchainImageCount( const EXREye eEye, const bool bIsDepth )
	{
		switch ( eEye )
		{
			case OpenXRProvider::EYE_LEFT:
				if ( bIsDepth )
					return ( uint32_t )m_xrSwapchainImages_Depth_L.size();
				return ( uint32_t )m_xrSwapchainImages_Color_L.size();
				break;

			case OpenXRProvider::EYE_RIGHT:
				if ( bIsDepth )
					return ( uint32_t )m_xrSwapchainImages_Depth_R.size();
				return ( uint32_t )m_xrSwapchainImages_Color_R.size();
				break;
		}

		return 0;
	}

#ifdef XR_USE_GRAPHICS_API_OPENGL
	uint32_t XRGraphicsAPI::GetTexture2D( const EXREye eEye, uint32_t nSwapchainIndex, const bool bGetDepth /*= false*/ )
	{
		return GetTextures2D( eEye, bGetDepth )[ nSwapchainIndex ].image;
	}

	std::vector< XrSwapchainImageOpenGLKHR > XRGraphicsAPI::GetTextures2D( const EXREye eEye, const bool bGetDepth )
	{
		switch ( eEye )
		{
			case OpenXRProvider::EYE_LEFT:
				if ( bGetDepth )
					return m_xrSwapchainImages_Depth_L;
				return m_xrSwapchainImages_Color_L;
				break;

			case OpenXRProvider::EYE_RIGHT:
				if ( bGetDepth )
					return m_xrSwapchainImages_Depth_R;
				return m_xrSwapchainImages_Color_R;
				break;
		}

		return m_xrSwapchainImages_Color_L;
	}

	std::string XRGraphicsAPI::GetTextureFormatName( int64_t nTextureFormat )
	{
		switch ( nTextureFormat )
		{
			case GL_RGBA16:
				return "GL_RGBA16";

			case GL_RGBA16F:
				return "GL_RGBA16F";

			case GL_RGB16F:
				return "GL_RGB16F";

			case GL_SRGB8:
				return "GL_SRGB8";

			case GL_SRGB8_ALPHA8:
				return "GL_SRGB8_ALPHA8";

			case GL_DEPTH_COMPONENT16:
				return "GL_DEPTH_COMPONENT16";

			case GL_DEPTH_COMPONENT24:
				return "GL_DEPTH_COMPONENT24";

			case GL_DEPTH_COMPONENT32:
				return "GL_DEPTH_COMPONENT32";
			
			default:
				return std::to_string( nTextureFormat );
		}

		return std::to_string( nTextureFormat );
	}

	bool XRGraphicsAPI::IsDepth( int64_t nDepthFormat )
	{
		switch ( nDepthFormat )
		{
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
			return true;
		}
		
		return false;
	}
	
	int64_t XRGraphicsAPI::GetDefaultDepthFormat()
	{
		return GL_DEPTH_COMPONENT16;
	}
#endif
} // namespace OpenXRProvider
