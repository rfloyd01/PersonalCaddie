#pragma once

#include "../Devices/PersonalCaddie.h"
#include "../Modes/mode.h"

class Settings : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	Settings(GL& graphics) : Mode(graphics)
	{
		mode_name = "Settings";
		mode_type = ModeType::SETTINGS;

		background_color = { 0.33, 0.33, 0.33 };

		clearAllText();
		clearAllImages();
	};
	//Updating and Advancement Functions
	void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	void processInput();
	void modeStart();
	void modeEnd();

private:
	//PRIVATE FUNCTIONS
	//Setup Functions
	void initializeText();
	int convertCharToInt(char c);
};