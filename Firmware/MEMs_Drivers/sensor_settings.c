#include "sensor_settings.h"
#include "lsm9ds1_reg.h"
#include "NXP/fxos8700/src/fxos8700_regdef.h"
#include "NXP/fxos8700/src/fxos8700_driver.h"
#include "NXP/fxas21002/fxas21002_regdef.h"
#include "Bosch/bmi2_defs.h"

#include <stdio.h>
#include <string.h>

void update_sensor_setting(uint8_t* settings_array, sensor_settings_t setting, uint8_t value)
{
    //This method basically just makes sure that the setting passed in is valid and if it is,
    //the given array is updated.
    if (setting >= 0 && setting <= EXTRA_2) settings_array[setting] = value;
}

uint8_t get_sensor_high_address(sensor_type_t sensor_type, uint8_t sensor_model)
{
    //returns the slave address for the given sensor when the address pin is held high
    if (sensor_type == ACC_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_ACC: return LSM9DS1_IMU_I2C_ADD_H >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case BMI270_ACC: return BMI2_I2C_SEC_ADDR;
            case FXOS8700_ACC: return FXOS8700_DEVICE_ADDR_SA_11;
            default: return 0;
        }
    }
    else if (sensor_type == GYR_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_GYR: return LSM9DS1_IMU_I2C_ADD_H >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case BMI270_GYR: return BMI2_I2C_SEC_ADDR;
            case FXAS21002_GYR: return FXAS21002_DEVICE_ADDR_SA_1;
            default: return 0;
        }
    }
    else if (sensor_type == MAG_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_MAG: return LSM9DS1_MAG_I2C_ADD_H >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case FXOS8700_MAG: return FXOS8700_DEVICE_ADDR_SA_11;
            default: return 0;
        }
    }
}

uint8_t get_sensor_low_address(sensor_type_t sensor_type, uint8_t sensor_model)
{
    //returns the slave address for the given sensor when the address pin is held low
    if (sensor_type == ACC_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_ACC: return LSM9DS1_IMU_I2C_ADD_L >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case BMI270_ACC: return BMI2_I2C_PRIM_ADDR;
            case FXOS8700_ACC: return FXOS8700_DEVICE_ADDR_SA_10;
            default: return 0;
        }
    }
    else if (sensor_type == GYR_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_GYR: return LSM9DS1_IMU_I2C_ADD_L >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case BMI270_GYR: return BMI2_I2C_PRIM_ADDR;
            case FXAS21002_GYR: return FXAS21002_DEVICE_ADDR_SA_0;
            default: return 0;
        }
    }
    else if (sensor_type == MAG_SENSOR)
    {
        switch (sensor_model)
        {
            case LSM9DS1_MAG: return LSM9DS1_MAG_I2C_ADD_L >> 1; //saved address for LSM9DS1 is 8-bit instead of 7 bit so right shift by 1
            case FXOS8700_MAG: return FXOS8700_DEVICE_ADDR_SA_10;
            default: return 0;
        }
    }
}

const wchar_t* get_sensor_model_string(uint8_t sensor_type, uint8_t sensor_model)
{
    //Returns the name of the sensor(s) that fits the given I2C slave address.
    //NOTE: Although it currently isn't the case, it's possible for I2C devices to have the same address, so I may need some way
    //to differentiate between sensors with the same address in the future
    switch (sensor_model)
    {
    case (0): return lsm9ds1_get_complete_settings_string(sensor_type, SENSOR_MODEL);
    case (2): return fxas_fxos_get_complete_settings_string(sensor_type, SENSOR_MODEL);
    default: return L"";
    }

}

const wchar_t* get_sensor_model_string_from_address(uint8_t sensor_type, uint8_t sensor_address)
{
    //Returns the name of the sensor(s) that fits the given I2C slave address.
    //NOTE: Although it currently isn't the case, it's possible for I2C devices to have the same address, so I may need some way
    //to differentiate between sensors with the same address in the future
    switch (sensor_address)
    {
    case (LSM9DS1_IMU_I2C_ADD_L >> 1):
    case (LSM9DS1_IMU_I2C_ADD_H >> 1):
        if (sensor_type == ACC_SENSOR || sensor_type == GYR_SENSOR) return lsm9ds1_get_complete_settings_string(sensor_type, SENSOR_MODEL);
        else return L"";
    case (LSM9DS1_MAG_I2C_ADD_L): //This should be right shifted by 1, however, it conflicted with the FXOS low address. For now I'm only using high addresses but this could become an issue at some point
    case (LSM9DS1_MAG_I2C_ADD_H >> 1):
        if (sensor_type == MAG_SENSOR) return lsm9ds1_get_complete_settings_string(sensor_type, SENSOR_MODEL);
        else return L"";
    case FXOS8700_DEVICE_ADDR_SA_10:
    case FXOS8700_DEVICE_ADDR_SA_11:
        if (sensor_type == ACC_SENSOR || sensor_type == MAG_SENSOR) return fxas_fxos_get_complete_settings_string(sensor_type, SENSOR_MODEL);
        else return L"";
    case FXAS21002_DEVICE_ADDR_SA_0:
    case FXAS21002_DEVICE_ADDR_SA_1:
        if (sensor_type == GYR_SENSOR) return fxas_fxos_get_complete_settings_string(sensor_type, SENSOR_MODEL);
        else return L"";
    default: return L"";
    }
    
}

void get_sensor_default_settings(uint8_t sensor_type, uint8_t sensor_model, uint8_t* settings_array)
{
    //Load the default settings for the given sensor into the passed in settings array.
    //First, reset the given array
    for (int i = SENSOR_MODEL; i <= EXTRA_2; i++) settings_array[i] = 0;

    switch (sensor_type)
    {
    case ACC_SENSOR:
    {
        if (sensor_model == LSM9DS1_ACC)
        {
            settings_array[SENSOR_MODEL] = LSM9DS1_ACC;
            settings_array[FS_RANGE] = LSM9DS1_4g;
            settings_array[ODR] = LSM9DS1_IMU_59Hz5;
            settings_array[POWER] = LSM9DS1_IMU_59Hz5;
            settings_array[FILTER_SELECTION] = LSM9DS1_LP_OUT;
            settings_array[LOW_PASS_FILTER] = LSM9DS1_LP_DISABLE;
            settings_array[HIGH_PASS_FILTER] = 0;
            settings_array[EXTRA_FILTER] = LSM9DS1_50Hz;
        }
        else if (sensor_model == FXOS8700_ACC)
        {
            settings_array[SENSOR_MODEL] = FXOS8700_ACC;
            settings_array[FS_RANGE] = FXOS8700_XYZ_DATA_CFG_FS_4G_0P488;
            settings_array[ODR] = FXOS8700_ODR_SINGLE_100_HZ;
            settings_array[POWER] = FXOS8700_ACCEL_NORMAL;
            settings_array[FILTER_SELECTION] = FXOS8700_XYZ_DATA_CFG_HPF_OUT_DISABLE;
            settings_array[HIGH_PASS_FILTER] = FXOS8700_HP_FILTER_CUTOFF_SEL_DISABLE;
        }
        break;
    }
    case GYR_SENSOR:
    {
        if (sensor_model == LSM9DS1_GYR)
        {
            settings_array[SENSOR_MODEL] = LSM9DS1_GYR;
            settings_array[FS_RANGE] = LSM9DS1_2000dps;
            settings_array[ODR] = LSM9DS1_IMU_59Hz5;
            settings_array[POWER] = LSM9DS1_IMU_59Hz5;
            settings_array[FILTER_SELECTION] = LSM9DS1_LPF1_HPF_OUT;
            settings_array[LOW_PASS_FILTER] = 0;
            settings_array[HIGH_PASS_FILTER] = LSM9DS1_HP_MEDIUM;
        }
        else if (sensor_model == FXAS21002_GYR)
        {
            settings_array[SENSOR_MODEL] = FXAS21002_GYR;
            settings_array[FS_RANGE] = FXAS21002_RANGE_2000DPS;
            settings_array[ODR] = FXAS21002_ODR_50_HZ;
            settings_array[POWER] = FXAS21002_POWER_READY;
            settings_array[FILTER_SELECTION] = FXAS21002_FILTER_LPF_HPF;
            settings_array[LOW_PASS_FILTER] = FXAS21002_LPF_STRONG;
            settings_array[HIGH_PASS_FILTER] = FXAS21002_HPF_STRONG;
        }
        break;
    }
    case MAG_SENSOR:
    {
        if (sensor_model == LSM9DS1_MAG)
        {
            settings_array[SENSOR_MODEL] = LSM9DS1_MAG;
            settings_array[FS_RANGE] = LSM9DS1_4Ga;
            settings_array[ODR] = LSM9DS1_MAG_LP_40Hz;
            settings_array[POWER] = LSM9DS1_MAG_LP_40Hz;
        }
        else if (sensor_model == FXOS8700_MAG)
        {
            settings_array[SENSOR_MODEL] = FXOS8700_MAG;
            settings_array[ODR] = FXOS8700_ODR_SINGLE_100_HZ;
            settings_array[POWER] = 0; //indicates mag on
        }
        break;
    }
    }
}

