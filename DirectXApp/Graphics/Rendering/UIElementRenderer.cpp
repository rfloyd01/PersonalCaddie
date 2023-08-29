#include "pch.h"
#include "UIElementRenderer.h"
#include "UIConstants.h"

using namespace D2D1;
using namespace winrt::Windows::ApplicationModel;

UIElementRenderer::UIElementRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources) :
	m_deviceResources(deviceResources)
{
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    //Create solid color brushes for all shape and text colors defined in the
    //UIShape and UIText classes
    for (int i = 0; i < static_cast<int>(UITextColor::END); i++)
    {
        D2D1::ColorF color(1, 1, 1, 1);
        switch (static_cast<UITextColor>(i))
        {
        case UITextColor::Black:
            color = { 0, 0, 0, 1 };
            break;
        case UITextColor::Blue:
            color = { 0, 0, 1, 1 };
            break;
        case UITextColor::Green:
            color = { 0, 1, 0, 1 };
            break;
        case UITextColor::Red:
            color = { 1, 0, 0, 1 };
            break;
        case UITextColor::White:
            color = { 1, 1, 1, 1 };
            break;
        }

        m_textColorBrushes.push_back(nullptr);
        winrt::check_hresult(
            d2dContext->CreateSolidColorBrush(
                color,
                m_textColorBrushes.back().put()
            )
        );
    }
    
    for (int i = 0; i < static_cast<int>(UIShapeColor::END); i++)
    {
        D2D1::ColorF color(1, 1, 1, 1);
        switch (static_cast<UIShapeColor>(i))
        {
        case UIShapeColor::Black:
            color = { 0, 0, 0, 1 };
            break;
        case UIShapeColor::Blue:
            color = { 0, 0, 1, 1 };
            break;
        case UIShapeColor::Green:
            color = { 0, 1, 0, 1 };
            break;
        case UIShapeColor::Red:
            color = { 1, 0, 0, 1 };
            break;
        case UIShapeColor::White:
            color = { 1, 1, 1, 1 };
            break;
        }

        m_shapeColorBrushes.push_back(nullptr);
        winrt::check_hresult(
            d2dContext->CreateSolidColorBrush(
                color,
                m_shapeColorBrushes.back().put()
            )
        );
    }
   
}

void UIElementRenderer::render(std::vector<std::shared_ptr<UIElement> > const& uiElements)
{
    //The input vector contains all of the UI Elements to be rendered from the current mode. The order in which 
    //the elements get rendered is important. Some elements (like scroll boxes) need to have shapes rendered over
    //their text to give the illusion that all text is contained inside the box.

    UIShape* shape = nullptr;
    UIText* text = nullptr;

    for (int i = 0; i < uiElements.size(); i++)
    {
        for (int j = 0; j < static_cast<int>(RenderOrder::End); j++)
        {
            int renderLength = uiElements[i]->getRenderVectorSize(static_cast<RenderOrder>(j));
            for (int k = 0; k < renderLength; k++)
            {

            }
        }
    }
    
}