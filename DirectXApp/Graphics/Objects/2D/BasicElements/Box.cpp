#include "pch.h"
#include "Box.h"

Box::Box(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
	UIColor color, UIShapeFillType fill, bool useAbsolute)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//simply create a ui rectangle with no fill using the given color and then resize it based on the size
	//of the current window.
	//D2D1_RECT_F const& rectangle, UIColor color, UIShapeFillType fillType, UIShapeType shapeType = UIShapeType::RECTANGLE
	m_shape = { {0, 0, 0, 0}, color, fill };
	resize();
}