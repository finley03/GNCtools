#include "mainwindow.h"

#include <iostream>
#include <memory>
#include <filesystem>

#include <glad/glad.h>

#include "files.h"
#include "gui.h"

namespace MainWindow {
	static bool begin = false;

	std::unique_ptr<GUI::GUI> gui;
	//std::unique_ptr<GUI::TextButton> button1;
	//std::unique_ptr<GUI::TextButton> button2;
	unsigned int fontHandle;

	void UserInterface() {
		static GUI::TextButton button1("Button 1");
		static GUI::TextButton button2("Button 2");

		gui->beginRender();

		//static int num = 2;
		//button2.setText(std::format("Button {}", num++));

		if (gui->renderTextButton(button1)) {
			std::cout << "button 1 pressed\n";
		}
		if (gui->renderTextButton(button2)) {
			std::cout << "button 2 pressed\n";
		}

		gui->endRender();
	}

	void WindowMain(WindowManager::Window& window) {





		gui = std::make_unique<GUI::GUI>(window);
		gui->newFont(fontHandle, "C:/Windows/Fonts/Segoeui.ttf", 14);

		//style = GUI::initDefaultStyle();
		
		//button1 = std::make_unique<GUI::TextButton>("Button 1");
		//button2 = std::make_unique<GUI::TextButton>("Button 2");

		/*gui->RenderTextButton(*button);*/

		//gui->setCursorMode(GUI::CursorMode::AdvanceHorizontal);




		begin = true;


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

		
		
		//button1.reset();
		//button2.reset();
		gui.reset();
	}

	// WndProc
	LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		WindowManager::Window* window;

		LRESULT wndProcHandlerReturn;
		if (wndProcHandlerReturn = WindowManager::HandleWndProc(hWnd, msg, wParam, lParam, window)) return wndProcHandlerReturn;
		if (wndProcHandlerReturn = gui->handleWndProc(hWnd, msg, wParam, lParam)) return wndProcHandlerReturn;

		switch (msg) {

		case WM_SIZE:
			if (begin) {
				//screen->setViewport(LOWORD(lParam), HIWORD(lParam));
				//testCamera->setViewport(window->getActualWidth(), window->getActualHeight());
				//testCamera->setScale(window->getNominalHeight());
				//testCamera->setPosition(window->getNominalWidth() / 2, window->getNominalHeight() / 2);

				//screen->clear();
			}
			return 0;
			
		case WM_PAINT:
			if (begin) {
				Graphics2D::Color clearColor(0.1f, 0.1f, 0.1f, 1.0f);
				//screen->setClearColor(clearColor);
				////screen->clear();
				//screen->clearRect(*buttonRect, clearColor);
				//Graphics2D::drawElement(*testElement, *testShader, *testCamera);
				//Graphics2D::drawTextElement(*textElement, *testFont, *textShader, *testCamera);
				//screen->swap();

				//if (count == 1000) gui->invalidateScreen();
				/*gui->beginRender();*/
				
				//if (gui->renderTextButton(*button1)) {
				//	std::cout << "button 1 pressed\n";
				//}
				//if (gui->renderTextButton(*button2)) {
				//	std::cout << "button 2 pressed\n";
				//}
				//if (count < 1000) {
				//	gui->renderTextButton(*button);
				//	gui->renderTextButton(*button);
				//}
				//gui->endRender();
				std::cout << "paint\n";

				UserInterface();

				if (gui->wantSwapBuffers()) {
					wglMakeCurrent(window->hdc, window->GLctx);
					SwapBuffers(window->hdc);
				}
				ValidateRect(window->hWnd, NULL);
			}
			//ValidateRect(window->hWnd, NULL);
			return 0;
			//break;

		//case WM_KEYDOWN:
		//	return 0;

		default:
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
	}

	WindowManager::Window* Create(int width, int height) {
		return CreateWindowThread(L"GNCtools", width, height, WindowMain, MainWindowProc);
	}
}