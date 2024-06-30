/*  ----------------------------------- INFOS
	This header file contains functions related to rendering with D3D11.
	
*/

// LOCAL DEPENDENCIES : types.h, platform.h, file.h

#ifndef _RENDERH_
#define _RENDERH_

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPLEMENTATION

#include <math.h>
#include <string.h>
#include <stddef.h>

// imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "imgui_impl_win32.cpp"
#include "imgui_impl_dx11.cpp"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

// custom libs
#include "platform.h"
#include "types.h"
#include "file.h"


// replace this with your favorite Assert() implementation
#include <intrin.h>

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#define STR2(x) #x
#define STR(x) STR2(x)


// ----------------------------------------------- STRUCTS

// types

struct vertex
{
    float position[2];
    float uv[2];
    float color[3];
};

struct quad_mesh {
	i32 x;
	i32 y;
	float size;
	float orientation;
};

struct viewport_size {
	int height;
	int width;
};

// ----- api stuff

struct render_context {
	// basic device stuff
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain1* swapChain;
	
	// rc states
	ID3D11RenderTargetView* rtView;
	ID3D11DepthStencilView* dsView; 
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rasterizerState;
	
	
	ID3D11VertexShader* vshader;
    ID3D11PixelShader* pshader;
	ID3D11ShaderResourceView* textureView;
	ID3D11DepthStencilState* depthState;
	ID3D11BlendState* blendState;
	
	// buffers and shaders
	int VertexCount;
	vertex VertexQueue[2048];
	ID3D11Buffer* vbuffer;
	ID3D11Buffer* ubuffer;
	ID3D11SamplerState* sampler;
};


// -------------------------------------------------------------------------------------------- FUNCTIONS

// --------------------------------- RENDER_QUEUE

void RENDER_QUEUE_Quad(quad_mesh* quadData, render_context* rContext) {
        // .topLeft = {quadData->x - (quadData->size/2), quadData->y + (quadData->size/2)},
        // .topRight = {quadData->x + (quadData->size/2), quadData->y + (quadData->size/2)},
        // .bottomLeft = {quadData->x - (quadData->size/2), quadData->y - (quadData->size/2)},
        // .bottomRight = {quadData->x + (quadData->size/2), quadData->y - (quadData->size/2)},
    
	
	// todo: refactor this 
    struct vertex VQuad[] =
    {
        // first triangle ◥
        { quadData->x - (quadData->size/2), quadData->y + (quadData->size/2), { 25.0f, 50.0f }, { 1, 0, 0 } },   // top left
        { quadData->x + (quadData->size/2), quadData->y + (quadData->size/2), {  0.0f,  0.0f }, { 0, 1, 0 } },   // top right
        { quadData->x + (quadData->size/2), quadData->y - (quadData->size/2), { 50.0f,  0.0f }, { 0, 0, 1 } },   // bottom right
        // second triangle ◣
        { quadData->x - (quadData->size/2), quadData->y + (quadData->size/2), { 50.0f, 0.0f }, { 0, 0, 1 } },    // top left
        { quadData->x - (quadData->size/2), quadData->y - (quadData->size/2), {  25.0f,  50.0f }, { 1, 0, 0 } }, // bottom left
        { quadData->x + (quadData->size/2), quadData->y - (quadData->size/2), { 0.0f,  0.0f }, { 0, 1, 0 } },    // bottom right
    };
	
    for(int i=0; i<6; i++) {
		rContext->VertexQueue[rContext->VertexCount] = VQuad[i];
		rContext->VertexCount++;
	};
}

// --------------------------------- RENDER_UPLOAD

void RENDER_UPLOAD_DYNAMIC_VertexQueue(render_context* rContext){
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map((ID3D11Resource*)rContext->vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, rContext->VertexQueue, sizeof(vertex)*rContext->VertexCount);
	rContext->context->Unmap((ID3D11Resource*)rContext->vbuffer, 0);
}

// --------------------------------- ROTATE OBJECT


void RENDER_INIT_IMMUTABLE_uBuffer(render_context *rContext){
	// setup 4x4c rotation matrix in uniform
	// angle += delta * 0.2f * (float)M_PI / 20.0f; // full rotation in 20 seconds
	// angle = fmodf(angle, 2.0f * (float)M_PI);

	// float aspect = (float)height / width;
	float matrix[16] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
	
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(matrix),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	D3D11_SUBRESOURCE_DATA initial = { .pSysMem = matrix };
    rContext->device->CreateBuffer(&desc, NULL, &rContext->ubuffer);
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map(rContext->ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, matrix, sizeof(matrix));
	rContext->context->Unmap(rContext->ubuffer, 0);
}

