#pragma once

#include "Mode.h"
#include "../Firmware/MEMs_Drivers/sensor_settings.h"

enum IMUSettingsState
{
	DISPLAY_SETTINGS = 1, //overwrite the active mode state
};

class IMUSettingsMode : public Mode
{
public:
	IMUSettingsMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	void getCurrentSettings(std::vector<uint8_t*> settings);

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	uint8_t m_currentSettings[SENSOR_SETTINGS_LENGTH]; //an array holding the current settings for the IMU
	uint8_t m_newSettings[SENSOR_SETTINGS_LENGTH]; //an array holding the new settings for the IMU
};