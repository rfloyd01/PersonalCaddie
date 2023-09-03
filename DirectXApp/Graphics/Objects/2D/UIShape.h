#pragma once

#include <string>

//used to select the correct D2D1 drawing method
enum class UIShapeFillType
{
	Fill,
	NoFill
};

enum class UIShapeType
{
	RECTANGLE = 0,
	ELLIPSE = 1,
	TRIANGLE = 2,
	LINE = 3,
	END = 4
};

enum class UIColor;

//This struct holds information about text to be rendered on the screen
struct UIShape
{
	//constructor defaults to a rectangle shape
	UIShape(D2D1_RECT_F const& rectangle, UIColor color, UIShapeFillType fillType, UIShapeType shapeType = UIShapeType::RECTANGLE) :
		m_rectangle(rectangle),
		m_color(color),
		m_fillType(fillType),
		m_shapeType(shapeType)
	{

	}

	UIShape() { m_shapeType = UIShapeType::END; }; //this tells other parts of the program that this UIShape hasn't been initialized yet

	D2D1_RECT_F m_rectangle;          //for now, all UIShapes are simply rectangles
	UIColor m_color; //holds the different colors of the text (in order)
	UIShapeFillType m_fillType; //holds the index of all characters where the text color switches. This vector must be 1 element longer than the colors vector
	UIShapeType m_shapeType;
};