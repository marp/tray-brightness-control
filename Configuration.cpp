#include "Configuration.h"
#include <tchar.h>

#define REGISTRY_KEY_NAME _T("TrayBrightnessControl")

Configuration::Configuration() {
    std::wstring tempStr(Configuration::GetDefaultConfigFilePath());
    if (!Configuration::ConfigFileExists(&tempStr[0])) {  
       Configuration::CreateConfigFile(&tempStr[0]);  
    }
    this->configFilePath = &tempStr[0];
    LoadConfiguration();
}

Configuration::Configuration(LPWSTR wstrFilePath) {
    if (!Configuration::ConfigFileExists(wstrFilePath)) {
		Configuration::CreateConfigFile(wstrFilePath);
    }
    this->configFilePath = wstrFilePath;
    LoadConfiguration();
}

BOOL Configuration::LoadConfiguration() {
	this->SetContextMenuValues(Configuration::GetContextMenuValues(const_cast<LPWSTR>(this->configFilePath.c_str())));
	this->SetDoubleClickValues(Configuration::GetDoubleClickValues(const_cast<LPWSTR>(this->configFilePath.c_str())));
	return true;
}

LPWSTR Configuration::GetConfigFilePath() {
    return const_cast<LPWSTR>(this->configFilePath.c_str());
}

VOID Configuration::SetConfigFilePath(LPWSTR wstrFilePath) {
    this->configFilePath = wstrFilePath;
}

