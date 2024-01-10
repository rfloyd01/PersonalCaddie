#include "pch.h"
#include "UIElementManager.h"
#include "algorithm"

#include <chrono>

//Populate the need text update vector -- deprecated
//std::vector<UIElementType> UIElementManager::m_textUpdateElements = { UIElementType::DROP_DOWN_MENU, UIElementType::PARTIAL_SCROLLING_TEXT_BOX, UIElementType::FULL_SCROLLING_TEXT_BOX };

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

	m_debugBoxCount = 0;

	//DEBUG: Uncomment the below lines to draw lines representing the grid on screen.
	//This is helpful for confirming which elements can be interacted with by the mouse
	/*for (int i = 0; i < GRID_WIDTH; i++)
	{
		Line vert(m_windowSize, { (float)i / (float)GRID_WIDTH, 0.0f }, { (float)i / (float)GRID_WIDTH, 1.0f }, UIColor::Red, 1.0f);
		Line hor(m_windowSize, { 0.0f, (float)i / (float)GRID_WIDTH }, { 1.0f, (float)i / (float)GRID_WIDTH }, UIColor::Red, 1.0f);
		addElement<Line>(vert, L"Vertical Line " + std::to_wstring(i));
		addElement<Line>(hor, L"Horizontal Line " + std::to_wstring(i));
	}*/
}

void UIElementManager::removeElementType(UIElementType type)
{
	//Removes all UI Elements with the given type
	for (auto it = m_uiElements.at(type).begin(); it != m_uiElements.at(type).end(); it++)
	{
		//Remove the UIElement from the render vector first.
		/*auto render_it = std::find(m_renderElements.begin(), m_renderElements.end(), it->get()->element);
		m_renderElements.erase(render_it);*/

		for (int i = 0; i < it->get()->grid_locations.size(); i++)
		{
			//The grid vectors aren't ordered so we need to search for the appropriate
			//smart pointer. This may not seem super effecient, especially because removing
			//elements from a vector forces all other elements to shift, however, if
			//the grid is small enough then there should never be more than a handful of 
			//pointers in each vector so this should be alright.
			std::pair<int, int> grid_location = it->get()->grid_locations[i];
			//auto grid_it = findElementByName(m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second], it->get()->name);
			auto grid_it = std::find(m_gridLocations[grid_location.first][grid_location.second].begin(), m_gridLocations[grid_location.first][grid_location.second].end(), *it);
			if (grid_it != m_gridLocations[grid_location.first][grid_location.second].end()) m_gridLocations[grid_location.first][grid_location.second].erase(grid_it);
		}
	}
	
	m_uiElements.at(type).clear(); //finally clear out the appropriate vector in the ui element map
}

void UIElementManager::removeAllElements()
{
	//Clear out the render and action vectors, the element map and the element grid
	//m_renderElements.clear();
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

void UIElementManager::updateScreenSize(winrt::Windows::Foundation::Size newWindowSize)
{ 
	//This method automatically gets called when the screen changes sizes, it causes all UI elements
	//on the screen to change proportionally with the new screen size.
	m_screenSize = std::make_shared<winrt::Windows::Foundation::Size>(newWindowSize); //all UIElements share this smart pointer so they see the change as well

	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		std::vector<std::shared_ptr<ManagedUIElement>>& elements = m_uiElements.at(static_cast<UIElementType>(i));
		for (int j = 0; j < elements.size(); j++)
		{
			elements[j]->element->resize();

			//Some elements are dynamically sized based on the text inside of them. If calling
			//the resize method on an element causes the NeedTextPixels flag to appear in the 
			//element's state, the element gets added to the m_updateText vector
			if (elements[j]->element->getState() & UIElementState::NeedTextPixels) m_updateText.push_back(elements[j]->element);
		}
	}
}

