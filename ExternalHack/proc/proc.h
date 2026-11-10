#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>

// Returns the PID of the first process whose exe name matches (case-insensitive), or 0 if not found.
DWORD getProcId(const wchar_t* procName);

// Returns the base address of a module (exe/dll) loaded in the given process, or 0 if not found.
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

// Follows a multi-level pointer chain in another process.
// Start at ptr, then for each offset: read the address stored there, add the offset.
uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, const std::vector<unsigned int>& offsets);
