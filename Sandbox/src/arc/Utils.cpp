/* Copyright 2021 Rune Berg (GitHub: https://github.com/1runeberg, Twitter: https://twitter.com/1runeberg)
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *	 https://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include "Utils.h"

Utils::Utils( const char *pLogTitle, const char *pLogFile )
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

Utils::~Utils() {}
