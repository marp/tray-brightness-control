#pragma once
#include <windows.h>
#include <vector>
#include <tchar.h>
#include <string>
class Configuration
{
private:
	LPWSTR configFilePath;
	std::vector<WORD> contextMenuValues;
	std::vector<WORD> doubleClickValues;

public:
	Configuration();
	Configuration(LPWSTR);

	BOOL LoadConfiguration();

	LPWSTR GetConfigFilePath();
	VOID SetConfigFilePath(LPWSTR);

	std::vector<WORD> GetContextMenuValues();
	VOID SetContextMenuValues(std::vector<WORD>);
	std::vector<WORD> GetDoubleClickValues();
	VOID SetDoubleClickValues(std::vector<WORD>);

//STATIC
	static BOOL ConfigFileExists(LPCWSTR);
	static BOOL CreateConfigFile(LPWSTR);
	static LPCWSTR GetDefaultConfigFilePath();
	static std::vector<WORD> GetContextMenuValues(LPWSTR);
	static std::vector<WORD> GetDoubleClickValues(LPWSTR);
	static VOID OpenTextEditor(LPWSTR);
	static BOOL IsAutostartEnabled();
	static BOOL SetAutostart(BOOL);
};

