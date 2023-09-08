#include "pch.h"

#include "PersonalCaddie.h"
#include "Modes/ModeScreen.h"
#include "../Math/quaternion_functions.h"
#include "../Math/sensor_fusion.h"

#include <iostream>
#include <fstream>
#include <functional>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

//PUBLIC FUNCTIONS
//Constructors
PersonalCaddie::PersonalCaddie(std::function<void(PersonalCaddieEventType, void*)> function)
{
    event_handler = function;

    this->ble_device_connected = false;
    this->p_ble = std::make_unique<BLE>(std::bind(&PersonalCaddie::BLEDeviceHandler, this, std::placeholders::_1));

    //After creating the BLE device, attempt to connect to the most recently paired
    //device. The 64-bit address of the most recently paired device is saved to a 
    //text file in the resources folder
    std::ifstream addressFile("../last_connected_device.txt");
    std::string addressString;
    uint64_t deviceAddress = 0;
    if (!addressFile.is_open())
    {
        std::getline(addressFile, addressString);
        if (addressString != "") deviceAddress = std::stoull(addressString);
        addressFile.close();
    }
    else OutputDebugString(L"Couldn't open file with previously connected Personal Caddie address.\n");

    automaticallyConnect(); //attempt to connect to the most recently connected device

    //Set the IMU and characteristic pointers to null, we need to connect to a physical 
    //device before these can be populated
    this->p_imu = nullptr;

    //Set the power mode as advertising
    this->current_power_mode = PersonalCaddiePowerMode::ADVERTISING_MODE;

    //Initialize all data vectors with zeros
    for (int dt = static_cast<int>(DataType::ACCELERATION); dt <= static_cast<int>(DataType::EULER_ANGLES); dt++)
    {
        std::vector<std::vector<float> > data_type;

        for (int axis = X; axis <= Z; axis++)
        {
            std::vector<float> data_type_axis(this->number_of_samples, 0);
            data_type.push_back(data_type_axis);
        }

        this->sensor_data.push_back(data_type);
    }
    
    //initialize all orientation quaternions to the starting position
    for (int i = 0; i < number_of_samples; i++)
    {
        glm::quat q = { 1, 0, 0, 0 };
        orientation_quaternions.push_back(q);
    }

}

void PersonalCaddie::automaticallyConnect()
{
    //We attempt to automatically connect to the last device that was connected to.
    //The address of this device is saved in a file in the local app folder. If the file
    //doesn't exist, or doesn't have a value it means that we haven't actually connected
    //to a device before.
    winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalCacheFolder();
    auto getAddressFile = localFolder.GetFileAsync(L"PersonalCaddieAddress.txt");

    getAddressFile.Completed([this, localFolder](
        IAsyncOperation<winrt::Windows::Storage::StorageFile> const& sender,
        AsyncStatus const asyncStatus)
        {
            if (asyncStatus == AsyncStatus::Error)
            {
                //this means that the file doesn't exist yet so we need to create it first.
                localFolder.CreateFileAsync(L"PersonalCaddieAddress.txt");
                p_ble->connectToDevice(0); //trying to connect with an address of 0 will cause a failure, making us connect to a device manually
            }
            else if (asyncStatus == AsyncStatus::Completed)
            {
                //read the address from the file
                auto readAddress = winrt::Windows::Storage::FileIO::ReadTextAsync(sender.get());
                readAddress.Completed([this](
                    IAsyncOperation<winrt::hstring> const& sender,
                    AsyncStatus const asyncStatus)
                    {
                        winrt::hstring addressString = sender.get();
                        uint64_t address = 0;
                        if (addressString != L"") address = std::stoull(addressString.c_str());
                        p_ble->connectToDevice(address); //try to connect with the read address. If there's no address then a value of 0 is passed in
                    });
            }
        });
}

void PersonalCaddie::updateMostRecentDeviceAddress(uint64_t address)
{
    //updates the address of the most recently connected to Personal Caddie device in the
    //local app folder
    winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalCacheFolder();
    auto getAddressFile = localFolder.GetFileAsync(L"PersonalCaddieAddress.txt");

    getAddressFile.Completed([address, localFolder](
        IAsyncOperation<winrt::Windows::Storage::StorageFile> const& sender,
        AsyncStatus const asyncStatus)
        {
            if (asyncStatus != AsyncStatus::Error)
            {
                //write the new address to the file
                winrt::Windows::Storage::FileIO::WriteTextAsync(sender.get(), std::to_wstring(address));
            }
        });
}

void PersonalCaddie::connectToDevice(uint64_t deviceAddress)
{
    //This method gets called when we attempt to connect to a device found with the device watcher.
    p_ble->connectToDevice(deviceAddress);
}

void PersonalCaddie::disconnectFromDevice()
{
    //Close the connection to the data service and then clear
    //the resources for each characteristic. We can then 
    //call the BLE class to do the same for the Bluetooth device
    //to close the connection.
    if (m_settings_characteristic.as<winrt::Windows::Foundation::IUnknown>() != NULL)
    {
        //TODO: Should put in a line here to make sure that none of the charcteristics
        //are currently set to notify

        //sever the connection to all services and set the m_services field
        //to null
        for (int i = 0; i < m_services.Size(); i++) m_services.GetAt(i).Close();
        m_services = nullptr;
        
        //set all the characteristics to null as well
        m_settings_characteristic = nullptr;
        m_accelerometer_data_characteristic = nullptr;
        m_gyroscope_data_characteristic = nullptr;
        m_magnetometer_data_characteristic = nullptr;

        p_ble->terminateConnection();
    }
}

