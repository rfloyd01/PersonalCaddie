#pragma once

#include <map>
#include <vector>

#include "Graphics/Objects/2D/UIElements.h"

#define GRID_WIDTH 5

//This enum holds all the different types of high level UI elements
enum class UIElementType
{
	BUTTON,
	TEXT_BUTTON,
	ARROW_BUTTON,
	TEXT_BOX,
	PARTIAL_SCROLLING_TEXT_BOX,
	FULL_SCROLLING_TEXT_BOX,
	HIGHLIGHTABLE_TEXT_OVERLAY,
	DROP_DOWN_MENU,
	GRAPH,
	END
};

struct ManagedUIElement
{
	std::wstring name;
	std::shared_ptr<UIElement> element;
	std::vector<std::pair<int, int> > grid_locations;
};

/*
*/
class UIElementManager
{
public:
	UIElementManager();

	void addElement(UIElementType type, std::shared_ptr<UIElement> element, std::wstring name);
	void removeElement(UIElementType type, std::wstring name);
	void removeAllElements();

	//Methods for getting appropriate tyep pointers. I'm not sure that there's
	//a way to automatically cast a parent pointer to the correct child pointer
	//without doing it manually, so for now there's an individual method for 
	//each pointer type that the manager can hold

	std::shared_ptr<Button> getButton(std::wstring name);
	/*std::shared_ptr<TextButton> getTextButton(std::wstring name);
	std::shared_ptr<ArrowButton> getArrowButton(std::wstring name);
	std::shared_ptr<TextBox> getTextBox(std::wstring name);
	std::shared_ptr<PartialScrollingTextBox> getPartialScrollingTextBox(std::wstring name);
	std::shared_ptr<FullScrollingTextBox> getFullScrollingTextBox(std::wstring name);
	std::shared_ptr<TextOverlay> getTextOverlay(std::wstring name);
	std::shared_ptr<HighlightableTextOverlay> getHighlightableTextOverlay(std::wstring name);
	std::shared_ptr<DropDownMenu> getDropDownMenu(std::wstring name);
	std::shared_ptr<Graph> getGraph(std::wstring name);*/

	template <typename T>
	std::shared_ptr<T> getElement(std::wstring name)
	{
		static_assert(std::is_base_of<UIElement, T>::value, "getElement<T>() must use a class that inherits from the UIElement class");

		//The UIElementType needs to match its equivalent typenameT or an error will occur.
		//TODO: There must be a good way to perform this check in code.
		UIElementType type = type_to_UIElementType<T>();
		if (type == UIElementType::END)
		{
			//A class was passed in that we haven't implemented the type_to_UIElementType<> template for yet.
			return nullptr;

		}
		auto it = findElementByName(type, name);
		if (it != m_uiElements.at(type).end())
		{
			return std::dynamic_pointer_cast<T>(it->get()->element);
		}
		
		//If the wanted UIElement isn't found we instead return a dummy element
		//and kick up an error message. This way we can let the user know that the
		//element wasn't updated as intended without causing any errors.
		std::wstring message = L"Couldn't find UIElement with name " + name;
		OutputDebugString(&message[0]);
		return std::dynamic_pointer_cast<T>(findElementByName(type, m_dummyName)->get()->element);
	}

private:
	std::wstring m_dummyName = L"dummy";

	std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > m_uiElements;
	std::vector<std::vector<std::vector<std::shared_ptr<ManagedUIElement> > > > m_gridLocations;

	std::vector<std::shared_ptr<ManagedUIElement> >::iterator findElementByName(UIElementType type, std::wstring name);
	void createDummyVariable(UIElementType type);

	//Methods for converting between UIElement classes and their corresponding UIElementType
	template <typename T>
	UIElementType type_to_UIElementType()
	{
		//If we try and call this method with a class that hasn't been implemented yet
		//the following assertion automatically fails and we get the appropriate error message.
		//This is to act as a reminder for me in the future that if I create more UIElement 
		//classes that I need to add to the UIElementType enum.
		static_assert(false, "type_to_UIElementType<>() method needs implementation.");
	}

	template<>
	UIElementType type_to_UIElementType<Button>() { return UIElementType::BUTTON; }

	template<>
	UIElementType type_to_UIElementType<ArrowButton>() { return UIElementType::ARROW_BUTTON; }

	template<>
	UIElementType type_to_UIElementType<FullScrollingTextBox>() { return UIElementType::FULL_SCROLLING_TEXT_BOX; }
};