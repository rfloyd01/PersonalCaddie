#include "pch.h"
#include "UIElement.h"

void* UIElement::render(RenderOrder render, int element)
{
	//The UIElement renderer will call this method once for each of the different
	//vectors of text and shapes that need to be rendered. Rendering must be done
	//in a specific order to create some elements like scroll boxes which is the
	//reason for doing it this way

	//Really the only thing that this render method does is iterate over the appropriate
	//vector and return a reference to the element to be rendered. The renderer class
	//needs to convert the void* to either a UIText* or UIShape*, extract the type of
	//shape or text to be drawn, and then call the appropriate drawing method.

	switch (render)
	{
	case RenderOrder::Background:
		return (void*)&m_backgroundShapes[element];
		break;
	case RenderOrder::ElementText:
		return (void*)&m_elementText[element];
		break;
	case RenderOrder::ForegroundNoOverlap:
		return (void*)&m_foregroundShapesNoOverlap[element];
		break;
	case RenderOrder::ForegroundWithOverlap:
		return (void*)&m_foregroundShapesCanOverlap[element];
		break;
	case RenderOrder::TextOverlay:
		return (void*)&m_textOverlay[element];
		break;
	}
}

int UIElement::getRenderVectorSize(RenderOrder render)
{
	//returns the size of the appropriate render vector
	switch (render)
	{
	case RenderOrder::Background:
		return m_backgroundShapes.size();
		break;
	case RenderOrder::ElementText:
		return m_elementText.size();
		break;
	case RenderOrder::ForegroundNoOverlap:
		return m_foregroundShapesNoOverlap.size();
		break;
	case RenderOrder::ForegroundWithOverlap:
		return m_foregroundShapesCanOverlap.size();
		break;
	case RenderOrder::TextOverlay:
		return m_textOverlay.size();
		break;
	}
}

bool UIElement::isAlert()
{
	//a special method used for finding alerts
	if (m_textOverlay.size() > 0)
	{
		for (int i = 0; i < m_textOverlay.size(); i++)
		{
			if (m_textOverlay[i].textType == UITextType::ALERT) return true;
		}
	}
	return false; //alerts must be text overlays
}