/*  ----------------------------------- INFOS
	This header file contains functions related to rendering with D3D11.
	
*/

// LOCAL DEPENDENCIES : types.h, platform.h, file.h

#ifndef _DX11H_
#define _DX11H_

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

// custom libs
#include "../types.h"
#include "render.h"
#include "../platform/platform.h"
#include "../platform/file.h"
#include "../platform/texture.h"


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
	int vCount;
	vertex vQueue[4096];
	ID3D11Buffer* vbuffer;
	ID3D11Buffer* ubuffer;
	ID3D11SamplerState* sampler;
};


// ---------------------- FUNCTIONS

void render_queue_quad(quad_mesh* quadData, v2 texposMin, v2 texposMax, render_context* rContext) {		
	// texture_convertTexposToNDC(hitCircle_pos);
	
	// todo: refactor this 
    struct vertex VQuad[] =
    {
		/*STRUCTURE OF A VERTICE : 
		
		{x, y, t, c}
		x -> horizontal position;
		y -> vertical position;
		t -> texture pos;
		c -> texture color; */  
		
		
        // first triangle ◥
        { quadData->x - (quadData->width/2.f), quadData->y + (quadData->height/2.f), { texposMin.x, texposMin.y }, { 1, 1, 1 } },  // top left (2)
        { quadData->x + (quadData->width/2.f), quadData->y + (quadData->height/2.f), { texposMax.x,  texposMin.y }, { 1, 1, 1 } },	// top right
        { quadData->x + (quadData->width/2.f), quadData->y - (quadData->height/2.f), { texposMax.x,  texposMax.y }, { 1, 1, 1 } },  // bottom right (2)
        // second triangle ◣
        { quadData->x - (quadData->width/2.f), quadData->y + (quadData->height/2.f), { texposMin.x, texposMin.y }, { 1, 1, 1 } },    	// top left (2)
        { quadData->x - (quadData->width/2.f), quadData->y - (quadData->height/2.f), { texposMin.x,  texposMax.y }, { 1, 1, 1 } }, 		// bottom left
        { quadData->x + (quadData->width/2.f), quadData->y - (quadData->height/2.f), { texposMax.x,  texposMax.y }, { 1, 1, 1 } },   // bottom right (2)
    };
	
    for(int i=0; i<6; i++) {
		rContext->vQueue[rContext->vCount] = VQuad[i];
		rContext->vCount++;
	};
};

void RENDER_UPLOAD_DYNAMIC_VertexQueue(render_context* rContext){
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map((ID3D11Resource*)rContext->vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, rContext->vQueue, sizeof(vertex)*rContext->vCount);
	rContext->context->Unmap((ID3D11Resource*)rContext->vbuffer, 0);
};

void render_upload_camera_uBuffer(render_context *rContext, game_camera* camera, viewport_size vp){	
	
	float width = (float)vp.width;
	float height = (float)vp.height;
	
	mx matrix = game_OrthographicProjection(camera, width, height);
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map(rContext->ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &matrix, sizeof(matrix));
	rContext->context->Unmap(rContext->ubuffer, 0);
};

void RENDER_INIT_PIPELINE(render_context* rContext, viewport_size* vpSize){
	

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
};


void RENDER_INIT_DYNAMIC_VertexBuffer(render_context *rContext, int bufferSize) {
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(vertex) * bufferSize,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	D3D11_SUBRESOURCE_DATA initial = { .pSysMem = rContext->vQueue };
    rContext->device->CreateBuffer(&desc, NULL, &rContext->vbuffer);
}

void render_init_ubuffer(render_context* rContext) {
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(mx) * 2,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

    rContext->device->CreateBuffer(&desc, NULL, &rContext->ubuffer); 
};


