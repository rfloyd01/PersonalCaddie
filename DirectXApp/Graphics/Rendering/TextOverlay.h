#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Modes/ModeScreen.h"

#include <string>

class TextOverlay
{
public:
	TextOverlay(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);
	TextOverlay(TextOverlay const&) = delete;
	void operator=(TextOverlay const&) = delete;

    void CreateDeviceDependentResources();
    void CreateTextBrushes(_In_ std::shared_ptr<ModeScreen> const& mode);
    void DeleteTextBrushes();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();

    void UpdateTextTypeMessage(TextType tt, std::wstring const& new_message, TextTypeColorSplit const& colors);

    void Render(_In_ std::shared_ptr<ModeScreen> const& mode);

private:
    void SetTextRegionAlignments(TextType tt);
    void UpdateTextTypeFontSize(TextType tt);

    void CreateTextTypeBrushes(TextType tt, TextTypeColorSplit const& colors);
    void DeleteTextTypeBrushes(TextType tt);
    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources> m_deviceResources;

    winrt::com_ptr<ID2D1SolidColorBrush> m_textBrush;
    winrt::com_ptr<IDWriteTextFormat>    m_textFormatBody;
    winrt::com_ptr<IDWriteTextFormat>    m_textFormatBodySymbol;

    winrt::com_ptr<IDWriteTextFormat>    m_textFormatTitleHeader;
    winrt::com_ptr<IDWriteTextFormat>    m_textFormatTitleBody;
    winrt::com_ptr<ID2D1Bitmap>          m_logoBitmap;
    winrt::com_ptr<IDWriteTextLayout>    m_titleHeaderLayout;
    winrt::com_ptr<IDWriteTextLayout>    m_titleBodyLayout;

    winrt::com_ptr<IDWriteTextLayout>    m_testTextLayout;
    winrt::com_ptr<IDWriteTextFormat>    m_testTextFormat;



    bool                                 m_showTitle;
    winrt::hstring                       m_titleHeader;
    winrt::hstring                       m_titleBody;

    float                                m_titleBodyVerticalOffset;
    D2D1_SIZE_F                          m_logoSize;
    D2D1_SIZE_F                          m_maxTitleSize;

    //My variables start
    std::vector<winrt::com_ptr<IDWriteTextFormat> >                          m_textFormats;
    std::vector<winrt::com_ptr<IDWriteTextLayout> >                          m_textLayouts;
    std::vector<std::vector<winrt::com_ptr<ID2D1SolidColorBrush> > >         m_textColorBrushes;
    std::vector<uint32_t>                                                    m_textLengths;
    std::vector<float>                                                       m_fontSizeRatios;
    std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> m_renderBorderRatios;
    std::vector<std::pair<float, float> >                                    m_startLocations;
    //std::vector<std::pair<TextColor, std>>
};