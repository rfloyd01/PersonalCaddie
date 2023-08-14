#pragma once


//Class Definition
class Sensor
{
public:
	//PUBLIC FUNCTIONS
	Sensor();

protected:
	float conversion_rate;
	float current_odr;

	//PRIVATE FUNCTIONS
	void getCalibrationNumbers();
	void setCalibrationNumbers();

	virtual void getConversionRate();
	virtual void getCurrentODR();
};