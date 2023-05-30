#pragma once

#include <2dGraphicsLibrary.h>

enum class MouseButton {
	Left,
	Middle,
	Right
};

enum class InputType {
	Mouse,
	Touch,
	//Pen
};

class InputManager {
private:
	const int nMouseButtons = 3;

	bool mouseButtonPosedge[3] = { false };
	bool mouseButtonNegedge[3] = { false };
	bool mouseButtonHeld[3] = { false };

	bool touchPosedge = false;
	bool touchNegedge = false;
	bool touchDown = false;

	bool touchMessageReceived = false; // set to true after touch message
	// if reportMousePosition is called and this is false the input type will be set to mouse.

	InputType inputType = InputType::Mouse;

	Graphics2D::Vector mousePosition;

public:
	void reportMouseButtonDown(MouseButton button);
	void reportMouseButtonUp(MouseButton button);
	bool getMousePosedge(MouseButton button);
	bool getMouseNegedge(MouseButton button);
	bool getMouseHeld(MouseButton button);

	void reportTouchDown();
	void reportTouchUp();
	bool getTouchPosedge();
	bool getTouchNegedge();
	bool getTouchDown();

	void reportMousePosition(Graphics2D::Vector position);
	void reportTouchPosition(Graphics2D::Vector position); // functionally the same as reportMousePosition except it counts as a touch message.
	Graphics2D::Vector getMousePosition();

	void setInputType(InputType type);
	InputType getInputType();
	//void setCursorVisibility(bool visible); // true if mouse input, false if touch input
	//bool getCursorVisibility();

	void endFrame();
};