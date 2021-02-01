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

#include <extensions/XRExtVisibilityMask.h>

namespace OpenXRProvider
{
	XRExtVisibilityMask::XRExtVisibilityMask( std::shared_ptr< spdlog::logger > pLogger )
		: XRBaseExt( pLogger )
	{
	}

	XRExtVisibilityMask::~XRExtVisibilityMask() {}

	bool XRExtVisibilityMask::GetVisibilityMask( EXREye eEye, EMaskType eMaskType, std::vector< float > &vMaskVertices, std::vector< uint32_t > &vMaskIndices )
	{
		if ( m_xrInstance == XR_NULL_HANDLE || m_xrSession == XR_NULL_HANDLE )
			return false;

		// Convert mask type to native OpenXR mask type
		XrVisibilityMaskTypeKHR xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR;
		switch ( eMaskType )
		{
			case OpenXRProvider::XRExtVisibilityMask::MASK_VISIBLE:
				xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_VISIBLE_TRIANGLE_MESH_KHR;
				break;
			case OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN:
				xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR;
				break;
			case OpenXRProvider::XRExtVisibilityMask::MASK_LINE_LOOP:
				xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_LINE_LOOP_KHR;
				break;
		}

		PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR = nullptr;
		m_xrLastCallResult =
			XR_CALL( xrGetInstanceProcAddr( m_xrInstance, "xrGetVisibilityMaskKHR", ( PFN_xrVoidFunction * )&xrGetVisibilityMaskKHR ), m_pXRLogger, false );

		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		// Get index and vertex counts
		XrVisibilityMaskKHR pXRVisibilityMask = { XR_TYPE_VISIBILITY_MASK_KHR };
		pXRVisibilityMask.indexCapacityInput = 0;
		pXRVisibilityMask.vertexCapacityInput = 0;

		m_xrLastCallResult = XR_CALL_SILENT(
			xrGetVisibilityMaskKHR( m_xrSession, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eEye == EYE_LEFT ? 0 : 1, xrVisibilityMaskType, &pXRVisibilityMask ),
			m_pXRLogger );

		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		if ( pXRVisibilityMask.indexCountOutput == 0 && pXRVisibilityMask.vertexCountOutput == 0 )
		{
			m_pXRLogger->warn( "Runtime does not have a Visibility Mask for eye ({})", eEye );
			return false;
		}
		else if ( xrVisibilityMaskType == XR_VISIBILITY_MASK_TYPE_LINE_LOOP_KHR && pXRVisibilityMask.indexCountOutput % 3 != 0 )
		{
			m_pXRLogger->error( "Runtime returned an invalid Visibility Mask" );
			return false;
		}

		// Setup target vectors for the receiving vertices and indices
		std::vector< XrVector2f > vXrVertices;
		uint32_t nVertexCount = pXRVisibilityMask.vertexCountOutput;
		uint32_t nIndexCount = pXRVisibilityMask.indexCountOutput;

		vXrVertices.reserve( nVertexCount );
		vMaskIndices.reserve( nIndexCount );

		// Get mask vertices and indices from the runtime
		pXRVisibilityMask = { XR_TYPE_VISIBILITY_MASK_KHR };
		pXRVisibilityMask.vertexCapacityInput = nVertexCount;
		pXRVisibilityMask.indexCapacityInput = nIndexCount;
		pXRVisibilityMask.indexCountOutput = 0;
		pXRVisibilityMask.vertexCountOutput = 0;
		pXRVisibilityMask.indices = vMaskIndices.data();
		pXRVisibilityMask.vertices = vXrVertices.data();

		m_xrLastCallResult = XR_CALL_SILENT(
			xrGetVisibilityMaskKHR( m_xrSession, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eEye == EYE_LEFT ? 0 : 1, xrVisibilityMaskType, &pXRVisibilityMask ),
			m_pXRLogger );

		if ( m_xrLastCallResult != XR_SUCCESS )
			return false;

		if ( !pXRVisibilityMask.indices || !pXRVisibilityMask.vertices )
		{
			m_pXRLogger->warn(
				"Runtime did not return any indices or vertices for eye ({}). Try again on  "
				"XrEventDataVisibilityMaskChangedKHR::XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR",
				eEye );
			return false;
		}

		// Fill in return vertices
		bool b = vMaskVertices.empty();
		for ( size_t i = 0; i < pXRVisibilityMask.vertexCountOutput; i++ )
		{
			vMaskVertices.push_back( pXRVisibilityMask.vertices[ i ].x );
			vMaskVertices.push_back( pXRVisibilityMask.vertices[ i ].y );
		}

		vMaskIndices.insert( vMaskIndices.end(), &pXRVisibilityMask.indices[ 0 ], &pXRVisibilityMask.indices[ nIndexCount ] );
		return true;
	}

} // namespace OpenXRProvider
