/*  ----------------------------------- INFOS
	This header file contains functions and structs related to rendering IMGUI components and UI in general.
	
*/

#ifndef _UIH_
#define _UIH_

// utils
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

// custom
#include "dx11.h"

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

#endif /* _UIH_ */