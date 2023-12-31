#include "pch.h"

#include "IMU.h"

//PUBLIC FUNCTIONS
//Constructors
IMU::IMU(uint8_t* imu_settings)
{
    //Constructor to create acc, gyr and mag sensors from an array of values obtained from a 
    //BLE characteristic
    p_acc = std::make_unique<Accelerometer>(imu_settings + ACC_START);
    p_gyr = std::make_unique<Gyroscope>(imu_settings + GYR_START);
    p_mag = std::make_unique<Magnetometer>(imu_settings + MAG_START);

    getODRFromSensors();
    getConversionRateFromSensors();
    //TODO: Need to get calibration info
}

std::vector<uint8_t*> IMU::getSensorSettings()
{
    //The IMU class doesn't actually hold any of the settings, each individual
    //sensor class holds its own settings. Collect the settings from all three,
    //put pointers to them in a vector and then send the vector off
    std::vector<uint8_t*> settings;

    //The order is important here, it must go acc, gyr, then mag.
    settings.push_back(p_acc->getCurrentSettings());
    settings.push_back(p_gyr->getCurrentSettings());
    settings.push_back(p_mag->getCurrentSettings());

    //Since the array is created here, we send a copy of it and 
    //not a reference to it. It's only three poitners so this is fine.
    return settings;
}

void IMU::updateSensorSettings(uint8_t* imu_settings)
{
    //If we successfully update the IMU settings then we need to update the 
    //settings arrays in each individual sensor class as well.
    p_acc->setCurrentSettings(imu_settings + ACC_START);
    p_gyr->setCurrentSettings(imu_settings + GYR_START);
    p_mag->setCurrentSettings(imu_settings + MAG_START);

    //When updating the settings we also need to update the internal
    //odr and full scale conversion rates for the IMU to make sure
    //everything lines up correctly
    getODRFromSensors();
    getConversionRateFromSensors();
}

void IMU::initializeNewSensor(uint8_t sensor_type, uint8_t* sensor_settings)
{
    //Through the sensor settings mode we have the chance to both update sensor settings,
    //and switch to new sensors entirely. If we switch to a new sensor we need to delete
    //the current one and create a new sensor object from scratch. We also need to recalculate
    //the current IMU odr, full-scale conversion rates and calibration data.
    switch (sensor_type)
    {
    case ACC_SENSOR:
        p_acc = nullptr; //delete current sensor
        p_acc = std::make_unique<Accelerometer>(sensor_settings + ACC_START);
        break;
    case GYR_SENSOR:
        p_gyr = nullptr; //delete current sensor
        p_gyr = std::make_unique<Gyroscope>(sensor_settings + GYR_START);
        break;
    case MAG_SENSOR:
        p_mag = nullptr; //delete current sensor
        p_mag = std::make_unique<Magnetometer>(sensor_settings + MAG_START);
        break;
    }

    getODRFromSensors();
    getConversionRateFromSensors();
}

float* IMU::getSensorODRs() { return this->IMU_sample_frequencies; }
float* IMU::getSensorConversionRates() { return this->IMU_data_sensitivity; }

void IMU::getODRFromSensors()
{
    //gets the odr information from each individual sensor and saves it in the IMU_sample_frequencies array

    //Some sensor's ODR depend on other sensor's ODR, for example, if both the FXOS8700 acc and mag are
    //engaged then their respective ODRs are cut in half. The sensor objects don't have access to each
    //others ODR so we need to perform that check here.
    uint8_t acc_model = p_acc->getCurrentSettings()[SENSOR_MODEL], gyr_model = p_gyr->getCurrentSettings()[SENSOR_MODEL], mag_model = p_mag->getCurrentSettings()[SENSOR_MODEL];
    if ((acc_model == FXOS8700_ACC) && (mag_model == FXOS8700_MAG))
    {
        float real_odr = fxos8700_odr_calculate(acc_model, mag_model, p_acc->getCurrentSettings()[ODR], p_mag->getCurrentSettings()[ODR]);
        p_acc->updateODR(real_odr);
        p_mag->updateODR(real_odr);
    }

    this->IMU_sample_frequencies[ACC_SENSOR] = this->p_acc->getCurrentODR();
    this->IMU_sample_frequencies[GYR_SENSOR] = this->p_gyr->getCurrentODR();
    this->IMU_sample_frequencies[MAG_SENSOR] = this->p_mag->getCurrentODR();

    //after getting all of the individual ODRs from the sensors, set the max ODR variable which is needed for
    //things like rendering and updating the Madgwick filter. Tried using std::max here but was running into
    //issues with it so just use this non-elegant approach
    this->max_odr = this->IMU_sample_frequencies[ACC_SENSOR];
    if (this->IMU_sample_frequencies[GYR_SENSOR] > this->max_odr) this->max_odr = this->IMU_sample_frequencies[GYR_SENSOR];
    if (this->IMU_sample_frequencies[MAG_SENSOR] > this->max_odr) this->max_odr = this->IMU_sample_frequencies[MAG_SENSOR];
}

