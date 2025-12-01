# Tray Brightness Control

Tray Brightness Control is a lightweight utility that lets you adjust your screen brightness directly from the system tray or via keyboard shortcuts.
It is highly customizable, unobtrusive, and designed for seamless everyday use.

The application is written in C++ and leverages the **DXVA2 API** to control display brightness, which requires Windows Vista or newer.

## Features
- Adjust screen brightness directly from the system tray
- Control brightness using customizable keyboard shortcuts
- Flexible per-monitor control: configure brightness for individual displays or synchronize changes across all monitors
- Live monitor overview: easily view all connected displays
- Optional autostart with Windows — the application is portable and requires no installation
- Lightweight and efficient: minimal memory footprint and disk usage

## Configuration
The configuration file is located in the same directory as the executable and is named `config.ini`.

## Flowchart
1. Initialization

2. Check for existing instance
```
 HWND hWndExisting = ::FindWindow(NazwaKlasy, NULL);
 if (hWndExisting != NULL)
 {
 	ShowWindow(hWndExisting, SW_NORMAL);
        BringWindowToTop(hWndExisting);
        SetForegroundWindow(hWndExisting);
        return(1);
 }
```

3. Check if Vista or newer
4. Check if INI file exists, if it does, load settings from it otherwise create it
5. Look for monitors, show errors if not found any.
6. Generate tray menu
7. Handle actions:
    * Double left-click on the tray icon to restore the previous brightness setting
    * Single left-click to open the settings menu
    * Single right-click to show the tray menu
        * Exit the application if selected from the menu
        * Change brightness to the value selected from the menu

## To-Do
- Add keyboard support.
- Include more tray menu icons.
- Improve high DPI support (currently, the program appears the same size across all DPIs).

## Compilation
Compile with Visual Studio 2022. It doesn't require any additional instructions.

## License
This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contributing
Feel free to contribute to this project by forking it and submitting a pull request.