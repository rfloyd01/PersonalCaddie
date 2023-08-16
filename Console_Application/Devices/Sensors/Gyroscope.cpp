#include "pch.h"
#include <iostream>

#include "Gyroscope.h"

Gyroscope::Gyroscope(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->gyr_model = static_cast<gyroscope_model_t>(current_settings[SENSOR_MODEL]);

	populateSensorSettingsArray(current_settings);
	setConversionRateFromSettings();
	setCurrentODRFromSettings();

	//set up calibration info
	this->cal_offset_number = 3; //gyroscopes need 3 axis offset values
	this->cal_gains_number = 9; //gyroscopes need 9 cross-axis gain values
	this->calibrationFile = "Resources/Calibration_Files/gyroscope_calibration.txt";
	getCalibrationNumbersFromTextFile();
}

void Gyroscope::populateSensorSettingsArray(uint8_t* current_settings)
{
	//Just copy the relevant info over
	for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) this->settings[setting] = current_settings[setting];
}

void Gyroscope::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_GYR:
		this->conversion_rate = lsm9ds1_fsr_conversion(GYR_SENSOR, this->settings[FS_RANGE]);
		break;
	default:
		this->conversion_rate = 0;
		break;
	}
}

void Gyroscope::setCurrentODRFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_GYR:
		this->current_odr = lsm9ds1_odr_calculate(this->settings[ODR], 0xC0); //the 0xC0 represents magnetometer off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}