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
//bool Button::inSpace(Message Msg)
//{
//	if (Msg.CursorPos._x >= this->ObjectPos._x &&
//		Msg.CursorPos._x <= this->ObjectPos._x + this->Length &&
//		Msg.CursorPos._y >= this->ObjectPos._y &&
//		Msg.CursorPos._y <= this->ObjectPos._y + this->Width)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
void Button::Render(_In_ ID2D1DeviceContext2* context)
{
	/*DWORD ButtonColor = 123;
	switch (this->ObjectState)
	{
	case OS_Default:     ButtonColor = MenuMediumColor; break;
	case OS_HighLighted: ButtonColor = MenuLightColor; break;
	case OS_Clicked:     ButtonColor = MenuDarkColor; break;
	}
	pD3D->FillRGB(this->ObjectPos.x,
		this->ObjectPos.y,
		this->Length,
		this->Width,
		ButtonColor);
	pD3D->DrawBoxWithLines(this->ObjectPos.x,
		this->ObjectPos.y,
		this->Length,
		this->Width,
		0xFF000000);
	pD3D->DrawBoxWithLines(this->ObjectPos.x + 2,
		this->ObjectPos.y + 2,
		this->Length - 4,
		this->Width - 4,
		0xFF000000);
	this->PostRender();*/

	const D2D1_ELLIPSE ell = D2D1::Ellipse(
		D2D1::Point2F(100.0f, 100.0f),
		75.0f,
		50.0f
	);

	const D2D1_RECT_F rect = D2D1::RectF(
		10, 10, 50, 50
	);

	//Create a brush
	winrt::com_ptr<ID2D1SolidColorBrush> new_brush = nullptr;
	winrt::check_hresult(
		context->CreateSolidColorBrush(
			D2D1::ColorF(1.0, 0, 0, 1.0),
			new_brush.put()
		)
	);

	//TODO: Why does the line draw but the rectangle doesn't?
	context->DrawLine(
		D2D1::Point2F(0.0f, 0.0),
		D2D1::Point2F(0.0, 100.0),
		new_brush.get(),
		0.5f
	);
	context->DrawLine(
		D2D1::Point2F(0.0f, 100.0),
		D2D1::Point2F(100.0, 100.0),
		new_brush.get(),
		0.5f
	);
	context->DrawLine(
		D2D1::Point2F(100.0f, 100.0),
		D2D1::Point2F(100.0, 0.0),
		new_brush.get(),
		0.5f
	);
	context->DrawLine(
		D2D1::Point2F(100.0f, 0.0),
		D2D1::Point2F(0.0, 0.0),
		new_brush.get(),
		0.5f
	);
	context->DrawEllipse(ell, new_brush.get(), 2.5f);
	context->FillRectangle(rect, new_brush.get());
	
	new_brush = nullptr;
}
//void Button::OnMessage(Message Msg, bool doRENDER)
//{
//	if (this->inSpace(Msg))
//	{
//		if (Msg.Info == MI_LMouseDown)
//		{
//			this->SetObjectState(OS_Clicked);
//		}
//		else if (Msg.Info == MI_LMouseUp)
//		{
//			if (this->LastChangeTimeStamp != Msg.TimeStamp)
//			{
//				this->LastChangeTimeStamp = Msg.TimeStamp;
//				this->SetObjectState(OS_Default);
//				this->OnClick();
//			}
//			else this->SetObjectState(OS_HighLighted);
//		}
//		else this->SetObjectState(OS_HighLighted);
//	}
//	else this->SetObjectState(OS_Default);
//
//	if (doRENDER)
//		this->Render();
//}
void Button::PostRender()
{
}
void Button::OnClick()
{
}