#pragma once

#include "Mode.h"

class MadgwickTestMode : public Mode
{
public:
	MadgwickTestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	virtual void update() override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	float m_currentRotation;
};