#pragma once
#include "windows.h"
#include "AntiDebug.h"

BOOL isDebugged = FALSE;

VOID NTAPI test_tls_callback(
	PVOID DllHandle,
	DWORD Reason,
	PVOID Reserved)
{
	if (Reason != DLL_PROCESS_ATTACH)
	{
		return;
	}

#ifndef DEBUG
	EnumWindows(&EnumWindowsAntiDebug, reinterpret_cast<LPARAM>(&isDebugged));
	if (isDebugged)
	{
		//Code for switching opcodes
	}
#endif // !DEBUG
}

#pragma comment (linker, "/INCLUDE:__tls_used")
#pragma comment (linker, "/INCLUDE:_p_tls_callback1")
#pragma data_seg(push)
#pragma data_seg(".CRT$XLAAA")
EXTERN_C PIMAGE_TLS_CALLBACK p_tls_callback1 = test_tls_callback;