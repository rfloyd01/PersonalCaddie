#include "pch.h"
#include "mode.h"

//Initialization of static function pointer
std::function<void(ModeAction, void*)> Mode::m_mode_screen_handler = nullptr;

void Mode::uiUpdate()
{
	//Since all modes contain UIElements, they all share the same method for updating them.
	//This method gets called in every single iteration of the main render loop.

	//First see if any UIElements need information from the renderer class
	m_uiManager.checkForTextResize();
	if (m_uiManager.elementsCurrentlyNeedingTextUpdate() > 0)
	{
		auto text = m_uiManager.getResizeText();
		m_mode_screen_handler(ModeAction::RendererGetTextSize, (void*)&text);

		//Once the text dimensions have been received we can reposition the text and resize the element as necessary
		m_uiManager.applyTextResizeUpdates();
	}

	//Then see if any UIElements currently need to have an action checked
	auto actionElements = m_uiManager.getActionElements();
	for (int i = actionElements.size() - 1; i >= 0; i--) //iterate backwards so we can pop each action from the back when complete
	{
		uiElementStateChangeHandler(actionElements[i]);

		//Elements must be clicked and released to be added to the actionElements array. Remove
		//the released state from elements after their actions have been complete.
		actionElements[i]->element->removeState(UIElementState::Released);
		m_uiManager.getActionElements().pop_back();
	}
}

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