void PersonalCaddie::BLEDeviceHandler(BLEState state)
{
    //This is my take on creating a handler function. A reference to this method gets passed to the BLE class via its cunstructor.
    //The BLE class will independently try and connect to a physical BLE device and as soon as it does, this method gets called
    //which alerts us that we can now get important info from the physical BLE device.

    //Upon first connecting we need to do two things, we need to create instances of the data and settings characteristics
    //from the BLE Device, and then we need to use data obtained from these characteristics to create an instance of the 
    //IMU class.

    //Get the Gatt Characteristics from the BLE Device. To do this we first need to get a list of all of the services from 
    //the device and then get the characteristics from each. Use the BluetoothCacheMode::Uncached flag so that we get the 
    //services from the physical device and not from what's saved in the PC's cache memory. This ensures that we are physically 
    //connecting to a device.
    switch (state)
    {
    case BLEState::DeviceFound:
    {
        std::wstring message = L"Found a Personal Caddie device, attempting to connect...\n";
        event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);

        m_services = this->p_ble->getBLEDevice()->GetGattServicesAsync(BluetoothCacheMode::Uncached).get().Services();
        if (m_services.Size() == 0)
        {
            message = L"Couldn't connect to the Personal Caddie. Go to the settings menu to manually connect.\n";
            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
            return;
        }

        //Successfully reading the Gatt services should initiate a connection with the ble device. If it does then 
        //the connectec block of this handler will execute.
        break;
    }
    case BLEState::DeviceNotFound:
    {
        std::wstring message = L"Couldn't find an existing Personal Caddie. Go to the settings menu to connect to one.";
        event_handler(PersonalCaddieEventType::PC_ALERT, (void*)&message);
        break;
    }
    case BLEState::NewAdvertisement:
    {
        //A new device was found by the advertisement watcher, send the updated device list to the mode screen class
        auto foundDevices = p_ble->getScannedDevices();
        event_handler(PersonalCaddieEventType::DEVICE_WATCHER_UPDATE, (void*)foundDevices);
        break;
    }
    case BLEState::Connected:
    {
        //We've initiated a connection to a new Personal Caddie device. Read the device to get some settings
        //off of it and use these to create an instance of th IMU class.
        this->ble_device_connected = true;
        current_power_mode = PersonalCaddiePowerMode::CONNECTED_MODE;

        std::wstring message;

        if (m_services.Size() == 0)
        {
            message = L"Couldn't connect to the Personal Caddie. Go to the settings menu to manually connect.\n";
            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
            return;
        }

        for (int i = 0; i < m_services.Size(); i++)
        {
            auto gattService = m_services.GetAt(i);
            uint16_t short_uuid = (gattService.Uuid().Data1 & 0xFFFF);

            switch (short_uuid)
            {
            case SENSOR_SERVICE_UUID:
                getDataCharacteristics(gattService);
                break;
            case SENSOR_INFO_UUID:
                OutputDebugString(L"Found the Device Info service, getting info about on-board sensors.\n");
                break;
            default:
                break;
            }
        }

        //Check to see if this device is currently paired with the computer. If it isn't, pair it for quicker
        //connection times in the future. Also, update the address of the last connect device in the file inside
        //the resources folder.
        auto pairingInformation = p_ble->getBLEDevice()->DeviceInformation().Pairing();
        if (pairingInformation.IsPaired() == false && pairingInformation.CanPair() == true) pairingInformation.PairAsync(); //asynchronously attempt to pair to the device
        updateMostRecentDeviceAddress(p_ble->getBLEDevice()->BluetoothAddress());

        //Read the sensor settings characteristic to get some basic information about the sensors attached to
        //the Personal Caddie
        uint8_t sensor_settings_array[SENSOR_SETTINGS_LENGTH] = { 0 };

        try
        {
            auto sensor_settings_buffer = m_settings_characteristic.ReadValueAsync(Bluetooth::BluetoothCacheMode::Uncached).get().Value(); //use unchached to read value from the device
            auto sensor_settings = Windows::Storage::Streams::DataReader::FromBuffer(sensor_settings_buffer);
            sensor_settings.ByteOrder(Windows::Storage::Streams::ByteOrder::LittleEndian); //the nRF52840 uses little endian so we match it here

            for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings_array[i] = sensor_settings.ReadByte();
        }
        catch (...)
        {
            OutputDebugString(L"something went wrong when reading characteristic\n");
        }

        //Use the data read from the settings characteristic to create a new IMU instance
        this->p_imu = std::make_unique<IMU>(sensor_settings_array);
        auto rates = this->p_imu->getSensorConversionRates();
        auto odrs = this->p_imu->getSensorODRs();

        sampleFreq = this->p_imu->getMaxODR(); //Set the sample frequency to be equal to the largest of the sensor ODRs

        //TODO: update calibration numbers here

        message = L"Successfully connected to the Personal Caddie\n";
        event_handler(PersonalCaddieEventType::CONNECTION_EVENT, (void*)&message);
        break;
    }
    case BLEState::Disconnected:
    {
        //The connection to the Personal Caddie has been lost, either purposely or by accident. We need
        //to alert the ModeScreen class about this disconnection so it can disable/enable certain features.
        this->ble_device_connected = false;
        current_power_mode = PersonalCaddiePowerMode::ADVERTISING_MODE;

        std::wstring message = L"Disconnected from the Personal Caddie\n";
        event_handler(PersonalCaddieEventType::CONNECTION_EVENT, (void*)&message);
        break;
    }
    }
}

