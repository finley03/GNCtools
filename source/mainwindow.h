#pragma once

#include "windowmanager.h"

namespace MainWindow {
	WindowManager::Window* Create(int width, int height, const wchar_t* title);
}