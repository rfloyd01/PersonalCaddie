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

	virtual uint32_t initializeMode() override;
	virtual void uninitializeMode() override;

	void enterActiveState(int state);

	//Updating and Advancement Functions
private:
	void initializeSettingsModeText();
	DeviceDiscoveryState m_state;

};