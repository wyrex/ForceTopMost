#define WIN32_LEAN_AND_MEAN
#define ITEM_ID 0x53C8

#include <Windows.h>
#include <string>

HINSTANCE g_hinstDLL = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hinstDLL = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		g_hinstDLL = NULL;
		break;
	}
	return TRUE;
}

//store handles to hooks as globals that are shared between processes calling dll
#pragma comment(linker, "/section:.SHARED,rws")
#pragma data_seg(".SHARED") 
HHOOK g_hhkCallWndProc = NULL;
HHOOK g_hhkGetMsgProc = NULL;
#pragma data_seg()

//window proc
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	/* If nCode is greater than or equal to HC_ACTION,
	* we should process the message. */
	if (nCode >= HC_ACTION)
	{
		const LPCWPSTRUCT lpcwprs = (LPCWPSTRUCT)lParam;
		HMENU menu = NULL;
		BOOL found = false;
		BOOL checked = false;
		UINT state;

		switch (lpcwprs->message)
		{
		case WM_INITMENUPOPUP:
			// If the menu is the window menu, this parameter is TRUE; 
			if ((BOOL)HIWORD(lpcwprs->lParam) == FALSE) 
				break;

			menu = GetSystemMenu(lpcwprs->hwnd, FALSE);
			if (menu == NULL)
				break;

			checked = (GetWindowLongPtr(lpcwprs->hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;

			if (state = GetMenuState(menu, ITEM_ID, MF_BYCOMMAND) == -1)
			{
				MENUITEMINFO info;
				info.cbSize = sizeof(info);
				info.fMask = MIIM_CHECKMARKS | MIIM_STATE | MIIM_ID | MIIM_STRING;
				info.hbmpChecked = NULL;
				info.hbmpUnchecked = NULL;
				info.wID = ITEM_ID;
				info.dwTypeData = L"Always on top";
				info.fState = checked ? MF_CHECKED : MF_UNCHECKED;
				InsertMenuItem(menu, SC_CLOSE, FALSE, &info);
			}

			break;
		case WM_UNINITMENUPOPUP:
			if ((HIWORD(lpcwprs->lParam) & MF_SYSMENU) != 0)
			{
				DeleteMenu((HMENU)lpcwprs->wParam, ITEM_ID, MF_BYCOMMAND);
			}	

			break;
		default:
			break;
		}
	}

	return CallNextHookEx(g_hhkCallWndProc, nCode, wParam, lParam);
}

//thread message queue
LRESULT CALLBACK GetMsgProc(INT code, WPARAM wParam, LPARAM lParam)
{
	LPMSG msg = (LPMSG)lParam;
	
	if (code == HC_ACTION)
	{
		if (msg->message == WM_SYSCOMMAND && msg->wParam == ITEM_ID)
		{
			SetWindowPos(msg->hwnd, (GetWindowLongPtr(msg->hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0 ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			SetFocus(msg->hwnd);
		}	
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

//export as C code for sane symbol names
extern "C"
{
	__declspec(dllexport) void Pump(void)
	{
		MSG msg;
		BOOL ret;

		//message pump
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	__declspec(dllexport) BOOL InstallHook(void)
	{
		if (!g_hhkCallWndProc)
		{
			g_hhkCallWndProc = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstDLL, 0);
			if (!g_hhkCallWndProc)
			{
				MessageBox(NULL, L"WndProc Hook installation failed!", L"Error", MB_OK);
				return FALSE;
			}
		}
		
		if (!g_hhkGetMsgProc)
		{
			g_hhkGetMsgProc = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstDLL, 0);
			if (!g_hhkGetMsgProc)
			{
				MessageBox(NULL, L"MsgProc Hook installation failed!", L"Error", MB_OK);
				return FALSE;
			}
		}

		return TRUE;
	}

	__declspec(dllexport) BOOL RemoveHook(void)
	{
		if (g_hhkCallWndProc)
		{
			if (!UnhookWindowsHookEx(g_hhkCallWndProc))
			{
				MessageBox(NULL, L"Hook remove failed!", L"Error", MB_OK);
				return FALSE;
			}
			g_hhkCallWndProc = NULL;
		}

		if (g_hhkGetMsgProc)
		{
			if (!UnhookWindowsHookEx(g_hhkGetMsgProc))
			{
				MessageBox(NULL, L"MsgProc hook remove failed!", L"Error", MB_OK);
				return FALSE;
			}
			g_hhkGetMsgProc = NULL;
		}
		
		return TRUE;
	}
}
