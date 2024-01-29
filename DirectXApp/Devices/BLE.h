#pragma once
#include <pch.h> //need this for access to bluetooth functions
#include <set>

#include <functional>

#include "../Math/glm.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Bluetooth;

struct DeviceInfoDisplay;

/*
* This class holds relevent information and methods for the Bluetooth Low Energy device that's
* on the Personal Caddie (in this case the physical chip is an nRF52840 SOC). Characteristic and 
* Service handles, methods for reading and writing characteristics, etc. is all contained in this 
* class.
*/

enum class BLEState
{
	DeviceFound,
	DeviceNotFound,
	Connected,
	NewAdvertisement,
	Disconnect,
	Reconnect,
	DeviceWatcherStatus,
	EnableDeviceWatcher,
	DisableDeviceWatcher
};

class BLE
{
public:
	//Constructors
	BLE(std::function<void(BLEState)> function);

	//Device Connection and Discovery Methods
	IAsyncOperation<BluetoothLEDevice> connectToExistingDevice();
	IAsyncOperation<BluetoothLEDevice> connectToDevice(uint64_t deviceAddress);
	
	void addScannedDevice(DeviceInfoDisplay device);

	bool isConnected();
	bool isDeviceWatcherOn();
	bool bleDeviceInitialized();

	volatile bool ble_device_created = false;

	std::set<DeviceInfoDisplay>* getScannedDevices() { return &m_scannedDevices; }

	BluetoothLEDevice* getBLEDevice() { return &(this->m_bleDevice); }

	void startBLEAdvertisementWatcher();
	void stopBLEAdvertisementWatcher();

	void terminateConnection();

private:

	//Handler Methods
	std::function<void(BLEState)> state_change_handler; //pointer to an event handler in the Personal Caddie class
	void deviceFoundHandler(IAsyncOperation<BluetoothLEDevice> const& sender, AsyncStatus const asyncStatus);
	void connectionChangedHandler();
	
	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);

	bool auto_connect;
	bool maintain_connection;
	volatile bool m_device_initialized;
	volatile bool m_is_connected;
	
	winrt::event_token connected_event_token;

	BluetoothLEDevice m_bleDevice{ nullptr }; //an instance of a Windows BLE Device
	//GenericAttributeProfile::GattSession m_gattSession{ nullptr }; //a pointer to a Gatt Session with a BLE Device
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