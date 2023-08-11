#include "pch.h"

#include "PersonalCaddie.h"

#include <functional>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Windows::Security::Cryptography;
using namespace Bluetooth::GenericAttributeProfile;

//PUBLIC FUNCTIONS
//Constructors
PersonalCaddie::PersonalCaddie()
{
    this->ble_device_connected = false;
    this->p_ble = new BLE(std::bind(&PersonalCaddie::BLEDeviceConnectedHandler, this));

    //Set the IMU and characteristic pointers to null, we need to connect to a physical 
    //device before these can be populated
    this->p_imu = nullptr;

    //Set the power mode as advertising
    this->current_power_mode = PersonalCaddiePowerMode::ADVERTISING_MODE;
}

concurrency::task<void> PersonalCaddie::BLEDeviceConnectedHandler()
{
    //This is my take on creating a handler function. A reference to this method gets passed to the BLE class via its cunstructor.
    //The BLE class will independently try and connect to a physical BLE device and as soon as it does, this method gets called
    //which alerts us that we can now get important info from the physical BLE device.

    //Upon first connecting we need to do two things, we read the information characteristic on the device to see what kind of 
    //sensors are attached. We use this information to create an IMU object. We then create pointers to the Gatt Characteristics
    //that we'll need to read and write data from.

    //TODO: For now just create the standard BLE 33 Sense IMU. Ultimately want to update this to allow for other chips though,
    //like the FXOS and FXAS sensors.
    this->p_imu = new IMU(Accelerometer::BLE_SENSE_33, Gyroscope::BLE_SENSE_33, Magnetometer::BLE_SENSE_33);

    //Get the Gatt Characteristics from the BLE Device. To do this we first need to get a list of all of the services from 
    //the device and then get the characteristics from each
    auto gattServices = (co_await this->p_ble->getBLEDevice()->GetGattServicesAsync()).Services();
    std::cout << "Services retrieved from the device. Looking at them now...\n" << std::endl;
    for (int i = 0; i < gattServices.Size(); i++)
    {
        auto gattService = gattServices.GetAt(i);
        uint16_t short_uuid = (gattService.Uuid().Data1 & 0xFFFF);
        
        switch (short_uuid)
        {
        case SENSOR_SERVICE_UUID:
            getDataCharacteristics(gattService);
            break;
        case SENSOR_INFO_UUID:
            std::cout << "Found the Device Info service, getting info about on-board sensors." << std::endl;
            break;
        default:
            break;
        }
    }

    std::cout << "All characteristics have been found and stored. Now Connected to the Personal Caddie." << std::endl;
    this->ble_device_connected = true;
}

concurrency::task<void> PersonalCaddie::getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service)
{
    std::cout << "Found the Sensor Data service, extracting characteristic information now." << std::endl;
    auto data_characteristics = (co_await data_service.GetCharacteristicsAsync()).Characteristics();

    for (int i = 0; i < data_characteristics.Size(); i++)
    {
        uint16_t short_uuid = (data_characteristics.GetAt(i).Uuid().Data1 & 0xFFFF);

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
            break;
        default:
            break;
        }
    }
}

concurrency::task<void> PersonalCaddie::changePowerMode(PersonalCaddiePowerMode mode)
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

    switch (mode)
    {
    case PersonalCaddiePowerMode::ADVERTISING_MODE:
        writer.WriteByte(0x00);
        break;
    case PersonalCaddiePowerMode::SENSOR_IDLE_MODE:
        writer.WriteByte(0x02);
        break;
    default:
        break;
    }

    //TODO: Eventually create separate functions for each mode
    auto writeBuffer = writer.DetachBuffer();
    auto err_code = co_await this->m_settings_characteristic.WriteValueAsync(writeBuffer);

    if (err_code != Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
    {
        std::cout << "Something went wrong, characteristic write returned with error code: " << static_cast<int>(err_code) << std::endl;
    }
    else
    {
        std::cout << "The write operation was successful." << std::endl;
    }
}