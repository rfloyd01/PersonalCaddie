#pragma once

#include "GraphData.h"
#include "Graphics/Objects/2D/BasicElements/OutlinedBox.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"
#include "Graphics/Objects/2D/Buttons/CheckBox.h"

/*
The GraphKey class is a class for creating dynamic keys for graphs.
The key holds information about each line currently visible on the graph,
as well as allowing the user the ability to toggle the visibility of 
each line individually. If a graph gets zoomed in on, making an existing
lint of data become invisible, that line will be automatically removed
from the key.
*/

class GraphKey : public UIElement
{
public:
	GraphKey(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size);

	void addGraphData(GraphData const& data);
};