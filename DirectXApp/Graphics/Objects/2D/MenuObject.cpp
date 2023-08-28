#include "pch.h"
#include "MenuObject.h"

std::vector<DirectX::XMFLOAT2> const& MenuObject::getObjectLocations()
{
	return this->m_locations;
}
void MenuObject::setObjectPosition(int i, DirectX::XMFLOAT2 pos)
{
	//moves a single one of the objects that composes the overall
	//menu object.
	if (i < 0 || i >= m_locations.size()) return; //index out of range so do nothing

	m_locations[i].x = pos.x;
	m_locations[i].y = pos.y;
}
void MenuObject::moveObject(DirectX::XMFLOAT2 pos)
{
	//Move all objects composing the menuObject by the amount given. This method
	//doesn't move TO a location, it moves BY the given amount. This means that 
	//negative numbers are allowed here.
	for (int i = 0; i < m_locations.size(); i++)
	{
		setObjectPosition(i, { m_locations[i].x + pos.x, m_locations[i].y + pos.y });
	}
}

std::vector<MenuObjectState> const& MenuObject::getObjectStates()
{
	return m_states;
}

void MenuObject::setObjectState(int i, MenuObjectState state)
{
	//sets the state of a single one of the objects that composes
	//the overall menu object
	if (i < 0 || i >= m_locations.size()) return; //index out of range so do nothing

	m_states[i] = state;
}

std::vector<DirectX::XMFLOAT2> const& MenuObject::getDimensions()
{
	return m_dimensions;
}

void MenuObject::changeDimensions(DirectX::XMFLOAT2 ratios)
{
	//increases or decreases the dimensions of all objects composing the 
	//menu object by the given ratio
	for (int i = 0; i < m_dimensions.size(); i++)
	{
		m_dimensions[i].x *= ratios.x;
		m_dimensions[i].y *= ratios.y;
	}
}

std::wstring MenuObject::getText()
{
	return m_text;
}

void MenuObject::updateText(std::wstring text)
{
	m_text = text;
}