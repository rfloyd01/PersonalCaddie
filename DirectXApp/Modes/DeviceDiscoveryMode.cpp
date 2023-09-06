#include "pch.h"
#include "DeviceDiscoveryMode.h"

DeviceDiscoveryMode::DeviceDiscoveryMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::PaleGray;
}

uint32_t DeviceDiscoveryMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	std::wstring buttonText = L"Connect to Device";
	std::wstring scrollText = L"Start the device watcher to being enumerating nearby BluetoothLE devices...";
	if (initialState & DeviceDiscoveryState::CONNECTED)
	{
		//We're already connected to a device so the connect button should have it's text 
		//updated and be enabled. The device watcher button whould be disabled.
		buttonText = L"Disconnect from Device";
		scrollText = L"Disconnect from the current device to use the device watcher...";
	}

	FullScrollingTextBox deviceWatcherResults(windowSize, { 0.5, 0.575 }, { 0.85, 0.35 }, scrollText, 0.05f, false, false);
	TextButton deviceWatcherButton(windowSize, { 0.4, 0.25 }, { 0.12, 0.1 }, L"Start Device Watcher");
	TextButton connectButton(windowSize, { 0.6, 0.25 }, { 0.12, 0.1 }, buttonText);

	if (!(initialState & DeviceDiscoveryState::CONNECTED)) connectButton.setState(UIElementState::Disabled); //The button is disabled until an actual device is selected
	else deviceWatcherButton.setState(UIElementState::Disabled); //disable until we disconnect from the current device

	m_uiElements.push_back(std::make_shared<FullScrollingTextBox>(deviceWatcherResults));
	m_uiElements.push_back(std::make_shared<TextButton>(deviceWatcherButton));
	m_uiElements.push_back(std::make_shared<TextButton>(connectButton));

	initializeTextOverlay(windowSize);

	m_state = initialState;

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::NeedTextUpdate);
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void DeviceDiscoveryMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Device Discovery";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

uint32_t DeviceDiscoveryMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		//This represent the large scrolling text box on the page. When clicking
		//this we're interested in whether or not any device has been selected.
		m_currentlySelectedDeviceAddress = ((FullScrollingTextBox*)m_uiElements[0].get())->getLastSelectedText();

		//extract the 64-bit address from the selected string
		std::wstring trimText = L"Address: ";
		int addressStartIndex = m_currentlySelectedDeviceAddress.find(trimText) + trimText.length();
		m_currentlySelectedDeviceAddress = m_currentlySelectedDeviceAddress.substr(addressStartIndex);

		m_uiElements[0]->removeState(UIElementState::Clicked); //remove the clicked state from the scroll box

		if (m_currentlySelectedDeviceAddress != L"")
		{
			//enable the connect button if it isn't already
			if (m_uiElements[2]->getState() & UIElementState::Disabled)
			{
				m_uiElements[2]->removeState(UIElementState::Disabled);
			}
		}
	}
	else if (i == 1)
	{
		//UI Element 1 is the device watcher button
		if (!(m_state & DeviceDiscoveryState::DISCOVERY))
		{
			m_uiElements[1]->getText()->message = L"Stop Device Watcher";
			((FullScrollingTextBox*)m_uiElements[0].get())->clearText();

			//since we're clearing the text, nothing is selected so disable the connect button
			m_uiElements[2]->setState(m_uiElements[2]->getState() | UIElementState::Disabled);
			//return ModeState::Active;
		}
		else if (m_state & DeviceDiscoveryState::DISCOVERY)
		{
			//m_state ^= (DeviceDiscoveryState::DISCOVERY | DeviceDiscoveryState::IDLE); //switch the discovery and idle states
			m_uiElements[1]->getText()->message = L"Start Device Watcher";
			//return ModeState::Idle;
		}
		m_state ^= DeviceDiscoveryState::DISCOVERY; //switch the discovery and idle states
	}
	else if (i == 2)
	{
		//this represents the connect/disconnect button. If we're currently
		//connected to a device then pressing this button will disconnect 
		//us from it. If we're not, and we've selected a device from the scroll
		//box, clicking this will attempt to make a connection
		if (m_state & DeviceDiscoveryState::CONNECTED) m_state |= DeviceDiscoveryState::DISCONNECT; //let the mode screen know we wish to disconnect
		else m_state |= DeviceDiscoveryState::ATTEMPT_CONNECT; //let the mode screen know we wish to connect
	}
	return m_state;
}

void DeviceDiscoveryMode::update()
{
	//when in active mode it means that the device watcher is running
}