#pragma once

#include "windowmanager.h"

namespace AboutWindow {
	WindowManager::Window* Create(int width, int height, const wchar_t* title);
}