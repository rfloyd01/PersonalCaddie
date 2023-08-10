#include "pch.h"

#include <iostream>
#include <iomanip>

#include "BLE.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;

//Constructors
BLE::BLE(std::function<void()> function)
{
	//When first creating a new BLE device we attempt to automatically connect to the first 'Personal Caddie' we can find.
    //Set up the device watcher so that it saves any new device it encounters, but stop the watcher if and when a Peronsal
    //Caddie is found and connected to.
    this->m_bleAdvertisementsWatcher = Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher();
    this->m_bleAdvertisementsWatcher.ScanningMode(Bluetooth::Advertisement::BluetoothLEScanningMode::Active);
    this->m_bleAdvertisementsWatcher.Received(Windows::Foundation::TypedEventHandler<Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher, Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs>(
        [this](Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher, Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            //We scan for devices and when a new one is found we save its name and address. To connect to a device, the Windows Bluetooth LE API requires a valid 
            //Bluetooth address, as well as that address to be in the computer's memory cache. So even if a device has been paired with previously this 
            //scanning step is necessary.
            auto serviceUuids = eventArgs.Advertisement().ServiceUuids();

            DeviceInfoDisplay foundDevice;
            foundDevice.device_name = eventArgs.Advertisement().LocalName();
            foundDevice.device_address.first = eventArgs.BluetoothAddress();
            foundDevice.device_address.second = formatBluetoothAddress(foundDevice.device_address.first);

            this->addScannedDevice(foundDevice); //add the device to the scanned device set for potential later use

            //After adding the device, check and see if it's a Personal Caddie and then attempt to connect to it
            if (foundDevice.device_name == winrt::to_hstring("Personal Caddie"))
            {
                this->connect(foundDevice.device_address.first);
            }
        })
    );

    std::cout << "Device watcher started" << std::endl;
    this->m_bleAdvertisementsWatcher.Start();

    //TEST: Pass in a function from the Personal Caddie class
    this->connected_handler = function;

}

void BLE::startDeviceWatcher()
{
    //When manually starting the device watcher we don't attempt to automatically connect to devices like we do
    //in the constructor for this class. We just scan for devices and add them to the m_scannedDevices set as a 
    //different function handles picking which one to connect to.
    this->m_bleAdvertisementsWatcher.Start();
}

concurrency::task<void> BLE::connect(uint64_t ble_address)
{
    std::cout << "Found a Personal Caddie, attempting to connect." << std::endl;
    //First create a winrt::BluetoothLEDevice for the BLE class
    this->m_bleDevice = co_await Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(ble_address);

    //Next, create a Gatt Session with the device
    this->m_gattSession = co_await Bluetooth::GenericAttributeProfile::GattSession::FromDeviceIdAsync(m_bleDevice.BluetoothDeviceId());

    //Once the session is established, initiate a connection and turn off the device watcher
    this->m_gattSession.MaintainConnection(true);
    this->m_bleAdvertisementsWatcher.Stop();

    //Envoke the connected event function passed in from the Personal Caddie class
    this->connected_handler();
}

bool BLE::isConnected()
{
    if (bleDeviceInitialized()) return (this->m_bleDevice.ConnectionStatus() == BluetoothConnectionStatus::Connected);
    else return false;
}

void BLE::addScannedDevice(DeviceInfoDisplay device)
{
    //attempts to put the given device into the m_scannedDevices set
    this->m_scannedDevices.insert(device);
}

std::wstring BLE::formatBluetoothAddress(unsigned long long BluetoothAddress)
{
    //This method takes a 64-bit Bluetooth Device address and turns it into a string that's
    //more readable to the human eye.
    std::wostringstream ret;
    ret << std::hex << std::setfill(L'0')
        << std::setw(2) << ((BluetoothAddress >> (5 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (4 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (3 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (2 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (1 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (0 * 8)) & 0xff);

    return ret.str();
}

bool BLE::bleDeviceInitialized()
{
    //this method checks to see if m_bleDevice has been changed from a NULL value
    return (m_bleDevice.as<winrt::Windows::Foundation::IUnknown>() != nullptr);
}