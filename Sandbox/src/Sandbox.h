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

#include <OpenXRProvider.h>
#include <Utils.h>

#include <iomanip>
#include <iostream>
#include <memory>

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <Windows.h>
#include <crtdbg.h>

#ifndef PLATFORM_WIN32
	#define PLATFORM_WIN32 1
#endif

#ifndef ENGINE_DLL
	#define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
	#define D3D11_SUPPORTED 1
#endif

#ifndef D3D12_SUPPORTED
	#define D3D12_SUPPORTED 1
#endif

#ifndef GL_SUPPORTED
	#define GL_SUPPORTED 1
#endif

#ifndef VULKAN_SUPPORTED
	#define VULKAN_SUPPORTED 1
#endif


#include "DiligentEngine/DiligentCore/Primitives/interface/BasicTypes.h"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

//#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D11/interface/RenderDeviceD3D11.h"
//#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineD3D12/interface/RenderDeviceD3D12.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngineOpenGL/interface/RenderDeviceGL.h"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h"

#include "DiligentEngine/DiligentCore/Common/interface/BasicMath.hpp"
#include "DiligentEngine/DiligentCore/Common/interface/Timer.hpp"
#include "DiligentEngine/DiligentCore/Common/interface/RefCntAutoPtr.hpp"

#include "DiligentEngine/DiligentCore/Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include "DiligentEngine/DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "DiligentEngine/DiligentTools/TextureLoader/interface/TextureUtilities.h"

using namespace Diligent;

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.

static const char *VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char *PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

class Sandbox
{
	/// Sandbox scenes
	enum ESandboxScene
	{
		SANDBOX_SCENE_SEA_OF_CUBES = 0,
		SANDBOX_SCENE_HAND_TRACKING = 1
	};


public:
	Sandbox();
	~Sandbox();
	
	bool InitializeDiligentEngine(HWND hWnd);
	void CreateResources();

	int AppSetup();
	bool InitOpenXR();			
	void CreateXRSwapchain();		// Create diligent engine compatible swapchain from OpenXR swapchain

	void InitRenderPipeline();
	void Render();
	void Present();

	void Update(double CurrTime, double ElapsedTime);

	void WindowResize(Uint32 Width, Uint32 Height);
	bool ProcessCommandLine(const char* CmdLine);

	RENDER_DEVICE_TYPE GetDeviceType() const { return m_DeviceType; }
	Utils* GetUtils() const { return pUtils; }
	XrSessionState GetCurrentSessionState() const { return xrCurrentSessionState; }
	void SetCurrentSessionState(XrSessionState xrSessionState) { xrCurrentSessionState = xrSessionState;  }
	OpenXRProvider::XRProvider* GetXRProvider() const { return pXRProvider; }

  private:
	RefCntAutoPtr< IRenderDevice > m_pDevice;
	RefCntAutoPtr< IDeviceContext > m_pImmediateContext;
	RefCntAutoPtr< ISwapChain > m_pSwapChain;
	RefCntAutoPtr< IPipelineState > m_pPSO;
	RefCntAutoPtr<IEngineFactory> m_pEngineFactory;

	RefCntAutoPtr< IBuffer > m_CubeVertexBuffer;
	RefCntAutoPtr< IBuffer > m_CubeIndexBuffer;
	RefCntAutoPtr< IBuffer > m_VSConstants;
	RefCntAutoPtr< ITextureView > m_TextureSRV;
	RefCntAutoPtr< IShaderResourceBinding > m_SRB;

	RENDER_DEVICE_TYPE m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
	float4x4 m_WorldViewProjMatrix;

	std::vector< RefCntAutoPtr< ITextureView > > m_xrSwapchainColor[ 2 ];
	std::vector< RefCntAutoPtr< ITextureView > > m_xrSwapchainDepth[ 2 ];

	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void LoadTexture();

	void CreateTextureViews(OpenXRProvider::EXREye eEye, uint32_t nXRSwapchainSize, XrSwapchain xrSwapchain, bool bIsDepth);

	void DrawFrame(OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex);
	void DrawScene_SeaOfCubes(OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex);

