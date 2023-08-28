#include "pch.h"
#include "TextRenderer.h"
#include "UIConstants.h"

using namespace D2D1;
using namespace winrt::Windows::ApplicationModel;

TextRenderer::TextRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources) :
	m_deviceResources(deviceResources)
{
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    //create a format, layout for each text type
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        m_textFormats.push_back(nullptr);
        m_textLayouts.push_back(nullptr);

        //save values from UIConstants page into vectors to make it easier to use
        //for loops to modify TextFormats and TextLayouts in the future
        TextType tt = static_cast<TextType>(i);
        switch (tt)
        {
        case TextType::TITLE:
            m_fontSizeRatios.push_back(UIConstants::TitleTextPointSize);
            m_renderBorderRatios.push_back({ {UIConstants::TitleRectangleX0, UIConstants::TitleRectangleY0 }, { UIConstants::TitleRectangleX1, UIConstants::TitleRectangleY1 } });
            break;
        case TextType::SUB_TITLE:
            m_fontSizeRatios.push_back(UIConstants::SubTitleTextPointSize);
            m_renderBorderRatios.push_back({ { UIConstants::SubTitleRectangleX0, UIConstants::SubTitleRectangleY0 }, { UIConstants::SubTitleRectangleX1, UIConstants::SubTitleRectangleY1 } });
            break;
        case TextType::SENSOR_INFO:
            m_fontSizeRatios.push_back(UIConstants::SensorInfoTextPointSize);
            m_renderBorderRatios.push_back({ {UIConstants::SensorInfoRectangleX0, UIConstants::SensorInfoRectangleY0 }, { UIConstants::SensorInfoRectangleX1, UIConstants::SensorInfoRectangleY1 } });
            break;
        case TextType::FOOT_NOTE:
            m_fontSizeRatios.push_back(UIConstants::FootNoteTextPointSize);
            m_renderBorderRatios.push_back({ {UIConstants::FootNoteRectangleX0, UIConstants::FootNoteRectangleY0 }, { UIConstants::FootNoteRectangleX1, UIConstants::FootNoteRectangleY1 } });
            break;
        case TextType::ALERT:
            m_fontSizeRatios.push_back(UIConstants::AlertTextPointSize);
            m_renderBorderRatios.push_back({ {UIConstants::AlertRectangleX0, UIConstants::AlertRectangleY0 }, { UIConstants::AlertRectangleX1, UIConstants::AlertRectangleY1 } });
            break;
        case TextType::BODY:
        default:
            m_fontSizeRatios.push_back(UIConstants::BodyTextPointSize);
            m_renderBorderRatios.push_back({ {UIConstants::BodyRectangleX0, UIConstants::BodyRectangleY0 }, { UIConstants::BodyRectangleX1, UIConstants::BodyRectangleY1 } });
            break;
        }

        winrt::check_hresult(
            dwriteFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_LIGHT,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                m_fontSizeRatios[i],
                L"en-us",
                m_textFormats[i].put()
            )
        );

        //Set the appropriate text/paragraph alignments for each text type
        SetTextRegionAlignments(tt);
        
        //default initialize the textLayouts, textLenghts and textStartLocations as these will
        //get changed as soon as a new mode get's loaded, including the starting mode
        winrt::check_hresult(dwriteFactory->CreateTextLayout(L"", 0, m_textFormats[i].get(),
            0, 0, m_textLayouts[i].put()));

        m_textLengths.push_back(0);
        m_startLocations.push_back({ 0, 0 });

        //We also need to create a vector to hold text brushes for each TextType
        std::vector<winrt::com_ptr<ID2D1SolidColorBrush> > textTypeBrushes;
        m_textColorBrushes.push_back(textTypeBrushes);

        //Finally, create a default brush (this can be overridden at any point, but we need 
        //something that isn't null to start off)
        auto d2dContext = m_deviceResources->GetD2DDeviceContext();
        winrt::check_hresult(
            d2dContext->CreateSolidColorBrush(
                D2D1::ColorF(1.0, 1.0, 1.0, 1.0),
                m_defaultBrush.put()
            )
        );
    }
}

