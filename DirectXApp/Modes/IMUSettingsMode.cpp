#include "pch.h"
#include "IMUSettingsMode.h"

IMUSettingsMode::IMUSettingsMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::LightBlue;
}

uint32_t IMUSettingsMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	std::wstring buttonText = L"Get Current Settings";
	TextButton updateButton(windowSize, { 0.5, 0.25 }, { 0.12, 0.1 }, buttonText);

	m_uiElements.push_back(std::make_shared<TextButton>(updateButton));

	initializeTextOverlay(windowSize);

	m_state = initialState;

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer);
}

void IMUSettingsMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void IMUSettingsMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"IMU Settings";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

void IMUSettingsMode::getCurrentSettings(std::vector<uint8_t*> settings)
{
	 //The vector passed into this method holds three pointers. One each that
	//points to the settings in the accelerometer, gyroscope and magnetometer
	//classes. We copy these into the m_currentSettings array
	for (int i = 0; i < 3; i++)
	{
		//TODO: shouldn't be using the number 10 here, should be getting this
		//value from the sensor_settings.h file somewhere to allow future updates
		for (int j = 0; j < 10; j++)
		{
			//the additional 1 is because the very first setting in the array
			//holds the current Personal Caddie power mode and nothing about
			//the sensors themselves.
			m_currentSettings[10 * i + j + 1] = *(settings[i] + j);
		}
	}
}

uint32_t IMUSettingsMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		//This is the get/set settings button
		if (!(m_state & IMUSettingsState::DISPLAY_SETTINGS))
		{
			//this will put us into the active state, causing the
			//mode screen class to give the current sensor settings.
			m_state |= IMUSettingsState::DISPLAY_SETTINGS;

			//Change the text of button and disable it. The button
			//only becomes enabled if any settings have actually changed.
			m_uiElements[0]->getText()->message = L"Update Settings";
			m_uiElements[0]->setState(m_uiElements[0]->getState() | UIElementState::Disabled);
		}
	}
	return m_state;
}

void IMUSettingsMode::update()
{
	//when in active mode it means that the device watcher is running
}

void IMUSettingsMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//TODO::
	//If the connection is lost while on this page then disable all buttons and 
	//drop down menus, prompting the user to attempt to reconnect.
}