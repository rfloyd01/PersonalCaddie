#include "pch.h"

#include "PersonalCaddie.h"
#include "../Math/quaternion_functions.h"
#include "../Math/sensor_fusion.h"

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
    
}

concurrency::task<void> PersonalCaddie::BLEDeviceConnectedHandler()
{
    //This is my take on creating a handler function. A reference to this method gets passed to the BLE class via its cunstructor.
    //The BLE class will independently try and connect to a physical BLE device and as soon as it does, this method gets called
    //which alerts us that we can now get important info from the physical BLE device.

    //Upon first connecting we need to do two things, we need to create instances of the data and settings characteristics
    //from the BLE Device, and then we need to use data obtained from these characteristics to create an instance of the 
    //IMU class.

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

    //Read the sensor settings characteristic to get some basic information about the sensors attached to
    //the Personal Caddie
    uint8_t sensor_settings_array[SENSOR_SETTINGS_LENGTH] = { 0 };

    try
    {
        auto sensor_settings_buffer = (co_await m_settings_characteristic.ReadValueAsync(Bluetooth::BluetoothCacheMode::Uncached)).Value(); //use unchached to read value from the device
        auto sensor_settings = Windows::Storage::Streams::DataReader::FromBuffer(sensor_settings_buffer);
        sensor_settings.ByteOrder(Windows::Storage::Streams::ByteOrder::LittleEndian); //the nRF52840 uses little endian so we match it here

        for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings_array[i] = sensor_settings.ReadByte();
    }
    catch (...)
    {
        std::cout << "something went wrong when reading characteristic" << std::endl;
    }
    

    this->p_imu = new IMU(sensor_settings_array);
    sampleFreq = 59.5; //TODO, need to actually get this from the sensor info characteristic at some point

    //Send an alert to the graphics interface letting it know that the connection has been made
    this->graphic_update_handler(34);
}

concurrency::task<void> PersonalCaddie::getDataCharacteristics(Bluetooth::GenericAttributeProfile::GattDeviceService& data_service)
{
    std::cout << "Found the Sensor Data service, extracting characteristic information now." << std::endl;
    auto data_characteristics = (co_await data_service.GetCharacteristicsAsync()).Characteristics();

    for (int i = 0; i < data_characteristics.Size(); i++)
    {
        uint16_t short_uuid = (data_characteristics.GetAt(i).Uuid().Data1 & 0xFFFF);
        bool setup_notifcations = true;

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
    DataType dt = DataType::EULER_ANGLES; //this is just to initialize the variable, it will get changed to either acc, gyr or mag

    //Get the appropriate data type by looking at the characteristic's UUID so we know which vector to update
    switch (uuid)
    {
    case ACC_DATA_CHARACTERISTIC_UUID:
        dt = DataType::RAW_ACCELERATION;
        break;
    case GYR_DATA_CHARACTERISTIC_UUID:
        dt = DataType::RAW_ROTATION;
        break;
    case MAG_DATA_CHARACTERISTIC_UUID:
        dt = DataType::RAW_MAGNETIC;
        break;
    default:
        break;
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
            this->sensor_data[static_cast<int>(dt)][axis][i] = axis_reading; //TODO: Need to convert from bytes to float
        }
    }

    //once the raw data has been read update it with the appropriate calibration numbers for each sensor
    updateRawDataWithCalibrationNumbers(dt);
}

