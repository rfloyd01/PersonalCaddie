#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMenuMode.h"
#include "DevelopmentMenuMode.h"
#include "DeviceDiscoveryMode.h"
#include "UITestMode.h"
#include "GraphMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU),
	alert_active(false),
	button_pressed(false),
	personal_caddy_event(PersonalCaddieEventType::NONE)
{
	//Create instances of all the different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS_MENU)] = std::make_shared<SettingsMenuMode>();
	m_modes[static_cast<int>(ModeType::DEVICE_DISCOVERY)] = std::make_shared<DeviceDiscoveryMode>();
	m_modes[static_cast<int>(ModeType::DEVELOPER_TOOLS)] = std::make_shared<DevelopmentMenuMode>();
	m_modes[static_cast<int>(ModeType::UI_TEST_MODE)] = std::make_shared<UITestMode>();
	m_modes[static_cast<int>(ModeType::GRAPH_MODE)] = std::make_shared<GraphMode>();

	//Set default times for various timers (in milliseconds)
	alert_timer_duration = 5000;
	button_pressed_duration = 100;
}

void ModeScreen::Initialize(
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_inputProcessor = input;
	m_renderer = renderer;

	//Load the main mode (which is set in the constructor) and pass any resources
	//generated to the master renderer
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize());
	m_renderer->CreateModeResources();
}

void ModeScreen::setPersonalCaddie(_In_ std::shared_ptr<PersonalCaddie> const& pc)
{
	m_personalCaddie = pc;
}

void ModeScreen::update()
{
	//TODO: the first thing to update is any new data from the Personal Caddie

	//check for any input form the mouse/keyboard that needs processing by the current mode
	auto inputUpdate = m_inputProcessor->update();
	if (inputUpdate->currentPressedKey != KeyboardKeys::DeadKey)
	{
		processKeyboardInput(inputUpdate->currentPressedKey);
		m_inputProcessor->setKeyboardState(KeyboardState::KeyProcessed); //let the input processor now to deactivate this key until it's released
	}

	processMouseInput(inputUpdate);

	//after processing input, see if there are any event handlers that were triggered
	processEvents();

	//finally, check to see if there are any timers that are going on or expired
	processTimers();

	//after all input, events and timers have been handled defer to the current mode
	//to update its state if necessary. This only occurs when in the Active ModeState
	if (m_modeState & ModeState::Active) m_modes[static_cast<int>(m_currentMode)]->update();
}

void ModeScreen::processKeyboardInput(winrt::Windows::System::VirtualKey pressedKey)
{
	//Depending on what the current mode is and the current ModeState the same key may be 
	//processed differently. We need to look at a combination of the current mode, ModeState
	//and pressedKey to figure out what action to take

	//TODO: I can already tell that this method will be a rat's nest of if statements. Should
	//I try and split this into multiple methods?

	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
		if (m_modeState & ModeState::CanTransfer)
		{
			//If we're on the Main Menu screen then pressing escape will quite the application.
			//Otherwise, if we're in a different mode we just go back to the Main Menu
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				OutputDebugString(L"Quitting the program.\n");
				//TODO: disconnect from the Personal Caddie if connected and release
				//any resources
			}
			else if (m_currentMode == ModeType::SETTINGS_MENU || m_currentMode == ModeType::DEVELOPER_TOOLS)
			{
				changeCurrentMode(ModeType::MAIN_MENU);
			}
			else if (m_currentMode == ModeType::DEVICE_DISCOVERY)
			{
				changeCurrentMode(ModeType::SETTINGS_MENU);
			}
			else if (m_currentMode == ModeType::UI_TEST_MODE || m_currentMode == ModeType::GRAPH_MODE)
			{
				changeCurrentMode(ModeType::DEVELOPER_TOOLS);
			}
		}
		else if (m_modeState & ModeState::Active)
		{
			if (m_currentMode == ModeType::DEVICE_DISCOVERY)
			{
				//For now pressing esc. will just change the mode back to CanTransfer
				m_modeState = ModeState::CanTransfer;
			}
		}
		break;
	case winrt::Windows::System::VirtualKey::Number1:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				//This should take us to the free swing menu but this hasn't been implemented yet
				//so display an alert for now
				createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
			}
			else if (m_currentMode == ModeType::SETTINGS_MENU)
			{
				changeCurrentMode(ModeType::DEVICE_DISCOVERY);
			}
			else if (m_currentMode == ModeType::DEVELOPER_TOOLS)
			{
				changeCurrentMode(ModeType::UI_TEST_MODE);
			}
		}

		break;
	case winrt::Windows::System::VirtualKey::Number2:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::DEVELOPER_TOOLS)
			{
				if (!m_personalCaddie->ble_device_connected) createAlert(L"Must be connected to a Personal Caddie to access this mode", UIColor::Red);
				else changeCurrentMode(ModeType::GRAPH_MODE);
			}
		}

		break;
	case winrt::Windows::System::VirtualKey::Number5:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::SETTINGS_MENU);
			}
		}
		break;
	case winrt::Windows::System::VirtualKey::Number6:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::DEVELOPER_TOOLS);
			}
		}
		break;
	}
}