//LSM9DS1 conversions
float lsm9ds1_compound_odr_calculate(uint8_t imu_odr_setting, uint8_t mag_odr_setting)
{
    //When all three sensors are LSM9DS1 model, this method returns the current ODR
    //of the sensor as indicated in the sensor settings array.
    //Normally the ODR is dictated by the gyroscope, however,
    //when the gyroscope is turned off then it's dictated by the accelerometer. If both the 
    //accelerometer and gyroscope are off then the ODR will be dictated by the magnetometer.

    float lsm9ds1_odr = 0.0;
    uint8_t odr_setting;
    if (imu_odr_setting == LSM9DS1_IMU_OFF)
    {
        //both the gyroscope and accelerometer are off so the ODR will be determined from the 
        //magnetometer.
        if (mag_odr_setting != LSM9DS1_MAG_POWER_DOWN)
        {
            odr_setting = (mag_odr_setting & 0x0F);
            switch (odr_setting)
            {
            case 0x00:
                if (mag_odr_setting != LSM9DS1_MAG_ONE_SHOT) lsm9ds1_odr = 0.625;
                break;
            case 0x01:
                lsm9ds1_odr = 1.25;
                break;
            case 0x02:
                lsm9ds1_odr = 2.5;
                break;
            case 0x03:
                lsm9ds1_odr = 5;
                break;
            case 0x04:
                lsm9ds1_odr = 10;
                break;
            case 0x05:
                lsm9ds1_odr = 20;
                break;
            case 0x06:
                lsm9ds1_odr = 40;
                break;
            case 0x07:
                lsm9ds1_odr = 80;
                break;
            case 0x08:
                if (mag_odr_setting == LSM9DS1_MAG_UHP_155Hz) lsm9ds1_odr = 155;
                else if (mag_odr_setting == LSM9DS1_MAG_HP_300Hz) lsm9ds1_odr = 300;
                else if (mag_odr_setting == LSM9DS1_MAG_MP_560Hz) lsm9ds1_odr = 560;
                else lsm9ds1_odr = 1000;
                break;
            default:
                lsm9ds1_odr = 0;
                break;
            }
        }
        else return 0.0; //all sensors are off so return an ODR of 0 Hz
    }
    else
    {
        //either the gyroscope or accelerometer is on.
        odr_setting = (imu_odr_setting & 0x0F);
        bool gyro_on = true;

        if (odr_setting == 0)
        {
            //the gyroscope is turned off so the ODR will be based on the accelerometer. This isn't 
            //a huge deal as the accelerometer has the same ODR settings as the gyro other than the 
            //lowest two settings
            odr_setting = ((imu_odr_setting & 0xF0) >> 4);
            gyro_on = false;
        }

        switch (odr_setting)
        {
        case 0x01:
            if (gyro_on) lsm9ds1_odr = 14.9;
            else lsm9ds1_odr = 10;
            break;
        case 0x02:
            if (gyro_on) lsm9ds1_odr = 59.5;
            else lsm9ds1_odr = 50;
            break;
        case 0x03:
            lsm9ds1_odr = 119;
            break;
        case 0x04:
            lsm9ds1_odr = 238;
            break;
        case 0x05:
            lsm9ds1_odr = 476;
            break;
        case 0x06:
            lsm9ds1_odr = 952;
            break;
        default:
            lsm9ds1_odr = 0;
            break;
        }
    }

    return lsm9ds1_odr;
}

float lsm9ds1_odr_calculate(uint8_t* settings_array, uint8_t acc_model, uint8_t gyr_model, uint8_t mag_model, uint8_t sensor)
{
    //Gets the ODR for one of the specific sensors on the LSM9DS1 chip.
    if (sensor == ACC_SENSOR) return lsm9ds1_compound_odr_calculate(settings_array[ACC_START + ODR], LSM9DS1_MAG_POWER_DOWN);
    else if (sensor == GYR_SENSOR) return lsm9ds1_compound_odr_calculate(settings_array[GYR_START + ODR], LSM9DS1_MAG_POWER_DOWN);
    else return lsm9ds1_compound_odr_calculate(LSM9DS1_IMU_OFF, settings_array[MAG_START + ODR]);
}

float lsm9ds1_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting)
{
    if (sensor == ACC_SENSOR)
    {
        switch (fsr_setting)
        {
        case LSM9DS1_2g:
            return 0.061;
        case LSM9DS1_4g:
            return 0.122;
        case LSM9DS1_8g:
            return 0.244;
        case LSM9DS1_16g:
            return 0.732;
        default:
            return 0; //invalid setting applied
        }
    }
    else if (sensor == GYR_SENSOR)
    {
        switch (fsr_setting)
        {
        case LSM9DS1_245dps:
            return 8.75;
        case LSM9DS1_500dps:
            return 17.5;
        case LSM9DS1_2000dps:
            return 70.0;
        default:
            return 0; //invalid setting applied
        }
    }
    else if (sensor == MAG_SENSOR)
    {
        switch (fsr_setting)
        {
        case LSM9DS1_4Ga:
            return 0.14;
        case LSM9DS1_8Ga:
            return 0.29;
        case LSM9DS1_12Ga:
            return 0.43;
        case LSM9DS1_16Ga:
            return 0.58;
        default:
            return 0; //invalid setting applied
        }
    }
    else return 0; //invalid setting applied

}

const wchar_t* lsm9ds1_get_complete_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type)
{
    //Creates a single string out of all the settings for a single sensor setting type. Each setting
    //is separated by a '\n' character.

    //Write out the strings instead of getting them with the lsm9ds1_get_settings_string() method so that we
    //avoid duplicates.
    switch (sensor_type)
    {
    case ACC_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Accelerometer 0x00";
        case FS_RANGE: return L"+/- 2 g 0x00\n+/- 4 g 0x02\n+/- 8 g 0x03\n+/- 16 g 0x01";
        case ODR: return L"0 Hz 0x00\n10 Hz 0x10\n14.9 Hz 0x10\n50 Hz 0x20\n59.5 Hz 0x20\n119 Hz 0x30\n238 Hz 0x40\n476 Hz 0x50\n952 Hz 0x60";
        case POWER: return L"Off 0x00\nOn 0x70";
        case FILTER_SELECTION: return L"LPF Enabled 0x0\nHPF Enabled 0x1";
        case LOW_PASS_FILTER: return L"0 Hz 0x00\nODR/50 Hz 0x10\nODR/100 Hz 0x11\nODR/9 Hz 0x12\nODR/400 Hz 0x13";
        case HIGH_PASS_FILTER: return L"ODR/50 Hz 0x0\nODR/100 Hz 0x1\nODR/9 Hz 0x2\nODR/400 Hz 0x3";
        case EXTRA_FILTER: return L"Auto 0x00\n50 Hz 0x13\n105 Hz 0x12\n211 Hz 0x11\n408 Hz 0x10";
        default: return L"";
        }
    case GYR_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Gyroscope 0x00";
        case FS_RANGE: return L"+/- 245 DPS 0x0\n+/- 500 DPS 0x1\n+/- 2000 DPS 0x3";
        case ODR: return L"0 Hz 0x00\n14.9 Hz 0x01\n59.5 Hz 0x02\n119 Hz 0x03\n238 Hz 0x04\n476 Hz 0x05\n952 Hz 0x06";
        case POWER: return L"Off 0x00\nLow Power 0x87\nNormal Mode 0x07";
        case FILTER_SELECTION: return L"LPF1 Only 0x00\nLPF1/HPF 0x01\nLPF1/LPF2 0x02\nLPF1/HPF/LPF2 0x12";
        case LOW_PASS_FILTER: return L"Ultra Light 0x3\nLight 0x2\nMedium 0x1\nStrong 0x0";
        case HIGH_PASS_FILTER: return L"Ultra Light 0x9\nLight 0x8\nUltra Low 0x7\nLow 0x6\nMedium 0x5\nHigh 0x4\nUltra High 0x3\nStrong 0x2\nUltra Strong 0x1\nExtreme 0x0";
        default: return L"";
        }
    case MAG_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Magnetometer 0x00";
        case FS_RANGE: return L"+/- 4 Ga 0x0\n+/- 8 Ga 0x1\n+/- 12 Ga 0x2\n+/- 16 Ga 0x3";
        case ODR: return L"0 Hz 0xC0\n0.625 Hz 0x00\n1.25 Hz 0x01\n2.5 Hz 0x02\n5 Hz 0x03\n10 Hz 0x04\n20 Hz 0x05\n40 Hz 0x06\n80 Hz 0x07\n155 Hz 0x08\n300 Hz 0x08\n560 Hz 0x08\n1000 Hz 0x08";
        case POWER: return L"Off 0xC0\nLow Power 0x00\nMedium Power 0x10\nHigh Power 0x20\nUltra High Power 0x30";
        default: return L"";
        }
    }
}

