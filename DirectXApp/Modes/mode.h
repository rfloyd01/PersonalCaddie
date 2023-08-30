#pragma once

#include "Graphics/Objects/2D/UIElements.h"
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
	UI_TEST_MODE = 6,
	END = 7 //allows for looping through of ModeType enum class. This number should be one more than previous value
};

//The ModeState enum keeps track of what the current state of the app is. Each of these
//enums act as binary flags that go into a larger number
enum ModeState
{
	Idle = 1,
	Active = 2,
	Recording = 4,
	CanTransfer = 8
};

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize) = 0;
	virtual void uninitializeMode() = 0;

	virtual void update() {}; //not a pure virtual method as not all modes require this method

	const float* getBackgroundColor();

	std::vector<std::shared_ptr<UIElement> > const& getUIElements() { return m_uiElements; }

	template <typename T>
	void addUIElement(T const& element) { m_uiElements.push_back(std::make_shared<T>(element)); }

	//Alert Methods
	void createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize);
	TextOverlay removeAlerts();

	virtual uint32_t handleUIElementStateChange(int i) = 0;

protected:
	float m_backgroundColor[4]; //represents the background color when this mode is being rendered

	std::vector<std::shared_ptr<UIElement> >    m_uiElements; //2d objects like Drop downs, combo boxes, buttons and text
};