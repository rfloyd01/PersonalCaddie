#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Graphics/Objects/2D/UIElement.h"
#include "Modes/ModeScreen.h"

#include <string>
#include <map>

class UIElementRenderer
{
public:
    UIElementRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);
    UIElementRenderer(UIElementRenderer const&) = delete;
	void operator=(UIElementRenderer const&) = delete;

    void render(std::vector<std::shared_ptr<UIElement> > const& uiElements);
    void render(std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& managedUIElements);

    void setTextLayoutPixels(UIText* text);

    D2D1::ColorF getClearColor(UIColor backgroundColor);

private:
    void createTextFormats();

    void renderUIElement(std::shared_ptr<UIElement> element);
    void renderShape(const UIShape* shape);
    void renderText(const UIText* text);

    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources>                     m_deviceResources;

    //Unlike when I created the TextRenderer class, the UIElementRenderer class only has a single
    //text layout. Every time a group of text is rendered on the screen a new layout will be created
    //and overwrite the same IDWriteTextLayout pointer. This will be inefficient but will get rid of 
    //the need of trying to keep track of which format belongs to which text
    winrt::com_ptr<IDWriteTextFormat>                         m_defaultTextFormat;
    std::vector<winrt::com_ptr<IDWriteTextFormat> >           m_textFormats;
    winrt::com_ptr<IDWriteTextLayout>                         m_textLayout;

    winrt::com_ptr<ID2D1SolidColorBrush>                      m_defaultBrush;
    std::vector<winrt::com_ptr<ID2D1SolidColorBrush> >        m_solidColorBrushes;

    UIElementColors                                           m_colors;
};