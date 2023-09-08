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
	DEVELOPER_TOOLS = 6,
	UI_TEST_MODE = 7,
	GRAPH_MODE = 8,
	END = 9 //allows for looping through of ModeType enum class. This number should be one more than previous value
};

//The ModeState enum keeps track of what the current state of the app is. Each of these
//enums act as binary flags that go into a larger number
enum ModeState
{
	Active = 1,
	Recording = 2,
	CanTransfer = 4,
	NeedTextUpdate = 8,
	PersonalCaddieSensorIdleMode = 16,
	PersonalCaddieSensorActiveMode = 32
};

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) = 0;
	virtual void uninitializeMode() = 0;

	virtual void update() {}; //not a pure virtual method as not all modes require this method

	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) {}; //Some modes need the ability to enable or disable features if the Personal Caddie gets disconnected

	const UIColor getBackgroundColor();

	std::vector<std::shared_ptr<UIElement> > const& getUIElements() { return m_uiElements; }

	uint32_t getModeState() { return m_state; }

	template <typename T>
	void addUIElement(T const& element) { m_uiElements.push_back(std::make_shared<T>(element)); }

	//Alert Methods
	void createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize);
	void createAlert(TextOverlay& alert);
	TextOverlay removeAlerts();

	virtual uint32_t handleUIElementStateChange(int i) = 0;

protected:
	UIColor m_backgroundColor; //represents the background color when this mode is being rendered

	std::vector<std::shared_ptr<UIElement> >  m_uiElements; //2d objects like Drop downs, combo boxes, buttons and text

	uint32_t m_state; //the state of the current mode
};