Day 1 03/11/2026

Go through running processes with 


DWORD getProcId(const wchar_t* procName)
{

	DWORD procId = 0;
	HANDLE hsnap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

	if (hsnap != INVALID_HANDLE_VALUE) {
		
		PPROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hsnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry->szExeFile, procName))
				{
					procId = procEntry->th32ProcessID;
					break
				}
			}while (Process32Next(hsnap, &procEntry))

		}
		return procId;
		CloseHandle(hsnap)
	}

}


windows api create toolhelp32snapshot
loop through offstes to get to the dynamic module address




