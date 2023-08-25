#include "pch.h"
#include "mode.h"

void Mode::initializeModeText()
{
	m_modeText = std::make_shared<std::map<TextType, std::wstring>>();
	m_modeTextColors = std::make_shared<std::map<TextType, TextTypeColorSplit>>();

	//Initialize the text map to have an empty vector for each category
	for (int i = 0; i < static_cast<int>(TextType::END); i++)
	{
		m_modeText->insert({ static_cast<TextType>(i), L""}); //text is null initialized with no color
		m_modeTextColors->insert({ static_cast<TextType>(i), { {}, {0} } }); //color split starts out empty

		//Note, the locations portion of the TextTypeColorSplit will always start with a 0, 
		//regardless of whether or not there's actually any text. Because of this the locations
		//vector will always have a length that's 1 greater than the colors vector.
	}
}

void Mode::clearModeText()
{
	//clears all Text and colors from the mode's text maps.
	for (int i = 0; i < static_cast<int>(TextType::END); i++)
	{
		TextType tt = static_cast<TextType>(i);

		m_modeText->at(tt) = L"";
		m_modeTextColors->at(tt) = { {}, {0} };
	}
}

const float* Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

std::pair<std::wstring, TextTypeColorSplit> Mode::getCurrentAlerts()
{
	return {m_modeText->at(TextType::ALERT), m_modeTextColors->at(TextType::ALERT)};
}

void Mode::setCurrentAlerts(std::pair<std::wstring, TextTypeColorSplit> alert)
{
	m_modeText->at(TextType::ALERT) = alert.first;
	m_modeTextColors->at(TextType::ALERT) = alert.second;
}

void Mode::removeCurrentAlerts()
{
	m_modeText->at(TextType::ALERT) = L"";
	m_modeTextColors->at(TextType::ALERT) = { {}, {0} };
}