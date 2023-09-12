#include "pch.h"

#include "Sensor.h"

#include <algorithm>
#include <iostream>
#include <fstream>

Sensor::Sensor()
{
	//Any kind of initialization that's common regardless of sensor type will go here
}

std::pair<const float*, const float**> Sensor::getCalibrationNumbers()
{
	//we pass const pointers to the calibration data because we don't want other classes
	//to be able to directly change the calibration values
	return { this->calibration_offsets, this->calibration_gains };
};

void Sensor::setCalibrationNumbers(float* offset, float** gain)
{

}

void Sensor::getCalibrationNumbersFromTextFile()
{
	//this method takes calibration numbers that were obtained in a previous session (if they exist) and updates the 
	//local calibration variables with them. If no previous calibration numbers exist then default values are used.
	std::vector<float> offsets, gains;

	//open the external calibration file, if it can't be found then use default calibration numbers
	try
	{
		int line_count = 0;
		std::ifstream inFile;
		inFile.open(this->calibrationFile);
		if (!inFile.good()) throw 34; //just throw a random number if the file doesn't exist
		char cal_value[256];

		//the offset values come first in the calibration file
		while (offsets.size() < this->cal_offset_number && !inFile.eof())
		{
			inFile.getline(cal_value, 256);

			//the calibration file has both text and float values so check to see if the next float value has been encountered
			try { offsets.push_back(std::stof(cal_value)); }
			catch (...) {}//intentionally left blank
		}

		//followed by the nine cross-axis gain values
		while (gains.size() < this->cal_gains_number && !inFile.eof())
		{
			inFile.getline(cal_value, 256);

			//the calibration file has both text and float values so check to see if the next float value has been encountered
			try { gains.push_back(std::stof(cal_value)); }
			catch (...) {}//intentionally left blank
		}

		if (offsets.size() < this->cal_offset_number || gains.size() < this->cal_gains_number)
		{
			std::cout << "Some calibration information wasn't updated, using default sensor calibration values." << std::endl;
			offsets = std::vector<float>(this->cal_offset_number, 0);
			gains = std::vector<float>(this->cal_gains_number, 0);
			for (int i = 0; i < this->cal_gains_number; i += (this->cal_gains_number / 3)) gains[i] = 1; //gains are 1 between the same axis
		}

		inFile.close();
	}
	catch (...)
	{
		OutputDebugString(L"Couldn't find the calibration file, using default sensor calibration values.\n");
		offsets = std::vector<float>(this->cal_offset_number, 0);
		gains = std::vector<float>(this->cal_gains_number, 0);
		for (int i = 0; i < this->cal_gains_number; i += (this->cal_gains_number / 3)) gains[i] = 1; //gains are 1 between the same axis
	}

	//Update the offset and gain arrays for the sensor
	std::copy(offsets.begin(), offsets.end(), this->calibration_offsets); //offsets

	//For now, all sensors 
	std::copy(gains.begin(), gains.begin() + (this->cal_gains_number) / 3, this->calibration_gain_x);
	std::copy(gains.begin() + (this->cal_gains_number) / 3, gains.begin() + 2 * (this->cal_gains_number) / 3, this->calibration_gain_y);
	std::copy(gains.begin() + 2 * (this->cal_gains_number) / 3, gains.begin() + (this->cal_gains_number), this->calibration_gain_z);
}

void Sensor::setCalibrationNumbersInTextFile()
{
	//will set the calibration numbers for the particular sensor
}

float Sensor::getConversionRate() { return this->conversion_rate; }
float Sensor::getCurrentODR() { return this->current_odr; }