#include "pch.h"
#include "Direct2DRenderer.h"

Direct2DRenderer::Direct2DRenderer(_In_ std::shared_ptr<DX::DeviceResources> const& deviceResources) :
	m_deviceResources(deviceResources),
	m_textRenderer(deviceResources),
	m_menuObjectRenderer(deviceResources)
{

}

void Direct2DRenderer::CreateWindowSizeDependentResources(_In_ std::vector<std::shared_ptr<MenuObject> > const& menuObjects)
{
	m_menuObjectRenderer.CreateWindowSizeDependentResources(menuObjects);
	m_textRenderer.CreateWindowSizeDependentResources();
}

void Direct2DRenderer::ReleaseDeviceDependentResources()
{
	m_menuObjectRenderer.ReleaseDeviceDependentResources();
	m_textRenderer.ReleaseDeviceDependentResources();
}

void Direct2DRenderer::Render(_In_ std::shared_ptr<ModeScreen> const& mode)
{
	//Render 2D menu objects before text because some of these objects
	//ned to have text rendered on top of them
	m_menuObjectRenderer.Render();
	m_textRenderer.Render(mode);
}

void Direct2DRenderer::UpdateText(Text const& text)
{
	m_textRenderer.UpdateText(text);
}

void Direct2DRenderer::addMenuObject(std::shared_ptr<MenuObject> menuObject)
{
	m_menuObjectRenderer.addMenuObject(menuObject);

	if (menuObject->getText() != L"")
	{
		//this menu object has some text to render which means we need to add a new
		//text layout/format to the text renderer
		m_textRenderer.addMenuObjectText(menuObject->getObjectLocations()[0], menuObject->getDimensions()[0], menuObject->getText(), menuObject->getObjectStates()[0]);
	}
}

void Direct2DRenderer::delteExistingMenuObjects()
{
	//deletes all menu objects from the screen and any associated text
	m_textRenderer.deleteMenuObjects();
	m_menuObjectRenderer.deleteMenuObjects();
}