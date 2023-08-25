#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//set the background color for the mode
	m_backgroundColor[0] = 0.0;
	m_backgroundColor[1] = 0.0;
	m_backgroundColor[2] = 0.0;
	m_backgroundColor[3] = 1.0;
}

void MainMenuMode::Initialize()
{
	//Create a new map for storing all of the text for this mode
	initializeModeText();
	initializeMainMenuModeText();
	
}

void MainMenuMode::processInput(InputState* inputState)
{
	if (inputState->currentPressedKey != KeyboardKeys::DeadKey)
	{
		//we have a key that needs processing
		OutputDebugString(L"Processing input in the MainMenuMode class!\n");
	}
}

void MainMenuMode::initializeMainMenuModeText()
{
	//Title information
	std::wstring titleText = L"Personal Caddie v1.0";
	m_modeText->at(TextType::TITLE) = titleText;
	m_modeTextColors->at(TextType::TITLE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::TITLE).locations.push_back(titleText.size());

	//Subtitle Information
	std::wstring subtitleText = L"(Press one of the keys listed below to select a mode)";
	m_modeText->at(TextType::SUB_TITLE) = subtitleText;
	m_modeTextColors->at(TextType::SUB_TITLE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::SUB_TITLE).locations.push_back(subtitleText.size());

	//Body Information
	std::wstring bodyText1 = L"1. Free Swing Mode \n";
	std::wstring bodyText2 = L"2. Swing Analysis Mode \n";
	std::wstring bodyText3 = L"3. Training Mode \n";
	std::wstring bodyText4 = L"4. Calibration Mode \n";
	std::wstring bodyText5 = L"5. Sensor Settings \n";
	m_modeText->at(TextType::BODY) = bodyText1 + bodyText2 + bodyText3 + bodyText4 + bodyText5;
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.39, 0.592, 0.592, 1 });
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.58, 0.93, 0.588, 1 });
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.71, 0.541, 0.416, 1 });
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.498, 0.498, 0.498, 1 });
	m_modeTextColors->at(TextType::BODY).colors.push_back({ 0.749, 0.749, 0.749, 1 });

	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText1.size());
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText2.size());
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText3.size());
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText4.size());
	m_modeTextColors->at(TextType::BODY).locations.push_back(bodyText5.size());

	//Footnote information
	std::wstring footnoteText = L"Press Esc. to exit the program.";
	m_modeText->at(TextType::FOOT_NOTE) = footnoteText;
	m_modeTextColors->at(TextType::FOOT_NOTE).colors.push_back({ 1, 1, 1, 1 });
	m_modeTextColors->at(TextType::FOOT_NOTE).locations.push_back(footnoteText.size());
}