const wchar_t* lsm9ds1_get_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
    //This method converts a specific sensor setting to a string that can be 
    //read by a human. Mainly used in displaying settings on a UI.

    //Shouldn't need break statements because every path of the switch returns a value
    switch (sensor_type)
    {
    case ACC_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Accelerometer 0x00";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_2g: return L"+/- 2g 0x00";
            case LSM9DS1_4g: return L"+/- 4g 0x02";
            case LSM9DS1_8g: return L"+/- 8g 0x03";
            case LSM9DS1_16g: return L"+/- 16g 0x01";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case LSM9DS1_IMU_OFF:
            case LSM9DS1_XL_OFF_GY_14Hz9:
            case LSM9DS1_XL_OFF_GY_59Hz5:
            case LSM9DS1_XL_OFF_GY_119Hz:
            case LSM9DS1_XL_OFF_GY_238Hz:
            case LSM9DS1_XL_OFF_GY_476Hz:
            case LSM9DS1_XL_OFF_GY_952Hz:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
                return L"0 Hz 0x00";
            case LSM9DS1_GY_OFF_XL_10Hz: return L"10 Hz 0x10";
            case LSM9DS1_IMU_14Hz9:
            case LSM9DS1_IMU_14Hz9_LP:
                return L"14.9 Hz 0x10";
            case LSM9DS1_GY_OFF_XL_50Hz: return L"50 Hz 0x20";
            case LSM9DS1_IMU_59Hz5:
            case LSM9DS1_IMU_59Hz5_LP:
                return L"59.5 Hz 0x20";
            case LSM9DS1_IMU_119Hz:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_GY_OFF_XL_119Hz:
                return L"119 Hz 0x30";
            case LSM9DS1_IMU_238Hz:
            case LSM9DS1_GY_OFF_XL_238Hz:
                return L"238 Hz 0x40";
            case LSM9DS1_IMU_476Hz:
            case LSM9DS1_GY_OFF_XL_476Hz:
                return L"476 Hz 0x50";
            case LSM9DS1_IMU_952Hz:
            case LSM9DS1_GY_OFF_XL_952Hz:
                return L"952 Hz 0x60";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case LSM9DS1_IMU_OFF:
            case LSM9DS1_XL_OFF_GY_14Hz9:
            case LSM9DS1_XL_OFF_GY_59Hz5:
            case LSM9DS1_XL_OFF_GY_119Hz:
            case LSM9DS1_XL_OFF_GY_238Hz:
            case LSM9DS1_XL_OFF_GY_476Hz:
            case LSM9DS1_XL_OFF_GY_952Hz:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
                return L"Off 0x00";
            default: return L"On 0x70";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case LSM9DS1_LP_OUT: return L"LPF Enabled 0x0";
            case LSM9DS1_HP_OUT: return L"HPF Enabled 0x1";
            default: return L"";
            }
        case LOW_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_LP_DISABLE: return L"0 Hz 0x00";
            case LSM9DS1_LP_ODR_DIV_50: return L"ODR/50 Hz 0x10";
            case LSM9DS1_LP_ODR_DIV_100: return L"ODR/100 Hz 0x11";
            case LSM9DS1_LP_ODR_DIV_9: return L"ODR/9 Hz 0x12";
            case LSM9DS1_LP_ODR_DIV_400: return L"ODR/400 Hz 0x13";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_HP_ODR_DIV_50: return L"ODR/50 Hz 0x0";
            case LSM9DS1_HP_ODR_DIV_100: return L"ODR/100 Hz 0x1";
            case LSM9DS1_HP_ODR_DIV_9: return L"ODR/9 Hz 0x2";
            case LSM9DS1_HP_ODR_DIV_400: return L"ODR/400 Hz 0x3";
            default: return L"";
            }
        case EXTRA_FILTER:
            switch (setting)
            {
            case LSM9DS1_AUTO: return L"Auto 0x00";
            case LSM9DS1_408Hz: return L"408 Hz 0x10";
            case LSM9DS1_211Hz: return L"211 Hz 0x11";
            case LSM9DS1_105Hz: return L"105 Hz 0x12";
            case LSM9DS1_50Hz: return L"50 Hz 0x13";
            default: return L"";
            }
        default: return L"";
        }
    }
    case GYR_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Gyroscope 0x00";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_245dps: return L"+/- 245 DPS 0x0";
            case LSM9DS1_500dps: return L"+/- 500 DPS 0x1";
            case LSM9DS1_2000dps: return L"+/- 2000 DPS 0x3";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case LSM9DS1_IMU_OFF:
            case LSM9DS1_GY_OFF_XL_10Hz:
            case LSM9DS1_GY_OFF_XL_50Hz:
            case LSM9DS1_GY_OFF_XL_119Hz:
            case LSM9DS1_GY_OFF_XL_238Hz:
            case LSM9DS1_GY_OFF_XL_476Hz:
            case LSM9DS1_GY_OFF_XL_952Hz:
                return L"0 Hz 0x00";
            case LSM9DS1_IMU_14Hz9:
            case LSM9DS1_IMU_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_14Hz9:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
                return L"14.9 Hz 0x01";
            case LSM9DS1_IMU_59Hz5:
            case LSM9DS1_IMU_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
                return L"59.5 Hz 0x02";
            case LSM9DS1_IMU_119Hz:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_XL_OFF_GY_119Hz:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
                return L"119 Hz 0x03";
            case LSM9DS1_IMU_238Hz:
            case LSM9DS1_XL_OFF_GY_238Hz:
                return L"238 Hz 0x04";
            case LSM9DS1_IMU_476Hz:
            case LSM9DS1_XL_OFF_GY_476Hz:
                return L"476 Hz 0x05";
            case LSM9DS1_IMU_952Hz:
            case LSM9DS1_XL_OFF_GY_952Hz:
                return L"952 Hz 0x06";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case LSM9DS1_IMU_OFF:
            case LSM9DS1_GY_OFF_XL_10Hz:
            case LSM9DS1_GY_OFF_XL_50Hz:
            case LSM9DS1_GY_OFF_XL_119Hz:
            case LSM9DS1_GY_OFF_XL_238Hz:
            case LSM9DS1_GY_OFF_XL_476Hz:
            case LSM9DS1_GY_OFF_XL_952Hz:
                return L"Off 0x00";
            case LSM9DS1_IMU_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
            case LSM9DS1_IMU_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
                return L"Low Power 0x87";
            default: return L"Normal Mode 0x07";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case LSM9DS1_LPF1_OUT: return L"LPF1 Only 0x00";
            case LSM9DS1_LPF1_HPF_OUT: return L"LPF1/HPF 0x01";
            case LSM9DS1_LPF1_LPF2_OUT: return L"LPF1/LPF2 0x02";
            case LSM9DS1_LPF1_HPF_LPF2_OUT: return L"LPF1/HPF/LPF2 0x12";
            default: return L"";
            }
        case LOW_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_LP_STRONG: return L"Strong 0x0";
            case LSM9DS1_LP_MEDIUM: return L"Medium 0x1";
            case LSM9DS1_LP_LIGHT: return L"Light 0x2";
            case LSM9DS1_LP_ULTRA_LIGHT: return L"Ultra Light 0x3";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_HP_EXTREME: return L"Extreme 0x0";
            case LSM9DS1_HP_ULTRA_STRONG: return L"Ultra Strong 0x1";
            case LSM9DS1_HP_STRONG: return L"Strong 0x2";
            case LSM9DS1_HP_ULTRA_HIGH: return L"Ultra High 0x3";
            case LSM9DS1_HP_HIGH: return L"High 0x4";
            case LSM9DS1_HP_MEDIUM: return L"Medium 0x5";
            case LSM9DS1_HP_LOW: return L"Low 0x6";
            case LSM9DS1_HP_ULTRA_LOW: return L"Ultra Low 0x7";
            case LSM9DS1_HP_LIGHT: return L"Light 0x8";
            case LSM9DS1_HP_ULTRA_LIGHT: return L"Ultra Light 0x9";
            default: return L"";
            }
        default: return L"";
        }
    }
    case MAG_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Magnetometer 0x00";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_4Ga: return L"+/- 4 Ga 0x0";
            case LSM9DS1_8Ga: return L"+/- 8 Ga 0x1";
            case LSM9DS1_12Ga: return L"+/- 12 Ga 0x2";
            case LSM9DS1_16Ga: return L"+/- 16 Ga 0x3";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case LSM9DS1_MAG_POWER_DOWN: return L"0 Hz 0xC0";
            case LSM9DS1_MAG_LP_0Hz625:
            case LSM9DS1_MAG_MP_0Hz625:
            case LSM9DS1_MAG_HP_0Hz625:
            case LSM9DS1_MAG_UHP_0Hz625:
                return L"0.625 Hz 0x00";
            case LSM9DS1_MAG_LP_1Hz25:
            case LSM9DS1_MAG_MP_1Hz25:
            case LSM9DS1_MAG_HP_1Hz25:
            case LSM9DS1_MAG_UHP_1Hz25:
                return L"1.25 Hz 0x01";
            case LSM9DS1_MAG_LP_2Hz5:
            case LSM9DS1_MAG_MP_2Hz5:
            case LSM9DS1_MAG_HP_2Hz5:
            case LSM9DS1_MAG_UHP_2Hz5:
                return L"2.5 Hz 0x02";
            case LSM9DS1_MAG_LP_5Hz:
            case LSM9DS1_MAG_MP_5Hz:
            case LSM9DS1_MAG_HP_5Hz:
            case LSM9DS1_MAG_UHP_5Hz:
                return L"5 Hz 0x03";
            case LSM9DS1_MAG_LP_10Hz:
            case LSM9DS1_MAG_MP_10Hz:
            case LSM9DS1_MAG_HP_10Hz:
            case LSM9DS1_MAG_UHP_10Hz:
                return L"10 Hz 0x04";
            case LSM9DS1_MAG_LP_20Hz:
            case LSM9DS1_MAG_MP_20Hz:
            case LSM9DS1_MAG_HP_20Hz:
            case LSM9DS1_MAG_UHP_20Hz:
                return L"20 Hz 0x05";
            case LSM9DS1_MAG_LP_40Hz:
            case LSM9DS1_MAG_MP_40Hz:
            case LSM9DS1_MAG_HP_40Hz:
            case LSM9DS1_MAG_UHP_40Hz:
                return L"40 Hz 0x06";
            case LSM9DS1_MAG_LP_80Hz:
            case LSM9DS1_MAG_MP_80Hz:
            case LSM9DS1_MAG_HP_80Hz:
            case LSM9DS1_MAG_UHP_80Hz:
                return L"80 Hz 0x07";
            case LSM9DS1_MAG_UHP_155Hz: return L"155 Hz 0x08";
            case LSM9DS1_MAG_HP_300Hz: return L"300 Hz 0x08";
            case LSM9DS1_MAG_MP_560Hz: return L"560 Hz 0x08";
            case LSM9DS1_MAG_LP_1000Hz: return L"1000 Hz 0x08";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case LSM9DS1_MAG_POWER_DOWN: return L"Off 0xC0";
            case LSM9DS1_MAG_LP_0Hz625:
            case LSM9DS1_MAG_LP_1Hz25:
            case LSM9DS1_MAG_LP_2Hz5:
            case LSM9DS1_MAG_LP_5Hz:
            case LSM9DS1_MAG_LP_10Hz:
            case LSM9DS1_MAG_LP_20Hz:
            case LSM9DS1_MAG_LP_40Hz:
            case LSM9DS1_MAG_LP_80Hz:
            case LSM9DS1_MAG_LP_1000Hz:
                return L"Low Power 0x00";
            case LSM9DS1_MAG_MP_0Hz625:
            case LSM9DS1_MAG_MP_1Hz25:
            case LSM9DS1_MAG_MP_2Hz5:
            case LSM9DS1_MAG_MP_5Hz:
            case LSM9DS1_MAG_MP_10Hz:
            case LSM9DS1_MAG_MP_20Hz:
            case LSM9DS1_MAG_MP_40Hz:
            case LSM9DS1_MAG_MP_80Hz:
            case LSM9DS1_MAG_MP_560Hz:
                return L"Medium Power 0x10";
            case LSM9DS1_MAG_HP_0Hz625:
            case LSM9DS1_MAG_HP_1Hz25:
            case LSM9DS1_MAG_HP_2Hz5:
            case LSM9DS1_MAG_HP_5Hz:
            case LSM9DS1_MAG_HP_10Hz:
            case LSM9DS1_MAG_HP_20Hz:
            case LSM9DS1_MAG_HP_40Hz:
            case LSM9DS1_MAG_HP_80Hz:
            case LSM9DS1_MAG_HP_300Hz:
                return L"High Power 0x20";
            case LSM9DS1_MAG_UHP_0Hz625:
            case LSM9DS1_MAG_UHP_1Hz25:
            case LSM9DS1_MAG_UHP_2Hz5:
            case LSM9DS1_MAG_UHP_5Hz:
            case LSM9DS1_MAG_UHP_10Hz:
            case LSM9DS1_MAG_UHP_20Hz:
            case LSM9DS1_MAG_UHP_40Hz:
            case LSM9DS1_MAG_UHP_80Hz:
            case LSM9DS1_MAG_UHP_155Hz:
                return L"Ultra High Power 0x30";
            default: return L"Off";
            }
        default: return L"";
        }
    }
    }
}

