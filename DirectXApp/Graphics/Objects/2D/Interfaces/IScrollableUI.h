#pragma once

#include "IHoverableUI.h"

//A pure virtual class meant to be used by UI Elements that
//have scrolling capabilities
struct IScrollableUI : public IHoverableUI
{
	virtual void onScrollUp() = 0;
	virtual void onScrollDown() = 0;
};