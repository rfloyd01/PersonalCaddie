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

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::HudBodyPointSize,
            L"en-us",
            m_textFormatBody.put()
        )
    );
    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI Symbol",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::HudBodyPointSize,
            L"en-us",
            m_textFormatBodySymbol.put()
        )
    );
    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI Light",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::HudTitleHeaderPointSize,
            L"en-us",
            m_textFormatTitleHeader.put()
        )
    );
    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI Light",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::HudTitleBodyPointSize,
            L"en-us",
            m_textFormatTitleBody.put()
        )
    );

    winrt::check_hresult(m_textFormatBody->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormatBody->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
    winrt::check_hresult(m_textFormatBodySymbol->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormatBodySymbol->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    /*winrt::check_hresult(m_textFormatTitleHeader->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormatTitleHeader->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));*/
    winrt::check_hresult(m_textFormatTitleHeader->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    winrt::check_hresult(m_textFormatTitleHeader->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    winrt::check_hresult(m_textFormatTitleBody->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormatTitleBody->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    //My Variables start
    //create a format and layout for each text type
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        m_textFormats.push_back(nullptr);
        m_textLayouts.push_back(nullptr);
        m_startLocations.push_back({ 0, 0 });
    }

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::TitleTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::TITLE)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::TITLE)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::TITLE)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
                

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::SubTitleTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::SUB_TITLE)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::SUB_TITLE)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::SUB_TITLE)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::BodyTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::BODY)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::BODY)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::BODY)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::SensorInfoTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::SENSOR_INFO)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::SENSOR_INFO)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::SENSOR_INFO)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::FootNoteTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::FOOT_NOTE)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::FOOT_NOTE)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::FOOT_NOTE)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            UIConstants::AlertTextPointSize,
            L"en-us",
            m_textFormats[static_cast<int>(TextType::ALERT)].put()
        )
    );
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::ALERT)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    winrt::check_hresult(m_textFormats[static_cast<int>(TextType::ALERT)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    //default initialize the textLayouts, these will get changed as soon
    //as a new mode get's loaded, including the starting mode
    for (int i = 0; i < static_cast<int>(TextType::END); i++)
    {
        winrt::check_hresult(dwriteFactory->CreateTextLayout(L"", 0, m_textFormats[i].get(),
            0, 0, m_textLayouts[i].put()));
        m_textLengths.push_back(0);
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

void TextOverlay::CreateWindowSizeDependentResources(_In_ std::shared_ptr<ModeScreen> const& mode)
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
    m_textLayouts[static_cast<int>(TextType::TITLE)]->SetMaxHeight(UIConstants::TitleRectangleWidth * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::SUB_TITLE)]->SetMaxWidth(UIConstants::SubTitleRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::SUB_TITLE)]->SetMaxHeight(UIConstants::SubTitleRectangleWidth * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::BODY)]->SetMaxWidth(UIConstants::BodyRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::BODY)]->SetMaxHeight(UIConstants::BodyRectangleWidth * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::SENSOR_INFO)]->SetMaxWidth(UIConstants::SensorInfoRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::SENSOR_INFO)]->SetMaxHeight(UIConstants::SensorInfoRectangleWidth * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::FOOT_NOTE)]->SetMaxWidth(UIConstants::FootNoteRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::FOOT_NOTE)]->SetMaxHeight(UIConstants::FootNoteRectangleWidth * windowBounds.Height);

    m_textLayouts[static_cast<int>(TextType::ALERT)]->SetMaxWidth(UIConstants::AlertRectangleWidth * windowBounds.Width);
    m_textLayouts[static_cast<int>(TextType::ALERT)]->SetMaxHeight(UIConstants::AlertRectangleWidth * windowBounds.Height);
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
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    winrt::check_hresult(
        dwriteFactory->CreateTextLayout(
            &new_message[0],
            new_message.size(),
            m_textFormats[static_cast<int>(tt)].get(),
            m_textLayouts[static_cast<int>(tt)]->GetMaxWidth(),
            m_textLayouts[static_cast<int>(tt)]->GetMaxHeight(),
            m_textLayouts[static_cast<int>(tt)].put()
        )
    );
}

