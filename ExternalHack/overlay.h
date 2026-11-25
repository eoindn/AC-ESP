#pragma once
#include <windows.h>


bool OverlayFindGame(const wchar_t* gameWindowTitle);
bool OverlayCreate();
bool OverlayFollowGame();

// Drawing: call Begin, draw boxes, call End, once per frame.
bool OverlayBeginFrame();
void OverlayEndFrame();
void OverlayDrawBox(float left, float top, float width, float height, COLORREF color);
void OverlayFillHealthBox(float left, float top, float width, float height, COLORREF color);
