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

#include "Sandbox.h"
#include "../libtiff/tiffiop.h"

#define APP_PROJECT_NAME "OpenXR Sandbox"
#define APP_PROJECT_VER 1
#define APP_ENGINE_NAME "Diligent Engine"
#define APP_ENGINE_VER DILIGENT_API_VERSION
#define APP_MIRROR_WIDTH 800
#define APP_MIRROR_HEIGHT 600

#define APP_LOG_TITLE "Sandbox"
#define APP_LOG_FILE L"\\logs\\openxr-provider-sandbox-log.txt"

Sandbox::Sandbox() 
 {
	AppSetup();
 }

Sandbox::~Sandbox() 
{
	delete pUtils;
}

bool Sandbox::InitializeDiligentEngine( HWND hWnd )
{
	SwapChainDesc SCDesc;
	switch ( m_DeviceType )
	{
#if D3D11_SUPPORTED
		case RENDER_DEVICE_TYPE_D3D11:
		{
			EngineD3D11CreateInfo EngineCI;
	#if ENGINE_DLL
			// Load the dll and import GetEngineFactoryD3D11() function
			auto *GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
	#endif
			auto *pFactoryD3D11 = GetEngineFactoryD3D11();
			pFactoryD3D11->CreateDeviceAndContextsD3D11( EngineCI, &m_pDevice, &m_pImmediateContext );
			Win32NativeWindow Window { hWnd };
			pFactoryD3D11->CreateSwapChainD3D11( m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc {}, Window, &m_pSwapChain );

			m_pEngineFactory = pFactoryD3D11;
		}
		break;
#endif

#if D3D12_SUPPORTED
		case RENDER_DEVICE_TYPE_D3D12:
		{
	#if ENGINE_DLL
			// Load the dll and import GetEngineFactoryD3D12() function
			auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
	#endif
			EngineD3D12CreateInfo EngineCI;

			auto *pFactoryD3D12 = GetEngineFactoryD3D12();
			pFactoryD3D12->CreateDeviceAndContextsD3D12( EngineCI, &m_pDevice, &m_pImmediateContext );
			Win32NativeWindow Window { hWnd };
			pFactoryD3D12->CreateSwapChainD3D12( m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc {}, Window, &m_pSwapChain );

			m_pEngineFactory = pFactoryD3D12;
		}
		break;
#endif

#if GL_SUPPORTED
		case RENDER_DEVICE_TYPE_GL:
		{
	#if EXPLICITLY_LOAD_ENGINE_GL_DLL
			// Load the dll and import GetEngineFactoryOpenGL() function
			auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
	#endif
			auto *pFactoryOpenGL = GetEngineFactoryOpenGL();

			EngineGLCreateInfo EngineCI;
			EngineCI.Window.hWnd = hWnd;

			pFactoryOpenGL->CreateDeviceAndSwapChainGL( EngineCI, &m_pDevice, &m_pImmediateContext, SCDesc, &m_pSwapChain );

			m_pEngineFactory = pFactoryOpenGL;
		}
		break;
#endif

#if VULKAN_SUPPORTED
		case RENDER_DEVICE_TYPE_VULKAN:
		{
	#if EXPLICITLY_LOAD_ENGINE_VK_DLL
			// Load the dll and import GetEngineFactoryVk() function
			auto GetEngineFactoryVk = LoadGraphicsEngineVk();
	#endif
			EngineVkCreateInfo EngineCI;

			auto *pFactoryVk = GetEngineFactoryVk();
			pFactoryVk->CreateDeviceAndContextsVk( EngineCI, &m_pDevice, &m_pImmediateContext );

			if ( !m_pSwapChain && hWnd != nullptr )
			{
				Win32NativeWindow Window { hWnd };
				pFactoryVk->CreateSwapChainVk( m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain );

				m_pEngineFactory = pFactoryVk;
			}
		}
		break;
#endif

		default:
			std::cerr << "Unknown/unsupported device type";
			return false;
			break;
	}

	return true;
}

