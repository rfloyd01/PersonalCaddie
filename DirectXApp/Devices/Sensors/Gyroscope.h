#pragma once

#include "Sensor.h"

//Class Definition
class Gyroscope : public Sensor
{
public:
	//PUBLIC FUNCTIONS
	Gyroscope(uint8_t* current_settings);

private:
	gyroscope_model_t gyr_model;

	//PRIVATE FUNCTIONS
	void setConversionRateFromSettings();
	void setCurrentODRFromSettings();
};