void PersonalCaddie::getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service)
{
    //std::cout << "Found the Sensor Data service, extracting characteristic information now." << std::endl;
    auto data_characteristics = data_service.GetCharacteristicsAsync().get().Characteristics();

    for (int i = 0; i < data_characteristics.Size(); i++)
    {
        uint16_t short_uuid = (data_characteristics.GetAt(i).Uuid().Data1 & 0xFFFF);
        bool setup_notifcations = true;
        OutputDebugString(L"Found characteristic: \n");

        switch (short_uuid)
        {
        case ACC_DATA_CHARACTERISTIC_UUID:
            this->m_accelerometer_data_characteristic = data_characteristics.GetAt(i);
            break;
        case GYR_DATA_CHARACTERISTIC_UUID:
            this->m_gyroscope_data_characteristic = data_characteristics.GetAt(i);
            break;
        case MAG_DATA_CHARACTERISTIC_UUID:
            this->m_magnetometer_data_characteristic = data_characteristics.GetAt(i);
            break;
        case SETTINGS_CHARACTERISTIC_UUID:
            this->m_settings_characteristic = data_characteristics.GetAt(i);
            setup_notifcations = false; //the settings characteristic doesn't have notification capability
            break;
        default:
            break;
        }

        if (setup_notifcations)
        {
            //set the notification event handler for data characteristics (but don't enable notifications yet)
            data_characteristics.GetAt(i).ValueChanged(Windows::Foundation::TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs>(
                [this](GattCharacteristic car, GattValueChangedEventArgs args)
                {
                    dataCharacteristicEventHandler(car, args); //handler defined elsewhere to prevent three identical code blocks being needed here
                }));
        }
    }
}

void PersonalCaddie::dataCharacteristicEventHandler(Bluetooth::GenericAttributeProfile::GattCharacteristic& car, Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs& args)
{
    //this method gets automatically called when one of the data characteristics has its value changed and notifications turned on

    //First we read the data from the characteristic and put it into the appropriate raw data vector
    auto uuid = (car.Uuid().Data1 & 0xFFFF); //TODO: Can this just be passed in to the lambda function when the ValueChanged() property is set?
    DataType rdt = DataType::EULER_ANGLES, dt = DataType::EULER_ANGLES; //this is just to initialize the variables, they will get changed to either acc, gyr or mag equivalents
    sensor_type_t sensor_type;
    std::pair<const float*, const float**> calibration_data;

    //Get the appropriate data type by looking at the characteristic's UUID so we know which vector to update
    switch (uuid)
    {
    case ACC_DATA_CHARACTERISTIC_UUID:
        rdt = DataType::RAW_ACCELERATION;
        dt = DataType::ACCELERATION;
        sensor_type = ACC_SENSOR;
        calibration_data = this->p_imu->getAccelerometerCalibrationNumbers();
        break;
    case GYR_DATA_CHARACTERISTIC_UUID:
        rdt = DataType::RAW_ROTATION;
        dt = DataType::ROTATION;
        sensor_type = GYR_SENSOR;
        calibration_data = this->p_imu->getGyroscopeCalibrationNumbers();
        break;
    case MAG_DATA_CHARACTERISTIC_UUID:
        rdt = DataType::RAW_MAGNETIC;
        dt = DataType::MAGNETIC;
        sensor_type = MAG_SENSOR;
        calibration_data = this->p_imu->getMagnetometerCalibrationNumbers();
        break;
    default:
        return; //if something other than a data characteristic calls this handler, return without doing anything
    }

    //read the physical data
    auto read_buffer = Windows::Storage::Streams::DataReader::FromBuffer(args.CharacteristicValue());
    read_buffer.ByteOrder(Windows::Storage::Streams::ByteOrder::LittleEndian); //the nRF52840 uses little endian so we match it here

    //Transfer the data in 16-bit chunks to the appropriate data array. A single reading of the sensor is comprised of 6 bytes, 2 each 
    //for each axes so the data in the read_buffer looks like so: [XL0, XH0, YL0, YH0, ZL0, ZH0, XL1, XH1, YL1, YH1, ...]. Since the 
    //data is little endian the least significant byte comes before the most significant.

    for (int i = 0; i < this->number_of_samples; i++)
    {
        for (int axis = X; axis <= Z; axis++)
        {
            int16_t axis_reading = read_buffer.ReadInt16();
            this->sensor_data[static_cast<int>(rdt)][axis][i] = axis_reading * this->p_imu->getConversionRate(sensor_type); //Apply appropriate conversion from LSB to the current unit
        }
    }

    //once the raw data has been read update it with the appropriate calibration numbers for each sensor
    updateRawDataWithCalibrationNumbers(rdt, dt, sensor_type, calibration_data.first, calibration_data.second);
}

