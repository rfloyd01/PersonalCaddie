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

class GraphDataSet : public UIElement
{
public:
	GraphDataSet(DirectX::XMFLOAT2 minimalDataPoint, DirectX::XMFLOAT2 maximalDataPoint);

	void addLine(Line const& l) { p_children.push_back(std::make_shared<Line>(l)); }
	void addEllipse(Ellipse const& e) { p_children.push_back(std::make_shared<Ellipse>(e)); }

	void addGridLines(winrt::Windows::Foundation::Size windowSize, int vertical_grid_lines, int horizontal_grid_lines, DirectX::XMFLOAT2 absoluteGraphMaximums, DirectX::XMFLOAT2 absoluteGraphMinimums);
	int getVerticalGridLines() { return m_vertical_grid_lines; }
	int getHorizontalGridLines() { return m_horizontal_grid_lines; }

	DirectX::XMFLOAT2 getMinimalDataPoint() { return m_minimalDataPoint; }
	DirectX::XMFLOAT2 getMaximalDataPoint() { return m_maximalDataPoint; }

private:
	DirectX::XMFLOAT2 m_minimalDataPoint, m_maximalDataPoint; //these variables hold the actual data locations for the x and y min/maxes in the data set
	int m_vertical_grid_lines, m_horizontal_grid_lines;
};