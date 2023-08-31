#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//set a black background color for the mode
	m_backgroundColor = UIColor::Black;
}

uint32_t MainMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	initializeTextOverlay(windowSize);

	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void MainMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	//TODO: get rid of UI Elements before leaving mode
}

void MainMenuMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Personal Caddie v1.0";
	TextOverlay title(title_message, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextType::TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Sub-Title information
	std::wstring subtitle_message = L"(Press one of the keys listed below to select a mode)";
	TextOverlay subtitle(subtitle_message, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextType::SUB_TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

	//Body information
	std::wstring body_message_1 = L"1. Free Swing Mode \n";
	std::wstring body_message_2 = L"2. Swing Analysis Mode \n";
	std::wstring body_message_3 = L"3. Training Mode \n";
	std::wstring body_message_4 = L"4. Calibration Mode \n";
	std::wstring body_message_5 = L"5. Sensor Settings \n";
	std::wstring body_message_6 = L"6. UI Element Testing \n";
	
	TextOverlay body(body_message_1 + body_message_2 + body_message_3 + body_message_4 + body_message_5 + body_message_6,
		{ UIColor::FreeSwingMode, UIColor::SwingAnalysisMode, UIColor::TrainingMode, UIColor::CalibrationMode, UIColor::PaleGray, UIColor::White },
		{ 0,  (unsigned int)body_message_1.length(),  (unsigned int)body_message_2.length(),  (unsigned int)body_message_3.length(), (unsigned int)body_message_4.length(), (unsigned int)body_message_5.length(), (unsigned int)body_message_6.length() },
		UITextType::BODY, windowSize);

	m_uiElements.push_back(std::make_shared<TextOverlay>(body));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to exit the program.";
	TextOverlay footNote(footnote_message, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextType::FOOT_NOTE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footNote));
}

uint32_t MainMenuMode::handleUIElementStateChange(int i)
{
	return ModeState::Idle;
}