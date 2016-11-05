
#include "stdafx.h"
#include "main.h"
#include <tchar.h>
#include <windows.h>
#include <shellapi.h>				// support shell execution
#include "GkeyImplementation.h"
#include <atlbase.h>
#include "Skype.h"

static WCHAR* EnglishCmdList[] =	{ L"Kill Active Window",
									  L"Skype Hang-up",
									  L"Skype Answer",
									  L"Launch TF2",
							          NULL
							        };
WCHAR** GetGkeyCommandList(unsigned int languageCode)
{
	return EnglishCmdList;  // This application only support English language.
}
void killActiveWindow() {
	HWND wnd = GetForegroundWindow();
	DWORD processID;
	GetWindowThreadProcessId(wnd,&processID);
	HANDLE procHandle = OpenProcess(PROCESS_TERMINATE, false, processID);
	TerminateProcess(procHandle,-1);
}
BOOL RunGkeyCommand (unsigned int commandID)
{
	BOOL retVal = TRUE;

	switch (commandID)
	{
	case 0: killActiveWindow();
		break;
	case 1: //hang up on skype
		if(!threadLoaded) {
			shouldHangup = false;
			threadHandle = CreateThread(NULL,0,skypeThread,NULL,0,&threadID);
			threadLoaded = true;
		}
		shouldHangup = true;
		break;
	case 2:
		if(!threadLoaded) {
			shouldAnswer = false;
			threadHandle = CreateThread(NULL,0,skypeThread,NULL,0,&threadID);
			threadLoaded = true;
		}
		tryAnswer();
		break;
	case 3: //launch TF2
		ShellExecuteA(NULL,"open","steam://rungameid/440",NULL, 0, SW_SHOWNORMAL);
		break;
	default:
		retVal = FALSE;
		break;
	};

	return retVal;
}