#include "pch.h"
#include "mode.h"

void Mode::initializeModeText()
{
	m_modeText = std::make_shared<std::map<TextType, Text>>();

	//Initialize the text map to have an empty vector for each category
	for (int i = 0; i < static_cast<int>(TextType::END); i++) m_modeText->insert({ static_cast<TextType>(i), {L"", 0, 0, {0, 0, 0, 0} } });
}

void Mode::clearModeText()
{
	//clears all Text from the vectors of the mode's text map (with the exception of alert texts as these
	//get displayed even when travelling between different modes).
	for (auto it = m_modeText->begin(); it != m_modeText->end(); it++) it->second = { L"", 0, 0, {0, 0, 0, 0} };
}