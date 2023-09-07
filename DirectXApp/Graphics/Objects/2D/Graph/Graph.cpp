#include "pch.h"
#include "Graph.h"

Graph::Graph(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor fillColor, UIColor outlineColor)
{
	//Simply create the background of the graph. Normally the background for the graph is white, although it
	//can be changed.
	OutlinedBox graphBackground(windowSize, location, size, false, fillColor, outlineColor);
	p_children.push_back(std::make_shared<OutlinedBox>(graphBackground));

	//Set the screen size dependent information for the TextBox
	m_size = size;
	m_location = location;
	m_minmalPoint = { location.x - size.x / 2.0f, location.y - size.y / 2.0f };
	m_maximalPoint = { location.x + size.x / 2.0f, location.y + size.y / 2.0f };
}

void Graph::addNewDataPoints(std::vector<DirectX::XMFLOAT2> const& dataPoints)
{
	//This function is basically for creating a new graph from a set of data points.
	//Make sure that there are at least two data points before attempting to 
	//create any lines
	if (dataPoints.size() < 2) return;

	//First, clear out any existing data points.
	removeAllLines();

	//Calculate the current size of the window
	auto currentWindowSize = getCurrentWindowSize();

	//Take an initial scan through all of the points to calculate the minimum values of x and y,
	//these will be used to calculate the aboslute minima and maxima.
	DirectX::XMFLOAT2 minimum = { dataPoints[0].x, dataPoints[0].y }, maximum = { dataPoints[0].x, dataPoints[0].y};
	for (int i = 0; i < dataPoints.size(); i++)
	{
		if (dataPoints[i].x < minimum.x) minimum.x = dataPoints[i].x;
		else if (dataPoints[i].x > maximum.x) maximum.x = dataPoints[i].x;

		if (dataPoints[i].y < minimum.y) minimum.y = dataPoints[i].y;
		else if (dataPoints[i].y > maximum.y) maximum.y = dataPoints[i].y;
	}

	//Now that we know the maximum and minimum data points, we can convert all of the data into
	//absolute screen coordinates based on the current values of m_minimalPoint and m_maximalPoint
	//by using the following ratio: (data_point - minimum_point) / (maximum_point - data_point)
	// = (data_point_absolute - minimum_absolute) / (maximum_absolute - data_point_absolute)
	DirectX::XMFLOAT2 difference = { maximum.x - minimum.x, maximum.y - minimum.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalPoint.x - m_minmalPoint.x, m_maximalPoint.y - m_minmalPoint.y };
	DirectX::XMFLOAT2 previousPoint = { absoluteDifference.x * ((dataPoints[0].x - minimum.x) / difference.x) + m_minmalPoint.x, absoluteDifference.y * ((dataPoints[0].y - minimum.y) / difference.y) + m_minmalPoint.y }, currentPoint = { 0, 0 };
	for (int i = 0; i < dataPoints.size(); i++)
	{
		currentPoint = { absoluteDifference.x * ((dataPoints[i].x - minimum.x) / difference.x) + m_minmalPoint.x, absoluteDifference.y * ((dataPoints[i].y - minimum.y) / difference.y) + m_minmalPoint.y };
		Line line(currentWindowSize, currentPoint, previousPoint);
		p_children.push_back(std::make_shared<Line>(line));
		previousPoint = currentPoint;
	}
}

void Graph::addLine(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2)
{
	//Create a line from the given data points and it it to the cild array
	Line dataLine(windowSize, point1, point2);
	p_children.push_back(std::make_shared<Line>(dataLine));
}

void Graph::removeAllLines()
{
	//Remove all lines from the graph. The first line starts at chilc element 1
	for (int i = 1; i < p_children.size(); i++) p_children[i] = nullptr;
	p_children.erase(p_children.begin() + 1, p_children.end());
}