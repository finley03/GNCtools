#include "diagnostics.h"
#include "comms.h"
#include <gnclink.h>


Diagnostics::Diagnostics(Serial::Port& port) {
	this->port = &port;
}

bool Diagnostics::enumerateProcesses() {
	// clear processes for fresh start
	processes.clear();

	int process_count = Comms::GetProcessCount(*port);

	uint32_t id = UINT32_MAX;
	bool firstTime = true;

	while (Comms::EnumerateProcesses(id, firstTime, *port)) {
		firstTime = false;

		Process_Diagnostics proc;
		proc.id = id;
		proc.name = Comms::GetProcessName(id, *port);
		if (proc.name.empty()) return false; // invalid process ID

		processes.push_back(proc);
	}

	return process_count == processes.size(); // return false if enumerated processes does not match given process count
}


bool Diagnostics::getProcessDiagnostics() {
	if (processes.size() != Comms::GetProcessCount(*port)) return false; // check process count has not changed

	for (Process_Diagnostics& proc : processes) {
		GNClink_PacketPayload_GetProcessDiagnostics_Response response = Comms::GetProcessDiagnostics(proc.id, *port);

		proc.call_count = response.call_count;
		proc.CPU_time = response.CPU_time;
		proc.max_stack = response.max_stack;
		proc.stack_use = response.stack_use;
		proc.available_stack = response.available_stack;
	}

	return true;
}