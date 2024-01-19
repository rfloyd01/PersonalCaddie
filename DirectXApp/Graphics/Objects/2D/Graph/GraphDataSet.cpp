#include "pch.h"
#include "GraphDataSet.h"
#include "Graphics/Objects/2D/TextBoxes/TextBox.h"

GraphDataSet::GraphDataSet(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
	DirectX::XMFLOAT2 minimalDataPoint, DirectX::XMFLOAT2 maximalDataPoint)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	m_minimalDataPoint = minimalDataPoint;
	m_maximalDataPoint = maximalDataPoint;

	m_vertical_grid_lines = 0;
	m_horizontal_grid_lines = 0;

	m_state &= ~UIElementState::Dummy; //elements created without the default constructor have the dummy flag removed
	m_state |= UIElementState::Idlee; //Graph Data can't be interacted with, setting this flag will skip the update loop for the data set and all children
}

void GraphDataSet::addGridLines(int vertical_grid_lines, int horizontal_grid_lines, DirectX::XMFLOAT2 absoluteGraphMaximums, DirectX::XMFLOAT2 absoluteGraphMinimums)
{
	//This method will add vertical and horizontal grid lines to the graph and add labels for each one.
	//The absoluteGraphWidth parameter holds the absolute locations for the left and right edges of the graph
	//while the absoluteGraphHeight holds the absolute locations for the top and bottom of the graph.
	m_vertical_grid_lines = vertical_grid_lines;
	m_horizontal_grid_lines = horizontal_grid_lines;

	//Add the vertical lines and labels first
	float line_spacing = (absoluteGraphMaximums.x - absoluteGraphMinimums.x) / ((float)vertical_grid_lines + 1);
	float data_spacing = (m_maximalDataPoint.x - m_minimalDataPoint.x) / ((float)vertical_grid_lines + 1);
	float location = absoluteGraphMinimums.x, label = m_minimalDataPoint.x;
	for (int i = 1; i <= vertical_grid_lines; i++)
	{
		location += line_spacing;
		label += data_spacing;
		Line grid_line(m_screenSize, { location, absoluteGraphMaximums.y }, { location, absoluteGraphMinimums.y }, UIColor::Black, 0.5f);
		TextBox line_label(m_screenSize, { location, absoluteGraphMaximums.y + 0.014f }, { 0.025f, 0.018f }, std::to_wstring(label), 0.008f / 0.018f, { UIColor::Black },
			{}, UITextJustification::CenterCenter, UIColor::White, UIColor::White, UIColor::White, 0.0f);

		p_children.push_back(std::make_shared<Line>(grid_line));
		p_children.push_back(std::make_shared<TextBox>(line_label));
	}

	//Add the horizontal lines and labels second
	line_spacing = (absoluteGraphMaximums.y - absoluteGraphMinimums.y) / ((float)horizontal_grid_lines + 1);
	data_spacing = (m_maximalDataPoint.y - m_minimalDataPoint.y) / ((float)horizontal_grid_lines + 1);
	location = absoluteGraphMinimums.y, label = m_maximalDataPoint.y;
	for (int i = 1; i <= horizontal_grid_lines; i++)
	{
		location += line_spacing;
		label -= data_spacing;
		Line grid_line(m_screenSize, { absoluteGraphMaximums.x, location }, { absoluteGraphMinimums.x, location }, UIColor::Black, 0.5f);
		TextBox line_label(m_screenSize, { absoluteGraphMinimums.x - 0.0175f, location }, { 0.025f, 0.018f }, std::to_wstring(label), 0.008f / 0.018f, { UIColor::Black },
			{}, UITextJustification::CenterCenter, UIColor::White, UIColor::White, UIColor::White, 0.0f);

		p_children.push_back(std::make_shared<Line>(grid_line));
		p_children.push_back(std::make_shared<TextBox>(line_label));
	}
}