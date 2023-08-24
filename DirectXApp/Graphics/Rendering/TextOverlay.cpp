#include "pch.h"
#include "TextOverlay.h"
#include "UIConstants.h"

using namespace D2D1;
using namespace winrt::Windows::ApplicationModel;

TextOverlay::TextOverlay(
	_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources,
	_In_ winrt::hstring const& titleHeader,
	_In_ winrt::hstring const& titleBody
    ) :
	m_deviceResources(deviceResources),
	m_titleHeader(titleHeader),
	m_titleBody(titleBody)
{
    m_showTitle = true;
    m_titleBodyVerticalOffset = UIConstants::Margin;
    m_logoSize = D2D1::SizeF(0.0f, 0.0f);

    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    //create a format, layout for each text type
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        m_textFormats.push_back(nullptr);
        m_textLayouts.push_back(nullptr);

        TextType tt = static_cast<TextType>(i);
        switch (tt)
        {
        case TextType::TITLE:
            m_fontSizeRatios.push_back(UIConstants::TitleTextPointSize);
            break;
        case TextType::SUB_TITLE:
            m_fontSizeRatios.push_back(UIConstants::SubTitleTextPointSize);
            break;
        case TextType::SENSOR_INFO:
            m_fontSizeRatios.push_back(UIConstants::SensorInfoTextPointSize);
            break;
        case TextType::FOOT_NOTE:
            m_fontSizeRatios.push_back(UIConstants::FootNoteTextPointSize);
            break;
        case TextType::ALERT:
            m_fontSizeRatios.push_back(UIConstants::AlertTextPointSize);
            break;
        case TextType::BODY:
        default:
            m_fontSizeRatios.push_back(UIConstants::BodyTextPointSize);
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
    }
}

