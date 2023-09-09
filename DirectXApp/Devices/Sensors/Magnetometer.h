#pragma once

#include "Sensor.h"

//Class Definition
class Magnetometer : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Magnetometer(uint8_t* current_settings);

private:
	magnetometer_model_t mag_model;

	//PRIVATE FUNCTIONS
	void setConversionRateFromSettings();
	void setCurrentODRFromSettings();
};