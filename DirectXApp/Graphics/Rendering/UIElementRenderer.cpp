#include "pch.h"
#include "UIElementRenderer.h"
#include "UIConstants.h"

using namespace D2D1;
using namespace winrt::Windows::ApplicationModel;

UIElementRenderer::UIElementRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources) :
	m_deviceResources(deviceResources)
{
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    //Create solid color brushes for all shape and text colors defined in the
    //UIShape and UIText classes
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0, 1.0),
            m_defaultBrush.put()
        )
    );
    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0.0, 0.0, 0.0, 1.0),
            m_menuObjectDefaultBrush.put()
        )
    );
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