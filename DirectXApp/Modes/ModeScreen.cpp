#include "pch.h"

#include "ModeScreen.h"

ModeScreen::ModeScreen()
{

}

void ModeScreen::Initialize(
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_inputProcessor = input;
	m_renderer = renderer;
}