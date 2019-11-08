#include <iostream>
#include "windows.h"
#include <emmintrin.h>
#include "AntiDebug.h"


#define CALL_FIRST 1
#define CALL_LAST 0 

LONG WINAPI
VectoredHandlerTest(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	std::cout << "exception handler\n";
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


//add exception handler for divide by zero for obfuscation
//add important code in exception handler
//add tls callback to decode resource

//Idea: VM with exception codes


int main()
{
	AddVectoredExceptionHandler(CALL_FIRST, &VectoredHandlerTest);
	//ExitDebug();
	std::cout << "Test!" << std::endl;

	//BeingDebuggedSoftwareBreakpoint();
	
	//Send to an OS function to make sure that the function isn't going to be omitted.
	SetLastError(weirdDivByZero());
	

	  
	std::cout << "Test2!" << std::endl;
	char _ = getchar();
	return 0;
}