void PersonalCaddie::updateRawDataWithCalibrationNumbers(DataType rdt, DataType dt, sensor_type_t sensor_type, const float* offset_cal, const float** gain_cal)
{
    for (int i = 0; i < number_of_samples; i++)
    {
        float r_x = getDataPoint(rdt, X, i), r_y = getDataPoint(rdt, Y, i), r_z = getDataPoint(rdt, Z, i);

        setDataPoint(dt, X, i, (gain_cal[0][0] * (r_x - offset_cal[0])) + (gain_cal[0][1] * (r_y - offset_cal[1])) + (gain_cal[0][2] * (r_z - offset_cal[2])));
        setDataPoint(dt, Y, i, (gain_cal[1][0] * (r_x - offset_cal[0])) + (gain_cal[1][1] * (r_y - offset_cal[1])) + (gain_cal[1][2] * (r_z - offset_cal[2])));
        setDataPoint(dt, Z, i, (gain_cal[2][0] * (r_x - offset_cal[0])) + (gain_cal[2][1] * (r_y - offset_cal[1])) + (gain_cal[2][2] * (r_z - offset_cal[2])));

        //if (dt == DataType::MAGNETIC)
        //{
        //    std::cout << getDataPoint(DataType::MAGNETIC, X, i) << ", " << getDataPoint(DataType::MAGNETIC, Y, i) << ", " << getDataPoint(DataType::MAGNETIC, Z, i) << std::endl;
        //}
    }

    //since data is read separately we need to set the data updated variable to true for the current sensor
    sensor_data_updated[sensor_type] = true;
}

std::pair<const float*, const float**> PersonalCaddie::getSensorCalibrationNumbers(sensor_type_t sensor)
{
    if (sensor == ACC_SENSOR) return this->p_imu->getAccelerometerCalibrationNumbers();
    else if (sensor == GYR_SENSOR) return this->p_imu->getGyroscopeCalibrationNumbers();
    else if (sensor == MAG_SENSOR) return this->p_imu->getMagnetometerCalibrationNumbers();
    else return { nullptr, nullptr };
}

void PersonalCaddie::startDataTransfer()
{
    //Calling this method will put the Personal Caddie into the appropriate power mode
    //and then turn on the data characteristic notifications to begin the transfer of
    //data from the IMU.
    switch (current_power_mode)
    {
    case PersonalCaddiePowerMode::ADVERTISING_MODE:
        return; //we aren't even connected to a device so no reason to attempt anything
        break;
    case PersonalCaddiePowerMode::CONNECTED_MODE:
        break;
    }
}

void PersonalCaddie::enableDataNotifications()
{
    //If the sensor data characteristics aren't currently notifying, then their CCCD descriptors will be written
    //so that they are.
    auto cccdRead = m_accelerometer_data_characteristic.ReadClientCharacteristicConfigurationDescriptorAsync();

    cccdRead.Completed([this](
        IAsyncOperation<GattReadClientCharacteristicConfigurationDescriptorResult> const& sender,
        AsyncStatus const status)
        {
            auto cccd_value = sender.get().ClientCharacteristicConfigurationDescriptor();
            if (cccd_value == Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::None)
            {
                //the data characteristics are currently set to notify so turn off notifications. There are three characteristic descriptors to write
                //so nest the asynchronous calls to make sure all calls actually get processed.
                auto cccdAccWriteOff = m_accelerometer_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
                cccdAccWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                    {
                        bool accSuccess = cccdWriteHandler(sender, status);
                        if (!accSuccess)
                        {
                            std::wstring message = L"An error occured when trying to enable ACC data notifications.\n";
                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                            return;
                        }

                        //initiaite the second asynchronous call after the first one completes.
                        auto cccdGyrWriteOff = m_gyroscope_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
                        cccdGyrWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                            {
                                bool gyrSuccess = cccdWriteHandler(sender, status);
                                if (!gyrSuccess)
                                {
                                    std::wstring message = L"An error occured when trying to enable GYR data notifications.\n";
                                    event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                    return;
                                }

                                //initiaite the third asynchronous call after the second one completes.
                                auto cccdMagWriteOff = m_magnetometer_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
                                cccdMagWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                                    {
                                        bool magSuccess = cccdWriteHandler(sender, status);
                                        if (!magSuccess)
                                        {
                                            std::wstring message = L"An error occured when trying to enable MAG data notifications.\n";
                                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                            return;
                                        }
                                        else
                                        {
                                            std::wstring message = L"Successfully enabled data characteristic notifications.\n";
                                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                            return;
                                        }
                                    });
                            });
                    });
            }
        });
}

