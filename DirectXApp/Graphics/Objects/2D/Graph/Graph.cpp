#include "pch.h"
#include "Graph.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"

Graph::Graph(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool line,  UIColor fillColor, UIColor outlineColor, bool isSquare)
{
	//Simply create the background of the graph. Normally the background for the graph is white, although it
	//can be changed.
	OutlinedBox graphBackground(windowSize, location, size, isSquare, fillColor, outlineColor);
	p_children.push_back(std::make_shared<OutlinedBox>(graphBackground));

	//Set the screen size dependent information for the TextBox
	m_size = size;
	m_location = location;

	m_minimalAbsolutePoint = { location.x - size.x / 2.0f, location.y - size.y / 2.0f };
	m_maximalAbsolutePoint = { location.x + size.x / 2.0f, location.y + size.y / 2.0f };

	//Default minimum and maximum values for a graph. Uses +/- 100 so more data has a chance of
	//being seen if the min and max aren't set by the user.
	m_minimalDataPoint = { -100, -100 };
	m_maximalDataPoint = { 100, 100 };

	m_lineGraph = line;
	//m_isSquare = isSquare;
}

void Graph::setAxisMaxAndMins(DirectX::XMFLOAT2 axis_minimums, DirectX::XMFLOAT2 axis_maximums)
{
	//Before adding data to the graph we need to establish what the local maximums and minimums are.
	//The reason for this is that we need to translate the data points from real coordinates, to values
	//that have meaning on the screen.
	
	//As an example, let's say we're looking at accelerometer data where the maximum value seen is -7 m/s^2
	//and the max value seen is 2 m/s^2. Now let's say that in absolute window coordinates, the graph
	//has a location of (0.5, 0.5) and a size of (0.5, 0.5). This means it's a square that takes up half
	//of the window and is located right in the center of the window. The lowest y-coordinate of the graph
	//in absolute window units woutl be 0.75 while the highest y-coordinate would be at 0.25 (window units
	//are positive in the downwards direction). This means that any data point we have with a y value of -7 should
	//have a window coordinate y value of 0.75, data with a value of 2 would have a window coordinate of 0.25, and
	//data with a value of -2.5 would get an absolute window coordinate of 0.5, right in the center.

	//As more data sets get added to the graph we need to have some idea of what the maximum and minimum values
	//for the actual data are to make sure everything looks correct. These values should be set after clearing
	//the graph, and before adding any new data sets. All data sets added after updating these variables will
	//be to the same scale.
	m_minimalDataPoint = axis_minimums;
	m_maximalDataPoint = axis_maximums;
}

void Graph::addDataSet(std::vector<DirectX::XMFLOAT2> const& dataPoints, UIColor lineColor)
{
	//Takes a full set of data and creates a line on the graph with it in the indicated color

	//Make sure that there are at least two data points before attempting to 
	//create any lines
	if (dataPoints.size() < 2) return;

	//Calculate the current size of the window
	auto currentWindowSize = getCurrentWindowSize();

	//Take an initial scan through all of the points to calculate the minimum values of x and y,
	//these will be used to calculate the aboslute minima and maxima. It's important to note that
	//the window coordinates in the Y-direction are the opposite of what a Cartesian graph would be
	//(positive Y goes downwards) so we need to flip y-axis values but not x-axis values.
	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };
	DirectX::XMFLOAT2 previousPoint = { absoluteDifference.x * ((dataPoints[0].x - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPoints[0].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) }, currentPoint = { 0, 0 };
	
	if (m_lineGraph)
	{
		for (int i = 0; i < dataPoints.size(); i++)
		{
			currentPoint = { absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			Line line(currentWindowSize, currentPoint, previousPoint, lineColor);
			p_children.push_back(std::make_shared<Line>(line));
			previousPoint = currentPoint;
		}
	}
	else
	{
		for (int i = 0; i < dataPoints.size(); i++)
		{
			//This creates small circles instead of lines to create the graph.
			currentPoint = { absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			
			//if (m_isSquare)
			//{
			//	//TODO: When a graph is a square the absolute location of the current point should change
			//	//to reflect this.
			//}
			
			Ellipse ell(currentWindowSize, currentPoint, { 0.0033f * m_size.y, 0.0033f * m_size.y }, true, lineColor);
			p_children.push_back(std::make_shared<Ellipse>(ell));
			previousPoint = currentPoint;
		}
	}
	
}

void Graph::addLine(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2)
{
	//Create a line from the given data points and add it to the cild array
	Line dataLine(windowSize, point1, point2);
	p_children.push_back(std::make_shared<Line>(dataLine));
}

void Graph::removeAllLines()
{
	//Remove all lines from the graph. The first line starts at child element 1
	for (int i = 1; i < p_children.size(); i++) p_children[i] = nullptr;
	p_children.erase(p_children.begin() + 1, p_children.end());
}

void Graph::addAxisLine(int axis, float location)
{
	//this method simply adds a straight black line going across the entire graph for the specified axis
	//and specified location. If the location is outside of the current min/max of the graph then it won't
	//actually be displayed. The location must be given in absolute coordinates.
	auto currentWindowSize = getCurrentWindowSize();

	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };

	switch (axis)
	{
	case 0:
	{
		//this is the x-axis, so we place a straight horizontal line at the specified y-value
		location = -1 * (absoluteDifference.y * ((location - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y);//convert the given location from data coordinates into absolute window coordinates (y-axis is flipped)
		Line dataLine(currentWindowSize, {m_location.x - m_size.x / 2.0f, location}, { m_location.x + m_size.x / 2.0f, location }, UIColor::Black, 1.5f);
		p_children.push_back(std::make_shared<Line>(dataLine));
		break;
	}
	case 1:
	{
		//this is the y-axis, so we place a straight vertical line at the specified x-value
		location = absoluteDifference.x * ((location - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x; //convert the given location from data coordinates into absolute window coordinates.
		Line dataLine(currentWindowSize, { location,  m_location.y - m_size.y / 2.0f }, { location,  m_location.y + m_size.y / 2.0f }, UIColor::Black, 1.5f);
		p_children.push_back(std::make_shared<Line>(dataLine));
		break;
	}
	}
}

void Graph::addAxisLabel(std::wstring label, float location)
{
	//adds a label to the y-axis of the graph at the specified height. The height
	//is specified in the same units as the data itself, so it must be converted into absolute
	//coordinates.
	float absoluteYLocation = convertUnitsToAbsolute({ 0, location }).y;
	TextOverlay graphText(getCurrentWindowSize(), { m_location.x, absoluteYLocation}, { m_size.x, 0.035 }, label, 0.015, { UIColor::Black }, { 0, (unsigned int)label.length() }, UITextJustification::UpperLeft);
	p_children.push_back(std::make_shared<TextOverlay>(graphText));
}

DirectX::XMFLOAT2 Graph::convertUnitsToAbsolute(DirectX::XMFLOAT2 coordinates)
{
	//The graph class needs to take points in the graph (which will be in whatever unit the graph is displaying) and convert
	//these into aboslute window coordinates so that everything will display correctly. This method handles the conversion.
	//It does this by looking at the minimum/maximum data values and the current locations of the edges of the graph

	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };
	return { absoluteDifference.x * ((coordinates.x - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((coordinates.y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
}