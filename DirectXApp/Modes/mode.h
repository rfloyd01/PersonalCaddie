#pragma once

#include "Graphics/Utilities/Text.h"
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
	SETTINGS = 4,
	END = 5 //allows for looping through of ModeType enum class
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
	//virtual void processInput(InputState* inputState) = 0;

	const float* getBackgroundColor();

	//Alert messages can carry over between different modes so they
	//have special functions for handling them
	std::pair<std::wstring, TextTypeColorSplit> getCurrentAlerts();
	void setCurrentAlerts(std::pair<std::wstring, TextTypeColorSplit> alert);
	void removeCurrentAlerts();

	std::shared_ptr<std::map<TextType, std::wstring> > getModeText() { return m_modeText; }
	std::shared_ptr<std::map<TextType, TextTypeColorSplit> > getModeTextColors() { return m_modeTextColors; }

protected:
	//PROTECTED FUNCTIONS
	void initializeModeText();
	void clearModeText();

	float m_backgroundColor[4]; //represents the background color when this mode is being rendered

	//after processing data and input, this get's updated to let the main program known if anything needs
	//to change (swapping to a new mode for example)
	//std::pair<ModeState, uint32_t> externalAction;

	//a map used to store all words to be rendered on screen and their colors,
	//a map is used to make it easier when adding and deleting messages
	std::shared_ptr<std::map<TextType, std::wstring > > m_modeText;
	std::shared_ptr<std::map<TextType, TextTypeColorSplit > > m_modeTextColors;
};