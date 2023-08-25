#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU)
{
	//Create instances for all different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS)] = std::make_shared<SettingsMode>();
}

void ModeScreen::Initialize(
	_In_ std::shared_ptr<PersonalCaddie> const& pc,
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_personalCaddie = pc;
	m_inputProcessor = input;
	m_renderer = renderer;

	//Load the main mode (which is set in the constructor) and update the current modeState
	initializeCurrentMode();
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
				OutputDebugString(L"Going back to main menu.\n");
				//TODO: deallocate whatever resources necessary and initialize the main menu mode
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
				OutputDebugString(L"Going to sensor settings page.\n");

				uninitializeCurrentMode();

				m_currentMode = ModeType::SETTINGS;
				initializeCurrentMode();
			}
		}
		break;
	}
	
}

void ModeScreen::initializeCurrentMode()
{
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode();
	m_renderer->CreateModeResources();
}

void ModeScreen::uninitializeCurrentMode()
{
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();
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