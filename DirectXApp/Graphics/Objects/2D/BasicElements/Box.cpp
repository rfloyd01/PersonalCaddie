#include "pch.h"
#include "Box.h"

Box::Box(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color, UIShapeFillType fill)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//simply create a ui rectangle with no fill using the given color and then resize it based on the size
	//of the current window.
	//D2D1_RECT_F const& rectangle, UIColor color, UIShapeFillType fillType, UIShapeType shapeType = UIShapeType::RECTANGLE
	m_shape = { {0, 0, 0, 0}, color, fill };
	resize();
}

float Box::fixSquareBoxDrift()
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
	return m_size.x * (m_screenSize->Width - m_screenSize->Height) / (2.0f * m_screenSize->Width);
}