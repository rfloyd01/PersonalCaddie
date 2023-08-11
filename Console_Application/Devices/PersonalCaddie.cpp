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

    //TODO: For now just create the standard BLE 33 Sense IMU. Ultimately want to update this to allow for other chips though,
    //like the FXOS and FXAS sensors.
    this->p_imu = new IMU(Accelerometer::BLE_SENSE_33, Gyroscope::BLE_SENSE_33, Magnetometer::BLE_SENSE_33);
    sampleFreq = 59.5; //TODO, need to actually get this from the sensor info characteristic at some point
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
    writer.WriteByte(static_cast<uint8_t>(mode));

    auto err_code = co_await this->m_settings_characteristic.WriteValueAsync(writer.DetachBuffer());

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

//Methods and fields from original BluetoothLE Class
float PersonalCaddie::getDataPoint(DataType dt, Axis a, int sample_number)
{
    if (accelerometer.size() == 0)
    {
        //was having an issue where a different part of the program was trying to access raw data after vectors were cleared, this is a quick fix for problem
        //TODO: implement a more elegant solution at some point
        return 0;
    }

    float ans; //define pointer to float vector

    //TODO: Consider making this function only take a data type and not a specific axis, that way the function won't have to be called three times to get a single data point
    if (dt == DataType::ACCELERATION) ans = accelerometer[a][sample_number];
    else if (dt == DataType::ROTATION) ans = gyroscope[a][sample_number];
    else if (dt == DataType::MAGNETIC) ans = magnetometer[a][sample_number];
    else if (dt == DataType::LINEAR_ACCELERATION) ans = linear_acceleration[a][sample_number];
    else if (dt == DataType::VELOCITY) ans = velocity[a][sample_number];
    else if (dt == DataType::LOCATION) ans = location[a][sample_number];
    else if (dt == DataType::EULER_ANGLES) ans = euler_angles[a][sample_number];

    return ans;
}

