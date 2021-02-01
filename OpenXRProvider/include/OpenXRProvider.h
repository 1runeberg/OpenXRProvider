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
#include <rendering/XRRender.h>
#include <input/XRInput.h>

namespace OpenXRProvider
{
	class XRProvider
	{
	  public:
		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		/// @param[in] xrAppInfo			App metadata used to create an instance and session to the active OpenXR runtime
		/// @param[in] xrAppGraphicsInfo	App graphics metadata based on current active api
		/// @param[in] xrRenderInfo			Information needed to create swapchain textures that the app will render to
		XRProvider( XRAppInfo xrAppInfo, XRAppGraphicsInfo xrAppGraphicsInfo, XRRenderInfo xrRenderInfo );

		/// Class Destructor
		~XRProvider();

		/// OpenXR Core System
		XRCore* Core() const { return m_pXRCoreSystem; }

		/// OpenXR Render Manager
		XRRender* Render() const { return m_pXRRenderManager; }

		/// OpenXR Input Manager
		XRInput *Input() const { return m_pXRInputManager; }

	  private:
		/// Pointer to the OpenXR Core System
		XRCore* m_pXRCoreSystem = nullptr;

		/// Pointer to the OpenXR Render Manager
		XRRender* m_pXRRenderManager = nullptr;

		/// Pointer to the OpenXR Input Manager
		XRInput *m_pXRInputManager = nullptr;
	};
}
