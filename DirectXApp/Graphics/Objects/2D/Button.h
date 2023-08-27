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

public:
	DWORD  LastChangeTimeStamp;

	int     GetButtonLength();
	void    SetButtonLength(int _Length);

	int     GetButtonWidth();
	void    SetButtonWidth(int _Width);

	void    SetButtonLengthAndWidth(int _Length, int _Width);
	void    SetButtonLengthAndWidth(DirectX::XMFLOAT2 LenghtWidth);

	//bool    inSpace(Message Msg);
	virtual void    Render(_In_ ID2D1DeviceContext2* context) override;

	//virtual void    OnMessage(Message Msg, bool doRENDER);

	virtual void    PostRender();
	virtual void    OnClick();
};