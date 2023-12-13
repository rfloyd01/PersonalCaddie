#include "pch.h"
#include "mode.h"

const UIColor Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

void Mode::createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize, long long duration)
{
	//Creates an alert to be displayed on the screen
	Alert newAlert(windowSize, { UIConstants::AlertTextLocationX, UIConstants::AlertTextLocationY }, { UIConstants::AlertTextSizeX, UIConstants::AlertTextSizeY },
		message, UIConstants::AlertTextPointSize, { color }, { 0, (unsigned int)message.length() }, UITextJustification::UpperCenter, duration);
	m_uiManager.addElement<Alert>(newAlert, L"");
}

//void Mode::createAlert(TextOverlay& alert)
//{
//	//creates a new alert from an existing one
//	m_uiManager.createAlert(alert);
//}

std::vector<std::shared_ptr<ManagedUIElement>> const& Mode::removeAlerts()
{
	//Creates a copy of the alerts in the current mode before they get 
	//delete. This is used to persist alerts through different modes.
	return m_uiManager.removeAlerts();
}

void Mode::overwriteAlerts(std::vector<std::shared_ptr<ManagedUIElement>> const& alerts)
{
	//Overwrites the alert vector of the current mode with the given one. This is useful
	//when transferring alerts between different modes.
	m_uiManager.overwriteAlerts(alerts);
}

//TextOverlay Mode::removeAlerts()
//{
//	//Since alerts can be embedded anywhere inside the ui element vector, this method
//	//searches for them, deletes the alert ui element, resizes the vector, and returns
//	//any found alert for potential further processing.
//
//	//Iterate backwards through the vector as alerts are more likely to be at the
//	//back. There should only ever be one alert textOverlay at a time so after removing
//	//it break off the search
//	TextOverlay alert({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0, { UIColor::Black }, { 0, 0 }, UITextJustification::CenterCenter);
//	for (int i = m_uiElements.size() - 1; i >= 0; i--)
//	{
//		if (m_uiElements[i]->isAlert())
//		{
//			alert = *((TextOverlay*)m_uiElements[i].get());
//			m_uiElements[i] = nullptr;
//			m_uiElements.erase(m_uiElements.begin() + i);
//			break;
//		}
//	}
//
//	return alert;
//}