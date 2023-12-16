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

    //Create solid color brushes for all colors in the UIColor map. The first loop is necessary 
    //to create nullptrs for all possible colors. Since a map is used then there's no way to 
    //iterate through it in the same order as the UIColor enum dictates.
    for (int i = 0; i < static_cast<int>(UIColor::END); i++) m_solidColorBrushes.push_back(nullptr);
    for (int i = 0; i < static_cast<int>(UIColor::END); i++)
    {
        winrt::check_hresult(
            d2dContext->CreateSolidColorBrush(
                m_colors.colors.at(static_cast<UIColor>(i)),
                m_solidColorBrushes[i].put()
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

void UIElementRenderer::setTextLayoutPixels(UIText* text)
{
    //The passed in text element needs to know the full size of it's text layout. To figure this
    //out simply create the text layout and call the GetMetrics() method on it.
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();
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

    //Make sure to update to the correct font size
    m_textLayout->SetFontSize(text->fontSize, { 0, (unsigned int)text->message.length() });

    DWRITE_TEXT_METRICS metrics;
    //DWRITE_OVERHANG_METRICS oMetrics;
    m_textLayout->GetMetrics(&metrics);
    //m_textLayout->GetOverhangMetrics(&oMetrics);

    text->renderDPI.x = metrics.width;
    text->renderDPI.y = metrics.height;
    text->renderLines = metrics.lineCount;
}

void UIElementRenderer::render(std::vector<std::shared_ptr<UIElement> > const& uiElements)
{
    //Renders all UI Elements in the given vector
    for (int i = 0; i < uiElements.size(); i++) renderUIElement(uiElements[i]);
}

void UIElementRenderer::render(std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& managedUIElements)
{
    //This render method takes a reference to the UIElementManager classes ui element map. This gives us a little
    //bit more ease/flexibility than just rendering from a single vector. With this method we have the ability to render
    //UIElements in order of their element type, instead of the order in which they are placed on a page. Certain elements
    //(like drop down boxes) should be rendered after other elements so that their children elements will be displayed
    //on the top level of the screen, ensuring that we can use the scroll box child without any issue.
    for (int i = 0; i < static_cast<int>(UIElementType::END); i++)
    {
        if (static_cast<UIElementType>(i) == UIElementType::DROP_DOWN_MENU) continue;

        auto elements = managedUIElements.at(static_cast<UIElementType>(i));
        for (int j = 0; j < elements.size(); j++) renderUIElement(elements[j]->element);
    }

    //Drop down menus get rendered last, and from top to bottom of the screen. This insures that nothing 
    //covers up their scroll boxes when activated.
    auto drop_downs = managedUIElements.at(UIElementType::DROP_DOWN_MENU);
    for (int i = 0; i < drop_downs.size(); i++) renderUIElement(drop_downs[i]->element);
}

void UIElementRenderer::renderUIElement(std::shared_ptr<UIElement> element)
{
    if (element->getState() & UIElementState::Invisible) return; //invisible elements don't get rendered

    //First, recursively, render all children elements first
    auto children = element->getChildren();
    if (children.size() > 0) render(children);

    if (element->getShape()->m_shapeType != UIShapeType::END) renderShape(element->getShape());
    if (element->getText()->textType != UITextType::END) renderText(element->getText());
}

void UIElementRenderer::renderShape(const UIShape* shape)
{
    //This mehod renders the given shape.

    //TODO: For now the only shapes I'm dealing with are rectangles. If I end up using
    //any of the other Direct2D shapes like ellipse or triangle then I'll need to update
    //this method
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    switch (shape->m_shapeType)
    {
    case UIShapeType::RECTANGLE:
        if (shape->m_fillType == UIShapeFillType::NoFill) d2dContext->DrawRectangle(shape->m_rectangle, m_solidColorBrushes[static_cast<int>(shape->m_color)].get(), 1.0f);
        else d2dContext->FillRectangle(shape->m_rectangle, m_solidColorBrushes[static_cast<int>(shape->m_color)].get());
        break;
    case UIShapeType::LINE:
        d2dContext->DrawLine({ shape->m_rectangle.left, shape->m_rectangle.top }, { shape->m_rectangle.right, shape->m_rectangle.bottom }, m_solidColorBrushes[static_cast<int>(shape->m_color)].get(), shape->m_lineWidth);
        break;
    case UIShapeType::ELLIPSE:
        const D2D1_ELLIPSE ell = { {shape->m_rectangle.left, shape->m_rectangle.top}, shape->m_rectangle.right, shape->m_rectangle.bottom }; //create an ellipse with the variables saved in the shape's rectangle struct
        if (shape->m_fillType == UIShapeFillType::NoFill) d2dContext->DrawEllipse(ell, m_solidColorBrushes[static_cast<int>(shape->m_color)].get(), shape->m_lineWidth);
        else d2dContext->FillEllipse(ell, m_solidColorBrushes[static_cast<int>(shape->m_color)].get());
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
        m_textLayout->SetDrawingEffect(m_solidColorBrushes[static_cast<int>(text->colors[i])].get(), { currentLocation,
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

D2D1::ColorF UIElementRenderer::getClearColor(UIColor backgroundColor)
{
    return m_colors.colors.at(backgroundColor);
}