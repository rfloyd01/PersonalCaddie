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
		break;
	case FXOS8700_MAG:
		this->calibrationFile = L"fxos8700_mag_calibration.txt";
		break;
	}
	
	getCalibrationNumbersFromTextFile();
}

void Magnetometer::setConversionRateFromSettings()
{
	switch (this->settings[SENSOR_MODEL])
	{
	case LSM9DS1_MAG:
		this->conversion_rate = lsm9ds1_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]) / 10; //convert from mgauss to uTesla
		break;
	case FXOS8700_MAG:
		this->conversion_rate = fxos_fxas_fsr_conversion(MAG_SENSOR, this->settings[FS_RANGE]) / 1000.0 * GRAVITY; //convert from mg to m/s^2
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

std::wstring Magnetometer::convertCalNumbersToText()
{
	std::wstring calText = L"";

	//First add the offsets
	calText += std::to_wstring(calibration_offsets[0]) + L"\n";
	calText += std::to_wstring(calibration_offsets[1]) + L"\n";
	calText += std::to_wstring(calibration_offsets[2]) + L"\n\n";

	//Then the gains
	calText += std::to_wstring(calibration_gain_x[0]) + L"\n";
	calText += std::to_wstring(calibration_gain_x[1]) + L"\n";
	calText += std::to_wstring(calibration_gain_x[2]) + L"\n";
	calText += std::to_wstring(calibration_gain_y[0]) + L"\n";
	calText += std::to_wstring(calibration_gain_y[1]) + L"\n";
	calText += std::to_wstring(calibration_gain_y[2]) + L"\n";
	calText += std::to_wstring(calibration_gain_z[0]) + L"\n";
	calText += std::to_wstring(calibration_gain_z[1]) + L"\n";
	calText += std::to_wstring(calibration_gain_z[2]) + L"\n";

	return calText;
}