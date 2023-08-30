#include "pch.h"
#include "SettingsMenuMode.h"

SettingsMenuMode::SettingsMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor[0] = 0.5;
	m_backgroundColor[1] = 0.5;
	m_backgroundColor[2] = 0.5;
	m_backgroundColor[3] = 1.0;
}

uint32_t SettingsMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeSettingsModeText();
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void SettingsMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	clearModeText();
}

void SettingsMenuMode::initializeSettingsModeText()
{
	//Title information
	int index = static_cast<int>(TextType::TITLE);
	std::wstring titleText = L"Sensor Settings";
	m_modeText->at(index).message = titleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(titleText.size());

	//Subtitle Information
	index = static_cast<int>(TextType::SUB_TITLE);
	std::wstring subtitleText = L"(Press one of the keys listed below to select a mode)";
	m_modeText->at(index).message = subtitleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(subtitleText.size());

	//Body Information
	index = static_cast<int>(TextType::BODY);
	std::wstring bodyText1 = L"1. Connect/Disconnect from Personal Caddie \n";
	std::wstring bodyText2 = L"2. IMU Settings \n";
	m_modeText->at(index).message = bodyText1 + bodyText2;
	m_modeText->at(index).colors.push_back({ 0, 0, 0, 1.0 });
	m_modeText->at(index).colors.push_back({ 0.2, 0.2, 0.2, 1 });
	m_modeText->at(index).locations.push_back(bodyText1.size());
	m_modeText->at(index).locations.push_back(bodyText2.size());

	//Footnote information
	index = static_cast<int>(TextType::FOOT_NOTE);
	std::wstring footnoteText = L"Press Esc. to return to main menu.";
	m_modeText->at(index).message = footnoteText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(footnoteText.size());

}

uint32_t SettingsMenuMode::handleUIElementStateChange(int i)
{
	return ModeState::Idle;
}