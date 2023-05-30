#include "commandline.h"

#include <iostream>
#include <map>
#include <format>

#include "windowmanager.h"

enum CommandLineArgumentEnum {
	CommandLineArgumentEnum_INVALID,
	CommandLineArgumentEnum_NOGUI,
	CommandLineArgumentEnum_ATTACH_CONSOLE,
	CommandLineArgumentEnum_ECHO_COMMAND_LINE_ARGUMENTS
};

std::map<std::wstring_view, int> CreateCommandLineMap(std::vector<std::wstring_view> commandLineArguments) {
	std::map<std::wstring_view, int> cmdLineMap;

	cmdLineMap[L"--nogui"] = CommandLineArgumentEnum_NOGUI;
	cmdLineMap[L"--attach-console"] = CommandLineArgumentEnum_ATTACH_CONSOLE;
	cmdLineMap[L"--echo-command-line-arguments"] = CommandLineArgumentEnum_ECHO_COMMAND_LINE_ARGUMENTS;

	return cmdLineMap;
}

void EvaluateCommandLine(std::vector<std::wstring_view> commandLineArguments) {
	// create map of command line arguments
	std::map<std::wstring_view, int> commandLineMap = CreateCommandLineMap(commandLineArguments);

	// preemptively handle potential --attach-console command to view stdout
	if (std::find(commandLineArguments.begin(), commandLineArguments.end(), L"--attach-console") != commandLineArguments.end())
		WindowManager::CreateConsole();

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
}