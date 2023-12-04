#include "pch.h"
#include <iostream>

#include "Gyroscope.h"

Gyroscope::Gyroscope(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->gyr_model = static_cast<gyroscope_model_t>(current_settings[SENSOR_MODEL]);

	setCurrentSettings(current_settings);

	//set the calibration file
	switch (gyr_model)
	{
	case LSM9DS1_GYR:
		this->calibrationFile = L"lsm9ds1_gyr_calibration.txt";
		this->axisFile = L"lsm9ds1_gyr_axes_orientations.txt";
		break;
	case FXAS21002_GYR:
		this->calibrationFile = L"fxas21002_gyr_calibration.txt";
		this->axisFile = L"fxas21002_gyr_axes_orientations.txt";
		break;
	case BMI270_GYR:
		this->calibrationFile = L"bmi270_gyr_calibration.txt";
		this->axisFile = L"bmi270_gyr_axes_orientations.txt";
		break;
	}

	getCalibrationNumbersFromTextFile();
	getAxisOrientationsFromTextFile();
}

void Gyroscope::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_GYR:
		this->conversion_rate = lsm9ds1_fsr_conversion(GYR_SENSOR, this->settings[FS_RANGE]) / 1000.0; //convert from mdps to dps
		break;
	case FXAS21002_GYR:
		this->conversion_rate = fxos_fxas_fsr_conversion(GYR_SENSOR, this->settings[FS_RANGE]) / 1000.0; //convert from mdps to dps
		break;
	case BMI270_GYR:
		this->conversion_rate = bmi_bmm_fsr_conversion(GYR_SENSOR, this->settings[FS_RANGE]); //given in dps, no need for conversion
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
		this->current_odr = lsm9ds1_compound_odr_calculate(this->settings[ODR], 0xC0); //the 0xC0 represents magnetometer off mode
		break;
	case FXAS21002_GYR:
		this->current_odr = fxas21002_odr_calculate(FXAS21002_GYR, this->settings[ODR]); //the 0xC0 represents magnetometer off mode
		break;
	case BMI270_GYR:
		this->current_odr = bmi270_gyr_odr_calculate(this->settings[ODR]); //the 0xC0 represents magnetometer off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}