void TextOverlay::SetTextRegionAlignments(TextType tt)
{
    int i = static_cast<int>(tt);
    switch (tt)
    {
    case TextType::TITLE:
    case TextType::SUB_TITLE:
        //Title and sub-title text starts in the center of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
        break;
    case TextType::SENSOR_INFO:
        //Sensor Info text starts at the bottom left of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        break;
    case TextType::ALERT:
        //Alert text starts at the top center of the render box
        winrt::check_hresult(m_textFormats[i]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        winrt::check_hresult(m_textFormats[i]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
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

void TextOverlay::CreateDeviceDependentResources()
{
     auto location = Package::Current().InstalledLocation();
    winrt::hstring path{ location.Path() + L"\\Assets\\windows-sdk.png" };

    auto wicFactory = m_deviceResources->GetWicImagingFactory();

    winrt::com_ptr<IWICBitmapDecoder> wicBitmapDecoder;
    winrt::check_hresult(
        wicFactory->CreateDecoderFromFilename(
            path.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            wicBitmapDecoder.put()
        )
    );

    winrt::com_ptr<IWICBitmapFrameDecode> wicBitmapFrame;
    winrt::check_hresult(
        wicBitmapDecoder->GetFrame(0, wicBitmapFrame.put())
    );

    winrt::com_ptr<IWICFormatConverter> wicFormatConverter;
    winrt::check_hresult(
        wicFactory->CreateFormatConverter(wicFormatConverter.put())
    );

    winrt::check_hresult(
        wicFormatConverter->Initialize(
            wicBitmapFrame.get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom  // The BGRA format has no palette so this value is ignored.
        )
    );

    double dpiX = 96.0f;
    double dpiY = 96.0f;
    winrt::check_hresult(
        wicFormatConverter->GetResolution(&dpiX, &dpiY)
    );

    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    // Create D2D Resources
    winrt::check_hresult(
        d2dContext->CreateBitmapFromWicBitmap(
            wicFormatConverter.get(),
            BitmapProperties(
                PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                static_cast<float>(dpiX),
                static_cast<float>(dpiY)
            ),
            m_logoBitmap.put()
        )
    );

    m_logoSize = m_logoBitmap->GetSize();

    /*winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            m_textBrush.put()
        )
    );*/
}

void TextOverlay::CreateWindowSizeDependentResources()
{
    auto windowBounds = m_deviceResources->GetLogicalSize();

    //When the size of the window changes we reset the font size for all
    //text so that it matches the new size of the window. We also need to update
    //the sizes of the rendering rectangle fo each TextLayout as well as the 
    //start location for where each TextLayout should be rendered

    //First update the rendering start locations
    m_startLocations[static_cast<int>(TextType::TITLE)].first = UIConstants::WidthMargin * windowBounds.Width;
    m_startLocations[static_cast<int>(TextType::TITLE)].second = UIConstants::HeightMargin * windowBounds.Height;

    m_startLocations[static_cast<int>(TextType::SUB_TITLE)].first = UIConstants::WidthMargin * windowBounds.Width;
    m_startLocations[static_cast<int>(TextType::SUB_TITLE)].second = m_startLocations[static_cast<int>(TextType::TITLE)].second + windowBounds.Height * (UIConstants::HeightMargin + UIConstants::TitleRectangleHeight);

    m_startLocations[static_cast<int>(TextType::BODY)].first = UIConstants::WidthMargin * windowBounds.Width;
    m_startLocations[static_cast<int>(TextType::BODY)].second = m_startLocations[static_cast<int>(TextType::SUB_TITLE)].second + windowBounds.Height * (UIConstants::HeightMargin + UIConstants::SubTitleRectangleHeight);

    m_startLocations[static_cast<int>(TextType::SENSOR_INFO)].first = UIConstants::WidthMargin * windowBounds.Width;
    m_startLocations[static_cast<int>(TextType::SENSOR_INFO)].second = m_startLocations[static_cast<int>(TextType::BODY)].second + windowBounds.Height * (UIConstants::HeightMargin + UIConstants::BodyRectangleHeight);

    m_startLocations[static_cast<int>(TextType::ALERT)].first = m_startLocations[static_cast<int>(TextType::SENSOR_INFO)].first + windowBounds.Width * (UIConstants::WidthMargin + UIConstants::SensorInfoRectangleWidth);
    m_startLocations[static_cast<int>(TextType::ALERT)].second = m_startLocations[static_cast<int>(TextType::SENSOR_INFO)].second;

    m_startLocations[static_cast<int>(TextType::FOOT_NOTE)].first = m_startLocations[static_cast<int>(TextType::ALERT)].first + windowBounds.Width * (UIConstants::WidthMargin + UIConstants::AlertRectangleWidth);
    m_startLocations[static_cast<int>(TextType::FOOT_NOTE)].second = m_startLocations[static_cast<int>(TextType::ALERT)].second;

    //Alter the font sizes to fit the current window, font's are only based on window height for now
    m_textLayouts[static_cast<int>(TextType::TITLE)]->SetFontSize(UIConstants::TitleTextPointSize * windowBounds.Height, {0, m_textLengths[static_cast<int>(TextType::TITLE)]});
    m_textLayouts[static_cast<int>(TextType::SUB_TITLE)]->SetFontSize(UIConstants::SubTitleTextPointSize * windowBounds.Height, { 0, m_textLengths[static_cast<int>(TextType::SUB_TITLE)] });
    m_textLayouts[static_cast<int>(TextType::BODY)]->SetFontSize(UIConstants::BodyTextPointSize * windowBounds.Height, { 0, m_textLengths[static_cast<int>(TextType::BODY)] });
    m_textLayouts[static_cast<int>(TextType::SENSOR_INFO)]->SetFontSize(UIConstants::SensorInfoTextPointSize * windowBounds.Height, { 0, m_textLengths[static_cast<int>(TextType::SENSOR_INFO)] });
    m_textLayouts[static_cast<int>(TextType::FOOT_NOTE)]->SetFontSize(UIConstants::FootNoteTextPointSize * windowBounds.Height, { 0, m_textLengths[static_cast<int>(TextType::FOOT_NOTE)] });
    m_textLayouts[static_cast<int>(TextType::ALERT)]->SetFontSize(UIConstants::AlertTextPointSize * windowBounds.Height, { 0, m_textLengths[static_cast<int>(TextType::ALERT)] });

    //Alter the rendring rectangles for each text type
    m_textLayouts[static_cast<int>(TextType::TITLE)]->SetMaxWidth(UIConstants::TitleRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::TITLE)]->SetMaxHeight(UIConstants::TitleRectangleHeight * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::SUB_TITLE)]->SetMaxWidth(UIConstants::SubTitleRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::SUB_TITLE)]->SetMaxHeight(UIConstants::SubTitleRectangleHeight * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::BODY)]->SetMaxWidth(UIConstants::BodyRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::BODY)]->SetMaxHeight(UIConstants::BodyRectangleHeight * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::SENSOR_INFO)]->SetMaxWidth(UIConstants::SensorInfoRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::SENSOR_INFO)]->SetMaxHeight(UIConstants::SensorInfoRectangleHeight * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::FOOT_NOTE)]->SetMaxWidth(UIConstants::FootNoteRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::FOOT_NOTE)]->SetMaxHeight(UIConstants::FootNoteRectangleHeight * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::ALERT)]->SetMaxWidth(UIConstants::AlertRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::ALERT)]->SetMaxHeight(UIConstants::AlertRectangleHeight * windowBounds.Height);
}

