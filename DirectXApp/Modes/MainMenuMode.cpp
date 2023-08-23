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

	//For now just load up all text to be rendered
	TextColor red{ 1, 0, 0, 0.5 };

	TextType tt = TextType::TITLE;
	Text t{ L"This is a real test", 15, 50, red };

	m_modeText->at(tt).push_back(t);
}