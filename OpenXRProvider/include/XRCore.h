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
#include <XREventHandler.h>
#include <rendering/XRGraphicsAwareTypes.h>
#include <extensions/XRExtHandTracking.h>
#include <extensions/XRExtHandJointsMotionRange.h>

#define LOG_TITLE	"OpenXR"

namespace OpenXRProvider
{
	/// Forward declaration on the XR Render Manager
	class XRRender;

	class XRCore
	{
	  public:
		// ** FUNCTIONS (PUBLIC) **/

		/// Class Constructor
		/// @param[in] xrAppInfo			App metadata used to create an instance and session to the active OpenXR runtime
		/// @param[in] xrAppGraphicsInfo	App graphics metadata based on current active api
		/// @param[in] bEnableDepth			Whether depth texture support should be enabled if the runtime supports it
		XRCore( XRAppInfo xrAppInfo, XRAppGraphicsInfo xrAppGraphicsInfo, bool bEnableDepth );

		/// Class Destructor
		~XRCore();

		/// Poll for OpenXR events so app can act on it (e.g. Session state changes, visibility mask changes, etc)
		void PollXREvents();

		/// Getter for the array of extensions names that are active in the current OpenXR session
		/// @return		Array of extensions names that are active in the current OpenXR session
		std::vector< const char * > GetEnabledExtensionNames() const { return m_vAppEnabledExtensions; }

		/// Getter for hand tracking extension object
		/// @return		Pointer to the hand tracking extension object
		XRExtHandTracking* GetExtHandTracking() const { return m_pXRHandTracking; }
		
		/// Getter for the graphics api object that handles graphics api (e.g. OpenGL, Vulkan, etc) specific rendering state and functions
		/// @return		Pointer to graphics api object that handles graphics api (e.g. OpenGL, Vulkan, etc) specific rendering state and functions	
		XRGraphicsAPI *GetGraphicsAPI() const { return m_pXRGraphicsAPI; }	

		/// Getter if depth textures are supported and enabled
		/// @return		If  depth textures are supported and runtime supports it
		bool GetIsDepthSupported() const { return m_bIsDepthSupported; }

		/// Getter for the logger object
		/// @return		Pointer to the logger object
		std::shared_ptr< spdlog::logger > GetLogger() const { return m_pLogger; }

		/// Getter for the Event Handler object that broadcasts events to listeners via their registered callback functions
		/// @return		Pointer to the event handler object
		XREventHandler *GetXREventHandler() const { return m_pXREventHandler; }

		/// Getter for the enabled OpenXR extension objects
		/// @return		Array of enabled OpenXR extensions
		std::vector< void * > GetXREnabledExtensions() const { return m_vXRAppEnabledExtensions; } 

		/// Getter for the active OpenXR instance
		/// @return		Active OpenXR instance
		XrInstance GetXRInstance() const { return m_xrInstance; } 

		/// Getter for the active OpenXR session
		/// @return		The active OpenXR session
		XrSession GetXRSession() const { return m_xrSession; }

		/// Getter for the current app reference space
		/// @return		The current app reference space
		XrSpace GetXRSpace() const { return m_xrSpace; }

		/// Getter for the current OpenXR System Id
		/// @return		The current OpenXR System Id of the active OpenXR runtime
		XrSystemId GetXRSystemId() const { return m_xrSystemId; }			

		/// Getter for the current OpenXR system properties
		/// @return		The current OpenXR System Properties of the active OpenXR runtime
		XrSystemProperties GetXRSystemProperties() const { return m_xrSystemProperties; } 

		/// Setter for the currently active OpenXR session
		/// @param[in]	xrSession	Set the active OpenXR session
		void SetXRSession( XrSession xrSession ) { m_xrSession = xrSession; }

		/// Call OpenXR xrBeginSession to start app frame synchronization
		/// @return	XrResult	The result of the xrEndSession call
		XrResult XRBeginSession();

		/// Call OpenXR xrEndSession to allow runtime to safely transition back to idle
		/// @return	XrResult	The result of the xrEndSession call
		XrResult XREndSession();

	  private:
		// ** FUNCTIONS (PRIVATE) **/

