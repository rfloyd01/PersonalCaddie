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

    //create a defualt text format
    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            0.33,
            L"en-us",
            m_defaultTextFormat.put()
        )
    );
    winrt::check_hresult(m_defaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    winrt::check_hresult(m_defaultTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

    //Create a default brush
    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0, 1.0),
            m_defaultBrush.put()
        )
    );

    m_textLayout = nullptr; //initialize the text layout to null
}

void UIElementRenderer::render(std::vector<std::shared_ptr<UIElement> > const& uiElements)
{
    //The input vector contains all of the UI Elements to be rendered from the current mode. The order in which 
    //the elements get rendered is important. Some elements (like scroll boxes) need to have shapes rendered over
    //their text to give the illusion that all text is contained inside the box.
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    UIShape* shape = nullptr;
    UIText* text = nullptr;

    for (int i = 0; i < uiElements.size(); i++)
    {
        for (int j = 0; j < static_cast<int>(RenderOrder::End); j++)
        {
            RenderOrder renderOrder = static_cast<RenderOrder>(j);
            int renderLength = uiElements[i]->getRenderVectorSize(renderOrder);
            if (renderOrder == RenderOrder::ElementText || renderOrder == RenderOrder::TextOverlay)
            {
                //This is one of the text sections so render text
                for (int k = 0; k < renderLength; k++)
                {
                    text = (UIText*)uiElements[i]->render(renderOrder, k);

                    //A new text layout is create every time text is rendered
                    winrt::check_hresult(
                        dwriteFactory->CreateTextLayout(
                            &text->message[0],
                            text->message.size(),
                            m_defaultTextFormat.get(),
                            text->renderArea.x,
                            text->renderArea.y,
                            m_textLayout.put()
                        )
                    );

                    //After creating the text layout, update colors and font size as necessary
                    unsigned int currentLocation = 0;
                    for (int l = 0; l < text->colors.size(); l++)
                    {
                        m_textLayout->SetDrawingEffect(m_textColorBrushes[static_cast<int>(text->colors[l])].get(), { currentLocation,
                            (unsigned int)text->colorLocations[l + 1] });

                        currentLocation += (unsigned int)text->colorLocations[l + 1];
                    }
                    m_textLayout->SetFontSize(text->fontSize, { 0, (unsigned int)text->message.length() });

                    d2dContext->DrawTextLayout(
                        Point2F(text->startLocation.x, text->startLocation.y),
                        m_textLayout.get(),
                        m_defaultBrush.get(),
                        D2D1_DRAW_TEXT_OPTIONS_CLIP //clip any text not inside the target rectangle
                    );
                }
            }
            else
            {
                //This is one of the shape sections so render shapes
                for (int k = 0; k < renderLength; k++)
                {
                    shape = (UIShape*)uiElements[i]->render(renderOrder, k);
                    switch (shape->m_fillType)
                    {
                    case UIShapeFillType::NoFill:
                        d2dContext->DrawRectangle(shape->m_rectangle, m_shapeColorBrushes[static_cast<int>(shape->m_color)].get(), 2.5f);
                        break;
                    case UIShapeFillType::Fill:
                        d2dContext->FillRectangle(shape->m_rectangle, m_shapeColorBrushes[static_cast<int>(shape->m_color)].get());
                        break;
                    }
                }
            }
        }
    }
    
}