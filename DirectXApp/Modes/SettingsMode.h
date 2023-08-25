#pragma once

#include "Mode.h"

class SettingsMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	SettingsMode();

	virtual uint32_t initializeMode() override;
	virtual void uninitializeMode() override;

	//Updating and Advancement Functions
private:
	void initializeSettingsModeText();

};