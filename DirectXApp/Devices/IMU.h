#pragma once

#include "Sensors/Accelerometer.h"
#include "Sensors/Gyroscope.h"
#include "Sensors/Magnetometer.h"

#include <vector>

//The IMU class helps ensure a seamless transition if one IMU is swapped out for another, or if different parts of different IMUs are decided
//to be used. This class keeps track of the orientation of the axes for each sensor that I currently have, as well as the necessary multipliers
//for each sensor to get the correct reading based on the current settings. I created this class because I currently have three sensors that all
//have slightly different directions for their axes somehow, so anytime I switch from one to the other it will mess up my implementation of the
//Madgwick filter. I used to have to manually change sensor axes orientations in code as well as conversions from bit readings to actual sensor
//readings, this class eliminates the need for all of that manual work. The sensor axes are all relative to each other, so even if a chip is
//installed backwards on a bread board the Madgwick filter should work just fine. If different parts from different IMUs are used, however,
//(for example if the gyro from the BLE Sense 33 is used and the acc + mag from an external FXOS8700 are use as well) then the orientation of the
//IMU's to eachother on the bread board WILL matter. For this reason a document showing the expected orientations for each IMU on a bread board
//is shown in the following document (TODO: create master orientation document) which is located in the project Resources folder

//Since I'm currently rendering everything using OpenGL, the axes for each IMU device will be swapped and inverted as necessary so that they match
//the expected axes in OpenGL (i.e. +x points to the right of screen +y points upwards into the sky and +z points directly out of computer screen
//and towards the user). Initially I had kept all sensor axes as is and swapped to OpenGL coordinates at the last minute by shuffling the positions
//of the elements in the OpenGL quaternion itself (i.e. to swap the x and y axes I would do all rotation calculations normally and then just alter the
//rotation quaternion at the end of calculations to look like this [q0, q2, q1, q3] as opposed to swapping axes first and keeping the rotation
//quaternion as [q0, q1, q2, q3]). Although this approach worked it really became a nightmare to keep track of things when trying to switch between
//sensors. Taking this different approach of forcibly swapping sensor axes so that they match OpenGL coordinates has made things much easier overall.
//Furthermore, if I ever change to a different 3d rendering API that has a different coordinate system than OpenGL it will be very easy to switch now
//by simply altering the master_axes_orientation global variable

//Ultimately I'd like to make a sub class called "Chip" or something similar to that. As of right now all of the IMU's that I have listed are really
//multiple microprocessors that are soldered onto the same break-out board. In the future though, for example,  I'd love to define the FXOS/FXAS break-out board in terms
//of an FXOS chip that's rotated at 0 degrees and an FXAS chip that's rotated at 90 degrees (or something similar to that). Essentially, all of the
//IMU types that I'm defining here are boards assembled by manufacturers. As I've started designing my own boards I'm realizing that the component
//micro processors (like the FXOS and FXAS chips) really have 4 different orientations that they can go in. It'd be nice to just define "Chips" in terms of
//their axes orientations and then define IMU's as a combination of chips in different orientations (these orientations would be limited to 0, 90,
//180 and 270 degrees)

enum Axis
{
	//x, y, and z correlate to 0, 1 and 2 respectively. It's a little easier to keep track this way then having numbers denote everything
	X = 0,
	Y = 1,
	Z = 2
};

//Global Definition of Positive Axes Directions
//The proper order for the global axes is {axis that points to the right side of computer screen, axis that points to top of computer screen, axis that points outwards of computer screen}
//Currently the global axes directions match those of OpenGL so the proper order for the axes is just {x, y, z}
static Axis global_positive_axes[3] = { X, Y, Z };

//Class Declaration
//class IMU;

//Class Definition
class IMU
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	IMU(uint8_t* imu_settings);

	float* getSensorODRs();
	float* getSensorConversionRates();

	float getMaxODR();
	float getConversionRate(sensor_type_t sensor);

	std::pair<const float*, const float**> getAccelerometerCalibrationNumbers();
	std::pair<const float*, const float**> getGyroscopeCalibrationNumbers();
	std::pair<const float*, const float**> getMagnetometerCalibrationNumbers();

	void updateAccelerometerCalibrationNumbers(float* offset, float** gain);
	void updateGyroscopeCalibrationNumbers(float* offset, float** gain);
	void updateMagnetometerCalibrationNumbers(float* offset, float** gain);

	//Setting Altering Functions
	//TODO: eventually create functions that will allow for the changing of sensor settings over bluetooth and TWI or SPI

	//Axes Altering Functions
	//TODO: eventually create functions that will allow for the swapping of axes to acoomodate flipping of sensors on bread board

	//Get Functions
	/*Accelerometer getAccelerometerType();
	Gyroscope getGyroscopeType();
	Magnetometer getMagnetometerType();
	double getFrequency();
	double getSensorSensitivity(Sensor s);

	Axis getOpenGLAxis(Sensor sensor_type, int sensor_axis);
	int getAxisPolarity(Sensor sensor_type, int sensor_axis);*/

private:
	//PRIVATE FUNCTIONS
	//void loadSensorInformation(IMU& sensor); //load the data on all current sensors that I own
	//void loadSensorDefaultSettings(IMU& sensor);
	//void loadAxisOrientations(IMU& sensor);
	void getODRFromSensors();
	void getConversionRateFromSensors();

	float max_odr = 0;

	//PRIVATE VARIABLES
	//Changeable Sensor Settings
	float IMU_sample_frequencies[3]; //sample frequencies for each sensor in Hz. Order is accelerometer, gyroscope, magnetometer
	float IMU_data_sensitivity[3]; //current sensitivity for each sensor. Order is accelerometer, gyroscope, magnetometer

	//Sensor Axis information
	//The expected axes order is {x, y, z}, some sensors are oriented differently, however. These arrays keep track of swapped axes for each sensor of the IMU
	Axis IMU_acc_axis_order[3]; 
	Axis IMU_gyr_axis_order[3];
	Axis IMU_mag_axis_order[3];

	//Apart from it being possible for axes to be swapped with eachother (i.e. x and y switch places) it's also possible for axes to be inverted (i.e. +x and -x are swapped)
	//these arrays keep track of axis inversion for each sensor of the IMU. A value of 1 means standard and -1 means inverted
	int IMU_acc_axis_inversion[3];
	int IMU_gyr_axis_inversion[3];
	int IMU_mag_axis_inversion[3];

	std::unique_ptr<Accelerometer> p_acc;
	std::unique_ptr<Gyroscope> p_gyr;
	std::unique_ptr<Magnetometer> p_mag;
};