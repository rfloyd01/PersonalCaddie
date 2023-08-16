#include "pch.h"

#include "IMU.h"

//PUBLIC FUNCTIONS
//Constructors
IMU::IMU(uint8_t* imu_settings)
{
    //Constructor to create acc, gyr and mag sensors from an array of values obtained from a 
    //BLE characteristic
    p_acc = new Accelerometer(imu_settings + ACC_START);
    p_gyr = new Gyroscope(imu_settings + GYR_START);
    p_mag = new Magnetometer(imu_settings + MAG_START);

    getODRFromSensors();
    getConversionRateFromSensors();
}

float* IMU::getSensorODRs() { return this->IMU_sample_frequencies; }
float* IMU::getSensorConversionRates() { return this->IMU_data_sensitivity; }

void IMU::getODRFromSensors()
{
    //gets the odr information from each individual sensor and saves it in the IMU_sample_frequencies array
    this->IMU_sample_frequencies[ACC_SENSOR] = this->p_acc->getCurrentODR();
    this->IMU_sample_frequencies[GYR_SENSOR] = this->p_gyr->getCurrentODR();
    this->IMU_sample_frequencies[MAG_SENSOR] = this->p_mag->getCurrentODR();
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

void updateAccelerometerCalibrationNumbers(float* offset, float** gain);
void updateGyroscopeCalibrationNumbers(float* offset, float** gain);
void updateMagnetometerCalibrationNumbers(float* offset, float** gain);

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