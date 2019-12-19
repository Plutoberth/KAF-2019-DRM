#pragma once
#include "windows.h"
#include "stdio.h"
#include "utils.h"
#include "Psapi.h"
#include "Winver.h"
#include <iostream>

typedef struct TranslationIdentifier {
	WORD wLanguage;
	WORD wCodePage;
} TranslationIdentifier;

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
	"Immunity Debugger, 32-bit analysing debugger"_hash,
	"x64dbg"_hash,
	"The Interactive Disassembler"_hash,
	"Sysinternals Process Explorer"_hash,
	"Process Monitor"_hash
};

inline BOOL CALLBACK EnumWindowsAntiDebug(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
) {
	DWORD procId = 0;

	//!!!! WINAPI BLOAT AHEAD !!!!

	GetWindowThreadProcessId(hwnd, &procId);

	HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);

	if (procHandle != 0)
	{
		WCHAR procPath[MAX_PATH] = { 0 };

		if (GetModuleFileNameExW(procHandle, 0, procPath, sizeof(procPath) - 1))
		{
			DWORD trash = 0;
			DWORD fileVersionSize = GetFileVersionInfoSizeW(procPath, &trash);
			if (fileVersionSize > 0 && fileVersionSize < 8192)
			{
				//Fetch file version info data
				VOID* fileVersionInfoBuf = alloca(fileVersionSize);
				if (GetFileVersionInfoW(procPath, 0, fileVersionSize, fileVersionInfoBuf))
				{
					TranslationIdentifier *lpTranslate;

					// Read the list of languages and code pages.
					UINT numTranslations = 0;
					if (VerQueryValue(fileVersionInfoBuf,
						TEXT("\\VarFileInfo\\Translation"),
						(LPVOID*)&lpTranslate,
						&numTranslations))
					{
						// Read the file description for each language and code page.
						for (int i = 0; i < (numTranslations / sizeof(TranslationIdentifier)); i++)
						{
							CHAR* fileDescription = NULL;
							UINT fileDescriptionLength = 0;
							CHAR FileDescBlock[51] = { 0 };
							//040904b0 (UTF-16 | en-US) should work for most
							snprintf(FileDescBlock, 50, "\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

							if (VerQueryValueA(
								fileVersionInfoBuf,
								FileDescBlock,
								(LPVOID*)&fileDescription,
								&fileDescriptionLength))
							{
								//fileDescriptionLength from VerQueryValueA includes null terminator
								size_t hash = str_hash(fileDescription, fileDescriptionLength - 1);
								
								for (size_t i = 0; i < sizeof(debuggerHashes); i++)
								{
									if (debuggerHashes[i] == hash && hash != 0)
									{
										BOOL* isDebugged = reinterpret_cast<BOOL*>(lParam);
										*isDebugged = TRUE;
										return FALSE; //Stop enuming windows
									}
								}
							}
						}
					}
				}
			}
		}
		CloseHandle(procHandle);
	}

	return TRUE;
}

__forceinline int beingDebuggedSoftwareBreakpoint()
{
	__try
	{
		__asm
		{
			int 2dh; Basically behaves the same as 0xcc
		}
	}
	__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return 0;
	}

	return 1;
}

__forceinline int beingDebuggedPeb()
{
#ifdef _DEBUG
	return 0;
#endif

	int isDebugged = 0;

	_asm
	{
		mov ebx, 50
		ror ebx, 2
		mov eax, fs:[0x18]
		xor eax, ebx
		xor eax, 0x8000000c
		mov eax, [eax + 0x30]
		movzx eax, byte ptr [eax+2]
		not eax
		mov isDebugged, eax
	}
	return ~isDebugged;
}
