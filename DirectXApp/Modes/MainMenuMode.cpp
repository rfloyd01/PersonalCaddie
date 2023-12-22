#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//set a black background color for the mode
	m_backgroundColor = UIColor::Black;
}

uint32_t MainMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	initializeTextOverlay(windowSize);

	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void MainMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
}

void MainMenuMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//The only thing we can do in main menu mode is navigate to a different mode. Simply 
	//figure out what the new mode is from the keypress and navigate there.

	ModeType newMode = ModeType::MAIN_MENU;
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		//TODO: Pressing the escape key from the main mode should quite out of 
		//the program. I should implement this at some point in the future
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	{
		newMode = ModeType::FREE;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number2:
	case winrt::Windows::System::VirtualKey::Number3:
	{
		//These modes haven't been implemented yet so for now just display
		//an alert letting the user know.
		createAlert(L"This mode hasn't been implemented yet.", UIColor::Red, m_uiManager.getScreenSize());
		break;
	}
	case winrt::Windows::System::VirtualKey::Number4:
	{
		newMode = ModeType::SETTINGS_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number5:
	{
		newMode = ModeType::DEVELOPER_TOOLS;
		break;
	}
	}

	if (newMode != ModeType::MAIN_MENU) m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
}

void MainMenuMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Personal Caddie v1.0";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Sub-Title information
	std::wstring subtitle_message = L"(Press one of the keys listed below to select a mode)";
	TextOverlay subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Subtitle Text");

	//Body information
	std::wstring body_message_1 = L"1. Free Swing Mode \n";
	std::wstring body_message_2 = L"2. Swing Analysis Mode \n";
	std::wstring body_message_3 = L"3. Training Mode \n";
	std::wstring body_message_4 = L"4. Sensor Settings \n";
	std::wstring body_message_5 = L"5. Developer Tools \n";
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		body_message_1 + body_message_2 + body_message_3 + body_message_4 + body_message_5, UIConstants::BodyTextPointSize,
		{ UIColor::FreeSwingMode, UIColor::SwingAnalysisMode, UIColor::TrainingMode, UIColor::CalibrationMode, UIColor::PaleGray },
		{ 0,  (unsigned int)body_message_1.length(),  (unsigned int)body_message_2.length(),  (unsigned int)body_message_3.length(), (unsigned int)body_message_4.length(), (unsigned int)body_message_5.length() },
		UITextJustification::UpperLeft);
	m_uiManager.addElement<TextOverlay>(body, L"Body Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to exit the program.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}