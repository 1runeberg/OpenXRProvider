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

#include <map>
#include <SandboxCommon.h>

// Windowing includes
#include <third_party/sdl/include/SDL.h>
#include <rendering/XRRender.h>


class IXRMirror
{
  public:
  	/// Class Constructor
	/// @param[in] pCommon		Pointer to the sandbox common object
	/// @param[in] nWidth		The width of the desktop window (XR Mirror)
	/// @param[in] nHeight		The height of the desktop window (XR Mirror)
	/// @param[in] pTitle		The title text that will be displayed on the console window
	/// @param[in] sCurrentPath	The absolute path to where the application executable is
	/// @param[in] sLogFile		The absolute path and filename of the log file
	IXRMirror( SandboxCommon *pCommon, int nWidth, int nHeight, const char *pTitle, std::wstring sCurrentPath, const char *sLogFile )
	: m_pCommon( pCommon )
	, m_nScreenWidth( nWidth )
	, m_nScreenHeight( nHeight )
	, m_sCurrentPath( sCurrentPath ) {}

	/// Class Destructor
	~IXRMirror() {}

	// ** INTERFACE FUNCTIONS **//

	/// Setup the graphics api objects needed for rendering to this xr mirror
	/// @param[in] pRender		Pointer to the OpenXRProvider Library Render Manager
	virtual const int Init( OpenXRProvider::XRRender *pRender ) = 0;

	/// Blit (copy) swapchain image that's rendered to the HMD to the desktop screen (XR Mirror)
	virtual void BlitToWindow() = 0;

	/// Clear texture
	/// @param[in]	v4ClearColor	Color (r,g,b,a)
	virtual void Clear( glm::vec4 v4ClearColor ) = 0;

	/// Render to the swapchain image (texture) that'll be rendered to the user's HMD and blitted (copied) to
	/// the Sandbox's window (XR Mirror)
	/// @param[in]	eCurrentScene		The current sandbox scene/level to be rendered
	/// @param[in] eEye					The eye (left/right) texture that will be rendered on
	/// @param[in] nSwapchainIndex		The index of the swapchain image (texture)
	virtual void DrawFrame( SandboxCommon::ESandboxScene eCurrentScene, OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex ) = 0;


	// ** ACCESSORS **//

	/// Getter for the screen width
	/// @return		Screen width in pixels
	int ScreenWidth() const { return m_nScreenWidth; }

	/// Setter for the screen width
	/// @param[in]	val		New screen width in pixels
	void ScreenWidth( int val ) { m_nScreenWidth = val; }

	/// Getter for the screen height
	/// @return		Screen height in pixels
	int ScreenHeight() const { return m_nScreenHeight; }

	/// Setter for the screen height
	/// @param[in]	val		New screen height in pixels
	void ScreenHeight( int val ) { m_nScreenHeight = val; }

	/// Getter for the absolute path to the app executable
	/// @return		Absolute path to the app executable
	std::wstring CurrentPath() const { return m_sCurrentPath; }

	/// Setter for the absolute path to the app executable
	/// @param[in]	val		New absolute path to the app executable
	void CurrentPath( std::wstring val ) { m_sCurrentPath = val; }

	/// Getter for the clear color to use in the graphics api Clear() calls
	/// @return		Color to use in the graphics api Clear() calls
	glm::vec4 ClearColor() const { return m_v4ClearColor; }

	/// Setter for the clear color to use in the graphics api Clear() calls
	/// @param[in]	val		New color to use in the graphics api Clear() calls
	void ClearColor( glm::vec4 val ) { m_v4ClearColor = val; }

	/// Getter for the Logger
	/// @return		Pointer to the logger
	std::shared_ptr< spdlog::logger > Logger() const { return m_pLogger; }

	/// Setter for the Logger
	/// @param[in]	val		Pointer to the new logger
	void Logger( std::shared_ptr< spdlog::logger > val ) { m_pLogger = val; }

	/// Getter for the Sandbox Common object
	/// @return		Pointer to the Sandbox Common object
	SandboxCommon *Common() const { return m_pCommon; }

	/// Setter for the Sandbox Common Object
	/// @param[in]	val		Pointer to the Sandbox Common Object
	void Common( SandboxCommon *val ) { m_pCommon = val; }

	/// Getter for the SDL Window
	/// @return		Pointer to the SDL Window
	SDL_Window *SDLWindow() const { return m_pSDLWindow; }

	/// Setter for the SDL window
	/// @param[in]	val		Pointer to the new sdl window
	void SDLWindow( SDL_Window *val ) { m_pSDLWindow = val; }

	/// Getter for the OpenXR Provider XR Render Manager
	/// @return		Pointer to the OpenXR Provider XR Render Manager
	OpenXRProvider::XRRender *Render() const { return m_pXRRender; }

	/// Setter for the OpenXR Provider XR Render Manager
	/// @param[in]	val		Pointer to the OpenXR Provider XR Render Manager
	void Render( OpenXRProvider::XRRender *val ) { m_pXRRender = val; }


  private:
	// ** MEMBER VARIABLES (PRIVATE) **/

	/// The width of the desktop window 
	int m_nScreenWidth = 1920;

	/// The height of the desktop window 
	int m_nScreenHeight = 1080;

	/// The absolute path to the app executable
	std::wstring m_sCurrentPath;

	/// Clear color
	glm::vec4 m_v4ClearColor { 0.5f, 0.9f, 1.0f, 1.0f };

	/// The logger object
	std::shared_ptr< spdlog::logger > m_pLogger;

	/// Pointer to the helper utilities log (logger lives here)
	SandboxCommon *m_pCommon;

	/// Pointer to the SDL desktop window (XR Mirror)
	SDL_Window *m_pSDLWindow;

	/// OpenXR Render Manager
	OpenXRProvider::XRRender *m_pXRRender;

};
