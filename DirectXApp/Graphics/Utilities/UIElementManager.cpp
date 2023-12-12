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

//void UIElementManager::removeElement(UIElementType type, std::wstring name)
//{
//	//When removing a UI element we need to remove it from the render vector, as well
//	//as all smart pointers stored in the element grid and element map.
//    auto it = findElementByName(m_uiElements.at(type), name);
//
//	if (it != m_uiElements.at(type).end())
//	{
//		//Remove the UIElement from the render vector first.
//		auto render_it = std::find(m_renderElements.begin(), m_renderElements.end(), it->get()->element);
//		m_renderElements.erase(render_it);
//
//		for (int i = 0; i < it->get()->grid_locations.size(); i++)
//		{
//			//The grid vectors aren't ordered so we need to search for the appropriate
//			//smart pointer. This may not seem super effecient, especially because removing
//			//elements from a vector forces all other elements to shift, however, if
//			//the grid is small enough then there should never be more than a handful of 
//			//pointers in each vector so this should be alright.
//			std::pair<int, int> grid_location = { it->get()->grid_locations[i].first, it->get()->grid_locations[i].second };
//			auto grid_it = findElementByName(m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second], name);
//			m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second].erase(grid_it);
//		}
//		m_uiElements.at(type).erase(it);
//	}
//	else OutputDebugString(L"Couldn't find the UIElement.");
//}

void UIElementManager::removeAllElements()
{
	//Clear out the render vector, the element map and the element grid
	m_renderElements.clear();

	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		m_uiElements.at(static_cast<UIElementType>(i)).clear();
	}

	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_WIDTH; j++) m_gridLocations[i][j].clear();
	}
}

std::vector<std::shared_ptr<ManagedUIElement> >::iterator UIElementManager::findElementByName(std::vector<std::shared_ptr<ManagedUIElement> >& vec, std::wstring name)
{
	//A lot of methods in this class involving finding elements in vectors. Since the ManagedUIElements
	//that make these vectors are compound structs, this helper method is used to search for them.
	return std::find_if(vec.begin(), vec.end(), [&n = name](const std::shared_ptr<ManagedUIElement>& e) -> bool {return n == e->name; });
}