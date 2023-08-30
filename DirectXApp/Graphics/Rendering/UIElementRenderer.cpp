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
        case UIShapeColor::UnpressedButton:
            color = { 0.9, 0.9, 0.9, 1 };
            break;
        case UIShapeColor::PressedButton:
            color = { 0.33, 0.33, 0.33, 1 };
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

    //create some default text formats to use
    createTextFormats();

    //create a defualt text format, this is used to be overwritten by 
    //colored text
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
    winrt::check_hresult(m_defaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_defaultTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    //Create a default brush
    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0, 1.0),
            m_defaultBrush.put()
        )
    );

    m_textLayout = nullptr; //initialize the text layout to null
}

void UIElementRenderer::createTextFormats()
{
    //Create nine default text formats, one for each of the possible justification combinations
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    for (int i = 0; i < static_cast<int>(UITextJustification::END); i++)
    {
        m_textFormats.push_back(nullptr);
        winrt::check_hresult(
            dwriteFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_LIGHT,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                1.0, //The font size doesn't matter as it gets overwritten before rendering
                L"en-us",
                m_textFormats.back().put()
            )
        );

        switch (static_cast<UITextJustification>(i))
        {
        case UITextJustification::UpperLeft:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
            break;
        case UITextJustification::UpperCenter:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
            break;
        case UITextJustification::UpperRight:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
            break;
        case UITextJustification::CenterLeft:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
            break;
        case UITextJustification::CenterCenter:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
            break;
        case UITextJustification::CenterRight:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
            break;
        case UITextJustification::LowerLeft:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
            break;
        case UITextJustification::LowerCenter:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
            break;
        case UITextJustification::LowerRight:
            winrt::check_hresult(m_textFormats.back()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
            winrt::check_hresult(m_textFormats.back()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
            break;
        }
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
            RenderOrder renderOrder = static_cast<RenderOrder>(j);
            int renderLength = uiElements[i]->getRenderVectorSize(renderOrder);
            if (renderOrder == RenderOrder::ElementText)
            {
                //This is one of the text sections so render text
                for (int k = 0; k < renderLength; k++) renderText((UIText*)uiElements[i]->render(renderOrder, k));
            }
            else if (renderOrder == RenderOrder::TextOverlay) continue; //the text overlay for all objects is rendered at the end
            else
            {
                for (int k = 0; k < renderLength; k++) renderShape((UIShape*)uiElements[i]->render(renderOrder, k));
            }
        }
    }

    //After all shapes and element text has been rendered, render any overlay text. This is typically
    //reserved for things like the title of a page.
    for (int i = 0; i < uiElements.size(); i++)
    {
        int renderLength = uiElements[i]->getRenderVectorSize(RenderOrder::TextOverlay);
        for (int j = 0; j < renderLength; j++) renderText((UIText*)uiElements[i]->render(RenderOrder::TextOverlay, j));
    }
}

void UIElementRenderer::renderShape(const UIShape* shape)
{
    //This mehod renders the given shape.

    //TODO: For now the only shapes I'm dealing with are rectangles. If I end up using
    //any of the other Direct2D shapes like ellipse or triangle then I'll need to update
    //this method
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    switch (shape->m_fillType)
    {
    case UIShapeFillType::NoFill:
        d2dContext->DrawRectangle(shape->m_rectangle, m_shapeColorBrushes[static_cast<int>(shape->m_color)].get(), 1.0f);
        break;
    case UIShapeFillType::Fill:
        d2dContext->FillRectangle(shape->m_rectangle, m_shapeColorBrushes[static_cast<int>(shape->m_color)].get());
        break;
    }
}

void UIElementRenderer::renderText(const UIText* text)
{
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
    m_textLayout = nullptr; //erase whatever settings were put into the layout previously

    //A new text layout is created every time text is rendered
    winrt::check_hresult(
        dwriteFactory->CreateTextLayout(
            &text->message[0],
            text->message.size(),
            m_textFormats[static_cast<int>(text->justification)].get(),
            text->renderArea.x,
            text->renderArea.y,
            m_textLayout.put()
        )
    );

    //After creating the text layout, update colors and font size as necessary
    unsigned int currentLocation = 0;
    for (int i = 0; i < text->colors.size(); i++)
    {
        m_textLayout->SetDrawingEffect(m_textColorBrushes[static_cast<int>(text->colors[i])].get(), { currentLocation,
            (unsigned int)text->colorLocations[i + 1] });

        currentLocation += (unsigned int)text->colorLocations[i + 1];
    }
    m_textLayout->SetFontSize(text->fontSize, { 0, (unsigned int)text->message.length() });

    d2dContext->DrawTextLayout(
        Point2F(text->startLocation.x, text->startLocation.y),
        m_textLayout.get(),
        m_defaultBrush.get(),
        D2D1_DRAW_TEXT_OPTIONS_CLIP //clip any text not inside the target rectangle
    );
}