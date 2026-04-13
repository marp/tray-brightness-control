#pragma once
#include <windows.h>
#include <string>
#include "Main.h"
#include "Monitor.h"
#include "Resource.h"

class GUI
{
	public: 
		HFONT hNormalFont; // Normal font
		HFONT hBoldFont; // Bold font
		HMENU hTrayMenu;

		Configuration* configuration;
		GUI();
		~GUI();
		BOOL SetTooltip(LPCWSTR szTooltip);
		BOOL AddTaskBarIcon(BOOL modify = false, INT8 passedBrightness = -1);
		BOOL UpdateTaskBarIcon(INT8 passedBrightness = -1);
		BOOL CreateTaskBarMenu();
		BOOL UpdateTaskBarMenu(WORD passedBrightness = -1);
		VOID CreateMainWindowControls();
		VOID DestroyTrayMenu();

//STATIC
		static HINSTANCE hInst;
		static HWND hWnd;
		static HBITMAP IconToBitmap(HICON);
		BOOL AddIconToMenuItem(HMENU hMenu, UINT itemID, HICON hIcon);
		static VOID AdjustWindowSizeForDPI(HWND);
};

