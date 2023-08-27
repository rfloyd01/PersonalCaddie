#include "pch.h"
#include "MenuObject.h"

DirectX::XMFLOAT2 MenuObject::GetObjectPos()
{
	return this->m_position;
}
void MenuObject::SetObjectPos(int x, int y)
{
	this->m_position.x = x;
	this->m_position.y = y;
}
void MenuObject::SetObjectPos(DirectX::XMFLOAT2 Pos)
{
	this->m_position = Pos;
}
int MenuObject::GetObjectState()
{
	return this->m_state;
}
void MenuObject::SetObjectState(int State)
{
	this->m_state = State;
}