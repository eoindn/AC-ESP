// Process / module lookup and pointer-chain helpers.
// Adapted from the Guided Hacking external trainer tutorial (credit this in README + report).

#include "proc.h"

#include <TlHelp32.h>   // CreateToolhelp32Snapshot, PROCESSENTRY32W, MODULEENTRY32W
#include <cwchar>       // _wcsicmp

// Get the process ID of a running exe by name.
DWORD getProcId(const wchar_t* procName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;                              

	PROCESSENTRY32W procEntry{};               // the struct itself (not a pointer), zero filled
	procEntry.dwSize = sizeof(procEntry);      // tell Windows which version of the struct we have

	DWORD procId = 0;
	if (Process32FirstW(hSnap, &procEntry))
	{
		do
		{
			if (_wcsicmp(procEntry.szExeFile, procName) == 0)
			{
				procId = procEntry.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnap, &procEntry));
	}

	CloseHandle(hSnap);                      
	return procId;
}

// Get the base address of a module so we can use offsets from it.
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	// SNAPMODULE32 
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;

	MODULEENTRY32W modEntry{};
	modEntry.dwSize = sizeof(modEntry);
	
	uintptr_t modBaseAddr = 0;                 
	if (Module32FirstW(hSnap, &modEntry))
	{
		do
		{
			if (_wcsicmp(modEntry.szModule, modName) == 0)
			{
				modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
				break;
			}
		} while (Module32NextW(hSnap, &modEntry));
	}

	CloseHandle(hSnap);
	return modBaseAddr;
}

// Follow a pointer chain: [[[ptr] + off0] + off1] 
uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, const std::vector<unsigned int>& offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); i++)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), nullptr);  
		addr += offsets[i];
	}
	return addr;
}
