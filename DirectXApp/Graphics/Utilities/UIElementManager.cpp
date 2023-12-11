#include "pch.h"
#include "UIElementManager.h"
#include "algorithm"

UIElementManager::UIElementManager()
{
	//create the map with an empty entry for each type of UI Element
	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		m_uiElements.insert({ static_cast<UIElementType>(i), {} });
		createDummyVariable(static_cast<UIElementType>(i));
	}

	//Create the location grid
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		std::vector<std::vector<std::shared_ptr<ManagedUIElement> > > vec;
		for (int j = 0; j < GRID_WIDTH; j++) vec.push_back({});

		m_gridLocations.push_back(vec);
	}
}

void UIElementManager::addElement(UIElementType type, std::shared_ptr<UIElement> element, std::wstring name)
{
	//We create a ManagedUIElement from the info passed in and add it to both the managedElement 
	//map and the location grid data structure. To calculate which portions of the grid the element
	//falls into we use the following equations:
	//Top Left location     = {floor(GRID_WIDTH * (location.x - size.x / 2.0)), floor(GRID_WIDTH * (location.y - size.y / 2.0))}
	//Bottom Right location = {floor(GRID_WIDTH * (location.x + size.x / 2.0)), floor(GRID_WIDTH * (location.y + size.y / 2.0))}
	//We just iterate over all grid square from the top left to the bottom right

	ManagedUIElement me = { name, element, {}};
	auto managedElement = std::make_shared<ManagedUIElement>(me); //create a copy of the element so we can get the absolute size and location

	auto size = managedElement->element->getAbsoluteSize();
	auto location = managedElement->element->getAbsoluteLocation();

	std::pair<int, int> top_left     = { GRID_WIDTH * (location.x - size.x / 2.0f), GRID_WIDTH * (location.y - size.y / 2.0f) };
	std::pair<int, int> bottom_right = { GRID_WIDTH * (location.x + size.x / 2.0f), GRID_WIDTH * (location.y + size.y / 2.0f) };

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

	//Potential: Make sure that no other UIElements with the same name and element type
	//exist. Also make sure that there's no overlap between this element and any others
	//(unless there's allowed to be)

	//Then add the element to the back of it's appropriate array in the m_uiElements map
	m_uiElements.at(type).push_back(managedElement);
}

void UIElementManager::removeElement(UIElementType type, std::wstring name)
{
	//Removes the element with the given name from the appropriate ElementType list.
	//If the element isn't there it will just print out an error statement.
	auto it = findElementByName(type, name);

	if (it != m_uiElements.at(type).end()) m_uiElements.at(type).erase(it);
	else OutputDebugString(L"Couldn't find the UIElement.");
}

void UIElementManager::removeAllElements()
{
	//Simply remove all of the ManagedUIElements in the arrays of the map
	for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
	{
		m_uiElements.at(static_cast<UIElementType>(i)).clear();
	}

	for (int i = 0; i < GRID_WIDTH; i++)
	{
		for (int j = 0; j < GRID_WIDTH; j++) m_gridLocations[i][j].clear();
	}
}

std::shared_ptr<Button> UIElementManager::getButton(std::wstring name)
{
	auto it = findElementByName(UIElementType::BUTTON, name);
	
	if (it != m_uiElements.at(UIElementType::BUTTON).end()) return std::dynamic_pointer_cast<Button>(it->get()->element);
	else OutputDebugString(L"Couldn't find the UIElement.");
}

std::vector<std::shared_ptr<ManagedUIElement> >::iterator UIElementManager::findElementByName(UIElementType type, std::wstring name)
{
	//A lot of methods of this class involving finding elements in vectors. Since the ManagedUIElements
	//that make these vectors are compound structs, this helper method is used to search for them.
	std::vector<std::shared_ptr<ManagedUIElement> >& vec = m_uiElements.at(type);
	return std::find_if(vec.begin(), vec.end(), [&n = name](const std::shared_ptr<ManagedUIElement>& e) -> bool {return n == e->name; });
}

void UIElementManager::createDummyVariable(UIElementType type)
{
	//This method is used to load up the uielement map with dummy pointers. In the case a UIElement isn't found 
	//and the user tries to call a method on a pointer to that element we instead pass a pointer to the appropriate
	//dummy element. This prevents null pointer exceptions from happening while also kicking out an error message to 
	//the user letting them know something went wrong.
	ManagedUIElement dummy = { m_dummyName, nullptr, {} };
	switch (type)
	{
	case UIElementType::BUTTON:
	{
		dummy.element = std::make_shared<Button>();
		break;
	}
	case UIElementType::TEXT_BUTTON:
	{
		TextButton dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"");
		dummy.element = std::make_shared<TextButton>(dum);
		break;
	}
	case UIElementType::ARROW_BUTTON:
	{
		ArrowButton dum({ 0, 0 }, { 0, 0 }, { 0, 0 });
		dummy.element = std::make_shared<ArrowButton>(dum);
		break;
	}
	case UIElementType::TEXT_BOX:
	{
		TextBox dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0.0f);
		dummy.element = std::make_shared<TextBox>(dum);
		break;
	}
	case UIElementType::PARTIAL_SCROLLING_TEXT_BOX:
	{
		PartialScrollingTextBox dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, UIColor::White, L"", 0.0f);
		dummy.element = std::make_shared<PartialScrollingTextBox>(dum);
		break;
	}
	case UIElementType::FULL_SCROLLING_TEXT_BOX:
	{
		FullScrollingTextBox dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0.0f);
		dummy.element = std::make_shared<FullScrollingTextBox>(dum);
		break;
	}
	case UIElementType::HIGHLIGHTABLE_TEXT_OVERLAY:
	{
		HighlightableTextOverlay dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"0", 0.0f, {UIColor::White}, {0, 1}, UITextJustification::CenterCenter);
		dummy.element = std::make_shared<HighlightableTextOverlay>(dum);
		break;
	}
	case UIElementType::DROP_DOWN_MENU:
	{
		DropDownMenu dum({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0.0f);
		dummy.element = std::make_shared<DropDownMenu>(dum);
		break;
	}
	case UIElementType::GRAPH:
	{
		Graph dum({ 0, 0 }, { 0, 0 }, { 0, 0 });
		dummy.element = std::make_shared<Graph>(dum);
		break;
	}
	}

	m_uiElements.at(type).push_back(std::make_shared<ManagedUIElement>(dummy)); //add the dummy managed ui element to the proper vector
}