#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMenuMode.h"
#include "DevelopmentMenuMode.h"
#include "DeviceDiscoveryMode.h"
#include "UITestMode.h"
#include "GraphMode.h"
#include "IMUSettingsMode.h"
#include "MadgwickTestMode.h"
#include "CalibrationMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

#include <functional>

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
	m_modes[static_cast<int>(ModeType::IMU_SETTINGS)] = std::make_shared<IMUSettingsMode>();
	m_modes[static_cast<int>(ModeType::MADGWICK)] = std::make_shared<MadgwickTestMode>();
	m_modes[static_cast<int>(ModeType::CALIBRATION)] = std::make_shared<CalibrationMode>();

	//After creating the modes, bind the mode handler method to the mode class so that all
	//different mode types can use it
	Mode::setHandlerMethod(std::bind(&ModeScreen::ModeHandler, this, std::placeholders::_1, std::placeholders::_2));

	//Set default times for various timers (in milliseconds)
	alert_timer_duration = 2500;
	button_pressed_duration = 100;
}

winrt::fire_and_forget ModeScreen::Initialize(
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_inputProcessor = input;
	m_renderer = renderer;

	//Load the main mode (which is set in the constructor) and pass any resources
	//generated to the master renderer
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize());
	co_await m_renderer->CreateModeResourcesAsync();
	m_renderer->FinalizeCreateDeviceResources();
}

void ModeScreen::setPersonalCaddie(_In_ std::shared_ptr<PersonalCaddie> const& pc)
{
	m_personalCaddie = pc;
}

