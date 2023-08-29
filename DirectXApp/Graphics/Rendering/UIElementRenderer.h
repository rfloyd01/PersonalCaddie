#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Graphics/Objects/2D/UIElement.h"

#include <string>

class UIElementRenderer
{
public:
    UIElementRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);
    UIElementRenderer(UIElementRenderer const&) = delete;
	void operator=(UIElementRenderer const&) = delete;

    void render(std::vector<std::shared_ptr<UIElement> > const& uiElements);

private:
    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources>                                     m_deviceResources;

    //Unlike when I created the TextRenderer class, the UIElementRenderer class only has a single
    //text layout. Every time a group of text is rendered on the screen a new layout will be created
    //and overwrite the same IDWriteTextLayout pointer. This will be inefficient but will get rid of 
    //the need of trying to keep track of which format belongs to which text
    winrt::com_ptr<IDWriteTextFormat>                                        m_defaultTextFormat;
    std::vector<winrt::com_ptr<IDWriteTextFormat> >                          m_textFormats;
    winrt::com_ptr<IDWriteTextLayout>                                        m_textLayout;

    winrt::com_ptr<ID2D1SolidColorBrush>                                     m_defaultBrush;
    std::vector<winrt::com_ptr<ID2D1SolidColorBrush> >                       m_textColorBrushes;
    std::vector<winrt::com_ptr<ID2D1SolidColorBrush> >                       m_shapeColorBrushes;
};