	float4x4 GetEyeProjection(XrFovf eyeFoV, float fNear, float fFar);

	// ** MEMBER VARIABLES **//
	double timeCurrent, timeElpased;

	/// If hand joints should be rendered (hand tracking extension required)
	bool bDrawHandJoints = false;

	/// Current active scene
	ESandboxScene eCurrentScene = SANDBOX_SCENE_SEA_OF_CUBES;

	/// Instanced cube data for its projection matrices
	unsigned int cubeInstanceDataVBO;

	/// Instanced controller mesh data for its projection matrices
	unsigned int controllerInstanceDataVBO;

	/// Instanced joint mesh data for its projection matrices
	unsigned int jointInstanceDataVBO;

	/// The width of the desktop window (XR Mirror)
	int nScreenWidth = 1920;

	/// The height of the desktop window (XR Mirror)
	int nScreenHeight = 1080;

	/// The number of images (Texture2D) that are in the swapchain created by the OpenXR runtime
	uint32_t nSwapchainCapacity = 0;

	/// The current index of the swapchain array
	uint32_t nSwapchainIndex = 0;

	/// The current frame number (internal to the sandbox application)
	uint64_t nFrameNumber = 1;

	/// The shader program ID for visibility mask that will be applied on all textures
	GLuint nShaderVisMask;

	/// The shader program ID for lit objects
	GLuint nShaderLit;

	/// The shader program ID for unlit
	GLuint nShaderUnlit;

	/// The shader program ID for textured
	GLuint nShaderTextured;

	/// The absolute path to the built Sandbox executable
	std::wstring sCurrentPath;

	/// The path and filename to write the OpenXR Provider library file to
	char pAppLogFile[ MAX_PATH ] = "";

	/// Sea of Cubes textures
	std::vector< unsigned int > vCubeTextures;

	/// Indices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< uint32_t > vMaskIndices_L;

	/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< uint32_t > vMaskIndices_R;

	/// Vertices for the left eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< float > vMaskVertices_L;

	/// Indices for the right eye visibility mask (hmd specific occlusion mesh reported by the active OpenXR runtime)
	std::vector< float > vMaskVertices_R;

	/// Pointer to the Utilities (Utils) class instantiated and used by the sandbox app (logging utility lives here)
	Utils *pUtils = nullptr;

	/// Stores the current OpenXR session state
	XrSessionState xrCurrentSessionState = XR_SESSION_STATE_UNKNOWN;

	/// Pointer to the XRProvider class of the OpenXR Provider library which handles all state, system and generic calls to the OpenXR runtime
	OpenXRProvider::XRProvider *pXRProvider = nullptr;

	/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
	OpenXRProvider::XRExtVisibilityMask *pXRVisibilityMask = nullptr;

	/// Pointer to the XRExtVisibilityMask class of the OpenXR Provider library which handles the OpenXR visibility mask extension for runtimes that support it
	OpenXRProvider::XRExtHandTracking *pXRHandTracking = nullptr;

	/// Pointer to the XrExtHandJointsMotionRange class of the OpenXR Provider library which handles specifying motion ranges for the hand joints for runtimes that support it
	OpenXRProvider::XRExtHandJointsMotionRange *pXRHandJointsMotionRange = nullptr;

	/// -------------------------------
	/// INPUTS
	/// -------------------------------

	/// Action set to use for this sandbox
	XrActionSet xrActionSet_Main;

	// Action states
	XrActionStateBoolean xrActionState_SwitchScene;
	XrActionStatePose xrActionState_PoseLeft, xrActionState_PoseRight;

	// Actions
	XrAction xrAction_SwitchScene, xrAction_Haptic;
	XrAction xrAction_PoseLeft, xrAction_PoseRight;

	// Location and velocities for the controllers
	XrSpaceLocation xrLocation_Left { XR_TYPE_SPACE_LOCATION };
	XrSpaceLocation xrLocation_Right { XR_TYPE_SPACE_LOCATION };

	XrSpaceVelocity xrVelocity_Left { XR_TYPE_SPACE_VELOCITY };
	XrSpaceVelocity xrVelocity_Right { XR_TYPE_SPACE_VELOCITY };
};
