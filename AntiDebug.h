#pragma once
#include "windows.h"
#include "stdio.h"
#include "utils.h"
#include "Psapi.h"
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
	"idaq64"_hash
};

inline BOOL CALLBACK EnumWindowsAntiDebug(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
) {
	DWORD procId = 0;

	GetWindowThreadProcessId(hwnd, &procId);

	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
	int a = GetLastError();

	if (h != 0)
	{
		CHAR buffer[MAX_PATH] = { 0 };

		if (GetModuleFileNameExA(h, 0, buffer, sizeof(buffer) - 1))
		{
			CHAR filename[MAX_PATH] = { 0 };
			if (_splitpath_s(buffer, NULL, NULL, NULL, NULL, filename, sizeof(filename), NULL, NULL) == 0)
			{
				size_t hash = str_hash(filename, strlen(filename));
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
		}
		CloseHandle(h);
	}

	return TRUE;
}


__forceinline void BeingDebuggedSoftwareBreakpoint();