#pragma once

#include "Sensor.h"

//Class Definition
class Accelerometer : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Accelerometer(uint8_t* current_settings);

private:
	accelerometer_model_t acc_model;

	//PRIVATE FUNCTIONS
	//void getCalibrationNumbersFromTextFile();

	virtual void setConversionRateFromSettings() override;
	virtual void setCurrentODRFromSettings() override;
};