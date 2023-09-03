#pragma once

#include "UIElement.h"
#include "Interfaces/IClickableUI.h"

class UIButton : public UIElement, IClickableUI, IHoverableUI
{
public:
	UIButton(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, winrt::Windows::Foundation::Size windowSize, std::wstring text = L"");
	~UIButton() {}

	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;
	virtual UIElementState update(InputState* inputState) override;

	void updateButtonText(std::wstring text);
	virtual void setState(UIElementState state) override;

protected:
	//virtual bool checkHover(DirectX::XMFLOAT2 mousePosition) override;
	bool checkHover(DirectX::XMFLOAT2 mousePosition);
	virtual void onHover() override;
	virtual void onClick() override;

	float m_fontSize;
};