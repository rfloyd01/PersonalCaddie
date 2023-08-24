#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//leave blank for now
}

void MainMenuMode::Initialize()
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeMainMenuModeText();
	
}

void MainMenuMode::initializeMainMenuModeText()
{
	//For now just load up all text to be rendered
	TextColor red{ 1, 0, 0, 0.5 };
	TextColor blue{ 0, 0, 1, 1 };
	TextColor white{ 1, 1, 1, 1 };

	TextType tt = TextType::SUB_TITLE;
	Text t{ L"This is a real test", 15, 50, white };

	m_modeText->at(tt) = t;
}