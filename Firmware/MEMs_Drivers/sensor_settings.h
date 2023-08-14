#ifndef SENSOR_SETTINGS_H
#define SENSOR_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

//This file includes different enums and structs for organizing the settings for various
//different IMU sensors

//A list of all the different accelerometers that currently have drivers
typedef enum
{
    LSM9DS1_ACC = 0x00,
} accelerometer_model_t;

//A list of all the different accelerometers that currently have drivers
typedef enum
{
    LSM9DS1_GYR = 0x00,
} gyroscope_model_t;

//A list of all the different accelerometers that currently have drivers
typedef enum
{
    LSM9DS1_MAG = 0x00,
} magnetometer_model_t;

//Settings categories for the LSM9DS1
typedef enum
{
    LSM9DS1_ACC_MODEL = 0x01,
    LSM9DS1_GYR_MODEL = 0x02,
    LSM9DS1_MAG_MODEL = 0x03,
    LSM9DS1_ACC_FS_RANGE = 0x04,
    LSM9DS1_GYR_FS_RANGE = 0x05,
    LSM9DS1_MAG_FS_RANGE = 0x06,
    LSM9DS1_ACC_GYR_ODR_POWER = 0x07,
    LSM9DS1_MAG_ODR_POWER = 0x09,
    LSM9DS1_ACC_FILTER_SELECTION = 0x0d,
    LSM9DS1_ACC_LOW_PASS_FILTER = 0x0e,
    LSM9DS1_ACC_HIGH_PASS_FILTER = 0x0f,
    LSM9DS1_ACC_ANTI_ALIASING_FILTER = 0x10,
    LSM9DS1_GYR_FILTER_SELECTION = 0x11,
    LSM9DS1_GYR_LOW_PASS_FILTER = 0x12,
    LSM9DS1_GYR_HIGH_PASS_FILTER = 0x13,

} lsm9ds1_settings_t;

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SETTINGS_H */