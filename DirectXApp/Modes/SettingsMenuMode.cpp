#include "pch.h"
#include "SettingsMenuMode.h"

SettingsMenuMode::SettingsMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Gray;
}

uint32_t SettingsMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Initialize all overlay text
	initializeTextOverlay();

	//See if we're currently connected to a Personal Caddie device or not, transferring to some modes from
	//this menu requires it
	std::pair<BLEState, uint64_t> action = { BLEState::Connected, 0 };
	m_mode_screen_handler(ModeAction::BLEConnection, (void*)&action);
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void SettingsMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
}

void SettingsMenuMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//Like the main menu mode the only thing we can really do here with key presses is 
	//transfer to a different mode. The difference here though, is that some of the modes
	//we can traverse to require an active connection to the Personal Caddie device. 

	ModeType newMode = ModeType::SETTINGS_MENU;
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		newMode = ModeType::MAIN_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	{
		newMode = ModeType::DEVICE_DISCOVERY;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number2:
	{
		if (m_connected) newMode = ModeType::IMU_SETTINGS;
		else createAlert(L"Must be connected to a Personal Caddie to go to the IMU Settings Mode.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number3:
	{
		if (m_connected) newMode = ModeType::CALIBRATION;
		else createAlert(L"Must be connected to a Personal Caddie to go to the Calibration Mode.", UIColor::Red);
		break;
	}
	}

	if (newMode != ModeType::SETTINGS_MENU) m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
}

void SettingsMenuMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Sensor Settings";
	TextOverlay title(m_uiManager.getScreenSize(), { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Sub-Title information
	std::wstring subtitle_message = L"(Press one of the keys listed below to select a mode)";
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Subtitle Text");


	//Body information
	std::wstring body_message_1 = L"1. Connect/Disconnect from Personal Caddie \n";
	std::wstring body_message_2 = L"2. IMU Settings \n";
	std::wstring body_message_3 = L"3. IMU Calibration \n";
	TextOverlay body(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		body_message_1 + body_message_2 + body_message_3, UIConstants::BodyTextPointSize, { UIColor::PaleGray, UIColor::DarkGray, UIColor::White }, { 0, (unsigned int)body_message_1.length(), (unsigned int)body_message_2.length(), (unsigned int)body_message_3.length() }, UITextJustification::UpperLeft);
	m_uiManager.addElement<TextOverlay>(body, L"Body Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void SettingsMenuMode::getBLEConnectionStatus(bool status) { m_connected = status; }

void SettingsMenuMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//In the case that the Personal Caddie becomes asynchronously connected or disconnected
	//while in this mode, this method will get called and updated the m_connected variable.
	//This will in turn allow us, or prevent us, from travelling to certain modes
	m_connected = connectionStatus;
}