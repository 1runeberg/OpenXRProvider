 /* Copyrigh1t 2021 Rune Berg (GitHub: https://github.com/1runeberg, Twitter: https://twitter.com/1runeberg)
  * Based on DiligentEngine Tutorials code by Egor Yusov, DiligentGraphics LLC under Apache 2.0
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

#include "Sandbox.h"

std::unique_ptr< Sandbox > g_pTheApp;

/// Callback for session state changes TODO: Use native OpenXR type
/// @param[in] xrEventType	The type of event that triggered the callback (e.g. Session state change, reference space changed, etc)
/// @param[in] xrEventData	The data payload of the event (e.g. The new session state, etc)
static void Callback_XR_Event( XrEventDataBuffer xrEvent )
{
	// assert( pUtils && pUtils->GetLogger() );

	const XrEventDataSessionStateChanged &xrEventDataSessionStateChanged = *reinterpret_cast< XrEventDataSessionStateChanged * >( &xrEvent );
	const XrEventDataInteractionProfileChanged &xrEventDataInteractionProfileChanged = *reinterpret_cast< XrEventDataInteractionProfileChanged * >( &xrEvent );

	switch ( xrEvent.type )
	{
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			g_pTheApp->GetUtils()->GetLogger()->info(
				"Session State changing from {} to {}", OpenXRProvider::XrEnumToString( g_pTheApp->GetCurrentSessionState() ), OpenXRProvider::XrEnumToString( xrEventDataSessionStateChanged.state ) );

			g_pTheApp->SetCurrentSessionState( xrEventDataSessionStateChanged.state );

			break;

		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
			g_pTheApp->GetUtils()->GetLogger()->info(
				"Interaction Profile changed to Left: {}, Right: {} ",
				g_pTheApp->GetXRProvider()->Input()->GetCurrentInteractionProfile( "/user/hand/left" ),
				g_pTheApp->GetXRProvider()->Input()->GetCurrentInteractionProfile( "/user/hand/right" ) );
			break;

		default:
			break;
	}
}

LRESULT CALLBACK MessageProc( HWND, UINT, WPARAM, LPARAM );

// Main
int WINAPI WinMain( HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow )
{
#if defined( _DEBUG ) || defined( DEBUG )
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	g_pTheApp.reset( new Sandbox );

	const auto *cmdLine = GetCommandLineA();
	if ( !g_pTheApp->ProcessCommandLine( cmdLine ) )
		return -1;

	std::wstring Title( L"OpenXR Sandbox" );
	switch ( g_pTheApp->GetDeviceType() )
	{
		case RENDER_DEVICE_TYPE_D3D11:
			Title.append( L" (D3D11)" );
			break;
		case RENDER_DEVICE_TYPE_D3D12:
			Title.append( L" (D3D12)" );
			break;
		case RENDER_DEVICE_TYPE_GL:
			Title.append( L" (GL)" );
			break;
		case RENDER_DEVICE_TYPE_VULKAN:
			Title.append( L" (VK)" );
			break;
	}
	// Register our window class
	WNDCLASSEX wcex = { sizeof( WNDCLASSEX ), CS_HREDRAW | CS_VREDRAW, MessageProc, 0L, 0L, instance, NULL, NULL, NULL, NULL, L"Sandbox", NULL };
	RegisterClassEx( &wcex );

	// Create a window
	LONG WindowWidth = 1280;
	LONG WindowHeight = 1024;
	RECT rc = { 0, 0, WindowWidth, WindowHeight };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	HWND wnd = CreateWindow( L"Sandbox", Title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, NULL );
	if ( !wnd )
	{
		MessageBox( NULL, L"Cannot create window", L"Error", MB_OK | MB_ICONERROR );
		return 0;
	}
	ShowWindow( wnd, cmdShow );
	UpdateWindow( wnd );

	if ( !g_pTheApp->InitializeDiligentEngine( wnd ) )
		return -1;

	Timer timer;
	auto previousTime = timer.GetElapsedTime();

	//g_pTheApp->CreateResources();

	// Initialize OpenXR
	g_pTheApp->InitOpenXR();
	g_pTheApp->CreateXRSwapchain();

	// Prep render
	g_pTheApp->InitRenderPipeline();

	// Register callback
	OpenXRProvider::XRCallback xrCallback = { XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED }; // XR_TYPE_EVENT_DATA_BUFFER = Register for all events
	OpenXRProvider::XRCallback *pXRCallback = &xrCallback;
	pXRCallback->callback = Callback_XR_Event;
	g_pTheApp->GetXRProvider()->Core()->GetXREventHandler()->RegisterCallback( pXRCallback );

	// Main message loop
	MSG msg = { 0 };
	while ( WM_QUIT != msg.message )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			g_pTheApp->GetXRProvider()->Core()->PollXREvents();

			auto currentTime = timer.GetElapsedTime();
			auto elapsedTime = currentTime - previousTime;
			previousTime = currentTime;
			g_pTheApp->Update(currentTime, elapsedTime);

			g_pTheApp->Render();
			g_pTheApp->Present();
		}
	}

	g_pTheApp.reset();

	return ( int )msg.wParam;
}

// Called every time the NativeNativeAppBase receives a message
LRESULT CALLBACK MessageProc( HWND wnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint( wnd, &ps );
			EndPaint( wnd, &ps );
			return 0;
		}
		case WM_SIZE: // Window size has been changed
			if ( g_pTheApp )
			{
				g_pTheApp->WindowResize( LOWORD( lParam ), HIWORD( lParam ) );
			}
			return 0;

		case WM_CHAR:
			if ( wParam == VK_ESCAPE )
				PostQuitMessage( 0 );
			return 0;

		case WM_DESTROY:
			PostQuitMessage( 0 );
			return 0;

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = ( LPMINMAXINFO )lParam;

			lpMMI->ptMinTrackSize.x = 320;
			lpMMI->ptMinTrackSize.y = 240;
			return 0;
		}

		default:
			return DefWindowProc( wnd, message, wParam, lParam );
	}
}
