#pragma once

#include "Button.h"

class ButtonWParim : public Button
{
public:
	ButtonWParim(int ParimType, int ParimDir, int x, int y, int L, int W, DWORD _ParimColor, int _ObjectState)
	{
		this->SetParimType(ParimType);
		this->SetParimDir(ParimDir);
		this->SetObjectPos(x, y);
		this->SetButtonLengthAndWidth(L, W);
		this->SetParimColor(_ParimColor);
		this->SetObjectState(_ObjectState);
		this->ClickOnOFF = false;
		this->LastChangeTimeStamp = 0;
	}
	~ButtonWParim() {}

protected:
	int   ParimType;
	int   ParimDir;
	DWORD ParimColor;
	bool  ClickOnOFF;

public:
	int     GetParimType();
	void    SetParimType(int _ParimType);

	int     GetParimDir();
	void    SetParimDir(int _ParimDir);

	int     GetParimColor();
	void    SetParimColor(DWORD _ParimColor);

	bool    isClickOnOFF();
	void    SetClickOnOFF(bool SetVal);

	void    PostRender();

	virtual void    OnClick();
};