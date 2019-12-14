#include <iostream>
#include "windows.h"
#include <emmintrin.h>
#include "TLSCallback.h"
#include "AntiDebug.h"
#include "VirtualMachine.h"

LONG WINAPI
VectoredHandlerTest(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	puts("In exception handler...");
	ExceptionInfo->ContextRecord->Eip++;
	
	return EXCEPTION_CONTINUE_EXECUTION;
}




//A weird divide by zero function with SSE instructions for "obfuscation".
volatile int weirdDivByZero()
{
	__m128d xmm0 = _mm_setr_pd(0xdeadbeef8badf00d, 0x0);
	__m128d xmm1 = _mm_setr_pd(0x0, 0x0);
	xmm0 = _mm_div_pd(xmm0, xmm1);

	int g = (int)xmm0.m128d_f64[0];
	int c = xmm0.m128d_f64[1];
	g += c;
	return c / g;
}


//add important code in exception handler
//Idea: replace IsDebuggerPresent with a 'stealthier' function, perhaps access PEB directly

//Opcode: 1 byte
//Other stuff: depends on opcode

void runVmCode(const char* opcodes, unsigned int len) 
{
	while (opcodes < opcodes + len)
	{
		Opcode opcode = Opcode(*opcodes);
		opcodes++; //Move to next opcode or next instruction
		switch (opcode)
		{
		case Opcode::DIV_BY_ZERO:
			weirdDivByZero();
			break;



		default:
			//Some default handler for opcodes
			break;
		}
	}
}

int main()
{
	std::cout << "no debuggers probs? " << isDebugged << std::endl;
	
	if (IsDebuggerPresent())
	{
		//Extremely simple red herring for anti debugging
		puts("DON'T DEBUG ME!!!!");
		exit(1337);
	}
	AddVectoredExceptionHandler(CALL_FIRST, &VectoredHandlerTest);
	
	//Send to an OS function to make sure that the function isn't going to be omitted.
	SetLastError(weirdDivByZero());
	
	return 0;
}
