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
#include <XRBaseInput.h>

namespace OpenXRProvider
{
	class XRInputProfile_HTCVivePro : public XRBaseInput
	{
	  public:
		// ** STATIC PROPERTIES **/
		inline static const char *Head = "/user/head";

		inline static const char *Button_System_Click = "/input/system/click";
		inline static const char *Button_VolumeUp_Click = "/input/volume_up/click";
		inline static const char *Button_VolumeDown_Click = "/input/volume_down/click";
		inline static const char *Button_MuteMic_Click = "/input/mute_mic/click";

		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		XRInputProfile_HTCVivePro() {}

		/// Class Destructor
		~XRInputProfile_HTCVivePro() {}

		/// Return the input profile path for this controller
		const char *GetInputProfile() const override { return "/interaction_profiles/htc/vive_pro"; }

	};
} // namespace OpenXRProvider