void UIElementManager::updateGridSquareElements(InputState* input)
{
	//If the mouse moves, clicks or scrolls we inspect all of the UIElements that are in the 
	//same grid square as the mouse and apply these updates. Mouse coordinates come in as 
	//absolute pixels and not relative ones.
	std::pair<int, int> mouseGridSquare = { GRID_WIDTH * (input->mousePosition.y / m_screenSize->Height), GRID_WIDTH * (input->mousePosition.x / m_screenSize->Width) };

	//Make sure the mouse calculation puts it inside of the screen
	if (mouseGridSquare.first < 0) mouseGridSquare.first = 0;
	else if (mouseGridSquare.first >= GRID_WIDTH) mouseGridSquare.first = GRID_WIDTH - 1;

	if (mouseGridSquare.second < 0) mouseGridSquare.second = 0;
	else if (mouseGridSquare.second >= GRID_WIDTH) mouseGridSquare.second = GRID_WIDTH - 1;

	/*std::wstring loc = L"Mouse Grid Location: {" + std::to_wstring(mouseGridSquare.first) + L", " + std::to_wstring(mouseGridSquare.second) + L"}\n";
	OutputDebugString(&loc[0]);*/

	/*std::wstring loc = L"Mouse Location: {" + std::to_wstring(input->mousePosition.x) + L", " + std::to_wstring(input->mousePosition.y) + L"}\n";
	OutputDebugString(&loc[0]);*/

	for (int i = 0; i < m_gridLocations[mouseGridSquare.first][mouseGridSquare.second].size(); i++)
	{
		auto uiElement = m_gridLocations[mouseGridSquare.first][mouseGridSquare.second][i];
		uint32_t uiElementState = uiElement.get()->element->update(input);

		//If the given UIElement has been clicked then add it to the UIElementManager's
		//clicked list. This list keeps track of elements that have been clicked even 
		//if the mouse moves to another grid square before being released. If the mouse 
		//is released over one of these clicked items then it gets added to the action list
		//which will cause the current mode to perform some kind of action.
		if ((input->mouseClickState == MouseClickState::MouseClicked) && (uiElementState & UIElementState::Clicked))
		{
			//Only add elements to the clickedElement array during a physical click event
			m_clickedElements.push_back(uiElement);
		}
		else if (uiElementState & UIElementState::Released)
		{
			m_clickedElements.erase(std::find(m_clickedElements.begin(), m_clickedElements.end(), uiElement));//remove the element from the clicked list
			m_actionElements.push_back(uiElement); //and add it to the action list
		}
	}

	//It's possible that an element was clicked with the mouse, but then the mouse moved off of 
	//the element before being released. In a case like this the release logic for the element won't 
	//occur (i.e. a button's color won't revert to the non-clicked color). Check to see if the 
	//MouseReleased state is active
	if (input->mouseClickState == MouseClickState::MouseReleased)
	{
		for (int i = 0; i < m_clickedElements.size(); i++)
		{
			//Any element in the click array will manually be unclicked here. These
			//elements won't be added to the action list though so their functionality
			//won't actually happen

			//TODO: There may be some cases where I do want the functionality to happen
			//so I may need to revisit this method in the future
			m_clickedElements[i]->element->update(input);
		}
		m_clickedElements.clear(); //remove everything from the clicked array when done
	}
}

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
	//for (int i = 0; i < alerts.size(); i++) m_renderElements.push_back(alerts[i]->element);
}

std::vector<std::shared_ptr<ManagedUIElement> >::iterator UIElementManager::findElementByName(std::vector<std::shared_ptr<ManagedUIElement> >& vec, std::wstring name)
{
	//A lot of methods in this class involving finding elements in vectors. Since the ManagedUIElements
	//that make these vectors are compound structs, this helper method is used to search for them.
	return std::find_if(vec.begin(), vec.end(), [&n = name](const std::shared_ptr<ManagedUIElement>& e) -> bool {return n == e->name; });
}

//void UIElementManager::checkForTextResize()
//{
//  DEPRECATED
//	//Some UIElements rely on the renderer class to update their dimensions by first rendering text
//	//and calculating the size in pixels. To make this process more seemless, this method gets called
//	//during each iteration of the render loop to check whether any UI Elements need an update. As of
//	//right now the only elements that require this feature are: FullScrollingTextBox, DropDownMenu,
//	//and PartialScrollingTextBox. As more UI Elements are created they will need to be added to the
//	//static m_textUpdateElements vector.
//	
//	for (int type = 0; type < m_textUpdateElements.size(); type++)
//	{
//		for (int i = 0; i < m_uiElements.at(m_textUpdateElements[type]).size(); i++)
//		{
//			if (m_uiElements.at(m_textUpdateElements[type])[i]->element->getState() & UIElementState::NeedTextPixels)
//			{
//				m_updateText.push_back(m_uiElements.at(m_textUpdateElements[type])[i]->element);
//			}
//		}
//	}
//}

//std::vector<UIText*> UIElementManager::getResizeText()
//{
//   DEPRECATED
//	//The m_updateText array holds entire UIElements, however, we really only care about the UIText elements inside
//	//of the UIElements. Extract any UIText that we need, and add it to a new vector which gets returned from this
//	//method.
//	std::vector<UIText*> textElements;
//	for (int i = 0; i < m_updateText.size(); i++)
//	{
//		auto elementText = m_updateText[i]->setTextDimension(); //get the actual UIText elements with the text needing updating
//		for (int j = 0; j < elementText.size(); j++) textElements.push_back(elementText[j]);
//	}
//
//	return textElements; //return a copy of this temporary vector
//}

