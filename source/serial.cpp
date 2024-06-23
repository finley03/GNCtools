#include "serial.h"
#include <iostream>
#include <format>
#include <memory>
#include <cwchar>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <initguid.h>
#include <devpkey.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <combaseapi.h>

namespace Serial {
	std::vector<Port> GetPorts() {
		// get GUID string from GUID value
		OLECHAR* GUIDString;
		if (StringFromCLSID(
			GUID_DEVCLASS_PORTS,
			&GUIDString
		) != S_OK) {
			std::cout << "Error getting string from 'GUID_DEVCLASS_PORTS' GUID.\n";
			return {};
		}

		// get required buffer size to hold the device instance IDs
		ULONG deviceInstanceIDBufferLength;
		if (CM_Get_Device_ID_List_SizeW(
			&deviceInstanceIDBufferLength,
			GUIDString,
			CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT
		) != CR_SUCCESS) {
			std::cout << "Error getting required buffer size to retreive the device instance IDs\
				after filtering for GUID 'GUID_DEVCLASS_PORTS'.\n";
			return {};
		}

		// create buffer
		std::unique_ptr<wchar_t[]> deviceInstanceIDBuffer(new wchar_t[deviceInstanceIDBufferLength]);

		// get device instance ID list
		if (CM_Get_Device_ID_ListW(
			GUIDString,
			deviceInstanceIDBuffer.get(),
			deviceInstanceIDBufferLength,
			CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT
		) != CR_SUCCESS) {
			std::cout << "Error getting device instance ID list.\n";
			return {};
		}

		// parse buffer to a vector of strings
		std::vector<std::wstring> deviceInstanceIDStringList;
		for (int index = 0; index < deviceInstanceIDBufferLength - 1; ++index) { // terminated by an EXTRA null
			// add string
			deviceInstanceIDStringList.push_back(std::wstring(deviceInstanceIDBuffer.get() + index));
			// find start of next string
			index = std::wcschr(deviceInstanceIDBuffer.get() + index, '\0') - deviceInstanceIDBuffer.get();
		}

		// get device instance Handles
		std::vector<DEVINST> deviceInstanceHandleList;
		deviceInstanceHandleList.reserve(deviceInstanceIDStringList.size());
		for (std::wstring deviceInstanceIDString : deviceInstanceIDStringList) {
			DEVINST deviceInstanceHandle;
			if (CM_Locate_DevNodeW(
				&deviceInstanceHandle,
				(DEVINSTID_W)deviceInstanceIDString.c_str(),
				CM_LOCATE_DEVNODE_NORMAL
			) != CR_SUCCESS) {
				std::cout << "Error locating device instance handle.\n";
				return {};
			}
			deviceInstanceHandleList.push_back(deviceInstanceHandle);
		}

		// get port names of valid devices
		//std::vector<std::wstring> portNames;
		std::vector<Port> ports;
		for (DEVINST deviceInstanceHandle : deviceInstanceHandleList) {
			wchar_t friendlyNameBuffer[128];
			DEVPROPTYPE devicePropertyType;
			ULONG friendlyNameBufferSize = 128;
			if (CONFIGRET ret = CM_Get_DevNode_PropertyW(
				deviceInstanceHandle,
				&DEVPKEY_Device_FriendlyName,
				&devicePropertyType,
				(PBYTE)friendlyNameBuffer,
				&friendlyNameBufferSize,
				0
			) != CR_SUCCESS) {
				std::cout << "Error getting port friendly name of device.\n";
				return {};
			}
			/*portNames.push_back(std::wstring(friendlyNameBuffer));*/
			std::wstring friendlyNameString(friendlyNameBuffer);
			int COMPosition = friendlyNameString.find(L"COM", 0);
			if (COMPosition == std::string::npos) {
				std::cout << "Error finding 'COM' in device friendly name.\n";
				return {};
			}
			const wchar_t* digits = L"0123456789";
			int COMPositionEnd = friendlyNameString.find_first_not_of(digits, COMPosition + 3);
			Port port = {
				.name = friendlyNameString.substr(COMPosition, COMPositionEnd - COMPosition),
				.deviceInstanceHandle = deviceInstanceHandle
			};
			ports.push_back(port);
		}

		return ports;
	}

	std::vector<Port> GetPortsWithBusReportedDeviceDescInList(std::vector<std::wstring>& list) {
		std::vector<Port> unfilteredPorts = GetPorts(), ports;
		for (Port port : unfilteredPorts) {
			wchar_t busReportedDeviceDescBuffer[128];
			DEVPROPTYPE devicePropertyType;
			ULONG busReportedDeviceDescBufferSize = 128;
			if (CM_Get_DevNode_PropertyW(
				port.deviceInstanceHandle,
				&DEVPKEY_Device_BusReportedDeviceDesc,
				&devicePropertyType,
				(PBYTE)busReportedDeviceDescBuffer,
				&busReportedDeviceDescBufferSize,
				0
			) != CR_SUCCESS) {
				std::cout << "Error getting bus reported device name.\n";
				return {};
			}
			for (std::wstring name : list) {
				if (std::wcscmp(name.c_str(), busReportedDeviceDescBuffer) == 0)
					ports.push_back(port);
			}
		}
		return ports;
	}

	bool OpenPort(Port& port, int baud) {
		std::wstring fileName = L"\\\\.\\";
		fileName.append(port.name);

		port.file = CreateFileW(
			fileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (port.file == INVALID_HANDLE_VALUE) {
			std::wcout << std::format(L"Error opening serial port {}.\n", port.name);
			return false;
		}

		DCB commState;
		if (!GetCommState(
			port.file,
			&commState
		)) {
			std::cout << "Error retreiving comm state.\n";
			ClosePort(port);
			return false;
		}

		commState.BaudRate = (DWORD)baud;
		commState.ByteSize = 8;
		commState.StopBits = ONESTOPBIT;
		commState.Parity = NOPARITY;

		if (!SetCommState(
			port.file,
			&commState
		)) {
			std::cout << "Error setting comm state.\n";
			ClosePort(port);
			return false;
		}

		COMMTIMEOUTS commTimeouts = {
			.ReadIntervalTimeout = 100,
			.ReadTotalTimeoutMultiplier = 0,
			.ReadTotalTimeoutConstant = 0,
			.WriteTotalTimeoutMultiplier = 0,
			.WriteTotalTimeoutConstant = 300
		};

		if (!SetCommTimeouts(
			port.file,
			&commTimeouts
		)) {
			std::cout << "Error setting comm timeout configuration.\n";
			ClosePort(port);
			return false;
		}

		port.open = true;

		return true;
	}

	bool ClosePort(Port& port) {
		if (port.open)
			if (CloseHandle(port.file)) {
				port.open = false;
				return true;
			}
		return false;
	}

	bool Read(Port& port, uint8_t* buffer, int count) {
		DWORD bytesRead;
		if (!ReadFile(port.file, (LPVOID)buffer, (DWORD)count, &bytesRead, NULL)) {
			std::cout << "Error reading data from port\n";
			ClosePort(port);
			return false;
		}

		//std::cout << std::format("Read {}/{} bytes.\n", bytesRead, count);
		return true;
	}

	bool Write(Port& port, uint8_t* buffer, int count) {
		DWORD bytesWritten;
		if (!WriteFile(port.file, (LPVOID)buffer, (DWORD)count, &bytesWritten, NULL)) {
			std::cout << "Error writing data from port\n";
			ClosePort(port);
			return false;
		}

		//std::cout << std::format("Written {}/{} bytes.\n", bytesWritten, count);
		return true;
	}
}