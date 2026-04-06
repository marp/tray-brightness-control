#include "GUI.h"

// Define static members
HINSTANCE GUI::hInst = nullptr;
HWND GUI::hWnd = nullptr;

GUI::GUI()
{
    this->hNormalFont = CreateFont(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));

	this->hBoldFont = CreateFont(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
}

GUI::~GUI()
{
	if (hTrayMenu) {
		DestroyMenu(hTrayMenu);
	}
}

BOOL GUI::AddTaskBarIcon(BOOL modify, INT8 passedBrightness)
{
    NOTIFYICONDATA nid = { 0 };

    std::wstring sTip;
    sTip.reserve(50);
    sTip = L"Average brightness: ";
    INT8 avgBrightness = passedBrightness != -1 ? passedBrightness : Monitor::GetCurrentAverageBrightnessOfAll();
    if (avgBrightness < 0)
    {
        sTip = L"Couldn't read average brightness";
    }
    else
    {
        sTip.append(std::to_wstring(avgBrightness));
        sTip.append(L"%");
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = GUI::hWnd;
    nid.uID = ID_TRAY1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = CMSG_TRAY1;

    // Determine the appropriate icon based on the average brightness
    INT8 iconIndex = 0;
    if (avgBrightness >= 90) {
        iconIndex = 5;
    }
    else if (avgBrightness >= 60) {
        iconIndex = 4;
    }
    else if (avgBrightness >= 40) {
        iconIndex = 3;
    }
    else if (avgBrightness >= 30) {
        iconIndex = 2;
    }
    else if (avgBrightness >= 10) {
        iconIndex = 1;
    }
    else
    {
        iconIndex = 1;
    }

    OutputDebugString(L"Icon index: " + iconIndex);


    // Load the appropriate icon resource
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MOON0 + iconIndex));

    if (!nid.hIcon) {  // If the icon fails to load, use the default application icon
        OutputDebugString(L"Failed to load icon");
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    lstrcpy(nid.szTip, sTip.c_str());

    BOOL res = Shell_NotifyIcon(modify?NIM_MODIFY:NIM_ADD, &nid);

    if (nid.hIcon)
        DestroyIcon(nid.hIcon);

    return res;
}

BOOL GUI::UpdateTaskBarIcon(INT8 passedBrightness) {
	return this->AddTaskBarIcon(true, passedBrightness);
}

BOOL GUI::SetTooltip(LPWSTR szTooltip) {
	if (szTooltip == NULL) {
		return FALSE;
	}
	// Allocate memory for the tooltip string
    //szTooltip = std::wstring;
	//if (szTooltip == NULL) {
		//return FALSE;
	//}
	// Copy the tooltip string
	//lstrcpy(szTooltip, szTooltip);
	// Set the tooltip text
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = GUI::hWnd;
	nid.uID = ID_TRAY1;
	nid.uFlags = NIF_TIP;
	lstrcpy(nid.szTip, szTooltip);
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL GUI::CreateTaskBarMenu()
{
    this->hTrayMenu = CreatePopupMenu();

    INT8 averageBrightness = Monitor::GetCurrentAverageBrightnessOfAll();

    std::vector<MenuValue> menuValues = this->configuration->GetContextMenuValues();
    if (!menuValues.empty()) {
        // Populate the tray menu with items read from the INI file
        for (size_t i = 0; i < menuValues.size(); ++i) {
            std::wstring menuItemText = menuValues[i].text;
            WORD val = menuValues[i].value;

            UINT flags = MF_STRING | (val == (WORD)averageBrightness ? MF_CHECKED : 0);
            AppendMenu(hTrayMenu, flags, IDM_TRAY_CONTEXTMENU + i, menuItemText.c_str());
        }
    }
    AppendMenu(hTrayMenu, MF_SEPARATOR, 0, 0);
    WCHAR szExit[MAX_LOADSTRING];
    LoadString(hInst, IDS_EXIT, szExit, MAX_LOADSTRING);

    // Create a menu item with the icon
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING | MIIM_ID | MIIM_BITMAP;
    mii.wID = IDM_EXIT;
    mii.dwTypeData = szExit;

    // Load the exit icon from resources
    HICON hExitIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EXIT));
    if (hExitIcon) {
        mii.hbmpItem = IconToBitmap(hExitIcon); // Helper function to convert HICON to HBITMAP
    }

    return InsertMenuItem(hTrayMenu, IDM_EXIT, FALSE, &mii);
}

