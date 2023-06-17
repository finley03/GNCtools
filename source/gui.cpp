#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

#include "windowmanager.h"

//#include "gui.h"




namespace WindowManager {

	void Window::initImGui(void (*uiFunction)(WindowManager::Window&)) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = NULL; // disable imgui ini

		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplOpenGL3_Init();

		io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Segoeui.ttf", 18);

		this->uiFunction = uiFunction;

        ImGuiStyleForDpi(dpi);

		initializedImGui = true;
	}

	void Window::ImGuiStyleForDpi(int dpi) {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.60f;         
        style.WindowPadding = ImVec2(8, 8);  
        style.WindowRounding = 0.0f;         
        style.WindowBorderSize = 0.0f;       
        style.WindowMinSize = ImVec2(32, 32);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left; 
        style.ChildRounding = 0.0f;       
        style.ChildBorderSize = 1.0f;     
        style.PopupRounding = 0.0f;       
        style.PopupBorderSize = 1.0f;     
        style.FramePadding = ImVec2(4, 3);
        style.FrameRounding = 0.0f;       
        style.FrameBorderSize = 0.0f;     
        style.ItemSpacing = ImVec2(8, 4); 
        style.ItemInnerSpacing = ImVec2(4, 4); 
        style.CellPadding = ImVec2(4, 2);      
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = 21.0f;   
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 14.0f;   
        style.ScrollbarRounding = 9.0f;
        style.GrabMinSize = 12.0f;     
        style.GrabRounding = 0.0f;     
        style.LogSliderDeadzone = 4.0f;
        style.TabRounding = 4.0f;      
        style.TabBorderSize = 0.0f;    
        style.TabMinWidthForCloseButton = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
        style.SeparatorTextBorderSize = 3.0f;
        style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
        style.SeparatorTextPadding = ImVec2(20.0f, 3.f);
        style.DisplayWindowPadding = ImVec2(19, 19);
        style.DisplaySafeAreaPadding = ImVec2(3, 3);
        style.MouseCursorScale = 1.0f;
        style.AntiAliasedLines = true;
        style.AntiAliasedLinesUseTex = true;            
        style.AntiAliasedFill = true;
        style.CurveTessellationTol = 1.25f;
        style.CircleTessellationMaxError = 0.30f;

        style.ScaleAllSizes((float)dpi / 96);
	}

}