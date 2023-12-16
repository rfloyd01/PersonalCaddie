#include "pch.h"
#include "DeviceDiscoveryMode.h"

DeviceDiscoveryMode::DeviceDiscoveryMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::PaleGray;
}

uint32_t DeviceDiscoveryMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	std::wstring buttonText = L"Connect to Device";
	std::wstring scrollText = L"Start the device watcher to begin enumerating nearby BluetoothLE devices...";

	FullScrollingTextBox deviceWatcherResults(windowSize, { 0.5, 0.575 }, { 0.85, 0.35 }, scrollText, 0.05f, false, false);
	TextButton deviceWatcherButton(windowSize, { 0.4, 0.25 }, { 0.12, 0.1 }, L"Start Device Watcher");
	TextButton connectButton(windowSize, { 0.6, 0.25 }, { 0.12, 0.1 }, buttonText);

	//Before initializing, see if we're currently connected to a BLE device or not as it will effect
	//the options available to use. Also, see if the device watcher is already turned on, if so, turn it off.
	std::pair<BLEState, uint64_t> action = { BLEState::Connected, 0 };
	m_mode_screen_handler(ModeAction::BLEConnection, (void*)&action);

	BLEState ble_state = BLEState::DeviceWatcherStatus;
	m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);

	if (m_deviceWatcherActive)
	{
		//Turn off the device watcher
		ble_state = BLEState::DisableDeviceWatcher;
		m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);
	}

	if (m_connected)
	{
		//We're already connected to a device so the connect button should have it's text 
		//updated to say "Disconnect" and be enabled. The device watcher button whould be disabled.
		connectButton.updateText(L"Disconnect from Device");
		deviceWatcherResults.clearText();
		deviceWatcherResults.addText(L"Disconnect from the current device to use the device watcher...", windowSize, false, false);
		deviceWatcherButton.setState(UIElementState::Disabled); //disable until we disconnect from the current device
	}
	else connectButton.setState(UIElementState::Disabled); //The button is disabled until an actual device is selected

	m_uiManager.addElement<FullScrollingTextBox>(deviceWatcherResults, L"Device Watcher Text Box");
	m_uiManager.addElement<TextButton>(deviceWatcherButton, L"Device Watcher Button");
	m_uiManager.addElement<TextButton>(connectButton, L"Connect Button");

	initializeTextOverlay(windowSize);

	return ModeState::CanTransfer;
}

void DeviceDiscoveryMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	//Check to see if the device watcher was left on, if so, turn it off
	if (m_deviceWatcherActive)
	{
		BLEState ble_state = BLEState::DisableDeviceWatcher;
		m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);
		m_deviceWatcherActive = false;
	}
}

void DeviceDiscoveryMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Device Discovery";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void DeviceDiscoveryMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	if (element->name == L"Device Watcher Text Box")
	{
		//This represent the large scrolling text box on the page. When clicking
		//this we're interested in whether or not any device has been selected.
		m_currentlySelectedDeviceAddress = m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->getLastSelectedText();
		if (m_currentlySelectedDeviceAddress == L"") return; //Nothing was actually selected so don't do anything

		//extract the 64-bit address from the selected string
		std::wstring trimText = L"Address: ";
		int addressStartIndex = m_currentlySelectedDeviceAddress.find(trimText) + trimText.length();
		m_currentlySelectedDeviceAddress = m_currentlySelectedDeviceAddress.substr(addressStartIndex);

		m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->removeState(UIElementState::Clicked); //remove the clicked state from the scroll box

		if (m_currentlySelectedDeviceAddress != L"")
		{
			//enable the connect button if it isn't already
			if (m_uiManager.getElement<TextButton>(L"Connect Button")->getState() & UIElementState::Disabled)
			{
				m_uiManager.getElement<TextButton>(L"Connect Button")->removeState(UIElementState::Disabled);
			}
		}
	}
	else if (element->name == L"Device Watcher Button")
	{
		if (!m_deviceWatcherActive)
		{
			//The device watcher is currently turned off, so clicking the button should turn it on. Its text
			//should also update to reflect that clicking again will turn off the device watcher.
			BLEState ble_state = BLEState::EnableDeviceWatcher;
			m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);
			m_deviceWatcherActive = true;

			m_uiManager.getElement<TextButton>(L"Device Watcher Button")->updateText(L"Stop Device Watcher");
			m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->clearText();

			//since we're clearing the text, nothing is selected so disable the connect button
			m_uiManager.getElement<TextButton>(L"Connect Button")->updateState(UIElementState::Disabled);
		}
		else
		{
			//The device watcher is currently turned on, so clicking the button should turn it off. Its text
			//should also update to reflect that clicking again will turn on the device watcher.
			BLEState ble_state = BLEState::DisableDeviceWatcher;
			m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);
			m_deviceWatcherActive = false;

			m_uiManager.getElement<TextButton>(L"Device Watcher Button")->updateText(L"Start Device Watcher");
		}
	}
	else if (element->name == L"Connect Button")
	{
		//If we're currently connected to a device then pressing this button will disconnect 
		//us from it. If we're not, and we've selected a device from the scroll
		//box, clicking this will attempt to make a connection
		BLEState ble_state = BLEState::Disconnect;
		uint64_t address = 0;
		if (!m_connected)
		{
			ble_state = BLEState::Reconnect;
			//auto deviceName = getCurrentlySelectedDevice();
			wchar_t* endString;
			
			address = std::wcstoull(&m_currentlySelectedDeviceAddress[0], &endString, 10); //convert the wide string representation of the address to an uint64_t
		}
		else m_connected = false; //if we're disconnecting from the current device then we can update the m_connected variable

		std::pair<BLEState, uint64_t> action = { ble_state, address };
		m_mode_screen_handler(ModeAction::BLEConnection, (void*)&action);
	}
}

void DeviceDiscoveryMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	if (!connectionStatus)
	{
		//if we lose the connection to the personal caddie while on the device discovery page it
	    //disables the Disconnect button and enables the Start device watcher button
		m_connected = false;
		m_uiManager.getElement<TextButton>(L"Device Watcher Button")->removeState(UIElementState::Disabled); //enable the device watcher button

		//update the connect button
		std::wstring buttonMessage = L"Connect to Device";
		m_uiManager.getElement<TextButton>(L"Connect Button")->updateState(UIElementState::Disabled);
		m_uiManager.getElement<TextButton>(L"Connect Button")->updateText(L"Connect to Device");

		//update the scroll box text
		std::wstring scrollText = L"Start the device watcher to begin enumerating nearby BluetoothLE devices...";
		m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->clearText();
		m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->addText(scrollText, { 0, 0 }, false, false);
	}
	else
	{
		//If we successfully connect to a device while on this page then it should
		//disable the device watcher button and enable the connect button.
		if (m_uiManager.getElement<TextButton>(L"Connect Button")->getState() & UIElementState::Disabled) m_uiManager.getElement<TextButton>(L"Connect Button")->removeState(UIElementState::Disabled);
		m_uiManager.getElement<TextButton>(L"Device Watcher Button")->setState(m_uiManager.getElement<TextButton>(L"Connect Button")->getState() | UIElementState::Disabled);

		//update the disconnect button
		std::wstring buttonMessage = L"Disconnect from Device";
		m_uiManager.getElement<TextButton>(L"Connect Button")->updateText(buttonMessage);

		//update the scroll box text
		std::wstring scrollText = L"Disconnect from the current device to use the device watcher...";
		m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->clearText();
		m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->addText(scrollText, { 0, 0 }, false, false);

		//Update the connection status on this page and turn off the device watcher
		m_connected = true;
		BLEState ble_state = BLEState::DisableDeviceWatcher;
		m_mode_screen_handler(ModeAction::BLEDeviceWatcher, (void*)&ble_state);
	}
}

void DeviceDiscoveryMode::getBLEConnectionStatus(bool status) { m_connected = status; };

void DeviceDiscoveryMode::getBLEDeviceWatcherStatus(bool status) { m_deviceWatcherActive = status; };

void DeviceDiscoveryMode::getString(std::wstring message)
{
	//This method gets called any time a new device is added to the device watcher. To avoid duplicate
	//entries we erase the text currently inside the scroll box and replaced it with the new string.
	m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->clearText();
	m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box")->addText(message, m_uiManager.getScreenSize(), true);
}