void lsm9ds1_update_acc_gyr_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting, wchar_t* acc_odr_text)
{
    if (setting_type == ODR || setting_type == POWER)
    {
        //We're using both an LSM9DS1 accelerometer and gyroscope. These two sensors will have a shared
        //ODR and power level. Furthermore, the odr and power level settings are also tied together.
        //This means that changing any of these four settings will case the other three to update as well.

        //calcualte and save a few variables that may come in handy down below
        uint8_t accOdr = current_settings[ACC_START + ODR] & 0b01110000, accPower = current_settings[ACC_START + ODR] & 0b01110000;
        uint8_t gyrOdr = current_settings[GYR_START + ODR] & 0b00000111, gyrPower = current_settings[GYR_START + ODR] & 0b10000111;

        //If the new setting is a power level setting
        if (setting_type == POWER)
        {
            if (sensor_type == ACC_SENSOR)
            {
                if (setting == 0)
                {
                    //If we're here it means we want to turn off the accelerometer. The LSM9DS1 doesn't support a gyroscope
                    //only mode so we also need to turn off the gyro here.
                    *newSetting = 0;
                }
                else
                {
                    //We're turning the accelerometer on. Since the gyroscope can't be on alone then it's also off.
                    //Only turn on the acc for now and give at an ODR of 50 Hz
                    *newSetting |= 0b00100000;
                }
            }
            else
            {
                //we're looking at gyroscope power. The gyroscope can either be turned off, put into low power mode, or be put into 
                //normal operating mode.
                if (setting == 0)
                {
                    //If we're here it means we want to turn off the gyroscope. To do this we set bits 0, 1 and 2 to 0
                    *newSetting &= 0b01111000;
                }
                else if (setting == 0x87)
                {
                    //We're putting the gyroscope into low power, flip the MSB to 1. Only 3 ODR's are available in low power mode:
                    //14.9 Hz, 59.5 Hz and 119 Hz. If we aren't using one of these ODR's already then put the
                    //sensors at 59.5 Hz.
                    *newSetting |= 0b10000000;

                    if (gyrOdr > 3 || gyrOdr == 0)
                    {
                        *newSetting &= 0b11111000; //remove current gyr odr
                        gyrOdr = 0b00000010;
                        *newSetting |= gyrOdr; //set current gyr odr to 59.5
                    }

                    //We also need to alter the acc odr to match the gyrODR (if the acc is on). The gyroscope can't be 
                    //on alone, see if the acc is off then turn it on.
                    *newSetting &= 0b10001111; //remove current acc odr
                    *newSetting |= (gyrOdr << 4); //set current acc odr to 59.5
                }
                else if (setting == 0x07)
                {
                    //We're putting the gyroscope into normal power mode. All odr's are available at normal power mode.
                    //The only thing that we need to make sure of is that if the gyroscope was previously off we match
                    //its odr to that of the acc. 
                    if (gyrOdr == 0)
                    {
                        if (accOdr == 0)
                        {
                            //put the gyro at 59.5 Hz.
                            *newSetting = 0b00100010; //the acc must be turne on as well
                        }
                        else
                        {
                            *newSetting = accOdr | (accOdr >> 4);
                        }
                    }
                    else *newSetting &= 0b01111111; //take the sensor out of low power mode if it's currently in it
                }
            }
        }
        else
        {
            if (sensor_type == ACC_SENSOR)
            {
                //We just changed the acc odr drop down. If the gyroscope is on then we need to change its ODR
                //to match with a few exceptions. The acc can only be set at 10 or 50 Hz if the gyroscope is off,
                //so if we change to one of these odrs then the gyroscope must be turned off. Also, if we set the 
                //acc odr over a frequency of 119 Hz then the gyroscope can't be in low power mode so we need to
                //take it out if it is. Finally, if we turn the acc off by making its odr 0 then we don't need
                //to change anything with the gyroscope.
                if (setting == 0)
                {
                    //an odr reading of 0 will turn off the accelerometer. The LSM9DS1 doesn't have a gyroscope
                    //only mode so we also need to turn off the gyro here.
                    *newSetting = 0;
                }
                else if (setting == 0x10 || setting == 0x20)
                {
                    //both of these options have the potential to turn off the gyro, but we can't know without looking
                    //at the text in the acc odr drop down.
                    
                    if (!wcscmp(acc_odr_text, L"10 Hz 0x10") || !wcscmp(acc_odr_text, L"50 Hz 0x20"))
                    {
                        //we need to turn off the gyroscope. Set all settings to the new setting
                        *newSetting = setting;
                    }
                    else
                    {
                        //We've selected an ODR that requires the gyroscope to be on. Make the gyroscope odr
                        //match the acc one, even if we need to turn on the gyro to do it.
                        *newSetting = setting | (setting >> 4);
                    }
                }
                else
                {
                    //We picked one of the standard ODR's, match the gyroscope ODR (unless the gyro is off). If the ODR is above
                    //119 Hz then make sure the gyro isn't in low power mode.
                    *newSetting &= 0b10001111; //remove the current acc odr setting
                    if (*newSetting & 0b00000111)
                    {
                        //the gyro is on so match the odrs
                        if ((*newSetting & 0b10000000) && (setting > 0x30))
                        {
                            //the gyro is in low power mode and the new odr is too high, take the gyro
                            //out of low power mode
                            *newSetting &= 0b01111111;
                        }

                        *newSetting = (*newSetting & 0b10000000) | (setting >> 4);
                    }
                   * newSetting |= setting;
                }
            }
            else
            {
                //we just changed to gyroscope odr drop down menu. All we really need to do is see if
                //this new odr will take us out of lower power mode and make sure that the acc odr matches.
                if (setting > 3 && (*newSetting & 0b10000000))
                {
                    //we need to leave low power mode 
                    *newSetting &= 0b01111111;
                }

                //make the acc and gyr odrs match (if the acc is off then turn it on because the gyroscope can't be
                //on without the acc). If the gyroscope is off then it will turn on here
                //and default to normal power mode.
                *newSetting = (*newSetting & 0b10000000) | (setting << 4 | setting); //remove existing acc odr while maintaining low power mode if it's engaged
                //newSetting &= 0b11111000; //remove the current gyr odr
                *newSetting |= setting;
            }
        }
    }
    else
    {
        //We've selected a setting that doesn't affect any other settings. We just need to update the
        //m_new settings array accordingly.
        uint8_t sensor_start_locations[3] = { ACC_START, GYR_START, MAG_START };
        current_settings[sensor_start_locations[sensor_type] + setting_type] = setting;
    }
}

