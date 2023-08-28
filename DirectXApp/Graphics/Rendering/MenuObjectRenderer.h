#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Modes/ModeScreen.h"

#include <string>

enum class MenuObjectBrushType
{
	BlackOutline = 0,
	ButtonPressedFill = 1,
	ButtonNotPressedFill = 2,
	WhiteBackground = 3,
	END = 4
};

class MenuObjectRenderer
{
public:
	MenuObjectRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);
	MenuObjectRenderer(MenuObjectRenderer const&) = delete;
	void operator=(MenuObjectRenderer const&) = delete;

	void updateMenuObjectBrush(int index, MenuObjectState state);
	void DeleteBrushes();
	void CreateWindowSizeDependentResources(_In_ std::vector<std::shared_ptr<MenuObject> > const& menuObjects);
	void ReleaseDeviceDependentResources();

	void addMenuObject(std::shared_ptr<MenuObject> menuObject);
	void deleteMenuObjects();

	void Render();

private:
	std::shared_ptr<DX::DeviceResources>                       m_deviceResources;

	std::vector<D2D1_RECT_F>                                   m_rectangles;
	std::vector<winrt::com_ptr<ID2D1SolidColorBrush> >         m_brushes;
	std::vector<int>                                           m_brushIndices;

};