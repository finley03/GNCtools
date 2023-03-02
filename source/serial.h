#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <vector>

namespace Serial {
	// port struct
	struct Port {
		std::wstring name;
		unsigned long deviceInstanceHandle;
		void* file;
		bool open;
	};

	std::vector<Port> GetPorts();
	std::vector<Port> GetPortsWithBusReportedDeviceDescInList(std::vector<std::wstring>& list);

	bool OpenPort(Port& port, int baud = 9600);
	bool ClosePort(Port& port);

	bool Read(Port& port, uint8_t* buffer, int count);
	bool Write(Port& port, uint8_t* buffer, int count);
}

#endif