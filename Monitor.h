#pragma once

#include <windows.h>
#include <HighLevelMonitorConfigurationAPI.h>
#include <PhysicalMonitorEnumerationAPI.h>
#include <string>
#include <vector>
#include <future>

#include "IOCTL.h"

struct ContinuousSetting
{
	unsigned long min;
	unsigned long max;
	unsigned long current;
};

class Monitor {

public:

	Monitor(HMONITOR nonPhysicalMonitor);

	~Monitor();

	bool isValid();

	std::string getName();
	LPCWSTR getNameW();

	void printCapabilities();

	//Single Actions
	bool degaussMonitor();

	bool restoreColourDefaults();

	bool restoreDefaults();

	bool saveSettings();

	//Get Setting Values (I won't cache these as they can change via external influences (Monitor controls))
	ContinuousSetting* getBrightness();


	//Set Settings

	/**
	 @fn	bool Monitor::setBrightness(unsigned long newValue);

	 @brief	Sets the brightness.

	 @author	Richard
	 @date	07/09/2016

	 @param	newValue	The new value.

	 @return	true if it succeeds, false if it fails.
	 */

	bool setBrightness(unsigned long newValue);


	bool supportsBrightness();

	bool supportsColourTemperature();

	bool supportsContrast();

	//CRT Only, just keeping for completeness
	bool supportsDegauss();

	bool supportsDisplayAreaPosition();

	bool supportsDisplayAreaSize();

	bool supportsTechnologyType();

	bool supportsRGBDrive();

	bool supportsRGBGain();

	bool supportsRestoreColourDefaults();

	bool supportsRestoreDefaults();

	bool supportsRestoreDefaultEX();

	static INT8 GetCurrentAverageBrightnessOfAll();

	static BOOL SetBrightnessForAll(unsigned long newValue);

	static VOID SetBrightnessForAllAsync(HWND, unsigned long);

	static std::vector<Monitor*> monitors;

	static void detectAllMonitors();

protected:

	PHYSICAL_MONITOR monitorPointer;

	std::string name;
	LPCWSTR nameW;

	std::string systemName;

	IOCTL* ioctl;


	bool valid;

	bool primaryMonitor;

	bool initialiseMonitor(HMONITOR nonPhysicalMonitor);


	//Capabilities
	bool getCapabilities();

	unsigned long capabilities;

	unsigned long colourCapabilities;


	//Misc
	bool settingChangeCheck(ContinuousSetting setting, unsigned long newValue);
};