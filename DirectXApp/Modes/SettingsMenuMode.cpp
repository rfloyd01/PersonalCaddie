#include "pch.h"
#include "SettingsMenuMode.h"

SettingsMenuMode::SettingsMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Gray;
}

uint32_t SettingsMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Initialize all overlay text
	initializeTextOverlay(windowSize);
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void SettingsMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map

	//TODO: delete all UI Elements when leaving mode
}

void SettingsMenuMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Sensor Settings";
	TextOverlay title(title_message, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextType::TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Sub-Title information
	std::wstring subtitle_message = L"(Press one of the keys listed below to select a mode)";
	TextOverlay subtitle(subtitle_message, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextType::SUB_TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

	//Body information
	std::wstring body_message_1 = L"1. Connect/Disconnect from Personal Caddie \n";
	std::wstring body_message_2 = L"2. IMU Settings \n";
	TextOverlay body(body_message_1 + body_message_2, { UIColor::PaleGray, UIColor::DarkGray },
		{ 0,  (unsigned int)body_message_1.length(),  (unsigned int)body_message_2.length() }, UITextType::BODY, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu.";
	TextOverlay footNote(footnote_message, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextType::FOOT_NOTE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footNote));
}

uint32_t SettingsMenuMode::handleUIElementStateChange(int i)
{
	return ModeState::Idle;
}