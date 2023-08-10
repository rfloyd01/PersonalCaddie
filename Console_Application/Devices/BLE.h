#pragma once
#include <pch.h> //need this for access to bluetooth functions
#include <set>

#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command
#include <functional>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Bluetooth;

/*
* This class holds relevent information and methods for the Bluetooth Low Energy device that's
* on the Personal Caddie (in this case the physical chip is an nRF52840 SOC). Characteristic and 
* Service handles, methods for reading and writing characteristics, etc. is all contained in this 
* class.
*/

struct DeviceInfoDisplay;

class BLE
{
public:
	//Constructors
	BLE(std::function<void()> function);

	//Device Connection and Discovery Methods
	//void startBLEScan();
	
	void addScannedDevice(DeviceInfoDisplay device);

	bool isConnected();
	bool bleDeviceInitialized();
	concurrency::task<void> connect(uint64_t ble_address);
	volatile bool ble_device_created = false;

	BluetoothLEDevice* getBLEDevice() { return &(this->m_bleDevice); }

	std::thread background_connect;

private:
	void startDeviceWatcher();
	std::function<void()> connected_handler; //pointer to a connected event handler in the Personal Caddie class
	
	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);

	bool auto_connect;
	
	winrt::event_token connected_event_token;

	BluetoothLEDevice m_bleDevice{ nullptr }; //an instance of a Windows BLE Device
	GenericAttributeProfile::GattSession m_gattSession{ nullptr }; //a pointer to a Gatt Session with a BLE Device
	Advertisement::BluetoothLEAdvertisementWatcher m_bleAdvertisementsWatcher;
	std::set<DeviceInfoDisplay> m_scannedDevices; //a set for storing devices found by the device watcher
};

struct DeviceInfoDisplay
{
	//A small struct that holds info an BLE Devices found with the device watcher of the BLE class
	winrt::hstring device_name;
	std::pair<uint64_t, std::wstring> device_address; //the long integer is the physical address, the string is just a more redable representation of it

	//overload < operator to allow for the creation of a DeviceInfoDisplay set
	bool operator< (const DeviceInfoDisplay& dev) const
	{
		return (dev.device_address.first < this->device_address.first);
	}
};