void lsm9ds1_update_acc_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting)
{
    if (setting_type == POWER)
    {
        if (setting == 0)
        {
            //If we're here it means we want to turn off the accelerometer. Doing this will also
            //set the ODR to 0
            *newSetting = 0;
        }
        else
        {
            //We're turning the accelerometer on. Put the ODR at 50 Hz if it is currently off,
            //otherwise leave it alone
            if (current_settings[ACC_START + POWER] == 0) *newSetting |= 0b00100000;
            else
            {
                int x = 5;
            }
        }
    }
    else if (setting_type == ODR)
    {
        if (setting == 0)
        {
            //an odr reading of 0 will turn off the accelerometer.
            *newSetting = 0;
        }
        else
        {
            //We picked one of the standard ODR's, remove the current setting and apply the new one
            *newSetting &= 0b10001111;
            *newSetting |= setting;
        }
    }
    else
    {
        //We've selected a setting that doesn't affect any other settings. We just need to update the
        //m_new settings array accordingly.
        current_settings[ACC_START + setting_type] = setting;
    }
}

void lsm9ds1_update_gyr_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting)
{
    //It's not possible to use the LSM9DS1 gyroscope without the LSM9DS1 accelerometer also being on. That said, we can still
    //mix and match sensors and use this gyr with a different acc (we'll just get a hit on power consumption for using two different
    //IMU chips). Changes made here won't effect other sensors, however, the gyroscope has multiple power modes which can effect
    //its own ODR.

    uint8_t gyrOdr = current_settings[GYR_START + ODR] & 0b00000111, gyrPower = current_settings[GYR_START + ODR] & 0b10000111;
    if (setting_type == POWER)
    {
        //We're looking at gyroscope power. The gyroscope can either be turned off, put into low power mode, or be put into 
        //normal operating mode.
        if (setting == 0)
        {
            //If we're here it means we want to turn off the gyroscope. To do this we set bits 0, 1 and 2 to 0
            *newSetting = 0;
        }
        else if (setting == 0x87)
        {
            //We're putting the gyroscope into low power, flip the MSB to 1. Only 3 ODR's are available in low power mode:
            //14.9 Hz, 59.5 Hz and 119 Hz. If we aren't using one of these ODR's already then put the
            //sensors at 59.5 Hz.
            *newSetting |= 0b10000000;

            if (gyrOdr > 3 || gyrOdr == 0)
            {
                *newSetting &= 0b11111000; //remove current gyr odr
                gyrOdr = 0b00000010;
                *newSetting |= gyrOdr; //set current gyr odr to 59.5
            }

            //We also need to alter the acc odr to match the gyrODR (even though the acc isn't actually there)
            *newSetting &= 0b10001111; //remove current acc odr
            *newSetting |= (gyrOdr << 4); //set current acc odr to 59.5
        }
        else if (setting == 0x07)
        {
            //We're putting the gyroscope into normal power mode. All odr's are available at normal power mode.
            //The only thing that we need to make sure of is that if the gyroscope was previously off we turn it on
            if (gyrOdr == 0)
            {
                //put the gyro at 59.5 Hz.
                *newSetting = 0b00100010; //the acc must be turne on as well
            }
        }
    }
    else if (setting_type == ODR)
    {
        if (setting == 0)
        {
            //an odr reading of 0 will turn off the accelerometer.
            *newSetting = 0;
        }
        else
        {
            //We picked one of the standard ODR's, remove the current odr from the gyr and acc
            //(even though the acc isn't present, we need to pretend like it is to make sure the
            //enum values are correct)
            *newSetting &= 0b10000000;

            //if we were currently in low power mode, but moved to an odr that's too high for it then
            //we need to get rid of low power mode
            if (setting > 3) *newSetting &= 0b01111111;
            *newSetting |= ((setting << 4) | setting);
        }
    }
    else
    {
        //We've selected a setting that doesn't affect any other settings. We just need to update the
        //m_new settings array accordingly.
        current_settings[GYR_START + setting_type] = setting;
    }
}

