#include "pch.h"
#include "SettingsMode.h"

SettingsMode::SettingsMode()
{
	//set a gray background color for the mode
	m_backgroundColor[0] = 0.5;
	m_backgroundColor[1] = 0.5;
	m_backgroundColor[2] = 0.5;
	m_backgroundColor[3] = 1.0;
}

uint32_t SettingsMode::initializeMode()
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeSettingsModeText();
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void SettingsMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	clearModeText();
}

void SettingsMode::initializeSettingsModeText()
{
	//Title information
	std::wstring titleText = L"Sensor Settings";
	m_modeText->at(TextType::TITLE) = titleText;
	m_modeTextColors->at(TextType::TITLE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::TITLE).locations.push_back(titleText.size());

	//Subtitle Information
	std::wstring subtitleText = L"(Press one of the keys listed below to select a mode)";
	m_modeText->at(TextType::SUB_TITLE) = subtitleText;
	m_modeTextColors->at(TextType::SUB_TITLE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::SUB_TITLE).locations.push_back(subtitleText.size());

	//Body Information
	std::wstring bodyText1 = L"1. Connect/Disconnect from Personal Caddie \n";
	std::wstring bodyText2 = L"2. IMU Settings \n";
	m_modeText->at(TextType::BODY) = bodyText1 + bodyText2;
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0, 0, 0, 1.0 });
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.2, 0.2, 0.2, 1 });
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText1.size());
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText2.size());

	//Footnote information
	std::wstring footnoteText = L"Press Esc. to return to main menu.";
	m_modeText->at(TextType::FOOT_NOTE) = footnoteText;
	m_modeTextColors->at(TextType::FOOT_NOTE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::FOOT_NOTE).locations.push_back(footnoteText.size());

}