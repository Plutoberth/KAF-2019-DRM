#include <iostream>
#include "windows.h"
#include <emmintrin.h>
#include "TLSCallback.h"
#include "AntiDebug.h"
#include "VirtualMachine.h"


int main()
{
#ifndef _DEBUG
	if (IsDebuggerPresent())
	{
		//Extremely simple red herring for anti debugging
		puts("DON'T DEBUG ME!!!!");
		exit(1337);
	}
#endif
	
	
	std::cout << "Welcome to the DRM challenge. Please enter your flag to win: ";
	runVmCode(opcodes, sizeof(opcodes));


	return 0;
}
