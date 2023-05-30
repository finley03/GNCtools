#include "inputmanager.h"
#include <iostream>

void InputManager::reportMouseButtonDown(MouseButton button) {
	mouseButtonPosedge[static_cast<int>(button)] = true;
	mouseButtonHeld[static_cast<int>(button)] = true;
}

void InputManager::reportMouseButtonUp(MouseButton button) {
	mouseButtonNegedge[static_cast<int>(button)] = true;
	mouseButtonHeld[static_cast<int>(button)] = false;
}

bool InputManager::getMousePosedge(MouseButton button) {
	return mouseButtonPosedge[static_cast<int>(button)];
}

bool InputManager::getMouseNegedge(MouseButton button) {
	return mouseButtonNegedge[static_cast<int>(button)];
}

bool InputManager::getMouseHeld(MouseButton button) {
	return mouseButtonHeld[static_cast<int>(button)];
}


void InputManager::reportTouchDown() {
	std::cout << "touch down\n";
	touchPosedge = true;
	touchDown = true;
	touchMessageReceived = true;
}

void InputManager::reportTouchUp() {
	std::cout << "touch up\n";
	touchNegedge = true;
	touchDown = false;
	touchMessageReceived = true;
}

bool InputManager::getTouchPosedge() {
	return touchPosedge;
}

bool InputManager::getTouchNegedge() {
	return touchNegedge;
}

bool InputManager::getTouchDown() {
	return touchDown;
}


void InputManager::reportMousePosition(Graphics2D::Vector position) {
	if (inputType != InputType::Mouse) {
		if (touchMessageReceived) {
			touchMessageReceived = false;
			return;
		}
		setInputType(InputType::Mouse);
	}
	mousePosition = position;
}

void InputManager::reportTouchPosition(Graphics2D::Vector position) {
	mousePosition = position;
	touchMessageReceived = true;
}

Graphics2D::Vector InputManager::getMousePosition() {
	return mousePosition;
}



void InputManager::setInputType(InputType type) {
	//std::cout << (int)type << "\n";
	inputType = type;
}

InputType InputManager::getInputType() {
	return inputType;
}



void InputManager::endFrame() {
	for (int i = 0; i < nMouseButtons; ++i) {
		mouseButtonPosedge[i] = false;
		mouseButtonNegedge[i] = false;
	}

	touchPosedge = false;
	touchNegedge = false;
}