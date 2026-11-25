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
    //wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

    // 4. Make black see-through
    if (ClientToScreen(g_gameWnd, &topLeft) == 0) {
        DWORD error = GetLastError();
        std::wcout << "stupid error u idiot: \n executing: rm -rf C:/msys64 " << error;
        return false;
    };


    //Build the window from the blueprint
    g_overlayWnd = CreateWindowExW(
       
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, wc.lpszClassName, L"", WS_POPUP,
        topLeft.x, topLeft.y, width, height,
        NULL, NULL, wc.hInstance, NULL);
    if (g_overlayWnd == NULL) {
        DWORD error = GetLastError();
        std::wcout << "Error creating window: " << error;
        return false;
    }



    
    const COLORREF black = 0x000000000;
    if (SetLayeredWindowAttributes(g_overlayWnd, black, 0, LWA_COLORKEY) == false) {
        
        DWORD error = GetLastError();
        std::wcout << "Error configuring window " << error;
        return false;
    
    
    };
    ShowWindow(g_overlayWnd, SW_SHOWNOACTIVATE);
    return true;
}


bool OverlayFollowGame() {
    
    RECT rect{};

    if(GetClientRect(g_gameWnd, &rect) == 0) {

        DWORD error = GetLastError();
        std::wcout << "Failed to set draw rect: " << error;
        return false;

    };
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    POINT topLeft = { 0,0 };
    if (ClientToScreen(g_gameWnd, &topLeft) == 0) {
        
        DWORD error = GetLastError();
        std::wcout << "stupid error u idiot: \n executing: rm -rf C:/msys64 " << error;
        return false;
    };


    if (SetWindowPos(g_overlayWnd, HWND_TOPMOST, topLeft.x, topLeft.y, width, height, SWP_NOACTIVATE) == 0) {

        DWORD error = GetLastError();
        std::wcout << "Failed to set window position: " << error;
        return false;
    
    };
    return true;




}


// ===================== DRAWING (double buffered GDI) =====================
// These only live between OverlayBeginFrame and OverlayEndFrame.
// static = private to this file, main never sees an HDC.
static HDC     g_screenDC  = NULL;   // BORROWED canvas of the real overlay window
static HDC     g_memDC     = NULL;   // OUR hidden canvas (we create it, we delete it)
static HBITMAP g_bitmap    = NULL;   // the "paper" inside the hidden canvas
static HBITMAP g_oldBitmap = NULL;   // the paper memDC came with, must be put back before cleanup
static int     g_width     = 0;
static int     g_height    = 0;

bool OverlayBeginFrame()
{
 

    RECT rc{};
    if (!GetClientRect(g_overlayWnd, &rc)) {
        DWORD error = GetLastError();
        std::wcout << "Failed to retrieve window cords " << error;
        return false;
    };
    g_width = rc.right - rc.left;
    g_height = rc.bottom - rc.top;



    g_screenDC = GetDC(g_overlayWnd);
    if (g_screenDC == NULL) {
        DWORD error = GetLastError();
        std::wcout << "Error device context: " << error;
        return false;
    }

    g_memDC = CreateCompatibleDC(g_screenDC);
    if (g_memDC == NULL) {
        DWORD error = GetLastError();
        std::wcout << "Error creating device context: " << error;
        return false;
    }

    g_bitmap = CreateCompatibleBitmap(g_screenDC, g_width, g_height);
    if (g_bitmap == NULL) {
        DWORD error = GetLastError();
        std::wcout << "Error creating bitmap: " << error;
        return false;
    }


    g_oldBitmap = (HBITMAP)SelectObject(g_memDC, g_bitmap);
    if (g_oldBitmap == NULL) {
        DWORD error = GetLastError();
        std::wcout << "Error selecting object: " << error;
        return false;
    
    }

    RECT r = { 0, 0, g_width, g_height };
    FillRect(g_memDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));



    return true;
}

void OverlayEndFrame()
{
    BitBlt(g_screenDC, 0, 0, g_width, g_height, g_memDC, 0, 0, SRCCOPY);


    
    SelectObject(g_memDC, g_oldBitmap);   
    DeleteObject(g_bitmap);
    DeleteDC(g_memDC);                    
    ReleaseDC(g_overlayWnd, g_screenDC);  
}

// Draws a 1px outline into the hidden canvas (shows up on screen at OverlayEndFrame).
void OverlayDrawBox(float left, float top, float width, float height, COLORREF color)
{
    // RECT holds whole pixels (LONG), so the casts make the rounding deliberate.
    RECT r = { (LONG)left, (LONG)top, (LONG)(left + width), (LONG)(top + height) };

    HBRUSH brush = CreateSolidBrush(color);   // GDI object: we create it, we delete it
    if (brush == NULL) {
        return;   // skip this box rather than draw with a bad handle
    }

    FrameRect(g_memDC, &r, brush);
    DeleteObject(brush);
}

// Same as OverlayDrawBox, but fills the whole rectangle (used for the health bar).
void OverlayFillHealthBox(float left, float top, float width, float height, COLORREF color)
{
    RECT r = { (LONG)left, (LONG)top, (LONG)(left + width), (LONG)(top + height) };

    HBRUSH brush = CreateSolidBrush(color);
    if (brush == NULL) {
        return;
    }

    FillRect(g_memDC, &r, brush);
    DeleteObject(brush);
}