void lsm9ds1_update_mag_setting(uint8_t* current_settings, sensor_settings_t setting_type, uint8_t* newSetting, uint8_t setting, wchar_t* mag_odr_text)
{
    //we're looking at the LSM9DS12 magnetometer. The ODR and power level are tied to each other so changing one will change the other.
        //The full scale range doesn't have any carry over effects though.
    

    if (setting_type == ODR)
    {
        //Update the mag odr setting
        current_settings[MAG_START + ODR] &= 0xF0; //remove the current odr setting (least significant byte)
        current_settings[MAG_START + ODR] |= setting; //apply the new odr

        //If one of the 0x08 odr options is selected it will cause the power level to change,
        //otherwise the power level remains the same.
        if (setting == 0x08)
        {
            //there are four options here, each one will causes us to go to a different power level.
            //We look at the actual text in the drop down to figure out which power level we need.
            current_settings[MAG_START + POWER] = 0x08; //erase the current power mode and set the second byte to 8
            
            if (!wcscmp(mag_odr_text, L"155 Hz 0x08")) current_settings[MAG_START + POWER] |= 0x30; //put the sensor into ultra high power mode
            else if (!wcscmp(mag_odr_text, L"300 Hz 0x08")) current_settings[MAG_START + POWER] |= 0x20; //put the sensor into high power mode
            else if (!wcscmp(mag_odr_text, L"560 Hz 0x08")) current_settings[MAG_START + POWER] |= 0x10; //put the sensor into medium power mode
            //if none of the three if statements above gets triggered then the chip will be put into low power mode

            current_settings[MAG_START + ODR] = current_settings[MAG_START + POWER]; //update the odr setting to reflect the power setting
        }
        else if (setting == 0xC0)
        {
            //putting the odr at 0 hz will turn off the magnetometer
            current_settings[MAG_START + ODR] = setting;
            current_settings[MAG_START + POWER] = setting;
        }
        else
        {
            //a normal odr was chosen. If the magnetometer is off turn it on by placing it into
            //low power mode
            if (current_settings[MAG_START + POWER] == 0xC0)
            {
                current_settings[MAG_START + ODR] &= 0x0F;
            }
            current_settings[MAG_START + POWER] = current_settings[MAG_START + ODR];;
        }
    }
    else if (setting_type == POWER)
    {
        //Changing the power will only effect the ODR if we're in high odr mode. This should happen automatically
        //though, just update the settings.
        current_settings[MAG_START + POWER] &= 0x0F; //erase the current power setting
        current_settings[MAG_START + POWER] |= setting; //erase the current power setting

        if (setting == 0xC0) current_settings[MAG_START + POWER] = setting; //turning the power off changes the whole setting
        current_settings[MAG_START + ODR] = current_settings[MAG_START + POWER];
    }
    else current_settings[MAG_START + setting_type] = setting; //update the full scale range
}

//FXAS/FXOS conersions

float fxos8700_odr_calculate(uint8_t acc_model, uint8_t mag_model, uint8_t acc_odr_setting, uint8_t mag_odr_setting)
{
    //Check to see if the acc model and mag model are both FXOS sensors, if so, the ODR will be 
    //cut in half with both sensors engaged. If one (or both) of the sensors are turned off then
    //the ODR setting will have a value of 0xFF.
    bool acc_on = (acc_model == FXOS8700_ACC), mag_on = (mag_model == FXOS8700_MAG);
    if (acc_on && mag_on)
    {
        if (acc_odr_setting == 0xFF) acc_on = false;
        else if (mag_odr_setting == 0xFF) mag_on = false;
        else
        {
            //Both sensors are on which means the ODR values for each sensor
            //are the same. Return the appropriate hybrid ODR
            switch (acc_odr_setting)
            {
            case FXOS8700_ODR_HYBRID_400_HZ: return 400.0;
            case FXOS8700_ODR_HYBRID_200_HZ: return 200.0;
            case FXOS8700_ODR_HYBRID_100_HZ: return 100.0;
            case FXOS8700_ODR_HYBRID_50_HZ: return 50.0;
            case FXOS8700_ODR_HYBRID_25_HZ: return 25.0;
            case FXOS8700_ODR_HYBRID_6P25_HZ: return 6.25;
            case FXOS8700_ODR_HYBRID_3P125_HZ: return 3.125;
            case FXOS8700_ODR_HYBRID_0P7813_HZ: return 0.7813;
            default: return 0.0;
            }
        }
    }

    if (acc_on)
    {
        //Only the acc is on (or present) so return the acc only value
        switch (acc_odr_setting)
        {
        case FXOS8700_ODR_SINGLE_800_HZ: return 800.0;
        case FXOS8700_ODR_SINGLE_400_HZ: return 400.0;
        case FXOS8700_ODR_SINGLE_200_HZ: return 200.0;
        case FXOS8700_ODR_SINGLE_100_HZ: return 100.0;
        case FXOS8700_ODR_SINGLE_50_HZ: return 50.0;
        case FXOS8700_ODR_SINGLE_12P5_HZ: return 12.5;
        case FXOS8700_ODR_SINGLE_6P25_HZ: return 6.25;
        case FXOS8700_ODR_SINGLE_1P5625_HZ: return 1.5625;
        default: return 0.0;
        }
    }
    else if (mag_on)
    {
        //Only the mag is on (or present) so return the mag only value
        switch (mag_odr_setting)
        {
        case FXOS8700_ODR_SINGLE_800_HZ: return 800.0;
        case FXOS8700_ODR_SINGLE_400_HZ: return 400.0;
        case FXOS8700_ODR_SINGLE_200_HZ: return 200.0;
        case FXOS8700_ODR_SINGLE_100_HZ: return 100.0;
        case FXOS8700_ODR_SINGLE_50_HZ: return 50.0;
        case FXOS8700_ODR_SINGLE_12P5_HZ: return 12.5;
        case FXOS8700_ODR_SINGLE_6P25_HZ: return 6.25;
        case FXOS8700_ODR_SINGLE_1P5625_HZ: return 1.5625;
        default: return 0.0;
        }
    }
    else return 0.0; //If neither sensor is on then the ODR is 0 Hz
}

float fxas21002_odr_calculate(uint8_t gyr_model, uint8_t odr_setting)
{
    if (gyr_model == FXAS21002_GYR)
    {
        switch (odr_setting)
        {
        case FXAS21002_ODR_800_HZ: return 800.0;
        case FXAS21002_ODR_400_HZ: return 400.0;
        case FXAS21002_ODR_200_HZ: return 200.0;
        case FXAS21002_ODR_100_HZ: return 100.0;
        case FXAS21002_ODR_50_HZ: return 50.0;
        case FXAS21002_ODR_25_HZ: return 25.0;
        case FXAS21002_ODR_12_5_HZ: return 12.5;
        default: return 0.0; //invalid setting passed in
        }
    }
    else return 0.0;
}

float fxos_fxas_fsr_conversion(sensor_type_t sensor, uint8_t fsr_setting)
{
    //Only the accelerometer has different options here (of which there are only three). 
    //The magnetometer FSR is fixed
    if (sensor == ACC_SENSOR)
    {
        switch (fsr_setting)
        {
        case FXOS8700_XYZ_DATA_CFG_FS_2G_0P244:
            return 0.244;
        case FXOS8700_XYZ_DATA_CFG_FS_4G_0P488:
            return 0.488;
        case FXOS8700_XYZ_DATA_CFG_FS_8G_0P976:
            return 0.976;
        default:
            return 0; //invalid setting applied
        }
    }
    else if (sensor == GYR_SENSOR)
    {
        switch (fsr_setting)
        {
        case FXAS21002_RANGE_4000DPS:
            return 125.0;
        case FXAS21002_RANGE_2000DPS:
            return 62.50;
        case FXAS21002_RANGE_1000DPS:
            return 31.25;
        case FXAS21002_RANGE_500DPS:
            return 15.625;
        case FXAS21002_RANGE_250DPS:
            return 7.8125;
        default:
            return 0; //invalid setting applied
        }
    }
    else if (sensor == MAG_SENSOR)
    {
        return 0.1; //Magnetometer full-scale is locked in at 0.1 uT/LSB
    }
}

