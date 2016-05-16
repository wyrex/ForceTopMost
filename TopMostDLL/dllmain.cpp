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
HHOOK mouseProc = NULL;
#pragma data_seg()

//from here on variables are instanced for calling process (default)
static bool selected = false, cur_topmost = false;
HWND cur_hwnd = NULL;
HMENU cur_menu = NULL;
int cur_menu_count = 0;

//Hook window proc
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	/* If nCode is greater than or equal to HC_ACTION,
	* we should process the message. */
	if (nCode >= HC_ACTION)
	{
		const LPCWPSTRUCT lpcwprs = (LPCWPSTRUCT)lParam;
		HMENU menu = NULL;
		bool found = false;

		switch (lpcwprs->message)
		{
		case WM_INITMENUPOPUP:
			menu = GetSystemMenu(lpcwprs->hwnd, FALSE);
			if (menu == NULL)
				break;

			cur_menu = menu;
			cur_menu_count = GetMenuItemCount(menu);

			for (int i = 0; i < cur_menu_count; ++i)
			{
				MENUITEMINFO info;
				info.cbSize = sizeof(MENUITEMINFO);
				info.fMask = MIIM_ID;
				GetMenuItemInfo(menu, i, TRUE, &info);
				if (info.wID == ITEM_ID)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				AppendMenu(menu, MF_STRING | MF_UNCHECKED, ITEM_ID, L"Always on top");
				++cur_menu_count;
			}
				
			cur_hwnd = lpcwprs->hwnd;
			cur_topmost = GetWindowLong(cur_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST;

			break;
		case WM_MENUSELECT:
			if (LOWORD(lpcwprs->wParam) == ITEM_ID)
			{
				if (HIWORD(lpcwprs->wParam) & MF_SYSMENU)
				{
					selected = true; //mouse hovers on top of menu our menu item
				}
			}
			else
			{
				selected = false;
			}

			break;
		default:
			break;
		}
	}

	return CallNextHookEx(g_hhkCallWndProc, nCode, wParam, lParam);
}

//Hook mouse proc
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WM_LBUTTONDOWN:
		if (selected)
		{
			selected = false;
			if (cur_hwnd != NULL)
			{
				if (!cur_topmost)
					SetWindowPos(cur_hwnd, HWND_TOPMOST, 100,100,100,100, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
				else
					SetWindowPos(cur_hwnd, HWND_NOTOPMOST, 100, 100, 100, 100, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

				cur_topmost = !cur_topmost;

				MENUITEMINFO info;
				info.cbSize = sizeof(MENUITEMINFO);
				info.fMask = MIIM_STATE;

				if (!GetMenuItemInfo(cur_menu, ITEM_ID, FALSE, &info))
					MessageBox(NULL, L"Failed to find menu item (ForceTopMost.exe)", L"Error", MB_OK);
				
				if (info.fState & MFS_CHECKED)
				{
					info.fState = MFS_UNCHECKED;
					info.fMask = MIIM_STATE;
					SetMenuItemInfo(cur_menu, cur_menu_count-1, TRUE, &info);
				}
				else
				{
					info.fState = MFS_CHECKED;
					info.fMask = MIIM_STATE;
					SetMenuItemInfo(cur_menu, cur_menu_count - 1, TRUE, &info);
				}
			}
		}

		break;
	}

	return CallNextHookEx(mouseProc, nCode, wParam, lParam);
}

//export as C code for sane symbol names
extern "C"
{
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

		if (!mouseProc)
		{
			mouseProc = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hinstDLL, 0);
			if (!mouseProc)
			{
				MessageBox(NULL, L"Mouse Hook installation failed!", L"Error", MB_OK);
				return FALSE;
			}
		}

		MessageBox(NULL, L"Hooks installed", L"Info", MB_OK);
		return TRUE;
	}

	__declspec(dllexport) BOOL RemoveHook(void)
	{
		/* Try to remove the WH_CALLWNDPROCRET hook, if it is installed. */
		if (g_hhkCallWndProc)
		{
			if (!UnhookWindowsHookEx(g_hhkCallWndProc))
			{
				MessageBox(NULL, L"Hook uninstall failed!", L"Error", MB_OK);
				return FALSE;
			}
			g_hhkCallWndProc = NULL;
		}
		
		if (!mouseProc)
		{
			if (!UnhookWindowsHookEx(mouseProc))
			{
				MessageBox(NULL, L"Mouse Hook uninstall failed!", L"Error", MB_OK);
				return FALSE;
			}
			mouseProc = NULL;
		}
		
		MessageBox(NULL, L"Hooks removed", L"Info", MB_OK);
		return TRUE;
	}
}
