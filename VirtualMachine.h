#pragma once
#include "emmintrin.h"
#include "windows.h"
#include <vector>
#include "AntiDebug.h"
#include <algorithm>    
#include "utils.h"
#include <iostream>

#define CALL_FIRST 1
#define CALL_LAST 0 

#define DR7_CONTROL_MASK 0b11111111

using std::vector;

enum class Opcode {
	DIV_BY_ZERO_LOAD_REG_IMM32 = 0xab,
	INVALID_HANDLE_HASH_REG2 = 0x10,
	PRIV_INSTRUCTION_CMP_REG_REG = 0xff,
	ILLEGAL_INSTRUCTION_XOR_REG_REG = 0x00, //you can put any invalid value here
	GETCHAR_INTO_REGISTER = 0xcd,
	BREAKPOINT_ROL_REGISTER_IMM8 = 0x73,
};

using uint32 = unsigned int;

uint32 flagOk;
uint32 R[3];

LONG WINAPI
DivZeroHandler(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	bool shouldRun = !(beingDebuggedPeb() || ((ExceptionInfo->ContextRecord->Dr7 & DR7_CONTROL_MASK) != 0));
#ifdef _DEBUG
	shouldRun = true;
#endif
	if (shouldRun)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO 
			|| ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_OVERFLOW)
		{
			ExceptionInfo->ContextRecord->Eip++;
			uint32 ebx = ExceptionInfo->ContextRecord->Edi;
			uint32  ecx = ExceptionInfo->ContextRecord->Ecx;

			if (ebx < 3)
			{
				R[ebx] = ecx;
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
PrivInstructionHandler(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	bool shouldRun = !(beingDebuggedPeb() || ((ExceptionInfo->ContextRecord->Dr7 & DR7_CONTROL_MASK) != 0));

#ifdef _DEBUG
	shouldRun = true;
#endif
	if (shouldRun)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
		{
			ExceptionInfo->ContextRecord->Eip++;
			uint32 ebx = ExceptionInfo->ContextRecord->Ebx;
			uint32 ecx = ExceptionInfo->ContextRecord->Ecx;

			if (ebx < 3 && ecx < 3)
			{
				if (R[ebx] != R[ecx] && flagOk)
				{
					flagOk = 0;
				}
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
IlegalInstructionHandler(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	bool shouldRun = !(beingDebuggedPeb() || ((ExceptionInfo->ContextRecord->Dr7 & DR7_CONTROL_MASK) != 0));

#ifdef _DEBUG
	shouldRun = true;
#endif
	if (shouldRun)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION)
		{
			ExceptionInfo->ContextRecord->Eip += 2;
			uint32 ebx = ExceptionInfo->ContextRecord->Ebx;
			uint32 ecx = ExceptionInfo->ContextRecord->Ecx;

			if (ebx < 3 && ecx < 3)
			{
				R[ebx] = R[ebx] ^ R[ecx];
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
	

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
BreakpointHandler(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		ExceptionInfo->ContextRecord->Eip++;
		uint32 reg = ExceptionInfo->ContextRecord->Ebx;
		uint32 rolNum = ExceptionInfo->ContextRecord->Ecx;

		if (reg < 3)
		{
			uint32 regValue = R[reg];
			_asm {
				mov eax, regValue
				mov ecx, rolNum
				rol eax, cl
				mov regValue, eax
			}
			R[reg] = regValue;
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

//A weird divide by zero function with SSE instructions for "obfuscation".
static volatile int weirdDivByZero(const unsigned char** opcodes)
{
	__m128d xmm0 = _mm_setr_pd(0xdeadbeef8badf00d, 0x0);
	__m128d xmm1 = _mm_setr_pd(0x0, 0x0);
	xmm0 = _mm_div_pd(xmm0, xmm1);

	int g = (int)xmm0.m128d_f64[0];
	int c = xmm0.m128d_f64[1];
	g += c;

	int reg = *(*opcodes);
	(*opcodes)++;

	int val = *((int*) (*opcodes));
	(*opcodes) += 4;

	_asm {
		mov edi, g
		xor edx, edx
		mov eax, c
		mov edi, reg
		mov ecx, val
		mov edx, 5
		div edx
	}

	return 1;
}

__forceinline static volatile void closeInvalidHandle() 
{
	DWORD dwBytesRead;

	HANDLE fileHandle = CreateFile(L"maybeFlag", GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//Cast to float for weird errors
	float fileHandleFloat = static_cast<float>(reinterpret_cast<int>(fileHandle)) + 1337.1337;
	float* fileHandleFloatPtr = &fileHandleFloat;

	DWORD fileSize = GetFileSize(fileHandle, &fileSize);
	void* buf = _malloca(fileSize);
	ReadFile(fileHandle, buf, fileSize, &dwBytesRead, NULL);
	
	fileHandle = *(reinterpret_cast<HANDLE*>(fileHandleFloatPtr));
	R[2] = str_hash((const char*)(&R[2]), sizeof(R[2]));
	
	CloseHandle(reinterpret_cast<HANDLE>(0x1337));
}

__forceinline static volatile void privInstruction(const unsigned char** opcodes)
{
	int reg1 = *(*opcodes);
	(*opcodes)++;

	int reg2 = *(*opcodes);
	(*opcodes)++;

	_asm {
		mov eax, reg1
		not eax
		mov ecx, reg2
		mov edx, 1337
		xor eax, edx
		add eax, reg2
		push ebx
		mov ebx, reg1
		cli
		mov edx, 0x21
		pop ebx
	}
}

__forceinline static volatile void invalidInstruction(const unsigned char** opcodes)
{
	int reg1 = *(*opcodes);
	(*opcodes)++;

	int reg2 = *(*opcodes);
	(*opcodes)++;

	//REPLACE WITH AN ACTUAL INVALID INSTRUCTION
	_asm {
		push ebx
		mov ecx, reg2
		mov ebx, reg1
		ud2
		pop ebx
	}
}

__forceinline static volatile void doBreakpoint(const unsigned char** opcodes)
{
	int reg1 = *(*opcodes);
	(*opcodes)++;

	int num = *(*opcodes);
	(*opcodes)++;

	_asm {
		mov ebx, reg1
		rol ebx, 12
		mov ecx, num
		nop
		shr ebx, 5
		rol ebx, 25
		int 3
		mov ecx, 234
	}
}
//Opcode: 1 byte
//Other stuff: depends on opcode

int runVmCode(const unsigned char* opcodes, unsigned int len)
{
	flagOk = 1;
	const PVECTORED_EXCEPTION_HANDLER handlers[] = { &DivZeroHandler, &PrivInstructionHandler, &IlegalInstructionHandler, &BreakpointHandler };
	auto opcodesEnd = opcodes + len;

	//Final beingDebugged check before killing our ability to do so (by adding the handler)
	if (beingDebuggedSoftwareBreakpoint())
	{
		killProgram();
	}
	AddVectoredExceptionHandler(CALL_FIRST, &DivZeroHandler);
	AddVectoredExceptionHandler(CALL_FIRST, &PrivInstructionHandler);
	AddVectoredExceptionHandler(CALL_FIRST, &IlegalInstructionHandler);
	AddVectoredExceptionHandler(CALL_FIRST, &BreakpointHandler);
	while (opcodes < opcodesEnd)
	{
		Opcode opcode = Opcode(*opcodes);
		opcodes++; //Move to next opcode or next instruction
		switch (opcode)
		{
		case Opcode::DIV_BY_ZERO_LOAD_REG_IMM32:
			weirdDivByZero(&opcodes);
			break;

		case Opcode::INVALID_HANDLE_HASH_REG2:
			closeInvalidHandle();
			break;
		case Opcode::PRIV_INSTRUCTION_CMP_REG_REG:
			privInstruction(&opcodes);
			break;
			
		case Opcode::GETCHAR_INTO_REGISTER:
		{
			int reg = *opcodes;
			opcodes++;
			if (reg < 3)
			{
				R[reg] = getchar();
			}
			break;
		}

		case Opcode::BREAKPOINT_ROL_REGISTER_IMM8:
			doBreakpoint(&opcodes);
			break;

			
		default:
			invalidInstruction(&opcodes);
			break;
		}
	}
	if (flagOk)
	{
		std::cout << std::endl << "Congratulations, you solved my challenge!" << std::endl;
	}
	else
	{
		std::cout << "This isn't the correct flag :(" << std::endl;
	}
	return 0;
}