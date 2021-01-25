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
	XRProvider::XRProvider( XRAppInfo xrAppInfo, XRAppGraphicsInfo xrAppGraphicsInfo )
		: m_sAppName( xrAppInfo.AppName )
		, m_nAppVersion( xrAppInfo.AppVersion )
		, m_sEngineName( xrAppInfo.EngineName )
		, m_nEngineVersion( xrAppInfo.EngineVersion )
		, m_xrReferenceSpaceType( xrAppInfo.TrackingSpace == OpenXRProvider::TRACKING_ROOMSCALE ? XR_REFERENCE_SPACE_TYPE_STAGE : XR_REFERENCE_SPACE_TYPE_LOCAL )
		, m_vAppRequestedExtensions( xrAppInfo.XRExtensions )
	{
		// Set Loggers
		std::vector< spdlog::sink_ptr > vLogSinks;

		// Add file logging if requested
		if (xrAppInfo.LogFile)
			vLogSinks.push_back( std::make_shared< spdlog::sinks::basic_file_sink_st >( xrAppInfo.LogFile ) );

		vLogSinks.push_back( std::make_shared< spdlog::sinks::stdout_color_sink_st >() );	
		m_pLogger = std::make_shared< spdlog::logger >( LOG_TITLE, begin( vLogSinks ), end( vLogSinks ) );

		m_pLogger->set_level( spdlog::level::trace );
		m_pLogger->set_pattern( "[%Y-%b-%d %a] [%T %z] [%^%L%$] [%n] %v" );
		m_pLogger->info( "G'day from {}! Logging to: {}", LOG_TITLE, xrAppInfo.LogFile );
		m_pLogger->info( "Logs powered by spdlog version {}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH );
	
		// Create event handler
		m_pXREventHandler = new XREventHandler( m_pLogger );

		// Initialize OpenXR
		OpenXRInit( xrAppInfo.EnableDepthTextureSupport );

		// Setup world
		WorldInit( &xrAppGraphicsInfo );
	}

	XRProvider::~XRProvider()
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
			m_xrLastCallResult = XR_CALL( xrDestroySpace( m_xrSpace ), m_pLogger, false );

		// Destroy OpenXR Session
		if ( m_xrSession != XR_NULL_HANDLE )
			m_xrLastCallResult = XR_CALL( xrDestroySession( m_xrSession ), m_pLogger, false );

		// Destroy OpenXR Instance
		if ( m_xrInstance != XR_NULL_HANDLE )
			m_xrLastCallResult = XR_CALL( xrDestroyInstance( m_xrInstance ), m_pLogger, false );

		spdlog::shutdown();
	}

	void XRProvider::OpenXRInit( bool bEnableDepthTextureSupport )
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

		m_xrLastCallResult = XR_CALL( xrCreateInstance( &xrInstanceCreateInfo, &m_xrInstance ), m_pLogger, true );

		m_pLogger->info( "..." );
		m_pLogger->info( "XR Instance created: Handle {} with {} extension(s) enabled", ( uint64_t )m_xrInstance, nNumEnxtesions );
		m_pLogger->info(
			"Instance info: OpenXR version {}.{}.{}",
			XR_VERSION_MAJOR( XR_CURRENT_API_VERSION ),
			XR_VERSION_MINOR( XR_CURRENT_API_VERSION ),
			XR_VERSION_PATCH( XR_CURRENT_API_VERSION ) );
		m_pLogger->info( "Instance info: Application {} version {}", m_sAppName, m_nAppVersion );
		m_pLogger->info( "Instance info: Engine {} version {}", m_sEngineName, m_nEngineVersion );

		// ========================================================================
		// (4) Get xr system (representing a collection of physical xr devices) from active OpenXR runtime
		// ========================================================================
		LoadXRSystem();

	}

	void XRProvider::WorldInit( XRAppGraphicsInfo *pXRAppGraphicsInfo )
	{
		if ( m_xrInstance == XR_NULL_HANDLE )
		{
			std::string eMessage = "Aborting world init due to an invalid OpenXR Instance. Did OpenXRInit fail?";
			m_pLogger->error( "{}", eMessage );
			throw std::runtime_error( eMessage );
		}

		if ( m_xrSystemId == XR_NULL_SYSTEM_ID )
		{
			std::string eMessage = "Aborting world init due to an invalid System Id. Did OpenXRInit fail?";
			m_pLogger->error( "{}", eMessage );
			throw std::runtime_error( eMessage );
		}

		// ========================================================================
		// (1) Create a session (app communication instance to render frames or send/receive input to/from the runtime)
		// ========================================================================

		m_pXRGraphicsAPI = new XRGraphicsAPI( &m_xrInstance, &m_xrSystemId, pXRAppGraphicsInfo, &m_xrSession, &m_xrLastCallResult );

		if ( m_xrLastCallResult != XR_SUCCESS )
		{
			const char *xrEnumStr = XrEnumToString( m_xrLastCallResult );
			std::string eMessage = "Failed creating OpenXR Session with Error ";
			eMessage.append( xrEnumStr );

			m_pLogger->error( "{} ({})", eMessage, std::to_string( m_xrLastCallResult ) );
			throw std::runtime_error( eMessage );
		}

		m_pLogger->info( "XR Session for this app successfully created (Handle {})", ( uint64_t )m_xrSession );

		// ========================================================================
		// (2) Create a reference space (room-scale vs seated)
		// ========================================================================

		XrPosef xrPose {};
		xrPose.orientation.w = 1.f;

		XrReferenceSpaceCreateInfo xrReferenceSpaceCreateInfo { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		xrReferenceSpaceCreateInfo.poseInReferenceSpace = xrPose;
		xrReferenceSpaceCreateInfo.referenceSpaceType = m_xrReferenceSpaceType;

		m_xrLastCallResult = XR_CALL( xrCreateReferenceSpace( m_xrSession, &xrReferenceSpaceCreateInfo, &m_xrSpace ), m_pLogger, true );
		m_pLogger->info( "XR Reference Space for this app successfully created (Handle {})", ( uint64_t )m_xrSpace );

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

	XrResult XRProvider::LoadXRSystem()
	{
		if ( m_xrInstance == XR_NULL_HANDLE )
		{
			std::string eMessage = "No OpenXR Instance found. Make sure to call Init first";
			m_pLogger->error( "{}. Error ({})", eMessage, std::to_string( m_xrLastCallResult ) );
			throw std::runtime_error( eMessage );
		}

		// Get user's system info
		XrSystemGetInfo xrSystemGetInfo = {};
		xrSystemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
		xrSystemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

		m_xrLastCallResult = XR_CALL( xrGetSystem( m_xrInstance, &xrSystemGetInfo, &m_xrSystemId ), m_pLogger, true );

		// Log user's system info
		m_xrLastCallResult = XR_CALL( xrGetSystemProperties( m_xrInstance, m_xrSystemId, &m_xrSystemProperties ), m_pLogger, true );

		std::string systemName = m_xrSystemProperties.systemName;
		m_pLogger->info( "Active runtime: {} (Vendor Id {}) ", m_xrSystemProperties.systemName, m_xrSystemProperties.vendorId );

		return m_xrLastCallResult;
	}

	void XRProvider::EnableInstanceExtensions( std::vector< const char * > &vXRExtensions, bool bEnableDepthTextureSupport )
	{
		vXRExtensions.clear();

		// Log and enable runtime's available extensions
		uint32_t nExtensionCount;
		m_xrLastCallResult = XR_CALL( xrEnumerateInstanceExtensionProperties( nullptr, 0, &nExtensionCount, nullptr ), m_pLogger, false );

		std::vector< XrExtensionProperties > vExtensions;
		for ( uint32_t i = 0; i < nExtensionCount; ++i )
		{
			vExtensions.push_back( XrExtensionProperties { XR_TYPE_EXTENSION_PROPERTIES, nullptr } );
		}

		m_xrLastCallResult =
			XR_CALL( xrEnumerateInstanceExtensionProperties( nullptr, nExtensionCount, &nExtensionCount, vExtensions.data() ), m_pLogger, false );

		m_pLogger->info( "Runtime supports the following extensions (* = will be enabled):" );
		bool bEnable = false;
		for ( uint32_t i = 0; i < nExtensionCount; ++i )
		{
			// Check for graphics api extension
			if ( strcmp( s_GraphicsExtensionName, &vExtensions[ i ].extensionName[ 0 ] ) == 0 )
			{
				// Add graphics api to the list of extensions that would be enabled when we create the openxr instance
				vXRExtensions.push_back( s_GraphicsExtensionName );
				m_pLogger->info( "*{}. {} version {}", i + 1, vExtensions[ i ].extensionName, vExtensions[ i ].extensionVersion );

				bEnable = true;
			}

			// Check for depth extension
			else if ( bEnableDepthTextureSupport && 
				strcmp( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, &vExtensions[ i ].extensionName[ 0 ] ) == 0 )
			{
				// Add depth handling to the list of extensions that would be enabled when we create the openxr instance
				vXRExtensions.push_back( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME );
				m_pLogger->info( "*{}. {} version {}", i + 1, vExtensions[ i ].extensionName, vExtensions[ i ].extensionVersion );
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

						m_pLogger->info( "*{}. {} version {}", i + 1, vExtensions[ i ].extensionName, vExtensions[ i ].extensionVersion );

						bEnable = true;
						break;
					}
				}
			}
			
			if ( !bEnable )
				m_pLogger->info( "{}. {} version {}", i + 1, vExtensions[ i ].extensionName, vExtensions[ i ].extensionVersion );
			bEnable = false;
		}
	}

	void XRProvider::PollXREvents()
	{
		assert( m_xrInstance );
		assert( m_xrSession );

		XrEventDataBuffer xrEvent { XR_TYPE_EVENT_DATA_BUFFER };
		xrEvent.next = nullptr;

		m_xrLastCallResult = XR_CALL_SILENT( xrPollEvent( m_xrInstance, &xrEvent ), m_pLogger );

		// Stop evaluating if there's no event returned or the call fails
		if ( xrEvent.type == XR_TYPE_EVENT_DATA_BUFFER )
			return;

		// Execute any callbacks registered for this event
		ExecuteCallbacks( xrEvent );
	}

	XrResult XRProvider::XRBeginSession()
	{
		if ( m_xrSession == XR_NULL_HANDLE )
		{
			m_xrLastCallResult = XR_ERROR_HANDLE_INVALID;
			return m_xrLastCallResult;
		}

		XrSessionBeginInfo xrSessionBeginInfo { XR_TYPE_SESSION_BEGIN_INFO };
		xrSessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

		m_xrLastCallResult = XR_CALL_SILENT( xrBeginSession( m_xrSession, &xrSessionBeginInfo ), m_pLogger );
		return m_xrLastCallResult;
	}

	XrResult XRProvider::XREndSession() 
	{
		if ( m_xrSession == XR_NULL_HANDLE )
		{
			m_xrLastCallResult = XR_ERROR_HANDLE_INVALID;
			return m_xrLastCallResult;
		}

		m_xrLastCallResult = XR_CALL_SILENT( xrEndSession( m_xrSession ), m_pLogger );
		return m_xrLastCallResult;
	}

	void XRProvider::ExecuteCallbacks( XrEventDataBuffer xrEvent )
	{
		for each( XRCallback * xrCallback in m_pXREventHandler->GetXRCallbacks() )
			{
				if ( xrCallback->type == xrEvent.type || xrCallback->type == XR_TYPE_EVENT_DATA_BUFFER )
					xrCallback->callback( xrEvent );
			}
	}

} // namespace OpenXRProvider
