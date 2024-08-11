/*  ----------------------------------- INFOS
    This header file contains most of the basic calls to Windows API.
    
*/

// LOCAL DEPENDENCIES : types.h

#ifndef _PLATFORMH_
#define _PLATFORMH_

#include <windows.h>
#include "../render/ui.h"
#include "../render/render.h"
#include "types.h"

//  ------------------------------------ TYPES

// todo: replace regular width and height by this:
struct screen_size {
    int width;
    int height;
};

struct viewport_size {
	ui32 height;
	ui32 width;
};

//  ------------------------------------ OS RELATED FUNCTIONS

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



HWND PLATFORM_CREATE_WINDOW(HINSTANCE instance, int width, int height) {
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
	
    return window;
}

viewport_size platform_getWindowSize(HWND window) {
	RECT rect;
	GetClientRect(window, &rect);
		
	viewport_size newWindowSize = {
	(ui32)rect.bottom - rect.top << 0,
	(ui32)rect.right - rect.left << 0,
	};
		
	return newWindowSize;
};

#endif /* _PLATFORMH_ */