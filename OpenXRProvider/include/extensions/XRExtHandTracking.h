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
	class XRExtHandTracking : public XRBaseExt
	{
	  public:
		// ** FUNCTIONS (PUBLIC) */

		/// Class Constructor
		/// @param[in] pLogger	Pointer to the logger object
		XRExtHandTracking( std::shared_ptr< spdlog::logger > pLogger );

		/// Class Destructor
		~XRExtHandTracking();
		bool bPrecise;
		/// Override from XRBaseExt returning the official OpenXR extension name that this object represents
		const char *GetExtensionName() const override { return XR_EXT_HAND_TRACKING_EXTENSION_NAME; }

		/// The active OpenXR Session
		XrSession m_xrSession = XR_NULL_HANDLE;

		/// The active OpenXR Instance
		XrInstance m_xrInstance = XR_NULL_HANDLE;

		bool bIsHandTrackingActive_Left = true;
		bool bIsHandTrackingActive_Right = true;

		bool bGetHandJointVelocities_Left = false;
		bool bGetHandJointVelocities_Right = false;

		XrHandJointLocationsEXT *GetHandJointLocations( XrHandEXT eHand )
		{
			if ( eHand == XR_HAND_LEFT_EXT )
				return &m_xrLocations_Left;

			return &m_xrLocations_Right;
		}

		void Init( const XrInstance xrInstance, XrSession xrSession );

		void LocateHandJoints( XrHandEXT eHand, XrSpace xrSpace, XrTime xrTime );

	  private:
		XrResult m_xrLastCallResult = XR_SUCCESS;

		XrHandJointLocationEXT m_XRHandJointsData_Left[ XR_HAND_JOINT_COUNT_EXT ];

		XrHandJointLocationEXT m_XRHandJointsData_Right[ XR_HAND_JOINT_COUNT_EXT ];

		XrHandJointVelocityEXT m_XRHandJointVelocities_Left[ XR_HAND_JOINT_COUNT_EXT ];

		XrHandJointVelocityEXT m_XRHandJointVelocities_Right[ XR_HAND_JOINT_COUNT_EXT ];

		XrHandJointVelocitiesEXT m_xrVelocities_Left { XR_TYPE_HAND_JOINT_VELOCITIES_EXT };
		XrHandJointVelocitiesEXT m_xrVelocities_Right { XR_TYPE_HAND_JOINT_VELOCITIES_EXT };

		XrHandJointLocationsEXT m_xrLocations_Left { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
		XrHandJointLocationsEXT m_xrLocations_Right { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };

		XrHandTrackerEXT m_HandTracker_Left = XR_NULL_HANDLE;
		XrHandTrackerEXT m_HandTracker_Right = XR_NULL_HANDLE;

		PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT = nullptr;
	};
} // namespace OpenXRProvider