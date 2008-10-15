#include <windows.h>
#include <tchar.h>
#include "injector.h"

#define MAX_FUNCTION_SIZE 4096
#define ORIGINAL_COPY_OFFSET 2048

struct SimpleInstruction
{
	DWORD  ins;
	DWORD  mask;
	size_t size;
};

// Disassembly example (HelloWorld in release mode):
// 00401350 8B 44 24 04      mov         eax,dword ptr [esp+4] 
// 00401354 85 C0            test        eax,eax 
static SimpleInstruction instructions[] =
{
	{    0x50,     0xF0, 1},  // push register
	{0x24448B, 0xFFFFFF, 4},  // mov         eax,dword ptr [esp+4] 
	{0x00C085, 0x00FFFF, 2},  // test        eax,eax
	{0, 0, 0}
};

static size_t simpleSize(void* code)
{
	DWORD ins = *(DWORD*)code;
	for (SimpleInstruction* iter = instructions;
		iter->mask != 0;
		++iter)
	{
		if ((ins & iter->mask) == iter->ins)
		{
			return iter->size;
		}
	}
	return 0;
}

static size_t copy(void* fnDst, void* fnSrc, size_t count)
{
	size_t done = 0;
	while (done < count)
	{
		size_t instructionSize = simpleSize(fnSrc);
		if (instructionSize == 0)
		{
			return 0; // unknown instruction...
		}
		memcpy(fnDst, fnSrc, instructionSize);
		done += instructionSize;
		fnDst = (char*)fnDst + instructionSize;
		fnSrc = (char*)fnSrc + instructionSize;
	}
	return done;
}

// For Win32, this is good enough...
static void* NearAllocate(void* fnNearMe)
{
	return VirtualAlloc(NULL, MAX_FUNCTION_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

void* inject(void* fnOriginal, void* fnReplace)
{
	void* fnInter = NearAllocate(fnOriginal);
	if (fnInter == NULL)
	{
		OutputDebugString(_T("Could not allocate memory.\n"));
		return NULL;
	}

	// Copy original code to the intermediate
	const size_t jumpSize = 5; // Win32...
	size_t bytes = copy(fnInter, fnOriginal, jumpSize);
	if (bytes == 0)
	{
		VirtualFree(fnInter, 0, MEM_RELEASE);
		return NULL;
	}

	// TODO: Append the JMP instruction

	// Keep a copy of the original code and the original pointer so that uninject can restore everything.
	memcpy((char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(WORD) + sizeof(void*), fnOriginal, bytes);
	*(void**)((char*)fnInter + ORIGINAL_COPY_OFFSET) = fnOriginal;
	*(WORD*)((char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(void*)) = (WORD)bytes;

	// TODO: un-protect memory and insert a JMP to the replacement function in the original
	// TODO: Flush the CPU cache
	return fnInter;
}

void uninject(void* fnInter)
{
	WORD bytes = *(WORD*)((char*)fnInter + ORIGINAL_COPY_OFFSET);
	memcpy(
		*(void**)((char*)fnInter + ORIGINAL_COPY_OFFSET),
		(char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(WORD),
		bytes);
	VirtualFree(fnInter, 0, MEM_RELEASE);
}