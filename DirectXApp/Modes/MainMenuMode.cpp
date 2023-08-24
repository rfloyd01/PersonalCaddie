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

	TextType tt = TextType::FOOT_NOTE;
	Text t{ L"This is a real test", 15, 50, white };

	tt = TextType::TITLE;

	m_modeText->at(TextType::TITLE) = { L"Personal Caddie v1.0", 15, 50, white };
	m_modeText->at(TextType::SUB_TITLE) = { L"(Press one of the keys listed below to select a mode)", 15, 50, white };
	m_modeText->at(TextType::FOOT_NOTE) = { L"Press Esc. to exit the program.", 15, 50, white };
}