#include "gui.h"

#include <windowsx.h>

#include "files.h"

namespace GUI {

	TextButton::TextButton(std::string text, Style* style) :
		boundingBox({}, {})
	{
		this->string = text;
		this->style = style;

	}

	void TextButton::setText(std::string text) {
		string = text;
		initialized = false;
		rendered = false;
	}

	void TextButton::initForStyle(Style* style, Graphics2D::FontFace& font) {
		if (initialized) return;

		if (!style) return;

		Graphics2D::Vector textPosition(style->buttonStyle.padding.x + style->buttonStyle.textHorizontalPadding, style->buttonStyle.padding.y + style->buttonStyle.size.y * 0.5);
		text = std::make_unique<Graphics2D::Text>(string, textPosition);

		textElement = std::make_unique<Graphics2D::TextElement>(style->buttonStyle.defaultTextColor);
		Graphics2D::bufferText(*textElement, *text);

		Graphics2D::Vector buttonPosition = style->buttonStyle.padding;
		Graphics2D::Vector buttonSize;
		if (style->buttonStyle.size.x == 0)
			buttonSize = Graphics2D::Vector(font.calculateStringLength(textElement->text) + style->buttonStyle.textHorizontalPadding * 2, style->buttonStyle.size.y);
		else
			buttonSize = style->buttonStyle.size;

		rect = std::make_unique<Graphics2D::Rectangle>(buttonPosition, buttonSize, style->buttonStyle.cornerRadius);
		rectElement = std::make_unique<Graphics2D::Element>(style->buttonStyle.defaultColor);
		Graphics2D::bufferRectangle(*rectElement, *rect);

		boundingBox.size = buttonSize + buttonPosition * 2;

		initialized = true;
		state = TextButtonState::Default;
	}


	bool GUI::reinitFonts() {
		for (auto& font : fonts) {
			font->init(font->size, window->getDpi());
		}

		return true;
	}

	void GUI::advanceCursor(const Graphics2D::Vector& boundingBox) {
		Graphics2D::Vector advance;

		switch (cursorMode) {
		default:
		case CursorMode::AdvanceVertical:
			advance = Graphics2D::Vector(0.0f, boundingBox.y);
			break;

		case CursorMode::AdvanceHorizontal:
			advance = Graphics2D::Vector(boundingBox.x, 0.0f);
			break;
		}

		cursorPosition += advance;
	}

	bool GUI::testRectIntersect(const Graphics2D::Vector& vec, const Graphics2D::Rectangle& rect) {
		return vec.x >= rect.topLeft.x &&
			vec.x <= rect.topLeft.x + rect.size.x &&
			vec.y >= rect.topLeft.y &&
			vec.y <= rect.topLeft.y + rect.size.y;
	}


	GUI::GUI(WindowManager::Window& window) :
		screen(window.getActualWidth(), window.getActualHeight(), screenStatus),
		camera(window.getNominalWidth(), window.getNominalHeight())
	{
		this->window = &window;

		// initialize shaders
		std::string meshVertexShader, meshFragmentShader, textVertexShader, textFragmentShader;
		readFileToString("shaders/renderer2d.vert", meshVertexShader);
		readFileToString("shaders/renderer2d.frag", meshFragmentShader);
		readFileToString("shaders/renderer2dBitmap.vert", textVertexShader);
		readFileToString("shaders/renderer2dBitmap.frag", textFragmentShader);
		meshShader = std::make_unique<Graphics2D::Shader>(meshVertexShader, meshFragmentShader, meshShaderStatus);
		textShader = std::make_unique<Graphics2D::Shader>(textVertexShader, textFragmentShader, textShaderStatus);

		// init camera
		camera.setScale(window.getNominalHeight());
		camera.setPosition(window.getNominalWidth() / 2, window.getNominalHeight() / 2);

		// set variables
		//screenSize = Graphics2D::Vector(width, height);
		//screenDpi = dpi;

		defaultStyle = initDefaultStyle();
	}

	GUI::~GUI() {

	}


	void GUI::setCurrentFont(unsigned int handle) {
		currentFont = handle;
	}


	void GUI::beginRender() {
		screen.setClearColor(defaultStyle.backgroundColor);

		// only clear entire screen if entire screen must be rendered
		if (!screenValid) screen.clear();

		cursorPosition = Graphics2D::Vector();

		renderOccured = false;
	}

	void GUI::endRender() {
		screenValid = true;

		inputs.endFrame();

		if (renderOccured)
			screen.swap();
	}

	bool GUI::wantSwapBuffers() {
		return renderOccured;
	}


	void GUI::setCursor(Graphics2D::Vector position) {
		cursorPosition = position;
	}

	Graphics2D::Vector GUI::getCursor() {
		return cursorPosition;
	}

