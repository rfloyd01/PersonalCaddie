#pragma once

#include <Modes/mode.h>

//Classes, structs and enums that are defined in other headers
class GL;

class MainMenu : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	MainMenu(GL* graphics);

	//Updating and Advancement Functions
	void update();
	void processInput();
	void modeStart();
	void modeEnd();
};