#pragma once

class MenuObject
{
public:
	MenuObject() {}
	~MenuObject() {}
protected:
	DirectX::XMFLOAT2   ObjectPos;
	int                 ObjectState;

public:
	virtual DirectX::XMFLOAT2 GetObjectPos();
	virtual void SetObjectPos(int x, int y);
	virtual void SetObjectPos(DirectX::XMFLOAT2 Pos);

	virtual int GetObjectState();
	virtual void SetObjectState(int State);

	virtual void OnMessage(std::wstring Msg, bool doRENDER);
	virtual void Render(_In_ ID2D1DeviceContext2* context) = 0;
};