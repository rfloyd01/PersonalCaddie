#include "pch.h"
#include "GraphKey.h"

GraphKey::GraphKey(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//The Graph Key itself is a pretty plain object, nothing more than an outlined box
	//that sits inside of its parent Graph
	OutlinedBox outline(windowSize, location, size);
	p_children.push_back(std::make_shared<OutlinedBox>(outline));
}

void GraphKey::addGraphData(GraphData const& data)
{
	//When a new line of data is added to the graph we add certain information about it
	//to the key. Mainly, the color of the line, the name of the data and whether or not
	//the data is currently being displayed in the graph.
	auto location = getAbsoluteLocation();
	auto size = getAbsoluteSize();

	float absolute_line_height = size.y / 5.0f; //default to a height that's 1/5 the height of the key
	float absolute_line_location = location.y - (size.y - absolute_line_height) / 2.0f; //default to a height that's 1/5 the height of the key

	if (p_children.size() > 1)
	{
		absolute_line_height = p_children.back()->getAbsoluteSize().y;
		absolute_line_location = p_children.back()->getAbsoluteLocation().y + absolute_line_height;
	}

	Line line(m_screenSize, { location.x - size.x / 2.0f + 0.005f, absolute_line_location }, { location.x - size.x / 2.0f + 0.0175f, absolute_line_location }, data.getDataColor());
	TextOverlay name(m_screenSize, { location.x, absolute_line_location }, { size.x / 2.0f, absolute_line_height }, data.getName(), 0.95f, { UIColor::Black },
		{ 0, data.getName().size() }, UITextJustification::CenterCenter, false);
	CheckBox box(m_screenSize, { location.x + size.x / 3.5f, absolute_line_location }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * absolute_line_height * 0.95f, absolute_line_height * 0.95f }, true);

	p_children.push_back(std::make_shared<Line>(line));
	p_children.push_back(std::make_shared<TextOverlay>(name));
	p_children.push_back(std::make_shared<CheckBox>(box));
}