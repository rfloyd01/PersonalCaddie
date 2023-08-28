#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMenuMode.h"
#include "DeviceDiscoveryMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU),
	alert_active(false),
	personal_caddy_event(PersonalCaddieEventType::NONE)
{
	//Create instances for all different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS_MENU)] = std::make_shared<SettingsMenuMode>();
	m_modes[static_cast<int>(ModeType::DEVICE_DISCOVERY)] = std::make_shared<DeviceDiscoveryMode>();

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
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode();
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

	processMouseInput(inputUpdate->mousePosition, inputUpdate->mouseClick);

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
				//TODO: disconnect from the Personal Caddie of connected and release
				//any resources
			}
			else if (m_currentMode == ModeType::SETTINGS_MENU)
			{
				changeCurrentMode(ModeType::MAIN_MENU);
			}
			else if (m_currentMode == ModeType::DEVICE_DISCOVERY)
			{
				changeCurrentMode(ModeType::SETTINGS_MENU);
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
				createModeScreenAlert(L"This mode hasn't been implemented yet.");
			}
			else if (m_currentMode == ModeType::SETTINGS_MENU)
			{
				changeCurrentMode(ModeType::DEVICE_DISCOVERY);
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
	}
}

void ModeScreen::processMouseInput(DirectX::XMFLOAT2 mousePosition, bool mouseClick)
{
	//need to poll all of the 2D elements in the current mode to see if the mouse is
	//over any of them
	for (int i = 0; i < m_modes[static_cast<int>(m_currentMode)]->getMenuObjects().size(); i++)
	{
		MenuObjectState objectState = m_modes[static_cast<int>(m_currentMode)]->getMenuObjects()[i]->update(mousePosition, mouseClick, m_renderer->getCurrentScreenSize());
		switch (objectState)
		{
		case MenuObjectState::Pressed:
		{
			m_modes[static_cast<int>(m_currentMode)]->handleMenuObjectClick(i);
			m_renderer->updateMenuObjects(m_modes[static_cast<int>(m_currentMode)]->getMenuObjects());

			//start a short timer that will change the color of the button from PRESSED to NOT_PRESSED
			button_pressed = true;
			button_pressed_timer = std::chrono::steady_clock::now();
			break;
		}
		}
		if (mouseClick) m_inputProcessor->setMouseState(MouseState::ButtonProcessed); //let the input processor know that the click has been handled
	}
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
		auto alerts = getCurrentModeText()->at(static_cast<int>(TextType::ALERT)); //Get the current alerts
		m_renderer->editText(alerts);

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
			//the timer has gone off so turn the timer off
			alert_active = false;

			//then remove the current alerts both from the mode text map, as well
			//as from being rendered on the screen
			setCurrentModeText({ L"", {}, {0}, TextType::ALERT });
			m_renderer->editText({ L"", {}, {0}, TextType::ALERT });
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
			for (int i = 0; i < m_modes[static_cast<int>(m_currentMode)]->getMenuObjects().size(); i++)
			{
				if (m_modes[static_cast<int>(m_currentMode)]->getMenuObjects()[i]->getReleventState() == MenuObjectState::Pressed)
				{
					m_modes[static_cast<int>(m_currentMode)]->getMenuObjects()[i]->setReleventState(MenuObjectState::NotPressed);
				}
			}

			//rerender all menu objects
			m_renderer->updateMenuObjects(m_modes[static_cast<int>(m_currentMode)]->getMenuObjects());
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
	auto currentAlerts = getCurrentModeText()->at(static_cast<int>(TextType::ALERT));
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode();

	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	if (currentAlerts.message != L"") setCurrentModeText(currentAlerts);
	m_renderer->CreateModeResources();
	
}

std::shared_ptr<std::vector<Text> > ModeScreen::getCurrentModeText()
{
	//returns a reference to any text that needs to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getModeText();
}

std::vector<std::shared_ptr<MenuObject> > const& ModeScreen::getCurrentModeMenuObjects()
{
	//returns a reference to all UI elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getMenuObjects();
}

const float* ModeScreen::getBackgroundColor()
{
	//returns the background color of the current mode
	return m_modes[static_cast<int>(m_currentMode)]->getBackgroundColor();
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
	case PersonalCaddieEventType::BLE_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		Text text(alertText, { AlertColors::alertLightBlue }, { 0, alertText.size() }, TextType::ALERT);
		addCurrentModeText(text); //add the alert on top of any existing ones
		break;
	}
	case PersonalCaddieEventType::PC_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		Text text(alertText, { AlertColors::alertYellow }, { 0, alertText.size() }, TextType::ALERT);
		addCurrentModeText(text); //add the alert on top of any existing ones
		break;
	}
	}

}

void ModeScreen::addCurrentModeText(Text const& text)
{
	//This method adds new text on top of existing text without deleting anything
	auto currentModeText = m_modes[static_cast<int>(m_currentMode)]->getModeText();

	//Add the new text
	int index = static_cast<int>(text.textType);
	currentModeText->at(index).message += text.message;

	//Then add the new colors and color rendering points
	for (int i = 0; i < text.colors.size(); i++)
	{
		currentModeText->at(index).colors.push_back(text.colors[i]);
		currentModeText->at(index).locations.push_back(text.locations[i + 1]);
		//*note - the locations vector is always 1 larger than the colors vector so the i + 1
		//in the above line is safe and intended
	}
}

void ModeScreen::setCurrentModeText(Text const& text)
{
	//This method overwrites the current mode text with the text given
	auto currentModeText = m_modes[static_cast<int>(m_currentMode)]->getModeText();
	currentModeText->at(static_cast<int>(text.textType)) = text;
}

void ModeScreen::createModeScreenAlert(std::wstring alert)
{
	//this method is for creating alerts originating from this class. This method
	//only gets invoked on the main rendering thread so there's no issues with 
	//making direct calls to the master renderer. These alerts are red colored.
	Text newAlert(alert, { AlertColors::alertRed }, { 0, alert.size() }, TextType::ALERT);
	addCurrentModeText(newAlert);
	m_renderer->editText(getCurrentModeText()->at(static_cast<int>(TextType::ALERT)));

	//Make sure to set/reset the alert timer
	alert_timer = std::chrono::steady_clock::now(); //set/reset the alert timer
	alert_active = true;
}