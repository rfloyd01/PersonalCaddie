#pragma once

#include "Graphics/Objects/2D/BasicElements/Line.h"
#include "Graphics/Objects/2D/BasicElements/OutlinedBox.h"
#include "Graphics/Objects/2D/BasicElements/Ellipse.h"

//The basic text box consists of two children UI Elements. There's a shadowed
//box which is meant as the background for text (default color is white) and
//then there's a TextOverlay element which displays text on top of the
//background. The text box doesn't have any interactions you can do with it
//so if too much text is added the box will need to be made bigger to display
//all of the words.
class Graph : public UIElement, IClickableUI
{
public:
	Graph(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool line = true, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black, bool isSquare = false, bool canZoom = true);
	Graph() {} //empty default constructor

	void addGraphData(std::vector<DirectX::XMFLOAT2> const& dataPoints, UIColor lineColor);

	void setAxisMaxAndMins(DirectX::XMFLOAT2 axis_minimums, DirectX::XMFLOAT2 axis_maximums);
	void addLine(DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2);
	void removeAllLines();

	void addAxisLine(int axis, float location);
	void addAxisLabel(std::wstring label, int axis, float location);

	virtual uint32_t update(InputState* inputState) override;

	//TODO: I should override the setChildrenAbsoluteSize() method to handle 
	//manual resizing of graphs at some point.

protected:
	//Click and Release Methods
	virtual void onMouseClick() override;
	virtual void onMouseRelease() override;
	virtual void onMouseRightClick() override;

	void calculateGraphEdgeIntercept(DirectX::XMFLOAT2& intercept_point, DirectX::XMFLOAT2 standard_point);
	bool calculateGraphEdgeIntercepts(DirectX::XMFLOAT2& intercept_point_one, DirectX::XMFLOAT2& intercept_point_two);

	void addUIElementBeforeData(std::shared_ptr<UIElement> element);

	DirectX::XMFLOAT2 m_minimalAbsolutePoint, m_maximalAbsolutePoint; //these variables hold the absolute locations for the x and y min/maxes in the graph
	DirectX::XMFLOAT2 m_minimalDataPoint, m_maximalDataPoint; //these variables hold the actual data locations for the x and y min/maxes in the graph
	bool m_lineGraph; //true if lines should be drawn between successive data points, otherwise the graph will just be a scatterplot
	
	//Zoom Variables
	bool m_zoomBoxActive; //this bool is true if we actively have a zoom box in place
	DirectX::XMFLOAT2 m_mouseLocation, m_zoomBoxOrigin;
	int m_currentZoomLevel;

	DirectX::XMFLOAT2 convertUnitsToAbsolute(DirectX::XMFLOAT2 coordinates);
};