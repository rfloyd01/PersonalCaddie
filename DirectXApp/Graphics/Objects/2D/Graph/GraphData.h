#pragma once

#include "Graphics/Objects/2D/BasicElements/Line.h"
#include "Graphics/Objects/2D/BasicElements/Ellipse.h"

//This class represents multiple points of data that are to 
//be rendered on a graph. These points can be connected by
//lines or simply be floating. This class isn't a UIElement
//per se, it's more a container for related UIElements that compose a graph.
//This class is handy when we want to alter an entire set of data
//in some way, whether it's making the data invisible, changing 
//the scale of the data, etc.

class GraphData : public UIElement
{
public:
	GraphData() {} //default no-args constructor

	void addLine(Line const& l) { p_children.push_back(std::make_shared<Line>(l)); }
	void addEllipse(Ellipse const& e) { p_children.push_back(std::make_shared<Ellipse>(e)); }
};