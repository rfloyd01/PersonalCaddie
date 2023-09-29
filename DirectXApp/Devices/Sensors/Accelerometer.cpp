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
	
	//set up calibration info
	this->cal_offset_number = 3; //accelerometers need 3 axis offset values
	this->cal_gains_number = 9; //accelerometers need 9 cross-axis gain values
	this->calibrationFile = "Resources/Calibration_Files/accelerometer_calibration.txt";
	getCalibrationNumbersFromTextFile();
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
	default:
		this->current_odr = 0;
		break;
	}
}

//void Accelerometer::getCalibrationNumbersFromTextFile()
//{
//	//this method takes calibration numbers that were obtained in a previous session (if they exist) and updates the 
//	//local calibration variables with them. If no previous calibration numbers exist then default values are used.
//	std::vector<float> offsets, cross_axis_gains;
//
//	//open the external calibration file, if it can't be found then use default calibration numbers
//	try
//	{
//		int line_count = 0;
//		std::fstream inFile;
//		inFile.open("Resources/accelerometer_calibration.txt");
//		char cal_value[256];
//
//		//the three offset values come first in the calibration file
//		while (offsets.size() < 3 && !inFile.eof())
//		{
//			inFile.getline(cal_value, 256);
//
//			//the calibration file has both text and float values so check to see if the next float value has been encountered
//			try { offsets.push_back(std::stof(cal_value)); }
//			catch(...) {}//intentionally left blank
//		}
//
//		//followed by the nine cross-axis gain values
//		while (cross_axis_gains.size() < 9 && !inFile.eof())
//		{
//			inFile.getline(cal_value, 256);
//
//			//the calibration file has both text and float values so check to see if the next float value has been encountered
//			try { cross_axis_gains.push_back(std::stof(cal_value)); }
//			catch (...) {}//intentionally left blank
//		}
//
//		if (offsets.size() < 3 || cross_axis_gains.size() < 9)
//		{
//			std::cout << "Some calibration information wasn't updated, using default accelerometer calibration values." << std::endl;
//			offsets = { 0, 0, 0 };
//			cross_axis_gains = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
//		}
//
//		inFile.close();
//	}
//	catch (...)
//	{
//		std::cout << "Couldn't find the calibration file, using default accelerometer calibration values." << std::endl;
//		offsets = { 0, 0, 0 };
//		cross_axis_gains = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
//	}
//
//	//After reading the calibration numbers create const arrays on the heap. This is done so that no other 
//	//class can alter these numbers accidentally.
//	this->p_calibration_offset = new const float[3]{ offsets[0], offsets[1], offsets[2] };
//
//	const float* x_gain = new float[3]{ cross_axis_gains[0], cross_axis_gains[1], cross_axis_gains[2] };
//	const float* y_gain = new float[3]{ cross_axis_gains[3], cross_axis_gains[4], cross_axis_gains[5] };
//	const float* z_gain = new float[3]{ cross_axis_gains[6], cross_axis_gains[7], cross_axis_gains[8] };
//	this->p_calibration_gain = new const float*[3]{ x_gain, y_gain, z_gain };
//}