void PersonalCaddie::disableDataNotifications()
{
    //If the sensor data characteristics aren currently notifying, then their CCCD descriptors will be written
    //so that they aren't notifying any longer
    auto cccdRead = m_accelerometer_data_characteristic.ReadClientCharacteristicConfigurationDescriptorAsync();// .get().ClientCharacteristicConfigurationDescriptor();
    cccdRead.Completed([this](
        IAsyncOperation<GattReadClientCharacteristicConfigurationDescriptorResult> const& sender,
        AsyncStatus const status)
        {
            auto cccd_value = sender.get().ClientCharacteristicConfigurationDescriptor();
            if (cccd_value == Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::Notify)
            {
                //the data characteristics are currently set to notify so turn off notifications. There are three characteristic descriptors to write
                //so nest the asynchronous calls to make sure all calls actually get processed.
                auto cccdAccWriteOff = m_accelerometer_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None);
                cccdAccWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                    {
                        bool accSuccess = cccdWriteHandler(sender, status);
                        if (!accSuccess)
                        {
                            std::wstring message = L"An error occured when trying to disable ACC data notifications.\n";
                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                            return;
                        }

                        //initiaite the second asynchronous call after the first one completes.
                        auto cccdGyrWriteOff = m_gyroscope_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None);
                        cccdGyrWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                            {
                                bool gyrSuccess = cccdWriteHandler(sender, status);
                                if (!gyrSuccess)
                                {
                                    std::wstring message = L"An error occured when trying to disable GYR data notifications.\n";
                                    event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                    return;
                                }

                                //initiaite the third asynchronous call after the second one completes.
                                auto cccdMagWriteOff = m_magnetometer_data_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None);
                                cccdMagWriteOff.Completed([this](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
                                    {
                                        bool magSuccess = cccdWriteHandler(sender, status);
                                        if (!magSuccess)
                                        {
                                            std::wstring message = L"An error occured when trying to disable MAG data notifications.\n";
                                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                            return;
                                        }
                                        else
                                        {
                                            std::wstring message = L"Successfully disabled data characteristic notifications.\n";
                                            event_handler(PersonalCaddieEventType::BLE_ALERT, (void*)&message);
                                            return;
                                        }
                                    });
                            });
                    });
            }
        });
}

bool PersonalCaddie::cccdWriteHandler(IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const status)
{
    //A method for handling asynchronous writes to Client Characteristic Configuration Descriptor
    if (status != AsyncStatus::Completed)
    {
        OutputDebugString(L"Couldn't write the Client Characteristic Configuration Descriptor.\n"); //TODO: should create a UI alert
        return false;
    }

    auto writeEvent = sender.get();
    if (writeEvent != GattCommunicationStatus::Success)
    {
        OutputDebugString(L"Something when wrong trying to turn off accelerometer data notifications.\n"); //TODO: should create a UI alert
        return false;
    }

    return true; //if we make it here the write was a success
}

PersonalCaddiePowerMode PersonalCaddie::getCurrentPowerMode()
{
    return this->current_power_mode;
}

