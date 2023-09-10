#include "sensor_settings.h"
#include "lsm9ds1_reg.h"

#include <stdio.h>
#include <string.h>

void update_sensor_setting(uint8_t* settings_array, sensor_settings_t setting, uint8_t value)
{
    //This method basically just makes sure that the setting passed in is valid and if it is,
    //the given array is updated.
    if (setting >= 0 && setting <= EXTRA_2) settings_array[setting] = value;
}

//LSM9DS1 conversions
float lsm9ds1_odr_calculate(uint8_t imu_odr_setting, uint8_t mag_odr_setting)
{
    //Returns the current ODR of the sensor as indicated in the sensor settings array.
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
    switch (sensor_type)
    {
    case ACC_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Accelerometer";
        case FS_RANGE: return L"+/- 2 g\n+/- 4 g\n+/- 8 g\n+/- 16 g";
        case ODR: return L"0 Hz\n10 Hz\n14.9 Hz\n50 Hz\n59.5 Hz\n119 Hz\n238 Hz\n476 Hz\n952 Hz";
        case POWER: return L"Off\nOn";
        case FILTER_SELECTION: return L"LPF Enabled\nHPF Enabled";
        case LOW_PASS_FILTER: return L"0 Hz\nODR/50 Hz\nODR/100 Hz\nODR/9 Hz\nODR/400 Hz";
        case HIGH_PASS_FILTER: return L"ODR/50 Hz\nODR/100 Hz\nODR/9 Hz\nODR/400 Hz";
        case EXTRA_FILTER: return L"Auto\n50 Hz\n105 Hz\n211 Hz\n408 Hz";
        default: return L"";
        }
    case GYR_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Gyroscope";
        case FS_RANGE: return L"+/- 245 DPS\n+/- 500 DPS\n+/- 2000 DPS";
        case ODR: return L"0 Hz\n14.9 Hz\n59.5 Hz\n119 Hz\n238 Hz\n476 Hz\n952 Hz";
        case POWER: return L"Off\nLow Power\nNormal Mode";
        case FILTER_SELECTION: return L"LPF1 Only\nLPF1/HPF\nLPF1/LPF2\nLPF1/HPF/LPF2";
        case LOW_PASS_FILTER: return L"Ultra Light\nLight\nMedium\nStrong";
        case HIGH_PASS_FILTER: return L"Ultra Light\nLight\nUltra Low\nLow\nMedium\nHigh\nUltra High\nStrong\nUltra Strong\nExtreme";
        default: return L"";
        }
    case MAG_SENSOR:
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Magnetometer";
        case FS_RANGE: return L"+/- 4 Ga\n+/- 8 Ga\n+/- 12 Ga\n+/- 16 Ga";
        case ODR: return L"0 Hz\n0.625 Hz\n1.25 Hz\n2.5 Hz\n5 Hz\n10 Hz\n20 Hz\n40 Hz\n80 Hz\n155 Hz\n300 Hz\n560 Hz\n1000 Hz";
        case POWER: return L"Off\nLow Power\nMedium Power\nHigh Power\nUltra High Power";
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
        case SENSOR_MODEL: return L"LSM9DS1 Accelerometer";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_2g: return L"+/- 2g";
            case LSM9DS1_4g: return L"+/- 4g";
            case LSM9DS1_8g: return L"+/- 8g";
            case LSM9DS1_16g: return L"+/- 16g";
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
                        return L"0 Hz";
            case LSM9DS1_GY_OFF_XL_10Hz: return L"10 Hz";
            case LSM9DS1_IMU_14Hz9:
            case LSM9DS1_IMU_14Hz9_LP:
                return L"14.9 Hz";
            case LSM9DS1_GY_OFF_XL_50Hz: return L"50 Hz";
            case LSM9DS1_IMU_59Hz5:
            case LSM9DS1_IMU_59Hz5_LP:
                return L"59.5 Hz";
            case LSM9DS1_IMU_119Hz:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_GY_OFF_XL_119Hz:
                return L"119 Hz";
            case LSM9DS1_IMU_238Hz:
            case LSM9DS1_GY_OFF_XL_238Hz:
                return L"238 Hz";
            case LSM9DS1_IMU_476Hz:
            case LSM9DS1_GY_OFF_XL_476Hz:
                return L"476 Hz";
            case LSM9DS1_IMU_952Hz:
            case LSM9DS1_GY_OFF_XL_952Hz:
                return L"952 Hz";
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
                return L"Off";
            default: return L"On";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case LSM9DS1_LP_OUT: return L"LPF Enabled";
            case LSM9DS1_HP_OUT: return L"HPF Enabled";
            default: return L"";
            }
        case LOW_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_LP_DISABLE: return L"0 Hz";
            case LSM9DS1_LP_ODR_DIV_50: return L"ODR/50 Hz";
            case LSM9DS1_LP_ODR_DIV_100: return L"ODR/100 Hz";
            case LSM9DS1_LP_ODR_DIV_9: return L"ODR/9 Hz";
            case LSM9DS1_LP_ODR_DIV_400: return L"ODR/400 Hz";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_HP_ODR_DIV_50: return L"ODR/50 Hz";
            case LSM9DS1_HP_ODR_DIV_100: return L"ODR/100 Hz";
            case LSM9DS1_HP_ODR_DIV_9: return L"ODR/9 Hz";
            case LSM9DS1_HP_ODR_DIV_400: return L"ODR/400 Hz";
            default: return L"";
            }
        case EXTRA_FILTER:
            switch (setting)
            {
            case LSM9DS1_AUTO: return L"Auto";
            case LSM9DS1_408Hz: return L"408 Hz";
            case LSM9DS1_211Hz: return L"211 Hz";
            case LSM9DS1_105Hz: return L"105 Hz";
            case LSM9DS1_50Hz: return L"50 Hz";
            default: return L"";
            }
        default: return L"";
        }
    }
    case GYR_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Gyroscope";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_245dps: return L"+/- 245 DPS";
            case LSM9DS1_500dps: return L"+/- 500 DPS";
            case LSM9DS1_2000dps: return L"+/- 2000 DPS";
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
                return L"0 Hz";
            case LSM9DS1_IMU_14Hz9:
            case LSM9DS1_IMU_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_14Hz9:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
                return L"14.9 Hz";
            case LSM9DS1_IMU_59Hz5:
            case LSM9DS1_IMU_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
                return L"59.5 Hz";
            case LSM9DS1_IMU_119Hz:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_XL_OFF_GY_119Hz:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
                return L"119 Hz";
            case LSM9DS1_IMU_238Hz:
            case LSM9DS1_XL_OFF_GY_238Hz:
                return L"238 Hz";
            case LSM9DS1_IMU_476Hz:
            case LSM9DS1_XL_OFF_GY_476Hz:
                return L"476 Hz";
            case LSM9DS1_IMU_952Hz:
            case LSM9DS1_XL_OFF_GY_952Hz:
                return L"952 Hz";
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
                return L"Off";
            case LSM9DS1_IMU_14Hz9_LP:
            case LSM9DS1_XL_OFF_GY_14Hz9_LP:
            case LSM9DS1_IMU_59Hz5_LP:
            case LSM9DS1_XL_OFF_GY_59Hz5_LP:
            case LSM9DS1_IMU_119Hz_LP:
            case LSM9DS1_XL_OFF_GY_119Hz_LP:
            default: return L"On";
            }
        case FILTER_SELECTION:
            switch (setting)
            {
            case LSM9DS1_LPF1_OUT: return L"LPF1 Only";
            case LSM9DS1_LPF1_HPF_OUT: return L"LPF1/HPF";
            case LSM9DS1_LPF1_LPF2_OUT: return L"LPF1/LPF2";
            case LSM9DS1_LPF1_HPF_LPF2_OUT: return L"LPF1/HPF/LPF2";
            default: return L"";
            }
        case LOW_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_LP_STRONG: return L"Strong";
            case LSM9DS1_LP_MEDIUM: return L"Medium";
            case LSM9DS1_LP_LIGHT: return L"Light";
            case LSM9DS1_LP_ULTRA_LIGHT: return L"Ultra Light";
            default: return L"";
            }
        case HIGH_PASS_FILTER:
            switch (setting)
            {
            case LSM9DS1_HP_ODR_DIV_50: return L"ODR/50 Hz";
            case LSM9DS1_HP_ODR_DIV_100: return L"ODR/100 Hz";
            case LSM9DS1_HP_ODR_DIV_9: return L"ODR/9 Hz";
            case LSM9DS1_HP_ODR_DIV_400: return L"ODR/400 Hz";
            default: return L"";
            }
        case EXTRA_FILTER:
            switch (setting)
            {
            case LSM9DS1_HP_EXTREME: return L"Extreme";
            case LSM9DS1_HP_ULTRA_STRONG: return L"Ultra Strong";
            case LSM9DS1_HP_STRONG: return L"Strong";
            case LSM9DS1_HP_ULTRA_HIGH: return L"Ultra High";
            case LSM9DS1_HP_HIGH: return L"High";
            case LSM9DS1_HP_MEDIUM: return L"Medium";
            case LSM9DS1_HP_LOW: return L"Low";
            case LSM9DS1_HP_ULTRA_LOW: return L"Ultra Low";
            case LSM9DS1_HP_LIGHT: return L"Light";
            case LSM9DS1_HP_ULTRA_LIGHT: return L"Ultra Light";
            default: return L"";
            }
        default: return L"";
        }
    }
    case MAG_SENSOR:
    {
        switch (setting_type)
        {
        case SENSOR_MODEL: return L"LSM9DS1 Magnetometer";
        case FS_RANGE:
            switch (setting)
            {
            case LSM9DS1_4Ga: return L"+/- 4 Ga";
            case LSM9DS1_8Ga: return L"+/- 8 Ga";
            case LSM9DS1_12Ga: return L"+/- 12 Ga";
            case LSM9DS1_16Ga: return L"+/- 16 Ga";
            default: return L"";
            }
        case ODR:
            switch (setting)
            {
            case LSM9DS1_MAG_POWER_DOWN: return L"0 Hz";
            case LSM9DS1_MAG_LP_0Hz625:
            case LSM9DS1_MAG_MP_0Hz625:
            case LSM9DS1_MAG_HP_0Hz625:
            case LSM9DS1_MAG_UHP_0Hz625:
                return L"0.625 Hz";
            case LSM9DS1_MAG_LP_1Hz25:
            case LSM9DS1_MAG_MP_1Hz25:
            case LSM9DS1_MAG_HP_1Hz25:
            case LSM9DS1_MAG_UHP_1Hz25:
                return L"1.25 Hz";
            case LSM9DS1_MAG_LP_2Hz5:
            case LSM9DS1_MAG_MP_2Hz5:
            case LSM9DS1_MAG_HP_2Hz5:
            case LSM9DS1_MAG_UHP_2Hz5:
                return L"2.5 Hz";
            case LSM9DS1_MAG_LP_5Hz:
            case LSM9DS1_MAG_MP_5Hz:
            case LSM9DS1_MAG_HP_5Hz:
            case LSM9DS1_MAG_UHP_5Hz:
                return L"5 Hz";
            case LSM9DS1_MAG_LP_10Hz:
            case LSM9DS1_MAG_MP_10Hz:
            case LSM9DS1_MAG_HP_10Hz:
            case LSM9DS1_MAG_UHP_10Hz:
                return L"10 Hz";
            case LSM9DS1_MAG_LP_20Hz:
            case LSM9DS1_MAG_MP_20Hz:
            case LSM9DS1_MAG_HP_20Hz:
            case LSM9DS1_MAG_UHP_20Hz:
                return L"20 Hz";
            case LSM9DS1_MAG_LP_40Hz:
            case LSM9DS1_MAG_MP_40Hz:
            case LSM9DS1_MAG_HP_40Hz:
            case LSM9DS1_MAG_UHP_40Hz:
                return L"40 Hz";
            case LSM9DS1_MAG_LP_80Hz:
            case LSM9DS1_MAG_MP_80Hz:
            case LSM9DS1_MAG_HP_80Hz:
            case LSM9DS1_MAG_UHP_80Hz:
                return L"80 Hz";
            case LSM9DS1_MAG_UHP_155Hz: return L"115 Hz";
            case LSM9DS1_MAG_HP_300Hz: return L"300 Hz";
            case LSM9DS1_MAG_MP_560Hz: return L"560 Hz";
            case LSM9DS1_MAG_LP_1000Hz: return L"1000 Hz";
            default: return L"";
            }
        case POWER:
            switch (setting)
            {
            case LSM9DS1_MAG_POWER_DOWN: return L"Off";
            case LSM9DS1_MAG_LP_0Hz625:
            case LSM9DS1_MAG_LP_1Hz25:
            case LSM9DS1_MAG_LP_2Hz5:
            case LSM9DS1_MAG_LP_5Hz:
            case LSM9DS1_MAG_LP_10Hz:
            case LSM9DS1_MAG_LP_20Hz:
            case LSM9DS1_MAG_LP_40Hz:
            case LSM9DS1_MAG_LP_80Hz:
            case LSM9DS1_MAG_LP_1000Hz:
                return L"Low Power";
            case LSM9DS1_MAG_MP_0Hz625:
            case LSM9DS1_MAG_MP_1Hz25:
            case LSM9DS1_MAG_MP_2Hz5:
            case LSM9DS1_MAG_MP_5Hz:
            case LSM9DS1_MAG_MP_10Hz:
            case LSM9DS1_MAG_MP_20Hz:
            case LSM9DS1_MAG_MP_40Hz:
            case LSM9DS1_MAG_MP_80Hz:
            case LSM9DS1_MAG_MP_560Hz:
                return L"Medium Power";
            case LSM9DS1_MAG_HP_0Hz625:
            case LSM9DS1_MAG_HP_1Hz25:
            case LSM9DS1_MAG_HP_2Hz5:
            case LSM9DS1_MAG_HP_5Hz:
            case LSM9DS1_MAG_HP_10Hz:
            case LSM9DS1_MAG_HP_20Hz:
            case LSM9DS1_MAG_HP_40Hz:
            case LSM9DS1_MAG_HP_80Hz:
            case LSM9DS1_MAG_HP_300Hz:
                return L"High Power";
            case LSM9DS1_MAG_UHP_0Hz625:
            case LSM9DS1_MAG_UHP_1Hz25:
            case LSM9DS1_MAG_UHP_2Hz5:
            case LSM9DS1_MAG_UHP_5Hz:
            case LSM9DS1_MAG_UHP_10Hz:
            case LSM9DS1_MAG_UHP_20Hz:
            case LSM9DS1_MAG_UHP_40Hz:
            case LSM9DS1_MAG_UHP_80Hz:
            case LSM9DS1_MAG_UHP_155Hz:
                return L"Ultra High Power";
            default: return L"Off";
            }
        default: return L"";
        }
    }
    }
}