const wchar_t* fxas_fxos_get_complete_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type)
{
    //Creates a single string out of all the settings for a single sensor setting type. Each setting
    //is separated by a '\n' character.

    //Write out the strings instead of getting them with the lsm9ds1_get_settings_string() method so that we
    //avoid duplicates.
    switch (sensor_type)
    {
    case ACC_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXOS8700 Accelerometer 0x02";
        case FS_RANGE: return L"+/- 2 g 0x00\n+/- 4 g 0x01\n+/- 8 g 0x02";
        case ODR: return L"0 Hz 0xFF\n1.5625 Hz 0x38\n6.25 Hz 0x30\n12.5 Hz 0x28\n50 Hz 0x20\n100 Hz 0x18\n200 Hz 0x10\n400 Hz 0x08\n800 Hz 0x00";
        case 100: return L"0 Hz 0xFF\n0.7813 Hz 0x38\n3.125 Hz 0x30\n6.25 Hz 0x28\n25 Hz 0x20\n50 Hz 0x18\n100 Hz 0x10\n200 Hz 0x08\n400 Hz 0x00";
        case POWER: return L"Normal 0x00\nLow Noise 0x08\nHigh Resolution 0x10\nLow Power 0x18\nOff 0xFF";
        case FILTER_SELECTION: return L"HPF Disabled 0x00\nHPF Ensabled 0x10";
        case HIGH_PASS_FILTER: return L"Cutoff Selection Disabled 0x0\nCutoff Selection Ensabled 0x1";
        default: return L"";
        }
    case GYR_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXAS21002 Gyroscope 0x02";
        case FS_RANGE: return L"+/- 250 DPS 0x03\n+/- 500 DPS 0x02\n+/- 1000 DPS 0x01\n+/- 2000 DPS 0x00\n+/- 4000 DPS 0x10";
        case ODR: return L"6.25 Hz 0x60\n12.5 Hz 0x5\n50 Hz 0x4\n100 Hz 0x3\n200 Hz 0x2\n400 Hz 0x1\n800 Hz 0x0";
        case POWER: return L"Standby Mode 0x0\nReady Mode 0x1";
        case FILTER_SELECTION: return L"LPF Only 0x0\nLPF/HPF 0x1";
        case LOW_PASS_FILTER: return L"Light 0x2\nMedium 0x1\nStrong 0x0";
        case HIGH_PASS_FILTER: return L"Light 0x3\nMedium 0x2\nStrong 0x1\nExtreme 0x0";
        default: return L"";
        }
    case MAG_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXOS8700 Magnetometer 0x02";
        case FS_RANGE: return L"+/- 1200 uT";
        case ODR: return L"0 Hz 0xFF\n1.5625 Hz 0x38\n6.25 Hz 0x30\n12.5 Hz 0x28\n50 Hz 0x20\n100 Hz 0x18\n200 Hz 0x10\n400 Hz 0x08\n800 Hz 0x00";
        case 100: return L"0 Hz 0xFF\n0.7813 Hz 0x38\n3.125 Hz 0x30\n6.25 Hz 0x28\n25 Hz 0x20\n50 Hz 0x18\n100 Hz 0x10\n200 Hz 0x08\n400 Hz 0x00";
        case POWER: return L"On 0x00\nOff 0xFF";
        default: return L"";
        }
    }
}

const wchar_t* fxas_fxos_get_settings_string(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
    //This method converts a specific sensor setting to a string that can be 
    //read by a human. Mainly used in displaying settings on a UI.

    //Shouldn't need break statements because every path of the switch returns a value
    switch (sensor_type)
    {
    case ACC_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXOS8700 Accelerometer 0x02";
        case FS_RANGE:
            switch (setting)
            {
            case FXOS8700_XYZ_DATA_CFG_FS_2G_0P244: return L"+/- 2g 0x00";
            case FXOS8700_XYZ_DATA_CFG_FS_4G_0P488: return L"+/- 4g 0x01";
            case FXOS8700_XYZ_DATA_CFG_FS_8G_0P976: return L"+/- 8g 0x02";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case FXOS8700_ODR_SINGLE_800_HZ: return L"800 Hz 0x00";
            case FXOS8700_ODR_SINGLE_400_HZ: return L"400 Hz 0x08";
            case FXOS8700_ODR_SINGLE_200_HZ: return L"200 Hz 0x10";
            case FXOS8700_ODR_SINGLE_100_HZ: return L"100 Hz 0x18";
            case FXOS8700_ODR_SINGLE_50_HZ: return L"50 Hz 0x20";
            case FXOS8700_ODR_SINGLE_12P5_HZ: return L"12.5 Hz 0x28";
            case FXOS8700_ODR_SINGLE_6P25_HZ: return L"6.25 Hz 0x30";
            case FXOS8700_ODR_SINGLE_1P5625_HZ: return L"1.5625 Hz 0x38";
            case FXOS8700_ODR_SINGLE_OFF: return L"0 Hz 0xFF";
            default: return L"";
            }
        case 100:
            //This is a special case that gets called when both the accelerometer and mag are enabled
            //and powered on. This is only used to obtain the ODR, which is cut in half when both
            //sensors are in use.
            switch (setting)
            {
            case FXOS8700_ODR_HYBRID_400_HZ: return L"400 Hz 0x00";
            case FXOS8700_ODR_HYBRID_200_HZ: return L"200 Hz 0x08";
            case FXOS8700_ODR_HYBRID_100_HZ: return L"100 Hz 0x10";
            case FXOS8700_ODR_HYBRID_50_HZ: return L"50 Hz 0x18";
            case FXOS8700_ODR_HYBRID_25_HZ: return L"25 Hz 0x20";
            case FXOS8700_ODR_HYBRID_6P25_HZ: return L"6.25 Hz 0x28";
            case FXOS8700_ODR_HYBRID_3P125_HZ: return L"3.125 Hz 0x30";
            case FXOS8700_ODR_HYBRID_0P7813_HZ: return L"0.7813 Hz 0x38";
            case FXOS8700_ODR_HYBRID_OFF: return L"0 Hz 0xFF";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case FXOS8700_ACCEL_NORMAL:  return L"Normal 0x00";
            case FXOS8700_ACCEL_LOWNOISE_LOWPOWER:  return L"Low Noise 0x08";
            case FXOS8700_ACCEL_HIGHRESOLUTION:  return L"High Resolution 0x10";
            case FXOS8700_ACCEL_LOWPOWER:  return L"Low Power 0x18";
            default: return L"Off 0xFF";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case FXOS8700_XYZ_DATA_CFG_HPF_OUT_DISABLE: return L"HPF Disabled 0x00";
            case FXOS8700_XYZ_DATA_CFG_HPF_OUT_EN: return L"HPF Enabled 0x10";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case FXOS8700_HP_FILTER_CUTOFF_SEL_DISABLE: return L"Cutoff Selection Disabled 0x0";
            case FXOS8700_HP_FILTER_CUTOFF_SEL_EN: return L"Cutoff Selection Enabled Hz 0x1";
            default: return L"";
            }
        default: return L"";
        }
    }
    case GYR_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXAS21002 Gyroscope 0x02";
        case FS_RANGE:
            switch (setting)
            {
            case FXAS21002_RANGE_4000DPS: return L"+/- 4000 DPS 0x10";
            case FXAS21002_RANGE_2000DPS: return L"+/- 2000 DPS 0x00";
            case FXAS21002_RANGE_1000DPS: return L"+/- 1000 DPS 0x01";
            case FXAS21002_RANGE_500DPS: return L"+/- 500 DPS 0x02";
            case FXAS21002_RANGE_250DPS: return L"+/- 250 DPS 0x03";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case FXAS21002_ODR_800_HZ: return L"800 Hz 0x0";
            case FXAS21002_ODR_400_HZ: return L"400 Hz 0x1";
            case FXAS21002_ODR_200_HZ: return L"200 Hz 0x2";
            case FXAS21002_ODR_100_HZ: return L"100 Hz 0x3";
            case FXAS21002_ODR_50_HZ: return L"50 Hz 0x4";
            case FXAS21002_ODR_25_HZ: return L"12.5 Hz 0x5";
            case FXAS21002_ODR_12_5_HZ: return L"6.25 Hz 0x6";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case FXAS21002_POWER_READY: return L"Ready Mode 0x1";
            case FXAS21002_POWER_STANDBY:
            default:
                return L"Standby Mode 0x0";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case FXAS21002_FILTER_LPF: return L"LPF Only 0x0";
            case FXAS21002_FILTER_LPF_HPF: return L"LPF/HPF 0x1";
            default: return L"";
            }
        case LOW_PASS_FILTER:
            switch (setting)
            {
            case FXAS21002_LPF_STRONG: return L"Strong 0x0";
            case FXAS21002_LPF_MEDIUM: return L"Medium 0x1";
            case FXAS21002_LPF_LIGHT: return L"Light 0x2";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case FXAS21002_HPF_EXTREME: return L"Extreme 0x0";
            case FXAS21002_HPF_STRONG: return L"Strong 0x1";
            case FXAS21002_HPF_MEDIUM: return L"Medium 0x2";
            case FXAS21002_HPF_LIGHT: return L"Light 0x3";
            default: return L"";
            }
        default: return L"";
        }
    }
    case MAG_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"FXOS8700 Magnetometer 0x02";
        case FS_RANGE: return L"+/- 1200 uT";
        case ODR:
            switch (setting)
            {
            case FXOS8700_ODR_SINGLE_800_HZ: return L"800 Hz 0x00";
            case FXOS8700_ODR_SINGLE_400_HZ: return L"400 Hz 0x08";
            case FXOS8700_ODR_SINGLE_200_HZ: return L"200 Hz 0x10";
            case FXOS8700_ODR_SINGLE_100_HZ: return L"100 Hz 0x18";
            case FXOS8700_ODR_SINGLE_50_HZ: return L"50 Hz 0x20";
            case FXOS8700_ODR_SINGLE_12P5_HZ: return L"12.5 Hz 0x28";
            case FXOS8700_ODR_SINGLE_6P25_HZ: return L"6.25 Hz 0x30";
            case FXOS8700_ODR_SINGLE_1P5625_HZ: return L"1.5625 Hz 0x38";
            case FXOS8700_ODR_SINGLE_OFF: return L"0Hz 0xFF";
            default: return L"";
            }
        case 100:
            //This is a special case that gets called when both the accelerometer and mag are enabled
            //and powered on. This is only used to obtain the ODR, which is cut in half when both
            //sensors are in use.
            switch (setting)
            {
            case FXOS8700_ODR_HYBRID_400_HZ: return L"400 Hz 0x00";
            case FXOS8700_ODR_HYBRID_200_HZ: return L"200 Hz 0x08";
            case FXOS8700_ODR_HYBRID_100_HZ: return L"100 Hz 0x10";
            case FXOS8700_ODR_HYBRID_50_HZ: return L"50 Hz 0x18";
            case FXOS8700_ODR_HYBRID_25_HZ: return L"25 Hz 0x20";
            case FXOS8700_ODR_HYBRID_6P25_HZ: return L"6.25 Hz 0x28";
            case FXOS8700_ODR_HYBRID_3P125_HZ: return L"3.125 Hz 0x30";
            case FXOS8700_ODR_HYBRID_0P7813_HZ: return L"0.7813 Hz 0x38";
            case FXOS8700_ODR_HYBRID_OFF: return L"0 Hz 0xFF";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case 0: return L"On 0x00";
            default: return L"Off 0xFF";
            }
        default: return L"";
        }
    }
    }
}

