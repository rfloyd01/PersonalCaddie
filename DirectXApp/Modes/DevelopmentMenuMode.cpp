#include "pch.h"
#include "DevelopmentMenuMode.h"

DevelopmentMenuMode::DevelopmentMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Gray;
}

uint32_t DevelopmentMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Initialize all overlay text
	initializeTextOverlay(windowSize);
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void DevelopmentMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void DevelopmentMenuMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Developer Tools";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Sub-Title information
	std::wstring subtitle_message = L"Modes to aid in the development of the Personal Caddie";
	TextOverlay subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

	//Body information
	std::wstring body_message_1 = L"1. UI Element Testing \n";
	std::wstring body_message_2 = L"2. Graph IMU Data \n";
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		body_message_1 + body_message_2, UIConstants::BodyTextPointSize, { UIColor::PaleGray, UIColor::DarkGray }, { 0, (unsigned int)body_message_1.length(), (unsigned int)body_message_2.length() }, UITextJustification::UpperLeft);
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

uint32_t DevelopmentMenuMode::handleUIElementStateChange(int i)
{
	return 0;
}