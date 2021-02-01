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
	class XRInputProfile_MicrosoftXBox : public XRBaseInput
	{
	  public:
		// ** STATIC PROPERTIES **/
		inline static const char *Gamepad = "/user/gamepad";

		inline static const char *Button_A_Click = "/input/a/click";
		inline static const char *Button_B_Click = "/input/b/click";
		inline static const char *Button_X_Click = "/input/x/click";
		inline static const char *Button_Y_Click = "/input/y/click";

		inline static const char *Button_DPad_Up_Click = "/input/dpad_up/click";
		inline static const char *Button_DPad_Down_Click = "/input/dpad_down/click";
		inline static const char *Button_DPad_Left_Click = "/input/dpad_left/click";
		inline static const char *Button_DPad_Right_Click = "/input/dpad_right/click";

		inline static const char *Button_Shoulder_Left_Click = "/input/shoulder_left/click";
		inline static const char *Button_Shoulder_Right_Click = "/input/shoulder_right/click";
		
		inline static const char *Button_Menu_Click = "/input/menu/click";
		inline static const char *Button_View_Click = "/input/view/click";

		inline static const char *Button_Trigger_Left_Value = "/input/trigger_left/value";
		inline static const char *Button_Trigger_Right_Value = "/input/trigger_right/value";

		inline static const char *Thumbstick_Left_X = "/input/thumbstick_Left/x";
		inline static const char *Thumbstick_Left_Y = "/input/thumbstick_Left/y";

		inline static const char *Thumbstick_Right_X = "/input/thumbstick_Right/x";
		inline static const char *Thumbstick_Right_Y = "/input/thumbstick_Right/y";

		inline static const char *Output_Haptic_Left = "/output/haptic_left";
		inline static const char *Output_Haptic_Right = "/output/haptic_right";

		inline static const char *Output_Haptic_Left_Trigger = "/output/haptic_left_trigger";
		inline static const char *Output_Haptic_Right_Trigger = "/output/haptic_right_trigger";

		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		XRInputProfile_MicrosoftXBox( std::shared_ptr< spdlog::logger > pLogger );

		/// Class Destructor
		~XRInputProfile_MicrosoftXBox();

		/// Return the input profile path for this controller
		const char *GetInputProfile() const override { return "/interaction_profiles/microsoft/xbox_controller"; }

	};
} // namespace OpenXRProvider
