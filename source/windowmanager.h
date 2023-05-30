#pragma once

#include <Windows.h>
#include <thread>

namespace WindowManager {

	class Window {
	private:
		// nominal width and height of client area AT 96 DPI
		float nominalWidth = 300.0f, nominalHeight = 300.0f;
		// actual width and height of client area in pixels
		int actualWidth = 300, actualHeight = 300;
		// window DPI
		unsigned int dpi = 96;

		int positionX = 0.0f, positionY = 0.0f;

		// calculate sizes after one has been updated
		void calculateNominalSize();
		void calculateActualSize();
	public:
		// thread of window
		std::thread thread;

		// window handle
		HWND hWnd;
		// client area device context
		HDC hdc;
		// opengl context
		HGLRC GLctx;

		// pointer assignable by user
		void* userPointer;

		// constructor
		Window();
		//Window(HWND hWnd, HDC hdc, HGLRC GLctx, int width, int height);

		// Set sizes and DPIs
		void setNominalSize(float width, float height);
		void setActualSize(int width, int height);
		float getNominalWidth();
		float getNominalHeight();
		int getActualWidth();
		int getActualHeight();
		void setDpi(int dpi);
		int getDpi();
		float getScale();
		void setPosition(int x, int y);
		void getPosition(int& x, int& y);
	};

	// Creates console for stdio
	// Returns false if operation is unsuccessful
	bool CreateConsole();

	// Releases attached console for stdio
	// Returns false if operation is unsuccessful
	bool ReleaseConsole();

	// Creates new window with its own thread
	Window* CreateWindowThread(const wchar_t* title, int width, int height, void (*windowFunction)(Window&), WNDPROC wndproc);

	// default handler for the WndProc function
	LRESULT HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Window*& window);
}