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

static size_t copyCorrected(void* fnDst, void* fnSrc)
{
	INT_PTR distance = (unsigned char*)fnSrc - (unsigned char*)fnDst;
	if ((*(unsigned char*)fnSrc & 0xE8) == 0xE8) // E9=jmp, E8=call
	{
        *(unsigned char*)fnDst = *(unsigned char*)fnSrc;
        *(int*)((unsigned char*)fnDst + 1) = *(int*)((unsigned char*)fnSrc + 1) + (int)(distance);
		return 5;
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
			instructionSize = copyCorrected(fnDst, fnSrc);
			if (instructionSize == 0)
			{
				return 0;
			}
		}
		else
		{
			memcpy(fnDst, fnSrc, instructionSize);
		}
		done += instructionSize;
		fnDst = (char*)fnDst + instructionSize;
		fnSrc = (char*)fnSrc + instructionSize;
	}
	return done;
}

// For Win32, this is good enough. For x64, find a spot near the current code.
// Hint: VirtualQuery and GetSystemInfo
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

	// Keep a copy of the original code and the original pointer so that uninject can restore everything.
	memcpy((char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(WORD) + sizeof(void*), fnOriginal, bytes);
	*(void**)((char*)fnInter + ORIGINAL_COPY_OFFSET) = fnOriginal;
	*(WORD*)((char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(void*)) = (WORD)bytes;

	// Append the JMP instruction to the intermediate
	unsigned char* jmpInstruction;
	jmpInstruction = (unsigned char*)fnInter + bytes;
	jmpInstruction[0] = 0xE9; // jmp
	*(DWORD*)(jmpInstruction + 1) = (DWORD)((unsigned char*)fnOriginal + bytes - (jmpInstruction + 5));

	// Tell CPU that we changed some code
	FlushInstructionCache(GetCurrentProcess(), fnInter, MAX_FUNCTION_SIZE);

	// un-protect memory and insert a JMP to the replacement function in the original
	DWORD previousVirtualProtect;
    if (!VirtualProtect(fnOriginal, MAX_FUNCTION_SIZE, PAGE_EXECUTE_READWRITE, &previousVirtualProtect))
	{
		VirtualFree(fnInter, 0, MEM_RELEASE);
		return NULL;
	}
	
	// Insert jump to replacement
	jmpInstruction = (unsigned char*)fnOriginal;
	jmpInstruction[0] = 0xE9; // jmp
	*(DWORD*)(jmpInstruction + 1) = (DWORD)((unsigned char*)fnReplace - ((unsigned char*)fnOriginal + 5));
	// Fill remainder with NOP to help the disassembly viewer.
	jmpInstruction += 5;
	for (int i = (int)(bytes-5); i != 0; --i)
	{
		*jmpInstruction = 0x90;
		++jmpInstruction;
	}

	// Flush the CPU cache
	FlushInstructionCache(GetCurrentProcess(), fnOriginal, bytes);

	return fnInter;
}

void uninject(void* fnInter)
{
	// Copy original code back from intermediate
	void* fnOriginal = *(void**)((char*)fnInter + ORIGINAL_COPY_OFFSET);
	WORD bytes = *(WORD*)((char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(void*));
	memcpy(
		fnOriginal,
		(char*)fnInter + ORIGINAL_COPY_OFFSET + sizeof(void*) + sizeof(WORD),
		bytes);
	FlushInstructionCache(GetCurrentProcess(), fnOriginal, bytes);
	VirtualFree(fnInter, 0, MEM_RELEASE);
}