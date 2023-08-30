#pragma once

//A pure virtual class meant to be used by UI Elements that
//need to detect when the mouse is hovering over them
struct IHoverableUI
{
	virtual bool checkHover(DirectX::XMFLOAT2 mousePosition) = 0; //checks to see if the element is being hovered over
	virtual void onHover() = 0;    //performs an action if the element is being hovered over
};