void ModeScreen::processMouseInput(InputState* inputState)
{
	//need to poll all of the 2D elements in the current mode to see if the mouse is
	//over any of them
	auto uiElements = m_modes[static_cast<int>(m_currentMode)]->getUIElements();
	for (int i = 0; i < uiElements.size(); i++)
	{
		uint32_t uiElementState = uiElements[i]->update(inputState);

		if (uiElementState & UIElementState::NeedTextPixels)
		{
			getTextRenderPixels(uiElements[i]->setTextDimension()); //get the necessary pixels
			uiElements[i]->repositionText(); //see if any text needs to be repositioned after getting new dimensions
			uiElements[i]->resize(m_renderer->getCurrentScreenSize()); //and then resize the ui element
		}
		else if ((uiElementState & UIElementState::Clicked) && inputState->mouseClick)
		{
			//The current UI Element has been clicked, see if clicking the button has
			//any effect outside of the UI Element (like clicking the device watcher
			//button in device discovery mode).
			uint32_t new_state = m_modes[static_cast<int>(m_currentMode)]->handleUIElementStateChange(i);

			//start a short timer that will change the color of the button from PRESSED to NOT_PRESSED
			button_pressed = true;
			button_pressed_timer = std::chrono::steady_clock::now();

			//See if the button press forced us into or out of active mode
			if (m_modeState & ModeState::Active)
			{
				if (!(new_state & ModeState::Active)) leaveActiveState();
				else stateUpdate();
			}
			else if (!(m_modeState & ModeState::Active))
			{
				if (new_state & ModeState::Active) enterActiveState();
				else stateUpdate();
			}

			break;
		}
	}

	//reset input states if necessary
	if (inputState->mouseClick) m_inputProcessor->setMouseState(MouseState::ButtonProcessed); //let the input processor know that the click has been handled
	if (inputState->scrollWheelDirection != 0) inputState->scrollWheelDirection = 0; //let the input processor know that mouse scroll has been handled
}

void ModeScreen::processEvents()
{
	//There are various event handlers throught the application. Any handler that needs to trickle
	//information to the current mode will do so in this method.
	switch (personal_caddy_event)
	{
	case PersonalCaddieEventType::PC_ALERT:
	case PersonalCaddieEventType::BLE_ALERT:
	case PersonalCaddieEventType::IMU_ALERT:
		//An alert was passed through, this means the text vector of the current mode has been 
		//altered. We need to pass this updated text to the master renderer

		alert_timer = std::chrono::steady_clock::now(); //set/reset the alert timer
		alert_active = true;
		personal_caddy_event = PersonalCaddieEventType::NONE;
		break;
	}

	//Other event types can be added down here once they're created
}

void ModeScreen::processTimers()
{
	if (alert_active)
	{
		//we have an active alert, see if the alert timer has expired yet and if so delete
		//all active alerts
		auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - alert_timer).count();
		if (time_elapsed >= alert_timer_duration)
		{
			//the timer has gone off so turn the timer off so remove all active alerts
			alert_active = false;
			m_modes[static_cast<int>(m_currentMode)]->removeAlerts();
		}
	}

	if (button_pressed)
	{
		//a button has been pressed recently, see if the button press timer has expired yet and if so change the color
		//of all buttons in the current mode to be PRESSED
		auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - button_pressed_timer).count();
		if (time_elapsed >= button_pressed_duration)
		{
			//the timer has gone off so turn the timer off
			button_pressed = false;

			//then change the color of all pressed buttons
			auto uiElements = m_modes[static_cast<int>(m_currentMode)]->getUIElements();
			for (int i = 0; i < uiElements.size(); i++)
			{
				if (uiElements[i]->getState() & UIElementState::Clicked)
				{
					//The ui button class overrides the ui element setState() method so case to a UI button
					//before setting the state to idle
					uiElements[i]->removeState(UIElementState::Clicked);
				}
			}
		}
	}
}

