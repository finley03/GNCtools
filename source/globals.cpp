#include "globals.h"
#include "comms.h"
#include <gnclink.h>

#include <iostream>
#include <chrono>
#include <format>

namespace Globals {
	void Globals::close() {
		Serial::ClosePort(*port);
	}

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

	void Globals::setPointerFromVar(GlobalVariable& var, void* pointer) {
		switch (var.type) {
		case I64:
		case U64:
		case F64:
			*(uint64_t*)pointer = var.value.u64;
			return;

		case I32:
		case U32:
		case FP32:
		case F32:
			*(uint32_t*)pointer = var.value.u32;
			return;

		case I16:
		case U16:
			*(uint16_t*)pointer = var.value.u16;
			return;

		case I8:
		case U8:
		case BOOL:
			*(uint8_t*)pointer = var.value.u8;
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

			for (int j = 0; j < i && it != variables.end(); ++j) {
				setVarFromPointer(it->second, rxpayload + index);
				index += getVariableSize(it->second);
				++it;
			}
		}
	}

	void Globals::pollList(std::vector<uint16_t> list) {
		uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(Comms::rxpacket);
		uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(Comms::txpacket);
		uint16_t* idList = (uint16_t*)(txpayload + 1);

		auto it = list.begin();
		while (it != list.end()) {
			int TXspaceLeft = GNCLINK_PACKET_MAX_PAYLOAD_LENGTH - 1;
			int RXspaceLeft = TXspaceLeft;

			auto it2 = it;

			int i, payloadSize = 1;
			for (i = 0; TXspaceLeft >= 2 && it2 != list.end(); ++i, ++it2) {
				int variableSize = getVariableSize(variables[*it2]);
				RXspaceLeft -= variableSize;
				if (RXspaceLeft < 0) break;
				idList[i] = *it2;
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

			for (int j = 0; j < i && it != list.end(); ++j) {
				setVarFromPointer(variables[*it], rxpayload + index);
				index += getVariableSize(variables[*it]);
				++it;
			}
		}
	}

	void Globals::setList(std::vector<uint16_t> list) {
		uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(Comms::rxpacket);
		uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(Comms::txpacket);

		auto it = list.begin();
		while (it != list.end()) {
			int i, payloadIndex = 1;
			for (i = 0; it != list.end(); ++i, ++it) {
				int variableSize = getVariableSize(variables[*it]);
				if (payloadIndex + 2 + variableSize > GNCLINK_PACKET_MAX_PAYLOAD_LENGTH) break;
				
				*((uint16_t*)(txpayload + payloadIndex)) = *it;
				setPointerFromVar(variables[*it], txpayload + payloadIndex + 2);

				payloadIndex += 2 + variableSize;
			}

			*txpayload = (uint8_t)i;

			GNClink_Construct_Packet(Comms::txpacket, GNClink_PacketType_SetValueList, GNClink_PacketFlags_None, payloadIndex);

			Comms::CommsLoop(*port);

			GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
			GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
			if (packetType != GNClink_PacketType_SetValueList || (packetFlags & GNClink_PacketFlags_Response) == 0) return;
		}
	}

	void Globals::loadList(std::vector<uint16_t> list) {
		uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(Comms::rxpacket);
		uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(Comms::txpacket);
		uint16_t* idList = (uint16_t*)(txpayload + 1);

		auto it = list.begin();
		while (it != list.end()) {
			int TXspaceLeft = GNCLINK_PACKET_MAX_PAYLOAD_LENGTH - 1;
			int RXspaceLeft = TXspaceLeft;

			int i, payloadSize = 1;
			for (i = 0; TXspaceLeft >= 2 && it != list.end(); ++i, ++it) {
				idList[i] = *it;
				TXspaceLeft -= 2;
				payloadSize += 2;
			}

			*txpayload = (uint8_t)i;

			GNClink_Construct_Packet(Comms::txpacket, GNClink_PacketType_LoadValueList, GNClink_PacketFlags_None, payloadSize);

			Comms::CommsLoop(*port);

			GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
			GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
			if (packetType != GNClink_PacketType_LoadValueList || (packetFlags & GNClink_PacketFlags_Response) == 0) return;
		}
	}

	void Globals::saveList(std::vector<uint16_t> list) {
		uint8_t* rxpayload = GNClink_Get_Packet_Payload_Pointer(Comms::rxpacket);
		uint8_t* txpayload = GNClink_Get_Packet_Payload_Pointer(Comms::txpacket);
		uint16_t* idList = (uint16_t*)(txpayload + 1);

		auto it = list.begin();
		while (it != list.end()) {
			int TXspaceLeft = GNCLINK_PACKET_MAX_PAYLOAD_LENGTH - 1;
			int RXspaceLeft = TXspaceLeft;

			int i, payloadSize = 1;
			for (i = 0; TXspaceLeft >= 2 && it != list.end(); ++i, ++it) {
				idList[i] = *it;
				TXspaceLeft -= 2;
				payloadSize += 2;
				std::cout << std::format("Global variable {} saved to NVM\n", *it);
			}

			*txpayload = (uint8_t)i;

			GNClink_Construct_Packet(Comms::txpacket, GNClink_PacketType_SaveValueList, GNClink_PacketFlags_None, payloadSize);

			Comms::CommsLoop(*port);

			GNClink_PacketType packetType = GNClink_Get_Packet_Type(Comms::rxpacket);
			GNClink_PacketFlags packetFlags = GNClink_Get_Packet_Flags(Comms::rxpacket);
			if (packetType != GNClink_PacketType_SaveValueList || (packetFlags & GNClink_PacketFlags_Response) == 0) return;
		}
	}

	Globals::Globals(Serial::Port& port) {
		this->port = &port;

		valueCount = Comms::GetValueCount(port);

		std::cout << valueCount << "\n";

		for (int i = 0; i < valueCount; ++i) {
			GlobalVariable var;
			
			uint16_t id = i + 1;
			
			var.name = Comms::GetValueName(id, var.type, port);
			
			variables.insert({ id, var });

			std::cout << var.name << "\n";
		}

		pollAll();


	}


	uint16_t Globals::getIdFromName(std::string name) {
		std::map<uint16_t, GlobalVariable>::iterator iterator = std::find_if(variables.begin(), variables.end(),
			[&](const std::pair<uint16_t, GlobalVariable>& var) { return var.second.name == name; });

		if (iterator == variables.end()) return 0;

		return (*iterator).first;
	}

}