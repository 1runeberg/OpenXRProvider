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

#include <input/XRInput.h>

namespace OpenXRProvider
{
	XRInput::XRInput( XRCore *pXRCore, XRRender* pXRRender )
		: m_pXRCore( pXRCore )
		, m_pXRRender( pXRRender )
	{
		if ( !m_pXRCore && !m_pXRCore->GetLogger() )
			throw std::runtime_error( "Failed to create XR Input Manager. Invalid XR Core provided." );

		// Retain pointer to logger
		m_pXRLogger = m_pXRCore->GetLogger();

		// Check xr render manager
		if ( !m_pXRRender )
		{
			std::string eMessage = "Failed to create XR Input manager due to an Invalid XR Render Manager.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Check xr instance
		if ( m_pXRCore->GetXRInstance() == XR_NULL_HANDLE )
		{
			std::string eMessage = "Failed to create XR Input manager due to an Invalid XR Instance.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Check xr session
		if ( m_pXRCore->GetXRSession() == XR_NULL_HANDLE )
		{
			std::string eMessage = "Failed to create XR Input manager due to an Invalid XR Session.";
			m_pXRLogger->error( eMessage );
			throw std::runtime_error( eMessage );
		}

		// Generate all supported input profiles
		GenerateInputProfiles();

		m_pXRLogger->info( "Input manager created successfully" );
	}

	XRInput::~XRInput()
	{
		// Delete actions
		if ( m_vActions.size() > 0 )
		{
			for	each( XrAction xrAction in m_vActions ) 
				m_xrLastCallResult = xrDestroyAction( xrAction );
		}

		// Delete action sets
		if (m_vActionSets.size() > 0)
		{
			for	each( XrActionSet xrActionSet in m_vActionSets ) 
				m_xrLastCallResult = xrDestroyActionSet( xrActionSet );
		}

		// Delete input profiles

		if ( m_pXRInputProfile_GoogleDaydream )
			delete m_pXRInputProfile_GoogleDaydream;

		if ( m_pXRInputProfile_HTCVive )
			delete m_pXRInputProfile_HTCVive;

		if ( m_pXRInputProfile_HTCVivePro )
			delete m_pXRInputProfile_HTCVivePro;

		if ( m_pXRInputProfile_KhronosSimple )
			delete m_pXRInputProfile_KhronosSimple;

		if ( m_pXRInputProfile_MicrosoftMR )
			delete m_pXRInputProfile_MicrosoftMR;

		if ( m_pXRInputProfile_MicrosoftXBox )
			delete m_pXRInputProfile_MicrosoftXBox;

		if ( m_pXRInputProfile_OculusGo )
			delete m_pXRInputProfile_OculusGo;

		if ( m_pXRInputProfile_OculusTouch )
			delete m_pXRInputProfile_OculusTouch;

		if ( m_pXRInputProfile_ValveIndex )
			delete m_pXRInputProfile_ValveIndex;
	}

	void XRInput::GenerateInputProfiles()
	{
		assert( m_pXRCore && m_pXRCore->GetXRSession() != XR_NULL_HANDLE );

		// Google Daydream
		m_pXRInputProfile_GoogleDaydream = new XRInputProfile_GoogleDaydream( m_pXRLogger );

		// HTC Vive
		m_pXRInputProfile_HTCVive = new XRInputProfile_HTCVive( m_pXRLogger );

		// HTC Vive Pro
		m_pXRInputProfile_HTCVivePro = new XRInputProfile_HTCVivePro( m_pXRLogger );

		// Khronos Simple
		m_pXRInputProfile_KhronosSimple = new XRInputProfile_KhronosSimple( m_pXRLogger );

		// Microsoft MR
		m_pXRInputProfile_MicrosoftMR = new XRInputProfile_MicrosoftMR( m_pXRLogger );

		// Microsoft XBox
		m_pXRInputProfile_MicrosoftXBox = new XRInputProfile_MicrosoftXBox(  m_pXRLogger );

		// Oculus Go
		m_pXRInputProfile_OculusGo = new XRInputProfile_OculusGo( m_pXRLogger );

		// Oculus Touch
		m_pXRInputProfile_OculusTouch = new XRInputProfile_OculusTouch( m_pXRLogger );

		// Valve Index Controller
		m_pXRInputProfile_ValveIndex = new XRInputProfile_ValveIndex( m_pXRLogger );
	}

	XrPath XRInput::GetXRPath( const char *sPath ) 
	{
		XrPath xrPath;
		xrStringToPath( m_pXRCore->GetXRInstance(), sPath, &xrPath );

		return xrPath;
	}

	XrActionSet XRInput::CreateActionSet( const char *pName, const char *pLocalizedName, uint32_t nPriority ) 
	{
		assert( m_pXRCore && m_pXRCore->GetXRInstance() != XR_NULL_HANDLE );

		XrActionSetCreateInfo xrActionSetCreateInfo { XR_TYPE_ACTION_SET_CREATE_INFO };
		strcpy_s( xrActionSetCreateInfo.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, pName );
		strcpy_s( xrActionSetCreateInfo.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, pLocalizedName );
		xrActionSetCreateInfo.priority = nPriority;

		XrActionSet xrActionSet;
		m_xrLastCallResult = XR_CALL_SILENT( xrCreateActionSet( m_pXRCore->GetXRInstance(), &xrActionSetCreateInfo, &xrActionSet ), m_pXRLogger );

		if ( m_xrLastCallResult == XR_SUCCESS )
			m_vActionSets.push_back( xrActionSet );
		else
			m_pXRLogger->error( "Unable to create action set {}. Runtime returned {}. Action set names should only contain lower ASCII characters, numbers, dash, period or forward slash", 
				pName, XrEnumToString( m_xrLastCallResult ) );

		return xrActionSet;
	}

	XrAction XRInput::CreateAction( XrActionSet xrActionSet, const char *pName, const char *pLocalizedName, 
		XrActionType xrActionType, uint32_t nFilterCount, XrPath *xrFilters )
	{
		assert( xrActionSet != 0 && m_pXRCore->GetXRSession() != XR_NULL_HANDLE );

		// Create action
		XrActionCreateInfo xrActionCreateInfo { XR_TYPE_ACTION_CREATE_INFO };
		strcpy_s( xrActionCreateInfo.actionName, XR_MAX_ACTION_SET_NAME_SIZE, pName );
		strcpy_s( xrActionCreateInfo.localizedActionName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, pLocalizedName );
		xrActionCreateInfo.actionType = xrActionType;
		xrActionCreateInfo.countSubactionPaths = nFilterCount;
		xrActionCreateInfo.subactionPaths = xrFilters;

		XrAction xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrCreateAction( xrActionSet, &xrActionCreateInfo, &xrAction ), m_pXRLogger );

		if ( m_xrLastCallResult == XR_SUCCESS )
		{
			// Add action to array of created actions for this session
			m_vActions.push_back( xrAction );

			// If this is a pose action, create a corresponding action space
			if ( xrActionType == XR_ACTION_TYPE_POSE_INPUT )
			{
				XrPosef xrPose {};
				xrPose.orientation.w = 1.f;

				XrActionSpaceCreateInfo xrActionSpaceCreateInfo { XR_TYPE_ACTION_SPACE_CREATE_INFO };
				xrActionSpaceCreateInfo.action = xrAction;
				xrActionSpaceCreateInfo.poseInActionSpace = xrPose;
				xrActionSpaceCreateInfo.subactionPath = nFilterCount > 0 ? xrFilters[0] : XR_NULL_PATH;

				XrSpace xrSpace;
				m_xrLastCallResult = xrCreateActionSpace( m_pXRCore->GetXRSession(), &xrActionSpaceCreateInfo, &xrSpace );

				if ( m_xrLastCallResult == XR_SUCCESS )
				{
					m_mapActionSpace.insert( std::pair< XrAction, XrSpace >( xrAction, xrSpace ) );
					m_pXRLogger->info( "Action {} created with reference space handle ({})", pName, ( uint64_t )xrSpace );
				}
				else
				{
					m_pXRLogger->error(	"Unable to create an action space for action {}. Result was {}", pName,	XrEnumToString( m_xrLastCallResult ) );
				}

				return xrAction;
			}
		}
		else
		{
			m_pXRLogger->error( "Unable to create action {}. Runtime returned {}. Action names should only contain lower ASCII characters, numbers, dash, period or forward slash",
				pName, XrEnumToString( m_xrLastCallResult ) );

			return xrAction;
		}

		m_pXRLogger->info( "Action {} created", pName );
		return xrAction;
	}

	XrResult XRInput::StringToXrPath( const char *sString, XrPath *xrPath )
	{
		assert( sString && m_pXRCore && m_pXRCore->GetXRInstance() != XR_NULL_HANDLE );

		m_xrLastCallResult = XR_CALL_SILENT( xrStringToPath( m_pXRCore->GetXRInstance(), sString, xrPath ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::CreateInputPath( const char *sControllerPath, const char *sComponentPath, XrPath *xrPath ) 
	{ 
		assert( m_pXRCore && m_pXRCore->GetXRInstance() != XR_NULL_HANDLE );
		assert( sControllerPath && sComponentPath && xrPath );

		char sFullPath[ XR_MAX_PATH_LENGTH ];
		strcpy_s( sFullPath, XR_MAX_PATH_LENGTH, sControllerPath );
		strcat_s( sFullPath, XR_MAX_PATH_LENGTH, sComponentPath );

		return StringToXrPath( sFullPath, xrPath );
	}

	XrActionSuggestedBinding XRInput::CreateActionBinding( XrAction xrAction, const char *sControllerPath, const char *sComponentPath, std::vector< XrActionSuggestedBinding > *vActionBindings ) 
	{ 
		assert( xrAction != 0 );

		// Check if the component path is a full path (i.e. already has the controller path such as /user/hand/left/input/pose
		// this can happen for controllers such as Oculus touch with handed keys (e.g. X,Y keys are only for the left hand)
		bool bIsFullPath = ( strstr( "/user/", sComponentPath ) != NULL );

		XrPath xrPath;
		if ( bIsFullPath )
			StringToXrPath( sComponentPath, &xrPath );
		else
			CreateInputPath( sControllerPath, sComponentPath, &xrPath );

		XrActionSuggestedBinding xrActionSuggestedBinding;
		xrActionSuggestedBinding.action = xrAction;
		xrActionSuggestedBinding.binding = xrPath;

		vActionBindings->push_back( xrActionSuggestedBinding );

		return xrActionSuggestedBinding;
	}

	XrResult XRInput::SuggestActionBindings( std::vector< XrActionSuggestedBinding > *vActionBindings, const char *sInteractionProfilePath ) 
	{
		assert
		(	
			m_pXRCore &&
			m_pXRCore->GetXRInstance() != XR_NULL_HANDLE  &&
			m_pXRCore->GetXRSession() != XR_NULL_HANDLE 
		);

		XrPath xrPath;
		xrStringToPath( m_pXRCore->GetXRInstance(), sInteractionProfilePath, &xrPath );

		m_xrLastCallResult = XR_ERROR_VALIDATION_FAILURE;
		if ( vActionBindings->size() < 1 )
		{
			m_pXRLogger->error( "No action bindings found. Create action bindings prior to calling SuggestActionBindings()" );
			return m_xrLastCallResult;
		}
		
		XrInteractionProfileSuggestedBinding xrInteractionProfileSuggestedBinding { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		xrInteractionProfileSuggestedBinding.interactionProfile = xrPath;
		xrInteractionProfileSuggestedBinding.suggestedBindings = vActionBindings->data();
		xrInteractionProfileSuggestedBinding.countSuggestedBindings = ( uint32_t )vActionBindings->size();

		m_xrLastCallResult = XR_CALL_SILENT( xrSuggestInteractionProfileBindings( 
			m_pXRCore->GetXRInstance(), &xrInteractionProfileSuggestedBinding ), m_pXRLogger );

		if ( m_xrLastCallResult != XR_SUCCESS )
			return m_xrLastCallResult;

		m_pXRLogger->info( "Interaction profile suggested to runtime: {}", sInteractionProfilePath );

		return m_xrLastCallResult;
	}

	void XRInput::ActivateActionSet( XrActionSet xrActionSet, XrPath xrFilter /*= XR_NULL_PATH */ ) 
	{ 
		assert( xrActionSet != XR_NULL_HANDLE );

		XrActiveActionSet xrActiveActionSet { xrActionSet, xrFilter };
		m_vActiveActionSets.push_back( xrActiveActionSet );

		XrSessionActionSetsAttachInfo xrSessionActionSetsAttachInfo { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
		xrSessionActionSetsAttachInfo.countActionSets = ( uint32_t )m_vActionSets.size();
		xrSessionActionSetsAttachInfo.actionSets = m_vActionSets.data();

		m_xrLastCallResult = XR_CALL_SILENT( xrAttachSessionActionSets( m_pXRCore->GetXRSession(), &xrSessionActionSetsAttachInfo ), m_pXRLogger );

		if ( m_xrLastCallResult == XR_SUCCESS )
			m_pXRLogger->info( "{} action sets attached to the current session ({})", xrSessionActionSetsAttachInfo.countActionSets, ( uint64_t )m_pXRCore->GetXRSession() );

	}

	XrResult XRInput::SyncActiveActionSetsData() 
	{ 
		assert( m_pXRCore && m_pXRCore->GetXRSession() );

		if ( m_vActiveActionSets.size() < 1 )
			return XR_SUCCESS;

		XrActionsSyncInfo xrActionSyncInfo { XR_TYPE_ACTIONS_SYNC_INFO };
		xrActionSyncInfo.countActiveActionSets = ( uint32_t )m_vActiveActionSets.size();
		xrActionSyncInfo.activeActionSets = m_vActiveActionSets.data();

		m_xrLastCallResult = XR_CALL_SILENT( xrSyncActions( m_pXRCore->GetXRSession(), &xrActionSyncInfo ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionPose( XrAction xrAction, XrTime xrTime, XrSpaceLocation *xrLocation ) 
	{
		// Find the action space for the action
		std::map< XrAction, XrSpace >::iterator const iter = m_mapActionSpace.find( xrAction );
		if ( iter != m_mapActionSpace.end() )
			m_xrLastCallResult = xrLocateSpace( iter->second, m_pXRCore->GetXRSpace(), xrTime, xrLocation );
		else
			return XR_ERROR_VALIDATION_FAILURE;

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateBoolean( XrAction xrAction, XrActionStateBoolean *xrActionState ) 
	{ 
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo { XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateBoolean( 
			m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateFloat( XrAction xrAction, XrActionStateFloat *xrActionState ) 
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo { XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateFloat( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateVector2f( XrAction xrAction, XrActionStateVector2f *xrActionState ) 
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo { XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateVector2f( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStatePose( XrAction xrAction, XrActionStatePose *xrActionState ) 
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo { XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStatePose( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	const char *XRInput::GetCurrentInteractionProfile( const char *sUserPath ) 
	{
		assert( m_pXRCore->GetXRSession() );

		XrPath xrPath = GetXRPath( sUserPath );

		XrInteractionProfileState xrInteractionProfileState { XR_TYPE_INTERACTION_PROFILE_STATE };
		m_xrLastCallResult = xrGetCurrentInteractionProfile(m_pXRCore->GetXRSession(), xrPath, &xrInteractionProfileState);

		XrPath xrInputProfilePath = GetXRPath( m_pXRInputProfile_ValveIndex->GetInputProfile() );

		if ( m_xrLastCallResult == XR_SUCCESS )
		{
			//if (xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_ValveIndex->GetInputProfile() ) )
			//	return m_pXRInputProfile_ValveIndex->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_KhronosSimple->GetInputProfile() ) )
			//	return m_pXRInputProfile_KhronosSimple->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_HTCVive->GetInputProfile() ) )
			//	return m_pXRInputProfile_HTCVive->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_HTCVivePro->GetInputProfile() ) )
			//	return m_pXRInputProfile_HTCVivePro->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_OculusTouch->GetInputProfile() ) )
			//	return m_pXRInputProfile_OculusTouch->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_OculusGo->GetInputProfile() ) )
			//	return m_pXRInputProfile_OculusGo->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_MicrosoftMR->GetInputProfile() ) )
			//	return m_pXRInputProfile_MicrosoftMR->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_MicrosoftXBox->GetInputProfile() ) )
			//	return m_pXRInputProfile_MicrosoftXBox->GetInputProfile();
			//else if ( xrInteractionProfileState.interactionProfile == GetXRPath( m_pXRInputProfile_GoogleDaydream->GetInputProfile() ) )
			//	return m_pXRInputProfile_GoogleDaydream->GetInputProfile();

			// Translate xrpath
			char buf[ MAX_PATH ];
			uint32_t nCapacity = 0;
			xrPathToString( m_pXRCore->GetXRInstance(), xrInteractionProfileState.interactionProfile, MAX_PATH, &nCapacity, buf );
			if (nCapacity > 0)
				m_pXRLogger->info( "Interaction profile for {} changed to: {}", sUserPath, buf );
		}

		return "";
	}

	XrResult XRInput::GenerateHaptic(
		XrAction xrAction,
		uint64_t nDuration /*= XR_MIN_HAPTIC_DURATION */,
		float fAmplitude /*= 0.5f*/,
		float fFrequency /*= XR_FREQUENCY_UNSPECIFIED*/)
	{
		assert( xrAction != XR_NULL_HANDLE && m_pXRCore && m_pXRCore->GetXRSession() );

		XrHapticVibration xrHapticVibration { XR_TYPE_HAPTIC_VIBRATION };
		xrHapticVibration.duration = nDuration;
		xrHapticVibration.amplitude = fAmplitude;
		xrHapticVibration.frequency = fFrequency;

		XrHapticActionInfo xrHapticActionInfo { XR_TYPE_HAPTIC_ACTION_INFO };
		xrHapticActionInfo.action = xrAction;

		m_xrLastCallResult = XR_CALL_SILENT( 
			xrApplyHapticFeedback( m_pXRCore->GetXRSession(), &xrHapticActionInfo, ( const XrHapticBaseHeader * )&xrHapticVibration ), m_pXRLogger );

		return m_xrLastCallResult;
	}

} // namespace OpenXRProvider
