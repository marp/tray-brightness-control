// Main.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Main.h"
#include <memory>

#pragma comment(lib,"Shcore.lib")

///////////////////////////
//   GLOBAL VARIABLES   //
/////////////////////////
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // main window
WCHAR szTitle[MAX_LOADSTRING];                  // the title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HFONT hNormalFont;                              // normal font
HWND  hAutostartCheckbox;                       // autostart checkbox

std::wstring configFilePath;

/////////////////////
//   CONSTANTS   ///
///////////////////
const DWORD doubleClickTime = GetDoubleClickTime();
const WCHAR* buildDate = __DATE__ L" ";
const WCHAR* buildArchitecture =
#if defined(_M_X64) || defined(__amd64__)
L"x64";
#elif defined(_M_IX86) || defined(__i386__)
L"x86";
#elif defined(_M_ARM) || defined(__arm__)
L"ARM";
#elif defined(_M_ARM64) || defined(__aarch64__)
L"ARM64";
#endif
const WCHAR* compilerVersion = L"MSVC " _CRT_STRINGIZE(_MSC_VER);


/////////////////////
//    GLOBALS    ///
///////////////////
BOOL g_doubleClickDetected = false;
std::unique_ptr<Configuration> config;
std::unique_ptr<GUI> gui;


// Forward declarations of functions included in this code module:
ATOM                TrayBrightnessControlClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                HandleDoubleClick();

void UpdateTrayIconTooltipAsync(HWND hWnd) {
    INT8 averageBrightness = Monitor::GetCurrentAverageBrightnessOfAll();
    if (averageBrightness < 0) {
        return;
    }

    // Prepare tooltip text  
    std::wstring sTip = L"Average brightness: " + std::to_wstring(averageBrightness) + L"%";

    // Update NOTIFYICONDATA  
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = ID_TRAY1;
    nid.uFlags = NIF_TIP;

    // Use safer string copy function  
    wcsncpy_s(nid.szTip, sTip.c_str(), _TRUNCATE);

    // Update tray icon text  
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

///////////////////
//   WIN MAIN   //
/////////////////
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	//Check for running instance
	HWND hExistingWnd = ::FindWindow(szWindowClass, NULL);
    if (hExistingWnd != NULL)
    {
        ShowWindow(hExistingWnd, SW_NORMAL);
        BringWindowToTop(hExistingWnd);
        SetForegroundWindow(hExistingWnd);
		return(1);
	}

	config = std::make_unique<Configuration>();

	// Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TRAYBRIGHTNESSCONTROL, szWindowClass, MAX_LOADSTRING);
    TrayBrightnessControlClass(hInstance);

    Monitor::detectAllMonitors();

    GUI::hInst = hInstance;
    gui = std::make_unique<GUI>();

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    GUI::hWnd = hWnd;
    gui->configuration = config.get();
    GUI::AdjustWindowSizeForDPI(hWnd);
    gui->CreateMainWindowControls();

    if (!gui->AddTaskBarIcon()) {
        MessageBox(NULL, L"Can't add task bar icon!", L"Error", MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
    }

    if (!gui->CreateTaskBarMenu()) {
        MessageBox(NULL, L"Can't add task bar menu!", L"Error", MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
    }

    // Don't show window if loaded from autostart
    if (!Configuration::IsAutostartEnabled()) {
        ShowWindow(hWnd, nCmdShow);
    }

    MSG msg;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRAYBRIGHTNESSCONTROL));
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Cleanup monitors
    for (Monitor* m : Monitor::monitors) {
        delete m;
    }
    Monitor::monitors.clear();

    return (int) msg.wParam;
}

