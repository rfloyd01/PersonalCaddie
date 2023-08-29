#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//set a black background color for the mode
	m_backgroundColor[0] = 0.0;
	m_backgroundColor[1] = 0.0;
	m_backgroundColor[2] = 0.0;
	m_backgroundColor[3] = 1.0;
}

uint32_t MainMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeMainMenuModeText();
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void MainMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	clearModeText();
}

void MainMenuMode::initializeMainMenuModeText()
{
	//Title information
	int index = static_cast<int>(TextType::TITLE);
	std::wstring titleText = L"Personal Caddie v1.0";
	m_modeText->at(index).message = titleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(titleText.size());

	//Subtitle Information
	index = static_cast<int>(TextType::SUB_TITLE);
	std::wstring subtitleText = L"(Press one of the keys listed below to select a mode)";
	m_modeText->at(index).message = subtitleText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(subtitleText.size());

	//Body Information
	index = static_cast<int>(TextType::BODY);
	std::wstring bodyText1 = L"1. Free Swing Mode \n";
	std::wstring bodyText2 = L"2. Swing Analysis Mode \n";
	std::wstring bodyText3 = L"3. Training Mode \n";
	std::wstring bodyText4 = L"4. Calibration Mode \n";
	std::wstring bodyText5 = L"5. Sensor Settings \n";
	m_modeText->at(index).message = bodyText1 + bodyText2 + bodyText3 + bodyText4 + bodyText5;
	m_modeText->at(index).colors.push_back({ 0.39, 0.592, 0.592, 1 });
	m_modeText->at(index).colors.push_back({ 0.58, 0.93, 0.588, 1 });
	m_modeText->at(index).colors.push_back({ 0.71, 0.541, 0.416, 1 });
	m_modeText->at(index).colors.push_back({ 0.498, 0.498, 0.498, 1 });
	m_modeText->at(index).colors.push_back({ 0.749, 0.749, 0.749, 1 });
	m_modeText->at(index).locations.push_back(bodyText1.size());
	m_modeText->at(index).locations.push_back(bodyText2.size());
	m_modeText->at(index).locations.push_back(bodyText3.size());
	m_modeText->at(index).locations.push_back(bodyText4.size());
	m_modeText->at(index).locations.push_back(bodyText5.size());

	//Footnote information
	index = static_cast<int>(TextType::FOOT_NOTE);
	std::wstring footnoteText = L"Press Esc. to exit the program.";
	m_modeText->at(index).message = footnoteText;
	m_modeText->at(index).colors.push_back({ 1, 1, 1, 1 });
	m_modeText->at(index).locations.push_back(footnoteText.size());
}

uint32_t MainMenuMode::handleMenuObjectClick(int i)
{
	//do nothing for now
	return ModeState::Idle;
}