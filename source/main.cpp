#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <thread>

#include "serial.h"
#include "gnclink.h"
#include "comms.h"


int main(int argc, char** argv) {
	std::vector<std::string_view> commandLineArguments(argc);
	for (int argumentIndex = 0; argumentIndex < argc; ++argumentIndex)
		commandLineArguments[argumentIndex] = std::string_view(argv[argumentIndex]);

	std::cout << "Scanning for compatible port...\n";

	std::vector<std::wstring> busReportedDeviceDescList{ L"GNC2" };
	std::vector<Serial::Port> ports;
	while (1) {
		ports = Serial::GetPortsWithBusReportedDeviceDescInList(busReportedDeviceDescList);
		if (ports.size()) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	//if (ports.size() == 0) {
	//	std::cout << "No compatible ports found, cannot proceed with autoconnect.\n";
	//	return 1;
	//}
	if (ports.size() > 1) {
		std::cout << "More than one compatible port found, cannot proceed with autoconnect.\n";
		return 1;
	}

	Serial::Port port = ports[0];
	Serial::OpenPort(port);
	std::wcout << std::format(L"Connected to {}.\n", port.name);

	//uint8_t packet[GNCLINK_PACKET_MAX_TOTAL_LENGTH] = { 0xFF };
	//uint8_t* payload = GNClink_Get_Packet_Payload_Pointer(packet);
	//if (!GNClink_Construct_Packet(packet, GNClink_PacketType_GetGlobalHash, GNClink_PacketFlags_None, 0)) return 1;
	//
	//uint8_t frame[GNCLINK_FRAME_TOTAL_LENGTH];
	//bool moreFrames;
	//if (!GNClink_Get_Frame(packet, frame, GNClink_FrameFlags_None, 0, &moreFrames)) return -1;

	//Serial::Write(port, frame, GNCLINK_FRAME_TOTAL_LENGTH);

	//Serial::Read(port, frame, GNCLINK_FRAME_TOTAL_LENGTH);

	//if (!GNClink_Check_Frame(frame)) return -1;

	//GNClink_Reconstruct_Packet_From_Frames(frame, packet, &moreFrames);

	//uint32_t crc = *(uint32_t*)payload;
	uint32_t crc = Comms::GetGlobalHash(port);
	crc = Comms::GetGlobalHash(port);
	crc = Comms::GetGlobalHash(port);
	

	std::cout << std::format("0x{:08x}\n", crc);
	Comms::test(port);


	return 0;
}