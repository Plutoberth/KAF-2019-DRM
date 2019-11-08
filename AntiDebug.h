#pragma once
#include "windows.h"
#include "stdio.h"

#define ExitDebugSimple() if (IsDebuggerPresent()) {exit(1337);}

__forceinline void killProgram();

__forceinline void BeingDebuggedSoftwareBreakpoint();