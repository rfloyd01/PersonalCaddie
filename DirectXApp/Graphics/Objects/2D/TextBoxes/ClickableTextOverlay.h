#pragma once

#include "HighlightableTextOverlay.h"

//The clickable text overlay is simply a highlightable text overlay that
//also has the ability to be clicked. A good use for this class is creating
//large clickable text where a button simply won't do, such as for switching
//modes on the various menu screens.

class ClickableTextOverlay : public HighlightableTextOverlay, IClickableUI
{
public:
	ClickableTextOverlay(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification, bool useAbsolute = true)
		: HighlightableTextOverlay(windowSize, location, size, message, fontSize, colors, colorLocations, justification, useAbsolute)
	{
		//Simply use the HighlightableTextOverlay's constructor and then set
		//the m_clickable boolean to true
		m_isClickable = true;
	}

	ClickableTextOverlay() {} //empty default constructor

protected:

	//For now, clicking on the ClickableTextOverlay doesn't have an effect
	//within the element itself. It will set the click and release flags
	//in the element's state though, which will then allow the current mode
	//to take any action it needs to.
	virtual void onMouseClick() override
	{

	}
	virtual void onMouseRelease() override
	{

	}
	virtual void onMouseRightClick() override
	{

	}

};