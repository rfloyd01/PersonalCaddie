#pragma once

#include <string>

//used to make and keep track of D2D1 brushes
enum class UIShapeColor
{
	White = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Black = 4,
	END = 5
};

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
	END = 3
};

//This struct holds information about text to be rendered on the screen
struct UIShape
{
	UIShape(D2D1_RECT_F const& rectangle, UIShapeColor color, UIShapeFillType fillType, UIShapeType shapeType) :
		m_rectangle(rectangle),
		m_color(color),
		m_fillType(fillType),
		m_shapeType(shapeType)
	{

	}

	D2D1_RECT_F m_rectangle;          //for now, all UIShapes are simply rectangles
	UIShapeColor m_color; //holds the different colors of the text (in order)
	UIShapeFillType m_fillType; //holds the index of all characters where the text color switches. This vector must be 1 element longer than the colors vector
	UIShapeType m_shapeType;
};