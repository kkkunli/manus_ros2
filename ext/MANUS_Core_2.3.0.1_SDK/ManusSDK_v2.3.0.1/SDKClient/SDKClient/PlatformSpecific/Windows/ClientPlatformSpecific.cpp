#include "ClientPlatformSpecific.hpp"

// Stop any Windows.h includes from declaring the min and max macros, because
// they conflict with std::min and std::max.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// CoreSdk_Shutdown, etc.
#include "ManusSDK.h"

// std::min
#include <algorithm>
// std::codecvt_utf8_utf16
#include <codecvt>
// std::filesystem
#include <filesystem>
// std::*fstream
#include <fstream>
// std::wstring_convert
#include <locale>
// SHGetKnownFolderPath
#include <ShlObj_core.h>
// spdlog
#include "spdlog/spdlog.h"
// wstringstream
#include <sstream>
// BOOL, DWORD, etc.
#include <Windows.h>

const std::string SDKClientPlatformSpecific::s_SlashForFilesystemPath = "\\";

/// @brief All keys till F24 key
bool g_PreviousKeyState[0x87] = { false };

/// @brief SDK Shutdown
/// @param p_fdwCtrlType 
/// @return 
static BOOL __stdcall ProcessConsoleShutdown(DWORD p_fdwCtrlType)
{
	switch (p_fdwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	{
		return CoreSdk_ShutDown();
	}
	default: return 0;
	}
}

/// @brief Get a string describing the error with the given error number.
/// The string is formatted so that it can be appended to an error message.
static std::string GetStringForError(int p_ErrorNumber)
{
	std::string t_Result;

	LPSTR t_ErrorMessage = NULL;

	DWORD t_NumChars = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		p_ErrorNumber,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		// Note the 'T' - this is NOT an LPSTR!
		// Microsoft's documentation says: "An LPWSTR if UNICODE is
		// defined, an LPSTR otherwise.",
		// and: "The letter "T" in a type definition, for example, TCHAR or
		// LPTSTR, designates a generic type that can be compiled for
		// either Windows code pages or Unicode."
		reinterpret_cast<LPTSTR>(&t_ErrorMessage),
		0,
		NULL);

	if (t_NumChars == 0)
	{
		DWORD t_FormatError = GetLastError();

		t_Result =
			std::string("(could not get an error string for this error number). ") +
			std::string("FormatMessage failed with error ") +
			std::to_string(t_FormatError) +
			std::string(".");
	}
	else
	{
		t_Result = std::string(static_cast<const char*>(t_ErrorMessage));
	}

	LocalFree(t_ErrorMessage);

	return t_Result;
}

static bool DoesWindowHaveFocus(void)
{
	HWND t_ConsoleWindow = GetConsoleWindow();
	HWND t_HCurWnd = GetForegroundWindow();
	
	return t_ConsoleWindow == t_HCurWnd;
}

std::string UTF16WstringToUTF8String(const std::wstring& p_Wstring)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> t_Converter;
	std::string t_Narrow = t_Converter.to_bytes(p_Wstring);

	return t_Narrow;
}

std::wstring UTF8StringtoUTF16Wstring(const std::string& p_String)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> t_Converter;
	std::wstring t_Wide = t_Converter.from_bytes(p_String);

	return t_Wide;
}

bool SDKClientPlatformSpecific::PlatformSpecificInitialization(void)
{
	// If the system's locale is set correctly, and a compatible font is used,
	// this makes it possible to display Unicode characters encoded using UTF-8
	// in the command prompt.
	if (SetConsoleOutputCP(CP_UTF8) == 0)
	{
		spdlog::error(
			"Failed to set the console output to UTF-8. The error number was {}.",
			GetLastError());

		return false;
	}

	if (SetConsoleCtrlHandler(ProcessConsoleShutdown, true) == false)
	{
		spdlog::error("Failed to initialize console shutdown events.");

		return false;
	}

	return true;
}

bool SDKClientPlatformSpecific::PlatformSpecificShutdown(void)
{
	return true;
}

void SDKClientPlatformSpecific::UpdateInput(void)
{
	// Nothing to update here for Windows.
}

bool SDKClientPlatformSpecific::CopyString(
	char* const p_Target,
	const size_t p_MaxLengthThatWillFitInTarget,
	const std::string& p_Source)
{
	// strcpy_s Is basically a Microsoft-only function.
	// https://stackoverflow.com/questions/4570147/safe-string-functions-in-mac-os-x-and-linux
	const errno_t t_CopyResult = strcpy_s(
		p_Target,
		p_MaxLengthThatWillFitInTarget,
		p_Source.c_str());
	if (t_CopyResult != 0)
	{
		spdlog::error(
			"Copying the string {} resulted in the error {}."
			, p_Source.c_str()
			, t_CopyResult);

		return false;
	}

	return true;
}

