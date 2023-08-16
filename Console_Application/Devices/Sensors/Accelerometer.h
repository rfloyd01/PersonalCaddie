#pragma once

#include <sensor_settings.h>
#include "Sensor.h"


//Class Definition
class Accelerometer : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Accelerometer(uint8_t* current_settings);

private:
	accelerometer_model_t acc_model;

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
	uint8_t settings[10];

	//PRIVATE FUNCTIONS
	void populateSensorSettingsArray(uint8_t* current_settings);

	//void getCalibrationNumbersFromTextFile();

	void setConversionRateFromSettings();
	void setCurrentODRFromSettings();
};