#pragma once

#include "Mode.h"
#include "../Firmware/MEMs_Drivers/sensor_settings.h"

class IMUSettingsMode : public Mode
{
public:
	IMUSettingsMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

	void getCurrentSettings(std::vector<uint8_t*> settings, std::vector<uint8_t> const& availableSensors, bool use_current = false);
	uint8_t* getNewSettings() { return m_newSettings; }

	bool dropDownsSet;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	void createDropDownMenus(winrt::Windows::Foundation::Size windowSize, bool use_current = false);

	void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element);

	void getAvailableSensors();
	void populateDropDownText();

	void updateSetting(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);

	std::wstring getSettingString(int setting);

	uint8_t convertStringToHex(std::wstring hexString);

	uint8_t m_currentSettings[SENSOR_SETTINGS_LENGTH]; //an array holding the current settings for the IMU
	uint8_t m_newSettings[SENSOR_SETTINGS_LENGTH]; //an array holding the new settings for the IMU
	std::vector<uint8_t> m_internalSensors; //a vector holding the sensor addresses available on the Personal Caddie internal TWI bus
	std::vector<uint8_t> m_externalSensors; //a vector holding the sensor addresses available on the Personal Caddie external TWI bus

	std::vector<std::vector<std::wstring> > m_dropDownText;
	int m_accFirstDropDown, m_gyrFirstDropDown, m_magFirstDropDown;
	std::vector<int> m_dropDownCategories; //Not all sensors feature all setting types, this array matches each drop down to its setting type
};