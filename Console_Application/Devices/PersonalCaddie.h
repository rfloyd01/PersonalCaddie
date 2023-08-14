#pragma once

#include <pch.h>
#include <vector>
#include <Graphics/graphics.h>

#include "IMU.h"
#include "BLE.h"

using namespace winrt;
using namespace Windows::Devices;

//Service and characteristic values are the same across Personal Caddie devices so define them here
#define SENSOR_INFO_UUID                  0xBEEF
#define SENSOR_SERVICE_UUID               0xBF34
#define SETTINGS_CHARACTERISTIC_UUID      0xBF35
#define ACC_DATA_CHARACTERISTIC_UUID      0xBF36
#define GYR_DATA_CHARACTERISTIC_UUID      0xBF37
#define MAG_DATA_CHARACTERISTIC_UUID      0xBF38

//enums and structs used by the Personal Caddie class
enum PersonalCaddiePowerMode
{
	ADVERTISING_MODE = 0,
	CONNECTED_MODE = 1,
	SENSOR_IDLE_MODE = 2,
	SENSOR_ACTIVE_MODE = 5
};

enum class DataType
{
	ACCELERATION,
	ROTATION,
	MAGNETIC,
	RAW_ACCELERATION,
	RAW_ROTATION,
	RAW_MAGNETIC,
	LINEAR_ACCELERATION,
	VELOCITY,
	LOCATION,
	EULER_ANGLES
};

/*
* This class is a representation of the physical Personal Caddie device. The real device is composed of a 
* BluetoothLE module (specifically the nRF52840) and an IMU (I've worked with a handful of different sensors
* at this point so we have the flexibility to individually select the accelerometer, gyroscope and magnetometer
* that are on the unit.
*/
class PersonalCaddie
{
public:
	//Constructors
	PersonalCaddie();

	//BLE Related Functions
	//void turnOnDataNotifications();
	concurrency::task<void> changePowerMode(PersonalCaddiePowerMode mode);

	volatile bool ble_device_connected;
	volatile bool data_available = false;

	void setGraphicsHandler(std::function<void(int)> function);

	PersonalCaddiePowerMode getCurrentPowerMode();

	//Methods and fields from original BluetoothLE Class
	void masterUpdate(); //master update function
	float getDataPoint(DataType dt, Axis a, int sample_number);
	int getCurrentSample();
	float getCurrentTime();
	glm::quat getOpenGLQuaternion();
	void setSampleFrequency(float freq);
	void setMagField();
	void resetTime();
	void resetPosition(); //resets sensor lin_acc., vel. and loc. to 0 so that club will be rendered back at center of screen
	void updateCalibrationNumbers();
	void setRotationQuaternion(glm::quat q);

private:

	BLE* p_ble; //a pointer to a Windows BLE Device
	IMU* p_imu;

	//BLE Functionality
	concurrency::task<void> BLEDeviceConnectedHandler();
	concurrency::task<void> getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service);
	void dataCharacteristicEventHandler(Bluetooth::GenericAttributeProfile::GattCharacteristic& car, Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs& args);
	void toggleDataCharacteristicNotifications();

	//Data Gathering/Manipulation
	void updateRawDataWithCalibrationNumbers(DataType dt);
	
	PersonalCaddiePowerMode current_power_mode;
	bool dataNotificationsOn;

	std::function<void(int)> graphic_update_handler; //pointer to a method in the graphic module

	//Characteristics obtained from m_ble
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_settings_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_accelerometer_data_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_gyroscope_data_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_magnetometer_data_characteristic{ nullptr };

	//    Methods and fields from original BluetoothLE Class     //
	//Internal Updating Functions
	void updateSensorData();
	void updateMadgwick();
	void updateLinearAcceleration();
	void updatePosition();
	void updateEulerAngles();

	void setDataPoint(DataType dt, Axis a, int sample_number, float data);

	float integrate(float one, float two, float dt);

	const int number_of_samples = 10; //number of sensor samples stored in the BLE characteristic at a given time. Due to the time associated with reading BLE broadcasts, its more efficient to store multiple data points at a single time then try to read each individual point
	int current_sample = 0; //when updating rotation quaternion with Madgwick filter, need to know which data point is currently being looked at

	glm::quat Quaternion = { 1, 0, 0, 0 }; //represents the current orientation of the sensor

	//Compound vector used for storing data
	std::vector<std::vector<std::vector<float> > > sensor_data; //vectors of size number_of_samples which hold current raw acceleration readings

	//Movement variables
	float lin_acc_threshold = 0.025; //linear acceleration will be set to zero unless it exceeds this threshold. This will help with location drift with time. This number was obtained experimentally as most white noise falls within +/- .025 of actual readings
	bool acceleration_event = 0; //when an acceleration above the threshold is detected, starts collecting velocity and position data. When acceleration has stopped in all directions, the event is set back to zero and velocity is halted
	float movement_scale = 1; //This number is to make sure that distance traveled looks accurate relative to how far sensor is from the camera
	bool just_stopped = 0; //when acceleration gets low enough, this variable is used to stop velocity and location from trickling forwards

	int data_counter = 0;

	//Frame conversion quaternions
	//These quaternions are used to rotate sensor data from one from to another
	glm::quat g_to_m, m_to_g; //quaternion that rotates the gravity frame to magnetic frame
	glm::quat m_to_mprime = { 1, 0, 0, 0 }; //The purpose of this quaternion is to rotate magnet data from current reading to 'desired' reading, i.e. magnetic north into computer screen so sensor lines up with computer screen

	//Timing Variables
	double position_timer = 0, end_timer = 0; //used for tracking start and stop times of accerleation events, to known if the club should actually move or not
	float time_stamp = 0; //the time in milliseconds from when the program connected to the BLE device, can be reset to 0 when looking at graphs
	float last_time_stamp = 0; //holds the time of the last measured sample, used to find delta_t for integration purposes

	//Madgwick items
	float sampleFreq, beta = 0.1; //beta changes how reliant the Madgwick filter is on acc and mag data, good value is 0.035
	float bx = -8, by = 40, bz = 15; //consider setting these values to the value of current location by default. Currently set to Conshohocken values

	//calibration numbers
	//These variables will change when a calibration is carried out and are initialized based on data in calibration.txt in Resources
	double acc_off[3] = { 0 };
	double acc_gain[3][3] = { 0 };
	double gyr_off[3] = { 0 };
	double gyr_gain[3] = { 0 };
	double mag_off[3] = { 0 };
	double mag_gain[3][3] = { 0 };
};