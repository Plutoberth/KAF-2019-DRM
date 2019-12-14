#pragma once
#include "windows.h"
#include "stdio.h"
#include "utils.h"
#include "Psapi.h"
#include "Winver.h"
#include "Strsafe.h"
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
	"OllyDbg, 32-bit analysing debugger"_hash,
	"x64dbg"_hash,
	"The Interactive Disassembler"_hash
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
		WCHAR filePath[MAX_PATH] = { 0 };

		if (GetModuleFileNameExW(h, 0, filePath, sizeof(filePath) - 1))
		{
			DWORD trash = 0;
			DWORD fileVersionSize = GetFileVersionInfoSizeW(filePath, &trash);
			if (fileVersionSize > 0 && fileVersionSize < 8192)
			{
				VOID* fileVersionInfoBuf = alloca(fileVersionSize);
				if (GetFileVersionInfoW(filePath, 0, fileVersionSize, fileVersionInfoBuf))
				{
					CHAR* fileDescription = NULL;
					UINT fileDescriptionLength = 0;

					struct LANGANDCODEPAGE {
						WORD wLanguage;
						WORD wCodePage;
					} *lpTranslate;

					// Read the list of languages and code pages.
					UINT cbTranslate = 0;
					if (VerQueryValue(fileVersionInfoBuf,
						TEXT("\\VarFileInfo\\Translation"),
						(LPVOID*)&lpTranslate,
						&cbTranslate))
					{
						// Read the file description for each language and code page.

						for (int i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
						{
							CHAR SubBlock[51] = { 0 };
							//040904b0 (UTF-16 | en-US should work for most)
							snprintf(SubBlock, 50, "\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

							if (VerQueryValueA(
								fileVersionInfoBuf,
								SubBlock,
								(LPVOID*)&fileDescription,
								&fileDescriptionLength))
							{
								size_t hash = str_hash(fileDescription, strlen(fileDescription));
								
								for (size_t i = 0; i < sizeof(debuggerHashes); i++)
								{
									if (debuggerHashes[i] == hash && hash != 0)
									{
										BOOL* isDebugged = reinterpret_cast<BOOL*>(lParam);
										*isDebugged = TRUE;
										return FALSE;
									}
								}
							}
						}
					}
				}
			}
		}
		CloseHandle(h);
	}

	return TRUE;
}


__forceinline void BeingDebuggedSoftwareBreakpoint();