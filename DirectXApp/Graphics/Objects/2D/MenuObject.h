#pragma once

enum class MenuObjectState
{
	PassiveOutline,
	PassiveBackground,
	Pressed,
	NotPressed
};

class MenuObject
{
public:
	MenuObject() {}
	~MenuObject() {}
protected:
	virtual void setObjectPosition(int i, DirectX::XMFLOAT2 pos);

	std::vector<DirectX::XMFLOAT2>   m_locations;
	std::vector<DirectX::XMFLOAT2>   m_dimensions;
	std::vector<MenuObjectState>     m_states;
	std::wstring                     m_text;

public:
	virtual std::vector<DirectX::XMFLOAT2> const& getObjectLocations();
	
	virtual void moveObject(DirectX::XMFLOAT2 pos);

	virtual std::vector<MenuObjectState> const& getObjectStates();
	virtual void setObjectState(int i, MenuObjectState state);

	virtual std::vector<DirectX::XMFLOAT2> const& getDimensions();
	void changeDimensions(DirectX::XMFLOAT2 ratios);

	std::wstring getText();
	void updateText(std::wstring text);

	virtual void update(DirectX::XMFLOAT2 mousePosition, bool mouseClick, winrt::Windows::Foundation::Size windowSize) = 0;
};