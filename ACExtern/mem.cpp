#include "mem.h"

void mem::PatchEx(BYTE* dest, BYTE* src, unsigned int size, HANDLE hProc) // Address with offset loaded, dMAA, edited value, sizeof(edited) value for bytes ? and process 
{
	DWORD oldprotect; // old protect is a 32 bit positive integer
	VirtualProtectEx(hProc, dest, size, PAGE_EXECUTE_READWRITE, &oldprotect); // changing protection on memory, allocating size of 32 bit int for edited values, whatnot
	WriteProcessMemory(hProc, dest, src, size, nullptr); // write memory at the address in process using the given value and size being how much memory, dont output # of bytes written
	VirtualProtectEx(hProc, dest, size, oldprotect, &oldprotect); // Change the protection again to finalize edit
					// Protect destination in proc with size(es)
}

void mem::NopEx(BYTE* dest, unsigned int size, HANDLE hProc) // Nop an arrar at destiniation with size in process
{
	BYTE* nopArray = new BYTE[size]; // new array of bytes with *size, assign
	memset(nopArray, 0x90, size); // memset used to fill array with NOP instruct with 0x90
	PatchEx(dest, nopArray, size, hProc); // array used to patch to mem in process
	delete[] nopArray; // delete is used to free up memory that was allocated using new[]
	// nop opcode - 0x90 for intel x86 CPU family(https://en.wikipedia.org/wiki/NOP_(code))
}

// Im not sure how well the functions work seeing as though offsets and addresses are not right, cool concept though