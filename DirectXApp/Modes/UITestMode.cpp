#include "pch.h"
#include "UITestMode.h"


UITestMode::UITestMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::LightBlue;
}

uint32_t UITestMode::initializeMode(winrt::Windows::Foundation::Size windowSize)
{
	//Create UI Elements on the page
	initializeTextOverlay(windowSize);

	//CURRENT TEST: Create a working Scroll Box
	//ScrollingTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, winrt::Windows::Foundation::Size windowSize);
	ScrollingTextBox scTB({ 0.5, 0.5 }, { 0.5, 0.33 }, L"My first scroll box!!", m_backgroundColor, windowSize);
	m_uiElements.push_back(std::make_shared<ScrollingTextBox>(scTB));

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::Idle);
}

void UITestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	//TODO: need to uninitialize UI Elements
}

void UITestMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring message = L"UI Testing";
	TextOverlay title(message, { UIColor::White }, { 0,  (unsigned int)message.length() }, UITextType::TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Sub-Title information
	std::wstring subtitle_message = L"A place to develop custom UI Elements";
	TextOverlay subtitle(subtitle_message, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextType::SUB_TITLE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

	//Footnote information
	message = L"Press Esc. to return to settings menu.";
	TextOverlay footNote(message, { UIColor::White }, { 0,  (unsigned int)message.length() }, UITextType::FOOT_NOTE, windowSize);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footNote));
}

uint32_t UITestMode::handleUIElementStateChange(int i)
{
	if (i == 1)
	{
		return 1;
	}
	return 0;
}