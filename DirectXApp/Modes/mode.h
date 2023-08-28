#pragma once

#include "Graphics/Utilities/Text.h"
#include "Graphics/Objects/2D/MenuObject.h"
#include "Input/InputProcessor.h"
#include <string>

//Classes, structs and enums that are helpful for this class
enum class ModeType
{
	//This enum represents all of the different types of modes there are for the program
	//when adding a new mode it should also be added to this list

	MAIN_MENU = 0,
	FREE = 1,
	CALIBRATION = 2,
	TRAINING = 3,
	SETTINGS_MENU = 4,
	DEVICE_DISCOVERY = 5,
	END = 6 //allows for looping through of ModeType enum class. This number should be one more than previous value
};

//The ModeState enum keeps track of what the current state of the app is. Each of these
//enums act as binary flags that go into a larger number
enum ModeState
{
	Idle = 0,
	Active = 1,
	Recording = 2,
	CanTransfer = 4
};

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	virtual uint32_t initializeMode() = 0;
	virtual void uninitializeMode() = 0;

	virtual void enterActiveState(int state) {}; //not a pure virtual method as not all modes require this method
	virtual void update() {}; //not a pure virtual method as not all modes require this method

	const float* getBackgroundColor();

	std::vector<std::shared_ptr<MenuObject> > const& getMenuObjects() { return m_menuObjects; }
	virtual void handleMenuObjectClick(int i) = 0;

	std::shared_ptr<std::vector<Text>> getModeText() { return m_modeText; }
	void setModeText(Text const& text);

protected:
	//PROTECTED FUNCTIONS
	void initializeModeText();
	void clearModeText();

	float m_backgroundColor[4]; //represents the background color when this mode is being rendered

	//after processing data and input, this get's updated to let the main program known if anything needs
	//to change (swapping to a new mode for example)
	//std::pair<ModeState, uint32_t> externalAction;

	//each mode owns all objects to be rendered on screen
	std::shared_ptr<std::vector<Text>>          m_modeText;

	std::vector<std::shared_ptr<MenuObject> >   m_menuObjects; //2d objects like Drop downs, combo boxes, buttons, etc
};