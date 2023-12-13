#include "pch.h"
#include "UIElementManager.h"
#include "algorithm"

#include <chrono>

UIElementManager::UIElementManager()
{
	//create the map with an empty entry for each type of UI Element
	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		m_uiElements.insert({ static_cast<UIElementType>(i), {} });
	}

	//Create the location grid
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		std::vector<std::vector<std::shared_ptr<ManagedUIElement> > > vec;
		for (int j = 0; j < GRID_WIDTH; j++) vec.push_back({});

		m_gridLocations.push_back(vec);
	}
}

void UIElementManager::removeElementType(UIElementType type)
{
	//Removes all UI Elements with the given type
	for (auto it = m_uiElements.at(type).begin(); it != m_uiElements.at(type).end(); it++)
	{
		//Remove the UIElement from the render vector first.
		auto render_it = std::find(m_renderElements.begin(), m_renderElements.end(), it->get()->element);
		m_renderElements.erase(render_it);

		for (int i = 0; i < it->get()->grid_locations.size(); i++)
		{
			//The grid vectors aren't ordered so we need to search for the appropriate
			//smart pointer. This may not seem super effecient, especially because removing
			//elements from a vector forces all other elements to shift, however, if
			//the grid is small enough then there should never be more than a handful of 
			//pointers in each vector so this should be alright.
			std::pair<int, int> grid_location = { it->get()->grid_locations[i].first, it->get()->grid_locations[i].second };
			auto grid_it = findElementByName(m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second], it->get()->name);
			m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second].erase(grid_it);
		}
	}
	
	m_uiElements.at(type).clear(); //finally clear out the appropriate vector in the ui element map
}

void UIElementManager::removeAllElements()
{
	//Clear out the render and action vectors, the element map and the element grid
	m_renderElements.clear();
	m_actionElements.clear();

	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		m_uiElements.at(static_cast<UIElementType>(i)).clear();
	}

	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_WIDTH; j++) m_gridLocations[i][j].clear();
	}
}

void UIElementManager::updateGridSquareElements(InputState* input)
{
	//If the mouse moves, clicks or scrolls we inspect all of the UIElements that are in the 
	//same grid square as the mouse and apply these updates. Mouse coordinates come in as 
	//absolute pixels and not relative ones.
	std::pair<int, int> mouseGridSquare = { GRID_WIDTH * (input->mousePosition.y / m_windowSize.Height), GRID_WIDTH * (input->mousePosition.x / m_windowSize.Width) };

	//Make sure the mouse calculation puts it inside of the screen
	if (mouseGridSquare.first < 0) mouseGridSquare.first = 0;
	else if (mouseGridSquare.first >= GRID_WIDTH) mouseGridSquare.first = GRID_WIDTH - 1;

	if (mouseGridSquare.second < 0) mouseGridSquare.second = 0;
	else if (mouseGridSquare.second >= GRID_WIDTH) mouseGridSquare.second = GRID_WIDTH - 1;

	/*std::wstring loc = L"Mouse Grid Location: {" + std::to_wstring(mouseGridSquare.first) + L", " + std::to_wstring(mouseGridSquare.second) + L"}\n";
	OutputDebugString(&loc[0]);*/

	for (int i = 0; i < m_gridLocations[mouseGridSquare.first][mouseGridSquare.second].size(); i++)
	{
		auto uiElement = m_gridLocations[mouseGridSquare.first][mouseGridSquare.second][i];
		uint32_t uiElementState = uiElement.get()->element->update(input);

		//Look at the state of the UIElement after processing the mouse input. Certain states require
		//the active mode to take an action (for example if a button is clicked). Any element which has
		//certain flags of its state change will be added to the action list of the UI Element Manager
		//for later processing by the current mode.
		if ((uiElementState & UIElementState::NeedTextPixels) || ((uiElementState & UIElementState::Clicked) && input->mouseClick))
		{
			m_actionElements.push_back(uiElement);
		}
	}
}