void fxos8700_update_acc_and_mag_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
    //The only common functionality between the fxos accelerometer and magnetometer is their odr. Not only are the ODRs the same, but when both
    //sensors are on the value of the ODR is actually cut in half. When the ODR is cut in half, the enum values actually stay the same though
    //so there isn't really anything to change in code
    int start_positions[3] = { ACC_START, 0, MAG_START };

    //first apply the new setting to the proper location
    current_settings[start_positions[sensor_type] + setting_type] = setting;

    //Calculate which sensor isn't currently changing, but might be affected by the change.
    int other_sensor = ((sensor_type - 2) % 4) * -1; //if current sensor is 2 (mag), this yields 0 (acc). If current sensors is 0 (acc) this yields 2 (mag)

    if (setting_type == POWER)
    {
        if (setting == FXOS8700_OFF)
        {
            //Set the odr of the current sensor to 0
            current_settings[start_positions[sensor_type] + ODR] = FXOS8700_ODR_SINGLE_OFF;
        }
        else
        {
            //If we're turning the sensor in question on, the ODR will need to change from 0 Hz.
            //A default value of 50 Hz is selected. If the other sensor is (acc or mag) is of 
            //FXOS type as well, and is currently on, then we need to pick hybrid 50 enum.
            if (current_settings[start_positions[sensor_type] + ODR] == FXOS8700_ODR_SINGLE_OFF)
            {
                if (current_settings[start_positions[other_sensor] + SENSOR_MODEL] == FXOS8700_ACC) //FXOS8700_ACC and FXOS8700_MAG enums have the same value
                {
                    if (current_settings[start_positions[other_sensor] + POWER] != FXOS8700_ODR_SINGLE_OFF)
                    {
                        //Put both sensors intyo hybrid 50Hz mode
                        current_settings[start_positions[sensor_type] + ODR] = FXOS8700_ODR_HYBRID_50_HZ;
                        current_settings[start_positions[other_sensor] + ODR] = FXOS8700_ODR_HYBRID_50_HZ;
                    }
                    else current_settings[start_positions[sensor_type] + ODR] = FXOS8700_ODR_SINGLE_50_HZ; //only one sensor is on/present so put it into single 50 Hz mode
                }
            }
        }
    }
    else if (setting_type == ODR)
    {
        if (setting == FXOS8700_ODR_SINGLE_OFF)
        {
            //Set the power of the current sensor to off
            current_settings[start_positions[sensor_type] + POWER] = FXOS8700_OFF;
        }
        else
        {
            //We aren't setting the odr of the current sensor to 0, so we need to see if the 
            //other sensor (acc or mag) is also a FXOS type and is turned on. If so, it's odr
            //will need to match the new odr of the current sensor
            if (current_settings[start_positions[other_sensor] + SENSOR_MODEL] == FXOS8700_ACC) //FXOS8700_ACC and FXOS8700_MAG enums have the same value
            {
                if (current_settings[start_positions[other_sensor] + POWER] != FXOS8700_ODR_SINGLE_OFF)
                {
                    //The other sensor is of FXOS type and is on, so change its ODR
                    current_settings[start_positions[other_sensor] + ODR] = setting;
                }
            }
        }
    }
}

void fxos8700_update_acc_or_mag_setting(uint8_t* current_settings, sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
    int start_positions[3] = { ACC_START, 0, MAG_START };

    //first apply the new setting to the proper location
    current_settings[start_positions[sensor_type] + setting_type] = setting;

    if (setting_type == POWER)
    {
        if (setting == FXOS8700_OFF)
        {
            //Set the odr of the current sensor to 0
            current_settings[start_positions[sensor_type] + ODR] = FXOS8700_ODR_SINGLE_OFF;
        }
        else
        {
            //If we're turning the sensor in question on, the ODR will need to change from 0 Hz.
            //A default value of 50 Hz is selected.
            if (current_settings[start_positions[sensor_type] + ODR] == FXOS8700_ODR_SINGLE_OFF)
            {
                current_settings[start_positions[sensor_type] + ODR] = FXOS8700_ODR_SINGLE_50_HZ;
            }
        }
    }
    else if (setting_type == ODR)
    {
        if (setting == FXOS8700_ODR_SINGLE_OFF)
        {
            //Set the power of the current sensor to off
            current_settings[start_positions[sensor_type] + POWER] = FXOS8700_OFF;
        }
        else
        {
            //If the sensor is currently off, turn it on
            if (current_settings[start_positions[sensor_type] + POWER] == FXOS8700_OFF) //FXOS8700_ACC and FXOS8700_MAG enums have the same value
            {
                current_settings[start_positions[sensor_type] + POWER] = 0;
            }
        }
    }
}