void TextRenderer::SetTextRegionAlignments(TextType tt)
{
    int i = static_cast<int>(tt);
    switch (tt)
    {
    case TextType::TITLE:
        //Title text starts in the center of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
        break;
    case TextType::SENSOR_INFO:
        //Sensor Info text starts at the bottom left of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        break;
    case TextType::SUB_TITLE:
        //Subtitle text starts at the top center of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        break;
    case TextType::ALERT:
        //Alert text starts at the bottom center of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        break;
    case TextType::FOOT_NOTE:
        //Foot note text starts at the bottom right of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        break;
    case TextType::BODY:
    default:
        //Body and default text starts at the top left of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        break;
    }
}

void TextRenderer::CreateDeviceDependentResources()
{
    // auto location = Package::Current().InstalledLocation();
    //winrt::hstring path{ location.Path() + L"\\Assets\\windows-sdk.png" };

    //auto wicFactory = m_deviceResources->GetWicImagingFactory();

    //winrt::com_ptr<IWICBitmapDecoder> wicBitmapDecoder;
    //winrt::check_hresult(
    //    wicFactory->CreateDecoderFromFilename(
    //        path.c_str(),
    //        nullptr,
    //        GENERIC_READ,
    //        WICDecodeMetadataCacheOnDemand,
    //        wicBitmapDecoder.put()
    //    )
    //);

    //winrt::com_ptr<IWICBitmapFrameDecode> wicBitmapFrame;
    //winrt::check_hresult(
    //    wicBitmapDecoder->GetFrame(0, wicBitmapFrame.put())
    //);

    //winrt::com_ptr<IWICFormatConverter> wicFormatConverter;
    //winrt::check_hresult(
    //    wicFactory->CreateFormatConverter(wicFormatConverter.put())
    //);

    //winrt::check_hresult(
    //    wicFormatConverter->Initialize(
    //        wicBitmapFrame.get(),
    //        GUID_WICPixelFormat32bppPBGRA,
    //        WICBitmapDitherTypeNone,
    //        nullptr,
    //        0.0,
    //        WICBitmapPaletteTypeCustom  // The BGRA format has no palette so this value is ignored.
    //    )
    //);

    //double dpiX = 96.0f;
    //double dpiY = 96.0f;
    //winrt::check_hresult(
    //    wicFormatConverter->GetResolution(&dpiX, &dpiY)
    //);

    //auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    //// Create D2D Resources
    //winrt::check_hresult(
    //    d2dContext->CreateBitmapFromWicBitmap(
    //        wicFormatConverter.get(),
    //        BitmapProperties(
    //            PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
    //            static_cast<float>(dpiX),
    //            static_cast<float>(dpiY)
    //        ),
    //        m_logoBitmap.put()
    //    )
    //);

    //m_logoSize = m_logoBitmap->GetSize();

    /*winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            m_textBrush.put()
        )
    );*/
}

void TextRenderer::CreateWindowSizeDependentResources()
{
    auto windowBounds = m_deviceResources->GetLogicalSize();

    //When the size of the window changes we reset the font size for all
    //text so that it matches the new size of the window. We also need to update
    //the sizes of the rendering rectangle fo each TextLayout as well as the 
    //start location for where each TextLayout should be rendered

    //Alter the font sizes and rendering rectangle sizes to fit the current windows (font's are only based on window height for now)
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        m_startLocations[i].first = m_renderBorderRatios[i].first.first * windowBounds.Width;
        m_startLocations[i].second = m_renderBorderRatios[i].first.second * windowBounds.Height;
        m_textLayouts[i]->SetMaxWidth((m_renderBorderRatios[i].second.first - m_renderBorderRatios[i].first.first) * windowBounds.Width);
        m_textLayouts[i]->SetMaxHeight((m_renderBorderRatios[i].second.second - m_renderBorderRatios[i].first.second) * windowBounds.Height);
        UpdateTextTypeFontSize(static_cast<TextType>(i));
    }
}

void TextRenderer::DeleteTextBrushes()
{
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        //TODO: will this free up the resources held by the com-ptrs?
        m_textColorBrushes[i].clear(); //just clear out all existing brushes
    }
}

