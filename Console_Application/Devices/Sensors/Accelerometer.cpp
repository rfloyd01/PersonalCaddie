#include "pch.h"

#include "Accelerometer.h"

Accelerometer::Accelerometer(accelerometer_model_t acc_model, uint8_t* current_settings)
{
	//Typically (although they don't have to be) sensors are only created after connecting to a Personal Caddie
	//and reading the Sensor Information characteristic, which tells us what sensors are on the device as well 
	//as their current settings (things like ODR, full-scale range, etc.)
	this->acc_model = acc_model;
	
	populateSensorSettingsArray(current_settings);
}

void Accelerometer::populateSensorSettingsArray(uint8_t* current_settings)
{
	//depending on the brand of accelerometer we're looking at the settings in the array may be packaged 
	//differently. For example, the LSM9DS1 accelerometer puts information on the ODR and power mode into 
	//the same Byte whereas the FXOS sensor separates them. It all depends on the individual drivers for 
	//the sensors. Because of this, this method extracts the settings we need on a sensor by sensor basis.
	 
}

void Accelerometer::getCalibrationNumbers()
{
	Sensor::getCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file

	//Update calibration numbers obtained from the calibration file here
}
void Accelerometer::setCalibrationNumbers()
{
	Sensor::setCalibrationNumbers(); //handles the opening of the file, reading the data, and closing the file
	//will set the calibration numbers for the particular sensor
}