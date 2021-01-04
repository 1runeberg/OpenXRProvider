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
	class XRExtVisibilityMask : public XRBaseExt
	{
	  public:
		
		/// The visibility mask type (visible, hidden, line-loop)
		enum EMaskType
		{
			/// Visible area of the user's hmd
			MASK_VISIBLE = 0,

			/// Hidden area of the user's hmd
			MASK_HIDDEN = 1,

			/// Line loop vertices indicating mask shape
			MASK_LINE_LOOP = 2
		};

		// ** FUNCTIONS (PUBLIC) */

		/// Class Constructor
		/// @param[in] pLogger	Pointer to the logger object
		XRExtVisibilityMask( std::shared_ptr< spdlog::logger > pLogger );

		/// Class Destructor
		~XRExtVisibilityMask();

		/// Override from XRBaseExt returning the official OpenXR extension name that this object represents
		const char *GetExtensionName() const override { return XR_KHR_VISIBILITY_MASK_EXTENSION_NAME; }

		/// Retrieve the visibility mask vertices and indices
		/// @param[in]	eEye			Which eye the visibility mask request is for
		/// @param[in]	eMaskType		The type of mask needed (hidden, visible, line loop)
		/// @param[out]	vMaskVertices	Vector that will hold the mask vertices (x,y coordinates in a flat float vector)
		/// @param[out]	vMaskVertices	Vector that will hold the mask indices 
		bool GetVisibilityMask( EXREye eEye, EMaskType eMaskType, std::vector< float > &vMaskVertices, std::vector< uint32_t > &vMaskIndices );

		/// The active OpenXR Instance
		XrInstance m_xrInstance = XR_NULL_HANDLE;

		/// The active OpenXR Session
		XrSession m_xrSession = XR_NULL_HANDLE;

	  private:

		XrResult m_xrLastCallResult = XR_SUCCESS;
	};
}