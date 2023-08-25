#pragma once

#include "Mode.h"

class MainMenuMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	MainMenuMode();

	virtual void Initialize() override;
	virtual void processInput(InputState* inputState) override;

	//Updating and Advancement Functions
private:
	void initializeMainMenuModeText();

};