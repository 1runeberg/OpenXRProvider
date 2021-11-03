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

#include <OpenXRProvider.h>

namespace OpenXRProvider
{
	XRCore::XRCore( XRAppInfo xrAppInfo, XRAppGraphicsInfo xrAppGraphicsInfo, bool bEnableDepth )
		: m_sAppName( xrAppInfo.AppName )
		, m_nAppVersion( xrAppInfo.AppVersion )
		, m_sEngineName( xrAppInfo.EngineName )
		, m_nEngineVersion( xrAppInfo.EngineVersion )
		, m_xrReferenceSpaceType( xrAppInfo.TrackingSpace == OpenXRProvider::TRACKING_ROOMSCALE ? XR_REFERENCE_SPACE_TYPE_STAGE : XR_REFERENCE_SPACE_TYPE_LOCAL )
		, m_vAppRequestedExtensions( xrAppInfo.XRExtensions )
	{	
		GetLogMessage()->clear();

		// Create event handler
		m_pXREventHandler = new XREventHandler();

		// Initialize OpenXR
		OpenXRInit( bEnableDepth );

		// Setup world
		WorldInit( &xrAppGraphicsInfo );
	}

	XRCore::~XRCore()
	{
		// Destroy hand tracker
		if ( m_pXRHandTracking )
			delete m_pXRHandTracking;

		// Destroy graphics API extension
		if ( m_pXRGraphicsAPI )
			delete m_pXRGraphicsAPI;

		// Destroy event handler
		if ( m_pXREventHandler )
			delete m_pXREventHandler;
			
		// Destroy OpenXR Reference Space
		if ( m_xrSpace != XR_NULL_HANDLE )
			m_xrLastCallResult = XR_CALL( xrDestroySpace( m_xrSpace ), GetLogMessage(), false );

		// Destroy OpenXR Session
		if ( m_xrSession != XR_NULL_HANDLE )
			m_xrLastCallResult = XR_CALL( xrDestroySession( m_xrSession ), GetLogMessage(), false );

		// Destroy OpenXR Instance
		if ( m_xrInstance != XR_NULL_HANDLE )
			m_xrLastCallResult = XR_CALL( xrDestroyInstance( m_xrInstance ), GetLogMessage(), false );
	}

	void XRCore::OpenXRInit( bool bEnableDepthTextureSupport )
	{

		// ========================================================================
		// (1) Set application general info(e.g.App name, version, engine name, etc)
		// ========================================================================

		XrInstanceCreateInfo xrInstanceCreateInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
		xrInstanceCreateInfo.applicationInfo = {};

		size_t nAppNameSize = m_sAppName.size() + 1;
		if ( nAppNameSize > XR_MAX_APPLICATION_NAME_SIZE )
			nAppNameSize = XR_MAX_APPLICATION_NAME_SIZE;

		strncpy_s( xrInstanceCreateInfo.applicationInfo.applicationName, m_sAppName.c_str(), nAppNameSize );

		size_t nEngineNameSize = m_sEngineName.size() + 1;
		if ( nEngineNameSize > XR_MAX_ENGINE_NAME_SIZE )
			nEngineNameSize = XR_MAX_ENGINE_NAME_SIZE;
		strncpy_s( xrInstanceCreateInfo.applicationInfo.engineName, m_sEngineName.c_str(), nEngineNameSize );

		xrInstanceCreateInfo.applicationInfo.applicationVersion = m_nAppVersion;
		xrInstanceCreateInfo.applicationInfo.engineVersion = m_nEngineVersion;
		xrInstanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

		// ========================================================================
		// (2) Check runtime's supported extensions and tag ones selected by the app to enable (e.g. Graphics type, Visibility Mask, Handtracking, etc)
		// ========================================================================

		EnableInstanceExtensions( m_vAppEnabledExtensions, bEnableDepthTextureSupport );
		uint32_t nNumEnxtesions = m_vAppEnabledExtensions.empty() ? 0 : ( uint32_t )m_vAppEnabledExtensions.size();

		if ( nNumEnxtesions > 0 )
		{
			xrInstanceCreateInfo.enabledExtensionCount = nNumEnxtesions;
			xrInstanceCreateInfo.enabledExtensionNames = m_vAppEnabledExtensions.data();
		}

		// ========================================================================
		// (3) Create an instance (object that allows communication to the runtime)
		// ========================================================================

		m_xrLastCallResult = XR_CALL( xrCreateInstance( &xrInstanceCreateInfo, &m_xrInstance ), GetLogMessage(), true );
		GetLogMessage()->append( "\n XR Instance created.\n" );

		// ========================================================================
		// (4) Get xr system (representing a collection of physical xr devices) from active OpenXR runtime
		// ========================================================================
		LoadXRSystem();

	}

