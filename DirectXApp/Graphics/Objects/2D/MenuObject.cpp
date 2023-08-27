#include "pch.h"
#include "MenuObject.h"

DirectX::XMFLOAT2 MenuObject::GetObjectPos()
{
	return this->ObjectPos;
}
void MenuObject::SetObjectPos(int x, int y)
{
	this->ObjectPos.x = x;
	this->ObjectPos.y = y;
}
void MenuObject::SetObjectPos(DirectX::XMFLOAT2 Pos)
{
	this->ObjectPos = Pos;
}
int MenuObject::GetObjectState()
{
	return this->ObjectState;
}
void MenuObject::SetObjectState(int State)
{
	this->ObjectState = State;
}
void MenuObject::OnMessage(std::wstring Msg, bool doRENDER)
{
}