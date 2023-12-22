#pragma once

#include "Mode.h"

class UITestMode : public Mode
{
public:
	UITestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	float m_angle = 0;

};