void PersonalCaddie::updateRawDataWithCalibrationNumbers(DataType dt)
{
    for (int i = 0; i < number_of_samples; i++)
    {
        float r_x = getDataPoint(dt, X, i), r_y = getDataPoint(dt, Y, i), r_z = getDataPoint(dt, Z, i);

        if (dt == DataType::RAW_ACCELERATION)
        {
            setDataPoint(DataType::ACCELERATION, X, i, (acc_gain[0][0] * (r_x - acc_off[0])) + (acc_gain[0][1] * (r_y - acc_off[1])) + (acc_gain[0][2] * (r_z - acc_off[2])));
            setDataPoint(DataType::ACCELERATION, Y, i, (acc_gain[1][0] * (r_x - acc_off[0])) + (acc_gain[1][1] * (r_y - acc_off[1])) + (acc_gain[1][2] * (r_z - acc_off[2])));
            setDataPoint(DataType::ACCELERATION, Z, i, (acc_gain[2][0] * (r_x - acc_off[0])) + (acc_gain[2][1] * (r_y - acc_off[1])) + (acc_gain[2][2] * (r_z - acc_off[2])));
        }
        else if (dt == DataType::RAW_ROTATION)
        {
            setDataPoint(DataType::ROTATION, X, i, (r_x - gyr_off[0]) * gyr_gain[0]);
            setDataPoint(DataType::ROTATION, Y, i, (r_y - gyr_off[1]) * gyr_gain[1]);
            setDataPoint(DataType::ROTATION, Z, i, (r_z - gyr_off[2]) * gyr_gain[2]);
        }
        else if (dt == DataType::RAW_MAGNETIC)
        {
            setDataPoint(DataType::MAGNETIC, X, i, (mag_gain[0][0] * (r_x - mag_off[0])) + (mag_gain[0][1] * (r_y - mag_off[1])) + (mag_gain[0][2] * (r_z - mag_off[2])));
            setDataPoint(DataType::MAGNETIC, Y, i, (mag_gain[1][0] * (r_x - mag_off[0])) + (mag_gain[1][1] * (r_y - mag_off[1])) + (mag_gain[1][2] * (r_z - mag_off[2])));
            setDataPoint(DataType::MAGNETIC, Z, i, (mag_gain[2][0] * (r_x - mag_off[0])) + (mag_gain[2][1] * (r_y - mag_off[1])) + (mag_gain[2][2] * (r_z - mag_off[2])));
        }
    }

    current_sample = 0; //lets the graphic interface know to start rendering from the first new data point
    data_available = true; //this will let the graphics interface know that new data is ready
}

void PersonalCaddie::toggleDataCharacteristicNotifications()
{
    //If the sensor data characteristics aren't currently notifying, then their CCCD descriptors will be written
    //so that they are (and vice versa).
}

