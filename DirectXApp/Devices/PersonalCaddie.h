#pragma once

#include <pch.h>
#include <vector>

#include "IMU.h"
#include "BLE.h"
#include "Modes/mode.h"

using namespace winrt;
using namespace Windows::Devices;

//Service and characteristic values are the same across Personal Caddie devices so define them here
#define PERSONAL_CADDIE_SERVICE_UUID           0xBF40
#define ERROR_CHARACTERISTIC_UUID              0xBF41
#define SENSOR_SERVICE_UUID                    0xBF34
#define SETTINGS_CHARACTERISTIC_UUID           0xBF35
#define ACC_DATA_CHARACTERISTIC_UUID           0xBF36
#define GYR_DATA_CHARACTERISTIC_UUID           0xBF37
#define MAG_DATA_CHARACTERISTIC_UUID           0xBF38
#define AVAILABLE_SENSORS_CHARACTERISTIC_UUID  0xBF39
#define MAX_SENSOR_SAMPLES                     39 //at most we can hold 39 sensor readings in a single characteristic and send out the notification in a single packet

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

enum class PersonalCaddieEventType
{
	NONE = 0,
	PC_ALERT = 1,
	BLE_ALERT = 2,
	IMU_ALERT = 3,
	DEVICE_WATCHER_UPDATE = 4,
	CONNECTION_EVENT = 5,
	NOTIFICATIONS_TOGGLE = 6,
	DATA_READY = 7,
	PC_ERROR = 8
};

enum class TextType;

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
	PersonalCaddie(std::function<void(PersonalCaddieEventType, void*)> function);

	//BLE Related Functions
	void changePowerMode(PersonalCaddiePowerMode mode);
	void updateIMUSettings(uint8_t* newSettings);

	void startDataTransfer();

	volatile bool ble_device_connected;

	PersonalCaddiePowerMode getCurrentPowerMode();

	void enableDataNotifications();
	void disableDataNotifications();

	std::pair<const float*, const float**> getSensorCalibrationNumbers(sensor_type_t sensor);
	void updateSensorCalibrationNumbers(sensor_type_t sensor, std::pair<float*, float**> cal_numbers);
	void updateSensorAxisOrientations(std::vector<int> axis_orientations);

	int getNumberOfSamples() { return this->number_of_samples; }
	float getMaxODR() { return this->p_imu->getMaxODR(); }
	float getDataTimeStamp() { return this->m_first_data_time_stamp; }

	void connectToDevice(uint64_t deviceAddress);
	void disconnectFromDevice();

	std::vector<uint8_t*> getIMUSettings() { return p_imu->getSensorSettings(); }
	std::vector<uint8_t> const& getAvailableSensors() { return m_availableSensors; }

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

	void startBLEAdvertisementWatcher() { return p_ble->startBLEAdvertisementWatcher(); }
	void stopBLEAdvertisementWatcher() { return p_ble->stopBLEAdvertisementWatcher(); }

	std::set<DeviceInfoDisplay>* getScannedDevices() { return p_ble->getScannedDevices(); }
	std::vector<std::vector<std::vector<float> > > const& getSensorData() { return sensor_data; }
	std::vector<glm::quat> const& getQuaternions() { return orientation_quaternions; }

private:
	std::unique_ptr<BLE> p_ble;
	std::unique_ptr<IMU> p_imu;

	//Handler Methods
	std::function<void(PersonalCaddieEventType, void*)> event_handler; //sends events to the ModeScreen class for rendering on screen
	void BLEDeviceHandler(BLEState state);
	bool cccdWriteHandler(IAsyncOperation<Bluetooth::GenericAttributeProfile::GattCommunicationStatus> const& sender, AsyncStatus const status);

	//BLE Functionality
	void getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service);
	void getErrorCharacteristic(Bluetooth::GenericAttributeProfile::GattDeviceService& pc_service);
	void dataCharacteristicEventHandler(Bluetooth::GenericAttributeProfile::GattCharacteristic& car, Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs& args);
	void automaticallyConnect();

	//Data Gathering/Manipulation
	void updateRawDataWithCalibrationNumbers(DataType rdt, DataType dt, sensor_type_t sensor_type, const float* offset_cal, const float** gain_cal);
	void updateMostRecentDeviceAddress(uint64_t address);
	float convertTicksToSeconds(uint32_t timer_ticks);
	
	PersonalCaddiePowerMode current_power_mode;
	bool dataNotificationsOn;

	std::vector<uint8_t> m_availableSensors;

	volatile bool sensor_data_updated[3] = { false, false, false };
	volatile bool data_available = false;

	//Gatt Settings and Characteristics obtained from m_ble
	winrt::Windows::Foundation::Collections::IVectorView<Bluetooth::GenericAttributeProfile::GattDeviceService>  m_services{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_error_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_settings_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_available_sensors_characteristic{ nullptr };
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

	int number_of_samples = 10; //number of sensor samples stored in the BLE characteristic at a given time. Due to the time associated with reading BLE broadcasts, its more efficient to store multiple data points at a single time then try to read each individual point
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
	float m_first_data_time_stamp = 0; //represents the point in time (from when sensors first start recording data) that the first bit of data in the current set was recorded

	//Madgwick items
	float sampleFreq, beta = 0.1; //beta changes how reliant the Madgwick filter is on acc and mag data, good value is 0.035
	float bx = -8, by = 40, bz = 15; //consider setting these values to the value of current location by default. Currently set to Conshohocken values

	//Axis Swapping variables
	//The axes for the sensors don't typically line up with the axes that DirectX uses, so these variables allow a seemless
	//swap. TODO: Need some methods to manually manipulate these variables
	Axis acc_axes_swap[3] = {Y, Z, X};
	int acc_axes_invert[3] = { -1, 1, 1 };

	Axis gyr_axes_swap[3] = { Y, Z, X };
	int gyr_axes_invert[3] = { -1, 1, 1 };

	Axis mag_axes_swap[3] = { Y, Z, X };
	int mag_axes_invert[3] = { -1, 1, 1 };
};