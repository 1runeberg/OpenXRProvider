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

#include <XRMirror_DX11.h>

XRMirror_DX11::XRMirror_DX11( SandboxCommon *pCommon, int nWidth, int nHeight, const char *pTitle, std::wstring sCurrentPath, const char *sLogFile )
	: IXRMirror( pCommon, nWidth, nHeight, pTitle, sCurrentPath, sLogFile )
{	
	// Initialize logger
	std::vector< spdlog::sink_ptr > vLogSinks;
	vLogSinks.push_back( std::make_shared< spdlog::sinks::stdout_color_sink_st >() );
	vLogSinks.push_back( std::make_shared< spdlog::sinks::basic_file_sink_st >( sLogFile ) );
	Logger(std::make_shared< spdlog::logger >( "XRMirror - DX11", begin( vLogSinks ), end( vLogSinks ) ));

	Logger()->set_level( spdlog::level::trace );
	Logger()->set_pattern( "[%Y-%b-%d %a] [%T %z] [%^%L%$] [%n] %v" );
	Logger()->info( "G'day from {}! Logging to: {}", "XRMirror - DX11", sLogFile );

	// Initialize sdl
	SDL_Init( SDL_INIT_VIDEO );

	// Create sdl window
	SDLWindow(SDL_CreateWindow( pTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nWidth, nHeight, 0 ));
	if ( !SDLWindow() )
	{
		std::string eMessage = "Failed to create SDL window";
		Logger()->error( "{}", eMessage );

		throw eMessage;
	}

	// Create mirror
	Logger()->info( "Mirror created {}x{}", nWidth, nHeight );
}

XRMirror_DX11::~XRMirror_DX11()
{
	// Delete logger
	if ( Logger() )
		spdlog::drop( "XRMirror - DX11" );

	// Destroy window
	if ( SDLWindow() )
		SDL_DestroyWindow( SDLWindow() );

	SDL_Quit();
}

const int XRMirror_DX11::Init( OpenXRProvider::XRRender *pRender )
{
	assert( pRender );
	Render( pRender );

	return 0;
}

void XRMirror_DX11::BlitToWindow()
{

}

void XRMirror_DX11::Clear( glm::vec4 v4ClearColor ) 
{

}

void XRMirror_DX11::DrawFrame( SandboxCommon::ESandboxScene eCurrentScene, OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex )
{

}

