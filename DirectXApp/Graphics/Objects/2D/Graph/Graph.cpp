#include "pch.h"
#include "Graph.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"
#include "Graphics/Objects/2D/Graph/GraphDataSet.h"

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

void Graph::addDataSet(winrt::Windows::Foundation::Size windowSize, std::vector<DirectX::XMFLOAT2> const& dataPoints, UIColor lineColor)
{
	//Takes a full set of data and creates a line on the graph with it in the indicated color

	//Make sure that there are at least two data points before attempting to 
	//create any lines
	if (dataPoints.size() < 2) return;

	//Take an initial scan through all of the points to calculate the minimum values of x and y,
	//these will be used to calculate the aboslute minima and maxima. It's important to note that
	//the window coordinates in the Y-direction are the opposite of what a Cartesian graph would be
	//(positive Y goes downwards) so we need to flip y-axis values but not x-axis values.
	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };
	DirectX::XMFLOAT2 previousPoint = { absoluteDifference.x * ((dataPoints[0].x - m_minimalDataPoint.x) / difference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPoints[0].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) }, currentPoint = { 0, 0 };
	
	//Check to see if the graph is a square graph, if so then all points need to be shifted accordingly
	//to make sure that they actually fall inside of the box child element
	float squareGraphRatioCorrection = 1.0f, squareGraphDriftCorrection = 0.0f;
	auto childBox = ((OutlinedBox*)p_children[0].get());
	if (childBox->isSquare())
	{
		//Get the ratio of the screen width to it's height. Since most screens are in portrait mode
		//instead of landscape this will almost always result and a ratio less than one, which will
		//effectively squish the data points along the x-axis to fit into the graph.
		squareGraphRatioCorrection = windowSize.Height / windowSize.Width;
		squareGraphDriftCorrection = childBox->fixSquareBoxDrift(windowSize);
	}

	//Create a GraphDataSet child element to add lines and or points to. This allows us to alter entire
	//data sets without needing to change the properties of each individual line or point.
	p_children.push_back(std::make_shared<GraphDataSet>());

	if (m_lineGraph)
	{
		for (int i = 0; i < dataPoints.size(); i++)
		{
			currentPoint = { squareGraphRatioCorrection * (absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			Line line(windowSize, currentPoint, previousPoint, lineColor);
			//p_children.push_back(std::make_shared<Line>(line));
			((GraphDataSet*)p_children.back().get())->addLine(line);
			previousPoint = currentPoint;
		}
	}
	else
	{
		for (int i = 0; i < dataPoints.size(); i++)
		{
			//This creates small circles instead of lines to create the graph.
			currentPoint = { squareGraphRatioCorrection * (absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			
			Ellipse ell(windowSize, currentPoint, { 0.0033f * m_size.y, 0.0033f * m_size.y }, true, lineColor);
			//p_children.push_back(std::make_shared<Ellipse>(ell));
			((GraphDataSet*)p_children.back().get())->addEllipse(ell);
			previousPoint = currentPoint;
		}
	}
	
}

void Graph::addLine(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2)
{
	//Create a line from the given data points and add it to the cild array. Before creating the line we need
	//to first see if the graph has the square parameter and change the points accordingly.

	//TODO: Need to add the square Graph ratio correction variable at some point
	float squareGraphCorrection = 0.0f;
	auto childBox = ((OutlinedBox*)p_children[0].get());
	if (childBox->isSquare()) squareGraphCorrection = childBox->fixSquareBoxDrift(windowSize);

	point1.x += squareGraphCorrection;
	point2.x += squareGraphCorrection;

	Line dataLine(windowSize, point1, point2);
	p_children.push_back(std::make_shared<Line>(dataLine));
}

void Graph::removeAllLines()
{
	//Remove all lines from the graph. The first line starts at child element 1
	for (int i = 1; i < p_children.size(); i++) p_children[i] = nullptr;
	p_children.erase(p_children.begin() + 1, p_children.end());
}

void Graph::addAxisLine(winrt::Windows::Foundation::Size windowSize, int axis, float location)
{
	//this method simply adds a straight black line going across the entire graph for the specified axis
	//and specified location. If the location is outside of the current min/max of the graph then it won't
	//actually be displayed. The location must be given in absolute coordinates.
	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };

    //Since thex-axis scale can get distorted if the graph is locked in as a square we need to compensate for
	//this to make sure the line shows up in the correct location
	float squareGraphRatioCorrection = 1.0f, squareGraphDriftCorrection = 0.0f;
	auto childBox = ((OutlinedBox*)p_children[0].get());
	if (childBox->isSquare())
	{
		//Get the ratio of the screen width to it's height. Since most screens are in portrait mode
		//instead of landscape this will almost always result and a ratio less than one, which will
		//effectively squish the data points along the x-axis to fit into the graph.
		squareGraphRatioCorrection = windowSize.Height / windowSize.Width;
		squareGraphDriftCorrection = childBox->fixSquareBoxDrift(windowSize);
	}

	switch (axis)
	{
	case 0:
	{
		//this is the x-axis, so we place a straight horizontal line at the specified y-value. 
		location = -1 * (absoluteDifference.y * ((location - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y);//convert the given location from data coordinates into absolute window coordinates (y-axis is flipped)
		DirectX::XMFLOAT2 corrected_x_location = { m_minimalAbsolutePoint.x + squareGraphDriftCorrection, squareGraphRatioCorrection * absoluteDifference.x + m_minimalAbsolutePoint.x + squareGraphDriftCorrection };
		Line dataLine(windowSize, { corrected_x_location.x, location}, { corrected_x_location.y, location }, UIColor::Black, 1.5f);
		p_children.push_back(std::make_shared<Line>(dataLine));
		break;
	}
	case 1:
	{
		//this is the y-axis, so we place a straight vertical line at the specified x-value.
		location = squareGraphRatioCorrection * (absoluteDifference.x * ((location - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection; //convert the given location from data coordinates into absolute window coordinates.
		Line dataLine(windowSize, { location,  m_location.y - m_size.y / 2.0f }, { location,  m_location.y + m_size.y / 2.0f }, UIColor::Black, 1.5f);
		p_children.push_back(std::make_shared<Line>(dataLine));
		break;
	}
	}
}

void Graph::addAxisLabel(winrt::Windows::Foundation::Size windowSize, std::wstring label, int axis, float location)
{
	//adds a label to the x or y-axis of the graph at the specified location. The location
	//is specified in the same units as the data itself, so it must be converted into absolute
	//coordinates.
	switch (axis)
	{
	case 0:
	{
		//This is an x-axis label so the location indicates the height that we want the label at
		float absoluteYLocation = convertUnitsToAbsolute({ 0, location }).y;
		TextOverlay graphText(getCurrentWindowSize(), { m_location.x, absoluteYLocation }, { m_size.x, 0.035 }, label, 0.015, { UIColor::Black }, { 0, (unsigned int)label.length() }, UITextJustification::UpperLeft);
		p_children.push_back(std::make_shared<TextOverlay>(graphText));
		break;
	}
	case 1:
	{
		//This is a y-axis label so the location indicates the width that we want the label at

		//Like is done for other Graph methods, check if the graph is in a square configuration
	    //before setting the axis label as the width calculation could become distorted
		float squareGraphRatioCorrection = 1.0f, squareGraphDriftCorrection = 0.0f;
		auto childBox = ((OutlinedBox*)p_children[0].get());
		if (childBox->isSquare())
		{
			//Get the ratio of the screen width to it's height. Since most screens are in portrait mode
			//instead of landscape this will almost always result and a ratio less than one, which will
			//effectively squish the data points along the x-axis to fit into the graph.
			squareGraphRatioCorrection = windowSize.Height / windowSize.Width;
			squareGraphDriftCorrection = childBox->fixSquareBoxDrift(windowSize);
		}

		DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
		DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };

		float absoluteXLocation = squareGraphRatioCorrection * (absoluteDifference.x * ((location - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection;

		TextOverlay graphText(getCurrentWindowSize(), { absoluteXLocation, m_location.y + m_size.y / 2.0f }, { m_size.x, 0.035 }, label, 0.015, { UIColor::Black }, { 0, (unsigned int)label.length() }, UITextJustification::UpperCenter);
		p_children.push_back(std::make_shared<TextOverlay>(graphText));
		break;
	}
	}
	
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