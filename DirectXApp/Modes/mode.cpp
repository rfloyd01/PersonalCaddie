#include "pch.h"
#include "mode.h"

void Mode::initializeModeText()
{
	m_modeText = std::make_shared<std::vector<Text>>();

	//Initialize the text vector so that there's an entry for every current TextType
	for (int i = 0; i < static_cast<int>(TextType::END); i++)
	{
		Text text(L"", {}, { 0 }, static_cast<TextType>(i)); //default text
		m_modeText->push_back(text);
	}
}

void Mode::clearModeText()
{
	//set all text to empty strings, and remove any colors
	for (int i = 0; i < static_cast<int>(TextType::END); i++)
	{
		Text defaultText(L"", {}, { 0 }, static_cast<TextType>(i));
		m_modeText->at(i) = defaultText;
	}
}

const float* Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

void Mode::setModeText(Text const& text)
{
	//sets the text for a single TextType in the m_modeText vector. To update
	//all text for the current mode this method needs to be called multiple
	//times.
	m_modeText->at(static_cast<int>(text.textType)) = text;
}