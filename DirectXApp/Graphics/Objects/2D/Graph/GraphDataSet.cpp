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

uint32_t GraphDataSet::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if any check boxes inside
	//of the key of the data set have been clicked. If so, toggle the visibility of 
	//the appropriate graph data.
	uint32_t currentState = UIElement::update(inputState);

	if (m_hasKey)
	{
		auto keyChildren = p_children.back()->getChildren();
		int graph_data_number = 0;
		for (int i = 0; i < keyChildren.size(); i++)
		{
			auto check_box = dynamic_cast<CheckBox*>(keyChildren[i].get());
			if (check_box != nullptr)
			{
				//For each check box, check the physical Graph Data object to 
				//see if the visibility matches what the check box implies. Since
				//data is added in order, there's no need to do this via a loop.
				std::wstring data_name = keyChildren[i - 1]->getText()->message;
				auto graph_data = p_children[2 * (m_vertical_grid_lines + m_horizontal_grid_lines) + graph_data_number];

				if (check_box->isChecked() && (graph_data->getState() & UIElementState::Invisible))
				{
					//The check box was just checked so make the graph data visible
					graph_data->removeState(UIElementState::Invisible);
				}
				else if (!check_box->isChecked() && !(graph_data->getState() & UIElementState::Invisible))
				{
					//The check box was just un-checked so make the graph data invisible
					graph_data->updateState(UIElementState::Invisible);
				}

				graph_data_number++; //iterate to the next graph data child
			}
		}
	}

	return currentState;
}

void GraphDataSet::addGraphData(GraphData const& data)
{
	if (m_hasKey)
	{
		//If the graph data set has a key then add this new line
		//to it. The key should be the last child element in the
		//array so this new graph data is added just in front of
		//the key
		((GraphKey*)p_children.back().get())->addGraphData(data);
		p_children.insert(p_children.end() - 1, std::make_shared<GraphData>(data));
	}
	else p_children.push_back(std::make_shared<GraphData>(data));
}

void GraphDataSet::addKey(GraphKey const& key)
{
	//A graph data set can only have 1 key, so make sure it doesn't have one 
	//already before adding
	if (!m_hasKey)
	{
		p_children.push_back(std::make_shared<GraphKey>(key));
		m_hasKey = true;
	}
}