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
	//virtual void processInput(InputState* inputState) override;

	//Updating and Advancement Functions
private:
	void initializeMainMenuModeText();

};