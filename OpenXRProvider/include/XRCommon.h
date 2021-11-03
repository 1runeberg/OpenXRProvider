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

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Provider classes
#include <XRBase.h>
#include <XRBaseExt.h>

// Platform and graphic apis
#include <rendering/XRGraphicsAPIDefines.h>
#include <XRPlatformDefines.h>

// OpenXR Headers
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>
#include <openxr/xr_linear.h>


namespace OpenXRProvider
{
	//** MACRO DEFINITIONS */

	/// Gets the string value of an enum (as per sample code in openxr_reflection.h)
	#define XR_ENUM_STRINGIFY( sEnum, val )	case sEnum: return #sEnum;
	#define XR_ENUM_TYPE_STRINGIFY( xrEnumType ) constexpr const char* XrEnumToString( xrEnumType eNum ) \
	{																		  						     \
		switch ( eNum ) { XR_LIST_ENUM_##xrEnumType( XR_ENUM_STRINGIFY )  		   					     \
		default : return "Unknown Enum. Define in openxr_reflection.h";	}				 			     \
	}																							

	XR_ENUM_TYPE_STRINGIFY( XrResult );
	XR_ENUM_TYPE_STRINGIFY( XrSessionState );
	XR_ENUM_TYPE_STRINGIFY( XrReferenceSpaceType );

	/// Execute a call to the OpenXR api and evaluate the result, will log success and fail and throw an exception if request
	#define XR_CALL( xrCall, sMessage, bThrow ) CheckXrResult( xrCall, sMessage, bThrow, #xrCall, __FILE__, __LINE__ );
	inline XrResult CheckXrResult(
		XrResult xrResult,
		std::string *sMessage,
		bool bThrow,
		const char *xrCall = nullptr,
		const char *srcFile = nullptr,
		int srcLineNum = 0 )
	{
		sMessage->clear();

		if ( XR_UNQUALIFIED_SUCCESS( xrCall ) )
		{
			if ( xrCall )
				sMessage->append( xrCall );

			sMessage->append( ": Error on OpenXR call. Error: " ); 
			sMessage->append( XrEnumToString(xrResult) );

			sMessage->append( " in file: ");
			sMessage->append( srcFile );

			sMessage->append( " line: " );
			sMessage->append( std::to_string( srcLineNum ) );

			if ( bThrow )
				throw std::runtime_error( *sMessage );
		}
		else
		{
			sMessage->append( xrCall );
			sMessage->append( ": Success." );
		}

		return xrResult;
	}

	/// Execute a call to the OpenXR api and evaluate the result, will log failures only
	#define XR_CALL_SILENT( xrCall, sMessage ) CheckXrResult( xrCall, sMessage, #xrCall, __FILE__, __LINE__ );
	inline XrResult CheckXrResult(
		XrResult xrResult,
		std::string* sMessage,
		const char *xrCall = nullptr,
		const char *srcFile = nullptr,
		int srcLineNum = 0 )
	{
		sMessage->clear();

		if ( XR_UNQUALIFIED_SUCCESS( xrCall ) )
		{
			if ( xrCall )
				sMessage->append( xrCall );

			sMessage->append( ": Error on OpenXR call. Error: " );
			sMessage->append( XrEnumToString( xrResult ) );

			sMessage->append( " in file: " );
			sMessage->append( srcFile );

			sMessage->append( " line: " );
			sMessage->append( std::to_string( srcLineNum ) );
		}

		return xrResult;
	}


	//** ENUMS */

	/// Tracking space (e.g. roomscale, seated)
	enum EXRTrackingSpace
	{
		TRACKING_ROOMSCALE = 0,
		TRACKING_SEATED = 1
	};

	/// Eye
	enum EXREye
	{
		EYE_LEFT = 0,
		EYE_RIGHT = 1
	};

	/// Supported extensions by the OpenXR Provider library (excluding Graphics API extensions)
	enum EXRInstanceExtension
	{
		EXT_INSTANCE_VISIBILITY_MASK = 0,
		EXT_INSTANCE_HAND_TRACKING = 1
	};


	//** STRUCTS */

	/// OpenXR application info used to initialize an OpenXR Instance and create a Session with the active OpenXR runtime
	struct XRAppInfo
	{
		/// The name of the application using the OpenXR Provider library
		std::string AppName;

		/// The version of the application using the OpenXR Provider library
		uint32_t AppVersion;

		/// The name of the engine using the OpenXR Provider library
		std::string EngineName;

		/// The version of the engine using the OpenXR Provider library
		uint32_t EngineVersion;

