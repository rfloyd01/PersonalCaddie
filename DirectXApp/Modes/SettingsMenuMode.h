#pragma once

#include "Mode.h"

class SettingsMenuMode : public Mode
{
public:
	SettingsMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
};