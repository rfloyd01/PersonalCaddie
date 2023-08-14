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
	//Just copy the relevant info over
	for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) this->settings[setting] = current_settings[setting];
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

//void Accelerometer::getConversionRate()
//{
//	switch (this->settings[0])
//	{
//	case LSM9DS1_ACC:
//		this->conversion_rate = lsm9ds1_fsr_conversion(ACCELEROMETER, this->settings[FS_RANGE]);
//	default:
//		this->conversion_rate = 0;
//	}
//}
//
//void Accelerometer::getCurrentODR()
//{
//	switch (this->settings[0])
//	{
//	case LSM9DS1_ACC:
//		this->conversion_rate = lsm9ds1_odr_calculate()
//	default:
//		this->conversion_rate = 0;
//	}
//}