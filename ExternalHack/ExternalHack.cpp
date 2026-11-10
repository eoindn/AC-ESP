#include <iostream>

#include "proc/proc.h"
#include <iomanip>

static_assert(sizeof(void*) == 4, "Build as x86, not x64: AssaultCube is a 32-bit game.");

// AssaultCube 1.2.0.2 offsets (will move to offsets.h later)
const uintptr_t OFF_LOCAL_PLAYER = 0x10F4F4;   
const uintptr_t OFF_HEALTH       = 0xF8;      
const uintptr_t OFF_FEET_POS = 0X34;
struct Vec3 {
	float x;
	float y;
	float z;
};
const uintptr_t OFF_MATRIX = 0x101AE8;
const uintptr_t OFF_TEAM = 0x32C;
const uintptr_t OFF_NAME = 0x225;

const uintptr_t OFF_ENTITY_LIST = 0X10F4F8;  //relative to module base; pointer to array of player
const uintptr_t OFF_PLAYER_COUNT = 0X10F500; // relative to module base; int


struct Vec2 {
	float x;
	float y;
};
bool WorldToScreen(Vec3 pos, Vec2& screen, const float m[16], int width, int height) {


	// stupid maths

	float clipX = pos.x * m[0] + pos.y * m[4] + pos.z * m[8] + m[12];
	float clipY = pos.x * m[1] + pos.y * m[5] + pos.z * m[9] + m[13];
	float clipW = pos.x * m[3] + pos.y * m[7] + pos.z * m[11] + m[15];


	// is it behind the camera

	if (clipW < 0.1f) {
		return false;
	};






}





















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


	// read the entity-list pointer from (base + OFF_ENTITY_LIST) into a uintptr_t entityList.

	uintptr_t entityListPointer = 0;

	ReadProcessMemory(hProc, (LPCVOID)(base + OFF_ENTITY_LIST),&entityListPointer, sizeof(entityListPointer),nullptr );
	std::wcout << L"Entity List Pointer: " << std::hex << entityListPointer << L"\n";




	// read the player count from (base + OFF_PLAYER_COUNT) into an int playerCount. Print it.
	
	int playerCount = 0;
	ReadProcessMemory(hProc, (LPCVOID)(base + OFF_PLAYER_COUNT), &playerCount, sizeof(playerCount), nullptr);
	std::wcout << L"Player Count: " << playerCount << L"\n\n\n";





	//if playerCount is < 0 or > 32, print an error, close the handle, and return 1.
	
	if (playerCount < 0 || playerCount > 32) {
	
		std::wcout << L"Player count is garbage: " << std::dec << playerCount << L"\n";
		CloseHandle(hProc);
		return 1;
	}



	// ---- Live display setup ----
	// Instead of printing new lines forever (scrolling), each frame we move the
	// console cursor back to the same spot and overwrite the old frame in place.
	// hConsole is OUR console's output handle. GetStdHandle doesn't open anything
	// new, so we must NOT CloseHandle it (unlike hProc, which we opened ourselves).
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE || hConsole == NULL)
	{
		std::wcout << L"GetStdHandle failed, error " << std::dec << GetLastError() << L"\n";
		CloseHandle(hProc);
		return 1;
	}

	// Remember where the cursor is right now (just below the startup info).
	// Every frame gets drawn starting from this position.
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
	{
		std::wcout << L"GetConsoleScreenBufferInfo failed, error " << std::dec << GetLastError() << L"\n";
		CloseHandle(hProc);
		return 1;
	}
	COORD frameStart = csbi.dwCursorPosition;

	// Hide the blinking cursor so it doesn't flicker around while we redraw.
	CONSOLE_CURSOR_INFO cursorInfo{};
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	while (true) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;   // leaves the loop, falls through to CloseHandle below
		}

		// Jump back to the top of the frame and draw over the previous one.
		SetConsoleCursorPosition(hConsole, frameStart);


		// get view matrix


		float matrix[16]{};
		ReadProcessMemory(hProc, (LPCVOID)(base + OFF_MATRIX), &matrix, sizeof(matrix), nullptr);




		std::wcout << L"View Matrix: " << L"\n";

		std::wcout << std::fixed << std::setprecision(2);




		for (int i = 0; i < 16; i++) {

			std::wcout << std::setw(8) << matrix[i];

			if ((i + 1) % 4 == 0) {
				std::wcout << L"\n";
			}


		}
		std::wcout << L"\n\n";




		for (int i = 0; i < playerCount; i++) {

			uintptr_t ent = 0;
			uintptr_t slotAddr = entityListPointer + i * 4;

			ReadProcessMemory(hProc, (LPCVOID)slotAddr, &ent, sizeof(ent), nullptr);
			if (ent == 0) {
				// Print a blank line instead of skipping, so a line left over
				// from an earlier frame gets wiped rather than staying on screen.
				std::wcout << std::setw(70) << L"" << L"\n";
				continue;
			}
			int entHealth = 0;
			ReadProcessMemory(hProc, (LPCVOID)(ent + OFF_HEALTH), &entHealth, sizeof(entHealth), nullptr);


			Vec3 feet{};


			ReadProcessMemory(hProc, (LPCVOID)(ent + OFF_FEET_POS), &feet, sizeof(feet), nullptr);


			int team = 0;

			ReadProcessMemory(hProc, (LPCVOID)(ent + OFF_TEAM), &team, sizeof(team), nullptr);
			char name[16]{};

			ReadProcessMemory(hProc, (LPCVOID)(ent + OFF_NAME), &name, sizeof(name), nullptr);
			name[15] = '\0';



			std::wcout << std::dec << i << L" | "

				<< L"name " << name << L" | "
				<< L"team " << team << L" | "
				<< entHealth << L"hp | "
				<< feet.x << L" " << feet.y << L" " << feet.z
				<< L"          \n";   // trailing spaces wipe leftovers if this line got shorter





		
		}



	










		Sleep(100);   // once per frame (~10 updates/sec), not once per player
	}

	// Give the user their cursor back before exiting.
	cursorInfo.bVisible = TRUE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	CloseHandle(hProc);
	return 0;





	
}