void ModeScreen::changeCurrentMode(ModeType mt)
{
	//This method unitializes the current mode and initializes the mode selected.
	//Any active alerts will get copied into the text of the new mode
	if (mt == m_currentMode) return; //no need to change anything, already on this mode

	//First, get any active alerts before deleting the text and color map of the 
	//current mode
	auto currentAlerts = m_modes[static_cast<int>(m_currentMode)]->removeAlerts();// .getText();

	//Check to see if the current mode is in the active state, if so then take it out
	//of that state. Then uninitialize the mode.
	if (m_modeState & ModeState::Active) leaveActiveState();
	m_modeState = 0;
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	uint32_t startingModeState = 0;

	//Some modes can be initialized with starting states, check to see if the new
	//mode applies
	switch (mt)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//The device discovery mode state depends on whether or not we're currently
		//connected to a BLE device
		if (m_personalCaddie->ble_device_connected) startingModeState |= DeviceDiscoveryState::CONNECTED;
	}
	}

	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize(), startingModeState);
	if (m_modeState & ModeState::PersonalCaddieSensorIdleMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
	else if (m_modeState & ModeState::PersonalCaddieSensorActiveMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
	
	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	if (currentAlerts.getText()->message != L"") m_modes[static_cast<int>(m_currentMode)]->createAlert(currentAlerts);
}

void ModeScreen::getTextRenderPixels(std::vector<UIText*> const& text)
{
	//Sets the size for the text overlay render box of the given text element
	for (int i = 0; i < text.size(); i++) m_renderer->setTextLayoutPixels(text[i]);
}

std::vector<std::shared_ptr<UIElement> > const& ModeScreen::getCurrentModeUIElements()
{
	//returns a reference to all UI elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getUIElements();
}

void ModeScreen::resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize)
{
	auto uiElements = getCurrentModeUIElements();
	for (int i = 0; i < uiElements.size(); i++) uiElements[i]->resize(windowSize);
}

const UIColor ModeScreen::getBackgroundColor()
{
	//returns the background color of the current mode
	return m_modes[static_cast<int>(m_currentMode)]->getBackgroundColor();
}

void ModeScreen::createAlert(std::wstring message, UIColor color)
{
	m_modes[static_cast<int>(m_currentMode)]->createAlert(message, color, m_renderer->getCurrentScreenSize());

	//after creating the alert, set the alert timer
	alert_active = true;
	alert_timer = std::chrono::steady_clock::now();
}