/////////////////////////
//   WINDOW CLASSES   //
///////////////////////
ATOM TrayBrightnessControlClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYBRIGHTNESSCONTROL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TRAYBRIGHTNESSCONTROL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_TRAYBRIGHTNESSCONTROL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(
       szWindowClass, 
       szTitle, 
       WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
       ((GetSystemMetrics(SM_CXSCREEN) - 185) / 2), ((GetSystemMetrics(SM_CYSCREEN) - 496) / 2),
       496, 
       185, 
       nullptr, 
       nullptr, 
       hInstance, 
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

  ///////////////////
 //   CALLBACKS   //
///////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        {
            if (wParam == SINGLE_CLICK_TIMER_ID)
            {
             
                // If the double-click flag is not set, perform the action for a single click
                if (!g_doubleClickDetected)
                {
                    ShowWindow(hWnd, SW_SHOW);
                    SetForegroundWindow(hWnd);
                }

                // Rest timer
                KillTimer(hWnd, SINGLE_CLICK_TIMER_ID);
            }
        }
        break;
    case CMSG_UPDATE_GUI:
        {
            if (gui) {
                INT8 passedBrightness = static_cast<INT8>(wParam);
                gui->UpdateTaskBarMenu(passedBrightness);
                gui->UpdateTaskBarIcon(passedBrightness);
            }
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:

            //Check if it is a context menu command, the maximum size of the context menu is 999
            if (wmId >= IDM_TRAY_CONTEXTMENU && wmId < IDM_TRAY_CONTEXTMENU + 999) {
                size_t index = wmId - IDM_TRAY_CONTEXTMENU;

                std::vector<MenuValue> menuValues = config->GetContextMenuValues();

                if (index < menuValues.size()) {
                    WORD brightness = menuValues[index].value;
                    OutputDebugStringW(menuValues[index].text.c_str());

                    // The error 'invoke': no matching overloaded function found typically occurs when you are trying to use std::invoke or a callable object incorrectly.
                    // In the provided code, the issue might be related to the lambda function or the way it is being passed to std::thread.
                    // Ensure that the lambda function captures the required variables correctly and matches the expected signature.


                    Monitor::SetBrightnessForAllAsync(hWnd, static_cast<unsigned long>(brightness));
                }
            }

            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDC_AUTOSTART_CHECKBOX:
                config->SetAutostart(SendMessage(hAutostartCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                break;
            case IDC_EDIT_INI:
                Configuration::OpenTextEditor(config->GetConfigFilePath());
                break;
            case IDC_RESTORE_DEFAULTS_SETTINGS:
                if (MessageBox(hWnd, L"Do you want to replace the existing config.ini with the default one?", L"Restore the default configuration", MB_YESNO | MB_ICONWARNING) == 6) {
                    Configuration::CreateConfigFile(config->GetConfigFilePath());
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case CMSG_TRAY1: // Handle tray
    {
        if (lParam == WM_LBUTTONDBLCLK)
        {
            g_doubleClickDetected = TRUE;
            KillTimer(hWnd, SINGLE_CLICK_TIMER_ID);
            HandleDoubleClick();
        }
        else if (lParam == WM_LBUTTONUP)
        {
            if (!g_doubleClickDetected) // If not double clicked
            {
				// Set timer for single click with delay same as double-click time
                SetTimer(hWnd, SINGLE_CLICK_TIMER_ID, doubleClickTime, NULL);
            }
            else
            {
				// Reset flag after double-click
                g_doubleClickDetected = false;
            }
        }
        else if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt); // Retrieve the current cursor position

            SetForegroundWindow(hWnd); // Required for closing menu when clicked outside
            TrackPopupMenu(gui->hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            PostMessage(hWnd, WM_NULL, 0, 0); // This closes the menu when clicked outside

        }
        else if (lParam == WM_MOUSEMOVE) 
        {
            UpdateTrayIconTooltipAsync(hWnd);
        }

    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
             //TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_MEASUREITEM:
    {
        LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
        if (lpMeasureItem->CtlType == ODT_MENU) {
			// Resize the menu item width and height to fit an icon
			lpMeasureItem->itemWidth = 150;  // menu width
            lpMeasureItem->itemHeight = 20;  // height to fit 16x16 icon
        }
        return TRUE;
    }
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE)
        {
			// Instead of minimizing the window, hide it
            ShowWindow(hWnd, SW_HIDE);
			return 0; // Return 0 to skip the default action (minimize)
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_DESTROY:
        gui.reset();
        config.reset();
        {
            NOTIFYICONDATA nid = {};
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = ID_TRAY1;
            Shell_NotifyIcon(NIM_DELETE, &nid);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        {
        std::wstring buildInfo = L"Build date: " + std::wstring(buildDate) + L"\nCompiler: " + std::wstring(compilerVersion) + L"\nArchitecture: " + buildArchitecture;
            SetDlgItemTextW(hDlg, IDC_BUILD_INFO, buildInfo.c_str());
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static Monitor* monitor = nullptr; // Pointer to the Monitor object

    switch (uMsg) {
    case WM_INITDIALOG:
        // Initialize the dialog with the monitor capabilities
        monitor = reinterpret_cast<Monitor*>(lParam); // Get the monitor pointer
        SetDlgItemText(hwndDlg, IDC_NAME, monitor->getNameW());
        SetDlgItemText(hwndDlg, IDC_BRIGHTNESS, monitor->supportsBrightness() ? L"Yes" : L"No");
        SetDlgItemText(hwndDlg, IDC_CONTRAST, monitor->supportsContrast() ? L"Yes" : L"No");
        SetDlgItemText(hwndDlg, IDC_DEGAUSS, monitor->supportsDegauss() ? L"Yes" : L"No");
        //SetDlgItemText(hwndDlg, IDC_DISPLAY_AREA_POSITION, monitor->supportsDisplayAreaPosition() ? L"Yes" : L"No");
        //SetDlgItemText(hwndDlg, IDC_DISPLAY_AREA_SIZE, monitor->supportsDisplayAreaSize() ? L"Yes" : L"No");
        //SetDlgItemText(hwndDlg, IDC_TECHNOLOGY_TYPE, monitor->supportsTechnologyType() ? L"Yes" : L"No");
        return TRUE; // Initialization complete
    case WM_CLOSE:
        EndDialog(hwndDlg, 0); // Close the dialog when the X button is clicked
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

///////////////////
//   FUNCTIONS   //
///////////////////

void HandleDoubleClick() {
    DWORD currentBrightness = (WORD)Monitor::GetCurrentAverageBrightnessOfAll();
	std::vector<WORD> doubleClickValues = config->GetDoubleClickValues();
    if (!doubleClickValues.empty()) {
        int closestIndex = 0;
        int closestDifference = abs(static_cast<int>(currentBrightness) - static_cast<int>(doubleClickValues[0]));

        for (size_t i = 1; i < doubleClickValues.size(); ++i) {
            int difference = abs(static_cast<int>(currentBrightness) - static_cast<int>(doubleClickValues[i]));
            if (difference < closestDifference) {
                closestIndex = static_cast<int>(i);
                closestDifference = difference;
            }
        }

        // Determine the next value in the sequence or loop back to the first value if it's the last one
        int nextIndex = (closestIndex + 1) % doubleClickValues.size();
        std::wstring nextValueText = L"Setting value: " + std::to_wstring(doubleClickValues[nextIndex]) + L"%";

        // Output the next value using OutputDebugString for debugging
        OutputDebugString(nextValueText.c_str());

        Monitor::SetBrightnessForAllAsync(hWnd, doubleClickValues[nextIndex]);
        PostMessageW(hWnd, CMSG_UPDATE_GUI, (WPARAM)doubleClickValues[nextIndex], 0);
    }
    else {
        OutputDebugString(L"DoubleClick values from INI file are empty.");
    }
}
