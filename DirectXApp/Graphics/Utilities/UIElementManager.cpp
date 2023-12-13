#include "pch.h"
#include "UIElementManager.h"
#include "algorithm"

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

std::vector<std::shared_ptr<ManagedUIElement> >::iterator UIElementManager::findElementByName(std::vector<std::shared_ptr<ManagedUIElement> >& vec, std::wstring name)
{
	//A lot of methods in this class involving finding elements in vectors. Since the ManagedUIElements
	//that make these vectors are compound structs, this helper method is used to search for them.
	return std::find_if(vec.begin(), vec.end(), [&n = name](const std::shared_ptr<ManagedUIElement>& e) -> bool {return n == e->name; });
}