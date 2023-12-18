#pragma once

#include "Mode.h"

class MainMenuMode : public Mode
{
public:
	MainMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
};