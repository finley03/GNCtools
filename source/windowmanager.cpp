#include "windowmanager.h"

#include <iostream>
#include <vector>
#include <string_view>
#include <format>

#include <Windows.h>
//#include <glad/glad.h>
#include <dwmapi.h>


#include <2dGraphicsLibrary.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

// global program variables
HINSTANCE _hInstance;
LPWSTR _lpCmdLine;
int _nCmdShow;

// declare main function
extern int main(std::vector<std::wstring_view> commandLineArguments);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// program entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	//// check windows version is 10 or above
	//if (!IsWindows10OrGreater())
	//	MessageBoxW(NULL, L"Sorry but this program does not support windows versions below 10.", L"Error", NULL);

	// store variables
	_hInstance = hInstance;
	_lpCmdLine = lpCmdLine;
	_nCmdShow = nCmdShow;

	// extract command line arguments
	int numArgs;
	LPWSTR* wargv = CommandLineToArgvW(lpCmdLine, &numArgs);

	std::vector<std::wstring_view> commandLineArguments(numArgs);

	for (int i = 0; i < numArgs; ++i) {
		commandLineArguments[i] = std::wstring_view(wargv[i]);
	}

	// indicate that program is fully dpi aware
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// call main
	return main(commandLineArguments);
}

namespace ImGui {
	bool WantRender() {
		//return true;

		ImGuiContext& g = *ImGui::GetCurrentContext();
		ImGuiIO& io = ImGui::GetIO();

		static int indices = 0;
		static unsigned int hoveredId = 0, activeId = 0;

		if (g.HoveredId != hoveredId || g.ActiveId != activeId || io.MetricsRenderIndices != indices) {
			hoveredId = g.HoveredId;
			activeId = g.ActiveId;
			indices = io.MetricsRenderIndices;

			return true;
		}

		return false;
	}
}

namespace WindowManager {

	void Window::calculateNominalSize() {
		nominalWidth = static_cast<float>(actualWidth * 96) / dpi;
		nominalHeight = static_cast<float>(actualHeight * 96) / dpi;
	}

	void Window::calculateActualSize() {
		actualWidth = nominalWidth * getScale();
		actualHeight = nominalHeight * getScale();
	}

	Window::Window() {
		hWnd = NULL;
		hdc = NULL;
		GLctx = NULL;

		setDpi(GetDpiForSystem());
	}

	void Window::setNominalSize(float width, float height) {
		nominalWidth = width;
		nominalHeight = height;

		calculateActualSize();

		renderNeeded = true;
	}

	void Window::setActualSize(int width, int height) {
		actualWidth = width;
		actualHeight = height;

		calculateNominalSize();

		screen->setViewport(width, height);

		renderNeeded = true;
	}

	float Window::getNominalWidth() {
		return nominalWidth;
	}

	float Window::getNominalHeight() {
		return nominalHeight;
	}

	int Window::getActualWidth() {
		return actualWidth;
	}

	int Window::getActualHeight() {
		return actualHeight;
	}

	void Window::setDpi(int dpi) {
		this->dpi = dpi;

		calculateNominalSize();

		renderNeeded = true;
	}

	int Window::getDpi() {
		return dpi;
	}

	float Window::getScale() {
		return static_cast<float>(dpi) / 96;
	}

	void Window::setPosition(int x, int y) {
		positionX = x;
		positionY = y;

		renderNeeded = true;
	}

	void Window::getPosition(int& x, int& y) {
		x = positionX;
		y = positionY;
	}


	void Window::setWantRender() {
		renderNeeded = true;
	}


	void Window::checkRender() {
		InvalidateRect(hWnd, NULL, false);
	}


	bool Window::wantRender(bool reset) {
		bool ret = renderNeeded;
		if (reset) renderNeeded = false;
		return ret;
	}



	// release attached stdio console
	bool ReleaseConsole() {
		bool status = true;

		FILE* fp;

		if (freopen_s(&fp, "NUL:", "r", stdin)) status = false;
		else setvbuf(stdin, NULL, _IONBF, 0);

		if (freopen_s(&fp, "NUL:", "r", stdout)) status = false;
		else setvbuf(stdout, NULL, _IONBF, 0);

		if (freopen_s(&fp, "NUL:", "r", stdout)) status = false;
		else setvbuf(stdout, NULL, _IONBF, 0);

		if (!FreeConsole()) status = false;

		return status;
	}

