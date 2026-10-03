#include <windows.h>

enum KEY
{
	SK_NULL,
	SK_EQUALS,
	SK_PAREN_OPEN,
	SK_PAREN_CLOSE,
	SK_FAV1,
	SK_FAV2,
	SK_FAV3,
	SK_FAV4,
	SK_FAV5,
	SK_ASTERISK,
	SK_PREV,
	SK_NEXT,
	SK_UP,
	SK_DOWN,
	SK_HOME,
	SK_SEARCH,
	SK_MAIL,
	SK_CALC,
	SK_VOL_UP,
	SK_VOL_DOWN,
	SK_MUTE,
	SK_PLAY
};

enum ACTION
{
	AC_KEY,
	AC_EXEC,
	AC_SCROLLDOWN,
	AC_SCROLLUP
};

typedef struct ENTRY
{
	UINT key;
	BOOL bShift;
	BOOL bCtrl;
	BOOL bAlt;
	UINT action;
	char what[255];
} Entry;

const Entry entries[] = {
	{ SK_FAV1,FALSE,FALSE,FALSE,AC_EXEC,"C:\\Windows\\system32\\cmd.exe" },
	{ SK_FAV2,FALSE,FALSE,FALSE,AC_EXEC,"C:\\Windows\\system32\\taskmgr.exe" },
	{ SK_FAV3,FALSE,FALSE,FALSE,AC_EXEC,"C:\\Dokumente\\PRIV\\Divers\\Privat.xlsx" },
	{ SK_FAV4,FALSE,FALSE,FALSE,AC_EXEC,"C:\\Dokumente\\PRIV\\Divers\\FI.xlsx" },
	{ SK_FAV5,FALSE,FALSE,FALSE,AC_EXEC,"C:\\Dokumente\\DWO\\Intern\\Firma.xlsx" },
	{ SK_EQUALS,FALSE,FALSE,FALSE,AC_KEY,"=" },
	{ SK_PAREN_OPEN,FALSE,FALSE,FALSE,AC_KEY,"(" },
	{ SK_PAREN_CLOSE,FALSE,FALSE,FALSE,AC_KEY,")" },
	{ SK_PAREN_OPEN,TRUE,FALSE,FALSE,AC_KEY,"[" },
	{ SK_PAREN_CLOSE,TRUE,FALSE,FALSE,AC_KEY,"]" },
	{ SK_PAREN_OPEN,FALSE,TRUE,FALSE,AC_KEY,"{" },
	{ SK_PAREN_CLOSE,FALSE,TRUE,FALSE,AC_KEY,"}" }
};

void SendKey(char c)
{
	INPUT i;
	ZeroMemory(&i,sizeof(INPUT));
	i.type = INPUT_KEYBOARD;
	i.ki.dwFlags = KEYEVENTF_UNICODE;
	i.ki.wScan = c;
	SendInput(1,&i,sizeof(INPUT));

	Sleep(30);

	i.ki.dwFlags = KEYEVENTF_KEYUP; 
	SendInput(1,&i,sizeof(INPUT));
}

UINT DecodeKey(BYTE data[8])
{
	switch(data[2])
	{
		case 0:
			switch(data[3])
			{
				case 103: return SK_EQUALS;
				case 182: return SK_PAREN_OPEN;
				case 183: return SK_PAREN_CLOSE;
			}

			switch(data[5])
			{
				case 5: return SK_FAV1;
				case 9: return SK_FAV2;
				case 17: return SK_FAV3;
				case 33: return SK_FAV4;
				case 65: return SK_FAV5;
				case 205: return SK_PLAY;
				case 226: return SK_MUTE;
				case 233: return SK_VOL_UP;
				case 234: return SK_VOL_DOWN;
			}
			break;

		case 1:
			switch(data[1])
			{
				case 130: return SK_EQUALS;
				case 146: return SK_ASTERISK;
			}
			break;

		case 2:
			switch(data[1])
			{
				case 33: return SK_SEARCH;
				case 35: return SK_HOME;
				case 36: return SK_PREV;
				case 37: return SK_NEXT;
				case 45: return SK_UP;
				case 46: return SK_DOWN;
				case 138: return SK_MAIL;
			}
			break;
	}

	return SK_NULL;
}

void ProcessKey(UINT k,BOOL bShift,BOOL bCtrl,BOOL bAlt)
{
	int nSize;
	const Entry* e;
	
	nSize = sizeof(entries) / sizeof(Entry);
	
	for(int i = 0;i < nSize;i++)
	{
		e = &entries[i];

		if(e->key == k && e->bAlt == bAlt && e->bCtrl == bCtrl && e->bShift == bShift)
		{
			switch(e->action)
			{
				case AC_KEY:
					SendKey(e->what[0]);
					break;

				case AC_EXEC:
					ShellExecuteA(NULL,"open",e->what,NULL,NULL,SW_SHOW);
					break; 
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{	
	const char szAppName[] = "MS4000";
	const char szWindowName[] = "MS4000Win";
	HWND hWndMain;
	MSG msg;
	UINT dataSize;
	RAWINPUTDEVICE rid;
	RAWINPUT ri;
		
	hWndMain = CreateWindowExA(0,"EDIT",szWindowName,WS_VISIBLE,0,0,0,0,HWND_MESSAGE,NULL,hInstance,0);

	if(!hWndMain)
		return 1;

	rid.usUsagePage = 12;
	rid.usUsage = 1;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hWndMain;
	
	if(!RegisterRawInputDevices(&rid,1,sizeof(RAWINPUTDEVICE)))
	{
		DestroyWindow(hWndMain);
		return 2;
	}

	while(GetMessage(&msg,NULL,0,0) != 0)
	{
		if(msg.message == WM_INPUT)
		{
			if(GetRawInputData((HRAWINPUT)msg.lParam,RID_INPUT,&ri,&dataSize,sizeof(RAWINPUTHEADER)) != (UINT)-1)
			{
				if(ri.header.dwType == RIM_TYPEHID)
				{
					if(ri.data.hid.dwSizeHid == 8)
					{
						UINT k = DecodeKey(ri.data.hid.bRawData);

						if(k != SK_NULL)
							ProcessKey(k,
								(ri.data.hid.bRawData[7] & 2) == 2,
								(ri.data.hid.bRawData[7] & 1) == 1,
								(ri.data.hid.bRawData[7] & 4) == 4);
					}
				}
			}
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyWindow(hWndMain);
	return 0;
}
