#pragma once

#include <vector>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <2dGraphicsLibrary.h>
#include "windowmanager.h"
#include "inputmanager.h"

namespace GUI {

	struct ButtonStyle {
		Graphics2D::Vector size;
		float cornerRadius;
		Graphics2D::Vector padding;
		float textHorizontalPadding;
		Graphics2D::Color defaultColor;
		Graphics2D::Color disabledColor;
		Graphics2D::Color hoveredColor;
		Graphics2D::Color pressedColor;
		Graphics2D::Color defaultTextColor;
		Graphics2D::Color disabledTextColor;
	};

	struct Style {
		ButtonStyle buttonStyle;

		Graphics2D::Color backgroundColor;
		int normalFontSize;
	};

	enum class TextButtonState {
		Default,
		Disabled,
		Hovered,
		Pressed
	};

	class TextButton {
	public:
		TextButtonState state;

		Style* style;

		Graphics2D::Rectangle boundingBox;

		std::string string;

		std::unique_ptr<Graphics2D::Rectangle> rect;
		std::unique_ptr<Graphics2D::Text> text;
		std::unique_ptr<Graphics2D::Element> rectElement;
		std::unique_ptr<Graphics2D::TextElement> textElement;

		bool initialized = false;
		bool rendered = false;

		TextButton(std::string text, Style* style = nullptr);
		void setText(std::string text);
		void initForStyle(Style* style, Graphics2D::FontFace& font);
	};


	enum class CursorMode {
		AdvanceVertical,
		AdvanceHorizontal
	};


	class GUI {
	private:
		WindowManager::Window* window;

		std::vector<std::shared_ptr<Graphics2D::FontFace>> fonts;

		unsigned int currentFont = 0;

		// Style
		Style defaultStyle;


		// Cursor specific variables
		Graphics2D::Vector cursorPosition; // current position of cursor
		CursorMode cursorMode = CursorMode::AdvanceVertical; // defines cursor increment mode

		// Screen specific variables
		//Graphics2D::Vector screenSize;
		//float screenDpi;
		Graphics2D::Screen screen; // screen (renderbuffer)
		bool screenStatus = false; // tracks screen init state
		bool screenValid = false; // true if screen has been rendered to
		bool renderOccured = false;

		// camera specific variables
		Graphics2D::Camera camera;

		// Shader specific variables
		std::unique_ptr<Graphics2D::Shader> meshShader;
		std::unique_ptr<Graphics2D::Shader> textShader;
		bool meshShaderStatus = false;
		bool textShaderStatus = false;

		// User input variables
		/*Graphics2D::Vector mousePosition;*/
		InputManager inputs;

		// private methods
		bool reinitFonts(); // reinitializes fonts

		void advanceCursor(const Graphics2D::Vector& boundingBox);

		//void initDefaultStyle();

		bool testRectIntersect(const Graphics2D::Vector& vec, const Graphics2D::Rectangle& rect);

	public:
		GUI(WindowManager::Window& window);
		~GUI();

		// Control functions
		void setCurrentFont(unsigned int handle);

		void beginRender();
		void endRender();
		bool wantSwapBuffers();

		void setCursor(Graphics2D::Vector position);
		Graphics2D::Vector getCursor();
		void setCursorMode(CursorMode mode);

		//void setScreenSize(Graphics2D::Vector size);
		//Graphics2D::Vector getScreenSize();
		//void setScreenDpi(float dpi);
		//float getScreenDpi();

		void invalidateScreen();

		void clearFonts();
		bool newFont(unsigned int& handle, const std::string& path, int sizepx);

		// Visual Elements
		bool renderTextButton(TextButton& button);


		LRESULT handleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};


	Style initDefaultStyle();
}