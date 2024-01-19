#pragma once

#include "Graphics/Objects/2D/BasicElements/Line.h"
#include "Graphics/Objects/2D/BasicElements/Ellipse.h"

//This class represents multiple points of data that are to 
//be rendered on a graph. These points can be connected by
//lines or simply be floating. This class isn't a UIElement
//per se, it's more a container for related UIElements that compose a graph.
//This class is handy when we want to alter an entire set of data
//in some way, whether it's making the data invisible, changing 
//the scale of the data, etc.

class GraphData : public UIElement
{
public:
	GraphData(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring name = L"")
	{ 
		m_screenSize = windowSize;
		updateLocationAndSize(location, size);

		m_name = name;

		m_state &= ~UIElementState::Dummy; //elements created without the default constructor have the dummy flag removed
		m_state |= UIElementState::Idlee; //Graph Data can't be interacted with, setting this flag will skip the update loop for the data set and all children
	}

	void addLine(Line const& l)
	{
		p_children.push_back(std::make_shared<Line>(l));
		m_dataColor = l.getLineColor();
	}

	void addEllipse(Ellipse const& e)
	{
		p_children.push_back(std::make_shared<Ellipse>(e));
		m_dataColor = e.getColor();
	}

	UIColor getDataColor() const { return m_dataColor; }
	std::wstring getName() const { return m_name; }

private:
	UIColor m_dataColor;
	std::wstring m_name;
};