void TextRenderer::UpdateText(Text const& text)
{
    //New text requires the creation of new textLayouts objects. I don't know how resource
    //intensive this is but I'd prefer to only create new Layouts if absolutely necessary.
    //Because of this, we don't update all text on screen in one shot, only individual text
    //snippets are updated at a time (this way we don't recreate resources for text not 
    //getting updated).

    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    int index = static_cast<int>(text.textType);
    auto originalWidth = m_textLayouts[index]->GetMaxWidth();
    auto originalHeight = m_textLayouts[index]->GetMaxHeight();

    winrt::check_hresult(
        dwriteFactory->CreateTextLayout(
            &text.message[0],
            text.message.size(),
            m_textFormats[static_cast<int>(text.textType)].get(),
            originalWidth,
            originalHeight,
            m_textLayouts[static_cast<int>(text.textType)].put()
        )
    );

    //Update the value in the m_textLengths vector and then make sure 
    //that any styling gets applied to the entirety of the new message
    m_textLengths[index] = text.message.length();
    UpdateTextTypeFontSize(text.textType);

    //After updating the physical text, we update the colors (if applicable). If the passed in string is empty
    //or the passed in colors array isn't empty, we delete any existing text brushes for the given TextType.
    //Furthermore, if the passed in colors array isn't empty, we create new brushes as specified.
    if (text.message.size() == 0 || text.colors.size() > 0) DeleteTextTypeBrushes(text.textType);
    if (text.colors.size() > 0) CreateTextTypeBrushes(text);
}

void TextRenderer::UpdateTextTypeFontSize(TextType tt)
{
    //Since the font sizes used are based off of the window size we won't know the true
    //size of the fonts until runtime. Furthermore, to apply the appropriate font size to 
    //a TextLayout we need to choose which letters to apply it to. This means that changing
    //the text by creating a new layout will default back to an unknown font size. This 
    //means that every time we update the text on screen we need to call this method to
    //make sure everything is rendered at the appropriate size.
    auto windowBounds = m_deviceResources->GetLogicalSize();

    int i = static_cast<int>(tt);
    m_textLayouts[i]->SetFontSize(m_fontSizeRatios[i] * windowBounds.Height, { 0, m_textLengths[i] });
}

void TextRenderer::Render(_In_ std::shared_ptr<ModeScreen> const& mode)
{
    //iterate through all the text types and render the TextLayout for
    //each
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        d2dContext->DrawTextLayout(
            Point2F(m_startLocations[i].first, m_startLocations[i].second),
            m_textLayouts[i].get(),
            m_defaultBrush.get()
        );
    }
}

void TextRenderer::ReleaseDeviceDependentResources()
{
    //TODO: need to update this to apply to all textBrushes in the vector
    //m_textBrush = nullptr;
    //m_logoBitmap = nullptr;
}

void TextRenderer::CreateTextTypeBrushes(Text const& text)
{
    //This method creates text brushes for a single TextType based on the text color
    //split passed in.
    
    //TODO: should consider replacing the above CreateTextBrushes() method with this one
    //and just call it as many times as necessary when initializing a new mode

    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    int i = static_cast<int>(text.textType);
    unsigned int current_location = 0;

    for (int j = 0; j < text.colors.size(); j++)
    {
        TextColor c = text.colors[j];
        m_textColorBrushes[i].push_back(nullptr);
        winrt::check_hresult(
            d2dContext->CreateSolidColorBrush(
                D2D1::ColorF(c.r, c.g, c.b, c.a),
                m_textColorBrushes[i].back().put()
            )
        );

        //After creating a new brush, apply it to the appropriate characters of the text. The locations vector
        //of the TextTypeColorSplit struct will always have a length that's 1 greater than the colors vector,
        //so accessing elements with j+1 is safe.
        m_textLayouts[i]->SetDrawingEffect(m_textColorBrushes[i].back().get(), { current_location,
            (unsigned int)text.locations[j + 1] });

        current_location += (unsigned int)text.locations[j + 1];
    }
}

void TextRenderer::DeleteTextTypeBrushes(TextType tt)
{
    //Deletes all text brushes (if they exist) for the given TextType
    for (int i = 0; i < m_textColorBrushes[static_cast<int>(tt)].size(); i++) m_textColorBrushes[static_cast<int>(tt)][i] = nullptr;
    m_textColorBrushes[static_cast<int>(tt)].clear();
}