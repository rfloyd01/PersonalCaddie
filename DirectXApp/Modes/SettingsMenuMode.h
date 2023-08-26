#pragma once

#include "Mode.h"

class SettingsMenuMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	SettingsMenuMode();

	virtual uint32_t initializeMode() override;
	virtual void uninitializeMode() override;

	//Updating and Advancement Functions
private:
	void initializeSettingsModeText();

};