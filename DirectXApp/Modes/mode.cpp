#include "pch.h"
#include "mode.h"

//void Mode::initializeModeText()
//{
//	m_modeText = std::make_shared<std::vector<Text>>();
//
//	//Initialize the text vector so that there's an entry for every current TextType
//	for (int i = 0; i < static_cast<int>(TextType::END); i++)
//	{
//		Text text(L"", {}, { 0 }, static_cast<TextType>(i)); //default text
//		m_modeText->push_back(text);
//	}
//}
//
//void Mode::clearModeText()
//{
//	//set all text to empty strings, and remove any colors
//	for (int i = 0; i < static_cast<int>(TextType::END); i++)
//	{
//		Text defaultText(L"", {}, { 0 }, static_cast<TextType>(i));
//		m_modeText->at(i) = defaultText;
//	}
//}

const float* Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

//void Mode::setModeText(Text const& text)
//{
//	//sets the text for a single TextType in the m_modeText vector. To update
//	//all text for the current mode this method needs to be called multiple
//	//times.
//	m_modeText->at(static_cast<int>(text.textType)) = text;
//}

void Mode::createAlert(std::wstring message, UITextColor color, winrt::Windows::Foundation::Size windowSize)
{
	//When creating a new alert, instead of overriding the current one we 
	//add it onto the back. The TextOverylay class doesn't allow for updating
	//text so we must delete the current alert (if it exists) and create
	//a new one from scratch
	auto existingAlert = removeAlerts().getText(); //This removes any existing alerts and saves the text

	if (existingAlert.message != L"")
	{
		//There was already an alert in place so add the new one on top of this one
		std::vector<UITextColor> colors = existingAlert.colors;
		std::vector<unsigned long long> colorLocations = existingAlert.colorLocations;

		colors.push_back(color);
		colorLocations.push_back(message.length());

		TextOverlay newAlert(existingAlert.message + message, colors, colorLocations, existingAlert.startLocation, existingAlert.renderArea,
			existingAlert.fontSize, existingAlert.textType, existingAlert.justification);

		addUIElement(newAlert);
	}
	else
	{
		//There were no existing alerts so we create a new one
		TextOverlay newAlert(message, { color }, { 0, (unsigned long long)message.length() }, UITextType::ALERT, windowSize);
		addUIElement(newAlert);
	}
}

TextOverlay Mode::removeAlerts()
{
	//Since alerts can be embedded anywhere inside the ui element vector, this method
	//searches for them, deletes the alert ui element, resizes the vector, and returns
	//any found alert for potential further processing.

	//Iterate backwards through the vector as alerts are more likely to be at the
	//back. There should only ever be one alert textOverlay at a time so after removing
	//it break off the search
	TextOverlay alert(L"", {}, {}, UITextType::ALERT, { 0, 0 }); //create an empty alert
	for (int i = m_uiElements.size() - 1; i >= 0; i--)
	{
		if (m_uiElements[i]->isAlert())
		{
			alert = *((TextOverlay*)(m_uiElements[i].get()));
			m_uiElements[i] = nullptr;
			m_uiElements.erase(m_uiElements.begin() + i);
			break;
		}
	}

	return alert;
}