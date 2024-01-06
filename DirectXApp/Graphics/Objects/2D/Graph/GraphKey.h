#pragma once

#include "Graphics/Objects/2D/BasicElements/Line.h"
#include "Graphics/Objects/2D/BasicElements/Ellipse.h"

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
	GraphKey() {} //default no-args constructor

	void addLine(Line const& l) { p_children.push_back(std::make_shared<Line>(l)); }
	void addEllipse(Ellipse const& e) { p_children.push_back(std::make_shared<Ellipse>(e)); }
};