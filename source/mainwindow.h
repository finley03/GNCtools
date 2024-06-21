#pragma once

//#include "gui.h"
#include "windowmanager.h"

namespace MainWindow {
	WindowManager::Window* Create(int width, int height, const wchar_t* title);

	//void Init();
}

//class MainWindow {
//private:
//	// UI elements
//	GUI::TextButton button1{ "Button 1" };
//	GUI::TextButton button2{ "Button 2" };
//
//	void UserInterface();
//
//public:
//	WindowManager::Window* Create(int width, int height);
//	LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//	void WindowMain(WindowManager::Window& window);
//};