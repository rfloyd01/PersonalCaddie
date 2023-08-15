#pragma once


//Class Definition
class Sensor
{
public:
	//PUBLIC FUNCTIONS
	Sensor();

	float getConversionRate();
	float getCurrentODR();

protected:
	float conversion_rate;
	float current_odr;

	//PRIVATE FUNCTIONS
	void getCalibrationNumbers();
	void setCalibrationNumbers();

	
};