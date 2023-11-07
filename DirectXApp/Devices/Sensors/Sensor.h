#pragma once

#include "../Firmware/MEMs_Drivers/sensor_settings.h"

//Class Definition
class Sensor
{
public:
	//PUBLIC FUNCTIONS
	Sensor();

	float getConversionRate();
	float getCurrentODR();

	uint8_t* getCurrentSettings() { return settings; }
	void setCurrentSettings(uint8_t* new_settings)
	{
		//Just copy the relevant info over
		for (int setting = SENSOR_MODEL; setting <= EXTRA_2; setting++) settings[setting] = new_settings[setting];

		//After the sensor updates its settings array, calculate the current
		//ODR and full scale conversion rate from the new settings.
		setConversionRateFromSettings();
		setCurrentODRFromSettings();
	}

	std::pair<const float*, const float**> getCalibrationNumbers();
	std::vector<int> getAxisOrientations();
	void setCalibrationNumbers(float* offset, float** gain);
	void setAxesOrientations(std::vector<int> axes_parameters);

	void updateODR(float new_odr) { current_odr = new_odr; }

protected:
	//An array that holds the relevant settings for the sensor. The indices of the array hold settings for the following:
	//0x00 = sensor model
	//0x01 = full scale range
	//0x02 = ODR
	//0x03 = power level
	//0x04 = filter selection
	//0x05 = low pass filter
	//0x06 = high pass filter
	//0x07 = other filter(s)
	//0x08 = extra info 1
	//0x09 = extra info 2
	uint8_t settings[10]; //an array holding the current settings for the sensor

	float conversion_rate;
	float current_odr;

	//calibration variables
	std::wstring calibrationFile; //the location and name of the calibration file for the sensor
	std::wstring axisFile; //the location and name of the calibration file for the sensor

	float calibration_offsets[3] = { 0, 0, 0 }; //points to axis offset numbers from calibration
	float calibration_gain_x[3] = { 1, 0, 0 }, calibration_gain_y[3] = { 0, 1, 0 }, calibration_gain_z[3] = { 0, 0, 1 }; //individual axis gains for calibration
	const float* calibration_gains[3] = { calibration_gain_x, calibration_gain_y, calibration_gain_z }; //combines the axis gain data into a single array, used by other classes
	int axes_swap[3] = { 0, 1, 2 }; //swaps data between axes if necessary
	int axes_polarity[3] = { 1, 1, 1 }; //inverts the readings of an axis between positive and negative if necessary

	//PRIVATE FUNCTIONS
	void getCalibrationNumbersFromTextFile();
	void setCalibrationNumbersInTextFile();
	void getAxisOrientationsFromTextFile();
	void setAxisOrientationsInTextFile();

	std::wstring convertCalNumbersToText();
	void convertTextToCalNumbers(winrt::hstring calInfo);

	std::wstring convertAxisNumbersToText();
	void convertTextToAxisNumbers(winrt::hstring calInfo);

	virtual void setConversionRateFromSettings() = 0;
	virtual void setCurrentODRFromSettings() = 0;
};