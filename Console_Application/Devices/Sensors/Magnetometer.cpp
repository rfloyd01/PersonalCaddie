#include "pch.h"
#include <iostream>

#include "Magnetometer.h"

Magnetometer::Magnetometer(magnetometer_model_t mag_model, uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->mag_model = mag_model;
	
	populateSensorSettingsArray(current_settings);
	setConversionRateFromSettings();
	setCurrentODRFromSettings();
}

Magnetometer::Magnetometer(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->mag_model = static_cast<magnetometer_model_t>(current_settings[SENSOR_MODEL]);

	populateSensorSettingsArray(current_settings);
	setConversionRateFromSettings();
	setCurrentODRFromSettings();
}

void Magnetometer::populateSensorSettingsArray(uint8_t* current_settings)
{
	//Just copy the relevant info over
	for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) this->settings[setting] = current_settings[setting];
}

void Magnetometer::getCalibrationNumbers()
{
	Sensor::getCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file

	//Update calibration numbers obtained from the calibration file here
}
void Magnetometer::setCalibrationNumbers()
{
	Sensor::setCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file
	//will set the calibration numbers for the particular sensor
}

void Magnetometer::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_MAG:
		this->conversion_rate = lsm9ds1_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]);
		break;
	default:
		this->conversion_rate = 0;
		break;
	}
}

void Magnetometer::setCurrentODRFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_MAG:
		this->current_odr = lsm9ds1_odr_calculate(0x00, this->settings[ODR]); //the 0x00 represents IMU off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}