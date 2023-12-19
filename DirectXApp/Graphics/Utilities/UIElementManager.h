#pragma once

#include <map>
#include <vector>

#include "Graphics/Objects/2D/UIElements.h"

#define GRID_WIDTH 10 //designates how many squares the screen is split into for update processing

//This enum holds all the different types of high level UI elements
enum class UIElementType
{
	BUTTON,
	TEXT_BUTTON,
	ARROW_BUTTON,
	TEXT_BOX,
	PARTIAL_SCROLLING_TEXT_BOX,
	FULL_SCROLLING_TEXT_BOX,
	TEXT_OVERLAY,
	HIGHLIGHTABLE_TEXT_OVERLAY,
	DROP_DOWN_MENU,
	GRAPH,
	LINE,
	ALERT,
	END
};

struct ManagedUIElement
{
	std::wstring name;
	std::shared_ptr<UIElement> element;
	std::vector<std::pair<int, int> > grid_locations;
	UIElementType type;
};

class UIElementManager
{
public:
	UIElementManager();

	template <typename T>
	void addElement(T const& element, std::wstring name)
	{
		//Before adding anything, make sure that any element with this type and name doesn't exist already. The name
	    //and type is how we uniquely identify each element, so having two with the same name could lead to unexpected
	    //results.
		UIElementType type = type_to_UIElementType<T>(); //convert the UIElement class to its corresponding UIElementType
		auto it = findElementByName(m_uiElements.at(type), name);
		if (it != m_uiElements.at(type).end())
		{
			OutputDebugString(L"A UI Element with this name and type exists already.\n");
			return; //don't add anything
		}

		//We create a ManagedUIElement from the info passed in and add it to both the managedElement 
		//map and the location grid data structure. To calculate which portions of the grid the element
		//falls into we use the following equations:
		//Top Left location     = {floor(GRID_WIDTH * (location.x - size.x / 2.0)), floor(GRID_WIDTH * (location.y - size.y / 2.0))}
		//Bottom Right location = {floor(GRID_WIDTH * (location.x + size.x / 2.0)), floor(GRID_WIDTH * (location.y + size.y / 2.0))}
		//We just iterate over all grid square from the top left to the bottom right

		ManagedUIElement me = { name, std::make_shared<T>(element), {}, type };
		auto managedElement = std::make_shared<ManagedUIElement>(me); //create a copy of the element so we can get the absolute size and location

		populateGridLocations(managedElement); //Populate the appropriate arrays of m_gridLocation with pointers to this new ManagedUIElement

		//Then add the element to the back of it's appropriate array in the m_uiElements map
		m_uiElements.at(type).push_back(managedElement);

		//Once the ManagedUIElement is created, we take the shared pointer for the actual UIElement and at
		//it to the render vector. This vector is what ultimately gets passed to the render class to create
		//visuals on screen. The order of the elements in this vector matters (things rendered at the back of 
		//the vector will be rendered on top of elements earlier in the vector in the case of overlap).
		//m_renderElements.push_back(managedElement->element);
	}

