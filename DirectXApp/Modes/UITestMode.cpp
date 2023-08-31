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
	std::wstring shorterText = L"At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident";
	std::wstring longerText = L"At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat.";

	ScrollingTextBox scTB({ 0.5, 0.5 }, { 0.5, 0.33 }, longerText, m_backgroundColor, windowSize);
	m_uiElements.push_back(std::make_shared<ScrollingTextBox>(scTB));

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::Idle | ModeState::NeedTextUpdate);
}

void UITestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
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