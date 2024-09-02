/*  ----------------------------------- INFOS
	Entry point of the program. Should be the entry point of the compiler too.
	
*/

// DEPENDENCIES : types.h, render.h, platform.h, file.h

// defines
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPLEMENTATION
#define RAYMATH_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

// assert
#define AssertHR(hr) Assert(SUCCEEDED(hr))


// macros
#define STR2(x) #x
#define STR(x) STR2(x)
#define internal static
#define local_persist static
#define global_variable static

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#include <windows.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

// libraries
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "raymath.h"
#include "stb_image.h"

// my stuff 
#include "types.h"
#include "game/location.h"
#include "render/ui.h"
#include "render/dx11.h"
#include "game/game.h"
#include "platform/platform.h"

enum GameState { menu, pause, game };

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	HRESULT hr;
	ui32 platformClockSpeed = platform_get_clock_speed();
	
	f32 time = platform_get_time(platformClockSpeed);
	
	// init window handle (and size)
    ui32 width = CW_USEDEFAULT;
    ui32 height = CW_USEDEFAULT;
	HWND window = PLATFORM_CREATE_WINDOW(instance, width, height);

    
	game_camera camera = {0};
	
	// Init D3D11 + context
	render_context rContext = {0};
	hr = RENDER_INIT_DX(window, &rContext, &camera);


    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    LARGE_INTEGER freq, c1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c1);
	
	IMGUI_INIT(window, rContext);
	
	viewport_size windowSize = {
		.height = 0,
		.width = 0,
	};
	
	// playfield aspect ratio is 4:3
	viewport_size playfield {
		.height = 384,
		.width = 512,
	};
	
	int clockFrequency;
	
	//  ------------------------------------------- frame loop

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
		
		// RENDERING (DX)

        // resize swap chain if needed
		RENDER_RESIZE_SWAP_CHAIN(window, &windowSize, &rContext);
		

        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
        if (rContext.rtView)
        {
			// resize the camera
			viewport_size vp = platform_getWindowSize(window);
			render_upload_camera_uBuffer(&rContext, &camera, vp);
			
            LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
            c1 = c2;

            // output viewport covering all client area of window


            // clear screen
            FLOAT color[] = { 0.f, 0.f, 0.f, 1.f };
            rContext.context->ClearRenderTargetView(rContext.rtView, color);
            rContext.context->ClearDepthStencilView(rContext.dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

			RENDER_INIT_PIPELINE(&rContext, &windowSize);
			
			// ---------------------------- add stuff to render
			quad_mesh quad1 = {
				.x = 0,
				.y = 0,
				.width = 128,
				.height = 128,
			};
			
            // draw vertices
            rContext.context->Draw(rContext.vCount, 0);
			
			// IMGUI RENDER
			IMGUI_RENDER();
			
			if(ImGui::Begin("test")){
				ImGui::Text("FPS : %f", 1.0f/delta);
				if(ImGui::Button("button")){
					ImGui::Text("pressed");
					render_queue_quad(&quad1, &rContext);
					RENDER_UPLOAD_DYNAMIC_VertexQueue(&rContext);
				}
				ImGui::Text("Vertex count : %d", rContext.vCount);
				
			} ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			
		
        }

        // change to FALSE to disable vsync
        BOOL vsync = FALSE;
        hr = rContext.swapChain->Present(vsync ? 1 : 0, 0);
		
		hr = rContext.device->GetDeviceRemovedReason();
		
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