	void XRCore::WorldInit( XRAppGraphicsInfo *pXRAppGraphicsInfo )
	{
		GetLogMessage()->clear();

		if ( m_xrInstance == XR_NULL_HANDLE )
		{
			GetLogMessage()->append( "Error: Aborting world init due to an invalid OpenXR Instance. Did OpenXRInit fail?" );
			throw std::runtime_error( *GetLogMessage() );
		}

		if ( m_xrSystemId == XR_NULL_SYSTEM_ID )
		{
			GetLogMessage()->append( "Aborting world init due to an invalid System Id. Did OpenXRInit fail?" );
			throw std::runtime_error( *GetLogMessage() );
		}

		// ========================================================================
		// (1) Create a session (app communication instance to render frames or send/receive input to/from the runtime)
		// ========================================================================

		m_pXRGraphicsAPI = new XRGraphicsAPI( &m_xrInstance, &m_xrSystemId, pXRAppGraphicsInfo, &m_xrSession, &m_xrLastCallResult );

		if ( m_xrLastCallResult != XR_SUCCESS )
		{
			GetLogMessage()->append( "\nFailed creating OpenXR Session with Error " );
			GetLogMessage()->append( XrEnumToString( m_xrLastCallResult ) );
			GetLogMessage()->append( "\n" );

			throw std::runtime_error( *GetLogMessage() );
		}

		GetLogMessage()->append( "\nXR Session for this app successfully created.\n" );

		// ========================================================================
		// (2) Create a reference space (room-scale vs seated)
		// ========================================================================

		XrPosef xrPose {};
		xrPose.orientation.w = 1.f;

		XrReferenceSpaceCreateInfo xrReferenceSpaceCreateInfo { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		xrReferenceSpaceCreateInfo.poseInReferenceSpace = xrPose;
		xrReferenceSpaceCreateInfo.referenceSpaceType = m_xrReferenceSpaceType;

		m_xrLastCallResult = XR_CALL( xrCreateReferenceSpace( m_xrSession, &xrReferenceSpaceCreateInfo, &m_xrSpace ), GetLogMessage(), true );
		GetLogMessage()->append( "XR Reference Space for this app successfully created." );

		// ========================================================================
		// (3) Keep track of instance extensions that's not render or input based
		// ========================================================================
		for	each( void *xrExtension in GetXREnabledExtensions() )
			{
				XRBaseExt *xrInstanceExtension = static_cast< XRBaseExt * >( xrExtension );

				// Hand tracking
				if ( strcmp( xrInstanceExtension->GetExtensionName(), XR_EXT_HAND_TRACKING_EXTENSION_NAME ) == 0 )
				{
					// Set the extension member
					m_pXRHandTracking = static_cast< XRExtHandTracking * >( xrExtension );

					// Initialize extension
					m_pXRHandTracking->Init( GetXRInstance(), GetXRSession() );
					break;
				}
			}
	}

	XrResult XRCore::LoadXRSystem()
	{
		if ( m_xrInstance == XR_NULL_HANDLE )
		{
			GetLogMessage()->append( "No OpenXR Instance found. Make sure to call Init first" );
			throw std::runtime_error( *GetLogMessage() );
		}

		// Get user's system info
		XrSystemGetInfo xrSystemGetInfo = {};
		xrSystemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
		xrSystemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

		m_xrLastCallResult = XR_CALL( xrGetSystem( m_xrInstance, &xrSystemGetInfo, &m_xrSystemId ), GetLogMessage(), true );

		// Log user's system info
		m_xrLastCallResult = XR_CALL( xrGetSystemProperties( m_xrInstance, m_xrSystemId, &m_xrSystemProperties ), GetLogMessage(), true );

		std::string systemName = m_xrSystemProperties.systemName;
		GetLogMessage()->append( "\nActive runtime: " + systemName + "\n" ); 

		return m_xrLastCallResult;
	}

	void XRCore::EnableInstanceExtensions( std::vector< const char * > &vXRExtensions, bool bEnableDepthTextureSupport )
	{
		vXRExtensions.clear();

		// Log and enable runtime's available extensions
		uint32_t nExtensionCount;
		m_xrLastCallResult = XR_CALL( xrEnumerateInstanceExtensionProperties( nullptr, 0, &nExtensionCount, nullptr ), GetLogMessage(), false );

		std::vector< XrExtensionProperties > vExtensions;
		for ( uint32_t i = 0; i < nExtensionCount; ++i )
		{
			vExtensions.push_back( XrExtensionProperties { XR_TYPE_EXTENSION_PROPERTIES, nullptr } );
		}

		m_xrLastCallResult =
			XR_CALL( xrEnumerateInstanceExtensionProperties( nullptr, nExtensionCount, &nExtensionCount, vExtensions.data() ), GetLogMessage(), false );

		GetLogMessage()->append( "Runtime supports the following extensions (* = will be enabled):" );
		bool bEnable = false;
		for ( uint32_t i = 0; i < nExtensionCount; ++i )
		{
			// Check for graphics api extension
			if ( strcmp( s_GraphicsExtensionName, &vExtensions[ i ].extensionName[ 0 ] ) == 0 )
			{
				// Add graphics api to the list of extensions that would be enabled when we create the openxr instance
				vXRExtensions.push_back( s_GraphicsExtensionName );
				std::string sMessage = "\n*" + std::to_string( i + 1 ) + ". " + vExtensions[i].extensionName + std::to_string( vExtensions[i].extensionVersion ) + "\n"; 
				GetLogMessage()->append( sMessage );

				bEnable = true;
			}

			// Check for depth extension
			else if ( bEnableDepthTextureSupport && 
				strcmp( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, &vExtensions[ i ].extensionName[ 0 ] ) == 0 )
			{
				// Add depth handling to the list of extensions that would be enabled when we create the openxr instance
				vXRExtensions.push_back( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME );
				std::string sMessage = "\n*" + std::to_string( i + 1 ) + ". " + vExtensions[ i ].extensionName + std::to_string( vExtensions[ i ].extensionVersion ) + "\n";
				GetLogMessage()->append( sMessage );

				m_bIsDepthSupported = true;
				bEnable = true;
			}
			else
			{
				// Otherwise, check if this extension was requested by the app
				for ( size_t j = 0; j < m_vAppRequestedExtensions.size(); j++ )
				{
					XRBaseExt *xrRequestedExtension = static_cast< XRBaseExt * >( m_vAppRequestedExtensions[ j ] );

					//m_pLogger->info("Processing extension: {} is equal to {}", xrRequestedExtension->GetExtensionName(), &vExtensions[i].extensionName[0]);
					
					if ( strcmp( xrRequestedExtension->GetExtensionName(), &vExtensions[ i ].extensionName[ 0 ] ) == 0 )
					{
						// Add to the list of extensions that would be enabled when we create the openxr instance
						vXRExtensions.push_back( xrRequestedExtension->GetExtensionName() );

						xrRequestedExtension->IsActive( true );
						m_vXRAppEnabledExtensions.push_back( m_vAppRequestedExtensions[ j ] );

						std::string sMessage = "\n*" + std::to_string( i + 1 ) + ". " + vExtensions[ i ].extensionName + std::to_string( vExtensions[ i ].extensionVersion ) + "\n";
						GetLogMessage()->append( sMessage );

						bEnable = true;
						break;
					}
				}
			}
			
			if ( !bEnable )
			{
				std::string sMessage = "\n" + std::to_string( i + 1 ) + ". " + vExtensions[ i ].extensionName + std::to_string( vExtensions[ i ].extensionVersion ) + "\n";
				GetLogMessage()->append( sMessage );
			}
				
			bEnable = false;
		}
	}

	void XRCore::PollXREvents()
	{
		assert( m_xrInstance );
		assert( m_xrSession );

		XrEventDataBuffer xrEvent { XR_TYPE_EVENT_DATA_BUFFER };
		xrEvent.next = nullptr;

		m_xrLastCallResult = XR_CALL_SILENT( xrPollEvent( m_xrInstance, &xrEvent ), GetLogMessage() );

		// Stop evaluating if there's no event returned or the call fails
		if ( xrEvent.type == XR_TYPE_EVENT_DATA_BUFFER )
			return;

		// Execute any callbacks registered for this event
		ExecuteCallbacks( xrEvent );
	}

	XrResult XRCore::XRBeginSession()
	{
		if ( m_xrSession == XR_NULL_HANDLE )
		{
			m_xrLastCallResult = XR_ERROR_HANDLE_INVALID;
			return m_xrLastCallResult;
		}

		XrSessionBeginInfo xrSessionBeginInfo { XR_TYPE_SESSION_BEGIN_INFO };
		xrSessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

		m_xrLastCallResult = XR_CALL_SILENT( xrBeginSession( m_xrSession, &xrSessionBeginInfo ), GetLogMessage() );
		return m_xrLastCallResult;
	}

	XrResult XRCore::XREndSession() 
	{
		if ( m_xrSession == XR_NULL_HANDLE )
		{
			m_xrLastCallResult = XR_ERROR_HANDLE_INVALID;
			return m_xrLastCallResult;
		}

		m_xrLastCallResult = XR_CALL_SILENT( xrEndSession( m_xrSession ), GetLogMessage() );
		return m_xrLastCallResult;
	}

	void XRCore::ExecuteCallbacks( XrEventDataBuffer xrEvent )
	{
		for each( XRCallback * xrCallback in m_pXREventHandler->GetXRCallbacks() )
			{
				if ( xrCallback->type == xrEvent.type || xrCallback->type == XR_TYPE_EVENT_DATA_BUFFER )
					xrCallback->callback( xrEvent );
			}
	}

} // namespace OpenXRProvider
