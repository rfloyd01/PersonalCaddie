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
	TextColor red{ 1, 0, 0, 1 };
	TextColor blue{ 0, 0, 1, 1 };

	m_modeText->at(TextType::TITLE) = L"Personal Caddie v1.0";
	m_modeText->at(TextType::SUB_TITLE) = L"(Press one of the keys listed below to select a mode)";
	m_modeText->at(TextType::FOOT_NOTE) = L"Press Esc. to exit the program.";

	std::vector<TextTypeColorSplit> textColors;
	TextTypeColorSplit yo = { {}, {} };
	textColors.push_back({ {}, {} });
	textColors.push_back({ {{1, 1, 1, 1}}, {(int)m_modeText->at(TextType::TITLE).size()} }); //The Title is solid white
	textColors.push_back({ {{1, 1, 1, 1}}, {(int)m_modeText->at(TextType::SUB_TITLE).size()} }); //The SubTitle is solid white
}