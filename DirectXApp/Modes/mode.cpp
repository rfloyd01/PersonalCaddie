#include "pch.h"
#include "mode.h"

const UIColor Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

void Mode::createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize)
{
	//When creating a new alert, instead of overriding the current one we 
	//add it onto the back. The TextOverylay class doesn't allow for updating
	//text so we must delete the current alert (if it exists) and create
	//a new one from scratch
	auto existingAlert = removeAlerts(); //This removes any existing alerts and saves the text

	if (existingAlert.getText()->message != L"")
	{
		//There was already an alert in place so add the new one on top of this one
		std::vector<UIColor> colors = existingAlert.getText()->colors;
		std::vector<unsigned long long> colorLocations = existingAlert.getText()->colorLocations;

		//if the new message is empty don't add any more colors, just copy the old colors
		if (message != L"") colors.push_back(color);
		if (message != L"") colorLocations.push_back(message.length());

		TextOverlay newAlert(windowSize, { UIConstants::AlertTextLocationX, UIConstants::AlertTextLocationY }, { UIConstants::AlertTextSizeX, UIConstants::AlertTextSizeY },
			existingAlert.getText()->message + message, UIConstants::AlertTextPointSize, colors, colorLocations, UITextJustification::UpperCenter);
		newAlert.setAlert(); //make the alert active

		m_uiElements.push_back(std::make_shared<TextOverlay>(newAlert));
	}
	else
	{
		//There were no existing alerts so we create a new one
		TextOverlay newAlert(windowSize, { UIConstants::AlertTextLocationX, UIConstants::AlertTextLocationY }, { UIConstants::AlertTextSizeX, UIConstants::AlertTextSizeY },
			message, UIConstants::AlertTextPointSize, { color }, { 0, (unsigned int)message.length() }, UITextJustification::UpperCenter);
		newAlert.setAlert(); //make the alert active

		m_uiElements.push_back(std::make_shared<TextOverlay>(newAlert));
	}
}

void Mode::createAlert(TextOverlay& alert)
{
	//creates a new alert from an existing one
	alert.setAlert(); //make the alert active
	m_uiElements.push_back(std::make_shared<TextOverlay>(alert));
}

TextOverlay Mode::removeAlerts()
{
	//Since alerts can be embedded anywhere inside the ui element vector, this method
	//searches for them, deletes the alert ui element, resizes the vector, and returns
	//any found alert for potential further processing.

	//Iterate backwards through the vector as alerts are more likely to be at the
	//back. There should only ever be one alert textOverlay at a time so after removing
	//it break off the search
	TextOverlay alert({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0, { UIColor::Black }, { 0, 0 }, UITextJustification::CenterCenter);
	for (int i = m_uiElements.size() - 1; i >= 0; i--)
	{
		if (m_uiElements[i]->isAlert())
		{
			alert = *((TextOverlay*)m_uiElements[i].get());
			m_uiElements[i] = nullptr;
			m_uiElements.erase(m_uiElements.begin() + i);
			break;
		}
	}

	return alert;
}