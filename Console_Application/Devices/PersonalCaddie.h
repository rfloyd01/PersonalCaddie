#pragma once

#include <pch.h>
#include <vector>
#include <Graphics/graphics.h>

#include "sensor.h"
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


private:

	BLE* p_ble; //a pointer to a Windows BLE Device
	IMU* p_imu;

	concurrency::task<void> BLEDeviceConnectedHandler();
	concurrency::task<void> getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service);
	
	PersonalCaddiePowerMode current_power_mode;

	//Characteristics obtained from m_ble
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_settings_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_accelerometer_data_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_gyroscope_data_characteristic{ nullptr };
	Bluetooth::GenericAttributeProfile::GattCharacteristic m_magnetometer_data_characteristic{ nullptr };
};