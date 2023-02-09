#include <iostream>
#include "proc.h"
#include "mem.h"

int main()
{
	const wchar_t gameName[] = L"ac_client.exe";
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Handle for console api/manipulation

	HANDLE hProc = 0;
	uintptr_t modBaseAddr = 0, playerPtr = 0, healthAddr = 0, flyAddr = 0;
	bool bHealth{ false }, bAmmo{ false }, bRecoil{ false }, bFlyHack{ false }; // Set hacks to false

	const int new_val = 999; // Health val to insert
	const int fly_on = 5; // toggle_val = 5 to enable, value{0} disable
	const int fly_off = 0;

	DWORD procId = GetProcId(gameName); // Find pID from pname, store in unsigned 32 bit, 16 bytes , For non negative long numbers ? 

	if (procId) // if theres a process found
	{ // Open it with access, 2 dont matter, and the pID to find the process
		hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);  // Handler for the process ? / Entry ?
		modBaseAddr = GetModuleBaseAddress(procId, gameName); // pid + ac_client.exe
		playerPtr = modBaseAddr + 0x0010F4F4; // "Module base address + Local pointer in the process memory
		healthAddr = FindMAAddy(hProc, playerPtr, { 0xF8 }); // playerptr + offset F8 = health - store in mem, refer to as &healthAddr for health ptr/thing
		flyAddr = FindMAAddy(hProc, playerPtr, { 0x338 }); // Using the process do, playerptr->Offset = "Address" - store in mem

		SetConsoleTextAttribute(hConsole, 2);
		std::cout << "ac_client.exe process FOUND!\nStarting menu...\n--------------------------\n"; // all connected, let user know
	}
	else {
		SetConsoleTextAttribute(hConsole, 4);
		std::cout << "ac_client.exe process NOT FOUND!\nOpen while the ac_client.exe process is running."; // Couldnt get a process id from process name ,x86,64?
		getchar();
		return 0;
	}// Couldnt get pid to hook, kill program


	DWORD dwExit = 0; // 

	std::string health_str = "[Z] INF HEALTH \n",
		ammo_str = "[C] INF AMMO \n",
		recoil_str = "[X] NO RECOIL \n",
		fly_str = "[RSHIFT] FLY HACK \n--------------------------\n";
	std::cout << health_str << ammo_str << recoil_str << fly_str;

	// While exit code from hproc is 0 and dwexit is 0,
	while (GetExitCodeProcess(hProc, &dwExit) && dwExit == STILL_ACTIVE) // While the Handle for hProc is still open and alive
	{
		(bHealth) ? health_str += "[ENABLED] | " : health_str;
		(bAmmo) ? ammo_str += "[ENABLED] | " : ammo_str;
		(bRecoil) ? recoil_str += "[ENABLED] | " : recoil_str;


		if (GetAsyncKeyState(0x5A) & 1) // Key Z - Health
		{
			bHealth = !bHealth;
			if (bHealth)
			{
				// in memory, goto the address, take source val 999, use four bytes, in the process identified
				mem::PatchEx((BYTE*)healthAddr, (BYTE*)&new_val, sizeof(new_val), hProc);
				SetConsoleTextAttribute(hConsole, 2);
				std::cout << "Health Activated...\n";
			}
			else
			{
				SetConsoleTextAttribute(hConsole, 4);
				std::cout << "Health Deactivated...\n";
			}
		}
		if (GetAsyncKeyState(0x43) & 1) //Key C - Ammo
		{
			bAmmo = !bAmmo;

			if (bAmmo)
			{
				mem::PatchEx((BYTE*)(modBaseAddr + 0x637E9), (BYTE*)"\x90\x90", 2, hProc); // FF 06 inc [esi]
				SetConsoleTextAttribute(hConsole, 2);
				std::cout << "Infinite Ammo Activated...\n";

			}
			else
			{
				mem::PatchEx((BYTE*)(modBaseAddr + 0x637E9), (BYTE*)"\xFF\x0E", 2, hProc); // FF 0E dec [esi]
				SetConsoleTextAttribute(hConsole, 4);
				std::cout << "Infinite Ammo Deactivated...\n";
			}
		}
		if (GetAsyncKeyState(0x58) & 1)
		{
			bRecoil = !bRecoil; // if false = !false, false does not equal false, true
			if (bRecoil)
			{
				mem::NopEx((BYTE*)(modBaseAddr + 0x63786), 10, hProc);
				//mem::PatchEx((BYTE*)(modBaseAddr + 0x63786), (BYTE*)"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 10, hProc);
				SetConsoleTextAttribute(hConsole, 2);
				std::cout << "No Recoil Activated...\n";
			}
			else
			{
				/*		Recoil Function
					---------------------------------
					50			- push eax
					8D 4C 24 1C 		- lea ecx, [esp + 1C]
					51			- push ecx
					8B CE			- mov ecx, esi
					FF D2			- call edx
					---------------------------------
				*/
				mem::PatchEx((BYTE*)(modBaseAddr + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1c\x51\x88\xCE\xFF\xD2", 10, hProc); // Fancy method of un-noping the recoil, replace instructions :dead:
				SetConsoleTextAttribute(hConsole, 4);
				std::cout << "No Recoil Disabled\n";
			}
		}
		if (GetAsyncKeyState(VK_RSHIFT) & 1)
		{
			bFlyHack = !bFlyHack;

			if (bFlyHack)
			{
				mem::PatchEx((BYTE*)flyAddr, (BYTE*)&fly_on, sizeof(fly_on), hProc); // Patch destination memory with value, using size of bytes to allocate mem in the process 
				SetConsoleTextAttribute(hConsole, 2);
				std::cout << "Fly Hack Enabled...\n";
			}
			else
			{
				mem::PatchEx((BYTE*)flyAddr, (BYTE*)&fly_off, sizeof(fly_on), hProc); // Patch (Addr{edit} byte size of edit in process
				SetConsoleTextAttribute(hConsole, 4);
				std::cout << "Fly Hack Deactivated\n";
			}
		}
		if (GetAsyncKeyState(VK_NUMLOCK) & 1)
		{
			return 0;
		}
		Sleep(25);
	}
	return 0;
}