//void UIElementManager::createAlert(TextOverlay& alert)
//{
//	//Alerts are special kinds of text overlays. The pop up on the screen and only last for 
//	//a few seconds before disappearing again. Also, unlike other UIElements they persist
//	//between modes.
//	ManagedUIElement me = { L"Alert " + std::to_wstring(m_uiElements.at(UIElementType::ALERT).size()), std::make_shared<TextOverlay>(alert), {}, UIElementType::ALERT};
//	auto me_point = std::make_shared<ManagedUIElement>(me);
//	m_uiElements.at(UIElementType::ALERT).push_back(me_point);
//
//	//Also add the alert to the render list so we can see it
//	m_renderElements.push_back(me_point->element);
//
//	//After creating the alert set a timer, once the timer goes off we then 
//	//remove the alert
//	concurrency::task<void> timer([this, me_point]()
//		{
//			auto timer = std::chrono::steady_clock::now();
//			while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer).count() < m_alertTimer) {}
//
//			//Remove the alert from the render vector
//			auto render_it = std::find(m_renderElements.begin(), m_renderElements.end(), me_point->element);
//			m_renderElements.erase(render_it);
//
//			//Then remove it from the element map
//			auto map_it = findElementByName(m_uiElements.at(UIElementType::ALERT), me_point->name);
//			m_uiElements.at(UIElementType::ALERT).erase(map_it);
//		});
//}

void UIElementManager::checkAlerts()
{
	//Alerts get automatically deleted when they've been on screen for a set period of time.
	//Since it's possible for alerts to persist between different modes, we can't leave the 
	//destruction of the alerts up to themselves, but instead the UIElementManager class
	//(unlike buttons for example which have their own timers for how long they stay pressed).
	//Simply iterate through the alert array of the element map and check the timestampe of when
	//the alert was first created. If it's longer than the alerts slotted duration, it gets
	//deleted
	auto timestamp = std::chrono::steady_clock::now();
	for (int i = m_uiElements.at(UIElementType::ALERT).size() - 1; i >= 0; i--) //iterate backwards so removing alerts won't effect the iteration
	{
		auto alert = ((Alert*)m_uiElements.at(UIElementType::ALERT)[i]->element.get());
		if (std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - alert->m_startTime).count() >= alert->m_duration)
		{
			removeElement<Alert>(m_uiElements.at(UIElementType::ALERT)[i]->name);
		}
	}
}

std::vector<std::shared_ptr<ManagedUIElement>> UIElementManager::removeAlerts()
{
	//When we leave one mode and go to another, any active alerts are brought over as well.
	//To do this, we just copy the entire alert array that's inside of the element map and
	//set it as the alert array for the new mode. We don't need to worry about deleting the 
	//alerts in the current mode as this will happen automatically when the mode is 
	//uninitialized.. We create a copy of the alerts so that they persist after being
	//delete in this mode.
	return m_uiElements.at(UIElementType::ALERT);
}

void UIElementManager::overwriteAlerts(std::vector<std::shared_ptr<ManagedUIElement>> const& alerts)
{
	//Overwrite the alerts for the current mode with the given vector.
	m_uiElements.at(UIElementType::ALERT) = alerts;

	//The alerts also get placed into the render vector
	for (int i = 0; i < alerts.size(); i++) m_renderElements.push_back(alerts[i]->element);
}

std::vector<std::shared_ptr<ManagedUIElement> >::iterator UIElementManager::findElementByName(std::vector<std::shared_ptr<ManagedUIElement> >& vec, std::wstring name)
{
	//A lot of methods in this class involving finding elements in vectors. Since the ManagedUIElements
	//that make these vectors are compound structs, this helper method is used to search for them.
	return std::find_if(vec.begin(), vec.end(), [&n = name](const std::shared_ptr<ManagedUIElement>& e) -> bool {return n == e->name; });
}