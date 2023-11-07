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

	//set the calibration file
	switch (mag_model)
	{
	case LSM9DS1_MAG:
		this->calibrationFile = L"lsm9ds1_mag_calibration.txt";
		this->axisFile = L"lsm9ds1_mag_axes_orientations.txt";
		break;
	case FXOS8700_MAG:
		this->calibrationFile = L"fxos8700_mag_calibration.txt";
		this->axisFile = L"fxos8700_mag_axes_orientations.txt";
		break;
	}
	
	getCalibrationNumbersFromTextFile();
	getAxisOrientationsFromTextFile();
}

void Magnetometer::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_MAG:
		this->conversion_rate = lsm9ds1_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]) / 10; //convert from mgauss to uTesla
		break;
	case FXOS8700_MAG:
		this->conversion_rate = fxos_fxas_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]); //uTesla
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
		this->current_odr = lsm9ds1_compound_odr_calculate(0x00, this->settings[ODR]); //the 0x00 represents IMU off mode
		break;
	case FXOS8700_MAG:
		this->current_odr = fxos8700_odr_calculate(0, FXOS8700_MAG, 0xFF, this->settings[ODR]); //the 0xC0 represents magnetometer off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}