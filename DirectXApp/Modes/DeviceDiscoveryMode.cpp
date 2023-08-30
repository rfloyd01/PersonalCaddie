#include "pch.h"
#include "DeviceDiscoveryMode.h"
#include "Graphics/Objects/2D/UIButton.h"


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
	//Create UI Elements on the page
	StaticTextBox stb({ 0.5, 0.575 }, { 0.85, 0.5 }, L"Start the device watcher to being enumerating nearby BluetoothLE devices...", windowSize);
	UIButton butt({ 0.4, 0.25 }, { 0.12, 0.1 }, windowSize, L"Start Device Watcher");
	m_uiElements.push_back(std::make_shared<StaticTextBox>(stb));
	m_uiElements.push_back(std::make_shared<UIButton>(butt));

	initializeTextOverlay(windowSize);

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::Idle);
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	//TODO: need to uninitialize UI Elements
}

void DeviceDiscoveryMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring message = L"Device Discovery";
	TextOverlay title(message, { UIColor::White }, { 0,  (unsigned int)message.length() }, UITextType::TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	message = L"Press Esc. to return to settings menu.";
	TextOverlay footNote(message, { UIColor::White }, { 0,  (unsigned int)message.length() }, UITextType::FOOT_NOTE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footNote));
}

uint32_t DeviceDiscoveryMode::handleUIElementStateChange(int i)
{
	if (i == 1)
	{
		//UI Element 1 is the device watcher button
		if (m_state == DeviceDiscoveryState::IDLE)
		{
			m_state = DeviceDiscoveryState::DISCOVERY;
			((UIButton*)m_uiElements[i].get())->updateButtonText(L"Stop Device Watcher");
			((StaticTextBox*)m_uiElements[0].get())->addText(L""); //clear out text in the results box when starting the device watcher
			return ModeState::Active;
		}
		else if (m_state == DeviceDiscoveryState::DISCOVERY)
		{
			m_state = DeviceDiscoveryState::IDLE;
			((UIButton*)m_uiElements[i].get())->updateButtonText(L"Start Device Watcher");
			return ModeState::Idle;
		}
	}
}