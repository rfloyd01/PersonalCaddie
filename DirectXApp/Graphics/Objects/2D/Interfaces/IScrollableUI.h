#pragma once

//A pure virtual class meant to be used be UI Elements that
//have scrolling capabilities
struct IScrollableUI
{
	virtual void onScrollUp() = 0;
	virtual void onScrollDown() = 0;
};