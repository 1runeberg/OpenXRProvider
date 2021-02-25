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

// Supported input profiles
#include <input/XRInputProfile_GoogleDaydream.h>
#include <input/XRInputProfile_HTCVive.h>
#include <input/XRInputProfile_HTCVivePro.h>
#include <input/XRInputProfile_KhronosSimple.h>
#include <input/XRInputProfile_MicrosoftMR.h>
#include <input/XRInputProfile_MicrosoftXBox.h>
#include <input/XRInputProfile_OculusGo.h>
#include <input/XRInputProfile_OculusTouch.h>
#include <input/XRInputProfile_ValveIndex.h>

namespace OpenXRProvider
{
	class XRInput
	{
	  public:
		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		/// @param[in] pXRCore			Pointer to the XR Core system
		/// @param[in] pXRRender		Pointer to the XR Render Manager
		XRInput( XRCore *pXRCore, XRRender *pXRRender );

		/// Class Destructor
		~XRInput();

		/// Input Profile object: Google Daydream
		XRInputProfile_GoogleDaydream *GoogleDaydream() const { return m_pXRInputProfile_GoogleDaydream; }

		/// Input Profile object: HTC Vive
		XRInputProfile_HTCVive *HTCVive() const { return m_pXRInputProfile_HTCVive; }

		/// Input Profile object: HTC Vive Pro
		XRInputProfile_HTCVivePro *HTCVivePro() const { return m_pXRInputProfile_HTCVivePro; }

		/// Input Profile object: Khronos Simple
		XRInputProfile_KhronosSimple *KhronosSimple() const { return m_pXRInputProfile_KhronosSimple; }

		/// Input Profile object: Microsoft MR
		XRInputProfile_MicrosoftMR *MicrosoftMR() const { return m_pXRInputProfile_MicrosoftMR; }

		/// Input Profile object: Microsoft XBox
		XRInputProfile_MicrosoftXBox *MicrosoftXBox() const { return m_pXRInputProfile_MicrosoftXBox; }

		/// Input Profile object: Oculus Go
		XRInputProfile_OculusGo *OculusGo() const { return m_pXRInputProfile_OculusGo; }

		/// Input Profile object: Oculus Touch
		XRInputProfile_OculusTouch *OculusTouch() const { return m_pXRInputProfile_OculusTouch; }

		/// Input Profile object: Valve Index
		XRInputProfile_ValveIndex* ValveIndex() const { return m_pXRInputProfile_ValveIndex; }

		/// Get the array of action sets
		std::vector< XrActionSet > ActionSets() const { return m_vActionSets; }

		/// Get the array of active action sets
		std::vector< XrActiveActionSet > ActiveActionSets() const { return m_vActiveActionSets; }

		/// Create an action set
		/// @param[in]	pName			Name of the action set
		/// @param[in]	pLocalizedName	Localized name of the action set (UTF-8)
		/// @param[in]	nPriority		Priority of this action set. The higher the number, the greater the priority
		/// @return		XrActionSet		Handle of the created action set. Zero here means no action set was created
		XrActionSet CreateActionSet( const char *pName, const char *pLocalizedName, uint32_t nPriority );

		/// Create an action
		/// @param[in]	xrActionSet		The handle of the action set that this action will belong to
		/// @param[in]	pName			Name of the action set
		/// @param[in]	pLocalizedName	Localized name of the action set (UTF-8)
		/// @param[in]	xrActionType	The input/output value of this action (e.g. boolean, float, etc)
		/// @param[in]	nFilterCount	The number of filters (xrFilters). Zero means no filtering will be applied
		/// @param[in]	xrFilters		Array of filters for this action (e.g. /user/hand/left, etc). Null means no filtering will be applied
		/// @return		XrAction		Handle of the created action. Zero here means no action was created
		XrAction CreateAction( XrActionSet xrActionSet, const char *pName, const char *pLocalizedName, XrActionType xrActionType, uint32_t nFilterCount, XrPath *xrFilters );

		/// Helper function to create an XrPath from a string
		/// @param[in]	sString			The string to convert to a path (e.g. /user/hand/right/trigger/click, /interaction_profiles/valve/index_controller)
		/// @param[out]	XrPath			The XrPath equivalent of the provided sString
		/// @return	XrResult			Result of attempt at converting a provided string to an XrPath
		XrResult StringToXrPath( const char *sString, XrPath *xrPath );

		/// Create a full input path to a controller component (e.g. /user/hand/right/trigger/click)
		/// @param[in]	xrAction			The handle of the action to bind a controller input to
		/// @param[in]	sControllerPath		The controller input path (e.g. /user/hand/left)
		/// @param[in]	sComponentPath		The controller's component input path (e.g. /trigger/click)
		/// @param[out]	XrPath				Full path to to a controller component (e.g. /user/hand/right/trigger/click)
		/// @return	XrResult				Result of attempt at creating the full input path
		XrResult CreateInputPath( const char* sControllerPath, const char* sComponentPath, XrPath* xrPath );

		/// Bind an action to a controller input path (e.g. Button A - Left Hand, Trigger - Right Hand, etc)
		/// @param[in]	xrAction					The handle of the action to bind a controller input to
		/// @param[in]	sControllerPath				The controller input path (e.g. /user/hand/left)
		/// @param[in]	sComponentPath				The controller's component input path (e.g. /trigger/click)
		/// @param[out]	vActionBindings				Array of action bindings that will hold the new action binding
		/// @return		XrActionSuggestedBinding	Handle of the created action binding. Zero here means no action binding was created
		XrActionSuggestedBinding CreateActionBinding( XrAction xrAction, const char *sControllerPath, const char *sComponentPath, std::vector< XrActionSuggestedBinding > *vActionBindings );

