/*  ----------------------------------- INFOS
	Entry point of the program. Should be the entry point of the compiler too.
	
*/

// DEPENDENCIES : types.h, render.h, platform.h, file.h

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

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")


// macros
#define STR2(x) #x
#define STR(x) STR2(x)
#define internal static
#define local_persist static
#define global_variable static

// my stuff 
#include "render.h"
#include "platform.h"
#include "types.h"

enum GameState { menu, pause, game };

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	HRESULT hr;
	
	// init window handle (and size)
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
	HWND window = PLATFORM_CREATE_WINDOW(instance, width, height);

    
	// Init D3D11 + context
	render_context rContext;
	hr = RENDER_INIT_DX(window, &rContext);


    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    LARGE_INTEGER freq, c1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c1);

    float angle = 0;
    DWORD currentWidth = 0;
    DWORD currentHeight = 0;
	
	IMGUI_INIT(window, rContext);
	
	
	game_camera Camera = {0};
	
	
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
		RENDER_RESIZE_SWAP_CHAIN(window, currentWidth, currentHeight, &rContext);

        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
        if (rContext.rtView)
        {
            LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
            c1 = c2;

            // output viewport covering all client area of window


            // clear screen
            FLOAT color[] = { 0.392f, 0.584f, 0.929f, 1.f };
            rContext.context->ClearRenderTargetView(rContext.rtView, color);
            rContext.context->ClearDepthStencilView(rContext.dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

			
			// ---------------------------- add stuff to render
			quad_mesh quad1 = {
				.x = 0,
				.y = 0,
				.size = .5f,
				.orientation = 0,
			};
			
			RENDER_EXEC_PIPELINE(&rContext, &viewport);

            // draw vertices
            rContext.context->Draw(rContext.VertexCount, 0);
			
			// IMGUI RENDER
			IMGUI_RENDER();
			
			if(ImGui::Begin("test")){
				ImGui::Text("FPS : %f", 1.0f/delta);
				if(ImGui::Button("button")){
					ImGui::Text("pressed");
					RENDER_QUEUE_Quad(&quad1, &rContext);
					RENDER_UPLOAD_DYNAMIC_VertexQueue(&rContext);
				}
				ImGui::Text("Vertex count : %d", rContext.VertexCount);
				
			} ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        // change to FALSE to disable vsync
        BOOL vsync = TRUE;
        hr = rContext.swapChain->Present(vsync ? 1 : 0, 0);
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
