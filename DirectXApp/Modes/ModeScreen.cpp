#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU),
	alert_active(false)
{
	//Create instances for all different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS)] = std::make_shared<SettingsMode>();

	//Set default time for alerts to be 5 seconds
	alert_timer_duration = 5000;
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
	if (inputUpdate != nullptr)
	{
		if (inputUpdate->currentPressedKey != KeyboardKeys::DeadKey) processKeyboardInput(inputUpdate->currentPressedKey);
	}

	//after processing input, see if there are any timers that are going on or expired
	processTimers();
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
			else
			{
				changeCurrentMode(ModeType::MAIN_MENU);
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
				changeCurrentMode(ModeType::SETTINGS);
			}
		}
		break;
	}
	
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
			m_modes[static_cast<int>(m_currentMode)]->removeCurrentAlerts();
			m_renderer->removeCurrentAlerts();
		}
	}

	//TODO: add other timers here after creating them
}

void ModeScreen::changeCurrentMode(ModeType mt)
{
	//This method unitializes the current mode and initializes the mode selected.
	//Any active alerts will get copied into the text map for the new mode

	if (mt == m_currentMode) return; //no need to change anything, already on this mode

	//First, get any active alerts before deleting the text and color map of the 
	//current mode
	auto currentAlerts = getCurrentModeAlerts();
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode();

	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	if (currentAlerts.first != L"") setCurrentModeAlerts(currentAlerts);
	m_renderer->CreateModeResources();
	
}

std::pair<std::wstring, TextTypeColorSplit> ModeScreen::getCurrentModeAlerts()
{
	return m_modes[static_cast<int>(m_currentMode)]->getCurrentAlerts();
}
void ModeScreen::setCurrentModeAlerts(std::pair<std::wstring, TextTypeColorSplit> alerts)
{
	m_modes[static_cast<int>(m_currentMode)]->setCurrentAlerts(alerts);
	if (!alert_active)
	{
		//this is a brand new alert so we need to set the alert timer
		alert_active = true;
		alert_timer = std::chrono::steady_clock::now();

		//we also need to tell the renderer to draw this alert
		m_renderer->renderNewAlerts(alerts);
	}
}

std::shared_ptr<std::map<TextType, std::wstring> > ModeScreen::getRenderText()
{
	//returns a reference to any text that needs to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getModeText();
}

std::shared_ptr<std::map<TextType, TextTypeColorSplit> > ModeScreen::getRenderTextColors()
{
	//returns a reference to any text colors that need to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getModeTextColors();
}

const float* ModeScreen::getBackgroundColor()
{
	//returns the background color of the current mode
	return m_modes[static_cast<int>(m_currentMode)]->getBackgroundColor();
}

void ModeScreen::PersonalCaddieAlertHandler(std::pair<std::wstring, TextTypeColorSplit> alert)
{
	//This method will automaticall get alerts from the Personal Caddie device. Messages
	//related to the BLE device are in light blue, messages related to the IMU are in green
	//and messages about the Personal Caddie itself are in yellow
	setCurrentModeAlerts(alert);
}