void PersonalCaddie::changePowerMode(PersonalCaddiePowerMode mode)
{
    //This method changes the current operating mode of the Personal Caddie by writing its Settings Characteristic. These modes
    //change the current power draw of the chip depending on what's currently happening and are as follows:

    /*
    * Advertising Mode:   This mode is entered when the application is first started up, or if the connection to the device is 
    *                     lost. The BLE device goes into advertisement mode to look for a Personal Caddie to connect to.
    * Connected Mode:     A Personal Caddie is connected to the application but no data readings are occuring. The IMU of the 
    *                     device is off, as well as the I2C bus used for transferring information.
    * Sensor Idle Mode:   The Personal Caddie is preparing to record data. Both the IMU and the I2C bus are turned on, but the 
    *                     IMU is placed into sleep mode to draw minimal power until data recording begins.
    * Sensor Acrive Mode: The Personal Caddie is actively recording data. The connection interval is greatly reduced in this 
    *                     mode to increase data throughput for smoother golf swing rendering.
    */

    winrt::Windows::Storage::Streams::DataWriter writer;
    writer.ByteOrder(winrt::Windows::Storage::Streams::ByteOrder::LittleEndian);
    writer.WriteByte(static_cast<uint8_t>(mode));

    auto writeOperation = this->m_settings_characteristic.WriteValueAsync(writer.DetachBuffer());

    writeOperation.Completed([this, mode](IAsyncOperation<GattCommunicationStatus> const& sender, AsyncStatus const asyncStatus)
        {
            if (asyncStatus != AsyncStatus::Completed) OutputDebugString(L"Something went wrong trying to write the Personal Caddie settings characteristic.\n");

            if (sender.get() != Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
            {
                OutputDebugString(L"Error writing Personal Caddie settings characteristic, write returned with error code: " + static_cast<int>(sender.get()));
                OutputDebugString(L"\n");
            }
            else
            {
                this->current_power_mode = mode; //update the current power mode
                std::wstring message = L"The Personal Caddie has been placed into ";
                switch (mode)
                {
                case PersonalCaddiePowerMode::CONNECTED_MODE:
                    message += L"Connected Mode";
                    break;
                case PersonalCaddiePowerMode::SENSOR_IDLE_MODE:
                    message += L"Sensor Idle Mode";
                    break;
                case PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE:
                    message += L"Sensor Active Mode ";
                    break;
                }

                //Alert the mode screen class that the mode change has taken effect
                event_handler(PersonalCaddieEventType::PC_ALERT, (void*)&message);
            }
        }
    );
}

//Methods and fields from original BluetoothLE Class
float PersonalCaddie::getDataPoint(DataType dt, Axis a, int sample_number)
{
    if (this->sensor_data[0].size() == 0)
    {
        //was having an issue where a different part of the program was trying to access raw data after vectors were cleared, this is a quick fix for problem
        //TODO: implement a more elegant solution at some point
        return 0;
    }

    return this->sensor_data[static_cast<int>(dt)][a][sample_number];
}

void PersonalCaddie::setDataPoint(DataType dt, Axis a, int sample_number, float data)
{
    if (this->sensor_data[0].size() != 0)
    {
        //was having an issue where a different part of the program was trying to access raw data after vectors were cleared, this is a quick fix for problem
        //TODO: implement a more elegant solution at some point
        
        this->sensor_data[static_cast<int>(dt)][a][sample_number] = data;
    }
}

glm::quat PersonalCaddie::getOpenGLQuaternion(int sample)
{
    //TODO: This function currently works because it's setup for the existing sensor, however, not all sensors have the same axes orientation
    //so swapping to a different sensor would mean that either this function needs to change or something would need to be swapped elsewhere.
    //Create an IMU class which will handle switching of axes itself on a sensor by sensor basis

    //The purpose of this function is to return the quaternion in a form that OpenGL likes
    //The axes of the chip are different than what OpenGL expects so the axes are switched accordingly here
    //+X OpenGL = +Y Sensor, +Y OpenGL = +Z Sensor, +Z OpenGL = +X Sensor
    //return { Quaternion.w, Quaternion.y, Quaternion.z, Quaternion.x };
    return { orientation_quaternions[sample].w, orientation_quaternions[sample].x, orientation_quaternions[sample].y, orientation_quaternions[sample].z };
}

int PersonalCaddie::getCurrentSample()
{
    return this->current_sample;
}

float PersonalCaddie::getCurrentTime()
{
    //returns the time that current sample was taken at in seconds
    return time_stamp / 1000.0;
}

void PersonalCaddie::setSampleFrequency(float freq)
{
    sampleFreq = freq;
}
void PersonalCaddie::setMagField()
{
    //TODO - Need to find a good way to rotate current magnetic reading to desired magnetic reading, this will allow the sensor to start off pointing straight when rendered
    bx = getDataPoint(DataType::MAGNETIC, X, this->number_of_samples - 1);
    bx = getDataPoint(DataType::MAGNETIC, Y, this->number_of_samples - 1);
    bx = getDataPoint(DataType::MAGNETIC, Z, this->number_of_samples - 1);

    float ax = getDataPoint(DataType::ACCELERATION, X, this->number_of_samples - 1);
    float ay = getDataPoint(DataType::ACCELERATION, Y, this->number_of_samples - 1);
    float az = getDataPoint(DataType::ACCELERATION, Z, this->number_of_samples - 1);

    float mag = Magnitude({ bx, by, bz });

    m_to_g = GetRotationQuaternion({ bx, by, bz }, { ax, ay, az }); //g_to_m is the quaternion that will move things from the gravity frame to the magnetic frame
    g_to_m = Conjugate(m_to_g);
    m_to_mprime = GetRotationQuaternion({ sqrt(bx * bx + by * by), 0, bz }, { bx, by, bz }); //this quaternion will rotate current magnetic reading to desired frame (i.e. +x and -z when pointing sensor at monitor)
}
void PersonalCaddie::resetTime()
{
    //reset time to be -1000 / the sample frequeny, this way when the first bit of data is processed it will start with a time stamp of 0.00
    time_stamp = -1000.0 / sampleFreq;
}

void PersonalCaddie::setRotationQuaternion(glm::quat q, int sample)
{
    orientation_quaternions[sample] = q;
}
void PersonalCaddie::dataUpdate()
{
    //Master update function, calls for Madgwick filter to be run and then uses that data to call updatePosition() which calculates current lin_acc., vel. and loc.
    if (sensor_data_updated[ACC_SENSOR] && sensor_data_updated[GYR_SENSOR] && sensor_data_updated[MAG_SENSOR])
    {
        //The most recent data has been read from the BLE device and had calibration data applied to it. We can 
        //now calculate any interpreted data, such as position quaternion, euler angles, linear acceleration, etc.
        updateMadgwick(); //update orientation quaternion
        updatePosition(); //use newly calculated orientation to get linear acceleration, and then integrate that to get velocity, and again for position
        updateEulerAngles(); //use newly calculated orientation quaternion to get Euler Angles of sensor (used in training modes)

        //set the current sample to 0 so the graphics module starts rendering the new data. It's possible that not 
        //all data from the last set will actually have been rendered but this is ok since each piece of data is on 
        //the scale of 10 milliseconds apart (at the most).
        current_sample = 0;

        //reset the data_updated variables to false. doing this means this method will instead increment the current
        //sample variable for the graphics module.
        sensor_data_updated[ACC_SENSOR] = false;
        sensor_data_updated[GYR_SENSOR] = false;
        sensor_data_updated[MAG_SENSOR] = false;
    }
    else
    {
        //no new data is ready so incerement the current_sample variable which let's the graphics module know what data
        //point should get rendered.
        current_sample++;

        //in the case that the graphics unit renders samples faster than they can come in, just keep rendering the 
        //last sample until new data is available. At most this should be a few milliseconds of rendering the same
        //frame which will mostly be unnoticeable.
        if (current_sample >= number_of_samples) current_sample--;
    }
}

//Internal Updating Functions
void PersonalCaddie::updateMadgwick()
{
    //set up current time information
    for (int i = 0; i < number_of_samples; i++)
    {
        //last_time_stamp = time_stamp;
        //time_stamp += 1000.0 / sampleFreq;
        float delta_t = 1000.0 / this->p_imu->getMaxODR(); //get the time (in milliseconds) between successive samples

        //TODO: Look into just sending references to the data instead of creating copies, not sure if this is really a huge time hit here but it can't hurt
        int cs = current_sample; //this is only here because it became annoying to keep writing out current_sample
        float acc_x = getDataPoint(DataType::ACCELERATION, X, cs), acc_y = getDataPoint(DataType::ACCELERATION, Y, cs), acc_z = getDataPoint(DataType::ACCELERATION, Z, cs);
        float gyr_x = getDataPoint(DataType::ROTATION, X, cs), gyr_y = getDataPoint(DataType::ROTATION, Y, cs), gyr_z = getDataPoint(DataType::ROTATION, Z, cs);
        float mag_x = getDataPoint(DataType::MAGNETIC, X, cs), mag_y = getDataPoint(DataType::MAGNETIC, Y, cs), mag_z = getDataPoint(DataType::MAGNETIC, Z, cs);

        //the first rotation quaternion of the new data set must build from the last rotation quaternion of the previous set. The rest can build off of
        //earlier samples from the current set
        if (i == 0) orientation_quaternions[i] = Madgwick(orientation_quaternions[number_of_samples - 1], gyr_x, gyr_y, gyr_z, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, this->p_imu->getMaxODR(), beta);
        else orientation_quaternions[i] = Madgwick(orientation_quaternions[i - 1], gyr_x, gyr_y, gyr_z, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, this->p_imu->getMaxODR(), beta);

        if (isinf(orientation_quaternions[i].w))
        {
            OutputDebugString(L"something went wrong\n");
        }
    }
}
void PersonalCaddie::updateLinearAcceleration()
{
    for (int i = 0; i < number_of_samples; i++)
    {
        std::vector<float> x_vector = { GRAVITY, 0, 0 };
        std::vector<float> y_vector = { 0, GRAVITY, 0 };
        std::vector<float> z_vector = { 0, 0, GRAVITY };

        QuatRotate(orientation_quaternions[i], x_vector);
        QuatRotate(orientation_quaternions[i], y_vector);
        QuatRotate(orientation_quaternions[i], z_vector);

        setDataPoint(DataType::LINEAR_ACCELERATION, X, i, getDataPoint(DataType::ACCELERATION, X, i) - x_vector[2]);
        setDataPoint(DataType::LINEAR_ACCELERATION, Y, i, getDataPoint(DataType::ACCELERATION, Y, i) - y_vector[2]);
        setDataPoint(DataType::LINEAR_ACCELERATION, Z, i, getDataPoint(DataType::ACCELERATION, Z, i) - z_vector[2]);

        //Set threshold on Linear Acceleration to help with drift
        //if (linear_acceleration[X][current_sample] < lin_acc_threshold && linear_acceleration[X][current_sample] > -lin_acc_threshold) linear_acceleration[X][current_sample] = 0;
        //if (linear_acceleration[Y][current_sample] < lin_acc_threshold && linear_acceleration[Y][current_sample] > -lin_acc_threshold) linear_acceleration[Y][current_sample] = 0;
        //if (linear_acceleration[Z][current_sample] < lin_acc_threshold && linear_acceleration[Z][current_sample] > -lin_acc_threshold) linear_acceleration[Z][current_sample] = 0;
    }
}
void PersonalCaddie::updatePosition()
{
    //TODO: this method needs to be updated so that all number_of_samples velocity and position get updated at the same time and not just one at a time
    updateLinearAcceleration();
    if (acceleration_event)
    {
        //these variables build on themselves so need to utilize previous value
        //because of the way sensor data is stored, may need to wrap around to end of vector to get previous value
        int last_sample = current_sample - 1;
        if (current_sample == 0) last_sample = number_of_samples - 1; //last data point would have been end of current vector

        if (getDataPoint(DataType::LINEAR_ACCELERATION, X, current_sample) == 0 && (getDataPoint(DataType::LINEAR_ACCELERATION, Y, current_sample) == 0 && getDataPoint(DataType::LINEAR_ACCELERATION, Z, current_sample) == 0))
        {
            if (just_stopped)
            {
                if (time_stamp - end_timer > .01) //if there's been no acceleration for .1 seconds, set velocity to zero to eliminate drift
                {
                    setDataPoint(DataType::VELOCITY, X, current_sample, 0);
                    setDataPoint(DataType::VELOCITY, Y, current_sample, 0);
                    setDataPoint(DataType::VELOCITY, Z, current_sample, 0);
                    acceleration_event = 0;
                    just_stopped = 0;
                }
            }
            else
            {
                just_stopped = 1;
                end_timer = time_stamp;
            }
        }
        else
        {
            if (just_stopped)
            {
                just_stopped = 0;
            }
        }

        //velocity variable builds on itself so need to reference previous value
        setDataPoint(DataType::VELOCITY, X, current_sample, getDataPoint(DataType::VELOCITY, X, last_sample) + integrate(getDataPoint(DataType::LINEAR_ACCELERATION, X, last_sample), getDataPoint(DataType::LINEAR_ACCELERATION, X, current_sample), 1.0 / sampleFreq));
        setDataPoint(DataType::VELOCITY, Y, current_sample, getDataPoint(DataType::VELOCITY, Y, last_sample) + integrate(getDataPoint(DataType::LINEAR_ACCELERATION, Y, last_sample), getDataPoint(DataType::LINEAR_ACCELERATION, Y, current_sample), 1.0 / sampleFreq));
        setDataPoint(DataType::VELOCITY, Z, current_sample, getDataPoint(DataType::VELOCITY, Z, last_sample) + integrate(getDataPoint(DataType::LINEAR_ACCELERATION, Z, last_sample), getDataPoint(DataType::LINEAR_ACCELERATION, Z, current_sample), 1.0 / sampleFreq));

        //location variables also build on themselves so need to reference previous values
        //(to flip direction of movement on screen minus signs can be added)
        setDataPoint(DataType::LOCATION, X, current_sample, getDataPoint(DataType::LOCATION, X, last_sample) + movement_scale * integrate(getDataPoint(DataType::VELOCITY, X, last_sample), getDataPoint(DataType::VELOCITY, X, current_sample), 1.0 / sampleFreq));
        setDataPoint(DataType::LOCATION, Y, current_sample, getDataPoint(DataType::LOCATION, Y, last_sample) + movement_scale * integrate(getDataPoint(DataType::VELOCITY, Y, last_sample), getDataPoint(DataType::VELOCITY, Y, current_sample), 1.0 / sampleFreq));
        setDataPoint(DataType::LOCATION, Z, current_sample, getDataPoint(DataType::LOCATION, Z, last_sample) + movement_scale * integrate(getDataPoint(DataType::VELOCITY, Z, last_sample), getDataPoint(DataType::VELOCITY, Z, current_sample), 1.0 / sampleFreq));
    }
    else
    {
        float lin_x = getDataPoint(DataType::LINEAR_ACCELERATION, X, current_sample), lin_y = getDataPoint(DataType::LINEAR_ACCELERATION, Y, current_sample), lin_z = getDataPoint(DataType::LINEAR_ACCELERATION, Z, current_sample);
        if (lin_x > lin_acc_threshold || lin_x < -lin_acc_threshold) acceleration_event = 1;
        else if (lin_y > lin_acc_threshold || lin_y < -lin_acc_threshold) acceleration_event = 1;
        else if (lin_z > lin_acc_threshold || lin_z < -lin_acc_threshold) acceleration_event = 1;

        if (acceleration_event) position_timer = time_stamp;
    }
}
void PersonalCaddie::updateEulerAngles()
{
    //TODO: currently calculating these angles every loop which really shouldn't be necessary, look into creating a bool variable that lets BLEDevice know if it should calculate angles

    for (int i = 0; i < number_of_samples; i++)
    {
        //calculate pitch separately to avoid NaN results
        float pitch = 2 * (orientation_quaternions[i].w * orientation_quaternions[i].y - orientation_quaternions[i].x * orientation_quaternions[i].z);
        if (pitch > 1) setDataPoint(DataType::EULER_ANGLES, Y, i, 1.570795); //case for +90 degrees
        else if (pitch < -1) setDataPoint(DataType::EULER_ANGLES, Y, i, -1.570795); //case for -90 degrees
        else  setDataPoint(DataType::EULER_ANGLES, Y, i, asinf(pitch)); //all other cases

        //no NaN issues for roll and yaw
        setDataPoint(DataType::EULER_ANGLES, X, i, atan2f(2 * (orientation_quaternions[i].w * orientation_quaternions[i].x + orientation_quaternions[i].y * orientation_quaternions[i].z), 1 - 2 * (orientation_quaternions[i].x * orientation_quaternions[i].x + orientation_quaternions[i].y * orientation_quaternions[i].y)));
        setDataPoint(DataType::EULER_ANGLES, Z, i, atan2f(2 * (orientation_quaternions[i].w * orientation_quaternions[i].z + orientation_quaternions[i].x * orientation_quaternions[i].y), 1 - 2 * (orientation_quaternions[i].y * orientation_quaternions[i].y + orientation_quaternions[i].z * orientation_quaternions[i].z)));
    }
}

float PersonalCaddie::integrate(float one, float two, float dt)
{
    //Returns the area under the curve of two adjacent points on a graph
    return ((one + two) / 2) * dt;
}