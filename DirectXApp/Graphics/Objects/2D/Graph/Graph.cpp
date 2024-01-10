#include "pch.h"
#include "Graph.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"
#include "Graphics/Objects/2D/Graph/GraphDataSet.h"

Graph::Graph(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool line,  UIColor fillColor, UIColor outlineColor, bool isSquare, bool canZoom)
{
	m_screenSize = windowSize;

	//Simply create the background of the graph. Normally the background for the graph is white, although it
	//can be changed.
	OutlinedBox graphBackground(windowSize, location, size, fillColor, outlineColor);
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
	m_zoomBoxActive = false;
	m_zoomBoxOrigin = { 0.0f, 0.0f };
	m_currentZoomLevel = 0;
	m_isClickable = canZoom; //we need to be able to click the graph to zoom in on data

	if (canZoom)
	{
		//For graphs that can be zoomed in on we create two messages in the bottom
		//corners of the graph letting the user know that they can zoom, unzoom.
		std::wstring zoom_message = L"Left click and drag the mouse to select an area of data to zoom in on.";
		TextOverlay zoom(windowSize, { location.x - size.x / 4.0f + 0.0025f, location.y + size.y / 4.0f }, { size.x / 2.0f, size.y / 2.0f }, zoom_message,
			size.y * 0.022f, { UIColor::Black }, { 0,  (unsigned int)zoom_message.length() }, UITextJustification::LowerLeft);
		zoom.setState(UIElementState::Invisible);

		std::wstring unzoom_message = L"Right click the mouse to go back a zoom level";
		TextOverlay unzoom(windowSize, { location.x + size.x / 4.0f - 0.0025f, location.y + size.y / 4.0f }, { size.x / 2.0f, size.y / 2.0f }, unzoom_message,
			size.y * 0.022f, { UIColor::Black }, { 0,  (unsigned int)unzoom_message.length() }, UITextJustification::LowerRight);
		unzoom.updateState(UIElementState::Invisible);

		p_children.push_back(std::make_shared<TextOverlay>(zoom));
		p_children.push_back(std::make_shared<TextOverlay>(unzoom));
	}
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

void Graph::addGraphData(std::vector<DirectX::XMFLOAT2> const& dataPoints, UIColor lineColor)
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
		squareGraphRatioCorrection = m_screenSize->Height / m_screenSize->Width;
		squareGraphDriftCorrection = childBox->fixSquareBoxDrift();
	}

	//If there's any existing GraphDataSet child element add this new set to it, otherwise
	//create a new GraphDataSet child element to add lines and or points to. This allows us to alter entire
	//data sets without needing to change the properties of each individual line or point.
	if (dynamic_cast<GraphDataSet*>(p_children.back().get()) == nullptr)
	{
		GraphDataSet gds(m_screenSize, m_minimalDataPoint, m_maximalDataPoint);

		//If we can zoom in on the graph then also add grid lines with labels
		//which help keep track of the current zoom level
		if (m_isClickable)
		{
			gds.addGridLines(6, 6, m_maximalAbsolutePoint, m_minimalAbsolutePoint);
			p_children[1]->removeState(UIElementState::Invisible);
		}

		p_children.push_back(std::make_shared<GraphDataSet>(gds));
	}

	GraphData newData;

	if (m_lineGraph)
	{
		for (int i = 1; i < dataPoints.size(); i++)
		{
			currentPoint = { squareGraphRatioCorrection * (absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			Line line(m_screenSize, currentPoint, previousPoint, lineColor);
			newData.addLine(line);
			previousPoint = currentPoint;
		}
	}
	else
	{
		for (int i = 1; i < dataPoints.size(); i++)
		{
			//This creates small circles instead of lines to create the graph.
			currentPoint = { squareGraphRatioCorrection * (absoluteDifference.x * ((dataPoints[i].x - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection, -1 * (absoluteDifference.y * ((dataPoints[i].y - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y) };
			
			Ellipse ell(m_screenSize, currentPoint, { 0.0033f * m_size.y, 0.0033f * m_size.y }, true, lineColor);
			newData.addEllipse(ell);
			previousPoint = currentPoint;
		}
	}

	//Add the new GraphData object to the current GraphDataSet object
	if (newData.getChildren().size() > 0) ((GraphDataSet*)p_children.back().get())->addGraphData(newData);
}

void Graph::addUIElementBeforeData(std::shared_ptr<UIElement> element)
{
	//After the introduction of the GraphDataSet class which holds all lines and points that make
	//up the actual data part of the graph, it's changed the way that other lines and child elements
	//need to be added to the graph. The zoom functionality works by making a stack of different
	//data set views at the end of the child element array, so anything that has nothing to do with
	//physical data must be added before sets of data. This method safely adds UIElements to the 
	//graph without breaking the zoom functionality of the graph

	if (dynamic_cast<GraphDataSet*>(p_children.back().get()) == nullptr) p_children.push_back(element); //can safely place at the back
	else
	{
		bool placed = false;
		for (auto it = p_children.end() - 2; it != p_children.begin(); it--)
		{
			if (dynamic_cast<GraphDataSet*>(it->get()) == nullptr)
			{
				p_children.insert(it + 1, element);
				placed = true;
				break;
			}
		}

		if (!placed) p_children.insert(p_children.begin() + 1, element); //add the line right after the graph box child element
	}
}

void Graph::addLine(DirectX::XMFLOAT2 point1, DirectX::XMFLOAT2 point2)
{
	//Create a line from the given data points and add it to the cild array. Before creating the line we need
	//to first see if the graph has the square parameter and change the points accordingly. Lines created by this method
	//persist through different zoom levels which may appear somewhat unexpected.

	//TODO: Need to add the square Graph ratio correction variable at some point
	float squareGraphCorrection = 0.0f;
	auto childBox = ((OutlinedBox*)p_children[0].get());
	if (childBox->isSquare()) squareGraphCorrection = childBox->fixSquareBoxDrift();

	point1.x += squareGraphCorrection;
	point2.x += squareGraphCorrection;

	Line dataLine(m_screenSize, point1, point2);
	addUIElementBeforeData(std::make_shared<Line>(dataLine)); //safely add the new line to the child array
}

void Graph::removeAllLines()
{
	//Removes all child elements from the graph (other than zoomable instructions on
	//graphs that have that capability).
	int start_index = 1;
	if (m_isClickable) start_index = 3;

	for (int i = start_index; i < p_children.size(); i++) p_children[i] = nullptr;
	p_children.erase(p_children.begin() + start_index, p_children.end());
}

void Graph::addAxisLine(int axis, float location)
{
	//This method simply adds a straight black line going across the entire graph for the specified axis
	//and specified location. If the location is outside of the current min/max of the graph then it won't
	//actually be displayed. The location must be given in absolute coordinates. Lines created by this method
	//persist through different zoom levels which may appear somewhat unexpected.
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
		squareGraphRatioCorrection = m_screenSize->Height / m_screenSize->Width;
		squareGraphDriftCorrection = childBox->fixSquareBoxDrift();
	}

	DirectX::XMFLOAT2 point_one = {0.0f, 0.0f}, point_two = { 0.0f, 0.0f };

	switch (axis)
	{
	case 0:
	{
		//this is the x-axis, so we place a straight horizontal line at the specified y-value.
		//This line needs to be correct if the graph is a "square"
		location = -1 * (absoluteDifference.y * ((location - m_minimalDataPoint.y) / difference.y) - m_maximalAbsolutePoint.y); //convert the given location from data coordinates into absolute window coordinates (y-axis is flipped)
		DirectX::XMFLOAT2 corrected_x_location = { m_minimalAbsolutePoint.x + squareGraphDriftCorrection, squareGraphRatioCorrection * absoluteDifference.x + m_minimalAbsolutePoint.x + squareGraphDriftCorrection };
		point_one = { corrected_x_location.x, location };
		point_two = { corrected_x_location.y, location };
		break;
	}
	case 1:
	{
		//this is the y-axis, so we place a straight vertical line at the specified x-value.
		location = squareGraphRatioCorrection * (absoluteDifference.x * ((location - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection; //convert the given location from data coordinates into absolute window coordinates.
		point_one = { location,  m_location.y - m_size.y / 2.0f };
		point_two = { location,  m_location.y + m_size.y / 2.0f };
		break;
	}
	}

	//Create the line and place it in the last location of the child
	//array that isn't a GraphDataSet type
	Line dataLine(m_screenSize, point_one, point_two, UIColor::Black, 1.5f);
	addUIElementBeforeData(std::make_shared<Line>(dataLine)); //safely add the line to the child array
}

void Graph::addAxisLabel(std::wstring label, int axis, float location)
{
	//adds a label to the x or y-axis of the graph at the specified location. The location
	//is specified in the same units as the data itself, so it must be converted into absolute
	//coordinates. Labels added with this method persist through different zoom levels which
	//may appear somewhat unexpected.
	switch (axis)
	{
	case 0:
	{
		//This is an x-axis label so the location indicates the height that we want the label at
		float absoluteYLocation = convertUnitsToAbsolute({ 0, location }).y;
		TextOverlay graphText(m_screenSize, { m_location.x, absoluteYLocation }, { m_size.x, 0.035 }, label, 0.015, { UIColor::Black }, { 0, (unsigned int)label.length() }, UITextJustification::UpperLeft);
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
			squareGraphRatioCorrection = m_screenSize->Height / m_screenSize->Width;
			squareGraphDriftCorrection = childBox->fixSquareBoxDrift();
		}

		DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
		DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };

		float absoluteXLocation = squareGraphRatioCorrection * (absoluteDifference.x * ((location - m_minimalDataPoint.x) / difference.x)) + m_minimalAbsolutePoint.x + squareGraphDriftCorrection;

		TextOverlay graphText(m_screenSize, { absoluteXLocation, m_location.y + m_size.y / 2.0f }, { m_size.x, 0.035 }, label, 0.015, { UIColor::Black }, { 0, (unsigned int)label.length() }, UITextJustification::UpperCenter);
		addUIElementBeforeData(std::make_shared<TextOverlay>(graphText)); //safely add the text to the child array
		break;
	}
	}
}

uint32_t Graph::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if the mouse is currently
	//being clicked. If it is, it means that the zoom box is active so moving the
	//mouse around should expand/contract the zoom box
	m_mouseLocation = inputState->mousePosition; //save current mouse location to set zoom box origin
	uint32_t currentState = UIElement::update(inputState);

	if (m_zoomBoxActive)
	{
		//The zoom box has one stationary corner that originates where the 
		//mouse was originally clicked. As the mouse is dragged the user will be 
		//moving the opposite corner of the box. Since all UI Elements have their
		//centers as their point of origin we need to update the absolute size and 
		//location of the zoom box as the mouse moves to reflect this.

		//Convert the current mouse location into absolute units and figure out 
		//what quadrant it's in relative to the boxes origin

		DirectX::XMFLOAT2 opposite_corner = { inputState->mousePosition.x / m_screenSize->Width, inputState->mousePosition.y / m_screenSize->Height };

		//If the user drags the box outside the boundary of the graph, force the box
		//to stay within the boundary
		if (opposite_corner.x < m_location.x - m_size.x / 2.0f) opposite_corner.x = m_location.x - m_size.x / 2.0f;
		else if (opposite_corner.x > m_location.x + m_size.x / 2.0f) opposite_corner.x = m_location.x + m_size.x / 2.0f;

		if (opposite_corner.y < m_location.y - m_size.y / 2.0f) opposite_corner.y = m_location.y - m_size.y / 2.0f;
		else if (opposite_corner.y > m_location.y + m_size.y / 2.0f) opposite_corner.y = m_location.y + m_size.y / 2.0f;

		DirectX::XMFLOAT2 new_origin = { (opposite_corner.x + m_zoomBoxOrigin.x) / 2.0f, (opposite_corner.y + m_zoomBoxOrigin.y) / 2.0f };
		DirectX::XMFLOAT2 new_size = { opposite_corner.x - m_zoomBoxOrigin.x, opposite_corner.y - m_zoomBoxOrigin.y };

		if (new_size.x < 0) new_size.x *= -1;
		if (new_size.y < 0) new_size.y *= -1;

		//The Zoom box can't be drawn until there's data in the graph so it will be 
		//the last child element.
		Box* zoom_box = ((Box*)p_children.back().get());
		zoom_box->setAbsoluteLocation(new_origin);
		zoom_box->setAbsoluteSize(new_size);
		zoom_box->resize(); //convert the new absoulte units into actual pixels
	}

	return currentState;
}

void Graph::onMouseClick()
{
	//If there's currently data in the graph we can use the mouse to zoom in
	//on it if we want. This is done by clicking and holding the mouse button
	//and then dragging the box that pops up over the data to be zoomed in on.
	if (dynamic_cast<GraphDataSet*>(p_children.back().get()) != nullptr)
	{
		m_zoomBoxOrigin = { m_mouseLocation.x / m_screenSize->Width, m_mouseLocation.y / m_screenSize->Height }; //set the origin of the zoom box

		Box opaque_box(m_screenSize, m_zoomBoxOrigin, { 0.0f, 0.0f }, UIColor::OpaqueBlue); //box starts off with no size
		p_children.push_back(std::make_shared<Box>(opaque_box));
		m_zoomBoxActive = true;
	}
}

void Graph::onMouseRelease()
{
	//Releasing the mouse while the zoom box is active will trigger the actual zoom
	//to occur.
	if (!m_zoomBoxActive) return;

	//Calculate the new max and min data points of the zoomed in section. This is done
	//by comparing the absoulte value of the zoom box with that of the entire graph 
	//and knowing that all minimal points are mapped to the edges of the graph.
	Box* zoom_box = ((Box*)p_children.back().get());

	DirectX::XMFLOAT2 absoluteDifference = { m_maximalAbsolutePoint.x - m_minimalAbsolutePoint.x, m_maximalAbsolutePoint.y - m_minimalAbsolutePoint.y };
	DirectX::XMFLOAT2 difference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	
	float new_x_data_min = -1 * ((m_maximalAbsolutePoint.x - (zoom_box->getAbsoluteLocation().x - zoom_box->getAbsoluteSize().x / 2.0f)) / absoluteDifference.x * difference.x - m_maximalDataPoint.x);
	float new_x_data_max = -1 * ((m_maximalAbsolutePoint.x - (zoom_box->getAbsoluteLocation().x + zoom_box->getAbsoluteSize().x / 2.0f)) / absoluteDifference.x * difference.x - m_maximalDataPoint.x);
	float new_y_data_min = -1 * (((zoom_box->getAbsoluteLocation().y + zoom_box->getAbsoluteSize().y / 2.0f) - m_minimalAbsolutePoint.y) / absoluteDifference.y * difference.y - m_maximalDataPoint.y);
	float new_y_data_max = -1 * (((zoom_box->getAbsoluteLocation().y - zoom_box->getAbsoluteSize().y / 2.0f) - m_minimalAbsolutePoint.y) / absoluteDifference.y * difference.y - m_maximalDataPoint.y);

	//Get rid of the zoom box before after mapping new max and min data points
	m_zoomBoxActive = false;
	p_children.back() = nullptr;
	p_children.pop_back();

	//Create the new GraphDataSet UIElement
	DirectX::XMFLOAT2 new_minimal_points = { new_x_data_min, new_y_data_min };
	DirectX::XMFLOAT2 new_maximal_points = { new_x_data_max, new_y_data_max };
	GraphDataSet zoomed_in_data_set(m_screenSize, new_minimal_points, new_maximal_points);

	//Add the same number of grid lines to the zoomed in view that the 
	//current view has
	int vertical_grid_lines = ((GraphDataSet*)p_children.back().get())->getVerticalGridLines();
	int horizontal_grid_lines = ((GraphDataSet*)p_children.back().get())->getHorizontalGridLines();
	zoomed_in_data_set.addGridLines(vertical_grid_lines, horizontal_grid_lines, m_maximalAbsolutePoint, m_minimalAbsolutePoint);

	//Now iterate through all the child lines and points of the current data set
	//and create new lines and points for the new zoomed in data set. All the existing data
	//is given in terms of absolute coordinates on the screen, so each point needs to be 
	//reverted back into its actual data component. Once this occurs, we need to figure
	//out if the old point should be placed on the zoomed in graph or not
	auto data_set_children = p_children.back()->getChildren();
	DirectX::XMFLOAT2 originalDifference = { m_maximalDataPoint.x - m_minimalDataPoint.x, m_maximalDataPoint.y - m_minimalDataPoint.y };
	DirectX::XMFLOAT2 newDifference = { new_x_data_max - new_x_data_min, new_y_data_max - new_y_data_min };

	//Don't add existing grid lines or their labels to the zoomed in graph as 
	//new ones were already added above.
	for (int i = 2 * (vertical_grid_lines + horizontal_grid_lines); i < data_set_children.size(); i++)
	{
		GraphData* data = dynamic_cast<GraphData*>(data_set_children[i].get());
		if (data != nullptr)
		{
			GraphData zoomed_in_data; //create a new GraphData object
			auto existing_data = data->getChildren();

			for (int j = 0; j < existing_data.size(); j++)
			{
				Line* line = dynamic_cast<Line*>(existing_data[j].get());
				if (line != nullptr)
				{
					//We're dealing with a line child element. Convert its two points from absolute
					//coordinates to data coordinates to see if the whole line, part of the line,
					//or none of the line will appear on the zoomed in graph
					auto absolute_points = line->getPointsAbsolute();

					//Convert the two absolute points of the line to their original data values
					DirectX::XMFLOAT2 dataPointOne = { (absolute_points.first.x - m_minimalAbsolutePoint.x) * originalDifference.x / absoluteDifference.x + m_minimalDataPoint.x, (m_maximalAbsolutePoint.y - absolute_points.first.y) * originalDifference.y / absoluteDifference.y + m_minimalDataPoint.y };
					DirectX::XMFLOAT2 dataPointTwo = { (absolute_points.second.x - m_minimalAbsolutePoint.x) * originalDifference.x / absoluteDifference.x + m_minimalDataPoint.x, (m_maximalAbsolutePoint.y - absolute_points.second.y) * originalDifference.y / absoluteDifference.y + m_minimalDataPoint.y };

					//Calculate the absolute locations of the above data points with the new
					//zoomed in graph boundaries
					DirectX::XMFLOAT2 newAbsoluteDataPointOne = { absoluteDifference.x * ((dataPointOne.x - new_x_data_min) / newDifference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPointOne.y - new_y_data_min) / newDifference.y) - m_maximalAbsolutePoint.y) };
					DirectX::XMFLOAT2 newAbsoluteDataPointTwo = { absoluteDifference.x * ((dataPointTwo.x - new_x_data_min) / newDifference.x) + m_minimalAbsolutePoint.x, -1 * (absoluteDifference.y * ((dataPointTwo.y - new_y_data_min) / newDifference.y) - m_maximalAbsolutePoint.y) };

					//See if the first point falls inside the new graph boundaries
					bool first_data_point_in_bounds = ((dataPointOne.x >= new_x_data_min) && (dataPointOne.x <= new_x_data_max)) && ((dataPointOne.y >= new_y_data_min) && (dataPointOne.y <= new_y_data_max));
					bool second_data_point_in_bounds = ((dataPointTwo.x >= new_x_data_min) && (dataPointTwo.x <= new_x_data_max)) && ((dataPointTwo.y >= new_y_data_min) && (dataPointTwo.y <= new_y_data_max));

					if (first_data_point_in_bounds)
					{
						//If both data points already fall inside the graph then there's no 
						//need to change anything.
						if (!second_data_point_in_bounds)
						{
							//Only the first data point fits inside the new zoomed in graph.
							//Alter the second point so that it sits on the edge of the graph.
							calculateGraphEdgeIntercept(newAbsoluteDataPointTwo, newAbsoluteDataPointOne);
						}
					}
					else
					{
						if (second_data_point_in_bounds)
						{
							//Only the second data point fits inside the new zoomed in graph.
							//Alter the first point so that it sits on the edge of the graph.
							calculateGraphEdgeIntercept(newAbsoluteDataPointOne, newAbsoluteDataPointTwo);
						}
						else
						{
							//Neither of the points of the line fit inside the new zoomed in graph,
							//however, the line itself may cross over the graph in which case two
							//new points need to be created. If the line doesn't cross over the viewing
							//area then skip ahead to the next line, if it does though then change the
							//locations of both points as necessary.
							if (!calculateGraphEdgeIntercepts(newAbsoluteDataPointOne, newAbsoluteDataPointTwo)) continue;
						}
					}

					Line new_line(m_screenSize, newAbsoluteDataPointOne, newAbsoluteDataPointTwo, line->getLineColor());
					zoomed_in_data.addLine(new_line);
				}
				else if (dynamic_cast<Ellipse*>(data_set_children[j].get()) != nullptr)
				{
					//Unlike lines, a graph point is either entirely in the viewing area, or not.
					//Simply see if the point falls in the viewing area and add it to the GraphData
					//object if it does.
					DirectX::XMFLOAT2 absolutePoint = dynamic_cast<Ellipse*>(existing_data[j].get())->getAbsoluteLocation();
					DirectX::XMFLOAT2 newPoint = { (absolutePoint.x - m_minimalAbsolutePoint.x) * originalDifference.x / absoluteDifference.x + m_minimalDataPoint.x, (m_maximalAbsolutePoint.y - absolutePoint.y) * originalDifference.y / absoluteDifference.y + m_minimalDataPoint.y };
					if (((newPoint.x >= new_x_data_min) && (newPoint.x <= new_x_data_max)) && ((newPoint.y >= new_y_data_min) && (newPoint.y <= new_y_data_max)))
					{
						Ellipse copy(*dynamic_cast<Ellipse*>(existing_data[j].get())); //Make a copy of the current ellipse to add to the new GraphData object
						zoomed_in_data.addEllipse(copy);
					}
				}
			}

			//Once all lines and points have been processed add the zoomed in GraphData object
			//to the zoomed in GraphDataSet object (if it contains any actual data)
			if (zoomed_in_data.getChildren().size() > 0) zoomed_in_data_set.addGraphData(zoomed_in_data);
		}
	}

	//Finally add the new graph data set to the graphs child UI Element list so it will
	//be displayed. We also update the m_minimal and m_maximal values for the graph
	//element. If no actual data points were selected by the zoom box then simply
	//return from this method without changing anything
	if (zoomed_in_data_set.getChildren().size() == (2 * (vertical_grid_lines + horizontal_grid_lines))) return;

	p_children.back()->setState(UIElementState::Invisible); //make the current data set invisible to reduce lines needing rendering
	p_children.push_back(std::make_shared<GraphDataSet>(zoomed_in_data_set));
	m_minimalDataPoint = new_minimal_points;
	m_maximalDataPoint = new_maximal_points;
	m_currentZoomLevel++;
	p_children[2]->removeState(UIElementState::Invisible); //display the message on how to unzoom if it isn't already
}

void Graph::onMouseRightClick()
{
	//Right clicking on the graph will have the effect of going back one zoom
	//level. If the graph is all the way zoomed out then nothing will happen.
	//Zooming out is accomplished by simply deleting the last child element of
	//of the graph and then removing the invisible state from the new last child
	//element.
	if (m_currentZoomLevel > 0)
	{
		m_currentZoomLevel--;
		p_children.pop_back();
		p_children.back()->removeState(UIElementState::Invisible);

		//reset the max and min data points for the graph to match
		//that of the current zoom level
		m_minimalDataPoint = ((GraphDataSet*)p_children.back().get())->getMinimalDataPoint();
		m_maximalDataPoint = ((GraphDataSet*)p_children.back().get())->getMaximalDataPoint();
	}
	
	//make the unzoom message invisible if it isn't already at top zoom level
	if (m_currentZoomLevel == 0) p_children[2]->updateState(UIElementState::Invisible);
}

void Graph::calculateGraphEdgeIntercept(DirectX::XMFLOAT2& intercept_point, DirectX::XMFLOAT2 standard_point)
{
	//When a line has one of its points fall on the outside of the graph, this method is used to 
	//trim the line and make sure it falls on the edge of the graph instead. Of the two points
	//passed into the method, intercept_point is the one outside of the graph while standard_point
	//is inside the graph.

	//We don't know if the line will need to be trimmed along a horizontal or vertical edge of the
	//graph, or which side of the graph at first, so these things need to be figured out.
	int x_location = 0, y_location = 0;
	if (intercept_point.x < (m_location.x - m_size.x / 2.0f)) x_location = -1; //point is outside left edge of graph
	else if (intercept_point.x > (m_location.x + m_size.x / 2.0f)) x_location = 1; //point is outside right edge of graph

	if (intercept_point.y < (m_location.y - m_size.y / 2.0f)) y_location = -1; //point is outside top edge of graph
	else if (intercept_point.y > (m_location.y + m_size.y / 2.0f)) y_location = 1; //point is outside bottom edge of graph

	//Split up the slope between the two points into a numerator and denominator
	DirectX::XMFLOAT2 slope = { intercept_point.x - standard_point.x, intercept_point.y - standard_point.y };

	if ((x_location != 0) && (y_location == 0))
	{
		//the line is guaranteed to intersect with either the left or right edge of the graph
		//so we need to calculate the y-intercept.
		float x_intercept = (m_location.x - m_size.x / 2.0f);
		if (x_location == 1) x_intercept = (m_location.x + m_size.x / 2.0f);

		float k = (x_intercept - intercept_point.x) / slope.x;
		float y_intercept = intercept_point.y + k * slope.y;
		intercept_point = { x_intercept, y_intercept };
	}
	else if ((x_location == 0) && (y_location != 0))
	{
		//the line is guaranteed to intersect with either the top or bottom edge of the graph
		//so we need to calculate the x-intercept.
		float y_intercept = (m_location.y - m_size.y / 2.0f);
		if (y_location == 1) y_intercept = (m_location.y + m_size.y / 2.0f);

		float k = (y_intercept - intercept_point.y) / slope.y;
		float x_intercept = intercept_point.x + k * slope.x;
		intercept_point = { x_intercept, y_intercept };
	}
	else
	{
		//the line can intersect with any edge of the graph at this point, so we start by
		//eliminating two options
		DirectX::XMFLOAT2 intersect_edges = { (m_location.x - m_size.x / 2.0f), (m_location.y - m_size.y / 2.0f) };
		if (x_location == 1) intersect_edges.x = (m_location.x + m_size.x / 2.0f);
		if (y_location == 1) intersect_edges.y = (m_location.y + m_size.y / 2.0f);

		//The line will technically intersect both edges of the graph, but only one of these
		//intersections will actually occur within the viewing area. Calculate both 
		//intersections and take whichever one is in the actual rendering area.
		float k = (intersect_edges.x - intercept_point.x) / slope.x;
		DirectX::XMFLOAT2 option_one = { intersect_edges.x, intercept_point.y + k * slope.y };

		if ((option_one.y > (m_location.y - m_size.y / 2.0f)) && (option_one.y < (m_location.y + m_size.y / 2.0f)))
		{
			intercept_point = option_one;
		}
		else
		{
			k = (intersect_edges.y - intercept_point.y) / slope.y;
			DirectX::XMFLOAT2 option_two = { intercept_point.x + k * slope.x, intersect_edges.y };
			intercept_point = option_two;
		}
	}
}

bool Graph::calculateGraphEdgeIntercepts(DirectX::XMFLOAT2& intercept_point_one, DirectX::XMFLOAT2& intercept_point_two)
{
	//this method gets called when two points exist outside of the graph and we want to figure out if
	//the line between them crosses through the viewing area. To start off, if both points are on the 
	//same side of the graph (i.e. both above it, both to the left side, etc.) then we know the line won't
	//cross into the viewing area right off the bat.
	bool hit = false;

	//Pre-calculate edges of graph
	float top_edge    = m_location.y - (m_size.y / 2.0f);
	float bottom_edge = m_location.y + (m_size.y / 2.0f);
	float left_edge   = m_location.x - (m_size.x / 2.0f);
	float right_edge  = m_location.x + (m_size.x / 2.0f);

	if ((intercept_point_one.x < left_edge) && (intercept_point_two.x < left_edge)) return hit; //no part of the line is in viewing area
	else if ((intercept_point_one.x > right_edge) && (intercept_point_two.x > right_edge)) return hit; //no part of the line is in viewing area

	if ((intercept_point_one.y < top_edge) && (intercept_point_two.y < top_edge)) return hit; //no part of the line is in viewing area
	else if ((intercept_point_one.y > bottom_edge) && (intercept_point_two.y > bottom_edge)) return hit; //no part of the line is in viewing area

	//If we haven't returned false yet then it's possible (although not guaranteed) that part of 
	//the line travels through the viewing area. Depending on where the two points are, we can pass
	//through 2, 3 or 4 edges of the graph (in the viewing area or not). Calculate which edges
	//we need to check by examining the location points.
	std::vector<float> x_edges_to_check, y_edges_to_check;

	if (intercept_point_one.y < top_edge || intercept_point_two.y < top_edge) x_edges_to_check.push_back(top_edge);
	if (intercept_point_one.y > bottom_edge || intercept_point_two.y > bottom_edge) x_edges_to_check.push_back(bottom_edge);
	if (intercept_point_one.x < left_edge || intercept_point_two.x < left_edge) y_edges_to_check.push_back(left_edge);
	if (intercept_point_one.x > right_edge || intercept_point_two.x > right_edge) y_edges_to_check.push_back(right_edge);

	//It isn't possible to only go through a single edge since both points are outside of the viewing
	//area of a graph. Likewise, since all lines are straight we can't go through more than two edges.
	//It is possible to go through to vertical or two horizontal edges, or one each of horizontal and
	//vertical. Because of this we need to check all possible edges of the graph.
	DirectX::XMFLOAT2 slope = { intercept_point_two.x - intercept_point_one.x, intercept_point_two.y - intercept_point_one.y };
	std::vector<DirectX::XMFLOAT2> intercepts;

	for (int i = 0; i < x_edges_to_check.size(); i++)
	{
		float k = (x_edges_to_check[i] - intercept_point_one.y) / slope.y;
		float x_intercept = intercept_point_one.x + k * slope.x;

		if ((x_intercept >= left_edge) && (x_intercept <= right_edge)) intercepts.push_back({ x_intercept, x_edges_to_check[i] });
	}

	for (int i = 0; i < y_edges_to_check.size(); i++)
	{
		float k = (y_edges_to_check[i] - intercept_point_one.x) / slope.x;
		float y_intercept = intercept_point_one.y + k * slope.y;

		if ((y_intercept >= top_edge) && (y_intercept <= bottom_edge)) intercepts.push_back({ y_edges_to_check[i], y_intercept });
	}

	if (intercepts.size() == 2)
	{
		//it doesn't really matter which edge intercept we choose for each point but just to make 
		//sure we don't swap the points by mistake, choose the intercept closest to each point.
		float intercept_one_distance = sqrt((intercepts[0].x - intercept_point_one.x) * (intercepts[0].x - intercept_point_one.x) + (intercepts[0].y - intercept_point_one.y) * (intercepts[0].y - intercept_point_one.y));
		float intercept_two_distance = sqrt((intercepts[1].x - intercept_point_one.x) * (intercepts[1].x - intercept_point_one.x) + (intercepts[1].y - intercept_point_one.y) * (intercepts[1].y - intercept_point_one.y));
		
		if (intercept_one_distance < intercept_two_distance)
		{
			intercept_point_one = intercepts[0];
			intercept_point_two = intercepts[1];
		}
		else
		{
			intercept_point_one = intercepts[1];
			intercept_point_two = intercepts[0];
		}
		
		return true;
	}
	else return false;
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