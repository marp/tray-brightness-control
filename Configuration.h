#pragma once
#include <windows.h>
#include <vector>
#include <tchar.h>
#include <string>

struct MenuValue {
	std::wstring text;
	WORD value;
};

class Configuration
{
private:
	LPWSTR configFilePath;
	std::vector<MenuValue> contextMenuValues;
	std::vector<WORD> doubleClickValues;

public:
	Configuration();
	Configuration(LPWSTR);

	BOOL LoadConfiguration();

	LPWSTR GetConfigFilePath();
	VOID SetConfigFilePath(LPWSTR);

	std::vector<MenuValue> GetContextMenuValues();
	VOID SetContextMenuValues(std::vector<MenuValue>);
	std::vector<WORD> GetDoubleClickValues();
	VOID SetDoubleClickValues(std::vector<WORD>);

//STATIC
	static BOOL ConfigFileExists(LPCWSTR);
	static BOOL CreateConfigFile(LPWSTR);
	static LPCWSTR GetDefaultConfigFilePath();
	static std::vector<MenuValue> GetContextMenuValues(LPWSTR);
	static std::vector<WORD> GetDoubleClickValues(LPWSTR);
	static VOID OpenTextEditor(LPWSTR);
	static BOOL IsAutostartEnabled();
	static BOOL SetAutostart(BOOL);
};