BOOL GUI::UpdateTaskBarMenu(INT8 passedBrightness) {
    if (!this->hTrayMenu) {
        return FALSE;
    }

    INT8 averageBrightness = passedBrightness != -1 ? passedBrightness : Monitor::GetCurrentAverageBrightnessOfAll();
    std::vector<MenuValue> menuValues = this->configuration->GetContextMenuValues();

    for (size_t i = 0; i < menuValues.size(); ++i) {
        WORD val = menuValues[i].value;
        UINT checkState = MF_BYCOMMAND | (val == (WORD)averageBrightness ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(this->hTrayMenu, IDM_TRAY_CONTEXTMENU + i, checkState);
    }

    return TRUE;
}

HBITMAP GUI::IconToBitmap(HICON hIcon)
{
    // Create a 16x16 device-independent bitmap (DIB)
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 16;
    bmi.bmiHeader.biHeight = 16;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; // 32-bit for transparency
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create a DIB section
    void* pvBits;
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    SelectObject(hdcMem, hBitmap);

    // Draw the icon into the 16x16 bitmap
    DrawIconEx(hdcMem, 0, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);

    // Cleanup
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return hBitmap;
}

BOOL GUI::AddIconToMenuItem(HMENU hMenu, UINT itemID, HICON hIcon)
{
    // Convert the icon to a 16x16 bitmap
    HBITMAP hBitmap = GUI::IconToBitmap(hIcon);

    if (!hBitmap) {
        return FALSE; // Failed to create the bitmap
    }

    // Prepare the MENUITEMINFO structure
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = hBitmap;

    // Set the bitmap to the specified menu item
    return SetMenuItemInfo(hMenu, itemID, FALSE, &mii);
}

VOID GUI::AdjustWindowSizeForDPI(HWND hWnd)
{
    // Get the DPI for the primary monitor
    UINT dpi = GetDpiForWindow(hWnd);

    // Calculate the scaling factor
    float scalingFactor = dpi / 96.0f;

    // Get the current window size
    RECT rect;
    GetWindowRect(hWnd, &rect);

    // Calculate the new window size
    int newWidth = static_cast<int>((rect.right - rect.left) * scalingFactor);
    int newHeight = static_cast<int>((rect.bottom - rect.top) * scalingFactor);

    // Adjust the window size
    SetWindowPos(hWnd, NULL, rect.left, rect.top, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE);
}

VOID GUI::CreateMainWindowControls() {
    HMENU hMenu = GetMenu(GUI::hWnd);  // Get the handle to the main window menu

    if (hMenu)
    {
        // Load the exit icon
        HICON hExitIcon = LoadIcon(GUI::hInst, MAKEINTRESOURCE(IDI_EXIT));

        if (hExitIcon)
        {
            // Add the icon to the "Exit" menu item
            AddIconToMenuItem(hMenu, IDM_EXIT, hExitIcon);
        }
    }

    HWND hText;

    DWORD yPosition = 0;
    DWORD xPosition = 0;
    DWORD height = 25;
    DWORD width = 225;
    DWORD margin = 5;

    ///////////////////
    //MONITORS GROUP//
	/////////////////
    for (Monitor* monitor : Monitor::monitors)
    {
        yPosition++;
        //OutputDebugString(monitor->getNameW());
        height = 20;

        DWORD y = 5 + height * yPosition;

        hText = CreateWindow(
            L"STATIC",                  
            monitor->getNameW(),                          
            WS_CHILD | WS_VISIBLE,  
            20, y,                      
            120, 20,                     
            GUI::hWnd,                        
            NULL,                        
            GUI::hInst,                       
            NULL);                       
        SendMessage(hText, WM_SETFONT, (WPARAM)hBoldFont, 0);

        hText = CreateWindow(
            L"STATIC",                   
            L"Brightness: ",                           
            WS_CHILD | WS_VISIBLE | SS_LEFT,  
            140, y,                      
            100, 20,                     
            GUI::hWnd,                        
            NULL,                        
            GUI::hInst,                       
            NULL);                       

        SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

        std::wstring aaa = L"?";
        if (monitor->supportsBrightness()) {
            aaa = std::to_wstring(monitor->getBrightness()->current);
            aaa.append(L"%");
        }

        hText = CreateWindow(
            L"STATIC",                   
            aaa.c_str(),                           
            WS_CHILD | WS_VISIBLE | SS_LEFT,  
            200, y,                      
            100, 20,                     
            GUI::hWnd,                        
            NULL,                       
            GUI::hInst,                       
            NULL);                       
        SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    }

    height = 25;
    width = 225;
    yPosition++;

    //GROUP BOX
    hText = CreateWindow(L"Button", L"Detected monitors",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        2* margin, margin, width, height * yPosition, GUI::hWnd, (HMENU)0, 0, NULL);
    SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    ///////////////////
    //CONFIG GROUP
    /////////////////
    yPosition = 0;
    xPosition = 245;
    width = 225;
    margin = 5;

    //GROUP BOX
    hText = CreateWindow(L"Button", L"Configuration",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        xPosition, yPosition + margin, width, 140, GUI::hWnd, (HMENU)0, 0, NULL);
    SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    width = 205;
    xPosition += margin;
    yPosition += height - 10;

    HWND hAutostartCheckbox = CreateWindow(
        _T("BUTTON"), _T("Start with Windows"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        xPosition + margin, yPosition + margin, width, height, GUI::hWnd, (HMENU)IDC_AUTOSTART_CHECKBOX, (HINSTANCE)GetWindowLongPtr(GUI::hWnd, GWLP_HINSTANCE), NULL
    );
    SendMessage(hAutostartCheckbox, WM_SETFONT, (WPARAM)this->hNormalFont, 0);
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            TCHAR exePath[MAX_PATH];
            DWORD exePathLen = sizeof(exePath);
            if (RegQueryValueEx(hKey, _T("MyProgram"), NULL, NULL, (LPBYTE)exePath, &exePathLen) == ERROR_SUCCESS) {
                SendMessage(hAutostartCheckbox, BM_SETCHECK, BST_CHECKED, 0);
            }
            RegCloseKey(hKey);
        }
    }

    yPosition += height;

    hText = CreateWindow(
        _T("BUTTON"), _T("Edit config.ini"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD,
        xPosition + margin, yPosition + margin, width-40, height, GUI::hWnd, (HMENU)IDC_EDIT_INI, (HINSTANCE)GetWindowLongPtr(GUI::hWnd, GWLP_HINSTANCE), NULL
    );
    SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    yPosition += height;

    hText = CreateWindow(
        _T("BUTTON"), _T("Restore default config"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD,
        xPosition + margin, yPosition + margin * 2, width-40, height, GUI::hWnd, (HMENU)IDC_RESTORE_DEFAULTS_SETTINGS, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
    );
    SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    yPosition += height;

    hText = CreateWindow(
        _T("BUTTON"), _T("Reload configuration"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD,
        xPosition + margin, yPosition + margin * 3, width-40, height, GUI::hWnd, (HMENU)IDC_RELOAD_CONFIG, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
    );
    SendMessage(hText, WM_SETFONT, (WPARAM)this->hNormalFont, 0);

    HICON hIconExit = LoadIcon(GUI::hInst, MAKEINTRESOURCE(IDI_EXIT));
}

VOID GUI::DestroyTrayMenu()
{
	if (this->hTrayMenu) {
		DestroyMenu(this->hTrayMenu);
        this->hTrayMenu = NULL;
	}
}