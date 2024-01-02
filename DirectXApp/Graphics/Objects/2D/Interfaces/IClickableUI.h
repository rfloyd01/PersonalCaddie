#pragma once

#include "IHoverableUI.h"

//A pure virtual class meant to be used by UI Elements that
//have the ability to be clicked
struct IClickableUI
{
	virtual void onMouseClick() = 0;
	virtual void onMouseRelease() = 0;
};