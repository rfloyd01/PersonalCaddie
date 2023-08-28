#include "pch.h"
#include "MenuObjectRenderer.h"

MenuObjectRenderer::MenuObjectRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources) :
	m_deviceResources(deviceResources)
{
	//Unlike the text renderer, the brushes for the menu object renderer are static so we create them
    //all when instantiating the class and save them in a vector. An enum class is used for easily accessing
    //the appropriate brush whenever we need it.
    for (int i = 0; i < static_cast<int>(MenuObjectBrushType::END); i++) m_brushes.push_back(nullptr);

    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0, 0, 0, 1.0),
            m_brushes[static_cast<int>(MenuObjectBrushType::BlackOutline)].put()
        )
    );

    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.75, 0.75, 0.75, 1.0),
            m_brushes[static_cast<int>(MenuObjectBrushType::ButtonNotPressedFill)].put()
        )
    );

    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.5, 0.5, 0.5, 1.0),
            m_brushes[static_cast<int>(MenuObjectBrushType::ButtonPressedFill)].put()
        )
    );

    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0, 1.0),
            m_brushes[static_cast<int>(MenuObjectBrushType::WhiteBackground)].put()
        )
    );
}

void MenuObjectRenderer::ReleaseDeviceDependentResources()
{
    //Delete all brushes
    DeleteBrushes();
}

void MenuObjectRenderer::DeleteBrushes()
{
	//Deletes all of the current paint brushes and clears the vector
	for (int i = 0; i < m_brushes.size(); i++) m_brushes[i] = nullptr;
	m_brushes.clear();
}

void MenuObjectRenderer::CreateWindowSizeDependentResources(_In_ std::vector<std::shared_ptr<MenuObject> > const& menuObjects)
{
    //The size and location of every rectangle that gets rendered is dependent
    //on the current size of the scren. Any time the size of the window changes we 
    //need to update all rectangles accordingly
    auto windowBounds = m_deviceResources->GetLogicalSize();
    m_rectangles.clear();   //clear out all existing render rectangles and start over from scratch
    m_brushIndices.clear(); //we also need to reset the vector holding the appropriate brush for each rectangle to be rendered
    int index = 0;
    
    for (auto&& object : menuObjects)
    {
        //Every menu object is composes of one or more rectangles. Each one of these rectangles
        //needs to be rendered
        auto locations = object->getObjectLocations();
        auto dimensions = object->getDimensions();
        auto states = object->getObjectStates();

        for (int i = 0; i < locations.size(); i++)
        {
            D2D1_RECT_F rect = D2D1::RectF(
                (locations[i].x - dimensions[i].x / 2.0) * windowBounds.Width,
                (locations[i].y - dimensions[i].y / 2.0) * windowBounds.Height,
                (locations[i].x + dimensions[i].x / 2.0) * windowBounds.Width,
                (locations[i].y + dimensions[i].y / 2.0) * windowBounds.Height
            );

            //Choose a brush for the new rectangle based on the current state of the object
            m_brushIndices.push_back(0); //default to the first brush
            updateMenuObjectBrush(index++, states[i]);
        }
    }
}

void MenuObjectRenderer::addMenuObject(std::shared_ptr<MenuObject> menuObject)
{
    //adds a single menu object to be rendered on screen
    auto windowBounds = m_deviceResources->GetLogicalSize();

    //Every menu object is composes of one or more rectangles. Each one of these rectangles
    //needs to be rendered
    auto locations = menuObject->getObjectLocations();
    auto dimensions = menuObject->getDimensions();
    auto states = menuObject->getObjectStates();

    for (int i = 0; i < locations.size(); i++)
    {
        D2D1_RECT_F rect = D2D1::RectF(
            (locations[i].x - dimensions[i].x / 2.0) * windowBounds.Width,
            (locations[i].y - dimensions[i].y / 2.0) * windowBounds.Height,
            (locations[i].x + dimensions[i].x / 2.0) * windowBounds.Width,
            (locations[i].y + dimensions[i].y / 2.0) * windowBounds.Height
        );

        //Choose a brush for the new rectangle based on the current state of the object
        m_brushIndices.push_back(0); //default to the first brush
        updateMenuObjectBrush(m_brushIndices.size() - 1, states[i]);
    }
}

void MenuObjectRenderer::updateMenuObjectBrush(int index, MenuObjectState state)
{
    switch (state)
    {
    case MenuObjectState::PassiveBackground:
        m_brushIndices[index] = static_cast<int>(MenuObjectBrushType::WhiteBackground);
        break;
    case MenuObjectState::NotPressed:
        m_brushIndices[index] = static_cast<int>(MenuObjectBrushType::ButtonNotPressedFill);
        break;
    case MenuObjectState::Pressed:
        m_brushIndices[index] = static_cast<int>(MenuObjectBrushType::ButtonPressedFill);
        break;
    case MenuObjectState::PassiveOutline:
    default:
        m_brushIndices[index] = static_cast<int>(MenuObjectBrushType::BlackOutline);
        break;
    }
}

void MenuObjectRenderer::Render()
{
    //iterate through all the render rectangles and draw each one
    //with the appropriate brush
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    for (int i = 0; i < m_rectangles.size(); i++)
    {
        switch (static_cast<MenuObjectBrushType>(m_brushIndices[i]))
        {
        case MenuObjectBrushType::BlackOutline:
            d2dContext->DrawRectangle(m_rectangles[i], m_brushes[m_brushIndices[i]].get(), 2.5f);
            break;
        default:
            d2dContext->FillRectangle(m_rectangles[i], m_brushes[m_brushIndices[i]].get());
            break;
        }
    }
}