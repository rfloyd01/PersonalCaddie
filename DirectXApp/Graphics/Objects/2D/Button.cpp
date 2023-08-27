#include "pch.h"
#include "Button.h"

int Button::GetButtonLength()
{
	return this->Length;
}
void Button::SetButtonLength(int _Length)
{
	this->Length = _Length;
}
int Button::GetButtonWidth()
{
	return this->Width;
}
void Button::SetButtonWidth(int _Width)
{
	this->Width = _Width;
}
void Button::SetButtonLengthAndWidth(int _Length, int _Width)
{
	this->Length = _Length;
	this->Width = _Width;
}
void Button::SetButtonLengthAndWidth(DirectX::XMFLOAT2 LenghtWidth)
{
	this->Length = LenghtWidth.x;
	this->Width = LenghtWidth.y;
}

bool Button::inSpace(DirectX::XMFLOAT2 const & mousePosition)
{
	if ((mousePosition.x >= (m_position.x - Length / 2.0)) &&
		(mousePosition.x <= (m_position.x + Length / 2.0)) &&
		(mousePosition.y >= (m_position.y - Width / 2.0)) &&
		(mousePosition.y <= (m_position.y + Width / 2.0)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Button::Render(_In_ ID2D1DeviceContext2* context)
{

	/*switch (this->ObjectState)
	{
	case OS_Default:     ButtonColor = MenuMediumColor; break;
	case OS_HighLighted: ButtonColor = MenuLightColor; break;
	case OS_Clicked:     ButtonColor = MenuDarkColor; break;
	}*/

	const D2D1_RECT_F outline = D2D1::RectF(
		m_position.x - Length / 2.0,
		m_position.y - Width / 2.0,
		m_position.x + Length / 2.0,
		m_position.y + Width / 2.0
	);

	//This acts as a shadow/outline for the main button
	const D2D1_RECT_F outline2 = D2D1::RectF(
		(m_position.x + 2) - (Length - 4) / 2.0,
		(m_position.y + 2) - (Width - 4) / 2.0,
		(m_position.x + 2) + (Length - 4) / 2.0,
		(m_position.y + 2) + (Width - 4) / 2.0
	);

	const D2D1_RECT_F fill = D2D1::RectF(
		m_position.x - Length / 2.0,
		m_position.y - Width / 2.0,
		m_position.x + Length / 2.0,
		m_position.y + Width / 2.0
	);

	//Create brushes
	winrt::com_ptr<ID2D1SolidColorBrush> fill_brush = nullptr, outline_brush = nullptr;
	winrt::check_hresult(
		context->CreateSolidColorBrush(
			D2D1::ColorF(1.0, 0, 0, 1.0),
			fill_brush.put()
		)
	);

	winrt::check_hresult(
		context->CreateSolidColorBrush(
			D2D1::ColorF(0, 0, 0, 1.0),
			outline_brush.put()
		)
	);

	context->DrawRectangle(outline, outline_brush.get(), 2.5f);
	context->DrawRectangle(outline2, outline_brush.get(), 2.5f);
	context->FillRectangle(fill, fill_brush.get());
}

void Button::update(DirectX::XMFLOAT2 mousePosition, bool mouseClick)
{
	
	if (this->inSpace(mousePosition))
	{
		if (mouseClick)
		{
			OutputDebugString(L"The button was clicked!!");
		}
	}
}

void Button::PostRender()
{
}
void Button::OnClick()
{
}