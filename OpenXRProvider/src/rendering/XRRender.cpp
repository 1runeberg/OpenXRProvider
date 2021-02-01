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

#include <rendering/XRRender.h>

namespace OpenXRProvider
{
	/// Median adult IPD (https://www.researchgate.net/publication/229084829_Variation_and_extrema_of_human_interpupillary_distance)
	const float XRRender::k_fMedianIPD =	0.063f;

	/// Minimum child IPD (https://www.researchgate.net/publication/229084829_Variation_and_extrema_of_human_interpupillary_distance)
	const float XRRender::k_fMinIPD = 0.04f;  

	/// Half of Pi
	const float XRRender::k_fTau = 1.570f; 

	XRRender::XRRender( XRCore *pXRCore, XRRenderInfo xrRenderInfo )
		: m_pXRCore( pXRCore )
	{
		if ( !m_pXRCore && !m_pXRCore->GetLogger() )
			throw std::runtime_error( "Failed to create XR Render Manager. Invalid XR Manager provided." );

		// Retain pointer to logger
		m_pXRLogger = m_pXRCore->GetLogger();

		// Check xr instance
		if ( m_pXRCore->GetXRInstance() == XR_NULL_HANDLE )
		{
			std::string eMessage = "Failed to create XR Render manager due to an Invalid XR Instance. OpenXRProvider must be initialized properly before "
								   "create an XR Render manager.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Check xr session
		if ( m_pXRCore->GetXRSession() == XR_NULL_HANDLE )
		{
			std::string eMessage = "Failed to create XR Render manager due to an Invalid XR Session. OpenXRProvider must be initialized properly before create "
								   "an XR Render manager.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Check xr graphics api
		if ( !m_pXRCore->GetGraphicsAPI() )
		{
			std::string eMessage = "Failed to create XR Render manager due to an Invalid Graphics API object. OpenXRProvider must be initialized properly "
								   "before create an XR Render manager.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Get number of view configurations the runtime supports
		uint32_t nViewConfigTypeCount = 0;
		m_xrLastCallResult = XR_CALL(
			xrEnumerateViewConfigurations( m_pXRCore->GetXRInstance(), m_pXRCore->GetXRSystemId(), 0, &nViewConfigTypeCount, nullptr ),
			m_pXRLogger,
			true );

		// Retrieve all the view configuration types the runtime supports
		std::vector< XrViewConfigurationType > xrViewConfigTypes( nViewConfigTypeCount );
		m_xrLastCallResult = XR_CALL_SILENT(
			xrEnumerateViewConfigurations(
				m_pXRCore->GetXRInstance(), m_pXRCore->GetXRSystemId(), nViewConfigTypeCount, &nViewConfigTypeCount, xrViewConfigTypes.data() ),
			m_pXRLogger );

		// Look for Stereo (VR) config. TODO: XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO (AR Support)
		bool bStereoFound = false;
		for ( uint32_t i = 0; i < nViewConfigTypeCount; ++i )
		{
			if ( xrViewConfigTypes[ i ] == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO )
			{
				bStereoFound = true;
				break;
			}
		}

		// Check if Stereo (VR) is supported by the runtime
		if ( !bStereoFound )
		{
			std::string eMessage = ( "Failed to create XR Render manager. The active runtime does not support VR" );
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		m_pXRLogger->info( "Runtime confirms VR support amongst its {} supported view configuration(s)", nViewConfigTypeCount );

		// Get number of configuration views supported by the runtime
		uint32_t nViewCount = 0;
		m_xrLastCallResult = xrEnumerateViewConfigurationViews(
			m_pXRCore->GetXRInstance(), m_pXRCore->GetXRSystemId(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &nViewCount, nullptr );

		// Retrieve configuration views from the runtime
		m_vXRViewConfigs.resize( nViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
		m_xrLastCallResult = XR_CALL(
			xrEnumerateViewConfigurationViews(
				m_pXRCore->GetXRInstance(),
				m_pXRCore->GetXRSystemId(),
				XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
				nViewCount,
				&nViewCount,
				m_vXRViewConfigs.data() ),
			m_pXRLogger,
			true );

		m_pXRLogger->info( "Successfully retrieved {} configuration views from the runtime. Should be two for VR (one for each eye)", nViewCount );

		// Begin XR Session
		XrSessionBeginInfo xrSessionBeginInfo = { XR_TYPE_SESSION_BEGIN_INFO };
		xrSessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		m_xrLastCallResult = XR_CALL( xrBeginSession( m_pXRCore->GetXRSession(), &xrSessionBeginInfo ), m_pXRLogger, true );

		m_pXRLogger->info( "XR Session started (Handle {})", ( uint64_t )m_pXRCore->GetXRSession() );

		// Allocate xr views
		m_vXRViews.resize( nViewCount, { XR_TYPE_VIEW } );

		// Final sanity check - view count must be 2 for VR support (one for each eye)
		if ( nViewCount != k_nVRViewCount )
		{
			m_pXRLogger->error( "Something went wrong. Runtime returned {} views for VR support, while we expected exactly 2 (one for each eye)", nViewCount );
			throw std::runtime_error( "Something went wrong. Runtime returned incorrect number of views for VR support. Expected 2 (one for each eye)" );
		}
		m_pXRLogger->info( "{} XR views successfully (one for each eye)", nViewCount );

		// Reset HMD state
		m_pXRHMDState = new XRHMDState();
		ResetHMDState();

		// Set swapchain details
		m_bDepthHandling = m_pXRCore->GetIsDepthSupported();
		m_nTextureArraySize = xrRenderInfo.TextureArraySize;
		m_nTextureMipCount = xrRenderInfo.TextureMipCount;
		m_nTextureWidth = m_vXRViewConfigs[ 0 ].recommendedImageRectWidth;
		m_nTextureHeight = m_vXRViewConfigs[ 0 ].recommendedImageRectHeight;
		SetSwapchainFormat( xrRenderInfo.RequestTextureFormats, xrRenderInfo.RequestDepthFormats );
	
		// Generate Swapchains
		GenerateSwapchains( false ); // Color textures

		if ( m_bDepthHandling )
			GenerateSwapchains( true ); // Depth textures

		// Generate Swapchain images (color)
		for ( uint32_t i = 0; i < m_vXRSwapChainsColor.size(); i++ )
		{
			m_xrLastCallResult = m_pXRCore->GetGraphicsAPI()->GenerateSwapchainImages( m_vXRSwapChainsColor[ i ], i, false );

			if ( m_xrLastCallResult != XR_SUCCESS )
			{
				const char *xrEnumStr = XrEnumToString( m_xrLastCallResult );
				std::string eMessage = "Failed to generate swapchain color buffers with error ";
				eMessage.append( xrEnumStr );

				m_pXRLogger->error( "{} ({})", eMessage, std::to_string( m_xrLastCallResult ) );
				throw std::runtime_error( eMessage );
			}

			m_pXRLogger->info(
				"{} Swapchain color buffers generated for eye ({})", m_pXRCore->GetGraphicsAPI()->GetSwapchainImageCount( i == 0 ? EYE_LEFT : EYE_RIGHT, false ), i );
		}

		// Generate Swapchain images (depth)
		if ( m_bDepthHandling )
		{
			for ( uint32_t i = 0; i < m_vXRSwapChainsDepth.size(); i++ )
			{
				m_pXRCore->GetGraphicsAPI()->GenerateSwapchainImages( m_vXRSwapChainsDepth[ i ], i, true );

				if ( m_xrLastCallResult != XR_SUCCESS )
				{
					const char *xrEnumStr = XrEnumToString( m_xrLastCallResult );
					std::string eMessage = "Failed to generate swapchain depth buffers with error ";
					eMessage.append( xrEnumStr );

					m_pXRLogger->error( "{} ({})", eMessage, std::to_string( m_xrLastCallResult ) );
					throw std::runtime_error( eMessage );
				}

				m_pXRLogger->info(
					"{} Swapchain depth buffers generated for eye ({})", m_pXRCore->GetGraphicsAPI()->GetSwapchainImageCount( i == 0 ? EYE_LEFT : EYE_RIGHT, true ), i );
			}
		}
		else
		{
			m_pXRLogger->info("Runtime does not support depth composition. No Swapchain depth buffers will be generated");
		}

		// Add supported extension - Visibility Mask
		for	each( void *xrExtension in m_pXRCore->GetXREnabledExtensions() )
			{
				XRBaseExt *xrInstanceExtension = static_cast< XRBaseExt * >( xrExtension );

				if ( strcmp( xrInstanceExtension->GetExtensionName(), XR_KHR_VISIBILITY_MASK_EXTENSION_NAME ) == 0 )
				{
					m_pXRVisibilityMask = static_cast< XRExtVisibilityMask * >( xrExtension );

					m_pXRVisibilityMask->m_xrInstance = m_pXRCore->GetXRInstance();
					m_pXRVisibilityMask->m_xrSession = m_pXRCore->GetXRSession();
					break;
				}
			}

		m_pXRLogger->info( "Render manager created successfully" );
	}

	XRRender::~XRRender()
	{
		// Destroy Swapchains
		DestroySwapchains( m_vXRSwapChainsColor );
		DestroySwapchains( m_vXRSwapChainsDepth );

		// Clear Swapchains
		m_vXRSwapChainsColor.clear();
		m_vXRSwapChainsDepth.clear();
	}

	void XRRender::DestroySwapchains( std::vector< XrSwapchain > &vXRSwapchains )
	{
		uint32_t nSwapChains = ( uint32_t )vXRSwapchains.size();
		for ( uint32_t i = 0; i < nSwapChains; i++ )
		{
			if ( vXRSwapchains[ i ] != XR_NULL_HANDLE )
			{
				m_xrLastCallResult = XR_CALL_SILENT( xrDestroySwapchain( vXRSwapchains[ i ] ), m_pXRLogger );
				if ( m_xrLastCallResult != XR_SUCCESS )
				{
					const char *xrEnumStr = XrEnumToString( m_xrLastCallResult );
					std::string eMessage = "Unable to destroy swapchain with error ";
					eMessage.append( xrEnumStr );

					m_pXRLogger->error( "{} ({})", eMessage, std::to_string( m_xrLastCallResult ) );
					throw std::runtime_error( eMessage );
				}
				else
				{
					m_pXRLogger->info( "Swapchain destroyed for eye ({})", i );
				}
			}
		}
	}

	void XRRender::ResetHMDState()
	{
		m_pXRHMDState->LeftEye.Pose = { { 0.f, 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f } };
		m_pXRHMDState->RightEye.Pose = { { 0.f, 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f } };

		m_pXRHMDState->LeftEye.FoV = { -k_fTau, k_fTau, k_fTau, -k_fTau };
		m_pXRHMDState->RightEye.FoV = { -k_fTau, k_fTau, k_fTau, -k_fTau };

		m_pXRHMDState->IsPositionTracked = false;
		m_pXRHMDState->IsOrientationTracked = false;
	}

	bool XRRender::ProcessXRFrame()
	{
		assert( m_pXRCore && m_pXRLogger );

		// ========================================================================
		// (1) Wait for a new frame
		// ========================================================================
		XrFrameWaitInfo xrWaitFrameInfo { XR_TYPE_FRAME_WAIT_INFO };
		XrFrameState xrFrameState { XR_TYPE_FRAME_STATE };

		m_xrLastCallResult = XR_CALL_SILENT( xrWaitFrame( m_pXRCore->GetXRSession(), &xrWaitFrameInfo, &xrFrameState ), m_pXRLogger );
		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		m_xrPredictedDisplayTime = xrFrameState.predictedDisplayTime;
		m_xrPredictedDisplayPeriod = xrFrameState.predictedDisplayPeriod;

		// ========================================================================
		// (2) Begin frame before doing any GPU work
		// ========================================================================
		XrFrameBeginInfo xrBeginFrameInfo { XR_TYPE_FRAME_BEGIN_INFO };
		m_xrLastCallResult = XR_CALL_SILENT( xrBeginFrame( m_pXRCore->GetXRSession(), &xrBeginFrameInfo ), m_pXRLogger );
		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		std::vector< XrCompositionLayerBaseHeader * > xrFrameLayers;
		XrCompositionLayerProjectionView xrFrameLayerProjectionViews[ k_nVRViewCount ];
		XrCompositionLayerProjection xrFrameLayerProjection { XR_TYPE_COMPOSITION_LAYER_PROJECTION };

		if ( xrFrameState.shouldRender )
		{
			// ========================================================================
			// (3) Get space and time information for this frame
			// ========================================================================

			XrViewLocateInfo xrFrameSpaceTimeInfo { XR_TYPE_VIEW_LOCATE_INFO };
			xrFrameSpaceTimeInfo.displayTime = xrFrameState.predictedDisplayTime;
			xrFrameSpaceTimeInfo.space = m_pXRCore->GetXRSpace();

			XrViewState xrFrameViewState { XR_TYPE_VIEW_STATE };
			uint32_t nFoundViewsCount;
			m_xrLastCallResult = XR_CALL_SILENT(
				xrLocateViews(
					m_pXRCore->GetXRSession(), &xrFrameSpaceTimeInfo, &xrFrameViewState, ( uint32_t )m_vXRViews.size(), &nFoundViewsCount, m_vXRViews.data() ),
				m_pXRLogger );

			if ( m_xrLastCallResult != XR_SUCCESS )
				return false;

			// ========================================================================
			// (4) Grab image from swapchain and render
			// ========================================================================

			// Update HMD State
			m_pXRHMDState->IsPositionTracked = xrFrameViewState.viewStateFlags & XR_VIEW_STATE_POSITION_TRACKED_BIT;
			m_pXRHMDState->IsOrientationTracked = xrFrameViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_TRACKED_BIT;

			if ( xrFrameViewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT && xrFrameViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT )
			{
				// Update hmd state
				SetHMDState( EXREye::EYE_LEFT, &( m_pXRHMDState->LeftEye ) );
				SetHMDState( EXREye::EYE_RIGHT, &( m_pXRHMDState->RightEye ) );

				// Grab the corresponding swapchain for each view location
				uint32_t nViewCount = ( uint32_t )m_vXRViewConfigs.size();

				for ( uint32_t i = 0; i < nViewCount; i++ )
				{
					// ----------------------------------------------------------------
					// (a) Acquire swapchain image
					// ----------------------------------------------------------------
					const XrSwapchain xrSwapchain = m_vXRSwapChainsColor[ i ];
					XrSwapchainImageAcquireInfo xrAcquireInfo { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
					uint32_t nImageIndex;
					m_xrLastCallResult = XR_CALL_SILENT( xrAcquireSwapchainImage( xrSwapchain, &xrAcquireInfo, &nImageIndex ), m_pXRLogger );

					if ( m_xrLastCallResult != XR_SUCCESS )
						return false;

					// ----------------------------------------------------------------
					// (b) Wait for swapchain image
					// ----------------------------------------------------------------
					XrSwapchainImageWaitInfo xrWaitInfo { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
					xrWaitInfo.timeout = XR_INFINITE_DURATION;
					m_xrLastCallResult = XR_CALL_SILENT( xrWaitSwapchainImage( xrSwapchain, &xrWaitInfo ), m_pXRLogger );

					if ( m_xrLastCallResult != XR_SUCCESS )
						return false;

					// ----------------------------------------------------------------
					// (c) Add projection view to swapchain image
					// ----------------------------------------------------------------

					xrFrameLayerProjectionViews[ i ] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
					xrFrameLayerProjectionViews[ i ].pose = m_vXRViews[ i ].pose;
					xrFrameLayerProjectionViews[ i ].fov = m_vXRViews[ i ].fov;
					xrFrameLayerProjectionViews[ i ].subImage.swapchain = xrSwapchain;
					xrFrameLayerProjectionViews[ i ].subImage.imageArrayIndex = 0;
					xrFrameLayerProjectionViews[ i ].subImage.imageRect.offset = { 0, 0 };
					xrFrameLayerProjectionViews[ i ].subImage.imageRect.extent = { ( int32_t )m_nTextureWidth, ( int32_t )m_nTextureHeight };

					if ( m_bDepthHandling )
					{
						XrCompositionLayerDepthInfoKHR xrDepthInfo{ XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
						xrDepthInfo.subImage.swapchain = m_vXRSwapChainsDepth[ i ];
						xrDepthInfo.subImage.imageArrayIndex = 0;
						xrDepthInfo.subImage.imageRect.offset = { 0, 0 };
						xrDepthInfo.subImage.imageRect.extent = { ( int32_t )m_nTextureWidth, ( int32_t )m_nTextureHeight };
						xrDepthInfo.minDepth = 0.0f;
						xrDepthInfo.maxDepth = 1.0f;
						xrDepthInfo.nearZ = 0.1f;
						xrDepthInfo.farZ = FLT_MAX;

						xrFrameLayerProjectionViews[i].next = &xrDepthInfo;
					}

					// ----------------------------------------------------------------
					// (d) Release swapchain image
					// ----------------------------------------------------------------
					XrSwapchainImageReleaseInfo xrSwapChainRleaseInfo { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
					m_xrLastCallResult = XR_CALL_SILENT( xrReleaseSwapchainImage( xrSwapchain, &xrSwapChainRleaseInfo ), m_pXRLogger );

					if ( m_xrLastCallResult != XR_SUCCESS )
						return false;
				}
			}

			// ----------------------------------------------------------------
			// (e) Assemble projection layers
			// ----------------------------------------------------------------
			xrFrameLayerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
			xrFrameLayerProjection.space = m_pXRCore->GetXRSpace();
			xrFrameLayerProjection.viewCount = k_nVRViewCount;
			xrFrameLayerProjection.views = xrFrameLayerProjectionViews;
			xrFrameLayers.push_back( reinterpret_cast< XrCompositionLayerBaseHeader * >( &xrFrameLayerProjection ) );
		}

		// ========================================================================
		// (5) End current frame
		// ========================================================================
		XrFrameEndInfo xrEndFrameInfo { XR_TYPE_FRAME_END_INFO };
		xrEndFrameInfo.displayTime = xrFrameState.predictedDisplayTime;
		xrEndFrameInfo.environmentBlendMode =
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE; // TODO: XR_ENVIRONMENT_BLEND_MODE_ADDITIVE / XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND (AR)
		xrEndFrameInfo.layerCount = ( uint32_t )xrFrameLayers.size();
		xrEndFrameInfo.layers = xrFrameLayers.data();

		m_xrLastCallResult = XR_CALL_SILENT( xrEndFrame( m_pXRCore->GetXRSession(), &xrEndFrameInfo ), m_pXRLogger );
		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		return true;
	}

	OpenXRProvider::XRGraphicsAPI *XRRender::GetGraphicsAPI()
	{
		assert( m_pXRCore );
		return m_pXRCore->GetGraphicsAPI();
	}

	void XRRender::SetHMDState( EXREye eEye, XREyeState *pEyeState )
	{
		assert( pEyeState );

		uint32_t nEye = ( eEye == EXREye::EYE_LEFT ) ? 0 : 1;

		// Update Position
		pEyeState->Pose = m_vXRViews[ nEye ].pose;

		// Update Orientation
		pEyeState->Pose.orientation = m_vXRViews[ nEye ].pose.orientation;

		// Update field of view
		pEyeState->FoV = m_vXRViews[ nEye ].fov;
	}

	float XRRender::GetCurrentIPD()
	{
		// Get mid-point of eye positions (disregard Z)
		XrVector2f midEyePosition;
		midEyePosition.x = ( m_pXRHMDState->LeftEye.Pose.position.x + m_pXRHMDState->RightEye.Pose.position.x ) / 2.0f;
		midEyePosition.y = ( m_pXRHMDState->LeftEye.Pose.position.y + m_pXRHMDState->RightEye.Pose.position.y ) / 2.0f;

		float IPD = ( std::sqrtf(
						std::powf( ( midEyePosition.x - m_pXRHMDState->LeftEye.Pose.position.x ), 2 ) +
						std::powf( ( midEyePosition.y - m_pXRHMDState->LeftEye.Pose.position.y ), 2 ) +
						std::powf( ( midEyePosition.y - m_pXRHMDState->LeftEye.Pose.position.y ), 2 ) ) ) *
					2.0f;

		if ( IPD < k_fMinIPD )
			return k_fMedianIPD;

		return IPD;
	}

	XrPosef XRRender::GetHMDPose()
	{
		// Get mid-point of eye positions
		XrVector3f midEyePosition = { 0 };
		midEyePosition.x = ( m_pXRHMDState->LeftEye.Pose.position.x + m_pXRHMDState->RightEye.Pose.position.x ) / 2.0f;
		midEyePosition.y = ( m_pXRHMDState->LeftEye.Pose.position.y + m_pXRHMDState->RightEye.Pose.position.y ) / 2.0f;
		midEyePosition.z = ( m_pXRHMDState->LeftEye.Pose.position.z + m_pXRHMDState->RightEye.Pose.position.z ) / 2.0f;

		return XrPosef { m_pXRHMDState->LeftEye.Pose.orientation, midEyePosition };
	}

	void XRRender::SetSwapchainFormat( std::vector< int64_t > vAppTextureFormats, std::vector< int64_t > vAppDepthFormats )
	{
		assert( m_pXRCore );
		assert( m_pXRCore->GetXRSession() != XR_NULL_HANDLE );

		// Check number of swapchain formats supported by the runtime
		uint32_t nNumOfSupportedFormats = 0;
		m_xrLastCallResult = XR_CALL( xrEnumerateSwapchainFormats( m_pXRCore->GetXRSession(), 0, &nNumOfSupportedFormats, nullptr ), m_pXRLogger, true )

		// Get swapchain formats supported by the runtime
		std::vector< int64_t > vRuntimeSwapChainFormats;
		vRuntimeSwapChainFormats.resize( nNumOfSupportedFormats );

		m_xrLastCallResult = XR_CALL(
				xrEnumerateSwapchainFormats( m_pXRCore->GetXRSession(), nNumOfSupportedFormats, &nNumOfSupportedFormats, vRuntimeSwapChainFormats.data() ),
			m_pXRLogger,
			true )

		// Look for a matching texture format that the app requested vs what's supported by the runtime
		m_pXRLogger->info( "Runtime supports the following texture formats in order of preference:" );
		uint32_t nNum = 0;
		int64_t nMatchColor = 0;
		int64_t nMatchDepth = 0;

		for ( int64_t SwapChainFormat : vRuntimeSwapChainFormats )
		{
			m_pXRLogger->info( "{}. {} ({})", ++nNum, m_pXRCore->GetGraphicsAPI()->GetTextureFormatName( SwapChainFormat ), SwapChainFormat );

			// Look for matching color texture format
			for ( size_t i = 0; i < vAppTextureFormats.size(); i++ )
			{
				if ( nMatchColor == 0 )
					nMatchColor = SwapChainFormat == vAppTextureFormats[ i ] ? SwapChainFormat : 0;
			}

			// Look for matching depth texture format
			if ( m_bDepthHandling )
			{
				for ( size_t i = 0; i < vAppDepthFormats.size(); i++ )
				{
					// Check if the this texture format is a depth format
					if ( nMatchDepth == 0 && m_pXRCore->GetGraphicsAPI()->IsDepth( SwapChainFormat ) )
					{
						nMatchDepth = SwapChainFormat == vAppDepthFormats[i] ? SwapChainFormat : 0;
					}
				}
			}
		}

		// Choose the strongest runtime preference if app texture request can't be found
		if ( nMatchColor == 0 )
			nMatchColor = vRuntimeSwapChainFormats[ 0 ];
	
		m_nTextureFormat = nMatchColor;
		m_pXRLogger->info( "XR Texture color format will be {} ({})", m_pXRCore->GetGraphicsAPI()->GetTextureFormatName( nMatchColor ), nMatchColor );

		// Process depth textures if available
		if ( m_bDepthHandling && vAppDepthFormats.size() > 0 )
		{
			// Choose a known depth format if app texture request can't be found or the app requested an invalid depth format
			if (nMatchDepth == 0 || !m_pXRCore->GetGraphicsAPI()->IsDepth(nMatchDepth))
				nMatchDepth = m_pXRCore->GetGraphicsAPI()->GetDefaultDepthFormat();

			m_nDepthFormat = nMatchDepth;
			m_pXRLogger->info( "XR Texture depth format will be {} ({})", m_pXRCore->GetGraphicsAPI()->GetTextureFormatName( nMatchDepth ), nMatchDepth );
		}
		else
		{
			m_pXRLogger->info( "Session will not support depth textures" );
			m_bDepthHandling = false;
		}	
	}

	void XRRender::GenerateSwapchains( bool bIsDepth )
	{
		for ( size_t i = 0; i < m_vXRViews.size(); i++ )
		{
			XrSwapchain xrSwapChain;
			XrSwapchainCreateInfo xrSwapChainCreateInfo { XR_TYPE_SWAPCHAIN_CREATE_INFO };
			xrSwapChainCreateInfo.arraySize = m_nTextureArraySize;
			xrSwapChainCreateInfo.format = bIsDepth ? m_nDepthFormat : m_nTextureFormat;
			xrSwapChainCreateInfo.width = m_vXRViewConfigs[ i ].recommendedImageRectWidth;
			xrSwapChainCreateInfo.height = m_vXRViewConfigs[ i ].recommendedImageRectHeight;
			xrSwapChainCreateInfo.mipCount = m_nTextureMipCount;
			xrSwapChainCreateInfo.faceCount = 1;
			xrSwapChainCreateInfo.sampleCount = m_vXRViewConfigs[ i ].recommendedSwapchainSampleCount;

			if ( bIsDepth )
			{
				xrSwapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				xrSwapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
			}

			m_xrLastCallResult = XR_CALL( xrCreateSwapchain( m_pXRCore->GetXRSession(), &xrSwapChainCreateInfo, &xrSwapChain ), m_pXRLogger, true );

			if ( bIsDepth )
			{
				m_vXRSwapChainsDepth.push_back( xrSwapChain );
				m_pXRLogger->info(
					"Depth Swapchain created for eye ({}). Textures are {}x{} with {} sample(s) and {} Mip(s)",
					i,
					m_nTextureWidth,
					m_nTextureHeight,
					m_vXRViewConfigs[ i ].recommendedSwapchainSampleCount,
					m_nTextureMipCount );
			}
			else
			{
				m_vXRSwapChainsColor.push_back( xrSwapChain );
				m_pXRLogger->info(
					"Color Swapchain created for eye ({}). Textures are {}x{} with {} sample(s) and {} Mip(s)",
					i,
					m_nTextureWidth,
					m_nTextureHeight,
					m_vXRViewConfigs[ i ].recommendedSwapchainSampleCount,
					m_nTextureMipCount );
			}
		}
	}

} // namespace OpenXRProvider
