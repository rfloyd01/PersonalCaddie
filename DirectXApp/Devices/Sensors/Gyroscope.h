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
	virtual void setConversionRateFromSettings() override;
	virtual void setCurrentODRFromSettings() override;

	virtual std::wstring convertCalNumbersToText() override;
};