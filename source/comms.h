#pragma once

#include "serial.h"
#include <gnclink.h>

// Test communications by introducing random errors
//#define COMMS_TEST

namespace Comms {
	extern uint8_t rxpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	extern uint8_t txpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	extern uint8_t rxframe[GNCLINK_FRAME_TOTAL_LENGTH];
	extern uint8_t txframe[GNCLINK_FRAME_TOTAL_LENGTH];

	bool CommsLoop(Serial::Port& port);

	uint32_t GetGlobalHash(Serial::Port& port);
	uint16_t GetValueCount(Serial::Port& port);
	std::string GetValueName(uint16_t id, uint8_t& type, Serial::Port& port);

	//void test(Serial::Port& port);
}