		/// Suggest controller bindings to the runtime
		/// @param[in]	vActionBindings				Array of action bindings that will hold the new action binding
		/// @param[in]	sInteractionProfilePath		The path to the interaction profile that we will suggest to the runtime (e.g. /interaction_profiles/valve/index_controller)
		/// @return		XrResult					Result of trying to suggest a action bindings to the runtime
		XrResult SuggestActionBindings( std::vector< XrActionSuggestedBinding > *vActionBindings, const char *sInteractionProfilePath );

		/// Set an action set as activated by adding it to the activated action sets cache
		/// @param[in]	xrActionSet		The handle of the action set
		/// @param[in]	xrFilter		Optional filter to use. If specified, make sure actions belonging to this action set have activated the filter as well (e.g. /user/hand/left, etc)
		void ActivateActionSet( XrActionSet xrActionSet, XrPath xrFilter = XR_NULL_PATH );

		/// Sync active action set data this frame. This must be called only during XR_SESSION_STATE_FOCUSED
		/// @param[in]	vActionSets	array of active action sets to sync in this frame
		/// @return		XrResult	Result of syncing the selected active action sets
		XrResult SyncActiveActionSetsData();

		/// Get the pose of pose action for the given time and reference space
		/// @param[in]	xrAction			The handle to the pose action to evaluate
		/// @param[in]	xrTime				The time in nanoseconds to get the predicted pose of
		/// @return		XrResult			Result of trying to get the pose from the pose action
		XrResult GetActionPose( XrAction xrAction, XrTime xrTime, XrSpaceLocation *xrLocation );

		/// Get the action state (boolean) from last call the SyncActiveActionSetsData()
		/// @param[in]	xrActionState	The action state to update
		/// @return		XrResult		Result of retrieving the action state
		XrResult GetActionStateBoolean( XrAction xrAction, XrActionStateBoolean *xrActionState );

		/// Get the action state (float) from last call the SyncActiveActionSetsData()
		/// @param[in]	xrActionState	The action state to update
		/// @return		XrResult		Result of retrieving the action state
		XrResult GetActionStateFloat( XrAction xrAction, XrActionStateFloat *xrActionState );

		/// Get the action state (vector2f) from last call the SyncActiveActionSetsData()
		/// @param[in]	xrActionState	The action state to update
		/// @return		XrResult		Result of retrieving the action state
		XrResult GetActionStateVector2f( XrAction xrAction, XrActionStateVector2f *xrActionState );

		/// Get the action state (pose) from last call the SyncActiveActionSetsData()
		/// @param[in]	xrActionState	The action state to update
		/// @return		XrResult		Result of retrieving the action state
		XrResult GetActionStatePose( XrAction xrAction, XrActionStatePose *xrActionState );

		/// Get the currently active interaction profile
		/// @param[in]	sUserPath	The top level user path (e.g. "/user/hand/left")
		/// @return		const char*	The currently active interaction profile
		const char* GetCurrentInteractionProfile( const char* sUserPath );

		/// Generate haptic feedback
		/// @param[in]  xrAction		Haptics action
		/// @param[in]	nDuration		Duration in nanoseconds
		/// @param[in]	fAmplitude		Amplitude between 0.1 and 1.0
		/// @param[in]	fFrequency		Frequency in Hz
		/// @return		XrResult		Result of applying haptic feedback to target action
		XrResult GenerateHaptic
		( 
			XrAction xrAction, 
			uint64_t nDuration = XR_MIN_HAPTIC_DURATION,
			float fAmplitude = 0.5f,
			float fFrequency = XR_FREQUENCY_UNSPECIFIED
		);

	  private:
		// ** FUNCTIONS (PRIVATE) **/

		/// Generate all supported input profile objects
		void GenerateInputProfiles();

		// ** MEMBER VARIABLES (PRIVATE) **/

		/// Pointer to the XR core system object
		XRCore *m_pXRCore = nullptr; 

		/// Pointer to the XR core system object
		XRRender *m_pXRRender = nullptr; 

		/// Results of the last call to the OpenXR api
		XrResult m_xrLastCallResult = XR_SUCCESS;

		/// Action sets
		std::vector< XrActionSet > m_vActionSets;

		/// Activated action sets
		std::vector< XrActiveActionSet > m_vActiveActionSets;

		/// Actions
		std::vector< XrAction > m_vActions;

		/// Action space map
		std::map< XrAction, XrSpace > m_mapActionSpace;

		/// Pointer to the logger
		std::shared_ptr< spdlog::logger > m_pXRLogger;


		// ** INPUT PROFILES ** //
		XRInputProfile_GoogleDaydream *m_pXRInputProfile_GoogleDaydream = nullptr;
		XRInputProfile_HTCVive *m_pXRInputProfile_HTCVive = nullptr;
		XRInputProfile_HTCVivePro *m_pXRInputProfile_HTCVivePro = nullptr;
		XRInputProfile_KhronosSimple *m_pXRInputProfile_KhronosSimple = nullptr;
		XRInputProfile_MicrosoftMR *m_pXRInputProfile_MicrosoftMR = nullptr;
		XRInputProfile_MicrosoftXBox *m_pXRInputProfile_MicrosoftXBox = nullptr;
		XRInputProfile_OculusGo *m_pXRInputProfile_OculusGo = nullptr;
		XRInputProfile_OculusTouch *m_pXRInputProfile_OculusTouch = nullptr;
		XRInputProfile_ValveIndex* m_pXRInputProfile_ValveIndex = nullptr;

		/// Get the currently active interaction profile
		/// @param[in]	sPath	The string path to get the OpenXR Path
		/// @return		XrPath	The XrPath 
		XrPath GetXRPath( const char* sPath );

	};
} // namespace OpenXRProvider
