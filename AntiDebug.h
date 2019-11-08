#pragma once
#include "windows.h"
#include "stdio.h"

#define ExitDebugSimple() if (IsDebuggerPresent()) {killme();}

__forceinline void killProgram();

__forceinline void BeingDebuggedSoftwareBreakpoint();