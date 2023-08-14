#pragma once

#include <sensor_settings.h>
#include "Sensor.h"


//Class Definition
class Accelerometer : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Accelerometer(accelerometer_model_t acc_model, uint8_t* current_settings);

private:
	accelerometer_model_t acc_model;

	//PRIVATE FUNCTIONS
	void populateSensorSettingsArray(uint8_t* current_settings);

	void getCalibrationNumbers();
	void setCalibrationNumbers();
};