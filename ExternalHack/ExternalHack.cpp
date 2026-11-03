#include <iostream>

#include "proc/proc.h"

//
//static_assert(sizeof(void*) == 4, "Build as x86, not x64: AssaultCube is a 32-bit game.");

// AssaultCube 1.2.0.2 offsets (will move to offsets.h later)
const uintptr_t OFF_LOCAL_PLAYER = 0x10F4F4;   
const uintptr_t OFF_HEALTH       = 0xF8;      

const uintptr_t OFF_ENTITY_LIST = 0X10F4F8;  //relative to module base; pointer to array of player
const uintptr_t OFF_PLAYER_COUNT = 0X10F500; // relative to module basel; int


int main()
{
	DWORD procId = getProcId(L"ac_client.exe");
	if (procId == 0)
	{
		std::wcout << L"AssaultCube isn't running.\n";
		return 1;
	}

	uintptr_t base = GetModuleBaseAddress(procId, L"ac_client.exe");

	std::wcout << L"PID:  " << procId << L"\n";
	std::wcout << L"Base: 0x" << std::hex << base << L"\n";   









	HANDLE hProc = OpenProcess(PROCESS_VM_READ, FALSE, procId);
	if (hProc == NULL)
	{
		std::wcout << L"OpenProcess failed, nuking your entire computer... error " << std::dec << GetLastError() << L"\n";
		return 1;
	}

	uintptr_t localPlayerAddr = base + OFF_LOCAL_PLAYER;     

	uintptr_t playerBase = 0;                                  
	ReadProcessMemory(hProc, (LPCVOID)localPlayerAddr, &playerBase, sizeof(playerBase), nullptr);

	int health = 0;                                           
	ReadProcessMemory(hProc, (LPCVOID)(playerBase + OFF_HEALTH), &health, sizeof(health), nullptr);

	std::wcout << L"playerBase: 0x" << std::hex << playerBase << L"\n";
	std::wcout << L"Health (B): " << std::dec << health << L"\n";


	uintptr_t healthAddr = FindDMAAddy(hProc, localPlayerAddr, { OFF_HEALTH });   
	int healthC = 0;                                                               
	ReadProcessMemory(hProc, (LPCVOID)healthAddr, &healthC, sizeof(healthC), nullptr);

	std::wcout << L"healthAddr: 0x" << std::hex << healthAddr << L"\n";
	std::wcout << L"Health (C): " << std::dec << healthC << L"\n";

	// ---------- Chunk 3a: entity list ----------
	// TODO 1: read the entity-list pointer from (base + OFF_ENTITY_LIST) into a uintptr_t entityList.

	uintptr_t entityListPointer = 0;

	ReadProcessMemory(hProc, (LPCVOID)(base + OFF_ENTITY_LIST),&entityListPointer, sizeof(entityListPointer),nullptr );
	std::wcout << L"Entity List Pointer: " << std::hex << entityListPointer << L"\n";




	// TODO 2: read the player count from (base + OFF_PLAYER_COUNT) into an int playerCount. Print it.
	
	int playerCount = 0;
	ReadProcessMemory(hProc, (LPCVOID)(base + OFF_PLAYER_COUNT), &playerCount, sizeof(playerCount), nullptr);
	std::wcout << L"Player Count: " << playerCount << L"\n";





	// TODO 3: if playerCount is < 0 or > 32, print an error, close the handle, and return 1.
	
	if (playerCount < 0 || playerCount > 32) {
	
		std::wcout << L"Player count is garbage" << std::dec << GetLastError() << L"\n";
		CloseHandle(hProc);
		return 1;
	}


	// TODO 4: for (int i = 0; i < playerCount; i++) {
	
	for (int i = 0; i < playerCount; i++) {

		uintptr_t ent = 0;
		uintptr_t slotAddr = entityListPointer + i * 4;
	
		ReadProcessMemory(hProc, (LPCVOID)slotAddr, &ent, sizeof(ent), nullptr);
		if (ent == 0) {
			continue;
		}
		int entHealth = 0;
		ReadProcessMemory(hProc, (LPCVOID)(ent + OFF_HEALTH), &entHealth, sizeof(entHealth), nullptr);

		std::wcout << "\n\n" << L"Player: " << i << L"\n" << std::hex << ent << L"\n" << L"Health: " << std::dec << entHealth << L"\n\n";
	}


	//     a. read the pointer at (entityList + i * 4) into a uintptr_t ent
	//     b. if ent == 0 → continue
	//     c. read an int from (ent + OFF_HEALTH) into entHealth
	//     d. print: i, ent (hex), entHealth (decimal)
	// }


	CloseHandle(hProc);
	return 0;
}
