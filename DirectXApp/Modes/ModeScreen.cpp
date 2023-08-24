#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU)
{
	//Create instances for all different modes.
	m_modes.push_back(std::make_shared<MainMenuMode>());
}

void ModeScreen::Initialize(
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_inputProcessor = input;
	m_renderer = renderer;

	//Load the main mode
	m_modes[static_cast<int>(m_currentMode)]->Initialize();
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