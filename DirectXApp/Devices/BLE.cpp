#include "pch.h"

#include <iomanip>
#include <sstream>

#include "BLE.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;

//Constructors
BLE::BLE(std::function<void(BLEState)> function)
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

            int currentFoundDevices = m_scannedDevices.size();
            if (foundDevice.device_name == L"Personal Caddie") this->addScannedDevice(foundDevice); //only add Personal Caddie devices to the list

            //If a new device is found, send an alert to the Personal Caddie to let it know
            if (m_scannedDevices.size() > currentFoundDevices) state_change_handler(BLEState::NewAdvertisement);
        })
    );

    //Set the onConnected() event handler from the Personal Caddie class
    this->state_change_handler = function;
    maintain_connection = false; //This gets set to true when we actually connect to a device
}

IAsyncOperation<BluetoothLEDevice> BLE::connectToDevice(uint64_t deviceAddress)
{
    IAsyncOperation<BluetoothLEDevice> FindBLEAsync = Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(deviceAddress);

    //create a handler that will get called when the BLEDevice is created
    FindBLEAsync.Completed([this, deviceAddress](
        IAsyncOperation<BluetoothLEDevice> const& sender,
        AsyncStatus const asyncStatus)
        {
            deviceFoundHandler(sender, asyncStatus);
        });

    return FindBLEAsync;
}

IAsyncOperation<BluetoothLEDevice> BLE::connectToExistingDevice()
{
    //This method attempts to connect to the most recently paired Personal Caddie device. If no device exists, or this method
    //is unsuccessful then the device watcher will be called

    uint64_t personal_caddie_address = 274381568618262;  //TODO: This should be saved in an external file
    return connectToDevice(personal_caddie_address);
}

void BLE::startBLEAdvertisementWatcher()
{
    //When manually starting the device watcher we don't attempt to automatically connect to devices like we do
    //in the constructor for this class. We just scan for devices and add them to the m_scannedDevices set as a 
    //different function handles picking which one to connect to.
    OutputDebugString(L"Device watcher started\n");
    this->m_bleAdvertisementsWatcher.Start();
}

void BLE::stopBLEAdvertisementWatcher()
{
    OutputDebugString(L"Device watcher stopped\n");
    this->m_bleAdvertisementsWatcher.Stop();
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

void BLE::deviceFoundHandler(IAsyncOperation<BluetoothLEDevice> const& sender, AsyncStatus const asyncStatus)
{
    //This device gets called when we've successfully created a Windows::Bluetooth::BluetoothLEDevice
    m_bleDevice = sender.get(); //nothing gets blocked as the async operation is complete

    //check to see if the device was actually found
    if (m_bleDevice.as<winrt::Windows::Foundation::IUnknown>() == NULL)
    {
        state_change_handler(BLEState::DeviceNotFound);
    }
    else
    {
        //set a handler for when the connection status of the device changes.
        m_bleDevice.ConnectionStatusChanged(winrt::Windows::Foundation::TypedEventHandler<BluetoothLEDevice, winrt::Windows::Foundation::IInspectable>(
            [this](BluetoothLEDevice device, winrt::Windows::Foundation::IInspectable eventArgs)
            {
                //We need to alert the Personal Caddie class whenever a new device is connected
                //or the current one is disconnected.
                if (device.ConnectionStatus() == Bluetooth::BluetoothConnectionStatus::Connected)
                {
                    maintain_connection = true; //this will change to false when we manually disconnect from the device
                    state_change_handler(BLEState::Connected);
                }
                else
                {
                    if (maintain_connection)
                    {
                        //If the maintain_connection boolean is set to true,
                        //then it means this disconnect happened be accident.
                        //Attempt to reconnect to the device.
                        state_change_handler(BLEState::Reconnect);
                    }
                }
            }));

        //Call the state change handler with the device found state
        //to attempt to connect to the found device
        state_change_handler(BLEState::DeviceFound);
    }
}

void BLE::terminateConnection()
{
    //If there's currently a BLE device connected, terminate the connection
    //by changing the MaintainConnection property of the GATT session and then
    //seting the m_bleDevice to nullptr.
    if (m_bleDevice.as<winrt::Windows::Foundation::IUnknown>() != NULL)
    {
        maintain_connection = false; //let the BLE connection handler know that this disonnect was on purpose
        m_bleDevice.Close();
        m_bleDevice = nullptr;
        state_change_handler(BLEState::Disconnected);
    }
}