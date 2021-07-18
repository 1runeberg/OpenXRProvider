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

#define _CRT_SECURE_NO_WARNINGS 1

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX_STRING_LEN 512

// Logger includes
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Utils
{
  public:
	// ** FUNCTIONS (PUBLIC) **/

	/// Class Constructor
	/// @param[in] pLogTitle	The title text that will be displayed on the logs and console window
	/// @param[in] pLogFile		The absolute path and filename of the log file
	Utils( const char *pLogTitle, const char *pLogFile );

	/// Class Destructor
	~Utils();

	// Getter for the logger
	/// @return									The shared pointer to the logger object
	std::shared_ptr< spdlog::logger > GetLogger() { return m_pLogger; }

  private:
	// ** FUNCTIONS (PRIVATE) **/

	// ** MEMBER VARIABLES (PRIVATE) **/

	/// The logger object
	std::shared_ptr< spdlog::logger > m_pLogger;
};
