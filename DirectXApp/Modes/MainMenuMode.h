#pragma once

#include "Mode.h"

class MainMenuMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	MainMenuMode();

	virtual uint32_t initializeMode() override;
	virtual void uninitializeMode() override;
	
	virtual void handleMenuObjectClick(int i) override;

	//Updating and Advancement Functions
private:
	void initializeMainMenuModeText();

};