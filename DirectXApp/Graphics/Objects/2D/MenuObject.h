#pragma once

class MenuObject
{
public:
	MenuObject() {}
	~MenuObject() {}
protected:
	DirectX::XMFLOAT2   m_position;
	int                 m_state;

public:
	virtual DirectX::XMFLOAT2 GetObjectPos();
	virtual void SetObjectPos(int x, int y);
	virtual void SetObjectPos(DirectX::XMFLOAT2 Pos);

	virtual int GetObjectState();
	virtual void SetObjectState(int State);

	virtual void update(DirectX::XMFLOAT2 mousePosition, bool mouseClick) = 0;
	virtual void Render(_In_ ID2D1DeviceContext2* context) = 0;
};