float PersonalCaddie::getRawDataPoint(DataType dt, Axis a, int sample_number)
{
    if (accelerometer.size() == 0)
    {
        //was having an issue where a different part of the program was trying to access raw data after vectors were cleared, this is a quick fix for problem
        //TODO: implement a more elegant solution at some point
        return 0;
    }

    float ans; //define pointer to float vector

    if (dt == DataType::ACCELERATION) ans = r_accelerometer[a][sample_number];
    else if (dt == DataType::ROTATION) ans = r_gyroscope[a][sample_number];
    else if (dt == DataType::MAGNETIC) ans = r_magnetometer[a][sample_number];

    return ans;
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
    bx = magnetometer[X].back();
    by = magnetometer[Y].back();
    bz = magnetometer[Z].back();

    float mag = Magnitude({ bx, by, bz });

    m_to_g = GetRotationQuaternion({ bx, by, bz }, { accelerometer[X].back(), accelerometer[Y].back(), accelerometer[Z].back() }); //g_to_m is the quaternion that will move things from the gravity frame to the magnetic frame
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
void PersonalCaddie::updateSensorData()
{
    //Updates raw data with calibration numbers and fills acceleromter, gyroscope and magnetometer vectors with updated values
    //The count variable counts which data point should be used as multiple data points are taken from the sensor at a time
    for (int i = 0; i < number_of_samples; i++)
    {
        accelerometer[X][i] = (acc_gain[0][0] * (r_accelerometer[X][i] - acc_off[0])) + (acc_gain[0][1] * (r_accelerometer[Y][i] - acc_off[1])) + (acc_gain[0][2] * (r_accelerometer[Z][i] - acc_off[2]));
        accelerometer[Y][i] = (acc_gain[1][0] * (r_accelerometer[X][i] - acc_off[0])) + (acc_gain[1][1] * (r_accelerometer[Y][i] - acc_off[1])) + (acc_gain[1][2] * (r_accelerometer[Z][i] - acc_off[2]));
        accelerometer[Z][i] = (acc_gain[2][0] * (r_accelerometer[X][i] - acc_off[0])) + (acc_gain[2][1] * (r_accelerometer[Y][i] - acc_off[1])) + (acc_gain[2][2] * (r_accelerometer[Z][i] - acc_off[2]));

        gyroscope[X][i] = (r_gyroscope[X][i] - gyr_off[0]) * gyr_gain[0];
        gyroscope[Y][i] = (r_gyroscope[Y][i] - gyr_off[1]) * gyr_gain[1];
        gyroscope[Z][i] = (r_gyroscope[Z][i] - gyr_off[2]) * gyr_gain[2];

        magnetometer[X][i] = (mag_gain[0][0] * (r_magnetometer[X][i] - mag_off[0])) + (mag_gain[0][1] * (r_magnetometer[Y][i] - mag_off[1])) + (mag_gain[0][2] * (r_magnetometer[Z][i] - mag_off[2]));
        magnetometer[Y][i] = (mag_gain[1][0] * (r_magnetometer[X][i] - mag_off[0])) + (mag_gain[1][1] * (r_magnetometer[Y][i] - mag_off[1])) + (mag_gain[1][2] * (r_magnetometer[Z][i] - mag_off[2]));
        magnetometer[Z][i] = (mag_gain[2][0] * (r_magnetometer[X][i] - mag_off[0])) + (mag_gain[2][1] * (r_magnetometer[Y][i] - mag_off[1])) + (mag_gain[2][2] * (r_magnetometer[Z][i] - mag_off[2]));

        masterUpdate(); //update rotation quaternion, lin_acc., velocity and position with every new data point that comes in

        //Each data point from the sensor is recorded at time intervals of 1/sampleFreq seconds from each other, however, this program can process them quicker than that
        //Hard code a wait time of 1/sampleFreq seconds to ensure that rendering looks smooth
        std::chrono::high_resolution_clock::time_point tt = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tt).count() <= 1000.0 / sampleFreq) {}
    }
    current_sample = 0; //since a new set of data has come in, start from the beginning of it when doing Madgwick filter
    data_available = true; //do I still need this variable?
}
void PersonalCaddie::updateMadgwick()
{
    //if (!data_available) return; //only do things if there's new data to process

    //set up current time information
    last_time_stamp = time_stamp;
    time_stamp += 1000.0 / sampleFreq;
    float delta_t = (float)((time_stamp - last_time_stamp) / 1000.0);

    //Call Madgwick function here
    int cs = current_sample; //this is only here because it became annoying to keep writing out current_sample
    //Quaternion = Madgwick(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], delta_t, beta);
    //Quaternion = MadgwickModified(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], { 0, bx, by, bz }, delta_t, beta);

    //The Madgwick filter expects the z-direction to be up, but OpenGL expects the y-direction to be up. To ensure proper rendering the y and z values are swapped as they're passed to the filter
    Quaternion = MadgwickVerticalY(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], delta_t, beta);

    //TODO: Delete below line when done with testing
    //std::cout << "Madgwick Rotation Quaternion: {" << Quaternion.w << ", " << Quaternion.x << ", " << Quaternion.y << ", " << Quaternion.z << "}" << std::endl;
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
    linear_acceleration[X][cs] = accelerometer[X][cs] - x_vector[2];
    linear_acceleration[Y][cs] = accelerometer[Y][cs] - y_vector[2];
    linear_acceleration[Z][cs] = accelerometer[Z][cs] - z_vector[2];

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

        if (linear_acceleration[X][current_sample] == 0 && (linear_acceleration[Y][current_sample] && linear_acceleration[Z][current_sample] == 0))
        {
            if (just_stopped)
            {
                if (time_stamp - end_timer > .01) //if there's been no acceleration for .1 seconds, set velocity to zero to eliminate drift
                {
                    velocity[X][current_sample] = 0;
                    velocity[Y][current_sample] = 0;
                    velocity[Z][current_sample] = 0;
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
        velocity[X][current_sample] = velocity[X][last_sample] + integrate(linear_acceleration[X][last_sample], linear_acceleration[X][current_sample], 1.0 / sampleFreq);
        velocity[Y][current_sample] = velocity[Y][last_sample] + integrate(linear_acceleration[Y][last_sample], linear_acceleration[Y][current_sample], 1.0 / sampleFreq);
        velocity[Z][current_sample] = velocity[Z][last_sample] + integrate(linear_acceleration[Z][last_sample], linear_acceleration[Z][current_sample], 1.0 / sampleFreq);

        //location variables also build on themselves so need to reference previous values
        //the minus signs are because movement was in opposite direction of what was expected
        location[X][current_sample] = location[X][last_sample] + movement_scale * integrate(velocity[X][last_sample], velocity[X][current_sample], 1.0 / sampleFreq);
        location[X][current_sample] = location[Y][last_sample] + movement_scale * integrate(velocity[Y][last_sample], velocity[Y][current_sample], 1.0 / sampleFreq);
        location[X][current_sample] = location[Z][last_sample] + movement_scale * integrate(velocity[Z][last_sample], velocity[Z][current_sample], 1.0 / sampleFreq);
    }
    else
    {
        if (linear_acceleration[X][current_sample] > lin_acc_threshold || linear_acceleration[X][current_sample] < -lin_acc_threshold) acceleration_event = 1;
        else if (linear_acceleration[Y][current_sample] > lin_acc_threshold || linear_acceleration[Y][current_sample] < -lin_acc_threshold) acceleration_event = 1;
        else if (linear_acceleration[Z][current_sample] > lin_acc_threshold || linear_acceleration[Z][current_sample] < -lin_acc_threshold) acceleration_event = 1;

        if (acceleration_event) position_timer = time_stamp;
    }
}
void PersonalCaddie::updateEulerAngles()
{
    //TODO: currently calculating these angles every loop which really shouldn't be necessary, look into creating a bool variable that lets BLEDevice know if it should calculate angles

    //calculate pitch separately to avoid NaN results
    float pitch = 2 * (Quaternion.w * Quaternion.y - Quaternion.x * Quaternion.z);
    if (pitch > 1) euler_angles[1][current_sample] = 1.570795; //case for +90 degrees
    else if (pitch < -1) euler_angles[1][current_sample] = -1.570795; //case for -90 degrees
    else  euler_angles[1][current_sample] = asinf(pitch); //all other cases

    //no NaN issues for roll and yaw
    euler_angles[0][current_sample] = atan2f(2 * (Quaternion.w * Quaternion.x + Quaternion.y * Quaternion.z), 1 - 2 * (Quaternion.x * Quaternion.x + Quaternion.y * Quaternion.y));
    euler_angles[2][current_sample] = atan2f(2 * (Quaternion.w * Quaternion.z + Quaternion.x * Quaternion.y), 1 - 2 * (Quaternion.y * Quaternion.y + Quaternion.z * Quaternion.z));
}

float PersonalCaddie::integrate(float one, float two, float dt)
{
    //Returns the area under the curve of two adjacent points on a graph
    return ((one + two) / 2) * dt;
}