		/// The application's tracking space - e.g. room scale, seated, etc
		EXRTrackingSpace TrackingSpace;

		/// List of extension objects representing each extension that the app wants to activate depending on runtime support
		std::vector< void * > XRExtensions;

		/// The absolute path and filename of the log file the OpenXR Provider library will write in to (optional)
		const char *LogFile;

		/// Struct Constructor
		/// @param[in]	appName						Name of the application using the OpenXR Provider library
		/// @param[in]	appVersion					Version of the application using the OpenXR Provider library
		/// @param[in]	engineName					Name of the engine using the OpenXR Provider library
		/// @param[in]	engineVersion				Version of the engine using the OpenXR Provider library
		/// @param[in]	trackingSpace				Application's tracking space - e.g. room scale, seated, etc
		/// @param[in]	xrExtensions				Array of supported extensions (objects) that app wants enabled if the runtime supports it
		/// @param[in]	pLogFile					(optional: null) Absolute path and filename for the logfile
		XRAppInfo(
			std::string appName,
			uint32_t appVersion,
			std::string engineName,
			uint32_t engineVersion,
			EXRTrackingSpace trackingSpace,
			std::vector< void * > xrExtensions,
			const char *pLogFile = nullptr )
			: AppName( appName )
			, AppVersion( appVersion )
			, EngineName( engineName )
			, EngineVersion( engineVersion )
			, TrackingSpace( trackingSpace )
			, XRExtensions( xrExtensions )
			, LogFile( pLogFile )
		{
		}
	};

	/// Information needed to create swapchain textures that the app will render to
	struct XRRenderInfo
	{
		/// Requested texture color formats  in order of preference. These are uint64_t nums that's defined by the graphics API (e.g. GL_RGBA16,
		/// DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc). Otherwise, a 0 in the array lets the runtime decide the format
		std::vector< int64_t > RequestTextureFormats;

		/// Requested texture depth formats  in order of preference. These are uint64_t nums that's defined by the graphics API (e.g. GL_DEPTH_COMPONENT,
		/// DXGI_FORMAT_D16_UNORM, etc). Otherwise, a 0 in the array lets the runtime decide the format
		std::vector< int64_t > RequestDepthFormats;

		/// The texture array size that will be generated by the OpenXR runtime for each swapchain image
		uint32_t TextureArraySize;

		/// The texture Mip count for each image in the swapchain
		uint32_t TextureMipCount;

		/// If the application plans to send depth textures along with the color textures
		bool EnableDepthTextureSupport;

		/// Struct Constructor
		/// @param[in]		vRequestTextureFormats		Requested texture color formats  in order of preference (e.g. GL_RGBA16,
		/// DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
		/// @param[in]		vRequestDepthFormats		Requested texture depth formats  in order of preference (e.g. GL_DEPTH_COMPONENT, DXGI_FORMAT_D16_UNORM,
		/// etc)
		/// @param[in]		nTextureArraySize			The texture array size that will be generated by the OpenXR runtime for each swapchain image
		/// @param[in]		nTextureMipCount			The texture Mip count for each image in the swapchain
		/// @param[in]		bEnableDepthTextureSupport	(optional: false) If depth composition (XR_KHR_composition_layer_depth) should be enabled if runtime
		/// supports it
		XRRenderInfo(
			std::vector< int64_t > vRequestTextureFormats,
			std::vector< int64_t > vRequestDepthFormats,
			uint32_t nTextureArraySize,
			uint32_t nTextureMipCount,
			bool bEnableDepthTextureSupport = false )
			: RequestTextureFormats( vRequestTextureFormats )
			, RequestDepthFormats( vRequestDepthFormats )
			, TextureArraySize( nTextureArraySize )
			, TextureMipCount( nTextureMipCount )
			, EnableDepthTextureSupport( bEnableDepthTextureSupport )
		{
		}
	};

	/// Eye State - contains pose, field of view info for an eye
	struct XREyeState
	{
		XrPosef Pose;
		XrFovf	FoV;
	};
	
	// HMD State - contains info for each eye such as fov, current pose and whether or not tracking is active
	struct XRHMDState
	{
		XREyeState LeftEye;
		XREyeState RightEye;
		bool IsPositionTracked;
		bool IsOrientationTracked;
	};

	//** CUSTOM TYPES */

	/// Event callback function used for registering functions to the Event Handler
	typedef void ( *Callback_XREvent )( XrEventDataBuffer );
	struct XRCallback
	{
		XrStructureType type;
		Callback_XREvent callback;
	};
}
