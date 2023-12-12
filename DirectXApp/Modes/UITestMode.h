#pragma once

#include "Mode.h"

class UITestMode : public Mode
{
public:
	UITestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

};