#pragma once

#include "MenuObject.h"

class Button : public MenuObject
{
public:
	Button(DirectX::XMFLOAT2 location);
	~Button() {}

protected:
	int buttonCount = 0;

public:
	DWORD  LastChangeTimeStamp;

	bool    inSpace(DirectX::XMFLOAT2 const& mousePosition, winrt::Windows::Foundation::Size windowSize);

	virtual void update(DirectX::XMFLOAT2 mousePosition, bool mouseClick, winrt::Windows::Foundation::Size windowSize) override;

	virtual void    PostRender();
	virtual void    OnClick();
};