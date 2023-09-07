#pragma once

#include "Mode.h"

class DevelopmentMenuMode : public Mode
{
public:
	DevelopmentMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
};