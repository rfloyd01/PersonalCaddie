#ifndef SENSOR_SETTINGS_H
#define SENSOR_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

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
    LSM9DS1_ACC   = 0x00,
    BMI270_ACC    = 0x01,
    FXOS8700_ACC  = 0x02,
    ACC_MODEL_END = 0x03
} accelerometer_model_t;

//A list of all the different accelerometers that currently have drivers
typedef enum
{
    LSM9DS1_GYR   = 0x00,
    BMI270_GYR    = 0x01,
    FXAS21002_GYR = 0x02,
    GYR_MODEL_END = 0x03
} gyroscope_model_t;

//A list of all the different accelerometers that currently have drivers
typedef enum
{
    LSM9DS1_MAG   = 0x00,
    BMM150_MAG    = 0x01,
    FXOS8700_MAG  = 0x02,
    MAG_MODEL_END = 0x03
} magnetometer_model_t;

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

//Common Methods
uint8_t get_sensor_high_address(sensor_type_t sensor_type, uint8_t sensor_model);
uint8_t get_sensor_low_address(sensor_type_t sensor_type, uint8_t sensor_model);

const wchar_t* get_sensor_model_string(uint8_t sensor_type, uint8_t sensor_model);
const wchar_t* get_sensor_model_string_from_address(uint8_t sensor_type, uint8_t sensor_address);
void update_sensor_setting(uint8_t* settings_array, sensor_settings_t setting, uint8_t value);

void get_sensor_default_settings(uint8_t sensor_type, uint8_t sensor_model, uint8_t* settings_array);

//LSM9DS1 conversions
float lsm9ds1_compound_odr_calculate(uint8_t imu_odr_setting, uint8_t mag_odr_setting);
float lsm9ds1_odr_calculate(uint8_t* settings_array, uint8_t acc_model, uint8_t gyr_model, uint8_t mag_model, uint8_t sensor);
float lsm9ds1_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting);

const wchar_t* lsm9ds1_get_complete_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type);
const wchar_t* lsm9ds1_get_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);

void lsm9ds1_update_acc_gyr_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting, wchar_t* acc_odr_text);
void lsm9ds1_update_acc_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting);
void lsm9ds1_update_gyr_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting);
void lsm9ds1_update_mag_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting, wchar_t* mag_odr_text);

//FXOS/FXAS conversions
float fxos8700_odr_calculate(uint8_t acc_model, uint8_t mag_model, uint8_t acc_odr_setting, uint8_t mag_odr_setting);
float fxas21002_odr_calculate(uint8_t gyr_model, uint8_t odr_setting);
float fxos_fxas_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting);

const wchar_t* fxas_fxos_get_complete_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type);
const wchar_t* fxas_fxos_get_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);

void fxos8700_update_acc_and_mag_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);
void fxos8700_update_acc_or_mag_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);

//BMI/BMM conversions
float bmi_bmm_odr_calculate(uint8_t acc_model, uint8_t gyr_model, uint8_t mag_model, uint8_t* current_settings);
float bmi270_acc_odr_calculate(uint8_t odr);
float bmi270_gyr_odr_calculate(uint8_t odr);
float bmm150_mag_odr_calculate(uint8_t odr);
float bmi_bmm_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting);

const wchar_t* bmi_bmm_get_complete_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type);
const wchar_t* bmi_bmm_get_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SETTINGS_H */