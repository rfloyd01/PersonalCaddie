#pragma once

#include "Mode.h"

class SettingsMenuMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	SettingsMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	//Updating and Advancement Functions
private:
	/*void initializeSettingsModeText();*/
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
};