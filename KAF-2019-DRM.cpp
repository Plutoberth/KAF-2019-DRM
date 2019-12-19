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
	unsigned char opcodes[] = {static_cast<unsigned char>(Opcode::PRIV_INSTRUCTION_CMP_REG_REG), 0x02, 0x1};
	std::cout << "Welcome to the DRM challenge. Please enter your flag to win: " << GetLastError();
	runVmCode(opcodes, sizeof(opcodes));

	return 0;
}
