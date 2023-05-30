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
#include "commandline.h"
#include "mainwindow.h"

#include <2dGraphicsLibrary.h>


int main(std::vector<std::wstring_view> commandLineArguments) {
	EvaluateCommandLine(commandLineArguments);

	//Graphics2D::init();

	WindowManager::Window* window = MainWindow::Create(600, 400);
	window->thread.join();

	Sleep(5000);

	//std::cout << "Scanning for compatible port...\n";

	//std::vector<std::wstring> busReportedDeviceDescList{ L"GNC2" };
	//std::vector<Serial::Port> ports;
	//while (1) {
	//	ports = Serial::GetPortsWithBusReportedDeviceDescInList(busReportedDeviceDescList);
	//	if (ports.size()) break;
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}

	//if (ports.size() > 1) {
	//	std::cout << "More than one compatible port found, cannot proceed with autoconnect.\n";
	//	return 1;
	//}

	//Serial::Port port = ports[0];
	//Serial::OpenPort(port);
	//std::wcout << std::format(L"Connected to {}.\n", port.name);

	//uint32_t crc = Comms::GetGlobalHash(port);
	//crc = Comms::GetGlobalHash(port);
	//crc = Comms::GetGlobalHash(port);
	//

	//std::cout << std::format("0x{:08x}\n", crc);
	//Comms::test(port);

	//helloworld();

	//std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}