#include "pch.h"
#include "DeviceDiscoveryMode.h"

DeviceDiscoveryMode::DeviceDiscoveryMode()
{
	//set a light gray background color for the mode
	m_backgroundColor[0] = 0.75;
	m_backgroundColor[1] = 0.75;
	m_backgroundColor[2] = 0.75;
	m_backgroundColor[3] = 1.0;

	m_state = DeviceDiscoveryState::IDLE;
}

uint32_t DeviceDiscoveryMode::initializeMode()
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeSettingsModeText();
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	clearModeText();
}

void DeviceDiscoveryMode::initializeSettingsModeText()
{
	//Title information
	int index = static_cast<int>(TextType::TITLE);
	std::wstring titleText = L"Device Discovery";
	m_modeText->at(index).message = titleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(titleText.size());

	//Subtitle Information
	index = static_cast<int>(TextType::SUB_TITLE);
	std::wstring subtitleText = L"(Select from one of the options below.)";
	m_modeText->at(index).message = subtitleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(subtitleText.size());

	//Body Information
	index = static_cast<int>(TextType::BODY);
	std::wstring bodyText1 = L"1. Connect to a Personal Caddie \n";
	std::wstring bodyText2 = L"2. Disconnect from Personal Caddie \n";
	m_modeText->at(index).message = bodyText1 + bodyText2;
	m_modeText->at(index).colors.push_back({ 0, 0, 0, 1.0 });
	m_modeText->at(index).colors.push_back({ 0.2, 0.2, 0.2, 1 });
	m_modeText->at(index).locations.push_back(bodyText1.size());
	m_modeText->at(index).locations.push_back(bodyText2.size());

	//Footnote information
	index = static_cast<int>(TextType::FOOT_NOTE);
	std::wstring footnoteText = L"Press Esc. to return to settings menu.";
	m_modeText->at(index).message = footnoteText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(footnoteText.size());

}

void DeviceDiscoveryMode::enterActiveState(int state)
{
	if (state == 1)
	{
		//We're entering the device discovery state
		m_state = DeviceDiscoveryState::DISCOVERY;

		//Update Title text
		int index = static_cast<int>(TextType::TITLE);
		std::wstring titleText = L"Connect to a Device";
		m_modeText->at(index).message = titleText;
		m_modeText->at(index).locations.back() = titleText.size();

		////Update the sub-title text
		//std::wstring subtitleText = L"(Select from one of the options below.)";
		//m_modeText->at(TextType::SUB_TITLE) = subtitleText;
		//m_modeTextColors->at(TextType::SUB_TITLE).colors.push_back({ 1, 1, 1, 1 });
		//m_modeTextColors->at(TextType::SUB_TITLE).locations.push_back(subtitleText.size());

		////Update the body text
		//std::wstring bodyText1 = L"1. Connect to a Personal Caddie \n";
		//std::wstring bodyText2 = L"2. Disconnect from Personal Caddie \n";
		//m_modeText->at(TextType::BODY) = bodyText1 + bodyText2;
		//m_modeTextColors->at(TextType::BODY).colors.push_back({ 0, 0, 0, 1.0 });
		//m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.2, 0.2, 0.2, 1 });
		//m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText1.size());
		//m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText2.size());
	}
}