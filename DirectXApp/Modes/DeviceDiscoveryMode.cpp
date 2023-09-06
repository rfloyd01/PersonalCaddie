#include "pch.h"
#include "DeviceDiscoveryMode.h"
#include "Graphics/Objects/2D/UIButton.h"


DeviceDiscoveryMode::DeviceDiscoveryMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::PaleGray;

	m_state = DeviceDiscoveryState::IDLE;
}

uint32_t DeviceDiscoveryMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Create UI Elements on the page
	FullScrollingTextBox deviceWatcherResults(windowSize, { 0.5, 0.575 }, { 0.85, 0.35 }, L"Start the device watcher to being enumerating nearby BluetoothLE devices...", 0.05f, false, false);
	TextButton deviceWatcherButton(windowSize, { 0.4, 0.25 }, { 0.12, 0.1 }, L"Start Device Watcher");
	TextButton connectButton(windowSize, { 0.6, 0.25 }, { 0.12, 0.1 }, L"Connect to Device");

	connectButton.setState(UIElementStateBasic::Disabled); //The button is disabled until an actual device is selected

	m_uiElementsBasic.push_back(std::make_shared<FullScrollingTextBox>(deviceWatcherResults));
	m_uiElementsBasic.push_back(std::make_shared<TextButton>(deviceWatcherButton));
	m_uiElementsBasic.push_back(std::make_shared<TextButton>(connectButton));

	initializeTextOverlay(windowSize);

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::Idle | ModeState::NeedTextUpdate);
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();

	for (int i = 0; i < m_uiElementsBasic.size(); i++) m_uiElementsBasic[i] = nullptr;
	m_uiElementsBasic.clear();
}

void DeviceDiscoveryMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Device Discovery";
	TextOverlayBasic title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElementsBasic.push_back(std::make_shared<TextOverlayBasic>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlayBasic footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElementsBasic.push_back(std::make_shared<TextOverlayBasic>(footnote));
}

uint32_t DeviceDiscoveryMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		//This represent the large scrolling text box on the page. When clicking
		//this we're interested in whether or not any device has been selected.
		m_currentlySelectedDevice = ((FullScrollingTextBox*)m_uiElementsBasic[0].get())->getLastSelectedText();
		m_uiElementsBasic[0]->removeState(UIElementStateBasic::Clicked); //remove the clicked state from the scroll box
	}
	else if (i == 1)
	{
		//UI Element 1 is the device watcher button
		if (m_state == DeviceDiscoveryState::IDLE)
		{
			m_state = DeviceDiscoveryState::DISCOVERY;
			m_uiElementsBasic[1]->getText()->message = L"Stop Device Watcher";
			((FullScrollingTextBox*)m_uiElementsBasic[0].get())->clearText();
			return ModeState::Active;
		}
		else if (m_state == DeviceDiscoveryState::DISCOVERY)
		{
			m_state = DeviceDiscoveryState::IDLE;
			m_uiElementsBasic[1]->getText()->message = L"Start Device Watcher";
			return ModeState::Idle;
		}
	}
	else if (i == 2)
	{
		//this represents the connect/disconnect button. We should only
		//interact if the button is actually enabled.
	}
	return ModeState::Idle;
}

void DeviceDiscoveryMode::update()
{
	//when in active mode it means that the device watcher is running
}