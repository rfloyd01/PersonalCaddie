#include "pch.h"
#include "DeviceDiscoveryMode.h"
#include "Graphics/Objects/2D/Button.h"
#include "Graphics/Objects/2D/TextBoxes/StaticTextBox.h"

DeviceDiscoveryMode::DeviceDiscoveryMode()
{
	//set a light gray background color for the mode
	m_backgroundColor[0] = 0.75;
	m_backgroundColor[1] = 0.75;
	m_backgroundColor[2] = 0.75;
	m_backgroundColor[3] = 1.0;

	m_state = DeviceDiscoveryState::IDLE;
}

uint32_t DeviceDiscoveryMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeSettingsModeText();

	//Create a button towards the top middle portion of the screen
	DirectX::XMFLOAT2 buttonLocation = { 0.4, 0.25 };
	m_menuObjects.push_back(std::make_shared<Button>(buttonLocation, L"Start Device Watcher"));
	m_menuObjects[0]->changeDimensions({ 1.25, 1.00 }); //Make the device watcher button a little wider than standard
	
	StaticTextBox stb({ 0.5, 0.575 }, { 0.85, 0.5 }, L"Start the device watcher to being enumerating nearby BluetoothLE devices...", windowSize);
	m_uiElements.push_back(std::make_shared<StaticTextBox>(stb));

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::Idle);
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	clearModeText();
	for (int i = 0; i < m_menuObjects.size(); i++) m_menuObjects[i] = nullptr;
	m_menuObjects.clear();
}

void DeviceDiscoveryMode::initializeSettingsModeText()
{
	//Title information
	int index = static_cast<int>(TextType::TITLE);
	std::wstring titleText = L"Device Discovery";
	m_modeText->at(index).message = titleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(titleText.size());

	////Subtitle Information
	//index = static_cast<int>(TextType::SUB_TITLE);
	//std::wstring subtitleText = L"(Select from one of the options below.)";
	//m_modeText->at(index).message = subtitleText;
	//m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	//m_modeText->at(index).locations.push_back(subtitleText.size());

	////Body Information
	//index = static_cast<int>(TextType::BODY);
	//std::wstring bodyText1 = L"1. Connect to a Personal Caddie \n";
	//std::wstring bodyText2 = L"2. Disconnect from Personal Caddie \n";
	//m_modeText->at(index).message = bodyText1 + bodyText2;
	//m_modeText->at(index).colors.push_back({ 0, 0, 0, 1.0 });
	//m_modeText->at(index).colors.push_back({ 0.2, 0.2, 0.2, 1 });
	//m_modeText->at(index).locations.push_back(bodyText1.size());
	//m_modeText->at(index).locations.push_back(bodyText2.size());

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
	}
}

uint32_t DeviceDiscoveryMode::handleMenuObjectClick(int i)
{
	if (i == 0)
	{
		if (m_state == DeviceDiscoveryState::IDLE)
		{
			m_state = DeviceDiscoveryState::DISCOVERY;
			m_menuObjects[0]->updateText(L"Stop Device Watcher");
			((StaticTextBox*)m_uiElements[0].get())->addText(L""); //clear out text in the results box when starting the device watcher
			return ModeState::Active;
		}
		else if (m_state == DeviceDiscoveryState::DISCOVERY)
		{
			m_state = DeviceDiscoveryState::IDLE;
			m_menuObjects[0]->updateText(L"Start Device Watcher");
			return ModeState::Idle;
		}
	}
}