#include "overlay.h"
#include <iostream>
#include <windows.h>



static HWND g_gameWnd = NULL; // static since only visible inside this c++ file

bool OverlayFindGame(const wchar_t* gameWindowTitle) {

	HWND hwnd = FindWindowW(L"SDL_APP", gameWindowTitle);

	if (hwnd == NULL) {


		DWORD error = GetLastError();
		std::cout << error;
		return false;

	}
	else {
        g_gameWnd = hwnd;

		std::wcout << L"Found Assault cube window handle: " << hwnd;

	}
	RECT rect;
	if (GetClientRect(hwnd, &rect) == 0) { 

			DWORD error = GetLastError();
			std::cout << error;
			return false;

		};


	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;



	std::wcout << L"Height: " << height << L"\nWidth " << width;


	return true;
}


static HWND g_overlayWnd = NULL;

// The overlay's inbox so to speak
static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    

    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool OverlayCreate()
{
    // 1. Describe + register the blueprint
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);

    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.lpszClassName = L"ExternalHackOverlay";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  

    if (RegisterClassExW(&wc) == 0) {
        DWORD error = GetLastError();

        std::wcout << "Failed to register class " << error;
        return false;
    };

    // 2. Where is the game on the actual monitor?
    RECT rc{};

   
    if (!GetClientRect(g_gameWnd, &rc)) {
        DWORD error = GetLastError();
        std::wcout << "Failed to retrieve window cords " << error;
        return false;
    };



    POINT topLeft = { 0, 0 };
  
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    if (ClientToScreen(g_overlayWnd, &topLeft) == 0) {
        DWORD error = GetLastError();
        std::wcout << "stupid error u idiot: \n executing: rm -rf C:/msys64 " << error;
    
    };
    



    //Build the window from the blueprint
    g_overlayWnd = CreateWindowExW(
       
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, wc.lpszClassName, L"", WS_POPUP,
        topLeft.x, topLeft.y, width, height,
        NULL, NULL, wc.hInstance, NULL);
    

    // 4. Make black see-through
    
    const COLORREF black = 0x000000000;
    SetLayeredWindowAttributes(g_overlayWnd, black, 0, LWA_COLORKEY);
    ShowWindow(g_overlayWnd, SW_SHOW);
    return true;
}
