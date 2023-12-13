#include "pch.h"
#include "mode.h"

//Initialization of static function pointer
std::function<void(ModeAction, void*)> Mode::m_mode_screen_handler = nullptr;

const UIColor Mode::getBackgroundColor()
{
	return m_backgroundColor;
}

void Mode::createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize, long long duration)
{
	//Creates an alert to be displayed on the screen
	Alert newAlert(windowSize, { UIConstants::AlertTextLocationX, UIConstants::AlertTextLocationY }, { UIConstants::AlertTextSizeX, UIConstants::AlertTextSizeY },
		message, UIConstants::AlertTextPointSize, { color }, { 0, (unsigned int)message.length() }, UITextJustification::UpperCenter, duration);
	m_uiManager.addElement<Alert>(newAlert, message.substr(0, 10)); //give the alert a name equal to the first 10 characters of it's message
}

std::vector<std::shared_ptr<ManagedUIElement>> Mode::removeAlerts()
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

void Mode::checkAlerts()
{
	//A check is performed to see if any active alerts have expired and need to be 
	//removed
	m_uiManager.checkAlerts();
}