BOOL Configuration::ConfigFileExists(LPCWSTR pcwstrFilePath) {

    DWORD dwAttrib = GetFileAttributesW(pcwstrFilePath);

    // return TRUE if file exists, FALSE if not exists or it's a directory
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL Configuration::CreateConfigFile(LPWSTR wstrFilePath) {
    HMODULE hModule = GetModuleHandle(NULL);

    //find the config.ini file in resources
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(101), RT_RCDATA);
    if (!hRes) {
        MessageBox(0, _T("Not found config.ini in program EXE!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    //size of config.ini file
    DWORD resSize = SizeofResource(hModule, hRes);
    if (resSize == 0) {
        MessageBox(0, _T("Size of config.ini is 0!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    // load a resource into memory
    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) {
        MessageBox(0, _T("Failed to load the config.ini resource!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    //get a pointer to a resource
    LPVOID pData = LockResource(hData);
    if (!pData) {
        MessageBox(0, _T("Failed to lock the config.ini resource!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    // create a file on disk
    HANDLE hFile = CreateFile(wstrFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(0, _T("Failed to create a configuration file!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    // save the data to a file
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hFile, pData, resSize, &bytesWritten, NULL);
    if (!result || bytesWritten != resSize) {
        MessageBox(0, _T("An error occurred while saving data to a file!"), _T("Error"), MB_ICONERROR);
        return FALSE;
    }

    return CloseHandle(hFile);
}

LPCWSTR Configuration::GetDefaultConfigFilePath() {  
   static std::wstring wstrFilePath;  
   wchar_t szFilePath[MAX_PATH];  
   if (GetModuleFileName(NULL, szFilePath, MAX_PATH) == 0) {  
       MessageBox(NULL, L"Failed to get the executable path.", L"Error", MB_ICONERROR);  
       return NULL;  
   }  

   wstrFilePath.assign(szFilePath);  

   size_t pos = wstrFilePath.find_last_of(L"\\/");  
   if (pos != std::wstring::npos) {  
       wstrFilePath.erase(pos + 1);  
   }  

   wstrFilePath.append(L"config.ini");  
   return wstrFilePath.c_str();  
}

std::vector<MenuValue> Configuration::GetContextMenuValues() {
	return this->contextMenuValues;
}

std::vector<MenuValue> Configuration::GetContextMenuValues(LPWSTR configFilePath) {
	wchar_t buffer[1024] = { 0 };

	// Reading the 'Values' key from the [MenuValues] section in the INI file
	GetPrivateProfileString(L"BrightnessValues", L"MenuValues", L"", buffer, 1024, configFilePath);

	// Split the comma-separated values into a vector of strings
	std::vector<MenuValue> values;
	wchar_t* context = nullptr; // Context variable for wcstok_s
	wchar_t* token = wcstok_s(buffer, L",", &context); // Use wcstok_s for secure tokenization
	while (token != nullptr) {
		MenuValue mv;
		std::wstring textStr = token;

		bool isNumeric = true;
		for (wchar_t c : textStr) {
			if (!iswdigit(c) && c != L' ') {
				isNumeric = false;
				break;
			}
		}

		if (isNumeric && !textStr.empty()) {
			mv.text = textStr + L"%";
		}
		else {
			mv.text = textStr;
		}
		mv.value = static_cast<WORD>(_wtoi(textStr.c_str()));

		values.push_back(mv);
		token = wcstok_s(nullptr, L",", &context);
	}
	return values;
}

VOID Configuration::SetContextMenuValues(std::vector<MenuValue> values) {
	// Assuming you want to set the context menu values from the INI file
	this->contextMenuValues = values;
}

std::vector<WORD> Configuration::GetDoubleClickValues() {
    return this->doubleClickValues;
}

std::vector<WORD> Configuration::GetDoubleClickValues(LPWSTR configFilePath) {
    wchar_t buffer[1024] = { 0 };

    // Reading the 'Values' key from the [MenuValues] section in the INI file
    GetPrivateProfileString(L"BrightnessValues", L"DoubleClickValues", L"", buffer, 1024, configFilePath);

    // Split the comma-separated values into a vector of integers
    std::vector<WORD> values;
    wchar_t* context = nullptr; // Context variable for wcstok_s
    wchar_t* token = wcstok_s(buffer, L",", &context); // Use wcstok_s for secure tokenization
    while (token != nullptr) {
        values.push_back(_wtoi(token)); // Convert each token to an integer
        token = wcstok_s(nullptr, L",", &context);
    }
    return values;
}

VOID Configuration::SetDoubleClickValues(std::vector<WORD> values) {
    // Assuming you want to set the context menu values from the INI file
    this->doubleClickValues = values;
}

VOID Configuration::OpenTextEditor(LPWSTR szConfigFilePath) {
   if (!Configuration::ConfigFileExists(szConfigFilePath)) {  
       MessageBox(NULL,  
           L"The configuration file (config.ini) could not be found. A new file will be created.",  
           L"Configuration File Missing",  
           MB_ICONEXCLAMATION);  

       // Allocate a writable buffer and copy the LPCWSTR value to it  
       //LPCWSTR defaultPath = Configuration::GetDefaultConfigFilePath();  
       //size_t len = wcslen(defaultPath) + 1;  
       //szConfigFilePath = new wchar_t[len];  
       //wcscpy_s(szConfigFilePath, len, defaultPath);  
   }  
   ShellExecute(NULL, _T("open"), _T("notepad.exe"), (LPWSTR)szConfigFilePath, NULL, SW_SHOW);

   // Clean up dynamically allocated memory  
   delete[] szConfigFilePath;  
}

BOOL Configuration::IsAutostartEnabled() {
    HKEY hKey;
    LPCTSTR subKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

    // Otwórz klucz rejestru
    if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        MessageBox(NULL, _T("Nie udało się otworzyć klucza rejestru!"), _T("Error"), MB_ICONERROR);
        return false; // Klucz nie został otwarty
    }

    // Sprawdź, czy wpis istnieje
    TCHAR exePath[MAX_PATH];
    DWORD exePathSize = sizeof(exePath);
    if (RegQueryValueEx(hKey, REGISTRY_KEY_NAME, NULL, NULL, (LPBYTE)exePath, &exePathSize) == ERROR_SUCCESS) {
        // Wpis istnieje
        RegCloseKey(hKey);
        return true; // Program jest w autostarcie
    }

    // Wpis nie istnieje
    RegCloseKey(hKey);
    return false; // Program nie jest w autostarcie
}

BOOL Configuration::SetAutostart(BOOL enabled) {
    HKEY hKey;
    LPCTSTR subKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

    // Open registry key
    if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        MessageBox(NULL, _T("Failed to open registry key"), _T("Error"), MB_ICONERROR);
        return false;
    }

    if (enabled) {
		// Get current executable path
        TCHAR exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);

        // Add registry key
        if (RegSetValueEx(hKey, REGISTRY_KEY_NAME, 0, REG_SZ, (const BYTE*)exePath, (_tcslen(exePath) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
            MessageBox(NULL, _T("Failed to add to autostart!"), _T("Error"), MB_ICONERROR);
            return false;
        }
    }
    else {
		// Remove registry key
        if (RegDeleteValue(hKey, REGISTRY_KEY_NAME) != ERROR_SUCCESS) {
            MessageBox(NULL, _T("Failed to remove from autostart!"), _T("Error"), MB_ICONERROR);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}