void UIElementManager::applyTextResizeUpdates()
{
	//Once the text elements in the m_updateText array have been updated we need to reposition the text, and 
	//then resize the UIElements that they're apart of to complete the update.
	for (int i = 0; i < m_updateText.size(); i++)
	{
		m_updateText[i]->repositionText(); //see if any text needs to be repositioned after getting new dimensions
		m_updateText[i]->resize(); //and then resize the ui element
	}

	//After all updates are made, clear out the m_updateText vector
	m_updateText.clear();
}

void UIElementManager::populateGridLocations(std::shared_ptr<ManagedUIElement> managedElement)
{
	//Looks at the dimensions and location of the passed in ManagedUIElement and creates pointers to 
	//the element in each appropriate vector of the m_gridLocations data structure.
	if (managedElement->type != UIElementType::ALERT) //Since we can't interact with alerts they're excluded from being placed inside the grid
	{
		//Remove any existing grid locations if the grid array for the managed element isn't empty. This situation
		//arises if we move an existing element after creating it.
		if (managedElement->grid_locations.size() > 0) managedElement->grid_locations.clear();

		//We don't add alerts to the grid as they can't be interacted with
		auto size = managedElement->element->getAbsoluteSize();
		auto location = managedElement->element->getAbsoluteLocation();

		std::pair<int, int> top_left = { GRID_WIDTH * (location.x - size.x / 2.0f), GRID_WIDTH * (location.y - size.y / 2.0f) };
		std::pair<int, int> bottom_right = { GRID_WIDTH * (location.x + size.x / 2.0f), GRID_WIDTH * (location.y + size.y / 2.0f) };

		if (managedElement->type == UIElementType::DROP_DOWN_MENU)
		{
			//Drop down menus feature an invisible scroll box which isn't normally included in the size calculation
			//of the element. To make sure all parts of this scroll box can be interacted with we add the height of
			//the invisible scroll box when placing grid pointers for the drop down menu. This only effects the
			//top_left grid location
			top_left = { GRID_WIDTH * (location.x - size.x / 2.0f), GRID_WIDTH * (location.y - size.y / 2.0f - managedElement->element->getChildren()[2]->getAbsoluteSize().y)};
		}

		//Ignore any squares that fall outside of the grid
		for (int row = top_left.second; row <= bottom_right.second; row++)
		{
			if (row < 0) continue;
			else if (row >= GRID_WIDTH) break;

			for (int col = top_left.first; col <= bottom_right.first; col++)
			{
				if (col < 0) continue;
				else if (col >= GRID_WIDTH) break;

				//Add the grid location to the managed element, and a reference to the managed element
				//in the appropriate grid location
				managedElement->grid_locations.push_back({ row, col });
				m_gridLocations[row][col].push_back(managedElement);
			}
		}
	}
}

void UIElementManager::refreshGrid()
{
	//There are times when a UI Element might be moved from the original location that it was created at,
	//A good example of this would be in IMU Settings mode where all dropdowns are initially created at the
	//location [0, 0] as we don't know where to place them until after they've been physically created (and
	//have their corresponding sizes). In this scenario, the physical location of the UI Element will no 
	//longer match its grid location within the UI Management class and we won't be able to interact with it
	//properly. Calling this method will simply delete the entire grid, and then go over every single UI Element
	//in the map, re-populating the grid with correct locations.

	//Clear out each vector currently in the grid
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_WIDTH; j++) m_gridLocations[i][j].clear();
	}

	//Iterate through all UI Elements and add new grid locations
	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		std::vector<std::shared_ptr<ManagedUIElement>>& elementVector = m_uiElements.at(static_cast<UIElementType>(i));
		for (int j = 0; j < elementVector.size(); j++) populateGridLocations(elementVector[j]);
	}
}

void UIElementManager::drawDebugOutline(std::shared_ptr<UIElement> element, bool draw_children)
{
	//This method draws red lines around the edge of the given UI Element
	//and places a small circle in the center to confirm that the element
	//looks correct. This effect can also be cascaded down to all child
	//elements if desired.
	Box debug(m_screenSize, element->getAbsoluteLocation(), element->getAbsoluteSize(), UIColor::Red, UIShapeFillType::NoFill);
	Ellipse ell(m_screenSize, element->getAbsoluteLocation(), { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.001f, 0.001f }, false, UIColor::Red);
	addElement<Box>(debug, L"Debug " + std::to_wstring(++m_debugBoxCount));
	addElement<Ellipse>(ell, L"Debug Ellipse " + std::to_wstring(m_debugBoxCount));

	if (draw_children)
	{
		for (int i = 0; i < element->getChildren().size(); i++) drawDebugOutline(element->getChildren()[i], draw_children);
	}
}