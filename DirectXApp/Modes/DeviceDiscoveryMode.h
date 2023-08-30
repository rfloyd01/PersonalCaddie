#pragma once

#include "Mode.h"

enum class DeviceDiscoveryState
{
	IDLE,
	DISCOVERY,
	DISCONNECT
};

class DeviceDiscoveryMode : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	DeviceDiscoveryMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	//Updating and Advancement Functions
private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	DeviceDiscoveryState m_state;

};