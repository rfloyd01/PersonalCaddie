#include "pch.h"

#include <iostream>

#include "Accelerometer.h"

Accelerometer::Accelerometer(uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->acc_model = static_cast<accelerometer_model_t>(current_settings[SENSOR_MODEL]);

	setCurrentSettings(current_settings);
	
	//set the calibration file
	switch (acc_model)
	{
	case LSM9DS1_ACC:
		this->calibrationFile = L"lsm9ds1_acc_calibration.txt";
		this->axisFile = L"lsm9ds1_acc_axes_orientations.txt";
		break;
	case FXOS8700_ACC:
		this->calibrationFile = L"fxos8700_acc_calibration.txt";
		this->axisFile = L"fxos8700_acc_axes_orientations.txt";
		break;
	case BMI270_ACC:
		this->calibrationFile = L"bmi270_acc_calibration.txt";
		this->axisFile = L"bmi270_acc_axes_orientations.txt";
		break;
	}

	//Then attempt to override the default values using the text file
	getCalibrationNumbersFromTextFile();
	getAxisOrientationsFromTextFile();
}

void Accelerometer::setConversionRateFromSettings()
{
	//the conversion rate depends on which model of accelerometer we have, and the current full scale range setting
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_ACC:
		this->conversion_rate = lsm9ds1_fsr_conversion(ACC_SENSOR, this->settings[FS_RANGE]) / 1000.0 * GRAVITY; //convert from mg to m/s^2
		break;
	case FXOS8700_ACC:
		this->conversion_rate = fxos_fxas_fsr_conversion(ACC_SENSOR, this->settings[FS_RANGE]) / 1000.0 * GRAVITY; //convert from mg to m/s^2
		break;
	case BMI270_ACC:
		this->conversion_rate = bmi_bmm_fsr_conversion(ACC_SENSOR, this->settings[FS_RANGE]) * GRAVITY; //convert from g to m/s^2
		break;
	default:
		this->conversion_rate = 0;
		break;
	}
}

void Accelerometer::setCurrentODRFromSettings()
{
	//uint8_t* settings_array, uint8_t acc_model, uint8_t gyr_model, uint8_t mag_model, uint8_t sensor
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_ACC:
		this->current_odr = lsm9ds1_compound_odr_calculate(this->settings[ODR], 0xC0); //the 0xC0 represents magnetometer off mode
		break;
	case FXOS8700_ACC:
		this->current_odr = fxos8700_odr_calculate(FXOS8700_ACC, 0, this->settings[ODR], 0xFF); //the 0xC0 represents magnetometer off mode
		break;
	case BMI270_ACC:
		this->current_odr = bmi270_acc_odr_calculate(this->settings[ODR]); //the 0xC0 represents magnetometer off mode
		break;
	default:
		this->current_odr = 0;
		break;
	}
}