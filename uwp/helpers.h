// Extend from Superpowered example

#ifndef helpers
#define helpers

#if _DEBUG
#define CONFIGURATION "Debug"
#else
#define CONFIGURATION "Release"
#endif
#if _M_ARM
#define PLATFORM "ARM"
#elif _M_X64
#define PLATFORM "x64"
#else
#define PLATFORM "x86"
#endif
#pragma comment(lib, "..\\Superpowered\\Windows\\SuperpoweredWinUWP_" CONFIGURATION "_" PLATFORM  ".lib")

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <stdio.h>
#include <ctime>

#define stringToChar(path, cpath) cpath[WideCharToMultiByte(CP_UTF8, 0, path->Data(), -1, cpath, MAX_PATH + 1, NULL, NULL)] = 0;
#define charToWchar(path, wpath) MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 0);

static void Log(const char *format, ...) {
	char buf[8192];
	va_list args;
	va_start(args, format);
	_vsnprintf_s_l(buf, 8192, 8192, format, NULL, args);
	va_end(args);
	OutputDebugStringA(buf);
}

static std::wstring formatTimeString(unsigned int second) {
	const unsigned int s = second % 60;
	unsigned int m = (second / 60);
	const unsigned int h = m / 60;
	m = (m >= 60) ? m % 60 : m;
	wchar_t msg[16];
	_snwprintf_s(msg, 15, L"%02d:%02d:%02d", h, m, s);
	return std::wstring(msg);
}

#endif
