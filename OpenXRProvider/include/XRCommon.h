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

// Logger includes
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Provider classes
#include <XRBase.h>
#include <XRBaseExt.h>

// Platform and graphic apis
#include <XRGraphicsAPIDefines.h>
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

	/// Execute a call to the OpenXR api and evaluate the result, will log success and fail and throw an exception if request
	#define XR_CALL( xrCall, xrLogger, bThrow ) CheckXrResult( xrCall, xrLogger, bThrow, #xrCall, __FILE__, __LINE__ );
	inline XrResult CheckXrResult(
		XrResult xrResult,
		std::shared_ptr< spdlog::logger > xrLogger,
		bool bThrow,
		const char *xrCall = nullptr,
		const char *srcFile = nullptr,
		int srcLineNum = 0 )
	{
		if ( XR_UNQUALIFIED_SUCCESS( xrCall ) )
		{
			const char *xrEnumStr = XrEnumToString( xrResult );
			std::string eMessage = "Error on OpenXR call ";
			if ( xrCall )
				eMessage.append( xrCall );

			xrLogger->error( "{}. Error ({}) {} in file {} line {}", eMessage, std::to_string( xrResult ), xrEnumStr, srcFile, srcLineNum );

			if ( bThrow )
				throw std::runtime_error( eMessage );
		}
		else
		{
			xrLogger->info( "Success: {}", xrCall );
		}

		return xrResult;
	}

	/// Execute a call to the OpenXR api and evaluate the result, will log failures only
	#define XR_CALL_SILENT( xrCall, xrLogger ) CheckXrResult( xrCall, xrLogger, #xrCall, __FILE__, __LINE__ );
	inline XrResult CheckXrResult(
		XrResult xrResult,
		std::shared_ptr< spdlog::logger > xrLogger,
		const char *xrCall = nullptr,
		const char *srcFile = nullptr,
		int srcLineNum = 0 )
	{
		if ( XR_UNQUALIFIED_SUCCESS( xrCall ) )
		{
			const char *xrEnumStr = XrEnumToString( xrResult );
			std::string eMessage = "Error on OpenXR call ";
			if ( xrCall )
				eMessage.append( xrCall );

			xrLogger->error( "{}. Error ({}) {} in file {} line {}", eMessage, std::to_string( xrResult ), xrEnumStr, srcFile, srcLineNum );
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
