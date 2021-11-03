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
	class XREventHandler : public XRBase
	{
	  public:
		// ** FUNCTIONS (PUBLIC) */

		/// Class Constructor
		/// @param[in] pLogger	Pointer to the logger object
		 XREventHandler() {}

		/// Class Destructor
		~XREventHandler() { m_pXRCallbacks.clear(); }

		/// Register a callback to the OpenXR Provider event system
		/// @param[in] pLogger	Pointer to the logger object
		inline void RegisterCallback( XRCallback *pXRCallback ) { m_pXRCallbacks.push_back( pXRCallback ); }

		/// De-register a registered callback in the OpenXR Provider event system
		/// @param[in] pXRCallback	Pointer to callback that needs de-registering
		inline void DeregisterCallback( XRCallback *pXRCallback )
		{
			m_pXRCallbacks.erase( std::remove( m_pXRCallbacks.begin(), m_pXRCallbacks.end(), pXRCallback ), m_pXRCallbacks.end() );
			m_pXRCallbacks.shrink_to_fit();
		}

		/// Getter for the array of callback function pointers
		/// @return		Array of callback function pointers that are currently registered in the OpenXR Provider event system
		std::vector< XRCallback * > GetXRCallbacks() { return m_pXRCallbacks; }

	  private:
		/// Results of the last call to the OpenXR api
		XrResult m_xrLastCallResult = XR_SUCCESS;

		/// Array of callback function pointers that are currently registered in the OpenXR Provider event system
		std::vector< XRCallback * > m_pXRCallbacks;
	};
}
