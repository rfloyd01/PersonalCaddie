#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Modes/ModeScreen.h"

class TextOverlay
{
public:
	TextOverlay(
		_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources,
		_In_ winrt::hstring const& titleHeader,
		_In_ winrt::hstring const& titleBody
	);
	TextOverlay(TextOverlay const&) = delete;
	void operator=(TextOverlay const&) = delete;

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void Render(_In_ std::shared_ptr<ModeScreen> const& mode);

private:
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



    bool                                 m_showTitle;
    winrt::hstring                       m_titleHeader;
    winrt::hstring                       m_titleBody;

    float                                m_titleBodyVerticalOffset;
    D2D1_SIZE_F                          m_logoSize;
    D2D1_SIZE_F                          m_maxTitleSize;

    //My variables start
    winrt::com_ptr<IDWriteTextFormat>    m_TitleFormat;
    winrt::com_ptr<IDWriteTextFormat>    m_SubTitleFormat;
    winrt::com_ptr<IDWriteTextFormat>    m_BodyFormat;
    winrt::com_ptr<IDWriteTextFormat>    m_SensorInfoFormat;
    winrt::com_ptr<IDWriteTextFormat>    m_FootNoteFormat;
    winrt::com_ptr<IDWriteTextFormat>    m_AlertFormat;

    std::vector<winrt::com_ptr<IDWriteTextFormat> >    m_Formats;
};