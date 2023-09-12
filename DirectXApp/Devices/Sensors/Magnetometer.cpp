#include "pch.h"
#include <iostream>

#include "Magnetometer.h"

Magnetometer::Magnetometer(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->mag_model = static_cast<magnetometer_model_t>(current_settings[SENSOR_MODEL]);

	setCurrentSettings(current_settings);

	//set up calibration info
	this->cal_offset_number = 3; //magnetometers need 3 axis offset values
	this->cal_gains_number = 9; //magnetometers need 9 cross-axis gain values
	this->calibrationFile = "Resources/Calibration_Files/magnetometer_calibration.txt";
	getCalibrationNumbersFromTextFile();
}

void Magnetometer::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_MAG:
		this->conversion_rate = lsm9ds1_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]) / 10; //convert from mgauss to uTesla
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