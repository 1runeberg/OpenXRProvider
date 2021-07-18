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

#include <main.h>

#define APP_PROJECT_NAME	"OpenXR Sandbox"
#define APP_PROJECT_VER		1
#define APP_ENGINE_NAME		"Diligent Engine"
#define APP_ENGINE_VER		2.5
#define APP_MIRROR_WIDTH	800
#define APP_MIRROR_HEIGHT	600

#define APP_LOG_TITLE		"Sandbox"
#define APP_LOG_FILE		L"\\logs\\openxr-provider-sandbox-log.txt"

int main()
{
    // Setup our sandbox application, which will be used for testing
	if ( AppSetup() != 0 )
		return -1;

}

int AppSetup()
{
	// Get executable directory
	wchar_t exePath[ MAX_PATH ];
	int nExePathLength = GetModuleFileNameW( NULL, exePath, MAX_PATH );

	if ( nExePathLength < 1 )
	{
		std::cout << "Fatal error! Unable to get current executable directory from operating system \n";
		return -1;
	}

	std::wstring::size_type nLastSlashIndex = std::wstring( exePath ).find_last_of( L"\\/" );
	sCurrentPath = std::wstring( exePath ).substr( 0, nLastSlashIndex );

	// Set the log file location for the OpenXR Library to use
	std::wcstombs( pAppLogFile, ( sCurrentPath + APP_LOG_FILE ).c_str(), MAX_PATH );

	// Setup helper utilities class
	pUtils = new Utils( APP_LOG_TITLE, pAppLogFile );

	return 0;
}