void ModeScreen::PersonalCaddieHandler(PersonalCaddieEventType pcEvent, void* eventArgs)
{
	//This method will get automatically called when certain things happen with the 
	//Personal Caddie device. For example, if we lose the connection to the Personal
	//Caddie a BLE_ALERT will come here so we can render it on the screen. Another 
	//example would be when we're in Device Discovery mode, we can send a list of all
	//the found Bluetooth devices here so the Discovery mode can process them.

	personal_caddy_event = pcEvent;
	switch (pcEvent)
	{
	case PersonalCaddieEventType::CONNECTION_EVENT:
	{
		//This is a special type of BLE event. We've either connected to a new device
		//or disconnected from the current one. This may potentially enable or disable
		//some features depending on the current mode.
		
		m_modes[static_cast<int>(m_currentMode)]->handlePersonalCaddieConnectionEvent(m_personalCaddie->ble_device_connected);
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Blue);

		break;
	}
	case PersonalCaddieEventType::BLE_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Blue);
		
		break;
	}
	case PersonalCaddieEventType::PC_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Yellow);
		break;
	}
	case PersonalCaddieEventType::DEVICE_WATCHER_UPDATE:
	{
		std::set<DeviceInfoDisplay>* foundDevices = (std::set<DeviceInfoDisplay>*)eventArgs;
		std::wstring devices;
		for (auto it = foundDevices->begin(); it != foundDevices->end(); it++)
		{
			devices += L"Name: " + it->device_name;
			devices += L", Address: " + std::to_wstring(it->device_address.first) + L"\n";
		}

		//Update the text in the device discovery text box
		if (m_currentMode == ModeType::DEVICE_DISCOVERY)
		{
			auto textBox = (FullScrollingTextBox*)(getCurrentModeUIElements()[0].get());
			textBox->clearText(); //clear the text each time to prevent adding duplicates
			textBox->addText(devices, m_renderer->getCurrentScreenSize(), true); //this new text will get resized in the main update loop
		}
		break;
	}
	case PersonalCaddieEventType::DATA_READY:
	{
		//The imu on the personal caddie has finished taking readings and has sent the data over.
		//Send the data to the current mode if it needs it.
		if (m_currentMode == ModeType::GRAPH_MODE)
		{
			((GraphMode*)m_modes[static_cast<int>(ModeType::GRAPH_MODE)].get())->addData(m_personalCaddie->getSensorData(), m_personalCaddie->getMaxODR());
		}
		break;
	}
	}

}

void ModeScreen::enterActiveState()
{
	m_modeState ^= (ModeState::Active | ModeState::CanTransfer); //toggle the active and can transfer states

	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//Going into active mode while in device discovery means that we need to start the BLEAdvertisement watcher of 
		//the Personal Caddie. The device watcher will only alert us when a new device is found. If it has been turned
		//on previously and already found all devices in the area, then we won't actually get any updates. Because of this
		//we grab the current list and send it to the device discovery page before turning on the watcher.
		auto foundDevices = m_personalCaddie->getScannedDevices();
		std::wstring devices;
		for (auto it = foundDevices->begin(); it != foundDevices->end(); it++)
		{
			devices += L"Name: " + it->device_name;
			devices += L", Address: " + std::to_wstring(it->device_address.first) + L"\n";
		}
		auto textBox = (FullScrollingTextBox*)(getCurrentModeUIElements()[0].get());
		textBox->clearText(); //clear the text each time to prevent adding duplicates
		textBox->addText(devices, m_renderer->getCurrentScreenSize(), true); //this new text will get resized in the main update loop

		m_personalCaddie->startBLEAdvertisementWatcher();
		break;
	}
	case ModeType::GRAPH_MODE:
	{
		//When we first enter graph mode the personal caddie is automatically put into the sensor idle
		//power mode. Entering the active state switches the personal caddie power mode to sensor active
		//and also enables data notifications.
		m_modeState ^= (ModeState::PersonalCaddieSensorActiveMode | ModeState::PersonalCaddieSensorIdleMode); //swap the idle and active mode flags. We also can't transfer modes in the active state
		m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
		m_personalCaddie->enableDataNotifications();
		break;
	}
	}
}

void ModeScreen::stateUpdate()
{
	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//The connect/disconnect button was clicked. Attempt to connect
		//to the specified device, or disconnect from the current one.
		if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & DeviceDiscoveryState::DISCONNECT)
		{
			//disconnect from the current device
			m_personalCaddie->disconnectFromDevice();
		}
		else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & DeviceDiscoveryState::ATTEMPT_CONNECT)
		{
			//attempt to connect to the device currently selected
			wchar_t* endString;
			uint64_t deviceAddress = std::wcstoull(&((DeviceDiscoveryMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentlySelectedDevice()[0], &endString, 10);
			m_personalCaddie->connectToDevice(deviceAddress);
		}
	}
	}
}

void ModeScreen::leaveActiveState()
{
	m_modeState ^= ModeState::Active; //turn off the active state

	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//Leaving active mode while in device discovery means that we need to stop the BLEAdvertisement watcher of 
		//the Personal Caddie
		m_personalCaddie->stopBLEAdvertisementWatcher();
	}
	case ModeType::GRAPH_MODE:
	{
		//Leaving the active state while in graph mode causes the personal caddie to enter the sensor idle 
		//power mode and turns off data notifications.
		m_personalCaddie->disableDataNotifications();
		m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
	}
	}
}