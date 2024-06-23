#include "mainwindow.h"

#include <iostream>
#include <memory>
#include <filesystem>

#include <glad/glad.h>

#include "files.h"
//#include "gui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



namespace AboutWindow {
	static bool begin = false;

	void UserInterface(WindowManager::Window& window) {
		ImGui::SetCurrentContext(window.ImGuiCTX);
		// set constraints on window position and size
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		// minimum window height is smaller then height of menu therefore minimum menu height applies
		ImGui::SetNextWindowSize(ImVec2(window.getActualWidth(), window.getActualHeight()));

		// flags for window creation
		ImGuiWindowFlags windowflags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove |
			//ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("About Window", nullptr, windowflags);

		ImGui::Text("GNC2 Ground Station");
		ImGui::Text("2024 Finley Blaine");

		ImGui::End();
	}

	void WindowMain(WindowManager::Window& window) {

		window.initImGui(UserInterface);

		InvalidateRect(window.hWnd, NULL, false);

		MSG msg;
		int bRet;
		while ((bRet = GetMessage(&msg, window.hWnd, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
	}

	// WndProc
	LRESULT CALLBACK AboutWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		WindowManager::Window* window;

		LRESULT wndProcHandlerReturn;
		if (wndProcHandlerReturn = WindowManager::HandleWndProc(hWnd, msg, wParam, lParam, window)) return wndProcHandlerReturn;

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	WindowManager::Window* Create(int width, int height, const wchar_t* title) {
		return CreateWindowThread(title, width, height, WindowMain, AboutWindowProc);
	}

}