bool SDKClientPlatformSpecific::ResizeWindow(
	const short int p_ConsoleWidth,
	const short int p_ConsoleHeight,
	const short int p_ConsoleScrollback)
{
	bool t_Absolute = true;
	HANDLE t_Console = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD t_BufferSize =
	{
		p_ConsoleWidth,
		p_ConsoleScrollback // Height includes the number of lines that can be
		                    // viewed by scrolling up.
	};

	auto t_MaxConsoleSize = GetLargestConsoleWindowSize(t_Console);
	const short int t_WindowHeight =
		std::min(t_BufferSize.Y,
			std::min(p_ConsoleHeight, t_MaxConsoleSize.Y));
	_SMALL_RECT t_ConsoleRect =
	{
		0,
		0,
		std::min(t_BufferSize.X, t_MaxConsoleSize.X) - 1,
		t_WindowHeight - 1
	};

	// Resize the buffer, but not the window.
	// If the window is not resized as well, a scrollbar can be used to see
	// text that won't fit.
	if (!SetConsoleScreenBufferSize(t_Console, t_BufferSize))
	{
		DWORD t_Error = GetLastError();
		spdlog::error(
			"Setting the console screen buffer size failed with error {}: {}",
			t_Error,
			GetStringForError(t_Error));
		return false;
	}

	// Resize the window itself.
	if (!SetConsoleWindowInfo(t_Console, t_Absolute, &t_ConsoleRect))
	{
		DWORD t_Error = GetLastError();
		spdlog::error(
			"Setting the console window size failed with error {}: {}",
			t_Error,
			GetStringForError(t_Error));
		return false;
	}

	return true;
}

void SDKClientPlatformSpecific::ApplyConsolePosition(
	const int p_ConsoleCurrentOffset)
{
	COORD t_Pos = { 0, static_cast<SHORT>(p_ConsoleCurrentOffset) };
	HANDLE t_Output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(t_Output, t_Pos);
}

void SDKClientPlatformSpecific::ClearConsole(void)
{
	COORD t_TopLeft = { 0, 0 };
	HANDLE t_Console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO t_Screen;
	DWORD t_Written;

	GetConsoleScreenBufferInfo(t_Console, &t_Screen);
	FillConsoleOutputCharacterA(
		t_Console,
		' ',
		t_Screen.dwSize.X * t_Screen.dwSize.Y,
		t_TopLeft,
		&t_Written);
	FillConsoleOutputAttribute(
		t_Console,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		t_Screen.dwSize.X * t_Screen.dwSize.Y,
		t_TopLeft,
		&t_Written);
	SetConsoleCursorPosition(t_Console, t_TopLeft);
}

bool SDKClientPlatformSpecific::GetKey(const int p_Key)
{
	if (DoesWindowHaveFocus()) // we got focus? then check key state.
	{
		bool t_Res = GetAsyncKeyState(p_Key) & 0x8000;
		g_PreviousKeyState[p_Key] = t_Res;

		return t_Res;
	}

	return false;
}

bool SDKClientPlatformSpecific::GetKeyDown(const int p_Key)
{
	if (DoesWindowHaveFocus()) // we got focus? then check key state.
	{
		bool t_Res = false;
		bool t_State = GetAsyncKeyState(p_Key) & 0x8000;
		if (t_State == true && g_PreviousKeyState[p_Key] == false)
			t_Res = true;
		g_PreviousKeyState[p_Key] = t_State;

		return t_Res;
	}

	return false;
}

bool SDKClientPlatformSpecific::GetKeyUp(const int p_Key)
{
	if (DoesWindowHaveFocus()) // we got focus? then check key state.
	{
		bool t_Res = false;
		bool t_State = GetAsyncKeyState(p_Key) & 0x8000;
		if (t_State == false && g_PreviousKeyState[p_Key] == true)
			t_Res = true;
		g_PreviousKeyState[p_Key] = t_State;

		return t_Res;
	}

	return false;
}

std::string SDKClientPlatformSpecific::GetDocumentsDirectoryPath_UTF8(void)
{
	std::wstringstream t_Wss;
	// try to get the documents local path

	wchar_t* t_Path = NULL;
	HRESULT t_Result = SHGetKnownFolderPath(
		FOLDERID_Documents,
		0,
		NULL,
		&t_Path);
	// If this fails, the system is either incredibly security locked, or very
	// bad.
	if (SUCCEEDED(t_Result))
	{
		std::wstringstream t_Wss;
		t_Wss << t_Path;
		// Due to the way SHGetKnownFolderPath works. we need to clean this up.
		CoTaskMemFree(t_Path);

		std::string t_NarrowDocumentsDir =
			UTF16WstringToUTF8String(t_Wss.str());

		return t_NarrowDocumentsDir;
	}
	else
	{
		spdlog::warn("Could not get the directory path for the documents.");
	}

	return std::string("");
}

std::ifstream SDKClientPlatformSpecific::GetInputFileStream(
	std::string p_Path_UTF8)
{
	std::wstring t_WidePath = UTF8StringtoUTF16Wstring(p_Path_UTF8);

	return std::ifstream(t_WidePath, std::ifstream::binary);
}

std::ofstream SDKClientPlatformSpecific::GetOutputFileStream(
	std::string p_Path_UTF8)
{
	std::wstring t_WidePath = UTF8StringtoUTF16Wstring(p_Path_UTF8);

	return std::ofstream(t_WidePath, std::ofstream::binary);
}

bool SDKClientPlatformSpecific::DoesFolderOrFileExist(std::string p_Path_UTF8)
{
	std::wstring t_WidePath = UTF8StringtoUTF16Wstring(p_Path_UTF8);

	return std::filesystem::exists(t_WidePath);
}

void SDKClientPlatformSpecific::CreateFolderIfItDoesNotExist(
	std::string p_Path_UTF8)
{
	std::wstring t_WidePath = UTF8StringtoUTF16Wstring(p_Path_UTF8);

	if (!DoesFolderOrFileExist(p_Path_UTF8))
	{
		std::filesystem::create_directory(t_WidePath);
	}
}
