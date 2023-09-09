#pragma once

#include "../Firmware/MEMs_Drivers/sensor_settings.h"

//Class Definition
class Sensor
{
public:
	//PUBLIC FUNCTIONS
	Sensor();

	float getConversionRate();
	float getCurrentODR();

	uint8_t* getCurrentSettings() { return settings; }
	void setCurrentSettings(uint8_t* new_settings)
	{
		//Just copy the relevant info over
		for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) settings[setting] = new_settings[setting];
	}

	std::pair<const float*, const float**> getCalibrationNumbers();
	void setCalibrationNumbers(float* offset, float** gain);

protected:
	//An array that holds the relevant settings for the sensor. The indices of the array hold settings for the following:
	//0x00 = sensor model
	//0x01 = full scale range
	//0x02 = ODR
	//0x03 = power level
	//0x04 = filter selection
	//0x05 = low pass filter
	//0x06 = high pass filter
	//0x07 = other filter(s)
	//0x08 = extra info 1
	//0x09 = extra info 2
	uint8_t settings[10]; //an array holding the current settings for the sensor

	float conversion_rate;
	float current_odr;

	//calibration variables
	int cal_offset_number, cal_gains_number; //holds the number of calibration offsets and gains the sensor has
	std::string calibrationFile; //the location and name of the calibration file for the sensor
	float calibration_offsets[3]; //points to axis offset numbers from calibration
	float calibration_gain_x[3], calibration_gain_y[3], calibration_gain_z[3]; //individual axis gains for calibration
	const float* calibration_gains[3] = { calibration_gain_x, calibration_gain_y, calibration_gain_z }; //combines the axis gain data into a single array

	//PRIVATE FUNCTIONS
	void getCalibrationNumbersFromTextFile();
	void setCalibrationNumbersInTextFile();

	
};