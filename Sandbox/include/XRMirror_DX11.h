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

#include <IXRMirror.h>

// Direct X11 includes


class XRMirror_DX11 : public IXRMirror
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] pCommon		Pointer to the sandbox common object
	/// @param[in] nWidth		The width of the desktop window (XR Mirror)
	/// @param[in] nHeight		The height of the desktop window (XR Mirror)
	/// @param[in] pTitle		The title text that will be displayed on the console window
	/// @param[in] sCurrentPath	The absolute path to where the application executable is
	/// @param[in] sLogFile		The absolute path and filename of the log file
	XRMirror_DX11( SandboxCommon *pCommon, int nWidth, int nHeight, const char *pTitle, std::wstring sCurrentPath, const char *sLogFile );

	/// Class Destructor
	~XRMirror_DX11();

	/// Setup the graphics api objects needed for rendering to this xr mirror
	/// @param[in] pRender		Pointer to the OpenXRProvider Library Render Manager
	const int Init( OpenXRProvider::XRRender *pRender ) override;

	/// Blit (copy) swapchain image that's rendered to the HMD to the desktop screen (XR Mirror)
	void BlitToWindow() override;

	/// Clear texture
	/// @param[in]	v4ClearColor	Color (r,g,b,a)
	void Clear( glm::vec4 v4ClearColor ) override;

	/// Render to the swapchain image (texture) that'll be rendered to the user's HMD and blitted (copied) to
	/// the Sandbox's window (XR Mirror)
	/// @param[in]	eCurrentScene		The current sandbox scene/level to be rendered
	/// @param[in] eEye					The eye (left/right) texture that will be rendered on
	/// @param[in] nSwapchainIndex		The index of the swapchain image (texture)
	void DrawFrame( SandboxCommon::ESandboxScene eCurrentScene, OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex ) override;

private:
	// ** MEMBER VARIABLES (PRIVATE) **/


	// ** FUNCTIONS (PRIVATE) **/


};
