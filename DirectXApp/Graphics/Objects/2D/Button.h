#pragma once

#include "MenuObject.h"

class Button : public MenuObject
{
public:
	Button() {}
	~Button() {}

protected:
	int    Length;
	int    Width;
	int buttonCount = 0;

public:
	DWORD  LastChangeTimeStamp;

	int     GetButtonLength();
	void    SetButtonLength(int _Length);

	int     GetButtonWidth();
	void    SetButtonWidth(int _Width);

	void    SetButtonLengthAndWidth(int _Length, int _Width);
	void    SetButtonLengthAndWidth(DirectX::XMFLOAT2 LenghtWidth);

	bool    inSpace(DirectX::XMFLOAT2 const& mousePosition);
	virtual void Render(_In_ ID2D1DeviceContext2* context) override;

	virtual void update(DirectX::XMFLOAT2 mousePosition, bool mouseClick) override;

	virtual void    PostRender();
	virtual void    OnClick();
};