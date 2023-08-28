#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "TextRenderer.h"
#include "MenuObjectRenderer.h"

#include <string>

//This class is responsible for the rendering of all 2D objects. This includes
//text, as well as menu objects like buttons, drop downs, combo boxes, etc.

class Direct2DRenderer
{
public:
	Direct2DRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources);

	void CreateWindowSizeDependentResources(_In_ std::vector<std::shared_ptr<MenuObject> > const& menuObjects);
	void ReleaseDeviceDependentResources();

	void Render(_In_ std::shared_ptr<ModeScreen> const& mode);

	void UpdateText(Text const& text);
	void addMenuObject(std::shared_ptr<MenuObject> menuObject);

	void delteExistingMenuObjects();

private:
	std::shared_ptr<DX::DeviceResources>        m_deviceResources;

	MenuObjectRenderer                          m_menuObjectRenderer;
	TextRenderer                                m_textRenderer;
};