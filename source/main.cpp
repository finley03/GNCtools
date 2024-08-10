#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <thread>
#include <map>
#include <exception>

#include "comms.h"
#include "commandline.h"
#include "mainwindow.h"
#include"aboutwindow.h"

#include "globals.h"
#include "diagnostics.h"

Globals::Globals* globals_ptr = nullptr;
Diagnostics* diagnostics_ptr = nullptr;
bool quit = false;


void printVariable(Globals::GlobalVariable variable) {
	char value[16];
	
	std::cout << variable.name << " : ";

	switch (variable.type) {
	case Globals::GlobalVariableTypes::I64:
		snprintf(value, sizeof(value), "%d", variable.value.i64);
		break;

	case Globals::GlobalVariableTypes::U64:
		snprintf(value, sizeof(value), "%u", variable.value.u64);
		break;

	case Globals::GlobalVariableTypes::F64:
		snprintf(value, sizeof(value), "%f", variable.value.f64);
		break;

	case Globals::GlobalVariableTypes::I32:
		snprintf(value, sizeof(value), "%d", variable.value.i32);
		break;

	case Globals::GlobalVariableTypes::U32:
		snprintf(value, sizeof(value), "%u", variable.value.u32);
		break;

	case Globals::GlobalVariableTypes::FP32:
		snprintf(value, sizeof(value), "%d", variable.value.i32);
		break;

	case Globals::GlobalVariableTypes::F32:
		snprintf(value, sizeof(value), "%f", variable.value.f32);
		break;

	case Globals::GlobalVariableTypes::I16:
		snprintf(value, sizeof(value), "%d", variable.value.i16);
		break;

	case Globals::GlobalVariableTypes::U16:
		snprintf(value, sizeof(value), "%u", variable.value.u16);
		break;

	case Globals::GlobalVariableTypes::I8:
		snprintf(value, sizeof(value), "%d", variable.value.i8);
		break;

	case Globals::GlobalVariableTypes::U8:
		snprintf(value, sizeof(value), "%u", variable.value.u8);
		break;

	case Globals::GlobalVariableTypes::BOOL:
		snprintf(value, sizeof(value), variable.value.b ? "TRUE" : "FALSE");
		break;

	default:
		snprintf(value, sizeof(value), "<ERROR TYPE>");
	}

	std::cout << value << "\n";

}


int main(std::vector<std::wstring_view> commandLineArguments) {
	EvaluateCommandLine(commandLineArguments);

	//MainWindow::Init();

	//Graphics2D::init();

	std::srand(std::time(nullptr)); // random seed


	WindowManager::Window* mainWindow = MainWindow::Create(1400, 1000, L"GNC2 Ground Station");
	//WindowManager::Window* aboutWindow = AboutWindow::Create(400, 300, L"About GNC2 Ground Station");

	while (!quit) {
		std::cout << "Scanning for compatible port...\n";

		std::vector<std::wstring> busReportedDeviceDescList{ L"GNC2", L"CP2102 USB to UART Bridge Controller" };
		std::vector<Serial::Port> ports;
		while (1) {
			ports = Serial::GetPortsWithBusReportedDeviceDescInList(busReportedDeviceDescList);
			if (ports.size()) break;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (ports.size() > 1) {
			std::cout << "More than one compatible port found, cannot proceed with autoconnect.\n";
			return 1;
		}

		Serial::Port port = ports[0];
		/*Serial::Port port;
		port.name = L"COM5";*/
		Serial::OpenPort(port, 4800);
		std::wcout << std::format(L"Connected to {}.\n", port.name);

		uint32_t crc = Comms::GetGlobalHash(port);
		std::cout << std::format("Global Hash: 0x{:08X}\n", crc);

		Globals::Globals globals(port);
		globals_ptr = &globals;

		std::wcout << std::format(L"Discovered {} variables.\n", globals.variables.size());

		//uint16_t process_count = Comms::GetProcessCount(port);
		//std::cout << std::format("There are {} running processes.\n", process_count);
		////std::string proc0name = Comms::GetProcessName(0, port);
		////std::cout << proc0name << " Process\n";

		//std::vector<uint32_t> processes;
		//uint32_t id = UINT32_MAX;
		//Comms::EnumerateProcesses(id, true, port);
		//processes.push_back(id);
		//while (Comms::EnumerateProcesses(id, false, port)) {
		//	processes.push_back(id);
		//}

		//for (const uint32_t& id : processes) {
		//	std::cout << std::format("Process {}: {}\n", id, Comms::GetProcessName(id, port));
		//	GNClink_PacketPayload_GetProcessDiagnostics_Response diagnostics = Comms::GetProcessDiagnostics(id, port);
		//	std::cout << std::format("Call count: {}\nCPU time: {}\n", diagnostics.call_count, diagnostics.CPU_time);
		//}

		Diagnostics diagnostics(port);
		diagnostics_ptr = &diagnostics;

		//diagnostics.enumerateProcesses();
		//diagnostics.getProcessDiagnostics();

		//for (Process_Diagnostics& proc : diagnostics.processes) {
		//	std::cout << std::format("[{}] {} Process: {}us CPU, {} calls.\n", proc.id, proc.name, proc.CPU_time, proc.call_count);
		//}

		std::vector<uint16_t> pollList = { 16, 17, 18, 19 };

		while (port.open) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			Comms::mutex.lock();
			globals.pollAll();
			//globals.pollList(pollList);
			Comms::mutex.unlock();

			mainWindow->checkRender();
			mainWindow->setWantRender();
		}

		globals_ptr = nullptr;
	}

	mainWindow->thread.join();
	//aboutWindow->thread.join();

	return 0;
}