	void GUI::setCursorMode(CursorMode mode) {
		cursorMode = mode;
	}




	void GUI::invalidateScreen() {
		screenValid = false;
	}


	void GUI::clearFonts() {
		fonts.clear();
		fonts.shrink_to_fit();
	}

	bool GUI::newFont(unsigned int& handle, const std::string& path, int sizepx) {
		handle = fonts.size();
		bool status = false;
		fonts.push_back(std::make_unique<Graphics2D::FontFace>(path, sizepx, window->getDpi(), status));
		return status;
	}

	
	bool GUI::renderTextButton(TextButton& button) {
		Style* style = &defaultStyle;
		if (button.style) style = button.style;

		button.initForStyle(style, *fonts[currentFont]);

		button.boundingBox.topLeft = cursorPosition;

		button.rectElement->position = cursorPosition;
		button.textElement->position = cursorPosition;

		TextButtonState lastState = button.state;

		bool mouseIntersectsRect = testRectIntersect(inputs.getMousePosition() / window->getScale() - cursorPosition, *button.rect);

		if (inputs.getInputType() == InputType::Touch) {
			switch (button.state) {
			case TextButtonState::Default:
				if (mouseIntersectsRect) {
					if (inputs.getTouchPosedge())
						button.state = TextButtonState::Pressed;
				}
				break;

			case TextButtonState::Disabled:
				break;

			case TextButtonState::Hovered:
				button.state = TextButtonState::Default;
				break;

			case TextButtonState::Pressed:
				if (inputs.getTouchNegedge())
					button.state = TextButtonState::Default;
				break;
			}
		}

		else {
			switch (button.state) {
			case TextButtonState::Default:
				if (mouseIntersectsRect) {
					if (inputs.getMousePosedge(MouseButton::Left))
						button.state = TextButtonState::Pressed;
					else if (!inputs.getMouseHeld(MouseButton::Left))
						button.state = TextButtonState::Hovered;
				}
				break;

			case TextButtonState::Disabled:
				break;

			case TextButtonState::Hovered:
				if (mouseIntersectsRect) {
					if (inputs.getMousePosedge(MouseButton::Left))
						button.state = TextButtonState::Pressed;
				}
				else
					button.state = TextButtonState::Default;
				break;

			case TextButtonState::Pressed:
				if (inputs.getMouseNegedge(MouseButton::Left) || !inputs.getMouseHeld(MouseButton::Left))
					button.state = (mouseIntersectsRect) ? TextButtonState::Hovered : TextButtonState::Default;
				break;
			}
		}

		if (lastState != button.state) { // run if state changes
			button.rendered = false;
			
			// set button colors
			switch (button.state) {
			case TextButtonState::Default:
				button.rectElement->color = style->buttonStyle.defaultColor;
				button.textElement->color = style->buttonStyle.defaultTextColor;
				break;

			case TextButtonState::Disabled:
				button.rectElement->color = style->buttonStyle.disabledColor;
				button.textElement->color = style->buttonStyle.disabledTextColor;
				break;

			case TextButtonState::Hovered:
				button.rectElement->color = style->buttonStyle.hoveredColor;
				button.textElement->color = style->buttonStyle.defaultTextColor;
				break;

			case TextButtonState::Pressed:
				button.rectElement->color = style->buttonStyle.pressedColor;
				button.textElement->color = style->buttonStyle.defaultTextColor;
				break;
			}
		}


		if (!button.rendered || !screenValid) {
			if (screenValid) {
				Graphics2D::Vector positionOnScreen(button.rect->topLeft.x + cursorPosition.x, window->getNominalHeight() - button.rect->topLeft.y - button.rect->size.y - cursorPosition.y);
				Graphics2D::Vector sizeOnScreen = button.rect->size;
				Graphics2D::Rectangle rectOnScreen(positionOnScreen * window->getScale(), sizeOnScreen * window->getScale());
				screen.clearRect(rectOnScreen, style->backgroundColor);
			}

			Graphics2D::drawElement(*(button.rectElement), *meshShader, camera);
			Graphics2D::drawTextElement(*(button.textElement), *(fonts[currentFont]), *textShader, camera);

			button.rendered = true;
			renderOccured = true;
		}

		advanceCursor(button.boundingBox.size);

		return mouseIntersectsRect && inputs.getMousePosedge(MouseButton::Left);
	}






