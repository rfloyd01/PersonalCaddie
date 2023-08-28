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

    void addMenuObjectText(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, MenuObjectState state);
    void updateMenuObjectText(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, MenuObjectState state, int i);

    void deleteMenuObjects();

    void Render(_In_ std::shared_ptr<ModeScreen> const& mode);

private:
    void SetTextRegionAlignments(TextType tt);
    void UpdateTextTypeFontSize(TextType tt);
    void UpdateMenuObjectFontSize(int index, float objectHeight);

    void CreateTextTypeBrushes(Text const& text);
    void DeleteTextTypeBrushes(TextType tt);

    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources>                                     m_deviceResources;

    //Text formats, layouts and locations for standard text
    std::vector<winrt::com_ptr<IDWriteTextFormat> >                          m_textFormats;
    std::vector<winrt::com_ptr<IDWriteTextLayout> >                          m_textLayouts;
    std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> m_renderBorderRatios;
    std::vector<std::pair<float, float> >                                    m_startLocations;
    std::vector<uint32_t>                                                    m_textLengths;
    std::vector<float>                                                       m_fontSizeRatios;

    //Text formats, layouts and locations for menu objects. These are mode dependent
    winrt::com_ptr<IDWriteTextFormat>                                        m_menuObjectTextCenterFormat;
    winrt::com_ptr<IDWriteTextFormat>                                        m_menuObjectTextTopLeftFormat;
    std::vector<winrt::com_ptr<IDWriteTextLayout> >                          m_menuObjectTextLayouts;
    std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> m_menuObjectRenderBorderRatios;
    std::vector<std::pair<float, float> >                                    m_menuObjectStartLocations;
    std::vector<uint32_t>                                                    m_menuObjectTextLengths;
    std::vector<float>                                                       m_menuObjectFontSizeRatios;

    winrt::com_ptr<ID2D1SolidColorBrush>                                     m_defaultBrush;
    winrt::com_ptr<ID2D1SolidColorBrush>                                     m_menuObjectDefaultBrush;
    std::vector<std::vector<winrt::com_ptr<ID2D1SolidColorBrush> > >         m_textColorBrushes;

    
    
};