#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMenuMode.h"
#include "DeviceDiscoveryMode.h"
#include "UITestMode.h"

#include "Graphics/Rendering/MasterRenderer.h"
#include "Graphics/Objects/2D/TextBoxes/StaticTextBox.h"
#include "Graphics/Objects/2D/UIButton.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU),
	alert_active(false),
	button_pressed(false),
	personal_caddy_event(PersonalCaddieEventType::NONE)
{
	//Create instances for all different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS_MENU)] = std::make_shared<SettingsMenuMode>();
	m_modes[static_cast<int>(ModeType::DEVICE_DISCOVERY)] = std::make_shared<DeviceDiscoveryMode>();
	m_modes[static_cast<int>(ModeType::UI_TEST_MODE)] = std::make_shared<UITestMode>();
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

	//See if any of the above input, timers or events have caused the need to update
	//any UI elements
	if (m_modeState & ModeState::NeedTextUpdate)
	{
		//some text in a ui element was changed so we need to recalculate the render height
		//for the text (hits happens in elements like scroll boxes where the height can
		//only be calculated in the renderer classes).
		getTextRenderPixels(getCurrentModeUIElements());
		m_modeState ^= ModeState::NeedTextUpdate; //remove the text update flag
	}

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
			else if (m_currentMode == ModeType::SETTINGS_MENU || m_currentMode == ModeType::UI_TEST_MODE)
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
				createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
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
	case winrt::Windows::System::VirtualKey::Number6:
		if (m_modeState & ModeState::CanTransfer)
		{
			if (m_currentMode == ModeType::MAIN_MENU)
			{
				//If we're on the Main Menu screen then pressing the 5 key will take us to the
				//sensor settings page
				changeCurrentMode(ModeType::UI_TEST_MODE);
			}
		}
		break;
	}
}

