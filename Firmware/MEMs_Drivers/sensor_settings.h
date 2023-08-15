#ifndef SENSOR_SETTINGS_H
#define SENSOR_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//This file includes different enums and structs for organizing the settings for various
//different IMU sensors

#define SENSOR_SETTINGS_LENGTH  32                                        /**< Defines the length of sensor settings array for a full IMU (in bytes) */
#define ACC_START 1                                                       /**< start of the accelerometer section of the settings array **/
#define GYR_START 11                                                      /**< start of the accelerometer section of the settings array **/
#define MAG_START 21                                                      /**< start of the accelerometer section of the settings array **/

//A list of the different sensor types
typedef enum
{
    ACC_SENSOR = 0,
    GYR_SENSOR = 1,
    MAG_SENSOR = 2,
} sensor_type_t;

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
//typedef enum
//{
//    ACC_MODEL = 0x01,
//    GYR_MODEL = 0x02,
//    MAG_MODEL = 0x03,
//    ACC_FS_RANGE = 0x04,
//    GYR_FS_RANGE = 0x05,
//    MAG_FS_RANGE = 0x06,
//    ACC_ODR = 0x07,
//    GYR_ODR = 0x08,
//    MAG_ODR = 0x09,
//    ACC_POWER = 0x0a,
//    GYR_POWER = 0x0b,
//    MAG_POWER = 0x0c,
//    ACC_FILTER_SELECTION = 0x0d,
//    ACC_LOW_PASS_FILTER = 0x0e,
//    ACC_HIGH_PASS_FILTER = 0x0f,
//    ACC_EXTRA_FILTER = 0x10,
//    GYR_FILTER_SELECTION = 0x11,
//    GYR_LOW_PASS_FILTER = 0x12,
//    GYR_HIGH_PASS_FILTER = 0x13,
//    GYR_EXTRA_FILTER = 0x14,
//    MAG_FILTER_SELECTION = 0x15,
//    MAG_LOW_PASS_FILTER = 0x16,
//    MAG_HIGH_PASS_FILTER = 0x17,
//    MAG_EXTRA_FILTER = 0x18,
//    FIFO = 0x19,
//    ACC_EXTRA_1 = 0x1a,
//    ACC_EXTRA_2 = 0x1b,
//    GYR_EXTRA_1 = 0x1c,
//    GYR_EXTRA_2 = 0x1d,
//    MAG_EXTRA_1 = 0x1e,
//    MAG_EXTRA_2 = 0x1f,
//} sensor_settings_t;

typedef enum
{
    SENSOR_MODEL = 0x00,
    FS_RANGE = 0x01,
    ODR = 0x02,
    POWER = 0x03,
    FILTER_SELECTION = 0x04,
    LOW_PASS_FILTER = 0x05,
    HIGH_PASS_FILTER = 0x06,
    EXTRA_FILTER = 0x07,
    EXTRA_1 = 0x08,
    EXTRA_2 = 0x09,
} sensor_settings_t;

void update_sensor_setting(uint8_t* settings_array, sensor_settings_t setting, uint8_t value);

//LSM9DS1 conversions
float lsm9ds1_odr_calculate(uint8_t imu_odr_setting, uint8_t mag_odr_setting);
float lsm9ds1_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SETTINGS_H */