#pragma once
#include "windows.h"
#include "stdio.h"

#define ExitDebugSimple() if (IsDebuggerPresent()) {exit(1337);}

// In header file because TLS callbacks can't call functions in a different TU, apparently.
__forceinline void killProgram()
{
	void (*killme)(void) = 0;
	//rdtsc is basically just here to make people think there's some kind of time based debugging and confuse them
	__asm __volatile
	{
		rdtsc 
		xor eax, edx
		mov killme, eax
		rdtsc
		xor eax, edx
	}
	killme();
}

__forceinline void BeingDebuggedSoftwareBreakpoint();