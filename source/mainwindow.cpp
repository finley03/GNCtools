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
extern bool quit;

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

	bool variableInput(Globals::GlobalVariable& variable) {
		const ImGuiInputTextFlags inputflags = ImGuiInputTextFlags_EnterReturnsTrue;

		char buffer[64];

		switch (variable.type) {
		case Globals::GlobalVariableTypes::I64:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_S64, & variable.value.i64, 0, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::U64:
			snprintf(buffer, sizeof(buffer), "Edit value (hex)##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_U64, &variable.value.u64, 0, 0, "%016X", inputflags);

		case Globals::GlobalVariableTypes::F64:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputDouble(buffer, &variable.value.f64, 0, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::I32:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputInt(buffer, &variable.value.i32, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::U32:
			snprintf(buffer, sizeof(buffer), "Edit value (hex)##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_U32, &variable.value.u32, 0, 0, "%08X", inputflags);

		case Globals::GlobalVariableTypes::FP32:
			ImGui::TextUnformatted("Editing FP32 values is currently unsupported");
			return false;

		case Globals::GlobalVariableTypes::F32:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputFloat(buffer, &variable.value.f32, 0, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::I16:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_S16, &variable.value.i16, 0, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::U16:
			snprintf(buffer, sizeof(buffer), "Edit value (hex)##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_U16, &variable.value.u16, 0, 0, "%04X", inputflags);

		case Globals::GlobalVariableTypes::I8:
			snprintf(buffer, sizeof(buffer), "Edit value##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_S8, &variable.value.i8, 0, 0, 0, inputflags);

		case Globals::GlobalVariableTypes::U8:
			snprintf(buffer, sizeof(buffer), "Edit value (hex)##%s", variable.name.c_str());
			return ImGui::InputScalar(buffer, ImGuiDataType_U8, &variable.value.u8, 0, 0, "%02X", inputflags);

		case Globals::GlobalVariableTypes::BOOL:
			snprintf(buffer, sizeof(buffer), "True or false##%s", variable.name.c_str());
			return ImGui::Checkbox(buffer, &variable.value.b);

		default:
			ImGui::TextUnformatted("<ERROR TYPE>");
			return false;
		}
		
	}

	void ShowGlobals(Globals::Globals* globals) {
		if (!globals) {
			ImGui::Text("Error: Global data structure not present.");
			return;
		}

		// Create table
		// Three columns

		const ImGuiTableFlags tableflags =
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersV |
			ImGuiTableFlags_ScrollY;

		ImGui::BeginChild("Globals Table Child", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0));

		static std::vector<bool> ID_list(globals->variables.size() + 1, false); // true if selected
		static std::vector<uint16_t> selected_IDs; // list of selected IDs.

		if (ImGui::BeginTable("Globals Table", 2, tableflags)) {
			ImGui::TableSetupColumn("Global Variable Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();

			for (auto it = globals->variables.begin(); it != globals->variables.end(); ++it) {
				char buffer[32];
				uint16_t id = it->first;
				Globals::GlobalVariable& variable = it->second;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				bool selected = ID_list.at(id);
				if (ImGui::Selectable(variable.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
					ImGuiIO& io = ImGui::GetIO();
					if (io.KeyCtrl) {
						ID_list.at(id) = !selected;
						auto location = std::find(selected_IDs.begin(), selected_IDs.end(), id);
						if (location == selected_IDs.end()) selected_IDs.push_back(id);
						else selected_IDs.erase(location);
					}
					else {
						std::fill(ID_list.begin(), ID_list.end(), false);
						ID_list.at(id) = true;
						selected_IDs.clear();
						selected_IDs.push_back(id);
					}
				}
				ImGui::TableSetColumnIndex(1);
				getVariableValueText(variable, buffer, sizeof(buffer));
				ImGui::Text(buffer);
			}


			ImGui::EndTable();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Globals Editor Child");

		//int selected_count = std::count(selected_IDs.begin(), selected_IDs.end(), true);
		int selected_count = selected_IDs.size();

		ImGui::Text("%d variables selected", selected_count);
		ImGui::SameLine();
		if (ImGui::Button("Select all")) {
			std::fill(ID_list.begin(), ID_list.end(), true);
			selected_IDs.clear();
			for (auto it = globals->variables.begin(); it != globals->variables.end(); ++it) {
				selected_IDs.push_back(it->first);
			}
		}

		if (selected_count) {
			if (ImGui::Button("Poll all values")) {
				globals_ptr->mutex.lock();
				globals_ptr->pollList(selected_IDs);
				globals_ptr->mutex.unlock();
			}

			ImGui::SameLine();
			if (ImGui::Button("Load default values for all")) {
				globals_ptr->mutex.lock();
				globals_ptr->loadList(selected_IDs);
				globals_ptr->pollList(selected_IDs);
				globals_ptr->mutex.unlock();
			}
		}

		for (auto it = selected_IDs.begin(); it != selected_IDs.end(); ++it) {
			char buffer[64];
			uint16_t id = *it;
			Globals::GlobalVariable& variable = globals->variables.at(id);

			ImGui::SeparatorText(variable.name.c_str());

			ImGui::Text("ID: %d", id);

			snprintf(buffer, sizeof(buffer), "Type: ");
			getVariableTypeText(variable, buffer + 6, sizeof(buffer) - 6);

			ImGui::TextUnformatted(buffer);

			snprintf(buffer, sizeof(buffer), "Poll current value##%s", variable.name.c_str());
			if (ImGui::Button(buffer)) {
				globals_ptr->mutex.lock();
				globals_ptr->pollList({ id });
				globals_ptr->mutex.unlock();
			}

			ImGui::SameLine();

			snprintf(buffer, sizeof(buffer), "Load default value from storage##%s", variable.name.c_str());
			if (ImGui::Button(buffer)) {
				globals_ptr->mutex.lock();
				globals_ptr->loadList({ id });
				globals_ptr->pollList({ id }); // Update values shown to the user
				globals_ptr->mutex.unlock();
			}

			ImGui::SameLine();

			snprintf(buffer, sizeof(buffer), "Save default value to storage##%s", variable.name.c_str());
			if (ImGui::Button(buffer)) {
				globals_ptr->mutex.lock();
				globals_ptr->setList({ id }); // ensure value saved is the one seen by the user, in case it has changed
				globals_ptr->saveList({ id });
				globals_ptr->mutex.unlock();
			}

			if (variableInput(variable)) {
				Globals::GlobalVariable cachedVar = variable;
				globals_ptr->mutex.lock();
				globals->variables[id] = cachedVar; // make sure variable doesn't change when mutex is blocking
				globals_ptr->setList({ id });
				globals_ptr->mutex.unlock();
			}
		}

		//if (ImGui::Button("Set var")) {
		//	globals_ptr->mutex.lock();
		//	globals_ptr->variables.at(7).value.i32 = 1000;
		//	globals_ptr->variables.at(8).value.i32 = 1001;
		//	globals_ptr->variables.at(9).value.i32 = 1002;
		//	globals_ptr->setList({ 7, 8, 9 });
		//	globals_ptr->mutex.unlock();
		//}
		//if (ImGui::Button("Load var")) {
		//	globals_ptr->mutex.lock();
		//	globals_ptr->loadList({ 7, 8, 9 });
		//	globals_ptr->mutex.unlock();
		//}

		ImGui::EndChild();


		//if (ImGui::BeginTable("Globals Table 1", 4, tableflags)) {
		//	ImGui::TableSetupColumn("Global Variable Name");
		//	ImGui::TableSetupColumn("Type");
		//	ImGui::TableSetupColumn("Value");
		//	ImGui::TableSetupColumn("ID");
		//	ImGui::TableHeadersRow();

		//	for (auto it = globals->variables.begin(); it != globals->variables.end(); ++it) {
		//		char buffer[32];

		//		Globals::GlobalVariable& variable = it->second;
		//		ImGui::TableNextRow();
		//		ImGui::TableSetColumnIndex(0);
		//		ImGui::Text(variable.name.c_str());
		//		ImGui::TableSetColumnIndex(1);
		//		getVariableTypeText(variable, buffer, sizeof(buffer));
		//		ImGui::Text(buffer);
		//		ImGui::TableSetColumnIndex(2);
		//		getVariableValueText(variable, buffer, sizeof(buffer));
		//		ImGui::Text(buffer);
		//		ImGui::TableSetColumnIndex(3);
		//		snprintf(buffer, sizeof(buffer), "%d", it->first);
		//		ImGui::Text(buffer);
		//	}

		//	

		//	ImGui::EndTable();
		//}
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

		switch (msg) {
		case WM_CLOSE:
			globals_ptr->close();
			quit = true;
			return 0;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	WindowManager::Window* Create(int width, int height, const wchar_t* title) {
		return CreateWindowThread(title, width, height, WindowMain, MainWindowProc);
	}

}