#pragma once


//Class Definition
class Sensor
{
public:
	//PUBLIC FUNCTIONS
	Sensor();

	float getConversionRate();
	float getCurrentODR();

	std::pair<const float*, const float**> getCalibrationNumbers();
	void setCalibrationNumbers(float* offset, float** gain);

protected:
	float conversion_rate;
	float current_odr;

	//calibration variables
	int cal_offset_number, cal_gains_number; //holds the number of calibration offsets and gains the sensor has
	std::string calibrationFile; //the location and name of the calibration file for the sensor
	float calibration_offsets[3]; //points to axis offset numbers from calibration
	float calibration_gain_x[3], calibration_gain_y[3], calibration_gain_z[3]; //individual axis gains for calibration
	const float* calibration_gains[3] = { calibration_gain_x, calibration_gain_y, calibration_gain_z }; //combines the axis gain data into a single array

	//PRIVATE FUNCTIONS
	void getCalibrationNumbersFromTextFile();
	void setCalibrationNumbersInTextFile();

	
};