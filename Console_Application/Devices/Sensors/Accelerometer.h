#pragma once

#include "Sensor.h"

//enums, structs, etc. needed for Accelerometer class
enum class AccelerometerModel
{
	LSM9DS1,
	FXOS8700
};

//Class Definition
class Accelerometer : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Accelerometer(AccelerometerModel acc_model, uint8_t* current_settings);

private:
	AccelerometerModel acc_model;

	//PRIVATE FUNCTIONS
	void populateSensorSettingsArray(uint8_t* current_settings);

	void getCalibrationNumbers();
	void setCalibrationNumbers();
};