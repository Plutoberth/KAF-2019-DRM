#include "AntiDebug.h"

void killProgram()
{
	void (*killme)(void) = 0;
	__asm __volatile
	{
		rdtsc
		xor eax, edx
		mov killme, eax
	}
	killme();
}

void BeingDebuggedSoftwareBreakpoint()
{
	__try
	{
		__asm
		{
			int 2dh  ; Basically behaves the same as 0xcc
		}
	}
	__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		printf("Not being debugged.\n");
		return;
	}

	killProgram();
}
