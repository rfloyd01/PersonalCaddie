#include "pch.h"
#include "mode.h"

//Initialization of static function pointer
std::function<void(ModeAction, void*)> Mode::m_mode_screen_handler = nullptr;

void Mode::uiUpdate()
{
	//The uiUpdate method is used to make updates to UI Elements that they 
	//can't make themselves. For example, if an element needs information
	//from the renderer class that will happen here. If an element has been
	//clicked and needs to interact with the current mode, that will happen here
	//as well for example

	//First see if any UIElements need information from the renderer class
	//m_uiManager.checkForTextResize();
	//if (m_uiManager.elementsCurrentlyNeedingTextUpdate() > 0)
	//{
	//	auto text = m_uiManager.getResizeText();
	//	m_mode_screen_handler(ModeAction::RendererGetTextSize, (void*)&text);

	//	//Once the text dimensions have been received we can reposition the text and resize the element as necessary
	//	m_uiManager.applyTextResizeUpdates();
	//}

	//If there are UIText objects that need their sizes updated from the renderer
	//class, add them to a new vector, send it to the rendered for updating, and
	//once that's complete have each UI Element update itself as necessary
	auto textUpdateElements = m_uiManager.getTextUpdateElements();
	if (textUpdateElements.size() > 0)
	{
		//First, get all UIText objects needing updating
		std::vector<UIText*> updateText;
		for (int i = 0; i < textUpdateElements.size(); i++)
		{
			auto elementText = textUpdateElements[i]->setTextDimension(); //get the actual UIText elements with the text needing updating
			for (int j = 0; j < elementText.size(); j++) updateText.push_back(elementText[j]);
		}

		//Then send these UIText objects to the renderer to get 
		//their sizes updated based on the current screen size
		m_mode_screen_handler(ModeAction::RendererGetTextSize, (void*)&updateText);

		//Once the text dimensions have been received the UI Manager can reposition
		//the text and resize the element as necessary
	    m_uiManager.applyTextResizeUpdates();
	}
	
	//If any clickable UI Elements have been clicked, their effects
	//on the current mode happens here.
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