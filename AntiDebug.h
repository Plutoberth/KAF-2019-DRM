#pragma once
#include "windows.h"
#include "stdio.h"
#include "utils.h"
#include "Psapi.h"
#include "Winver.h"
#include <iostream>
#include <unordered_set>

// Implement in header file because TLS callbacks can't call functions in a different TU, apparently.
__forceinline void killProgram()
{
	void (*killme)(void) = 0;
	//rdtsc is basically just here to make people think there's some kind of time based debugging and confuse them
	__asm __volatile
	{
		rdtsc 
		mov ebx, eax
		xor eax, edx
		mov killme, eax
		rdtsc
		sub eax, ebx
		xor eax, edx
	}
	killme();
}


constexpr static std::size_t debuggerHashes[]{
	"x32dbg"_hash,
	"x64dbg"_hash,
	"x96dbg"_hash,
	"ida64"_hash,
	"idaq"_hash,
	"idaq64"_hash,
	"OLLYDBG"_hash
};

inline BOOL CALLBACK EnumWindowsAntiDebug(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
) {
	DWORD procId = 0;

	GetWindowThreadProcessId(hwnd, &procId);

	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);

	if (h != 0)
	{
		WCHAR buffer[MAX_PATH] = { 0 };

		if (GetModuleFileNameExW(h, 0, buffer, sizeof(buffer) - 1))
		{
			DWORD trash = 0;
			DWORD fileVersionSize = GetFileVersionInfoSizeW(buffer, &trash);
			if (fileVersionSize > 0 && fileVersionSize < 8192)
			{
				VOID* fileVersionInfoBuf = alloca(fileVersionSize);
				if (GetFileVersionInfoW(buffer, 0, fileVersionSize, fileVersionInfoBuf))
				{
					CHAR* fileDescription = NULL;
					UINT fileDescriptionLength = 0;
					
					if (VerQueryValueA(
						fileVersionInfoBuf,
						"\\StringFileInfo\\0c004e9f\\FileDescription",
						(LPVOID*)&fileDescription,
						&fileDescriptionLength))
					{
						size_t hash = str_hash(fileDescription, fileDescriptionLength);
						for (size_t i = 0; i < sizeof(debuggerHashes); i++)
						{
							if (debuggerHashes[i] == hash)
							{
								BOOL* isDebugged = reinterpret_cast<BOOL*>(lParam);
								*isDebugged = TRUE;
								return FALSE;
							}
						}
					}
					else {
						int a = GetLastError();
						wchar_t buf[256];
						FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
						std::wcout << buf << std::endl;
					}
					
					

				}
			}
		
		}
		CloseHandle(h);
	}

	return TRUE;
}


__forceinline void BeingDebuggedSoftwareBreakpoint();