// camera projection
matrix game_OrthographicProjection(game_camera* camera, float width, float height)
{
    float L = camera->position.x - width / 2.f;
    float R = camera->position.x + width / 2.f;
    float T = camera->position.y + height / 2.f;
    float B = camera->position.y - height / 2.f;
    matrix res = 
    {
        2.0f/(R-L),   0.0f,           0.0f,       0.0f,
        0.0f,         2.0f/(T-B),     0.0f,       0.0f,
        0.0f,         0.0f,           0.5f,       0.0f,
        (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f,
    };
    
    return res;
}

void RENDER_EXEC_PIPELINE(render_context* rContext, viewport_size* vpSize){
	        D3D11_VIEWPORT viewport =
            {
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = (FLOAT)vpSize->width,
                .Height = (FLOAT)vpSize->height,
                .MinDepth = 0,
                .MaxDepth = 1,
            };
	 
            // Input Assembler
            rContext->context->IASetInputLayout(rContext->layout);
            rContext->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            UINT stride = sizeof(struct vertex);
            UINT offset = 0;
			rContext->context->IASetVertexBuffers(0, 1, &rContext->vbuffer, &stride, &offset);

            // Vertex Shader
            rContext->context->VSSetConstantBuffers(0, 1, &rContext->ubuffer);
            rContext->context->VSSetShader(rContext->vshader, NULL, 0);

            // Rasterizer Stage
            rContext->context->RSSetViewports(1, &viewport);
            rContext->context->RSSetState(rContext->rasterizerState);

            // Pixel Shader
            rContext->context->PSSetSamplers(0, 1, &rContext->sampler);
            rContext->context->PSSetShaderResources(0, 1, &rContext->textureView);
            rContext->context->PSSetShader(rContext->pshader, NULL, 0);

            // Output Merger
			rContext->context->OMSetBlendState(rContext->blendState, NULL, ~0U);
            rContext->context->OMSetDepthStencilState(rContext->depthState, 0);
            rContext->context->OMSetRenderTargets(1, &rContext->rtView, rContext->dsView);	
}

// --------------------------------- RENDER_INIT

void RENDER_INIT_DYNAMIC_VertexBuffer(render_context *rContext, int bufferSize) {
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(vertex) * bufferSize,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	D3D11_SUBRESOURCE_DATA initial = { .pSysMem = rContext->VertexQueue };
    rContext->device->CreateBuffer(&desc, NULL, &rContext->vbuffer);
}

// --------------------------------- RENDER_RESIZE_SWAP_CHAIN

void RENDER_RESIZE_SWAP_CHAIN(HWND window, DWORD currentWidth, DWORD currentHeight, render_context* rContext) {
	
        // get current size for window client area
        RECT rect;
        GetClientRect(window, &rect);
		
		viewport_size viewport = {
			.height = rect.bottom - rect.top,
			.width = rect.right - rect.left,
		};
		
	    if (rContext->rtView == NULL || viewport.width != currentWidth || viewport.height != currentHeight) {
            if (rContext->rtView)
            {
                // release old swap chain buffers
                rContext->context->ClearState();
                rContext->rtView->Release();
                rContext->dsView->Release();
                rContext->rtView = NULL;
            }

            // resize to new size for non-zero size
            if (width != 0 && height != 0)
            {
                hr = rContext->swapChain->ResizeBuffers(0, viewport.width, viewport.height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D11Texture2D* backbuffer;
                rContext->swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbuffer);
                rContext->device->CreateRenderTargetView((ID3D11Resource*)backbuffer, NULL, &rContext.rtView);
                backbuffer->Release();

                D3D11_TEXTURE2D_DESC depthDesc = 
                {
                    .Width = (ui32)viewport.width,
                    .Height = (ui32)viewport.height,
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_D32_FLOAT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
                    .SampleDesc = { 1, 0 },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                };

                // create new depth stencil texture & DepthStencil view
                ID3D11Texture2D* depth;
                rContext->device->CreateTexture2D(&depthDesc, NULL, &depth);
                rContext->device->CreateDepthStencilView((ID3D11Resource*)depth, NULL, &rContext.dsView);
                depth->Release();
            }

            currentWidth = viewport.width;
            currentHeight = viewport.height;
        }
}



// --------------------------------- RENDER_QUEUE

HRESULT RENDER_INIT_DX(HWND window, render_context* rContext) {
	
	HRESULT hr;
	
    // create D3D11 device & context
    {
        UINT flags = 0;
	#ifndef NDEBUG
		// this enables VERY USEFUL debug messages in debugger output
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        hr = D3D11CreateDevice(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, ARRAYSIZE(levels),
            D3D11_SDK_VERSION, &rContext->device, NULL, &rContext->context);

        // We could try D3D_DRIVER_TYPE_WARP driver type which enables software rendering
        // (could be useful on broken drivers or remote desktop situations)
        AssertHR(hr);
    }

	#ifndef NDEBUG
	
    // for debug builds enable VERY USEFUL debug break on API errors
    {
        ID3D11InfoQueue* info;
        rContext->device->QueryInterface(IID_ID3D11InfoQueue, (void**)&info);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        info->Release(info);
    }

    // enable debug break for DXGI too
    {
        IDXGIInfoQueue* dxgiInfo;
        hr = DXGIGetDebugInterface1(0, &IID_IDXGIInfoQueue, (void**)&dxgiInfo);
        AssertHR(hr);
        dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        dxgiInfo->SetBreakOnSeverity(dxgiInfo, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        dxgiInfo->Release(dxgiInfo);
    }

    // after this there's no need to check for any errors on device functions manually
    // so all HRESULT return  values in this code will be ignored
    // debugger will break on errors anyway
	#endif
	


    // create DXGI swap chain
    {
        // get DXGI device from D3D11 device
		IDXGIDevice* dxgiDevice;
        hr = rContext->device->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
        AssertHR(hr);

        // get DXGI adapter from DXGI device
        IDXGIAdapter* dxgiAdapter;
        hr = dxgiDevice->GetAdapter(&dxgiAdapter);
        AssertHR(hr);

        // get DXGI factory from DXGI adapter
        IDXGIFactory2* factory;
        hr = dxgiAdapter->GetParent(IID_IDXGIFactory2, (void**)&factory);
        AssertHR(hr);

        DXGI_SWAP_CHAIN_DESC1 desc =
        {
            // default 0 value for width & height means to get it from HWND automatically
            //.Width = 0,
            //.Height = 0,

            // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,

            // FLIP presentation model does not allow MSAA framebuffer
            // if you want MSAA then you'll need to render offscreen and manually
            // resolve to non-MSAA framebuffer
            .SampleDesc = { 1, 0 },

            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,

            // we don't want any automatic scaling of window content
            // this is supported only on FLIP presentation model
            .Scaling = DXGI_SCALING_NONE,

            // use more efficient FLIP presentation model
            // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
            // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        hr = factory->CreateSwapChainForHwnd((IUnknown*)rContext->device, window, &desc, NULL, NULL, &rContext->swapChain);
        AssertHR(hr);

        // disable silly Alt+Enter changing monitor resolution to match window size
        factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

        factory->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();
    }


   

    // vertex & pixel shaders for drawing triangle, plus input layout for vertex input
    {
        // these must match vertex shader input layout (VS_INPUT in vertex shader source below)
        D3D11_INPUT_ELEMENT_DESC desc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };


		char vsLocation[] = "triangle.vs";
		char psLocation[] = "triangle.ps";
	
		complete_file vblob = {0};
		complete_file pblob = {0};
		
		FILE_FULLREAD(vsLocation, &vblob);
		FILE_FULLREAD(psLocation, &pblob);
		
		
		// this is where we send shaders to the GPU
		
        rContext->device->CreateVertexShader(vblob.memory, vblob.size, NULL, &rContext->vshader);
        rContext->device->CreatePixelShader(pblob.memory, pblob.size, NULL, &rContext->pshader);
        rContext->device->CreateInputLayout(desc, ARRAYSIZE(desc), vblob.memory, vblob.size, &rContext->layout);

		FILE_FULLFREE(&vblob);
		FILE_FULLFREE(&pblob);
    }

    {
        // checkerboard texture, with 50% transparency on black colors
		
		// todo: asset pipeline for textures
        unsigned int pixels[] =
        {
            0x80000000, 0xffffffff,
            0xffffffff, 0x80000000,
        };
        UINT width = 2;
        UINT height = 2;

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        D3D11_SUBRESOURCE_DATA data =
        {
            .pSysMem = pixels,
            .SysMemPitch = width * sizeof(unsigned int),
        };

        ID3D11Texture2D* texture;
        rContext->device->CreateTexture2D(&desc, &data, &texture);
        rContext->device->CreateShaderResourceView((ID3D11Resource*)texture, NULL, &rContext->textureView);
        texture->Release();
    }

    
    {
        D3D11_SAMPLER_DESC desc =
        {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .MipLODBias = 0,
            .MaxAnisotropy = 1,
            .MinLOD = 0,
            .MaxLOD = D3D11_FLOAT32_MAX,
        };

        rContext->device->CreateSamplerState(&desc, &rContext->sampler);
    }


    {
        // enable alpha blending
        D3D11_BLEND_DESC desc = {0};
		desc.RenderTarget[0] =
            {
                .BlendEnable = TRUE,
                .SrcBlend = D3D11_BLEND_SRC_ALPHA,
                .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D11_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA,
                .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOpAlpha = D3D11_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
            },
   
        rContext->device->CreateBlendState(&desc, &rContext->blendState);
    }

    
    {
        // disable culling
        D3D11_RASTERIZER_DESC desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_NONE,
            .DepthClipEnable = TRUE,
        };
        rContext->device->CreateRasterizerState(&desc, &rContext->rasterizerState);
    }

    
    {
        // disable depth & stencil test
        D3D11_DEPTH_STENCIL_DESC desc =
        {
            .DepthEnable = FALSE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
            // .FrontFace = ... 
            // .BackFace = ...
        };
        rContext->device->CreateDepthStencilState(&desc, &rContext->depthState);
    }


	// create Dynamic Shader Buffer
    {
		RENDER_INIT_DYNAMIC_VertexBuffer(rContext, 2048);
		RENDER_INIT_IMMUTABLE_uBuffer(rContext);
    }

    rContext->rtView = NULL;
    rContext->dsView = NULL;


	return hr;
}

// ------------------------------------- IMGUI CUSTOM STUFF

void IMGUI_INIT(HWND window, render_context rContext) {
	// IMGUI Init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(rContext.device, rContext.context);
}

void IMGUI_RENDER() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
} 



#endif /* _RENDERH_ */