	// create attached console for stdio
	bool CreateConsole() {
		bool status = false;

		ReleaseConsole();

		if (AllocConsole()) {
			status = true;

			FILE* fp;

			if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE) {
				if (freopen_s(&fp, "CONIN$", "r", stdin)) status = false;
				else setvbuf(stdin, NULL, _IONBF, 0);
			}

			if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) {
				if (freopen_s(&fp, "CONOUT$", "w", stdout)) status = false;
				else setvbuf(stdout, NULL, _IONBF, 0);
			}

			if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
				if (freopen_s(&fp, "CONOUT$", "w", stderr)) status = false;
				else setvbuf(stderr, NULL, _IONBF, 0);
			}

			std::ios_base::sync_with_stdio(true);

			std::wcout.clear();
			std::cout.clear();
			std::wcerr.clear();
			std::cerr.clear();
			std::wcin.clear();
			std::cin.clear();
		}

		return status;
	}


	void HandleNCCreate(HWND hWnd, CREATESTRUCTW* cs) {
		Window* window = (Window*)cs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)window);

	}




	LRESULT HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Window*& window) {
		window = (Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		if (!window) {
			/* Due to a longstanding Windows bug, overlapped windows will receive a
			   WM_GETMINMAXINFO message before WM_NCCREATE. This is safe to ignore.
			   It doesn't need any special handling anyway. */
			if (msg == WM_NCCREATE)
				HandleNCCreate(hWnd, (CREATESTRUCTW*)lParam);
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}

		ImGui::SetCurrentContext(window->ImGuiCTX);
		LRESULT ImGuiWndProcReturn;
		if (ImGuiWndProcReturn = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return ImGuiWndProcReturn;

		switch (msg) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// if window size has changed
		case WM_SIZE:
			window->setActualSize(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_SIZING:
			return TRUE;

		case WM_MOVE:
			window->setPosition(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_DPICHANGED:
			window->setDpi(LOWORD(wParam));

			// Resize window to work around a DWM bug??
			RECT windowRect;
			GetWindowRect(window->hWnd, &windowRect);
			SetWindowPos(window->hWnd, NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left,
					windowRect.bottom - windowRect.top + 1, NULL);
			return 0;

		// Any window events that require a render pass should be listed here
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			InvalidateRect(window->hWnd, NULL, false);
			break;

		// Renders window
		case WM_PAINT:
			if (window->initializedImGui) {
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				window->uiFunction(*window);

				ImGui::Render();

				if (ImGui::WantRender() || window->wantRender(true)) {
					wglMakeCurrent(window->hdc, window->GLctx);

					window->screen->clear();
					ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
					window->screen->swap();

					SwapBuffers(window->hdc);
				}
				ValidateRect(window->hWnd, NULL);
			}
			return 0;

		}

		return 0;
	}

	//bool Init(const wchar_t* title, WNDPROC wndproc) {
	//	bool status;

	//	// create window class information
	//	WNDCLASSEXW wcex;
	//	// set size
	//	wcex.cbSize = sizeof(WNDCLASSEXW);
	//	// window style:
	//	// redraw if size adjust
	//	wcex.style = CS_HREDRAW | CS_VREDRAW;
	//	// pointer to WindowProc
	//	wcex.lpfnWndProc = wndproc;
	//	// 0 extra bytes allocated
	//	wcex.cbClsExtra = 0;
	//	wcex.cbWndExtra = 0;
	//	// handle to window instance
	//	wcex.hInstance = _hInstance;
	//	// use default icon
	//	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	//	// load cursor
	//	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//	// background
	//	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//	// no default manu
	//	wcex.lpszMenuName = NULL;
	//	// name
	//	wcex.lpszClassName = title; // L"GNCtools";
	//	// icon
	//	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	//	// register wcex class with windows
	//	if (!RegisterClassExW(&wcex)) {
	//		MessageBoxW(NULL, L"\"RegisterClassEx\" call failed.", title, NULL);
	//		status = false;
	//		return false;
	//	}

	//	return true;
	//}



	void CreateGLContext(Window& window) {
		PIXELFORMATDESCRIPTOR pfd = {
			// size of struct
			sizeof PIXELFORMATDESCRIPTOR,
			// version?
			1,
			// flags
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			// pixel buffer type
			PFD_TYPE_RGBA,
			// color bits
			32,
			// color bits and shifts (ignore)
			0, 0, 0, 0, 0, 0, 0, 0,
			// accum bits
			0,
			// accum bits and shifts
			0, 0, 0, 0,
			// number of depth buffer bits
			24,
			// number of stencil buffer bits
			8,
			// number of aux bits,
			0,
			// technically ignored
			PFD_MAIN_PLANE,
			// reserved
			0,
			// ignore (two ignored)
			0, 0, 0
		};

		int format = ChoosePixelFormat(window.hdc, &pfd);
		SetPixelFormat(window.hdc, format, &pfd);
		// create context
		window.GLctx = wglCreateContext(window.hdc);
		wglMakeCurrent(window.hdc, window.GLctx);
	}

	void WindowThreadInit(Window* window, const wchar_t* title, int width, int height, void (*windowFunction)(Window&), WNDPROC wndproc) {

		bool status;

		// create window class information
		WNDCLASSEXW wcex;
		// set size
		wcex.cbSize = sizeof(WNDCLASSEXW);
		// window style:
		// redraw if size adjust
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		// pointer to WindowProc
		wcex.lpfnWndProc = wndproc;
		// 0 extra bytes allocated
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		// handle to window instance
		wcex.hInstance = _hInstance;
		// use default icon
		wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
		// load cursor
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		// background
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		// no default manu
		wcex.lpszMenuName = NULL;
		// name
		wcex.lpszClassName = title; // L"GNCtools";
		// icon
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		// register wcex class with windows
		if (!RegisterClassExW(&wcex)) {
			MessageBoxW(NULL, L"\"RegisterClassEx\" call failed.", title, NULL);
			status = false;
			return;
		}

		//window->dpi = GetDpiForSystem();
		window->setNominalSize(width, height);

		// create Window
		window->hWnd = CreateWindowExW(
			0,
			title,
			title,
			WS_OVERLAPPEDWINDOW,
			300, 300,
			window->getActualWidth(), window->getActualHeight(),
			NULL,
			NULL,
			_hInstance,
			window);

		if (!window->hWnd) {
			MessageBoxW(NULL, L"\"CreateWindowEx\" call failed.", title, NULL);
			return;
		}

		// set window as touch capable
		RegisterTouchWindow(window->hWnd, TWF_FINETOUCH);

		// get device context
		window->hdc = GetDC(window->hWnd);

		COLORREF captionColor = 0x00202020;
		COLORREF textColor = 0x00FFFFFF;
		COLORREF borderColor = 0x00CF4868;
		DwmSetWindowAttribute(window->hWnd, DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, &captionColor, sizeof(COLORREF));
		DwmSetWindowAttribute(window->hWnd, DWMWINDOWATTRIBUTE::DWMWA_TEXT_COLOR, &textColor, sizeof(COLORREF));
		DwmSetWindowAttribute(window->hWnd, DWMWINDOWATTRIBUTE::DWMWA_BORDER_COLOR, &borderColor, sizeof(COLORREF));





		CreateGLContext(*window);

		//gladLoadGL();

		//glEnable(GL_DEPTH_TEST); // some opengl rendering settings
		//glDepthMask(GL_TRUE);
		//glDepthFunc(GL_LEQUAL);
		//glDisable(GL_CULL_FACE);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_BLEND);
		//glEnable(GL_MULTISAMPLE);
		//glFrontFace(GL_CCW);
		//glEnable(GL_SCISSOR_TEST);

		Graphics2D::init();

		bool screenStatus = false;
		window->screen = new Graphics2D::Screen(window->getActualWidth(), window->getActualHeight(), screenStatus);
		if (!screenStatus) {
			MessageBoxW(NULL, L"Failed to create window screen.", title, NULL);
			return;
		}

		RECT rect;
		GetClientRect(window->hWnd, &rect);
		window->setActualSize(rect.right - rect.left, rect.bottom - rect.top);

		window->setDpi(GetDpiForWindow(window->hWnd));

		// show created window
		ShowWindow(window->hWnd, _nCmdShow);
		UpdateWindow(window->hWnd);

		windowFunction(*window);

		//std::exit(0);
	}

	Window* CreateWindowThread(const wchar_t* title, int width, int height, void (*windowFunction)(Window&), WNDPROC wndproc) {
		// create window struct
		Window* window = new Window;

		window->thread = std::thread(WindowThreadInit, window, title, width, height, windowFunction, wndproc);

		return window;
	}
}