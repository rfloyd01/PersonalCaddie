#include "pch.h"
#include "Box.h"

Box::Box(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color, UIShapeFillType fill, bool isSquare)
{
	m_location = location;
	m_size = size;
	m_isSquare = isSquare;

	//if (m_isSquare) m_size.x = m_size.y; //If the box is a square, set the width to be equal to the height

	//simply create a ui rectangle with no fill using the given color and then resize it based on the size
	//of the current window.
	//D2D1_RECT_F const& rectangle, UIColor color, UIShapeFillType fillType, UIShapeType shapeType = UIShapeType::RECTANGLE
	m_shape = { {0, 0, 0, 0}, color, fill };
	resize(windowSize);
}

void Box::resize(winrt::Windows::Foundation::Size windowSize)
{
	//if the button is a square than we resize both dimensions off of the height, otherwise
	//we just use the normal resize() method.
	if (m_isSquare)
	{
		if (m_shape.m_shapeType != UIShapeType::END)
		{
			m_shape.m_rectangle.left = windowSize.Width * m_location.x - windowSize.Height * m_size.x / (float)2.0;
			m_shape.m_rectangle.top = windowSize.Height * (m_location.y - m_size.y / (float)2.0);
			m_shape.m_rectangle.right = windowSize.Width * m_location.x + windowSize.Height * m_size.x / (float)2.0;
			m_shape.m_rectangle.bottom = windowSize.Height * (m_location.y + m_size.y / (float)2.0);
		}

		if (m_text.textType != UITextType::END)
		{
			m_text.startLocation = { windowSize.Width * m_location.x - windowSize.Height * m_size.x / (float)2.0, windowSize.Height * (m_location.y - m_size.y / (float)2.0) }; //text always starts at the top left of the UI Element
			m_text.renderArea = { windowSize.Height * m_size.x, windowSize.Height * m_size.y };
			m_text.fontSize = windowSize.Height * m_fontSize;
		}

		for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(windowSize);
	}
	else UIElementBasic::resize(windowSize);
}

float Box::fixSquareBoxDrift(winrt::Windows::Foundation::Size const& currentWindowSize)
{
	//Since the x-dimensions for square boxes aren't tied to the width of the screen,
	//then resizing the screen can cause boxes that need to be tied to a certain relative
	//x-dimension will appear to be drifting. For example, an arrow button that should always be
	//at the right side of a scroll box. The square boxes aren't actually drifting, but since their
	//widths won't change equally relative to the things around them it will appear that they are.
	//This effect can be compensated for by physically moving square boxes as the screen
	//is resized. If the box is just floating on its own out in space then this compensation isn't
	//necessary, it's only when the box needs to have either it's left or right side in the 
	//same location relative to something that isn't a square.
	return m_size.x * (currentWindowSize.Width - currentWindowSize.Height) / (2.0f * currentWindowSize.Width);
}