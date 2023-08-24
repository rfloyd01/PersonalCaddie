#pragma once

#include "Graphics/Utilities/Text.h"

//Classes, structs and enums that are helpful for this class
enum class ModeType
{
	//This enum represents all of the different types of modes there are for the program
	//when adding a new mode it should also be added to this list

	MAIN_MENU = 0,
	FREE = 1,
	CALIBRATION = 2,
	TRAINING = 3,
	SETTINGS = 4
};

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	virtual void Initialize() = 0;

	std::shared_ptr<std::map<TextType, Text> > getModeText() { return m_modeText; }

protected:
	//PROTECTED FUNCTIONS
	void initializeModeText();
	void clearModeText();

	//a map used to store all words to be rendered on screen, a map is used to make it easier when adding and deleting messages
	std::shared_ptr<std::map<TextType, Text > > m_modeText; //TODO: I can probably make this a unique pointer 
};