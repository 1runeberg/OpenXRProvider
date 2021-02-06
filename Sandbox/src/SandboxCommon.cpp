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

#include "SandboxCommon.h"

SandboxCommon::SandboxCommon( const char *pLogTitle, const char *pLogFile )
	: m_pLogTitle( pLogTitle )
{
	// Initialize logger
	std::vector< spdlog::sink_ptr > vLogSinks;
	vLogSinks.push_back( std::make_shared< spdlog::sinks::stdout_color_sink_st >() );
	vLogSinks.push_back( std::make_shared< spdlog::sinks::basic_file_sink_st >( pLogFile ) );
	m_pLogger = std::make_shared< spdlog::logger >( pLogTitle, begin( vLogSinks ), end( vLogSinks ) );

	m_pLogger->set_level( spdlog::level::trace );
	m_pLogger->set_pattern( "[%Y-%b-%d %a] [%T %z] [%^%L%$] [%n] %v" );
	m_pLogger->info( "G'day from {}! Logging to: {}", pLogTitle, pLogFile );
}

SandboxCommon::~SandboxCommon() 
{
	// Delete logger
	if ( m_pLogger )
		spdlog::drop( m_pLogTitle );
}

glm::mat4 SandboxCommon::GetEyeProjection( XrFovf eyeFoV, float fNear, float fFar )
{
	// Convert angles and fovs
	float fovLeft = std::tanf( eyeFoV.angleLeft );
	float fovRight = std::tanf( eyeFoV.angleRight );
	float fovUp = std::tanf( eyeFoV.angleUp );
	float fovDown = std::tanf( eyeFoV.angleDown );

	float fovWidth = fovRight - fovLeft;
	float fovHeight = fovUp - fovDown;

	float fOffset = fNear;
	float fLength = fFar - fNear;

	// Calculate eye projection matrix
	glm::mat4 mProjection = glm::mat4( 1.0f );

	mProjection[ 0 ][ 0 ] = 2.0f / fovWidth;
	mProjection[ 0 ][ 1 ] = 0.0f;
	mProjection[ 0 ][ 2 ] = 0.0f;
	mProjection[ 0 ][ 3 ] = 0.0f;

	mProjection[ 1 ][ 0 ] = 0.0f;
	mProjection[ 1 ][ 1 ] = 2.0f / fovHeight;
	mProjection[ 1 ][ 2 ] = 0.0f;
	mProjection[ 1 ][ 3 ] = 0.0f;

	mProjection[ 2 ][ 0 ] = ( fovRight + fovLeft ) / fovWidth;
	mProjection[ 2 ][ 1 ] = ( fovUp + fovDown ) / fovHeight;
	mProjection[ 2 ][ 2 ] = -( fFar + fOffset ) / fLength;
	mProjection[ 2 ][ 3 ] = -1.0f;

	mProjection[ 3 ][ 0 ] = 0.0f;
	mProjection[ 3 ][ 1 ] = 0.0f;
	mProjection[ 3 ][ 2 ] = -( fFar * ( fNear + fOffset ) ) / fLength;
	mProjection[ 3 ][ 3 ] = 0.0f;

	return mProjection;
}

glm::mat4 SandboxCommon::InvertMatrix( const glm::mat4 mMatrix )
{
	glm::mat4 mInvertedMatrix( 1.0f );

	mInvertedMatrix[ 0 ][ 0 ] = mMatrix[ 0 ][ 0 ];
	mInvertedMatrix[ 0 ][ 1 ] = mMatrix[ 1 ][ 0 ];
	mInvertedMatrix[ 0 ][ 2 ] = mMatrix[ 2 ][ 0 ];
	mInvertedMatrix[ 0 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 1 ][ 0 ] = mMatrix[ 0 ][ 1 ];
	mInvertedMatrix[ 1 ][ 1 ] = mMatrix[ 1 ][ 1 ];
	mInvertedMatrix[ 1 ][ 2 ] = mMatrix[ 2 ][ 1 ];
	mInvertedMatrix[ 1 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 2 ][ 0 ] = mMatrix[ 0 ][ 2 ];
	mInvertedMatrix[ 2 ][ 1 ] = mMatrix[ 1 ][ 2 ];
	mInvertedMatrix[ 2 ][ 2 ] = mMatrix[ 2 ][ 2 ];
	mInvertedMatrix[ 2 ][ 3 ] = 0.0f;

	mInvertedMatrix[ 3 ][ 0 ] = -( mMatrix[ 0 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 0 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 0 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 1 ] = -( mMatrix[ 1 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 1 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 1 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 2 ] = -( mMatrix[ 2 ][ 0 ] * mMatrix[ 3 ][ 0 ] + mMatrix[ 2 ][ 1 ] * mMatrix[ 3 ][ 1 ] + mMatrix[ 2 ][ 2 ] * mMatrix[ 3 ][ 2 ] );
	mInvertedMatrix[ 3 ][ 3 ] = 1.0f;

	return mInvertedMatrix;
}
