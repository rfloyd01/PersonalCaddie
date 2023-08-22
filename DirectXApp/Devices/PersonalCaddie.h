#pragma once

#include <pch.h>
#include <vector>

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
	void changePowerMode(PersonalCaddiePowerMode mode);

	volatile bool ble_device_connected;

	void setGraphicsHandler(std::function<void(int)> function);

	PersonalCaddiePowerMode getCurrentPowerMode();

	void enableDataNotifications();
	void disableDataNotifications();

	std::pair<const float*, const float**> getSensorCalibrationNumbers(sensor_type_t sensor);

	BLE* getBLEDevice() { return this->p_ble; }
	int getNumberOfSamples() { return this->number_of_samples; }
	float getMaxODR() { return this->p_imu->getMaxODR(); } //TODO: Should put a nullptr check here

	//Methods and fields from original BluetoothLE Class
	void dataUpdate(); //master update function
	float getDataPoint(DataType dt, Axis a, int sample_number);
	int getCurrentSample();
	float getCurrentTime();
	glm::quat getOpenGLQuaternion(int sample);
	void setSampleFrequency(float freq);
	void setMagField();
	void resetTime();
	void resetPosition(); //resets sensor lin_acc., vel. and loc. to 0 so that club will be rendered back at center of screen
	//void updateCalibrationNumbers();
	void setRotationQuaternion(glm::quat q, int sample);

private:

	BLE* p_ble; //a pointer to a Windows BLE Device
	IMU* p_imu;

	//BLE Functionality
	void BLEDeviceConnectedHandler();
	void getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service);
	void dataCharacteristicEventHandler(Bluetooth::GenericAttributeProfile::GattCharacteristic& car, Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs& args);

	//Data Gathering/Manipulation
	void updateRawDataWithCalibrationNumbers(DataType rdt, DataType dt, sensor_type_t sensor_type, const float* offset_cal, const float** gain_cal);
	
	PersonalCaddiePowerMode current_power_mode;
	bool dataNotificationsOn;

	std::function<void(int)> graphic_update_handler; //pointer to a method in the graphic module

	volatile bool sensor_data_updated[3] = { false, false, false };
	volatile bool data_available = false;

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

	//glm::quat Quaternion = { 1, 0, 0, 0 }; //represents the current orientation of the sensor

	//the following compound vector holds raw data from the sensors as well as calculated data such as (calibrated acc, gyr and mag readings, linear acceleration,
	//velocity, etc.). The outermost vector holds everything, the second vector represents a specific data type and the innermost vector represents a specific
	//axis of that data type. The innermost vector holds a set number of data points, equal to the number_of_samples field above. To show how this structure 
	//looks in practice: { { {raw_acc_x_1, raw_acc_x_2, ..., raw_acc_x_numberofsamples}, {raw_acc_y_1 ...}, {raw_acc_z_1 ...} }, { {raw_gyr_x_1...} ....}
	std::vector<std::vector<std::vector<float> > > sensor_data; 
	std::vector<glm::quat> orientation_quaternions; //this vector holds number_of_samples quaternions, where each quaternion matches the sensor orientation at a point in time

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
};