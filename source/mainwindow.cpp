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

#include "globals.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern Globals::Globals* globals_ptr;

namespace MainWindow {
	void getVariableValueText(Globals::GlobalVariable variable, char* buffer, int length) {
		switch (variable.type) {
		case Globals::GlobalVariableTypes::I64:
			snprintf(buffer, length, "%d", variable.value.i64);
			break;

		case Globals::GlobalVariableTypes::U64:
			snprintf(buffer, length, "0x%016X", variable.value.u64);
			break;

		case Globals::GlobalVariableTypes::F64:
			snprintf(buffer, length, "%f", variable.value.f64);
			break;

		case Globals::GlobalVariableTypes::I32:
			snprintf(buffer, length, "%d", variable.value.i32);
			break;

		case Globals::GlobalVariableTypes::U32:
			snprintf(buffer, length, "0x%08X", variable.value.u32);
			break;

		case Globals::GlobalVariableTypes::FP32:
			snprintf(buffer, length, "%d", variable.value.i32);
			break;

		case Globals::GlobalVariableTypes::F32:
			snprintf(buffer, length, "%f", variable.value.f32);
			break;

		case Globals::GlobalVariableTypes::I16:
			snprintf(buffer, length, "%d", variable.value.i16);
			break;

		case Globals::GlobalVariableTypes::U16:
			snprintf(buffer, length, "0x%04X", variable.value.u16);
			break;

		case Globals::GlobalVariableTypes::I8:
			snprintf(buffer, length, "%d", variable.value.i8);
			break;

		case Globals::GlobalVariableTypes::U8:
			snprintf(buffer, length, "0x%02X", variable.value.u8);
			break;

		case Globals::GlobalVariableTypes::BOOL:
			snprintf(buffer, length, variable.value.b ? "TRUE" : "FALSE");
			break;

		default:
			snprintf(buffer, length, "<ERROR TYPE>");
		}
	}

	void getVariableTypeText(Globals::GlobalVariable variable, char* buffer, int length) {
		switch (variable.type) {
		case Globals::GlobalVariableTypes::I64:
			snprintf(buffer, length, "64 bit signed integer");
			break;

		case Globals::GlobalVariableTypes::U64:
			snprintf(buffer, length, "64 bit unsigned integer");
			break;

		case Globals::GlobalVariableTypes::F64:
			snprintf(buffer, length, "Double precision floating point");
			break;

		case Globals::GlobalVariableTypes::I32:
			snprintf(buffer, length, "32 bit signed integer");
			break;

		case Globals::GlobalVariableTypes::U32:
			snprintf(buffer, length, "32 bit unsigned integer");
			break;

		case Globals::GlobalVariableTypes::FP32:
			snprintf(buffer, length, "32 bit fixed point");
			break;

		case Globals::GlobalVariableTypes::F32:
			snprintf(buffer, length, "Single precision floating point");
			break;

		case Globals::GlobalVariableTypes::I16:
			snprintf(buffer, length, "16 bit signed integer");
			break;

		case Globals::GlobalVariableTypes::U16:
			snprintf(buffer, length, "16 bit unsigned integer");
			break;

		case Globals::GlobalVariableTypes::I8:
			snprintf(buffer, length, "8 bit signed integer");
			break;

		case Globals::GlobalVariableTypes::U8:
			snprintf(buffer, length, "8 bit unsigned integer");
			break;

		case Globals::GlobalVariableTypes::BOOL:
			snprintf(buffer, length, "Boolean");
			break;

		default:
			snprintf(buffer, length, "<ERROR TYPE>");
		}
	}

	void ShowGlobals(Globals::Globals* globals) {
		if (!globals) {
			ImGui::Text("Error: Global data structure not present.");
			return;
		}

		// Create table
		// Three columns

		static ImGuiTableFlags tableflags =
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersV;


		if (ImGui::BeginTable("Globals Table 1", 4, tableflags)) {
			ImGui::TableSetupColumn("Global Variable Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("Value");
			ImGui::TableSetupColumn("ID");
			ImGui::TableHeadersRow();

			for (auto it = globals->variables.begin(); it != globals->variables.end(); ++it) {
				char buffer[32];

				Globals::GlobalVariable& variable = it->second;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(variable.name.c_str());
				ImGui::TableSetColumnIndex(1);
				getVariableTypeText(variable, buffer, sizeof(buffer));
				ImGui::Text(buffer);
				ImGui::TableSetColumnIndex(2);
				getVariableValueText(variable, buffer, sizeof(buffer));
				ImGui::Text(buffer);
				ImGui::TableSetColumnIndex(3);
				snprintf(buffer, sizeof(buffer), "%d", it->first);
				ImGui::Text(buffer);
			}

			ImGui::EndTable();
		}
	}

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

		ImGui::Begin("Main Window", nullptr, windowflags);

		//ImGui::SetWindowFontScale((float)(window.getDpi()) / 96);

		if (globals_ptr) {
			ShowGlobals(globals_ptr);
			ImGui::Spacing();
			if (ImGui::Button("Set var")) {
				globals_ptr->mutex.lock();
				globals_ptr->variables.at(7).value.i32 = 1000;
				globals_ptr->variables.at(8).value.i32 = 1001;
				globals_ptr->variables.at(9).value.i32 = 1002;
				globals_ptr->setList({ 7, 8, 9 });
				globals_ptr->mutex.unlock();
			}
		}
		else {
			ImGui::Text("Please connect a device.");
			ImGui::ShowDemoWindow();
		}


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
	LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		WindowManager::Window* window;

		LRESULT wndProcHandlerReturn;
		if (wndProcHandlerReturn = WindowManager::HandleWndProc(hWnd, msg, wParam, lParam, window)) return wndProcHandlerReturn;

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	WindowManager::Window* Create(int width, int height, const wchar_t* title) {
		return CreateWindowThread(title, width, height, WindowMain, MainWindowProc);
	}

}