void IMU::getConversionRateFromSensors()
{
    //gets the conversion rate information from each individual sensor and saves it in the IMU_data_sensitivity array
    this->IMU_data_sensitivity[ACC_SENSOR] = this->p_acc->getConversionRate();
    this->IMU_data_sensitivity[GYR_SENSOR] = this->p_gyr->getConversionRate();
    this->IMU_data_sensitivity[MAG_SENSOR] = this->p_mag->getConversionRate();
}

float IMU::getMaxODR() { return this->max_odr; }

float IMU::getConversionRate(sensor_type_t sensor) { return this->IMU_data_sensitivity[sensor]; }

std::pair<const float*, const float**> IMU::getAccelerometerCalibrationNumbers() { return this->p_acc->getCalibrationNumbers(); }
std::pair<const float*, const float**> IMU::getGyroscopeCalibrationNumbers() { return this->p_gyr->getCalibrationNumbers(); }
std::pair<const float*, const float**> IMU::getMagnetometerCalibrationNumbers() { return this->p_mag->getCalibrationNumbers(); }

std::pair<const int*, const int*> IMU::getAccelerometerAxisOrientations() { return this->p_acc->getAxisOrientations(); }
std::pair<const int*, const int*> IMU::getGyroscopeAxisOrientations() { return this->p_gyr->getAxisOrientations(); }
std::pair<const int*, const int*> IMU::getMagnetometerAxisOrientations() { return this->p_mag->getAxisOrientations(); }

void IMU::setCalibrationNumbers(sensor_type_t sensor, std::pair<float*, float**> cal_numbers)
{
    if (sensor == ACC_SENSOR) p_acc->setCalibrationNumbers(cal_numbers.first, cal_numbers.second);
    else if (sensor == GYR_SENSOR) p_gyr->setCalibrationNumbers(cal_numbers.first, cal_numbers.second);
    else if (sensor == MAG_SENSOR) p_mag->setCalibrationNumbers(cal_numbers.first, cal_numbers.second);
}

void IMU::setAxesOrientations(sensor_type_t sensor, std::pair<int*, int*> axis_orientations)
{
    if (sensor == ACC_SENSOR) p_acc->setAxesOrientations(axis_orientations.first, axis_orientations.second);
    else if (sensor == GYR_SENSOR) p_gyr->setAxesOrientations(axis_orientations.first, axis_orientations.second);
    else if (sensor == MAG_SENSOR) p_mag->setAxesOrientations(axis_orientations.first, axis_orientations.second);
}