	template <typename T>
	void removeElement(std::wstring name)
	{
		//When removing a UI element we need to remove it from the render vector, as well
		//as all smart pointers stored in the element grid and element map.
		UIElementType type = type_to_UIElementType<T>(); //convert the UIElement class to its corresponding UIElementType
		auto it = findElementByName(m_uiElements.at(type), name);

		if (it != m_uiElements.at(type).end())
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
				std::pair<int, int> grid_location = { it->get()->grid_locations[i].first, it->get()->grid_locations[i].second };
				auto grid_it = findElementByName(m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second], name);
				m_gridLocations[it->get()->grid_locations[i].first][it->get()->grid_locations[i].second].erase(grid_it);
			}
			m_uiElements.at(type).erase(it);
		}
		else OutputDebugString(L"Couldn't find the UIElement.");
	}

	void removeAllElements();
	void removeElementType(UIElementType type);

	//Get Methods for rendering and updating
	std::vector<std::vector<std::vector<std::shared_ptr<ManagedUIElement> > > > & getElementGrid() { return m_gridLocations; } //not a const reference as we need the ability to change the state of UIElements in teh grid
	//std::vector<std::shared_ptr<UIElement> > const& getRenderElements() { return m_renderElements; }
	std::vector<std::shared_ptr<ManagedUIElement> > & getActionElements() { return m_actionElements; }

	//Methods for interactions of UIElements with the Mouse
	void updateScreenSize(winrt::Windows::Foundation::Size newWindowSize);
	winrt::Windows::Foundation::Size getScreenSize() { return m_windowSize; }
	void updateGridSquareElements(InputState* input);

	template <typename T>
	std::shared_ptr<T> getElement(std::wstring name)
	{
		//This method is for getting appropriate type pointers for the UIElement that we
		//want. If a UIElement of type T exists in the appropriate vecctor of the 
		//element map with the name "name" then this method returns a pointer to it 
		//of the appropriate type. This allows the user to seemlessly dereference the 
		//pointer inline and get appropriate methods (i.e. getElement<TextBox>("My Text Box")->updateText(L"new text")
		//would allow us to update the text inside of a text box without the need to 
		//manually cast to a TextBox* pointer).
		static_assert(std::is_base_of<UIElement, T>::value, "getElement<T>() must use a class that inherits from the UIElement class"); //this method only works classes in the UIElement heirarchy

		UIElementType type = type_to_UIElementType<T>();
		auto it = findElementByName(m_uiElements.at(type), name);
		if (it != m_uiElements.at(type).end())
		{
			return std::dynamic_pointer_cast<T>(it->get()->element);
		}
		else
		{
			//If the wanted UIElement isn't found we instead return a dummy element
		    //and kick up an error message. This way we can let the user know that the
		    //element wasn't updated as intended without causing an null ptr errors. Whatever
			//changes the user tries and makes to the dummy variable will work, but nothing
			//will display on screen.
			std::wstring message = L"Couldn't find UIElement with name " + name;
			OutputDebugString(&message[0]);
			return createDummyUIElement<T>();
		}
	}

	//Methods for Handling Alerts
	void checkAlerts();
	std::vector<std::shared_ptr<ManagedUIElement>> removeAlerts();
	void overwriteAlerts(std::vector<std::shared_ptr<ManagedUIElement>> const& alerts);

	//Automatic Text Updating Methods
	void checkForTextResize();
	std::vector<UIText*> getResizeText();
	void applyTextResizeUpdates();
	int elementsCurrentlyNeedingTextUpdate() { return m_updateText.size(); }
	void refreshGrid();

	//Get Methods
	std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& getElementsMap() { return m_uiElements; } //useful for rendering elements

private:
	//Data Structures
	std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > m_uiElements;
	std::vector<std::vector<std::vector<std::shared_ptr<ManagedUIElement> > > > m_gridLocations; //splits the screen into a grid and keeps track of which elements are in which sector, useful for mouse hover detection
	std::vector<std::shared_ptr<ManagedUIElement> > m_actionElements; //Any UI Elements that have been interacted with and require the current mode to carry out some action will be added to this vector
	std::vector<std::shared_ptr<UIElement>> m_updateText; //An array of elements that currently require text dimension info from the Renderer class

	winrt::Windows::Foundation::Size m_windowSize; //Keeps track of the current size of the window. UIElements have dimensions that are relative to the window size
	static std::vector<UIElementType> m_textUpdateElements;
	long long m_alertTimer = 2000; //The amount of time (in milliseconds) that alerts remain on screen before disappearing

	void populateGridLocations(std::shared_ptr<ManagedUIElement> managedElement);

	std::vector<std::shared_ptr<ManagedUIElement> >::iterator findElementByName(std::vector<std::shared_ptr<ManagedUIElement> > & vec, std::wstring name);

	template <typename T>
	std::shared_ptr<T> createDummyUIElement()
	{
		//This method is used to prevent null ptr exceptions from happening when we try and
		//access a UIElement that's not included in the manager

		//NOTE: This only works if the supplied class T has a default constructor, otherwise
		//there will be an error when trying to create the shared pointer
		return std::make_shared<T>();
	}

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
	UIElementType type_to_UIElementType<TextButton>() { return UIElementType::TEXT_BUTTON; }

	template<>
	UIElementType type_to_UIElementType<ArrowButton>() { return UIElementType::ARROW_BUTTON; }

	template<>
	UIElementType type_to_UIElementType<TextBox>() { return UIElementType::TEXT_BOX; }

	template<>
	UIElementType type_to_UIElementType<PartialScrollingTextBox>() { return UIElementType::PARTIAL_SCROLLING_TEXT_BOX; }

	template<>
	UIElementType type_to_UIElementType<FullScrollingTextBox>() { return UIElementType::FULL_SCROLLING_TEXT_BOX; }

	template<>
	UIElementType type_to_UIElementType<TextOverlay>() { return UIElementType::TEXT_OVERLAY; }

	template<>
	UIElementType type_to_UIElementType<HighlightableTextOverlay>() { return UIElementType::HIGHLIGHTABLE_TEXT_OVERLAY; }

	template<>
	UIElementType type_to_UIElementType<DropDownMenu>() { return UIElementType::DROP_DOWN_MENU; }

	template<>
	UIElementType type_to_UIElementType<Graph>() { return UIElementType::GRAPH; }

	template<>
	UIElementType type_to_UIElementType<Line>() { return UIElementType::LINE; }

	template<>
	UIElementType type_to_UIElementType<Alert>() { return UIElementType::ALERT; }
};