void TextOverlay::UpdateTextTypeMessage(TextType tt, std::wstring const& new_message)
{
    //This method gets called either when a new mode is first entered, or, when 
    //a new part of the mode is entered requiring different rendering text. New text
    //requires the creation of new textLayouts objects. I don't know how resource
    //intensive this is but I'd prefer to only create new Layouts if absolutely necessary.
    //Because of this, we don't update all text, only individual text types for the 
    //current mode. This method is only for updating the physical message of the text,
    //none of the formatting gets changed

    if (new_message.size() == 0) return;
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    auto originalWidth = m_textLayouts[static_cast<int>(tt)]->GetMaxWidth();
    auto originalHeight = m_textLayouts[static_cast<int>(tt)]->GetMaxHeight();

    winrt::check_hresult(
        dwriteFactory->CreateTextLayout(
            &new_message[0],
            new_message.size(),
            m_textFormats[static_cast<int>(tt)].get(),
            originalWidth,
            originalHeight,
            m_textLayouts[static_cast<int>(tt)].put()
        )
    );

    //Update the value in the m_textLengths vector and then make sure 
    //that any styling gets applied to the entirety of the new message
    m_textLengths[static_cast<int>(tt)] = new_message.length();
    UpdateTextTypeFontSize(tt);
}

void TextOverlay::UpdateTextTypeFontSize(TextType tt)
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

void TextOverlay::Render(_In_ std::shared_ptr<ModeScreen> const& mode)
{
    //iterate through all the text types and render the TextLayout for
    //each
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    //TODO: Need to figure out a better way to handle colors
    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0),
            m_textBrush.put()
        )
    );
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        d2dContext->DrawTextLayout(
            Point2F(m_startLocations[i].first, m_startLocations[i].second),
            m_textLayouts[i].get(),
            m_textBrush.get()
        );
    }

    //auto dwriteFactory = m_deviceResources->GetDWriteFactory();
    //m_testTextLayout = nullptr;
    //winrt::check_hresult(dwriteFactory->CreateTextLayout(
    //    L"Test Text",
    //    10,
    //    m_textFormats[0].get(),
    //    100.0,
    //    100.0,
    //    m_testTextLayout.put()
    //));

    //d2dContext->DrawTextLayout(
    //    //Point2F(m_startLocations[i].first, m_startLocations[i].second),
    //    Point2F(0, 0),
    //    m_testTextLayout.get(),
    //    m_textBrush.get()
    //);
}

void TextOverlay::ReleaseDeviceDependentResources()
{
    m_textBrush = nullptr;
    m_logoBitmap = nullptr;
}