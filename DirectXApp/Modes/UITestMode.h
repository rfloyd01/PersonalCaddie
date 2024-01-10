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
	void initializeTextOverlay();

	void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element);

	float m_angle = 0;

};