void ModeScreen::update()
{
	//check for any input form the mouse/keyboard that needs processing by the current mode
	auto inputUpdate = m_inputProcessor->update();
	if (inputUpdate->currentPressedKey != KeyboardKeys::DeadKey)
	{
		processKeyboardInput(inputUpdate->currentPressedKey);
		m_inputProcessor->setKeyboardState(KeyboardState::KeyProcessed); //let the input processor know to deactivate this key until it's released
	}

	processMouseInput(inputUpdate);

	//after processing input, see if there are any event handlers that were triggered
	processEvents();

	//finally, check to see if there are any timers that are going on or expired
	//processTimers();

	//After all input, timers and events have been processed we defer to the 
	//current mode on handling this new information
	//m_modes[static_cast<int>(m_currentMode)]->update();

	//Call any necessary functions based on the current mode state
	//TODO: Make this a separate method when refactoring in the future
	//TODO: Make the pickMaterial method return a new mode state
	if (m_modeState & ModeState::NeedMaterial)
	{
		auto volumeElements = m_modes[static_cast<int>(m_currentMode)]->getVolumeElements();
		auto materialTypes = m_modes[static_cast<int>(m_currentMode)]->getMaterialTypes();
		for (int i = 0; i < volumeElements.size(); i++)
		{
			m_renderer->setMaterialAndMesh(volumeElements[i], materialTypes[i]);
		}
		m_modeState ^= ModeState::NeedMaterial; //TODO: Not a safe call as materials may not actually be loaded when this get's called the first time
	}

	//After all input, events and timers have been handled we check to see if there's anything
	//the current mode needs from the mode state class (i.e. putting the Personal Caddie into sensor active
	//mode). When that check is complete we then defer to the current mode
	//to update its own state if necessary. This only occurs when in the Active ModeState
	/*if (m_modeState & ModeState::Enter_Active)
	{
		enterActiveState();
		m_modeState ^= ModeState::Enter_Active;
	}
	else if (m_modeState & ModeState::Leave_Active)
	{
		leaveActiveState();
		m_modeState ^= ModeState::Leave_Active;
	}
	else if (m_modeState & ModeState::Active)
	{
		stateUpdate();
	}*/

	//Have the current state make any necessary updates based on the 
	//input and events that have just been processed
	getCurrentMode()->uiUpdate(); //Updates based on interaction with UI Elements on screen
	getCurrentMode()->update();  //Updates based on internal state of the mode
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
			else if (m_currentMode == ModeType::DEVICE_DISCOVERY || m_currentMode == ModeType::IMU_SETTINGS || m_currentMode == ModeType::CALIBRATION)
			{
				changeCurrentMode(ModeType::SETTINGS_MENU);
			}
			else if (m_currentMode == ModeType::UI_TEST_MODE || m_currentMode == ModeType::GRAPH_MODE || m_currentMode == ModeType::MADGWICK)
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
			else if (m_currentMode == ModeType::MADGWICK)
			{
				//Update the data type to display on screen
				((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(1);
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
			else if (m_currentMode == ModeType::SETTINGS_MENU)
			{
				//TODO: After completing design work on IMU settings page,
				//make it so you must be connected before going there
				//if (!m_personalCaddie->ble_device_connected) createAlert(L"Must be connected to a Personal Caddie to access this mode", UIColor::Red);
				//else changeCurrentMode(ModeType::GRAPH_MODE);
				changeCurrentMode(ModeType::IMU_SETTINGS);
			}
			else if (m_currentMode == ModeType::MADGWICK)
			{
				//Update the data type to display on screen
				((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(2);
			}
		}

		break;
	case winrt::Windows::System::VirtualKey::Number3:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::DEVELOPER_TOOLS)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::MADGWICK);
			}
			else if (m_currentMode == ModeType::SETTINGS_MENU)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::CALIBRATION);
			}
			else if (m_currentMode == ModeType::MADGWICK)
			{
				//Update the data type to display on screen
				((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(3);
			}
		}
		break;
	case winrt::Windows::System::VirtualKey::Number4:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::SETTINGS_MENU);
			}
			else if (m_currentMode == ModeType::MADGWICK)
			{
				//Update the data type to display on screen
				((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(4);
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
				changeCurrentMode(ModeType::DEVELOPER_TOOLS);
			}
			else if (m_currentMode == ModeType::MADGWICK)
			{
				//Update the data type to display on screen
				((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(5);
			}
		}
		break;
	case winrt::Windows::System::VirtualKey::Number6:
		if (m_currentMode == ModeType::MADGWICK)
		{
			//Update the data type to display on screen
			((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->switchDisplayDataType(6);
		}
		break;
	case winrt::Windows::System::VirtualKey::Enter:
		if (m_currentMode == ModeType::MADGWICK)
		{
			//Pressing the enter key toggles whether or not we can see a live feed of data
			//coming from the sensor
			((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->toggleDisplayData();
		}
		break;
	case winrt::Windows::System::VirtualKey::Space:
	{
		if (m_currentMode == ModeType::MADGWICK)
		{
			//Pressing the space key will reset the heading offset rotation quaternion for the Personal Caddie
			glm::quat heading_offset = ((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentHeadingOffset();
			((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->setHeadingOffset(heading_offset); //set the heading offset quaternion in Madgwick test mode
			m_personalCaddie->setHeadingOffset(heading_offset);
		}
		break;
	}
	case winrt::Windows::System::VirtualKey::Up:
	{
		if (m_currentMode == ModeType::MADGWICK)
		{
			//Pressing the up key will swap the current sensor fusion filter
			((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->toggleFilter(); //set the heading offset quaternion in Madgwick test mode
		}
		break;
	}
	}
}

void ModeScreen::processMouseInput(InputState* inputState)
{
	//If the mouse hasn't moved, been clicked, or the scroll wheel
	//hasn't rotated then there's nothing to process here.
	if (!inputState->mouseClick && inputState->scrollWheelDirection == 0 && ((inputState->mousePosition.x == m_previousMousePosition.x) && (inputState->mousePosition.y == m_previousMousePosition.y)))
	{
		return;
	}

	//If the mouse HAS moved, clicked or scrolled then the UIElementManager of the currently
	//active mode will check all of the UIElements that are in the same section of the screen 
	//(the screen is partitioned into a grid) as the mouse currently is to see if this mouse 
	//input changes any of their states.
	m_modes[static_cast<int>(m_currentMode)]->getUIElementManager().updateGridSquareElements(inputState);
	m_previousMousePosition = inputState->mousePosition; //update the mouse position variable

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

	//Check to see if any active alerts need to be removed
	m_modes[static_cast<int>(m_currentMode)]->checkAlerts();
}

void ModeScreen::processTimers()
{
	//if (alert_active)
	//{
	//	//we have an active alert, see if the alert timer has expired yet and if so delete
	//	//all active alerts
	//	auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - alert_timer).count();
	//	if (time_elapsed >= alert_timer_duration)
	//	{
	//		//the timer has gone off so turn the timer off so remove all active alerts
	//		alert_active = false;
	//		m_modes[static_cast<int>(m_currentMode)]->removeAlerts();
	//	}
	//}

	//if (button_pressed)
	//{
	//	//a button has been pressed recently, see if the button press timer has expired yet and if so change the color
	//	//of all buttons in the current mode to be PRESSED
	//	auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - button_pressed_timer).count();
	//	if (time_elapsed >= button_pressed_duration)
	//	{
	//		//the timer has gone off so turn the timer off
	//		button_pressed = false;

	//		//then change the color of all pressed buttons
	//		auto uiElements = m_modes[static_cast<int>(m_currentMode)]->getUIElements();
	//		for (int i = 0; i < uiElements.size(); i++)
	//		{
	//			if (uiElements[i]->getState() & UIElementState::Clicked)
	//			{
	//				//The ui button class overrides the ui element setState() method so case to a UI button
	//				//before setting the state to idle
	//				uiElements[i]->removeState(UIElementState::Clicked);
	//			}
	//		}
	//	}
	//}
}

void ModeScreen::changeCurrentMode(ModeType mt)
{
	//This method unitializes the current mode and initializes the mode selected.
	//Any active alerts will get copied into the text of the new mode
	if (mt == m_currentMode) return; //no need to change anything, already on this mode

	//First, get any active alerts before deleting the text and color map of the 
	//current mode
	//auto currentAlerts = m_modes[static_cast<int>(m_currentMode)]->removeAlerts();// .getText();
	auto currentAlerts = m_modes[static_cast<int>(m_currentMode)]->removeAlerts();

	//Check to see if the current mode is in the active state, if so then take it out
	//of that state. Then uninitialize the mode.
	if (m_modeState & ModeState::Active) leaveActiveState();

	//If we're in sensor active mode, we need to first enter sensor idle mode, and then connected mode
	//if (m_modeState & ModeState::PersonalCaddieSensorIdleMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::CONNECTED_MODE);
	//else if (m_modeState & ModeState::PersonalCaddieSensorActiveMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);

	m_modeState = 0;
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	uint32_t startingModeState = 0;

	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize(), startingModeState);
	//if (m_modeState & ModeState::PersonalCaddieSensorIdleMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
	//else if (m_modeState & ModeState::PersonalCaddieSensorActiveMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
	
	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	//if (currentAlerts.getText()->message != L"") m_modes[static_cast<int>(m_currentMode)]->createAlert(currentAlerts);
	m_modes[static_cast<int>(m_currentMode)]->overwriteAlerts(currentAlerts);
}

void ModeScreen::getTextRenderPixels(std::vector<UIText*> const& text)
{
	//Sets the size for the text overlay render box of the given text element
	for (int i = 0; i < text.size(); i++) m_renderer->setTextLayoutPixels(text[i]);
}

//std::vector<std::shared_ptr<UIElement> > const& ModeScreen::getCurrentModeUIElements()
//{
//	//returns a reference to all UI elements to be rendered on screen
//	return m_modes[static_cast<int>(m_currentMode)]->getUIElements();
//}

std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& ModeScreen::getCurrentModeUIElementMap()
{
	return getCurrentMode()->getUIElementManager().getElementsMap();
}

void ModeScreen::resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize)
{
	//auto uiElements = getCurrentModeUIElements();
	//for (int i = 0; i < uiElements.size(); i++) uiElements[i]->resize(windowSize);

	//Update the m_screenSize variable of the current modes UIElementManager
	m_modes[static_cast<int>(m_currentMode)]->getUIElementManager().updateScreenSize(windowSize);
}

std::vector<std::shared_ptr<VolumeElement> > const& ModeScreen::getCurrentModeVolumeElements()
{
	//returns a reference to all 3D elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getVolumeElements();
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
	//alert_active = true;
	//alert_timer = std::chrono::steady_clock::now();
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
		//TODO: Instead of sending a message of text, send an instance of the PeronsalCaddiePowerMode
		//enum which contains the new mode. Craft an appropriate message depending on the mode obtained.
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Yellow);

		if (alertText == L"The Personal Caddie has been placed into Sensor Idle Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
			//if (!(m_modeState & (ModeState::PersonalCaddieSensorIdleMode | ModeState::PersonalCaddieSensorActiveMode)))
			//{
			//	//If we're being put into sensor idle mode, and neither the idle mode or active mode flags are
			//	//active, it means we're trying to get from active mode to connected mode
			//	m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::CONNECTED_MODE);
			//}
			//else if (m_currentMode == ModeType::CALIBRATION)
			//{
			//	((CalibrationMode*)m_modes[static_cast<int>(m_currentMode)].get())->stopDataCapture();
			//	m_modeState ^= ModeState::Active; //re-enter the active state to resume updates
			//}
			//else if (m_currentMode == ModeType::MADGWICK)
			//{
			//	//In Madwick testing mode we go straight into sensor idle mode upon entering
			//	m_modeState ^= (ModeState::PersonalCaddieSensorIdleMode | ModeState::PersonalCaddieSensorActiveMode);
			//	((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->setHeadingOffset(m_personalCaddie->getHeadingOffset()); //set the heading offset quaternion
			//	m_personalCaddie->setMadgwickBeta(2.5f); //increase Madwick filter beta value for faster convergence
			//	m_personalCaddie->enableDataNotifications();
			//}
		}
		else if (alertText == L"The Personal Caddie has been placed into Sensor Active Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
			//if (m_currentMode == ModeType::CALIBRATION)
			//{
			//	((CalibrationMode*)m_modes[static_cast<int>(m_currentMode)].get())->startDataCapture();
			//	m_modeState ^= ModeState::Active; //re-enter the active state to resume updates
			//}
		}
		else if (alertText == L"The Personal Caddie has been placed into Connected Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::CONNECTED_MODE);

			//When entering connected mode, we make sure that data notifications are turned off
			m_personalCaddie->disableDataNotifications();
		}
		break;
	}
	case PersonalCaddieEventType::IMU_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Green);

		if (alertText == L"Updated Calibration Info" || alertText == L"Updated Axis Orientation Info")
		{
			if (m_currentMode == ModeType::CALIBRATION)
			{
				//((CalibrationMode*)m_modes[static_cast<int>(m_currentMode)].get())->updateComplete();
				m_modeState ^= ModeState::Active; //completing the update causes us to leave active mode
			}
		}
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

		getCurrentMode()->getString(devices); //pass the list of devices to the current mode

		////Update the text in the device discovery text box
		//if (m_currentMode == ModeType::DEVICE_DISCOVERY)
		//{
		//	auto textBox = (FullScrollingTextBox*)(getCurrentModeUIElements()[0].get());
		//	textBox->clearText(); //clear the text each time to prevent adding duplicates
		//	textBox->addText(devices, m_renderer->getCurrentScreenSize(), true); //this new text will get resized in the main update loop
		//}
		break;
	}
	case PersonalCaddieEventType::NOTIFICATIONS_TOGGLE:
	{
		//We've either turned data notifications on or off. If we've turned them on, then we need
		//to put the Personal Caddie into the power mode dictated by the m_modeState variable. If we've
		//turned them off then we put the Personal Caddie into sensor idle mode to stop transfering data.

		//Note: Notifactions need to be enabled before data transfer starts, which is why we wait to change into 
		//sensor active mode until this event occurs. The opposite is true when turning notifications off, we need
		//to leave sensor active mode first, and then disable notifications. For that reason we don't leave
		//sensor active mode in this event.

		//Enabling notifications have the ability to a
		std::wstring alertText = L"Data Notifications have been turned " + *((std::wstring*)eventArgs) + L"\n";
		createAlert(alertText, UIColor::Blue);

		if (*((std::wstring*)eventArgs) == L"On")
		{
			//m_modes[static_cast<int>(m_currentMode)]->ble_NotificationsChange(1);
			//if (m_modeState & ModeState::PersonalCaddieSensorIdleMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
			//else if (m_modeState & ModeState::PersonalCaddieSensorActiveMode) m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);

			//TODO: Due to a timing bug in the firmware we need to wait for notifications to be turned on before
			//entering sensor idle mode, and we must enter sensor idle mode before sensor active mode. This means 
			//that any time notifications are enabled we should default to changing the power mode to sensor idle
			m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
		}
		else m_modes[static_cast<int>(m_currentMode)]->ble_NotificationsChange(0);
		
		
		break;
	}
	case PersonalCaddieEventType::DATA_READY:
	{
		//The imu on the personal caddie has finished taking readings and has sent the data over.
		//Send the data to the current mode if it needs it.
		
		if (m_currentMode == ModeType::GRAPH_MODE || m_currentMode == ModeType::CALIBRATION)
		{
			m_modes[static_cast<int>(m_currentMode)]->addData(m_personalCaddie->getSensorData(), m_personalCaddie->getMaxODR(), m_personalCaddie->getDataTimeStamp(), m_personalCaddie->getNumberOfSamples());
		}
		else if (m_currentMode == ModeType::MADGWICK)
		{
			m_modes[static_cast<int>(m_currentMode)]->addQuaternions(m_personalCaddie->getQuaternions(), m_personalCaddie->getNumberOfSamples(), m_personalCaddie->getCurrentTime(), 1.0f / m_personalCaddie->getMaxODR());
			m_modes[static_cast<int>(m_currentMode)]->addData(m_personalCaddie->getSensorData(), m_personalCaddie->getMaxODR(), m_personalCaddie->getDataTimeStamp(), m_personalCaddie->getNumberOfSamples());
		}
		break;
	}
	case PersonalCaddieEventType::PC_ERROR:
	{
		//Some sort of error occured on the nRF device, whether it be a TWI read error, notification set
		//up error, or something similar. All we do for now is just create an alert in red text which 
		//displays the pertinent error code.

		std::wstring alertText = *((std::wstring*)eventArgs);
		createAlert(alertText, UIColor::Red);
		break;
	}
	}

}

void ModeScreen::ModeHandler(ModeAction action, void* eventArgs)
{
	//Since the individual mode classes don't have direct access to the Personal Caddie class this handler
	//method is used to forward on actions that the current mode needs onwards to the Personal 
	//Caddie (and by extension the BLE and Sensor classes). As an example, when Discovery mode is active,
	//we need to turn on the BLE class's device watcher to start scanning for Personal Caddie devices.
	//When the scan button is clicked in the mode it forwards the request to this handler method, which 
	//then asks the BLE class to turn on the device watcher. The decision to deny direct access between
	//Personal Caddie and mode classes was deliberate. The mode screen class can almost be thought of as
	//like ther service layer in a three layerd software architecture.
	switch (action)
	{
	case PersonalCaddieChangeMode:
	{
		//We want to change the power mode of the personal caddie. The event args
		//in this case will be an instance of the PersonalCaddiePowerMode enum. Other
		//handler methods will handle getting data to the mode if and when necessary.
		
		//TODO: There's a bug in the firmware that messes up the timing between enabling
		//characteristic notifications and starting to read data. To prevent this, we need to
		//make sure that notifications are enabled BEFORE the sensor is put into sensor idle
		//mode. For now, hardcode it so that when entering idle mode from connected mode we
		//wait for notifications to be enabled first, and when entering connected mode from 
		//idle mode we disbale notifications AFTER.
		PersonalCaddiePowerMode currentMode = m_personalCaddie->getCurrentPowerMode();
		PersonalCaddiePowerMode newMode = *((PersonalCaddiePowerMode*)eventArgs);

		if (newMode == PersonalCaddiePowerMode::SENSOR_IDLE_MODE && currentMode == PersonalCaddiePowerMode::CONNECTED_MODE) m_personalCaddie->enableDataNotifications();
		else m_personalCaddie->changePowerMode(newMode);
		break;
	}
	case RendererGetTextSize:
	{
		//Some UIElements are sized so that they can perfectly fit their text inside of themselves,
		//the DropDownMenu is a good example of this. It isn't possible to know just how large the text
		//will be (in pixels) though until it actually gets rendered. This leads to a strange race scenario
		//where we need to create a UIElement with dimensions, and yet, can't know what those dimensions are
		//until after it's already been rendered. To help out with this, we can send a vector containing any
		//text that we need sized directly to the renderer. This will synchronously create the text while
		//the UIElement is being created, allowing us to take the dimensions that get placed in the UIText
		//objects and extracting them to the UIElement. --Warning-- this method can only be called from 
		//the main render loop or it can lead to unexpected errors.
		std::vector<UIText*> const& text = *((std::vector<UIText*>*)eventArgs);
		getTextRenderPixels(text);
		break;
	}
	case BLEConnection:
	{
		//There are a few actions we can take regarding the BLE class. We can see the current connection status
		//(or we connected to a device or not), we can attempt to connect to a device, and we can disconnect from 
		//the currently connected device. Wrapped inside of the eventArgs is an instance of the BLEState enum class
		//as well as a 64-bit integer. This integer represents the address of a device that we potentially want to
		//connect to.
		std::pair<BLEState, uint64_t> state = *((std::pair<BLEState, uint64_t>*)eventArgs);
		switch (state.first)
		{
		case BLEState::Connected:
		default:
			//In this case we're simply curious to see if we're currently connected to a BLEDevice
			getCurrentMode()->getBLEConnectionStatus(m_personalCaddie->bleConnectionStatus());
			break;
		case BLEState::Disconnect:
			//Disconnect from the current device
			m_personalCaddie->disconnectFromDevice();
			break;
			//
		case BLEState::Reconnect:
			m_personalCaddie->connectToDevice(state.second);
			break;
		}
		break;
	}
	case BLEDeviceWatcher:
	{
		//For the BLE Device Watcher we can more or less take the same actions as for the BLE Connection. That is,
		//we can confirm if it's currently running, we can turn it on, or we can turn it off. The device watcher status
		//is wrapped inside of the same BLEState enum class as above.
		BLEState state = *((BLEState*)eventArgs);
		switch (state)
		{		
		case BLEState::DeviceWatcherStatus:
		default:
			//In this case we're simply curious to see if the device watcher is already on
			getCurrentMode()->getBLEDeviceWatcherStatus(m_personalCaddie->bleDeviceWatcherStatus());
			break;
		case BLEState::DisableDeviceWatcher:
			//Turn off the device watcher
			m_personalCaddie->stopBLEAdvertisementWatcher();
			break;
		case BLEState::EnableDeviceWatcher:
			//Turn on the device watcher
			m_personalCaddie->startBLEAdvertisementWatcher();
			break;
		}
		break;
	}
	case BLENotifications:
	{
		//We pass in a value of 1 to enable notifications and a value of 0 to disable them.
		//Cast the evenArgs to an integer to see what we need to do here.
		if (*((int*)eventArgs)) m_personalCaddie->enableDataNotifications();
		else m_personalCaddie->disableDataNotifications();

		break;
	}
	case SensorCalibration:
	{
		//There are four main actions we can take here. We can get the current sensor calibration, set the current sensor calibration,
		//get the IMU axis calibration or set the IMU axis calibration. Since there are 3 different sensors that make up the IMU then
		//in reality there are 8 different things we can do here. One of these 8 options is wrapped inside the eventArgs. If we just
		//want to get an existing cal all we need is the action, however, if we want to update an existing cal then we need the new
		//numbers. These are also wrapped inside of the event args. A struct called CalibrationRequest is used to wrap all of this info
		//together.
		auto cal_info = *((CalibrationRequest*)eventArgs);
		switch (cal_info.action)
		{
		case SensorCalibrationAction::GET_SENSOR_CAL:
			getCurrentMode()->getSensorCalibrationNumbers(cal_info.sensor, m_personalCaddie->getSensorCalibrationNumbers(cal_info.sensor));
			break;
		case SensorCalibrationAction::SET_SENSOR_CAL:
			m_personalCaddie->updateSensorCalibrationNumbers(cal_info.sensor, cal_info.cal_numbers);
			break;
		case SensorCalibrationAction::GET_SENSOR_AXIS_CAL:
			getCurrentMode()->getSensorAxisCalibrationNumbers(cal_info.sensor, m_personalCaddie->getSensorAxisCalibrationNumbers(cal_info.sensor));
			break;
		case SensorCalibrationAction::SET_SENSOR_AXIS_CAL:
			m_personalCaddie->updateSensorAxisOrientations(cal_info.sensor, cal_info.axis_numbers);
			break;
		}
		break;
	}
	case SensorSettings:
	{
		//For the sensor settings there are two actions we can take. Either we get the current settings, or, we update the 
		//settings. The eventArgs in this case will be a pointer to an uint8_t type. If the pointer is null it means we want
		//to simply get the current settings. If the pointer isn't null though, it's pointing to an array with new settings that
		//will be used to overwrite the current ones.
		uint8_t* settings = (uint8_t*)eventArgs;
		if (settings == nullptr) ((IMUSettingsMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentSettings(m_personalCaddie->getIMUSettings(), m_personalCaddie->getAvailableSensors());
		break;
	}
	case MadgwickUpdateFilter:
	default: return;
	}
}

void ModeScreen::enterActiveState()
{
	m_modeState ^= (ModeState::Active | ModeState::CanTransfer); //toggle the active and can transfer states

	switch (m_currentMode)
	{
	//case ModeType::DEVICE_DISCOVERY:
	//{
	//	//Going into active mode while in device discovery means that we need to start the BLEAdvertisement watcher of 
	//	//the Personal Caddie. The device watcher will only alert us when a new device is found. If it has been turned
	//	//on previously and already found all devices in the area, then we won't actually get any updates. Because of this
	//	//we grab the current list and send it to the device discovery page before turning on the watcher.
	//	auto foundDevices = m_personalCaddie->getScannedDevices();
	//	std::wstring devices;
	//	for (auto it = foundDevices->begin(); it != foundDevices->end(); it++)
	//	{
	//		devices += L"Name: " + it->device_name;
	//		devices += L", Address: " + std::to_wstring(it->device_address.first) + L"\n";
	//	}
	//	//auto textBox = (FullScrollingTextBox*)(getCurrentModeUIElements()[0].get());
	//	textBox->clearText(); //clear the text each time to prevent adding duplicates
	//	textBox->addText(devices, m_renderer->getCurrentScreenSize(), true); //this new text will get resized in the main update loop

	//	m_personalCaddie->startBLEAdvertisementWatcher();
	//	break;
	//}
	//case ModeType::GRAPH_MODE:
	//{
	//	//When we first enter graph mode the personal caddie is automatically put into the sensor idle
	//	//power mode. Entering the active state switches the personal caddie power mode to sensor active
	//	//and also enables data notifications.
	//	m_modeState ^= (ModeState::PersonalCaddieSensorActiveMode | ModeState::PersonalCaddieSensorIdleMode);
	//	m_personalCaddie->enableDataNotifications();
	//	break;
	//}
	case ModeType::IMU_SETTINGS:
	{
		//When we enter the active state on the IMU Settings page, we make a request to the
		//Personal Caddie for the current IMU sensor settings. These are used to populate
		//the text for individual drop down menus

		//((IMUSettingsMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentSettings(m_renderer->getCurrentScreenSize(), m_personalCaddie->getIMUSettings(), m_personalCaddie->getAvailableSensors());
		//m_modeState |= ModeState::CanTransfer; //We still need the ability to leave the page after going active
		break;
	}
	case ModeType::CALIBRATION:
	{
		//Entering the active state puts the sensors into idle mode, as well as enables data notifications
		//m_modeState |= ModeState::PersonalCaddieSensorIdleMode;
		//m_personalCaddie->enableDataNotifications();

		//if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::SET_AXES_NUMBERS)
		//{
		//	//Before doing an axis calibration we must set the current axes to their default settings. It's easier to do this by
		//	//updating the existing variables in the sensor classes so data isn't manipulated as it comes in
		//	m_personalCaddie->updateSensorAxisOrientations({ 0, 1, 2, 1, 1, 1, 0, 1, 2, 1, 1, 1, 0, 1, 2, 1, 1, 1 });
		//}
		break;
	}
	//case ModeType::MADGWICK:
	//{
	//	//When entering Madgwick testing mode, we put the sensor directly into active mode to start taking readings. This of course
	//	//requires entering sensor idle mode first.
	//	m_modeState |= ModeState::PersonalCaddieSensorIdleMode;
	//	m_personalCaddie->enableDataNotifications();
	//	break;
	//}
	}
}

void ModeScreen::stateUpdate()
{
	switch (m_currentMode)
	{
	case ModeType::IMU_SETTINGS:
	{
		//if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & IMUSettingsState::UPDATE_SETTINGS)
		//{
		//	//New settings have been applied, send the new settings array to the personal caddie. Before
		//	//doing this, update elements 0 and 31 of the array so that they reflect the current settings
		//	//(as these aren't changed in the IMU settings menu).
		//	m_personalCaddie->updateIMUSettings(((IMUSettingsMode*)m_modes[static_cast<int>(m_currentMode)].get())->getNewSettings());
		//}
		//else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & IMUSettingsState::GET_SETTINGS)
		//{
		//	//A new sensor has been selected so we need to refresh the drop down menus. Just call the getCurrentSettings
		//	//((IMUSettingsMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentSettings(m_renderer->getCurrentScreenSize(), m_personalCaddie->getIMUSettings(), m_personalCaddie->getAvailableSensors(), true);
		//}
		break;
	}
	case ModeType::CALIBRATION:
	{
		//if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::READY_TO_RECORD)
		//{
		//	//If the ready_to_recordflag is active it means we need to put the 
		//	//Personal Caddie into sensor active mode and start recording data.
		//	m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
		//	m_modeState ^= ModeState::Active; //temporarily leave the active state to make sure we don't change the power mode multiple times
		//}
		//else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::STOP_RECORD)
		//{
		//	//When we've recorded all the data we need we can put the Personal Caddie back into sensor idle mode
		//	//m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
		//	//m_modeState ^= ModeState::Active; //temporarily leave the active state to make sure we don't change the power mode multiple times
		//}
		//else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::UPDATE_CAL_NUMBERS)
		//{
		//	//We've carried out a successful calibration so we need to update the appropriate calibration text file
		//	sensor_type_t current_sensor;
		//	if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::ACCELEROMETER) current_sensor = ACC_SENSOR;
		//	else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::GYROSCOPE) current_sensor = GYR_SENSOR;
		//	else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::MAGNETOMETER) current_sensor = MAG_SENSOR;
		//	m_personalCaddie->updateSensorCalibrationNumbers(current_sensor, ((CalibrationMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCalibrationResults());
		//}
		//else if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & CalibrationModeState::UPDATE_AXES_NUMBERS)
		//{
		//	//We've carried out a successful axis calibration so we need to update the appropriate axis calibration text files
		//	m_personalCaddie->updateSensorAxisOrientations(((CalibrationMode*)m_modes[static_cast<int>(m_currentMode)].get())->getNewAxesOrientations());
		//}
		
		break;
	}
	case ModeType::MADGWICK:
	{
		if (m_modes[static_cast<int>(m_currentMode)]->getModeState() & MadgwickModeState::BETA_UPDATE)
		{
			//The filter has converged so we change the beta value back to its standard value
			m_personalCaddie->setMadgwickBeta(0.041f);
			((MadgwickTestMode*)m_modes[static_cast<int>(m_currentMode)].get())->betaUpdate(); //let the Madwick test mode that the filter has successfully been updated
		}
		break;
	}
	}
}

void ModeScreen::leaveActiveState()
{
	m_modeState ^= (ModeState::Active | ModeState::CanTransfer); //turn off the active state

	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//Leaving active mode while in device discovery means that we need to stop the BLEAdvertisement watcher of 
		//the Personal Caddie
		m_personalCaddie->stopBLEAdvertisementWatcher();
		break;
	}
	case ModeType::GRAPH_MODE:
	{
		//Leaving the active state while in graph mode causes the personal caddie to enter the sensor idle 
		//power mode and turns off data notifications. We must leave sensor active mode before disabling
		//data notifications to prevent errors.
		m_modeState ^= (ModeState::PersonalCaddieSensorActiveMode | ModeState::PersonalCaddieSensorIdleMode); //swap the sensor idle and active states
		m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
		m_personalCaddie->disableDataNotifications();
		break;
	}
	case ModeType::CALIBRATION:
	{
		//Entering the active state puts the sensors into idle mode, as well as enables data notifications
		//m_modeState ^= ModeState::PersonalCaddieSensorIdleMode; //swap the sensor idle and active states
		//m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::CONNECTED_MODE);
		//m_personalCaddie->disableDataNotifications();
		break;
	}
	}
}

bool ModeScreen::needs3DRendering()
{
	//return true if the current mode requires 3D rendering and all 3D resources have
	//been loaded
	return m_modes[static_cast<int>(m_currentMode)]->m_needsCamera;
}