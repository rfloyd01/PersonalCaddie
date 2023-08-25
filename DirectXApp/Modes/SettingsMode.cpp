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

}