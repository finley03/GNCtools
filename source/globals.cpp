#include "globals.h"
#include "comms.h"
#include <gnclink.h>

#include <iostream>
#include <chrono>
#include <format>

namespace Globals {
	/*extern uint8_t rxpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	extern uint8_t txpacket[GNCLINK_PACKET_MAX_TOTAL_LENGTH];
	extern uint8_t rxframe[GNCLINK_FRAME_TOTAL_LENGTH];
	extern uint8_t txframe[GNCLINK_FRAME_TOTAL_LENGTH];*/

	size_t Globals::getVariableSize(const GlobalVariable& var) {
		switch (var.type) {
		case I64:
		case U64:
		case F64:
			return 8;

		case I32:
		case U32:
		case FP32:
		case F32:
			return 4;

		case I16:
		case U16:
			return 2;

		case I8:
		case U8:
		case BOOL:
			return 1;

		default:
			return 0;
		}
	}

	void Globals::setVarFromPointer(GlobalVariable& var, void* pointer) {
		switch (var.type) {
		case I64:
		case U64:
		case F64:
			var.value.u64 = *(uint64_t*)pointer;
			return;

		case I32:
		case U32:
		case FP32:
		case F32:
			var.value.u32 = *(uint32_t*)pointer;
			return;

		case I16:
		case U16:
			var.value.u16 = *(uint16_t*)pointer;
			return;

		case I8:
		case U8:
		case BOOL:
			var.value.u8 = *(uint8_t*)pointer;
			return;

		default:
			return;
		}
	}

	void Globals::pollAll() {
		uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(Comms::rxpacket);
		uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(Comms::txpacket);
		uint16_t* idList = (uint16_t*)(txpayload + 1);

		auto it = variables.begin();
		while (it != variables.end()) {
			int TXspaceLeft = GNCLINK_PACKET_MAX_PAYLOAD_LENGTH - 1;
			int RXspaceLeft = TXspaceLeft;

			auto it2 = it;

			int i, payloadSize = 1;
			for (i = 0; TXspaceLeft >= 2 && it2 != variables.end(); ++i, ++it2) {
				int variableSize = getVariableSize(it2->second);
				RXspaceLeft -= variableSize;
				if (RXspaceLeft < 0) break;
				idList[i] = it2->first;
				TXspaceLeft -= 2;
				payloadSize += 2;
			}

			*txpayload = (uint8_t)i;

			GNClink_Construct_Packet(Comms::txpacket, GNClink_PacketType_GetValueList, GNClink_PacketFlags_None, payloadSize);

			Comms::CommsLoop(*port);

			GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
			GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
			if (packetType != GNClink_PacketType_GetValueList || (packetFlags & GNClink_PacketFlags_Response) == 0) return;

			int index = 0;

			for (int j = 0; j <= i && it != variables.end(); ++j) {
				setVarFromPointer(it->second, rxpayload + index);
				index += getVariableSize(it->second);
				++it;
			}
		}
	}

	Globals::Globals(Serial::Port& port) {
		this->port = &port;

		valueCount = Comms::GetValueCount(port);

		for (int i = 0; i < valueCount; ++i) {
			GlobalVariable var;
			
			uint16_t id = i + 1;
			
			var.name = Comms::GetValueName(id, var.type, port);
			
			variables.insert({ id, var });

			//std::cout << name << "\n";
		}

		pollAll();


	}

}