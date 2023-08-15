#include "pch.h"

#include <iostream>

#include "Accelerometer.h"

Accelerometer::Accelerometer(accelerometer_model_t acc_model, uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->acc_model = acc_model;
	
	populateSensorSettingsArray(current_settings);
	setConversionRateFromSettings();
	setCurrentODRFromSettings();
}

Accelerometer::Accelerometer(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->acc_model = static_cast<accelerometer_model_t>(current_settings[SENSOR_MODEL]);

	populateSensorSettingsArray(current_settings);
	setConversionRateFromSettings();
	setCurrentODRFromSettings();
}

void Accelerometer::populateSensorSettingsArray(uint8_t* current_settings)
{
	//Just copy the relevant info over
	for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) this->settings[setting] = current_settings[setting];
}

void Accelerometer::getCalibrationNumbers()
{
	Sensor::getCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file

	//Update calibration numbers obtained from the calibration file here
}
void Accelerometer::setCalibrationNumbers()
{
	Sensor::setCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file
	//will set the calibration numbers for the particular sensor
}

void Accelerometer::setConversionRateFromSettings()
{
	//the conversion rate depends on which model of accelerometer we have, and the current full scale range setting
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_ACC:
		this->conversion_rate = lsm9ds1_fsr_conversion(ACC_SENSOR, this->settings[FS_RANGE]);
		break;
	default:
		this->conversion_rate = 0;
		break;
	}
}

void Accelerometer::setCurrentODRFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_ACC:
		this->current_odr = lsm9ds1_odr_calculate(this->settings[ODR], 0xC0); //the 0xC0 represents magnetometer off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}