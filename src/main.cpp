// example how to set up D3D11 rendering on Windows in C

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

// replace this with your favorite Assert() implementation
#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#define STR2(x) #x
#define STR(x) STR2(x)

#include "./render/render.h"

enum GameState { menu, pause, game };

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(ImGui_ImplWin32_WndProcHandler(wnd, msg, wparam, lparam))
    {
        return true;
    }
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
	
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
    // register window class to have custom WindowProc callback
    WNDCLASSEXW wc =
    {
        .cbSize = sizeof(wc),
        .lpfnWndProc = WindowProc,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = L"d3d11_window_class",
    };
    ATOM atom = RegisterClassExW(&wc);
    Assert(atom && "Failed to register window class");

    // window properties - width, height and style
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
    // WS_EX_NOREDIRECTIONBITMAP flag here is needed to fix ugly bug with Windows 10
    // when window is resized and DXGI swap chain uses FLIP presentation model
    // DO NOT use it if you choose to use non-FLIP presentation model
    // read about the bug here: https://stackoverflow.com/q/63096226 and here: https://stackoverflow.com/q/53000291
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // uncomment in case you want fixed size window
    //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    //RECT rect = { 0, 0, 1280, 720 };
    //AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    //width = rect.right - rect.left;
    //height = rect.bottom - rect.top;

    // create window
    HWND window = CreateWindowExW(
        exstyle, wc.lpszClassName, L"D3D11 Window", style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, wc.hInstance, NULL);
    Assert(window && "Failed to create window");

    HRESULT hr;

    ID3D11Device* device;
    ID3D11DeviceContext* context;


	// RENDER LOOP

    for (;;)
    {
        // process all incoming Windows messages
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        // get current size for window client area
        RECT rect;
        GetClientRect(window, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
		
		// RENDERING (DX)

        // resize swap chain if needed
        if (rtView == NULL || width != currentWidth || height != currentHeight)
        {
            if (rtView)
            {
                // release old swap chain buffers
                context->ClearState();
                rtView->Release();
                dsView->Release();
                rtView = NULL;
            }

            // resize to new size for non-zero size
            if (width != 0 && height != 0)
            {
                hr = swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D11Texture2D* backbuffer;
                swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbuffer);
                device->CreateRenderTargetView((ID3D11Resource*)backbuffer, NULL, &rtView);
                backbuffer->Release();

                D3D11_TEXTURE2D_DESC depthDesc = 
                {
                    .Width = (uint32_t)width,
                    .Height = (uint32_t)height,
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_D32_FLOAT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
                    .SampleDesc = { 1, 0 },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                };

                // create new depth stencil texture & DepthStencil view
                ID3D11Texture2D* depth;
                device->CreateTexture2D(&depthDesc, NULL, &depth);
                device->CreateDepthStencilView((ID3D11Resource*)depth, NULL, &dsView);
                depth->Release();
            }

            currentWidth = width;
            currentHeight = height;
        }

        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
        if (rtView)
        {
            LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
            c1 = c2;

            // output viewport covering all client area of window
            D3D11_VIEWPORT viewport =
            {
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = (FLOAT)width,
                .Height = (FLOAT)height,
                .MinDepth = 0,
                .MaxDepth = 1,
            };

            // clear screen
            FLOAT color[] = { 0.392f, 0.584f, 0.929f, 1.f };
            context->ClearRenderTargetView(rtView, color);
            context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

            // setup 4x4c rotation matrix in uniform
            {
                angle += delta * 0.2f * (float)M_PI / 20.0f; // full rotation in 20 seconds
                angle = fmodf(angle, 2.0f * (float)M_PI);

                float aspect = (float)height / width;
                float matrix[16] =
                {
                    cosf(angle) * aspect, -sinf(angle), 0, 0,
                    sinf(angle) * aspect,  cosf(angle), 0, 0,
                                       0,            0, 0, 0,
                                       0,            0, 0, 1,
                };

                D3D11_MAPPED_SUBRESOURCE mapped;
				
                context->Map((ID3D11Resource*)ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                memcpy(mapped.pData, matrix, sizeof(matrix));
                context->Unmap((ID3D11Resource*)ubuffer, 0);
            }

            // Input Assembler
            context->IASetInputLayout(layout);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            UINT stride = sizeof(struct Vertex);
            UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &vbuffer, &stride, &offset);

            // Vertex Shader
            context->VSSetConstantBuffers(0, 1, &ubuffer);
            context->VSSetShader(vshader, NULL, 0);

            // Rasterizer Stage
            context->RSSetViewports(1, &viewport);
            context->RSSetState(rasterizerState);

            // Pixel Shader
            context->PSSetSamplers(0, 1, &sampler);
            context->PSSetShaderResources(0, 1, &textureView);
            context->PSSetShader(pshader, NULL, 0);

            // Output Merger
            context->OMSetBlendState(blendState, NULL, ~0U);
            context->OMSetDepthStencilState(depthState, 0);
            context->OMSetRenderTargets(1, &rtView, dsView);

            // draw 3 vertices
            context->Draw(6, 0);
			
			// IMGUI RENDER
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			
			if(ImGui::Begin("test")){
				ImGui::Text("FPS : %f", 1.0f/delta);
				if(ImGui::Button("button")){
					ImGui::Text("pressed");
				}
				
			} ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        // change to FALSE to disable vsync
        BOOL vsync = TRUE;
        hr = swapChain->Present(vsync ? 1 : 0, 0);
        if (hr == DXGI_STATUS_OCCLUDED)
        {
            // window is minimized, cannot vsync - instead sleep a bit
            if (vsync)
            {
                Sleep(10);
            }
        }
        else if (FAILED(hr))
        {
            FatalError("Failed to present swap chain! Device lost?");
        }
    }
	return 1;
}