		/// Initialize OpenXR - establish a connection to an active runtime.
		/// (1) Set application general info (e.g. App name, version, engine name, etc)
		/// (2) Check runtime's supported extensions and tag ones selected by the app to enable (e.g. Graphics type, Visibility Mask, Handtracking, etc),
		/// (3) Create an instance (object that allows communication to the runtime) and
		/// (4) Get xr system (representing a collection of physical xr devices) from active OpenXR runtime
		void OpenXRInit( bool bEnableDepthTextureSupport = false );

		/// Enable provided extensions if supported by the currently active OpenXR runtime
		/// @param[in]	pXRExtensions	List of extensions names to enable if the active OpenXR runtime supports it
		void EnableInstanceExtensions( std::vector< const char * > &pXRExtensions, bool bEnableDepthTextureSupport );

		/// Execute all registered callback functions. Used after an OpenXR poll and an event state has been triggered by the runtime
		/// @param[in]	xrEventType	The type of OpenXR event that triggered the callback
		/// @param[in]	eXREventData
		void ExecuteCallbacks( XrEventDataBuffer xrEvent );

		/// Load the current OpenXR runtime and retrieve its system properties
		/// @return		Result of the OpenXR calls to retrieve the xr system properties from the active OpenXR runtime
		XrResult LoadXRSystem();

		/// Initialize the world - setup the space and time the app will live in
		/// (1) Create a session (app communication instance to render frames or send/receive input to/from the runtime) and
		/// (2) Create a reference space (room-scale vs seated)
		/// @param[in]	pXRAppGraphicsInfo	Pointer to the application's graphics api (e.g. OpenGL, DirectX, Vulkan) dependent graphics info 
		void WorldInit( XRAppGraphicsInfo *pXRAppGraphicsInfo );

	  private:
		// ** MEMBER VARIABLES (PRIVATE) **/

		///  If depth textures are supported
		bool m_bIsDepthSupported = false;

		/// Version of the application using this library
		uint32_t m_nAppVersion;

		/// Version of the engine using this library
		uint32_t m_nEngineVersion;

		/// Name of the application using this library
		std::string m_sAppName;

		/// Name of the engine using this library
		std::string m_sEngineName;

		/// List of OpenXR extension names that are activated and supported by the currently active OpenXR runtime
		std::vector< const char * > m_vAppEnabledExtensions;

		/// Array of OpenXR extension object that the app requested to be activated - may or may not be activated depending on active OpenXR support
		std::vector< void * > m_vAppRequestedExtensions;

		/// Array of OpenXR enabled extension objects that are activated and supported by the currently active OpenXR runtime
		std::vector< void * > m_vXRAppEnabledExtensions;

		/// Pointer to the logger
		std::shared_ptr< spdlog::logger > m_pLogger;

		/// Pointer to the active Event Handler that broadcasts events to listeners via their registered callback functions
		XREventHandler *m_pXREventHandler = nullptr;

		/// Pointer to the graphics api object that handles graphics api (e.g. OpenGL, Vulkan, etc) specific rendering state and functions
		XRGraphicsAPI *m_pXRGraphicsAPI = nullptr;

		/// The current OpenXR instance
		XrInstance m_xrInstance = XR_NULL_HANDLE;

		/// Results of the last call to the OpenXR api
		XrResult m_xrLastCallResult = XR_SUCCESS;

		/// Type of reference space currently is use (roomscale, seated)
		XrReferenceSpaceType m_xrReferenceSpaceType = { XR_REFERENCE_SPACE_TYPE_STAGE };

		/// The active OpenXR Session
		XrSession m_xrSession = XR_NULL_HANDLE;		

		/// Current session state
		XrSessionState m_xrSessionState = { XR_SESSION_STATE_UNKNOWN }; 

		/// The application space which represents the user's play space
		XrSpace m_xrSpace = XR_NULL_HANDLE;		

		/// The active OpenXR runtime's system id
		XrSystemId m_xrSystemId = XR_NULL_SYSTEM_ID;	

		/// The active OpenXR runtime's system properties
		XrSystemProperties m_xrSystemProperties = { XR_TYPE_SYSTEM_PROPERTIES };

		/// The headset (HMD) view space
		XrSpace m_xrViewSpace = XR_NULL_HANDLE;

		/// The current predicted display time
		XrTime m_xrPredictedDisplayTime = { 0 }; // The current display time, use this as a single point of truth for current frame time

		
		// ** EXTENSIONS **/
		XRExtHandTracking* m_pXRHandTracking = nullptr;
		
	};
}
