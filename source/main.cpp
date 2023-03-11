#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <thread>
#include <map>
#include <exception>

#include "windowmanager.h"
#include "serial.h"
#include "gnclink.h"
#include "comms.h"

enum CommandLineArgumentEnum {
	CommandLineArgumentEnum_INVALID,
	CommandLineArgumentEnum_NOGUI,
	CommandLineArgumentEnum_ATTACH_CONSOLE,
	CommandLineArgumentEnum_ECHO_COMMAND_LINE_ARGUMENTS
};

std::map<std::wstring_view, int> createCommandLineMap(std::vector<std::wstring_view> commandLineArguments) {
	std::map<std::wstring_view, int> cmdLineMap;
	
	cmdLineMap[L"--nogui"] = CommandLineArgumentEnum_NOGUI;
	cmdLineMap[L"--attach-console"] = CommandLineArgumentEnum_ATTACH_CONSOLE;
	cmdLineMap[L"--echo-command-line-arguments"] = CommandLineArgumentEnum_ECHO_COMMAND_LINE_ARGUMENTS;

	return cmdLineMap;
}

int main(std::vector<std::wstring_view> commandLineArguments) {
	// create map of command line arguments
	std::map<std::wstring_view, int> commandLineMap = createCommandLineMap(commandLineArguments);

	// preemptively handle potential --attach-console command to view stdout
	if (commandLineMap.contains(L"--attach-console")) CreateConsole();

	// switch command line arguments
	for (std::wstring_view arg : commandLineArguments) {
		int argEnum;
		try {
			argEnum = commandLineMap.at(arg);
		}
		catch (std::exception& e) {
			argEnum = CommandLineArgumentEnum_INVALID;
		}

		switch (commandLineMap[arg]) {
		case CommandLineArgumentEnum_NOGUI:
			break;

		case CommandLineArgumentEnum_ATTACH_CONSOLE:
			// already handled
			break;

		case CommandLineArgumentEnum_ECHO_COMMAND_LINE_ARGUMENTS:
			for (std::wstring_view& arg : commandLineArguments) std::wcout << arg << "\n";
			break;

		case CommandLineArgumentEnum_INVALID:
		default:
			std::wcout << std::format(L"Warning: \"{}\" is not a valid command line argument and has been ignored.\n", arg);
			break;
		}
	}


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

	std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}