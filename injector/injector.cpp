#include <windows.h>
#include <tchar.h>
#include "injector.h"

#define MAX_FUNCTION_SIZE 4096
#define ORIGINAL_COPY_OFFSET 2048
#define TRAMPOLINE_OFFSET 512

struct SimpleInstruction
{
	DWORD  ins;
	DWORD  mask;
	size_t size;
};

// Disassembly example (HelloWorld in release mode):
// 00401350 8B 44 24 04      mov         eax,dword ptr [esp+4] 
// 00401354 85 C0            test        eax,eax 
// Helloworld in x64 mode:
// 0000000140001660 85 C9            test        ecx,ecx 
// 0000000140001662 8D 04 09         lea         eax,[rcx+rcx] 
// 000007FF798A80E0 4C 8B DC         mov         r11,rsp 
// 000007FF798A80E3 48 83 EC 58      sub         rsp,58h 

static SimpleInstruction instructions[] =
{
	{    0x50,     0xF0, 1},  // push register
	{0x24448B, 0xFFFFFF, 4},  // mov  eax,dword ptr [esp+4]
	{  0xC08B,   0xC0FF, 2},  // mov  reg,reg
	{  0xC085,   0xF0FF, 2},  // test reg,reg        (not 100% sure about the mask here)
	{0x09048D, 0xFFFFFF, 3},  // lea  eax,[rcx+rcx]  (needs investigating)
#ifdef _WIN64
	{  0xEC8348,   0xFFFFFF, 4},		// sub rsp, ?
	{0x24448948, 0xFF44FFFB, 5},		// mov qword ptr [rsp+?h],r64 (denk ik)
	{  0xC08B4C,   0xC0FFFF, 3},    // mov  r64,r64
#endif
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
#include <stdio.h>

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
				OutputDebugString(_T("Some CPU instructions not understood, aborting\n"));
				char d[128];
				unsigned char* b = (unsigned char*)fnSrc;
				sprintf(d, "Instruction dump: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", (int)b[0], (int)b[1], (int)b[2], (int)b[3], (int)b[4], (int)b[5], (int)b[6], (int)b[7]);
				OutputDebugStringA(d);
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

#ifdef _WIN64
static void* NearAllocate(void* fnNearMe)
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

	// Step through all memory blocks and find a region marked as MEM_FREE
	MEMORY_BASIC_INFORMATION memoryBlock;
	while (VirtualQuery(fnNearMe, &memoryBlock, sizeof(memoryBlock)) != 0)
	{
		fnNearMe = memoryBlock.BaseAddress;
		if (memoryBlock.State == MEM_FREE)
		{
            void* result = VirtualAlloc((char*)fnNearMe + systemInfo.dwAllocationGranularity - 1, MAX_FUNCTION_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (result != NULL)
			{
				return result;
			}
		}
		fnNearMe = (char*)fnNearMe + memoryBlock.RegionSize;
	}

	OutputDebugString(_T("Failed to find NEAR memory, cannot intercept\n"));
	return NULL;
}
#else
// For Win32, this is good enough
static void* NearAllocate(void* /*fnNearMe*/)
{
	return VirtualAlloc(NULL, MAX_FUNCTION_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}
#endif

static bool IsFarJump(void* fnDst, void* fnSrc)
{
	INT_PTR dist = (char*)fnDst - (char*)fnSrc;
	return (dist > 0x7FFFFFF) || (dist < -0x7FFFFFF);
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

	// Append the JMP instruction to the intermediate (which is always a near jump)
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

	if (IsFarJump(fnOriginal, fnReplace))
	{
		// jump to a location in the intermediate, where we'll encode a jump to the actual
		// replacement. We'll call it the trampoline for obvious reasons :)

		// TODO: Make this a 64-bit indirect jump!
		unsigned char* fnTrampoline = (unsigned char*)fnInter + TRAMPOLINE_OFFSET;
		//fnTrampoline[0] = 0xE9;
		//*(DWORD*)(fnTrampoline + 1) = (DWORD)((unsigned char*)fnReplace - (fnTrampoline + 5));
	
		fnTrampoline[0] = 0xFF; // JMP
		fnTrampoline[1] = 0x25; // mem32

		// This ifdef will be incredibly silly because you can only get here in x64 mode anyway.
		// But it's really convenient for testing whether the trampoline just works.
#ifdef _WIN64
		//x64 uses relative addressing
		*(DWORD*)(fnTrampoline + 2) = 0;
		*(void**)(fnTrampoline + 6) = fnReplace;
#else
		// x86 uses absolute addressing
		*(DWORD*)(fnTrampoline + 2) = (DWORD)(UINT_PTR)(fnTrampoline + 16);
		*(void**)(fnTrampoline + 16) = fnReplace;
#endif

		// Jump to trampoline
		jmpInstruction[0] = 0xE9; // jmp
		*(DWORD*)(jmpInstruction + 1) = (DWORD)(fnTrampoline - ((unsigned char*)fnOriginal + 5));
	}
	else
	{
		// Jump directly to replacement, it's a near jump
		jmpInstruction[0] = 0xE9; // jmp
		*(DWORD*)(jmpInstruction + 1) = (DWORD)((unsigned char*)fnReplace - ((unsigned char*)fnOriginal + 5));
	}
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