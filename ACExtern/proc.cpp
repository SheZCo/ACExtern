#include "proc.h"
#include <iostream>

DWORD GetProcId(const wchar_t *procName)
{
	DWORD procId = 0; // inits procId to DWORD, Windows unsigned 32 bit value, stores pid of process
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Makes Snapshot of all running processes, TH32_CS  means snapshot should include ALL processes in system , 0 means return snap for current process
	if (hSnap != INVALID_HANDLE_VALUE) // If handle is invalid, no snapshot made, return 0
	{
		PROCESSENTRY32 procEntry; // pE -> PE, used to store info about a process
		procEntry.dwSize = sizeof(PROCESSENTRY32); // Sets size of pE to PE

		if (Process32First(hSnap, &procEntry)) // takes info about process using P32, hSnap is handle to snapshot, &pE ptr to PE structure
		{
			do
			{

				if (!wcscmp(procEntry.szExeFile, procName)) // compares p name in pE.sz with p name passed as param in function
				{
					procId = procEntry.th32ProcessID; // if match, pId = pId of wanted process, stores in .th32ProcessID
					break; // breaks out of loop if it worked
				}
			} while (Process32Next(hSnap, &procEntry)); // continues loop through process list for next process
		}
	}
	CloseHandle(hSnap); // Close handle cus not needed to run nymore

	return procId; // return process id for wanted process
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) // Module Base Addr = procId + process name, return basically  
{ // just the game entry point which gets added to pointer to look like, ac_client.exe + 0x0010F4F4, = playerptr


	uintptr_t moduleBaseAddress = 0; // pointer value stored as mem add which is int val, ptr,int have dif size requirements, 32 bit uintptr_t is same as unsigned int, 64 unsigned long long , way to store ptr without size issue
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId); // hSnap handle to look for all modules with process id
	if (hSnap != INVALID_HANDLE_VALUE) // hsnap = right handle?
	{
		MODULEENTRY32 moduleEntry; // Store the process in Mod32 process handler
		moduleEntry.dwSize = sizeof(MODULEENTRY32); // set to sizeof mod32 to indicate structure size to Module32First
		// set size because structure can be different so set to corrct size mean function retrive all info about module regardless of windows version

		if (Module32First(hSnap, &moduleEntry)) // if the func returned true cus it found module in the snapshot, info now stored in modet32 pointed at by modEntry
		{
			do
			{ // case insensitive comparison between towo wide char strings
				if (!_wcsicmp(moduleEntry.szModule, modName)) //! negates result of comparison so if =, true
				{
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr; // if = modBA = val of ModEntry.Base which is cast to uintptr_t to march modBase type
					break;
				}
			} while (Module32Next(hSnap, &moduleEntry)); // continue loop, call mod32 as long as its non zero, updates modEntry structure with info about next module
		} // when no more modules, return 0 & kill loo
	}
	CloseHandle(hSnap);
	return moduleBaseAddress;
}

//  1 handle to mems process, 2 starting address, 3 array of ints that need to be applied to ptr 
uintptr_t FindMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) // goto pointer with offset(s) in process // find memaddr of object in other process
{
	uintptr_t addr = ptr; // while i less than offset array, do process one more time for offset
	for (unsigned int i = 0; i < offsets.size(); i++) 
	{ //					returns the val of mem addr in &addr
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0); //Handle->addr to read->buffer to recv val->sizeof buff, and 0.
		addr += offsets[i];
		// playerPtr = modBaseAddr + 0x0010F4F4;
	}
	return addr; // returns addr, mfmory addr of object being seeked
}