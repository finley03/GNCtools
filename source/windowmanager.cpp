#include "windowmanager.h"

#include <iostream>
#include <vector>
#include <string_view>

#include <Windows.h>

// global program variables
HINSTANCE _hInstance;
LPWSTR _lpCmdLine;
int _nCmdShow;

// declare main function
extern int main(std::vector<std::wstring_view> commandLineArguments);

// program entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	// store variables
	_hInstance = hInstance;
	_lpCmdLine = lpCmdLine;
	_nCmdShow = nCmdShow;

	// extract command line arguments
	int numArgs;
	LPWSTR* wargv = CommandLineToArgvW(lpCmdLine, &numArgs);

	std::vector<std::wstring_view> commandLineArguments(numArgs);

	for (int i = 0; i < numArgs; ++i) {
		commandLineArguments[i] = std::wstring_view(wargv[i]);
	}

	// call main
	return main(commandLineArguments);
}

// release attached stdio console
bool ReleaseConsole() {
	bool status = true;

	FILE* fp;

	if (freopen_s(&fp, "NUL:", "r", stdin)) status = false;
	else setvbuf(stdin, NULL, _IONBF, 0);

	if (freopen_s(&fp, "NUL:", "r", stdout)) status = false;
	else setvbuf(stdout, NULL, _IONBF, 0);

	if (freopen_s(&fp, "NUL:", "r", stdout)) status = false;
	else setvbuf(stdout, NULL, _IONBF, 0);

	if (!FreeConsole()) status = false;

	return status;
}

// create attached console for stdio
bool CreateConsole() {
	bool status = false;

	ReleaseConsole();

	if (AllocConsole()) {
		status = true;

		FILE* fp;

		if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE) {
			if (freopen_s(&fp, "CONIN$", "r", stdin)) status = false;
			else setvbuf(stdin, NULL, _IONBF, 0);
		}

		if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) {
			if (freopen_s(&fp, "CONOUT$", "w", stdout)) status = false;
			else setvbuf(stdout, NULL, _IONBF, 0);
		}

		if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
			if (freopen_s(&fp, "CONOUT$", "w", stderr)) status = false;
			else setvbuf(stderr, NULL, _IONBF, 0);
		}

		std::ios_base::sync_with_stdio(true);

		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
	}

	return status;
}