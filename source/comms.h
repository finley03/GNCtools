#pragma once

#include "serial.h"

namespace Comms {
	bool CommsLoop(Serial::Port& port);

	uint32_t GetGlobalHash(Serial::Port& port);

	void test(Serial::Port& port);
}