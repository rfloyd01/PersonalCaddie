#include "pch.h"

#include "PersonalCaddie.h"

#include <functional>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
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
}

bool PersonalCaddie::BLEDeviceDiscovered()
{
    return (this->p_ble->bleDeviceInitialized());
}

concurrency::task<void> PersonalCaddie::BLEDeviceConnectedHandler()
{
    //This is my take on creating a handler function. A reference to this method gets passed to the BLE class via its cunstructor.
    //The BLE class will independently try and connect to a physical BLE device and as soon as it does, this method gets called
    //which alerts us that we can now get important info from the physical BLE device.

    std::cout << "Connected to Personal Caddie" << std::endl;
    this->ble_device_connected = true;

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
    std::cout << "All characteristics have been found and stored." << std::endl;
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