void ModeScreen::processMouseInput(InputState* inputState)
{
	//need to poll all of the 2D elements in the current mode to see if the mouse is
	//over any of them
	/*for (int i = 0; i < m_modes[static_cast<int>(m_currentMode)]->getUIElements().size(); i++)
	{
		UIElementState objectState = m_modes[static_cast<int>(m_currentMode)]->getUIElements()[i]->update(inputState);
		switch (objectState)
		{
		case UIElementState::Clicked:
		{
			//The current UI Element has been clicked
			uint32_t new_state = m_modes[static_cast<int>(m_currentMode)]->handleUIElementStateChange(i);

			//start a short timer that will change the color of the button from PRESSED to NOT_PRESSED
			button_pressed = true;
			button_pressed_timer = std::chrono::steady_clock::now();

			//See if the button press forced into or out of active mode
			if (m_modeState & ModeState::Active)
			{
				if (new_state & ModeState::Idle) leaveActiveState();
			}
			else if (m_modeState & ModeState::Idle)
			{
				if (new_state & ModeState::Active) enterActiveState();
			}

			break;
		}
		}
	}*/

	auto uiElements = m_modes[static_cast<int>(m_currentMode)]->getUIElementsBasic();
	for (int i = 0; i < uiElements.size(); i++)
	{
		uint32_t uiElementState = uiElements[i]->update(inputState);

		if (uiElementState & UIElementStateBasic::NeedTextPixels)
		{
			getTextRenderPixelsBasic(uiElements[i]->setTextDimension()); //get the necessary pixels
			uiElements[i]->repositionText(); //see if any text needs to be repositioned after getting new dimensions
			uiElements[i]->resize(m_renderer->getCurrentScreenSize()); //and then resize the ui element
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
				if (m_modes[static_cast<int>(m_currentMode)]->getUIElements()[i]->getState() == UIElementState::Clicked)
				{
					//The ui button class overrides the ui element setState() method so case to a UI button
					//before setting the state to idle
					((UIButton*)m_modes[static_cast<int>(m_currentMode)]->getUIElements()[i].get())->setState(UIElementState::Idle);
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
	auto currentAlerts = m_modes[static_cast<int>(m_currentMode)]->removeAlerts().getText();
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize());
	
	//See if any UI elements in the mode need to know their full render height (needed for items
	//like scroll boxes).
	//getTextRenderHeights(getCurrentModeUIElements());

	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	if (currentAlerts.message != L"") createAlert(currentAlerts.message, currentAlerts.colors[0]); //TODO: this will make all alerts the same color, come back to this

	if (m_modeState & ModeState::NeedTextUpdate)
	{
		//See if the new mode has any complex UI text elements that need help from the master renderer to initialize.
		getTextRenderPixels(getCurrentModeUIElements());
		m_modeState ^= ModeState::NeedTextUpdate; //remove the text update flag
	}
}

void ModeScreen::getTextRenderPixels(std::vector<std::shared_ptr<UIElement>> const& uiElements)
{
	//iterate through all of the UI elements in the mode to see if any of them need the
	//height in pixels for text layouts (this will happen for elements like scroll boxes).
	for (int i = 0; i < uiElements.size(); i++)
	{
		if (uiElements[i]->needTextRenderHeight())
		{
			//This UI Element needs the height of a text layout. Get the text layout height for it,
			//and any of its children. Of all the UI Element components, only elementText has this feature.
			int elementTextComponents = uiElements[i]->getRenderVectorSize(RenderOrder::ElementText);
			for (int j = 0; j < elementTextComponents; j++)
			{
				UIText* renderText = (UIText*)uiElements[i]->getRenderItem(RenderOrder::ElementText, j);
				if (renderText->needDPI) m_renderer->setTextLayoutPixels(renderText);
			}

			int overlayTextComponents = uiElements[i]->getRenderVectorSize(RenderOrder::TextOverlay);
			for (int j = 0; j < overlayTextComponents; j++)
			{
				UIText* renderText = (UIText*)uiElements[i]->getRenderItem(RenderOrder::TextOverlay, j);
				if (renderText->needDPI) m_renderer->setTextLayoutPixels(renderText);
			}

			//Check any children UIElements as well, this is done recursively
			getTextRenderPixels(uiElements[i]->getChildrenUIElements());
		}
	}
}

void ModeScreen::getTextRenderPixelsBasic(UIText* text)
{
	//Sets the size for the text overlay render box of the given text element
	m_renderer->setTextLayoutPixels(text);
}

std::vector<std::shared_ptr<UIElement> > const& ModeScreen::getCurrentModeUIElements()
{
	//returns a reference to all UI elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getUIElements();
}

std::vector<std::shared_ptr<UIElementBasic> > const& ModeScreen::getCurrentModeUIElementsBasic()
{
	//returns a reference to all UI elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getUIElementsBasic();
}

void ModeScreen::resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize)
{
	auto uiElements = getCurrentModeUIElements();
	for (int i = 0; i < uiElements.size(); i++) uiElements[i]->resize(windowSize);
	getTextRenderPixels(uiElements);
}

void ModeScreen::resizeCurrentModeUIElementsBasic(winrt::Windows::Foundation::Size windowSize)
{
	auto uiElements = getCurrentModeUIElementsBasic();
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
			devices += L", Address: " + it->device_address.second + L"\n";
		}

		//Update the text in the device discovery text box
		if (m_currentMode == ModeType::DEVICE_DISCOVERY)
		{
			auto textBox = (ScrollingTextBox*)(getCurrentModeUIElements()[0].get());
			m_modeState |= textBox->addText(devices); //this new text will get resized in the main update loop
		}
		break;
	}
	}

}

void ModeScreen::enterActiveState()
{
	m_modeState ^= (ModeState::Active | ModeState::Idle); //swap the active and idle states

	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//Going into active mode while in device discovery means that we need to start the BLEAdvertisement watcher of 
		//the Personal Caddie
		m_personalCaddie->startBLEAdvertisementWatcher();
	}
	}
}

void ModeScreen::leaveActiveState()
{
	m_modeState ^= (ModeState::Active | ModeState::Idle); //swap the active and idle states

	switch (m_currentMode)
	{
	case ModeType::DEVICE_DISCOVERY:
	{
		//Leaving active mode while in device discovery means that we need to stop the BLEAdvertisement watcher of 
		//the Personal Caddie
		m_personalCaddie->stopBLEAdvertisementWatcher();
	}
	}
}