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
	class XRBaseInput : public XRBaseExt
	{
     public:

		/// Class Constructor
		/// @param[in] pLogger	The logger object to use
	   XRBaseInput::XRBaseInput( std::shared_ptr< spdlog::logger > pLogger )
		   : XRBaseExt( pLogger )
	   {
	   }

		/// Identify this class as an implementation of an openxr input profile
		const char *GetExtensionName() const override { return "InputProfile"; }

		/// Return the input profile path for this controller
		virtual const char *GetInputProfile() const = 0;

		/// Getter for activation status of this extension
		bool IsActive() const { return m_bIsActive; }
		
		/// Setter for activation status of this extension
		void IsActive( bool val ) { m_bIsActive = val; }

		/// Getter for the list of action bindings related to this controller
		std::vector< XrActionSuggestedBinding > *ActionBindings() { return &m_vActionBindings; }

	  private:
		bool m_bIsActive = false;

		/// Action bindings for this controller
		std::vector< XrActionSuggestedBinding > m_vActionBindings;
	};
} // namespace OpenXRProvider
