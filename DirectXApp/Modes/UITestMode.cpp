#include "pch.h"
#include "UITestMode.h"

UITestMode::UITestMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::LightBlue;
}

uint32_t UITestMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	initializeTextOverlay(windowSize);

	//CURRENT TEST: Create an options box
	std::wstring shorterText = L"At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident";
	std::wstring longerText = L"At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat.";
	std::wstring options = L"Option 1\nOption 2\nOption Three is Long\nOption 4\nOption 5\nOption 4b.\nOption 6\nOption 4\nOption 5\nOption 4b.\nOption 6";

	//ScrollingTextBox scTB({ 0.5, 0.45 }, { 0.25, 0.25 }, longerText, m_backgroundColor, windowSize);
	//OptionsBox optionBox({ 0.5, 0.75 }, 0.25, options, m_backgroundColor, windowSize);
	
	//m_uiElements.push_back(std::make_shared<OptionsBox>(optionBox));
	//m_uiElements.push_back(std::make_shared<ScrollingTextBox>(scTB));

	ArrowButton arrow(windowSize, { 0.5, 0.5 }, { 0.15, 0.15 }, false, true);
	FullScrollingTextBox scroll(windowSize, { 0.5, 0.5 }, { 0.5, 0.25 }, options, 0.025, true, true);
	//m_uiElementsBasic.push_back(std::make_shared<ArrowButton>(arrow));
	m_uiElementsBasic.push_back(std::make_shared<FullScrollingTextBox>(scroll));

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer | ModeState::NeedTextUpdate);
}

void UITestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	for (int i = 0; i < m_uiElementsBasic.size(); i++) m_uiElementsBasic[i] = nullptr;
	m_uiElementsBasic.clear();
}

void UITestMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"UI Testing";
	HighlightableTextOverlayBasic title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	title.updateSecondaryColor(UIColor::Red);
	m_uiElementsBasic.push_back(std::make_shared<HighlightableTextOverlayBasic>(title));

	//Sub-Title information
	std::wstring subtitle_message = L"A place to develop custom UI Elements (hover over the title to see some stuff in action!)";
	TextOverlayBasic subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiElementsBasic.push_back(std::make_shared<TextOverlayBasic>(subtitle));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlayBasic footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElementsBasic.push_back(std::make_shared<TextOverlayBasic>(footnote));
}

uint32_t UITestMode::handleUIElementStateChange(int i)
{
	if (i == 1)
	{
		return 1;
	}
	return 0;
}