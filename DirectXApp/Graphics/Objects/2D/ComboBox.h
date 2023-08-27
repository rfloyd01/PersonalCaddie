#pragma once

#include "Button.h"

class ComboBox : public MenuObject
{
public:
	ComboBox(int x, int y, int L, int W)
	{
		this->SetObjectPos(x, y);
		this->SetLengthAndWidth(L, W);
		this->ActiveOptionIndex = 0;
		this->OptionIndexMouseOver = 10;

		pButton = new cButtonWParim(BP_Triangle, BP_PointDown, x + L - W, y, W, W, 0xAA000000, OS_Default);
	}
	~ComboBox()
	{
		delete pButton;
	}

protected:
	cButtonWParim* pButton;
	int  ActiveOptionIndex;
	int  OptionIndexMouseOver;
	std::vector<std::string> Options;
	int Length;
	int Width;

public:
	void    PushString(std::string str);
	std::string GetOptionByIndex(int i);

	int     GetLength();
	void    SetLength(int _Length);

	int     GetWidth();
	void    SetWidth(int _Width);

	void    SetLengthAndWidth(int _Length, int _Width);
	void    SetLengthAndWidth(Point LenghtWidth);

	void    inOptionSpaceCalc(Message Msg);
	void    Render();

	void    OnMessage(Message Msg, bool doRENDER);
};