//Get Functions
//Accelerometer IMU::getAccelerometerType()
//{
//    return IMU_acc;
//}
//Gyroscope IMU::getGyroscopeType()
//{
//    return IMU_gyr;
//}
//Magnetometer IMU::getMagnetometerType()
//{
//    return IMU_mag;
//}
//double IMU::getFrequency()
//{
//    //TODO: Currently using only accelerometer frequency for data capture, ultimately would like to be able to fill in "empty"
//    //data points when IMU frequencies don't match up
//    return IMU_sample_frequencies[0];
//}
//double IMU::getSensorSensitivity(Sensor s)
//{
//    if (s == Sensor::ACCELEROMETER) return IMU_data_sensitivity[0];
//    else if (s == Sensor::GYROSCOPE) return IMU_data_sensitivity[1];
//    else if (s == Sensor::MAGNETOMETER) return IMU_data_sensitivity[2];
//}
//Axis IMU::getOpenGLAxis(Sensor sensor_type, int sensor_axis)
//{
//    //This function returns the appropriate sensor axis in terms of the OpenGL frame
//    if (sensor_type == Sensor::ACCELEROMETER)
//    {
//        return IMU_acc_axis_order[sensor_axis];
//    }
//    else if (sensor_type == Sensor::GYROSCOPE)
//    {
//        return IMU_gyr_axis_order[sensor_axis];
//    }
//    else if (sensor_type == Sensor::MAGNETOMETER)
//    {
//        return IMU_mag_axis_order[sensor_axis];
//    }
//}
//int IMU::getAxisPolarity(Sensor sensor_type, int sensor_axis)
//{
//    //This function returns the correct polarity for each axis in the OpenGL frame. The polarity isn't as important for the overall
//    //functioning of the program as axis locations because negative values can be calibrated out, however, it's more ideal to not need to have negative
//    //calibration numbers
//    if (sensor_type == Sensor::ACCELEROMETER)
//    {
//        return IMU_acc_axis_inversion[sensor_axis];
//    }
//    else if (sensor_type == Sensor::GYROSCOPE)
//    {
//        return IMU_gyr_axis_inversion[sensor_axis];
//    }
//    else if (sensor_type == Sensor::MAGNETOMETER)
//    {
//        return IMU_mag_axis_inversion[sensor_axis];
//    }
//}
//
////Functions for loading known IMU data
//void IMU::loadSensorInformation(IMU& sensor)
//{
//    //when a sensor is created it will be loaded with the default settings as perscribed by the manufacturer
//    loadSensorDefaultSettings(sensor);
//
//    //load axis information for the sensor
//    loadAxisOrientations(sensor);
//}
//void IMU::loadSensorDefaultSettings(IMU& sensor)
//{
//    //first load accelerometer default settings
//    if (sensor.getAccelerometerType() == Accelerometer::LSM9DS1)
//    {
//        //load acc data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getAccelerometerType() == Accelerometer::FXOS8700)
//    {
//        //load acc data for the Adafruit FXOS8700 chip
//    }
//    else if (sensor.getAccelerometerType() == Accelerometer::BLE_SENSE_33)
//    {
//        //load default settings for Arduino Nano BLE 33 Sense built in accelerometer. default setting for accelerometer is
//        // +/- 4G sensitivity
//        this->IMU_sample_frequencies[0] = 400; //the sample rate for accelerometer data is 400Hz
//        this->IMU_data_sensitivity[0] = 9.80665 * 4.0 / 32768.0; //LSB coming from acc must be multiplied by this value to get actual reading
//    }
//
//    //second load gyroscope default settings
//    if (sensor.getGyroscopeType() == Gyroscope::LSM9DS1)
//    {
//        //load gyr data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getGyroscopeType() == Gyroscope::FXAS21002)
//    {
//        //load gyr data for the Adafruit FXAS21002 chip
//    }
//    else if (sensor.getGyroscopeType() == Gyroscope::BLE_SENSE_33)
//    {
//        //load default settings for Arduino Nano BLE 33 Sense built in gyroscope. default setting for gyroscope is
//        // +/- 500deg/s sensitivity
//        this->IMU_sample_frequencies[1] = 400; //the sample rate for gyroscope data is 400Hz
//        this->IMU_data_sensitivity[1] = 2000.0 / 32768.0; //LSB coming from gyr must be multiplied by this value to get actual reading
//    }
//
//    //third load magnetometer default settings
//    if (sensor.getMagnetometerType() == Magnetometer::LSM9DS1)
//    {
//        //load mag data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getMagnetometerType() == Magnetometer::FXOS8700)
//    {
//        //load mag data for the Adafruit FXOS8700 chip
//    }
//    else if (sensor.getMagnetometerType() == Magnetometer::BLE_SENSE_33)
//    {
//        //load default settings for Arduino Nano BLE 33 Sense built in magnetometer. default setting for magnetometer is
//        // +/- 100uT sensitivity
//        this->IMU_sample_frequencies[2] = 400; //the sample rate for magnetometer data is 400Hz
//        this->IMU_data_sensitivity[2] = 4.0 * 100.0 / 32768.0; //LSB coming from mag must be multiplied by this value to get actual reading
//    }
//}
//void IMU::loadAxisOrientations(IMU& sensor)
//{
//    //first load accelerometer axis orientations
//    if (sensor.getAccelerometerType() == Accelerometer::LSM9DS1)
//    {
//        //load acc data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getAccelerometerType() == Accelerometer::FXOS8700)
//    {
//        //load acc data for the Adafruit FXOS8700 chip
//    }
//    else if (sensor.getAccelerometerType() == Accelerometer::BLE_SENSE_33)
//    {
//        //load axis orientation for Arduino Nano BLE 33 Sense built in accelerometer
//        this->IMU_acc_axis_order[X] = Z; //X axis data from sensor is transferred to Z axis in OpenGL
//        this->IMU_acc_axis_order[Y] = X; //Y axis data from sensor is transferred to X axis in OpenGL
//        this->IMU_acc_axis_order[Z] = Y; //Z axis data from sensor is transferred to Y axis in OpenGL
//
//        this->IMU_acc_axis_inversion[X] =  1; //X axis data from sensor is kept positive
//        this->IMU_acc_axis_inversion[Y] = -1; //Y axis data from sensor is flipped negative
//        this->IMU_acc_axis_inversion[Z] =  1; //Z axis data from sensor is kept positive
//    }
//
//    //second load gyroscope axis orientations
//    if (sensor.getGyroscopeType() == Gyroscope::LSM9DS1)
//    {
//        //load gyr data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getGyroscopeType() == Gyroscope::FXAS21002)
//    {
//        //load gyr data for the Adafruit FXAS21002 chip
//    }
//    else if (sensor.getGyroscopeType() == Gyroscope::BLE_SENSE_33)
//    {
//        //load axis orientation for Arduino Nano BLE 33 Sense built in gyroscope
//        this->IMU_gyr_axis_order[X] = Z; //X axis data from sensor is transferred to Z axis in OpenGL
//        this->IMU_gyr_axis_order[Y] = X; //Y axis data from sensor is transferred to X axis in OpenGL
//        this->IMU_gyr_axis_order[Z] = Y; //Z axis data from sensor is transferred to Y axis in OpenGL
//
//        this->IMU_gyr_axis_inversion[X] =  1; //X axis data from sensor is kept positive
//        this->IMU_gyr_axis_inversion[Y] = -1; //Y axis data from sensor is flipped negative
//        this->IMU_gyr_axis_inversion[Z] =  1; //Z axis data from sensor is kept positive
//    }
//
//    //third load magnetometer axis orientations
//    if (sensor.getMagnetometerType() == Magnetometer::LSM9DS1)
//    {
//        //load mag data for the Adafruit LSM9DS1 chip
//    }
//    else if (sensor.getMagnetometerType() == Magnetometer::FXOS8700)
//    {
//        //load mag data for the Adafruit FXOS8700 chip
//    }
//    else if (sensor.getMagnetometerType() == Magnetometer::BLE_SENSE_33)
//    {
//        //load axis orientation for Arduino Nano BLE 33 Sense built in magnetometer
//        this->IMU_mag_axis_order[X] = Z; //X axis data from sensor is transferred to Z axis in OpenGL
//        this->IMU_mag_axis_order[Y] = X; //Y axis data from sensor is transferred to X axis in OpenGL
//        this->IMU_mag_axis_order[Z] = Y; //Z axis data from sensor is transferred to Y axis in OpenGL
//
//        this->IMU_mag_axis_inversion[X] =  1; //X axis data from sensor is kept positive
//        this->IMU_mag_axis_inversion[Y] =  1; //Y axis data from sensor is kept positive
//        this->IMU_mag_axis_inversion[Z] = -1; //Z axis data from sensor is flipped negative
//    }
//}