#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Modes/ModeScreen.h"

#include <string>

class TextRenderer
{
public:
	TextRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);
	TextRenderer(TextRenderer const&) = delete;
	void operator=(TextRenderer const&) = delete;

    void CreateDeviceDependentResources();
    void DeleteTextBrushes();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();

    void UpdateText(Text const& text);

    void Render(_In_ std::shared_ptr<ModeScreen> const& mode);

private:
    void SetTextRegionAlignments(TextType tt);
    void UpdateTextTypeFontSize(TextType tt);

    void CreateTextTypeBrushes(Text const& text);
    void DeleteTextTypeBrushes(TextType tt);

    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources>                                     m_deviceResources;

    winrt::com_ptr<ID2D1SolidColorBrush>                                     m_defaultBrush;
    std::vector<winrt::com_ptr<IDWriteTextFormat> >                          m_textFormats;
    std::vector<winrt::com_ptr<IDWriteTextLayout> >                          m_textLayouts;
    std::vector<std::vector<winrt::com_ptr<ID2D1SolidColorBrush> > >         m_textColorBrushes;
    std::vector<uint32_t>                                                    m_textLengths;
    std::vector<float>                                                       m_fontSizeRatios;
    std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> m_renderBorderRatios;
    std::vector<std::pair<float, float> >                                    m_startLocations;
};