void Sandbox::CreateResources()
{
	// Pipeline state object encompasses configuration of all GPU stages

	GraphicsPipelineStateCreateInfo PSOCreateInfo;

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSOCreateInfo.PSODesc.Name = "OpenXR Sandbox PSO";

	// This is a graphics pipeline
	PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

	// clang-format off
	// This tutorial will render to a single render target
	PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
	// Use the depth buffer format from the swap chain
	PSOCreateInfo.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// No back face culling for this tutorial
	PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
	// Disable depth testing
	PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
	// clang-format on

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;
	// Create a vertex shader
	RefCntAutoPtr< IShader > pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Triangle vertex shader";
		ShaderCI.Source = VSSource;
		m_pDevice->CreateShader( ShaderCI, &pVS );
	}

	// Create a pixel shader
	RefCntAutoPtr< IShader > pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Triangle pixel shader";
		ShaderCI.Source = PSSource;
		m_pDevice->CreateShader( ShaderCI, &pPS );
	}

	// Finally, create the pipeline state
	PSOCreateInfo.pVS = pVS;
	PSOCreateInfo.pPS = pPS;
	m_pDevice->CreateGraphicsPipelineState( PSOCreateInfo, &m_pPSO );
}

int Sandbox::AppSetup()
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

bool Sandbox::InitOpenXR()
{
	// SETUP OPENXR PROVIDER
	// OpenXR Provider is a wrapper library to the official OpenXR loader.
	//
	// This library provides classes that when instantiated, exposes to an app simpler access
	// to any currently running OpenXR runtime (e.g SteamVR, OculusVR, etc) by abstracting away most
	// of the necessary OpenXR calls and their strict call sequence

	// (1) Optional: Create Extension class(es) for the OpenXR extensions the app wants to activate.
	//     There is no need to specify the graphics api extension as it will be automatically activated

	pXRVisibilityMask = new OpenXRProvider::XRExtVisibilityMask( pUtils->GetLogger() );
	pXRHandTracking = new OpenXRProvider::XRExtHandTracking( pUtils->GetLogger() );
	pXRHandJointsMotionRange = new OpenXRProvider::XRExtHandJointsMotionRange( pUtils->GetLogger() );

	std::vector< void * > RequestExtensions { pXRVisibilityMask, pXRHandTracking, pXRHandJointsMotionRange };

	// (2) Setup the application metadata which will be used by the OpenXR Provider
	//     library to setup an instance and session with the OpenXR loader

	OpenXRProvider::XRAppInfo xrAppInfo(
		APP_PROJECT_NAME,
		APP_PROJECT_VER,
		APP_ENGINE_NAME,
		APP_ENGINE_VER,
		OpenXRProvider::TRACKING_ROOMSCALE,
		RequestExtensions, // optional: see step (1), otherwise an empty vector
		pAppLogFile		   // optional: log file logging for the library
	);

	// (3) Setup currently active graphics api info that'll be used to access the user's hardware
	//     and app context
#ifdef XR_USE_GRAPHICS_API_OPENGL
	OpenXRProvider::XRAppGraphicsInfo xrAppGraphicsInfo( wglGetCurrentDC(), wglGetCurrentContext() );
#endif

	// (4) Setup required render info. Texture/images in the swapchain will be generated using the
	//     information provided here
	OpenXRProvider::XRRenderInfo xrRenderInfo(
		{ GL_RGBA16 }, // Request texture formats here in order of preference.
			   // These are uint64_t nums that's defined by the graphics API
			   // (e.g. GL_RGBA16, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, etc)
			   // Otherwise, put 0 in the array to let the runtime decide the format

		{ 0x81A5 }, // Request depth texture formats here in order of preference.
			   // These are uint64_t nums that's defined by the graphics API
			   // (e.g. GL_DEPTH_COMPONENT, DXGI_FORMAT_D16_UNORM, etc)
			   // Otherwise, put 0 in the array to get a default depth format

		1,	  // Texture array size. 1 if not an array.
		1,	  // Mip count
		true // optional: depth texture support if the active runtime supports it
	);

	// (5) Finally, create an XRProvider class
	try
	{
		pXRProvider = new OpenXRProvider::XRProvider( xrAppInfo, xrAppGraphicsInfo, xrRenderInfo );
	}
	catch ( const std::exception &e )
	{
		pUtils->GetLogger()->info( "Unable to create OpenXR Provider. {}", e.what() );
		pUtils->GetLogger()->info( "OpenXR Instance and Session can't be established with the active OpenXR runtime" );
	}

	//// (6) Create input bindings (if any)

	//// 6.1 Setup space locations that will hold the controller poses per frame. Optionally add velocity in the queries
	// xrLocation_Left.next = &xrVelocity_Left;
	// xrLocation_Right.next = &xrVelocity_Right;

	//// 6.2 Create action set(s)
	// xrActionSet_Main = pXRProvider->Input()->CreateActionSet( "main", "main", 0 );

	//// 6.3 Create actions mapped to specific action set(s)
	// xrActionState_PoseLeft.type = XR_TYPE_ACTION_STATE_POSE;
	// xrAction_PoseLeft = pXRProvider->Input()->CreateAction( xrActionSet_Main, "pose_left", "Pose (Left)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );

	//// xrActionState_PoseRight.type = XR_TYPE_ACTION_STATE_POSE;
	// xrAction_PoseRight = pXRProvider->Input()->CreateAction( xrActionSet_Main, "pose_right", "Pose (Right)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );

	// xrActionState_SwitchScene.type = XR_TYPE_ACTION_STATE_BOOLEAN;
	// xrAction_SwitchScene = pXRProvider->Input()->CreateAction( xrActionSet_Main, "switch_scene", "Switch Scenes", XR_ACTION_TYPE_BOOLEAN_INPUT, 0, NULL );

	// xrAction_Haptic = pXRProvider->Input()->CreateAction( xrActionSet_Main, "haptic", "Haptic Feedback", XR_ACTION_TYPE_VIBRATION_OUTPUT, 0, NULL );

	//// 6.4 Create action bindings for every controller this app supports
	//// CreateInputActionBindings();

	//// 6.5 Suggest controller-specific action bindings to the runtime (one call for each controller this app supports)
	// //pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->ValveIndex()->ActionBindings(), pXRProvider->Input()->ValveIndex()->GetInputProfile() );
	// //pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->HTCVive()->ActionBindings(), pXRProvider->Input()->HTCVive()->GetInputProfile() );
	// //pXRProvider->Input()->SuggestActionBindings( pXRProvider->Input()->OculusTouch()->ActionBindings(), pXRProvider->Input()->OculusTouch()->GetInputProfile() );

	//// 6.6 Activate all action sets that we want to update per frame (this can also be changed per frame or anytime app wants to sync a different action set data)
	// //pXRProvider->Input()->ActivateActionSet( xrActionSet_Main );

	// (7) Cache anything your app needs in the frame loop

	// 7.1 Test for activated instance extensions
	 bDrawHandJoints = pXRHandTracking->IsActive();

	// 7.2 Get swapchain capacity
	 nSwapchainCapacity = pXRProvider->Render()->GetGraphicsAPI()->GetSwapchainImageCount( OpenXRProvider::EYE_LEFT );

	 if ( nSwapchainCapacity < 1 )
		{
			// This shouldn't really happen with conformant OpenXR runtimes
			pUtils->GetLogger()->info( "Not enough swapchain capacity ({}) to do any rendering work", nSwapchainCapacity );
		}

	// (8) Optional: Use any pre-render loop extensions

	// Retrieve visibility mask from runtime if available
	 OpenXRProvider::XRExtVisibilityMask *pXRVisibilityMask = pXRProvider->Render()->GetXRVisibilityMask();
	 if ( pXRVisibilityMask )
		{
			pXRVisibilityMask->GetVisibilityMask( OpenXRProvider::EYE_LEFT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN, vMaskVertices_L, vMaskIndices_L );
			pXRVisibilityMask->GetVisibilityMask( OpenXRProvider::EYE_RIGHT, OpenXRProvider::XRExtVisibilityMask::MASK_HIDDEN, vMaskVertices_R, vMaskIndices_R );
		}

	//// pUtils->GetLogger()->info( "Runtime returned a visibility mask with {} verts and {} indices for the left eye (0)", vMaskVertices_L.size() / 2, vMaskIndices_L.size() );
	//// pUtils->GetLogger()->info( "Runtime returned a visibility mask with {} verts and {} indices for the right eye (1)", vMaskVertices_R.size() / 2, vMaskIndices_R.size() );

	return true;
}

