
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>
#include <windows.h>
DWORD WINAPI skypeThread( LPVOID lpParam );
extern volatile bool shouldHangup;
extern volatile bool shouldAnswer;
extern int activeCallID;
extern bool threadLoaded;
extern HWND hInit_MainWindowHandle;
extern wchar_t acInit_WindowClassName[128];
void tryAnswer();