void TextOverlay::Render(_In_ std::shared_ptr<ModeScreen> const& mode)
{
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
    auto windowBounds = m_deviceResources->GetLogicalSize();

    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    auto renderTextMap = mode->getRenderText();

    winrt::check_hresult(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(1.0, 1.0, 1.0),
            m_textBrush.put()
        )
    );

    d2dContext->DrawTextLayout(
        Point2F(UIConstants::Margin, UIConstants::Margin),
        m_textLayouts[static_cast<int>(TextType::TITLE)].get(),
        m_textBrush.get()
    );

    /*winrt::check_hresult(
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            windowBounds.Height * 0.05,
            L"en-us",
            m_testTextFormat.put()
        )
    );*/


    //for (auto it = renderTextMap->begin(); it != renderTextMap->end(); it++)
    //{
    //    for (int i = 0; i < it->second.size(); i++)
    //    {
    //        Text* rt = &it->second[i];

    //        //First create a new textBrush with the appropriate color
    //        winrt::check_hresult(
    //            d2dContext->CreateSolidColorBrush(
    //                D2D1::ColorF(rt->color.r, rt->color.g, rt->color.b, rt->color.a),
    //                m_textBrush.put()
    //            )
    //        );

    //        //Then draw the text
    //        /*d2dContext->DrawText(
    //            rt->message,
    //            rt->getMessageLength(),
    //            m_Formats[static_cast<int>(it->first)].get(),
    //            D2D1::RectF(rt->x, rt->y, 500, 500),
    //            m_textBrush.get()
    //        );*/

    //        d2dContext->DrawTextLayout(
    //            Point2F(UIConstants::Margin, UIConstants::Margin),
    //            m_testTextLayout.get(),
    //            m_textBrush.get()
    //        );
    //    }
    //}

    /*if (m_showTitle)
    {
        d2dContext->DrawBitmap(
            m_logoBitmap.get(),
            D2D1::RectF(
                UIConstants::Margin,
                UIConstants::Margin,
                m_logoSize.width + UIConstants::Margin,
                m_logoSize.height + UIConstants::Margin
            )
        );
        d2dContext->DrawTextLayout(
            Point2F(UIConstants::Margin, UIConstants::Margin),
            m_titleHeaderLayout.get(),
            m_textBrush.get()
        );
        d2dContext->DrawTextLayout(
            Point2F(UIConstants::Margin, m_titleBodyVerticalOffset),
            m_titleBodyLayout.get(),
            m_textBrush.get()
        );
        d2dContext->DrawTextLayout(
            Point2F(UIConstants::Margin, m_titleBodyVerticalOffset + 500.0),
            m_testTextLayout.get(),
            m_textBrush.get()
        );
    }*/

    //if (game != nullptr)
    //{
    //    // This section is only used after the game state has been initialized.
    //    static const int bufferLength = 256;
    //    static wchar_t wsbuffer[bufferLength];
    //    int length = swprintf_s(
    //        wsbuffer,
    //        bufferLength,
    //        L"Hits:\t%10dShots:\t%10d\nTizime:\t%8.1f",
    //        game->TotalHits(),
    //        game->TotalShots(),
    //        game->TimeRemaining()
    //    );

    //    d2dContext->DrawText(
    //        wsbuffer,
    //        length,
    //        m_textFormatBody.get(),
    //        D2D1::RectF(
    //            windowBounds.Width - UIConstants::HudRightOffset,
    //            UIConstants::HudTopOffset,
    //            windowBounds.Width,
    //            UIConstants::HudTopOffset + (UIConstants::HudBodyPointSize + UIConstants::Margin) * 3
    //        ),
    //        m_textBrush.get()
    //    );

    //    uint32_t levelCharacter[6];
    //    for (uint32_t i = 0; i < 6; i++)
    //    {
    //        levelCharacter[i] = 0x2780 + i + ((static_cast<uint32_t>(game->LevelCompleted()) == i) ? 10 : 0);
    //    }
    //    length = swprintf_s(
    //        wsbuffer,
    //        bufferLength,
    //        L"%lc %lc %lc %lc %lc %lc",
    //        levelCharacter[0],
    //        levelCharacter[1],
    //        levelCharacter[2],
    //        levelCharacter[3],
    //        levelCharacter[4],
    //        levelCharacter[5]
    //    );
    //    d2dContext->DrawText(
    //        wsbuffer,
    //        length,
    //        m_textFormatBodySymbol.get(),
    //        D2D1::RectF(
    //            windowBounds.Width - UIConstants::HudRightOffset,
    //            UIConstants::HudTopOffset + (UIConstants::HudBodyPointSize + UIConstants::Margin) * 3 + UIConstants::Margin,
    //            windowBounds.Width,
    //            UIConstants::HudTopOffset + (UIConstants::HudBodyPointSize + UIConstants::Margin) * 4
    //        ),
    //        m_textBrush.get()
    //    );

    //    if (game->IsActivePlay())
    //    {
    //        // Draw a rectangle for the touch input for the move control.
    //        d2dContext->DrawRectangle(
    //            D2D1::RectF(
    //                0.0f,
    //                windowBounds.Height - UIConstants::TouchRectangleSize,
    //                UIConstants::TouchRectangleSize,
    //                windowBounds.Height
    //            ),
    //            m_textBrush.get()
    //        );
    //        // Draw a rectangle for the touch input for the fire control.
    //        d2dContext->DrawRectangle(
    //            D2D1::RectF(
    //                windowBounds.Width - UIConstants::TouchRectangleSize,
    //                windowBounds.Height - UIConstants::TouchRectangleSize,
    //                windowBounds.Width,
    //                windowBounds.Height
    //            ),
    //            m_textBrush.get()
    //        );

    //        // Draw the cross hairs
    //        d2dContext->DrawLine(
    //            D2D1::Point2F(windowBounds.Width / 2.0f - UIConstants::CrossHairHalfSize, windowBounds.Height / 2.0f),
    //            D2D1::Point2F(windowBounds.Width / 2.0f + UIConstants::CrossHairHalfSize, windowBounds.Height / 2.0f),
    //            m_textBrush.get(),
    //            3.0f
    //        );
    //        d2dContext->DrawLine(
    //            D2D1::Point2F(windowBounds.Width / 2.0f, windowBounds.Height / 2.0f - UIConstants::CrossHairHalfSize),
    //            D2D1::Point2F(windowBounds.Width / 2.0f, windowBounds.Height / 2.0f + UIConstants::CrossHairHalfSize),
    //            m_textBrush.get(),
    //            3.0f
    //        );
    //    }
    //}
}

void TextOverlay::ReleaseDeviceDependentResources()
{
    m_textBrush = nullptr;
    m_logoBitmap = nullptr;
}