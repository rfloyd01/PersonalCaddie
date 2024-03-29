#include "pch.h"
#include "UIElementManager.h"
#include "algorithm"

#include <chrono>

UIElementManager::UIElementManager()
{
	//Create a shared pointer that will hold the current screen size.This pointer will
	//be shared by every single UI Element that's managed by the manager class. Any 
	//time the screen size changes, UI Elements will have direct access to the new size.
	winrt::Windows::Foundation::Size defaultSize = { 0, 0 };
	m_screenSize = std::make_shared<winrt::Windows::Foundation::Size>(defaultSize);

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
}

void UIElementManager::removeElementType(UIElementType type)
{
	//Removes all UI Elements with the given type
	for (auto it = m_uiElements.at(type).begin(); it != m_uiElements.at(type).end(); it++)
	{
		for (int i = it->get()->top_left_grid_location.first; i <= it->get()->bottom_right_grid_location.first; i++)
		{
			for (int j = it->get()->top_left_grid_location.second; j <= it->get()->bottom_right_grid_location.second; j++)
			{
				//The grid vectors aren't ordered so we need to search for the appropriate
				//smart pointer. This may not seem super effecient, especially because removing
				//elements from a vector forces all other elements to shift, however, if
				//the grid is small enough then there should never be more than a handful of 
				//pointers in each vector so this should be alright.
				auto grid_it = std::find(m_gridLocations[j][i].begin(), m_gridLocations[j][i].end(), *it);
				if (grid_it != m_gridLocations[j][i].end()) m_gridLocations[j][i].erase(grid_it);
			}
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
	//on the screen to change proportionally with the new screen size. This action can be somewhat
	//computationally expensive, so to help make things more efficient a resize will only actually
	//occur if the area of the screen changes by a certain threshold.
	*m_screenSize.get() = newWindowSize; //all UIElements share this smart pointer so they see the change as well
	float screenArea = newWindowSize.Width * newWindowSize.Height;

	if (((screenArea / m_lastScreenResizeArea) >= (1.0f + m_screenResizeThreshold)) || ((screenArea / m_lastScreenResizeArea) <= (1.0f - m_screenResizeThreshold)))
	{
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

		//Resizing the screen can cause UI Elements to shift slightly, causing them to move into 
		//new grid locations. If we still want to be able to interact with UI Elements then we 
		//also need to refresh the grid on every resize.
		refreshGrid();

		//Update the m_lastScreenResizeArea variable
		m_lastScreenResizeArea = screenArea;
	}

	//DEBUG: Uncomment the below lines to draw lines representing the grid on screen.
	//This is helpful for confirming which elements can be interacted with by the mouse
	/*for (int i = 0; i < GRID_WIDTH; i++)
	{
		Line vert(m_screenSize, { (float)i / (float)GRID_WIDTH, 0.0f }, { (float)i / (float)GRID_WIDTH, 1.0f }, UIColor::Red, 1.0f, true);
		Line hor(m_screenSize, { 0.0f, (float)i / (float)GRID_WIDTH }, { 1.0f, (float)i / (float)GRID_WIDTH }, UIColor::Red, 1.0f, true);
		addElement<Line>(vert, L"Vertical Line " + std::to_wstring(i));
		addElement<Line>(hor, L"Horizontal Line " + std::to_wstring(i));
	}*/
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

	for (int i = 0; i < m_gridLocations[mouseGridSquare.first][mouseGridSquare.second].size(); i++)
	{
		auto uiElement = m_gridLocations[mouseGridSquare.first][mouseGridSquare.second][i];
		uint32_t uiElementState = uiElement.get()->element->update(input);

		//If the given UIElement has been clicked then add it to the UIElementManager's
		//clicked list. This list keeps track of elements that have been clicked even 
		//if the mouse moves to another grid square before being released. If the mouse 
		//is released over one of these clicked items, or if the mouse is hovering over a
		//hoverable item, then it gets added to the action list. This will cause the current
		//mode to perform an action of its own choosing.
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
		else if (uiElementState & UIElementState::Hovered)
		{
			m_actionElements.push_back(uiElement); //add to action list without removing from clicked list
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
			m_clickedElements[i]->element->update(input);
		}
		m_clickedElements.clear(); //remove everything from the clicked array when done
	}

	//TODO: If the mouse moves off of a hovered item too quickly the item will remain hovered
	//sometimes. I should put in some check to prevent this from happening.
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

	//Finally, refresh the locations of each UI Element in the manager's grid.
	//Updating elements with text sizes may change their total size and or location
	//so we want to make sure the grid stays up to date.
	refreshGrid();
}

void UIElementManager::populateGridLocations(std::shared_ptr<ManagedUIElement> managedElement)
{
	//Looks at the dimensions and location of the passed in ManagedUIElement and creates pointers to 
	//the element in each appropriate vector of the m_gridLocations data structure.
	if (managedElement->type != UIElementType::ALERT) //Since we can't interact with alerts they're excluded from being placed inside the grid
	{
		//Remove any existing grid locations if the grid array for the managed element isn't empty. This situation
		//arises if we move an existing element after creating it.
		/*if (managedElement->grid_locations.size() > 0) managedElement->grid_locations.clear();*/

		//The manager's grid system uses absolute coordinates while UI Elements use relative coordinates
		//and converting between the two systems can get a little confusing. To help, simply use pixel
		//coordinates to figure out which elements fall in which grid squares.
		auto pixel_size = managedElement->element->getPixelSize();
		auto pixel_location = managedElement->element->getPixelLocation();

		//Calculate the new range of grid locations
		std::pair<int, int> top_left = { GRID_WIDTH * (pixel_location.x - pixel_size.x / 2.0f) / m_screenSize->Width, GRID_WIDTH * (pixel_location.y - pixel_size.y / 2.0f) / m_screenSize->Height };
		std::pair<int, int> bottom_right = { GRID_WIDTH * (pixel_location.x + pixel_size.x / 2.0f) / m_screenSize->Width, GRID_WIDTH * (pixel_location.y + pixel_size.y / 2.0f) / m_screenSize->Height};

		//See if either of the new grid locations is different than the existing grid locations.
		//If not then there's nothing to update
		if ((top_left == managedElement->top_left_grid_location) && (bottom_right == managedElement->bottom_right_grid_location)) return;

		//Between the new and old grid locations, select the most extreme left, right, top and 
		//bottom grid locations and iterate between them. By comparing the old and new upper_left and
		//bottom_right grid locations vs. the current place in the iteration we can deduce whether
		//the current spot needs to have a reference added, removed, or simply be left alone.
		int new_left = top_left.first, old_left = managedElement->top_left_grid_location.first;
		int new_right = bottom_right.first, old_right = managedElement->bottom_right_grid_location.first;
		int new_top = top_left.second, old_top = managedElement->top_left_grid_location.second;
		int new_bottom = bottom_right.second, old_bottom = managedElement->bottom_right_grid_location.second;

		int leftmost_point = new_left < old_left ? new_left : old_left;
		int rightmost_point = new_right > old_right ? new_right : old_right;
		int topmost_point = new_top < old_top ? new_top : old_top;
		int bottommost_point = new_bottom > old_bottom ? new_bottom : old_bottom;

		//If the new element is out of bounds at all update its grid locations
		//accordingly
		if (leftmost_point < 0) leftmost_point = top_left.first;
		if (topmost_point < 0) topmost_point = top_left.second;
		if (rightmost_point >= GRID_WIDTH) rightmost_point = GRID_WIDTH - 1;
		if (bottommost_point >= GRID_WIDTH) bottommost_point = GRID_WIDTH - 1;

		for (int i = leftmost_point; i <= rightmost_point; i++)
		{
			for (int j = topmost_point; j <= bottommost_point; j++)
			{
				bool inside_old_location = ((i >= old_left && i <= old_right) && (j >= old_top && j <= old_bottom));
				bool inside_new_location = ((i >= new_left && i <= new_right) && (j >= new_top && j <= new_bottom));

				//Check to see if we have a new point. A point is considered new if it lies inside
				//the new location but not the old location
				if (inside_new_location && !inside_old_location)
				{
					//care needs to be taken when adding pointers to the grid since the 
					//order is (row, column) which would be (y, x) using normal Cartesean
					//coordinates
					m_gridLocations[j][i].push_back(managedElement);
				}
				else if (inside_old_location && !inside_new_location)
				{
					//Likewise with adding a reference, when we remove one we need
					//to make sure we properly swap the i and j variables from 
					//Cartesean coordinates.
					auto it = std::find(m_gridLocations[j][i].begin(), m_gridLocations[j][i].end(), managedElement);
					
					//Make sure the pointer was actually found before erasing it
					if (it != m_gridLocations[j][i].end()) m_gridLocations[j][i].erase(it);
				}

				//If neither of the above blocks execues it means that tthe point lies inside both 
				//locations so nothing needs to be changed
			}
		}

		//Finally, update the top left and bottom right grid locations for the Managed Element
		managedElement->top_left_grid_location = top_left;
		managedElement->bottom_right_grid_location = bottom_right;
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