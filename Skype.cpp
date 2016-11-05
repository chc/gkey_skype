#include "stdafx.h"
#include "Skype.h"
UINT MsgID_SkypeControlAPIAttach;
UINT MsgID_SkypeControlAPIDiscover;
HWND SkypeAPIWindowHandle=NULL;
HWND hInit_MainWindowHandle;
HINSTANCE hInit_ProcessHandle;
volatile bool shouldHangup;
volatile bool shouldAnswer;
int activeCallID;
HANDLE threadHandle;
DWORD threadID;
wchar_t acInit_WindowClassName[128];
#include <time.h>
bool threadLoaded = false;
enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS=0,								// Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION=1,	// Skype has acknowledged connection request and is waiting for confirmation from the user.
																									// The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED=2,								// User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE=3,					// API is not available at the moment. For example, this happens when no user is currently logged in.
																									// Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further
																									// connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE=0x8001
};
void sendSkypeCommand(char *cmd) {
	COPYDATASTRUCT oCopyData;
	oCopyData.dwData=0;
	oCopyData.lpData=cmd;
	oCopyData.cbData=strlen(cmd)+1;
	SendMessage( SkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)hInit_MainWindowHandle, (LPARAM)&oCopyData);
}
void parseSkypeMessage(PCOPYDATASTRUCT poCopyData) {
	if(strncmp((const char *)poCopyData->lpData,"CALL",4) == 0) {
		char *id = (char *)(poCopyData->lpData)+5;
		char *s = strchr(id,' ');
		if(s != NULL) *s = 0;
		activeCallID = atoi(id);
		char hangupText[128];
		if(shouldHangup) {
			sprintf_s(hangupText,sizeof(hangupText),"SET CALL %d STATUS FINISHED",activeCallID);
			sendSkypeCommand(hangupText);
			shouldHangup = false;
		}
	}
}
static LRESULT APIENTRY WindowProc(HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM ulParam)
{
	LRESULT lReturnCode;
	bool fIssueDefProc;

	lReturnCode=0;
	fIssueDefProc=false;
	switch(uiMessage)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_COPYDATA:
			if( SkypeAPIWindowHandle==(HWND)uiParam )
				{
				PCOPYDATASTRUCT poCopyData=(PCOPYDATASTRUCT)ulParam;
				parseSkypeMessage(poCopyData);
				lReturnCode=1;
				}
			break;
		default:
			if( uiMessage==MsgID_SkypeControlAPIAttach )
				{
				switch(ulParam)
					{
					case SKYPECONTROLAPI_ATTACH_SUCCESS:
						//printf("!!! Connected; to terminate issue #disconnect\n");
						SkypeAPIWindowHandle=(HWND)uiParam;
						break;
					case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
						//printf("!!! Pending authorization\n");
						break;
					case SKYPECONTROLAPI_ATTACH_REFUSED:
						//printf("!!! Connection refused\n");
						break;
					case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
						//printf("!!! Skype API not available\n");
						break;
					case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
						//printf("!!! Try connect now (API available); issue #connect\n");
						break;
					}
				lReturnCode=1;
				break;
				}
			fIssueDefProc=true;
			break;
		}
	if( fIssueDefProc )
		lReturnCode=DefWindowProc( hWindow, uiMessage, uiParam, ulParam);
	return(lReturnCode);
}
DWORD WINAPI skypeThread( LPVOID lpParam ) {
	hInit_ProcessHandle=(HINSTANCE)OpenProcess( PROCESS_DUP_HANDLE, FALSE, GetCurrentProcessId());
	int systime = time(0);
	wsprintf(acInit_WindowClassName,L"CHCSkypeAPI-%d",systime);
	MsgID_SkypeControlAPIAttach=RegisterWindowMessage(L"SkypeControlAPIAttach");
	MsgID_SkypeControlAPIDiscover=RegisterWindowMessage(L"SkypeControlAPIDiscover");
	WNDCLASS oWindowClass;
	oWindowClass.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	oWindowClass.lpfnWndProc=(WNDPROC)&WindowProc;
	oWindowClass.cbClsExtra=0;
	oWindowClass.cbWndExtra=0;
	oWindowClass.hInstance=hInit_ProcessHandle;
	oWindowClass.hIcon=NULL;
	oWindowClass.hCursor=NULL;
	oWindowClass.hbrBackground=NULL;
	oWindowClass.lpszMenuName=NULL;
	oWindowClass.lpszClassName=acInit_WindowClassName;
	if(!RegisterClass(&oWindowClass)) {
		MessageBox(NULL,L"RegisterClass failed..",L"Error",MB_ICONSTOP|MB_OK);
	}
	hInit_MainWindowHandle=CreateWindowEx(WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,
		acInit_WindowClassName, L"CHCGKeySkype", WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, 0, 0);
	SendMessageTimeout( HWND_BROADCAST, MsgID_SkypeControlAPIDiscover, (WPARAM)hInit_MainWindowHandle, 0, SMTO_ABORTIFHUNG, 1000, NULL);
	MSG oMessage;
	shouldHangup = false;
	shouldAnswer = false;
	while(GetMessage( &oMessage, 0, 0, 0) != FALSE)
	{
		TranslateMessage(&oMessage);
		DispatchMessage(&oMessage);
	}
	return 0;
}
void tryAnswer() {
	char hangupText[128];
	sprintf_s(hangupText,sizeof(hangupText),"SET CALL %d STATUS INPROGRESS",activeCallID);
	sendSkypeCommand(hangupText);
}