#pragma once

#include "Graphics/Objects/2D/BasicElements/Line.h"
#include "Graphics/Objects/2D/BasicElements/OutlinedBox.h"

//The basic text box consists of two children UI Elements. There's a shadowed
//box which is meant as the background for text (default color is white) and
//then there's a TextOverlay element which displays text on top of the
//background. The text box doesn't have any interactions you can do with it
//so if too much text is added the box will need to be made bigger to display
//all of the words.

class Graph : public UIElement
{
public:
	Graph(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black);

	void addNewDataPoints(std::vector<DirectX::XMFLOAT2> const& dataPoints);

	void addLine(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2);
	void removeAllLines();

protected:
	DirectX::XMFLOAT2 m_minmalPoint, m_maximalPoint; //these variables hold the absolute locations for the x and y min/maxes in the graph
};