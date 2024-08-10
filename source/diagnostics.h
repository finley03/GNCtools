#pragma once

#include <vector>
#include "serial.h"

struct Process_Diagnostics {
	unsigned id;
	std::string name;
	unsigned call_count;
	unsigned CPU_time; // microseconds
	unsigned stack_use;
	unsigned max_stack;
	unsigned available_stack;
};

class Diagnostics {
private:
	Serial::Port* port;

public:
	std::vector<Process_Diagnostics> processes;

	Diagnostics(Serial::Port& port);

	bool enumerateProcesses();
	bool getProcessDiagnostics();

};