PersonalCaddiePowerMode PersonalCaddie::getCurrentPowerMode()
{
    return this->current_power_mode;
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
    writer.WriteByte(static_cast<uint8_t>(mode));

    auto err_code = co_await this->m_settings_characteristic.WriteValueAsync(writer.DetachBuffer());

    //TODO: When going into sensor active mode we need to write the notify flag of the data characteristics to actually get data

    if (err_code != Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
    {
        std::cout << "Something went wrong, characteristic write returned with error code: " << static_cast<int>(err_code) << std::endl;
    }
    else
    {
        std::cout << "The write operation was successful." << std::endl;
        this->current_power_mode = mode; //update the current power mode
    }
}

void PersonalCaddie::setGraphicsHandler(std::function<void(int)> function)
{
    //The 'function' parameter is a method that's defined in the Graphics interface. We can use this 
    //function to automatically make graphics updates when certain events happen (such as the BLE device
    //getting disconnected)
    this->graphic_update_handler = function;
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

glm::quat PersonalCaddie::getOpenGLQuaternion()
{
    //TODO: This function currently works because it's setup for the existing sensor, however, not all sensors have the same axes orientation
    //so swapping to a different sensor would mean that either this function needs to change or something would need to be swapped elsewhere.
    //Create an IMU class which will handle switching of axes itself on a sensor by sensor basis

    //The purpose of this function is to return the quaternion in a form that OpenGL likes
    //The axes of the chip are different than what OpenGL expects so the axes are switched accordingly here
    //+X OpenGL = +Y Sensor, +Y OpenGL = +Z Sensor, +Z OpenGL = +X Sensor
    //return { Quaternion.w, Quaternion.y, Quaternion.z, Quaternion.x };
    return { Quaternion.w, Quaternion.x, Quaternion.y, Quaternion.z };
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
void PersonalCaddie::updateCalibrationNumbers()
{
    int line_count = 0;
    std::fstream inFile;
    inFile.open("Resources/calibration.txt");
    char name[256];

    while (!inFile.eof())
    {
        inFile.getline(name, 256);
        ///TODO: this if statement is crappy, code up something better later, 
        if (line_count == 2)      acc_off[0] = std::stof(name);
        else if (line_count == 3) acc_off[1] = std::stof(name);
        else if (line_count == 4) acc_off[2] = std::stof(name);

        else if (line_count == 7) acc_gain[0][0] = std::stof(name);
        else if (line_count == 8) acc_gain[0][1] = std::stof(name);
        else if (line_count == 9) acc_gain[0][2] = std::stof(name);
        else if (line_count == 10) acc_gain[1][0] = std::stof(name);
        else if (line_count == 11) acc_gain[1][1] = std::stof(name);
        else if (line_count == 12) acc_gain[1][2] = std::stof(name);
        else if (line_count == 13) acc_gain[2][0] = std::stof(name);
        else if (line_count == 14) acc_gain[2][1] = std::stof(name);
        else if (line_count == 15) acc_gain[2][2] = std::stof(name);

        else if (line_count == 18) gyr_off[0] = std::stof(name);
        else if (line_count == 19) gyr_off[1] = std::stof(name);
        else if (line_count == 20) gyr_off[2] = std::stof(name);

        else if (line_count == 23) gyr_gain[0] = std::stof(name);
        else if (line_count == 24) gyr_gain[1] = std::stof(name);
        else if (line_count == 25) gyr_gain[2] = std::stof(name);

        else if (line_count == 28) mag_off[0] = std::stof(name);
        else if (line_count == 29) mag_off[1] = std::stof(name);
        else if (line_count == 30) mag_off[2] = std::stof(name);

        else if (line_count == 33) mag_gain[0][0] = std::stof(name);
        else if (line_count == 34) mag_gain[0][1] = std::stof(name);
        else if (line_count == 35) mag_gain[0][2] = std::stof(name);
        else if (line_count == 36) mag_gain[1][0] = std::stof(name);
        else if (line_count == 37) mag_gain[1][1] = std::stof(name);
        else if (line_count == 38) mag_gain[1][2] = std::stof(name);
        else if (line_count == 39) mag_gain[2][0] = std::stof(name);
        else if (line_count == 40) mag_gain[2][1] = std::stof(name);
        else if (line_count == 41) mag_gain[2][2] = std::stof(name);

        line_count++;
    }

    if (line_count < 42) std::cout << "Some calibration information wasn't updated." << std::endl;

    inFile.close();
}
void PersonalCaddie::setRotationQuaternion(glm::quat q)
{
    Quaternion = q;
}
void PersonalCaddie::masterUpdate()
{
    //Master update function, calls for Madgwick filter to be run and then uses that data to call updatePosition() which calculates current lin_acc., vel. and loc.
    updateMadgwick(); //update orientation quaternion
    updatePosition(); //use newly calculated orientation to get linear acceleration, and then integrate that to get velocity, and again for position
    updateEulerAngles(); //use newly calculated orientation quaternion to get Euler Angles of sensor (used in training modes)

    current_sample++; //once all updates have been made go on to the next sample
    if (current_sample >= number_of_samples)
    {
        data_available = false; //there is no more new data to look at, setting this variable to false would prevent Madgwick filter from running
        current_sample--; //set current sample to last sample while waiting for more data to come in
    }
}

//Internal Updating Functions
void PersonalCaddie::updateMadgwick()
{
    //if (!data_available) return; //only do things if there's new data to process

    //set up current time information
    last_time_stamp = time_stamp;
    time_stamp += 1000.0 / sampleFreq;
    float delta_t = (float)((time_stamp - last_time_stamp) / 1000.0);

    //TODO: Look into just sending references to the data instead of creating copies, not sure if this is really a huge time hit here but it can't hurt
    int cs = current_sample; //this is only here because it became annoying to keep writing out current_sample
    float acc_x = getDataPoint(DataType::ACCELERATION, X, cs), acc_y = getDataPoint(DataType::ACCELERATION, Y, cs), acc_z = getDataPoint(DataType::ACCELERATION, Z, cs);
    float gyr_x = getDataPoint(DataType::ROTATION, X, cs), gyr_y = getDataPoint(DataType::ROTATION, Y, cs), gyr_z = getDataPoint(DataType::ROTATION, Z, cs);
    float mag_x = getDataPoint(DataType::MAGNETIC, X, cs), mag_y = getDataPoint(DataType::MAGNETIC, Y, cs), mag_z = getDataPoint(DataType::MAGNETIC, Z, cs);

    //The Madgwick filter expects the z-direction to be up, but OpenGL expects the y-direction to be up. To ensure proper rendering the y and z values are swapped as they're passed to the filter
    Quaternion = MadgwickVerticalY(Quaternion, gyr_x, gyr_y, gyr_z, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, delta_t, beta);
}
void PersonalCaddie::updateLinearAcceleration()
{
    //if (!data_available) return;

    ///TODO: There's probably a quicker way to get vector than doing three quaternion rotations, update this at some point
    std::vector<float> x_vector = { GRAVITY, 0, 0 };
    std::vector<float> y_vector = { 0, GRAVITY, 0 };
    std::vector<float> z_vector = { 0, 0, GRAVITY };

    QuatRotate(Quaternion, x_vector);
    QuatRotate(Quaternion, y_vector);
    QuatRotate(Quaternion, z_vector);

    int cs = current_sample; //only put this in here because it was tedious to keep writing out current_sample
    setDataPoint(DataType::LINEAR_ACCELERATION, X, cs, getDataPoint(DataType::ACCELERATION, X, cs) - x_vector[2]);
    setDataPoint(DataType::LINEAR_ACCELERATION, Y, cs, getDataPoint(DataType::ACCELERATION, Y, cs) - y_vector[2]);
    setDataPoint(DataType::LINEAR_ACCELERATION, Z, cs, getDataPoint(DataType::ACCELERATION, Z, cs) - z_vector[2]);

    //Set threshold on Linear Acceleration to help with drift
    //if (linear_acceleration[X][current_sample] < lin_acc_threshold && linear_acceleration[X][current_sample] > -lin_acc_threshold) linear_acceleration[X][current_sample] = 0;
    //if (linear_acceleration[Y][current_sample] < lin_acc_threshold && linear_acceleration[Y][current_sample] > -lin_acc_threshold) linear_acceleration[Y][current_sample] = 0;
    //if (linear_acceleration[Z][current_sample] < lin_acc_threshold && linear_acceleration[Z][current_sample] > -lin_acc_threshold) linear_acceleration[Z][current_sample] = 0;
}
void PersonalCaddie::updatePosition()
{
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

    //calculate pitch separately to avoid NaN results
    float pitch = 2 * (Quaternion.w * Quaternion.y - Quaternion.x * Quaternion.z);
    if (pitch > 1) setDataPoint(DataType::EULER_ANGLES, Y, current_sample, 1.570795); //case for +90 degrees
    else if (pitch < -1) setDataPoint(DataType::EULER_ANGLES, Y, current_sample, -1.570795); //case for -90 degrees
    else  setDataPoint(DataType::EULER_ANGLES, Y, current_sample, asinf(pitch)); //all other cases

    //no NaN issues for roll and yaw
    setDataPoint(DataType::EULER_ANGLES, X, current_sample, atan2f(2 * (Quaternion.w * Quaternion.x + Quaternion.y * Quaternion.z), 1 - 2 * (Quaternion.x * Quaternion.x + Quaternion.y * Quaternion.y)));
    setDataPoint(DataType::EULER_ANGLES, Z, current_sample, atan2f(2 * (Quaternion.w * Quaternion.z + Quaternion.x * Quaternion.y), 1 - 2 * (Quaternion.y * Quaternion.y + Quaternion.z * Quaternion.z)));
}

float PersonalCaddie::integrate(float one, float two, float dt)
{
    //Returns the area under the curve of two adjacent points on a graph
    return ((one + two) / 2) * dt;
}