	Style initDefaultStyle() {
		Style style;

		style.backgroundColor = Graphics2D::Color(0.1f, 0.1f, 0.1f, 1.0f);
		style.normalFontSize = 14;

		// Button style
		style.buttonStyle.size = Graphics2D::Vector(0.0f, 30.0f);
		style.buttonStyle.cornerRadius = 5;
		style.buttonStyle.padding = Graphics2D::Vector(6, 4);
		style.buttonStyle.textHorizontalPadding = 10;
		style.buttonStyle.defaultColor = Graphics2D::Color(0.2f, 0.2f, 0.2f, 1.0f);
		style.buttonStyle.disabledColor = Graphics2D::Color(0.2f, 0.2f, 0.2f, 0.6f);
		style.buttonStyle.hoveredColor = Graphics2D::Color(0.3f, 0.3f, 0.3f, 1.0f);
		style.buttonStyle.pressedColor = Graphics2D::Color(0.6f, 0.4f, 0.4f, 1.0f);
		style.buttonStyle.defaultTextColor = Graphics2D::Color(1.0f, 1.0f, 1.0f, 1.0f);
		style.buttonStyle.disabledTextColor = Graphics2D::Color(0.4f, 0.4f, 0.4f, 0.6f);

		return style;
	}



	LRESULT GUI::handleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (!this) return 0;

		switch (msg) {
		case WM_SIZE:
			//setScreenSize(Graphics2D::Vector(LOWORD(lParam), HIWORD(lParam)));
			screen.setViewport(window->getActualWidth(), window->getActualHeight());
			camera.setViewport(window->getNominalWidth(), window->getNominalHeight());
			camera.setScale(window->getNominalHeight());
			camera.setPosition(window->getNominalWidth() / 2, window->getNominalHeight() / 2);
			invalidateScreen();

			return 0;

		case WM_DPICHANGED:
			reinitFonts();
			invalidateScreen();

			return 0;


		case WM_MOUSEMOVE:
		{
			CURSORINFO cursor;
			cursor.cbSize = sizeof(CURSORINFO);
			if (GetCursorInfo(&cursor) && (cursor.flags & CURSOR_SUPPRESSED) != 0)
				inputs.setInputType(InputType::Touch);
			else
				inputs.setInputType(InputType::Mouse);

			inputs.reportMousePosition(Graphics2D::Vector(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			InvalidateRect(window->hWnd, NULL, false);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			//if (inputs.getInputType() == InputType::Mouse) {
				inputs.reportMouseButtonDown(MouseButton::Left);
				InvalidateRect(window->hWnd, NULL, false);
			//}

			return 0;
		}

		case WM_MBUTTONDOWN:
			inputs.reportMouseButtonDown(MouseButton::Middle);
			InvalidateRect(window->hWnd, NULL, false);
			return 0;

		case WM_RBUTTONDOWN:
			inputs.reportMouseButtonDown(MouseButton::Right);
			InvalidateRect(window->hWnd, NULL, false);
			return 0;

		case WM_LBUTTONUP:
			//if (inputs.getInputType() == InputType::Mouse) {
				inputs.reportMouseButtonUp(MouseButton::Left);
				InvalidateRect(window->hWnd, NULL, false);
			//}
			return 0;

		case WM_MBUTTONUP:
			inputs.reportMouseButtonUp(MouseButton::Middle);
			InvalidateRect(window->hWnd, NULL, false);
			return 0;

		case WM_RBUTTONUP:
			inputs.reportMouseButtonUp(MouseButton::Right);
			InvalidateRect(window->hWnd, NULL, false);
			return 0;

		case WM_TOUCH:
		{
			unsigned int nTouchInputs = LOWORD(wParam);
			const unsigned int maxNTouchInputs = 4;
			if (nTouchInputs > maxNTouchInputs) nTouchInputs = maxNTouchInputs;
			TOUCHINPUT touchInputs[maxNTouchInputs];
			if (GetTouchInputInfo((HTOUCHINPUT)lParam, maxNTouchInputs, touchInputs, sizeof(TOUCHINPUT))) {
				for (int i = 0; i < nTouchInputs; ++i) {
					unsigned int flags = touchInputs[i].dwFlags;

					// skip all touches but primary touch
					if (!(flags & TOUCHEVENTF_PRIMARY)) continue;

					// set input type for input manager
					inputs.setInputType((flags & TOUCHEVENTF_PEN) ? InputType::Mouse : InputType::Touch);

					int clientPositionX, clientPositionY;
					window->getPosition(clientPositionX, clientPositionY);
					Graphics2D::Vector touchPosition(static_cast<float>(touchInputs[i].x) / 100 - clientPositionX, static_cast<float>(touchInputs[i].y) / 100 - clientPositionY);
					inputs.reportTouchPosition(touchPosition);

					if (flags & TOUCHEVENTF_DOWN) {
						inputs.reportTouchDown();
						SendMessage(window->hWnd, WM_PAINT, 0, 0);
					}
					if (flags & TOUCHEVENTF_UP) inputs.reportTouchUp();



				}

				CloseTouchInputHandle((HTOUCHINPUT)lParam);
			}
			return 0;
		}

		}

		return 0;
	}
}