void RENDER_RESIZE_SWAP_CHAIN(HWND window, viewport_size* windowSize, render_context* rContext) {
	
		HRESULT hr;
	
        // get current size for window client area
        RECT rect;
        GetClientRect(window, &rect);
		
		viewport_size newWindowSize = {
			(ui32)rect.bottom - rect.top << 0,
			(ui32)rect.right - rect.left << 0,
		};
		
		
	    if (rContext->rtView == NULL || newWindowSize.width != windowSize->width || newWindowSize.height != windowSize->height) {
            if (rContext->rtView)
            {
                // release old swap chain buffers
                rContext->context->ClearState();
                rContext->rtView->Release();
                rContext->dsView->Release();
                rContext->rtView = NULL;
            }

            // resize to new size for non-zero size
            if (newWindowSize.width != 0 && newWindowSize.height != 0)
            {
                hr = rContext->swapChain->ResizeBuffers(0, newWindowSize.width, newWindowSize.height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D11Texture2D* backbuffer;
                hr = rContext->swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbuffer);
				// rContext->swapChain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
                hr = rContext->device->CreateRenderTargetView(backbuffer, NULL, &rContext->rtView);
                backbuffer->Release();

                D3D11_TEXTURE2D_DESC depthDesc = 
                {
                    .Width = newWindowSize.width,
                    .Height = newWindowSize.height,
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
                rContext->device->CreateDepthStencilView(depth, NULL, &rContext->dsView);
                depth->Release();
            }

			windowSize->width = newWindowSize.width;
			windowSize->height = newWindowSize.height;
        }
};

HRESULT RENDER_INIT_DX(HWND window, render_context* rContext, game_camera* camera) {
	
	HRESULT hr;
	
	viewport_size vp = platform_getWindowSize(window);
	
    // create D3D11 device & context
    {
        UINT flags = 0;
	#ifdef NDEBUG
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

	#ifdef NDEBUG
	
    // for debug builds enable VERY USEFUL debug break on API errors
    {
        ID3D11InfoQueue* info;
        rContext->device->QueryInterface(IID_ID3D11InfoQueue, (void**)&info);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        info->Release();
    }

    // enable debug break for DXGI too
    {
        IDXGIInfoQueue* dxgiInfo;
        hr = DXGIGetDebugInterface1(0, IID_IDXGIInfoQueue, (void**)&dxgiInfo);
        AssertHR(hr);
        dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        dxgiInfo->Release();
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

	// init needed buffers 
	{
		render_init_ubuffer(rContext);	
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


		char vsLocation[] = "triangle.vs.fxc";
		char psLocation[] = "triangle.ps.fxc";
	
		complete_file vblob = {0};
		complete_file pblob = {0};
		
		file_fullread(vsLocation, &vblob);
		file_fullread(psLocation, &pblob);
		
		
		// this is where we send shaders to the GPU
		
        hr = rContext->device->CreateVertexShader(vblob.memory, vblob.size, NULL, &rContext->vshader);
        hr = rContext->device->CreatePixelShader(pblob.memory, pblob.size, NULL, &rContext->pshader);
        hr = rContext->device->CreateInputLayout(desc, ARRAYSIZE(desc), vblob.memory, vblob.size, &rContext->layout);

		file_fullfree(&vblob);
		file_fullfree(&pblob);
    }

    {
        // checkerboard texture, with 50% transparency on black colors
		
		// todo: asset pipeline for textures
		
		// for testing
        // unsigned int pixels[] =
        // {
            // 0x80000000, 0xffffffff,
            // 0xffffffff, 0x80000000,
        // };
		
		// open and decode the hitcircle texture
		char* location = gameTextureLocation;
		complete_file textureFile = {0};
		complete_img hitcircle_sprite = file_decodePNG(location, &textureFile);

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width = hitcircle_sprite.x,
            .Height = hitcircle_sprite.y,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        D3D11_SUBRESOURCE_DATA data =
        {
            .pSysMem = hitcircle_sprite.memory,
            .SysMemPitch = hitcircle_sprite.x * hitcircle_sprite.channels_in_file,
        };

        ID3D11Texture2D* texture;
        rContext->device->CreateTexture2D(&desc, &data, &texture);
        rContext->device->CreateShaderResourceView((ID3D11Resource*)texture, NULL, &rContext->textureView);
        texture->Release();
		file_fullfree(&textureFile);
    }

    
    {
        D3D11_SAMPLER_DESC desc =
        {
            .Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
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
		render_upload_camera_uBuffer(rContext, camera, vp);
    }

    rContext->rtView = NULL;
    rContext->dsView = NULL;


	return hr;
};

void render_reset_frame(render_context* rContext){
	rContext->vCount = 0;
};

#endif /* _DX11H_ */
