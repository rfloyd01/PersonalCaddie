#pragma once

#include "MenuObject.h"

class Button : public MenuObject
{
public:
	Button(DirectX::XMFLOAT2 location, std::wstring text = L"");
	~Button() {}

protected:
	int buttonCount = 0;

public:
	DWORD  LastChangeTimeStamp;

	bool    inSpace(DirectX::XMFLOAT2 const& mousePosition, winrt::Windows::Foundation::Size windowSize);

	virtual MenuObjectState update(DirectX::XMFLOAT2 mousePosition, bool mouseClick, winrt::Windows::Foundation::Size windowSize) override;

	virtual MenuObjectState getReleventState() override;
	virtual void setReleventState(MenuObjectState state) override;
};