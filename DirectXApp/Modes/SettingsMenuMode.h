#pragma once

#include "Mode.h"

class SettingsMenuMode : public Mode
{
public:
	SettingsMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
};