void Sandbox::CreateXRSwapchain() 
{
	// Color swapchain
	uint32_t nXRSwapchainSize = 3; // TO-DO OpenXRPRovider (Render) - Get swapchain size

	CreateTextureViews(OpenXRProvider::EXREye::EYE_LEFT, nXRSwapchainSize, pXRProvider->Render()->GetSwapchainColor()[0], false);
	CreateTextureViews(OpenXRProvider::EXREye::EYE_RIGHT, nXRSwapchainSize, pXRProvider->Render()->GetSwapchainColor()[1], false);

	CreateTextureViews( OpenXRProvider::EXREye::EYE_LEFT, nXRSwapchainSize, pXRProvider->Render()->GetSwapchainDepth()[ 0 ], true );
	CreateTextureViews( OpenXRProvider::EXREye::EYE_RIGHT, nXRSwapchainSize, pXRProvider->Render()->GetSwapchainDepth()[ 1 ], true );
}

void Sandbox::InitRenderPipeline() 
{
	CreatePipelineState();
	CreateVertexBuffer();
	CreateIndexBuffer();
	LoadTexture();
}

void Sandbox::CreatePipelineState() 
{
	// Pipeline state object encompasses configuration of all GPU stages

	GraphicsPipelineStateCreateInfo PSOCreateInfo;

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSOCreateInfo.PSODesc.Name = "Cube PSO";

	// This is a graphics pipeline
	PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

	// clang-format off
	// This tutorial will render to a single render target
	PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_RGBA16_UNORM;//m_pSwapChain->GetDesc().ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	PSOCreateInfo.GraphicsPipeline.DSVFormat = TEX_FORMAT_D16_UNORM;//m_pSwapChain->GetDesc().DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// Cull back faces
	PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
	// Enable depth testing
	PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
	// clang-format on

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	RefCntAutoPtr< IShaderSourceInputStreamFactory > pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory( nullptr, &pShaderSourceFactory );
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr< IShader > pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube VS";
		ShaderCI.FilePath = "cube.vsh";
		m_pDevice->CreateShader( ShaderCI, &pVS );
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		CreateUniformBuffer( m_pDevice, sizeof( float4x4 ), "VS constants CB", &m_VSConstants );
	}

	// Create a pixel shader
	RefCntAutoPtr< IShader > pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube PS";
		ShaderCI.FilePath = "cube.psh";
		m_pDevice->CreateShader( ShaderCI, &pPS );
	}

	// clang-format off
	// Define vertex shader input layout
	LayoutElement LayoutElems[] =
	{
		// Attribute 0 - vertex position
		LayoutElement{0, 0, 3, VT_FLOAT32, False},
		// Attribute 1 - texture coordinates
		LayoutElement{1, 0, 2, VT_FLOAT32, False}
	};
	// clang-format on

	PSOCreateInfo.pVS = pVS;
	PSOCreateInfo.pPS = pPS;

	PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
	PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof( LayoutElems );

	// Define variable type that will be used by default
	PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

	// clang-format off
	// Shader variables should typically be mutable, which means they are expected
	// to change on a per-instance basis
	ShaderResourceVariableDesc Vars[] =
	{
		{SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	};
	// clang-format on
	PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
	PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof( Vars );

	// clang-format off
	// Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
	SamplerDesc SamLinearClampDesc
	{
		FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
		TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
	};
	ImmutableSamplerDesc ImtblSamplers[] =
	{
		{SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
	};
	// clang-format on
	PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
	PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof( ImtblSamplers );

	m_pDevice->CreateGraphicsPipelineState( PSOCreateInfo, &m_pPSO );

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables
	// never change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName( SHADER_TYPE_VERTEX, "Constants" )->Set( m_VSConstants );

	// Since we are using mutable variable, we must create a shader resource binding object
	// http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
	m_pPSO->CreateShaderResourceBinding( &m_SRB, true );
}

void Sandbox::CreateVertexBuffer() 
{
	// Layout of this structure matches the one we defined in the pipeline state
	struct Vertex
	{
		float3 pos;
		float2 uv;
	};

	// Cube vertices

	//      (-1,+1,+1)________________(+1,+1,+1)
	//               /|              /|
	//              / |             / |
	//             /  |            /  |
	//            /   |           /   |
	//(-1,-1,+1) /____|__________/(+1,-1,+1)
	//           |    |__________|____|
	//           |   /(-1,+1,-1) |    /(+1,+1,-1)
	//           |  /            |   /
	//           | /             |  /
	//           |/              | /
	//           /_______________|/
	//        (-1,-1,-1)       (+1,-1,-1)
	//

	// clang-format off
	// This time we have to duplicate verices because texture coordinates cannot
	// be shared
	Vertex CubeVerts[] =
	{
		{float3(-1,-1,-1), float2(0,1)},
		{float3(-1,+1,-1), float2(0,0)},
		{float3(+1,+1,-1), float2(1,0)},
		{float3(+1,-1,-1), float2(1,1)},

		{float3(-1,-1,-1), float2(0,1)},
		{float3(-1,-1,+1), float2(0,0)},
		{float3(+1,-1,+1), float2(1,0)},
		{float3(+1,-1,-1), float2(1,1)},

		{float3(+1,-1,-1), float2(0,1)},
		{float3(+1,-1,+1), float2(1,1)},
		{float3(+1,+1,+1), float2(1,0)},
		{float3(+1,+1,-1), float2(0,0)},

		{float3(+1,+1,-1), float2(0,1)},
		{float3(+1,+1,+1), float2(0,0)},
		{float3(-1,+1,+1), float2(1,0)},
		{float3(-1,+1,-1), float2(1,1)},

		{float3(-1,+1,-1), float2(1,0)},
		{float3(-1,+1,+1), float2(0,0)},
		{float3(-1,-1,+1), float2(0,1)},
		{float3(-1,-1,-1), float2(1,1)},

		{float3(-1,-1,+1), float2(1,1)},
		{float3(+1,-1,+1), float2(0,1)},
		{float3(+1,+1,+1), float2(0,0)},
		{float3(-1,+1,+1), float2(1,0)}
	};
	// clang-format on

	BufferDesc VertBuffDesc;
	VertBuffDesc.Name = "Cube vertex buffer";
	VertBuffDesc.Usage = USAGE_IMMUTABLE;
	VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = sizeof( CubeVerts );
	BufferData VBData;
	VBData.pData = CubeVerts;
	VBData.DataSize = sizeof( CubeVerts );
	m_pDevice->CreateBuffer( VertBuffDesc, &VBData, &m_CubeVertexBuffer );
}

void Sandbox::CreateIndexBuffer() 
{
	// clang-format off
	Uint32 Indices[] =
	{
		2,0,1,    2,3,0,
		4,6,5,    4,7,6,
		8,10,9,   8,11,10,
		12,14,13, 12,15,14,
		16,18,17, 16,19,18,
		20,21,22, 20,22,23
	};
	// clang-format on

	BufferDesc IndBuffDesc;
	IndBuffDesc.Name = "Cube index buffer";
	IndBuffDesc.Usage = USAGE_IMMUTABLE;
	IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = sizeof( Indices );
	BufferData IBData;
	IBData.pData = Indices;
	IBData.DataSize = sizeof( Indices );
	m_pDevice->CreateBuffer( IndBuffDesc, &IBData, &m_CubeIndexBuffer );
}

void Sandbox::LoadTexture() 
{
	TextureLoadInfo loadInfo;
	loadInfo.IsSRGB = true;
	RefCntAutoPtr< ITexture > Tex;
	CreateTextureFromFile( "t_bellevue_valve.png", loadInfo, m_pDevice, &Tex );
	// Get shader resource view from the texture
	m_TextureSRV = Tex->GetDefaultView( TEXTURE_VIEW_SHADER_RESOURCE );

	// Set texture SRV in the SRB
	m_SRB->GetVariableByName( SHADER_TYPE_PIXEL, "g_Texture" )->Set( m_TextureSRV );
}

void Sandbox::CreateTextureViews(OpenXRProvider::EXREye eEye, uint32_t nXRSwapchainSize, XrSwapchain xrSwapchain, bool bIsDepth )
{
	std::vector<RefCntAutoPtr<ITexture>> vTextures;

	switch (m_DeviceType)
	{
#if GL_SUPPORTED 
#if XR_USE_GRAPHICS_API_OPENGL
	case RENDER_DEVICE_TYPE_GL:
	{
		std::vector< XrSwapchainImageOpenGLKHR > xrSwapchainImages;
		xrSwapchainImages.resize(nXRSwapchainSize, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
		XrResult xrResult = xrEnumerateSwapchainImages(xrSwapchain, nXRSwapchainSize, &nXRSwapchainSize, (XrSwapchainImageBaseHeader*)&xrSwapchainImages[0]);

		// Create diligent textures from openxr textures
		for (const XrSwapchainImageOpenGLKHR& xrSwapchainImage : xrSwapchainImages)
		{
			RefCntAutoPtr< ITexture > pTexture;
			TextureDesc texDesc;
			texDesc.Type = RESOURCE_DIM_TEX_2D;
			texDesc.BindFlags = bIsDepth ?  BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE : BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

			IRenderDeviceGL* renderDevice = (IRenderDeviceGL*)m_pDevice.RawPtr();
			renderDevice->CreateTextureFromGLHandle(xrSwapchainImage.image, GL_TEXTURE_2D, texDesc, RESOURCE_STATE_UNKNOWN, &pTexture);
			vTextures.push_back(pTexture);
		}
	}		
	break;
#endif
#endif

	} // end of switch m_DeviceType

	// Create diligent texture views (our xr swapchain) that we can render to
	for (RefCntAutoPtr< ITexture >& pTexture : vTextures)
	{
		TextureViewDesc viewDesc;
		viewDesc.ViewType = bIsDepth ? TEXTURE_VIEW_DEPTH_STENCIL : TEXTURE_VIEW_RENDER_TARGET;
		viewDesc.AccessFlags = UAV_ACCESS_FLAG_WRITE;

		RefCntAutoPtr< ITextureView > pEyeView;
		pTexture->CreateView(viewDesc, &pEyeView);

		if (bIsDepth)
			m_xrSwapchainDepth[ eEye ].push_back( pEyeView );
		else
			m_xrSwapchainColor[eEye].push_back(pEyeView);	
	}
}

void Sandbox::Render()
{
	// Check if runtime wants to close the app
	if ( xrCurrentSessionState == XR_SESSION_STATE_EXITING )
		return;

	if ( xrCurrentSessionState == XR_SESSION_STATE_IDLE )
	{
		// HMD is not ready or inactive, clear window with clear color
		glClearColor( 0.5f, 0.9f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
		//BlitToWindow();
	}
	else if ( xrCurrentSessionState == XR_SESSION_STATE_READY )
	{
		// Begin the session - e.g. coming back from stopping state
		XrResult xrResult = pXRProvider->Core()->XRBeginSession();
		bool bResult = pXRProvider->Render()->ProcessXRFrame();

		pUtils->GetLogger()->info( "OpenXR Session started ({}) and initial frame processed ({})", OpenXRProvider::XrEnumToString( xrResult ), bResult );
	}
	else if ( xrCurrentSessionState == XR_SESSION_STATE_STOPPING )
	{
		// End session so runtime can safely transition back to idle
		XrResult xrResult = pXRProvider->Core()->XREndSession();

		pUtils->GetLogger()->info( "OpenXR Session ended ({})", OpenXRProvider::XrEnumToString( xrResult ) );
	}
	else if ( xrCurrentSessionState > XR_SESSION_STATE_IDLE )
	{
		// (2) Process frame - call ProcessXRFrame from the render manager
		if ( pXRProvider->Render() && pXRProvider->Render()->ProcessXRFrame() )
		{
			// 2.1 Render to swapchain image
			nSwapchainIndex = nSwapchainIndex > nSwapchainCapacity - 1 ? 0 : nSwapchainIndex;

			DrawFrame( OpenXRProvider::EYE_LEFT, nSwapchainIndex );
			DrawFrame( OpenXRProvider::EYE_RIGHT, nSwapchainIndex );

			// Blit (copy) texture to XR Mirror
			//BlitToWindow();

			// Update app frame state
			++nFrameNumber;
			++nSwapchainIndex;

			// [DEBUG] pUtils->GetLogger()->info("HMD IPD is currently set to: {}", xrRenderManager->GetCurrentIPD());
		}

		// (3) Process input
		//if ( xrCurrentSessionState == XR_SESSION_STATE_FOCUSED && pXRProvider->Input() )
		//{
		//	// 3.1 Sync data with runtime for all active action sets
		//	pXRProvider->Input()->SyncActiveActionSetsData();

		//	// 3.2 Process all received input states from previous sync call
		//	ProcessInputStates();

		//	// 3.3 Update controller and/or other action poses
		//	//     as we are not a pipelined app (single threaded), we're using one time period ahead of the current frame pose
		//	uint64_t nPredictedTime = pXRProvider->Render()->GetPredictedDisplayTime() + pXRProvider->Render()->GetPredictedDisplayPeriod();

		//	pXRProvider->Input()->GetActionPose( xrAction_PoseLeft, nPredictedTime, &xrLocation_Left );
		//	pXRProvider->Input()->GetActionPose( xrAction_PoseRight, nPredictedTime, &xrLocation_Right );

		//	// 3.4 Update any other input dependent poses (e.g. handtracking extension)
		//	if ( bDrawHandJoints )
		//	{
		//		// Left Hand (Open Hand/Unobstructed)
		//		pXRProvider->Core()->GetExtHandTracking()->LocateHandJoints( XR_HAND_LEFT_EXT, pXRProvider->Core()->GetXRSpace(), nPredictedTime );

		//		// Right Hand (With Controller if motion range is available)
		//		if ( pXRHandJointsMotionRange != nullptr )
		//		{
		//			pXRProvider->Core()->GetExtHandTracking()->LocateHandJoints(
		//				XR_HAND_RIGHT_EXT, pXRProvider->Core()->GetXRSpace(), nPredictedTime, XR_HAND_JOINTS_MOTION_RANGE_CONFORMING_TO_CONTROLLER_EXT );
		//		}
		//		else
		//		{
		//			pXRProvider->Core()->GetExtHandTracking()->LocateHandJoints( XR_HAND_RIGHT_EXT, pXRProvider->Core()->GetXRSpace(), nPredictedTime );
		//		}
		//	}
		//}
	}

}

void Sandbox::Present() 
{
	m_pSwapChain->Present();
}

void Sandbox::DrawFrame( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex )
{
	// Check if hmd is tracking
	if ( !pXRProvider->Render()->GetHMDState()->IsPositionTracked || !pXRProvider->Render()->GetHMDState()->IsPositionTracked )
		return;

	// Draw vismask on XR Mirror
	uint32_t nVertCount = ( eEye == OpenXRProvider::EYE_LEFT ) ? ( uint32_t )vMaskVertices_L.size() : ( uint32_t )vMaskVertices_L.size();
	float *vMask = ( eEye == OpenXRProvider::EYE_LEFT ) ? vMaskVertices_L.data() : vMaskVertices_R.data();

	if ( nVertCount > 0 )
	{
		// TO DO: Apply vismask, set clip values
	}

	// Draw current active scene
	switch ( eCurrentScene )
	{
		case SANDBOX_SCENE_SEA_OF_CUBES:
			// glm::vec3 cubeScale = glm::vec3( 0.5f, 0.5f, 0.5f );
			// DrawSeaOfCubesScene( eEye, nSwapchainIndex, cubeScale, 1.5f, 1.5f );
			DrawScene_SeaOfCubes(eEye, nSwapchainIndex);
			break;

		case SANDBOX_SCENE_HAND_TRACKING:
			//DrawHandTrackingScene( eEye, nSwapchainIndex );

		default:
			break;
	}
}

void Sandbox::DrawScene_SeaOfCubes( OpenXRProvider::EXREye eEye, uint32_t nSwapchainIndex ) 
{
	// Clear the back buffer
	const float ClearColor[] = { 0.5f, 0.9f, 1.0f, 1.0f };
	m_pImmediateContext->SetRenderTargets( 1, &m_xrSwapchainColor[eEye][nSwapchainIndex], m_xrSwapchainDepth[eEye][nSwapchainIndex], RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
	m_pImmediateContext->ClearRenderTarget( m_xrSwapchainColor[eEye][nSwapchainIndex], ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
	m_pImmediateContext->ClearDepthStencil( m_xrSwapchainDepth[eEye][nSwapchainIndex], CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

	// Get eye projection
	float4x4 eyeProjection = (eEye == OpenXRProvider::EYE_LEFT) ? GetEyeProjection( pXRProvider->Render()->GetHMDState()->LeftEye.FoV, 0.1f, 100.f ) 
		: GetEyeProjection( pXRProvider->Render()->GetHMDState()->RightEye.FoV, 0.1f, 100.f );

	// Get eye views
	XrPosef eyePose = (eEye == OpenXRProvider::EYE_LEFT) ? pXRProvider->Render()->GetHMDState()->LeftEye.Pose : pXRProvider->Render()->GetHMDState()->RightEye.Pose;
	XrMatrix4x4f xrEyeView;
	XrVector3f xrScale { 1.0, 1.0, 1.0f };
	XrMatrix4x4f_CreateTranslationRotationScale( &xrEyeView, &eyePose.position, &eyePose.orientation, &xrScale );

	float4x4 eyeView = float4x4::MakeMatrix( xrEyeView.m );

	// Calculate mvp
	float4x4 CubeModelTransform = float4x4::RotationY( static_cast< float >( timeCurrent ) * 1.0f ) * float4x4::RotationX( -PI_F * 0.1f );

	m_WorldViewProjMatrix = CubeModelTransform * eyeView.Inverse() * eyeProjection;

	// Map the buffer and write current world-view-projection matrix
	MapHelper< float4x4 > CBConstants( m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD );
	*CBConstants = m_WorldViewProjMatrix.Transpose();

	// Bind vertex and index buffers
	Uint32 offset = 0;
	IBuffer *pBuffs[] = { m_CubeVertexBuffer };
	m_pImmediateContext->SetVertexBuffers( 0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET );
	m_pImmediateContext->SetIndexBuffer( m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

	// Set the pipeline state
	m_pImmediateContext->SetPipelineState( m_pPSO );
	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
	// makes sure that resources are transitioned to required states.
	m_pImmediateContext->CommitShaderResources( m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

	DrawIndexedAttribs DrawAttrs;	 // This is an indexed draw call
	DrawAttrs.IndexType = VT_UINT32; // Index type
	DrawAttrs.NumIndices = 36;
	// Verify the state of vertex and index buffers
	DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
	m_pImmediateContext->DrawIndexed( DrawAttrs );

	//DrawAttribs drawAttrs;
	//drawAttrs.NumVertices = 3; // Render 3 vertices
	//m_pImmediateContext->Draw( drawAttrs );

	// TODO: Blit to mirror instead of redrawing
	if (eEye == OpenXRProvider::EYE_LEFT)
	{
		auto *pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
		auto *pDSV = m_pSwapChain->GetDepthBufferDSV();
		m_pImmediateContext->SetRenderTargets( 1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

		// Clear the back buffer
		m_pImmediateContext->ClearRenderTarget( pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		m_pImmediateContext->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

		//DrawAttribs drawAttrs;
		//drawAttrs.NumVertices = 3; // Render 3 vertices
		//m_pImmediateContext->Draw( drawAttrs );

		// Clear the back buffer
		//m_pImmediateContext->ClearRenderTarget( pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		//m_pImmediateContext->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		//{
		//	// Map the buffer and write current world-view-projection matrix
		//	MapHelper< float4x4 > CBConstants( m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD );
		//	*CBConstants = m_WorldViewProjMatrix.Transpose();
		//}

		//// Bind vertex and index buffers
		//Uint32 offset = 0;
		//IBuffer *pBuffs[] = { m_CubeVertexBuffer };
		//m_pImmediateContext->SetVertexBuffers( 0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET );
		//m_pImmediateContext->SetIndexBuffer( m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

		//// Set the pipeline state
		//m_pImmediateContext->SetPipelineState( m_pPSO );
		//// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
		//// makes sure that resources are transitioned to required states.
		//m_pImmediateContext->CommitShaderResources( m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

		//DrawIndexedAttribs DrawAttrs;	 // This is an indexed draw call
		//DrawAttrs.IndexType = VT_UINT32; // Index type
		//DrawAttrs.NumIndices = 36;
		//// Verify the state of vertex and index buffers
		//DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		//m_pImmediateContext->DrawIndexed( DrawAttrs );
	}
}

float4x4 Sandbox::GetEyeProjection( XrFovf eyeFoV, float fNear, float fFar ) 
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
	float4x4 mProjection = float4x4( 1.0f );

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

void Sandbox::Update(double CurrTime, double ElapsedTime)
{ 
	timeCurrent = CurrTime;
	timeElpased = ElapsedTime;
}

void Sandbox::WindowResize( Uint32 Width, Uint32 Height )
{
	if ( m_pSwapChain )
		m_pSwapChain->Resize( Width, Height );
}

bool Sandbox::ProcessCommandLine( const char *CmdLine )
{
	const auto *Key = "-mode ";
	const auto *pos = strstr( CmdLine, Key );
	if ( pos != nullptr )
	{
		pos += strlen( Key );
		if ( _stricmp( pos, "D3D11" ) == 0 )
		{
#if D3D11_SUPPORTED
			m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
#else
			std::cerr << "Direct3D11 is not supported. Please select another device type";
			return false;
#endif
		}
		else if ( _stricmp( pos, "D3D12" ) == 0 )
		{
#if D3D12_SUPPORTED
			m_DeviceType = RENDER_DEVICE_TYPE_D3D12;
#else
			std::cerr << "Direct3D12 is not supported. Please select another device type";
			return false;
#endif
		}
		else if ( _stricmp( pos, "GL" ) == 0 )
		{
#if GL_SUPPORTED
			m_DeviceType = RENDER_DEVICE_TYPE_GL;
#else
			std::cerr << "OpenGL is not supported. Please select another device type";
			return false;
#endif
		}
		else if ( _stricmp( pos, "VK" ) == 0 )
		{
#if VULKAN_SUPPORTED
			m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;
#else
			std::cerr << "Vulkan is not supported. Please select another device type";
			return false;
#endif
		}
		else
		{
			std::cerr << "Unknown device type. Only the following types are supported: D3D11, D3D12, GL, VK";
			return false;
		}
	}
	else
	{
//#if D3D12_SUPPORTED
//		m_DeviceType = RENDER_DEVICE_TYPE_D3D12;
//#elif VULKAN_SUPPORTED
//		m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;
//#elif D3D11_SUPPORTED
//		m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
#if GL_SUPPORTED
		m_DeviceType = RENDER_DEVICE_TYPE_GL;
#endif
	}
	return true;
}

