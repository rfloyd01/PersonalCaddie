#include "fxas21002.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;

void fxas21002init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip. The FXOS driver comes with their
    //own communication struct which we must utilize to communicate with the
    //chip. If both the FXOS acc and mag ar in use then the built-in communication
    //struct will default to the magnetometer.
    imu_comm = comm;

    //initialize read/write methods, address, and default settings for acc
    if (imu_comm->sensor_model[GYR_SENSOR] == FXAS21002_GYR)
    {
        //Apply default settings for the acc
        update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, FXAS21002_RANGE_2000DPS); //gyroscope full scale range (+/- 2000 dps)

        update_sensor_setting(p_sensor_settings + GYR_START, ODR, FXAS21002_ODR_50_HZ); //gyroscope ODR (50 Hz)

        update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, FXAS21002_FILTER_LPF_HPF); //gyroscope filter selection (low and high pass filter active)
        update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, FXAS21002_LPF_STRONG); //gyroscope low pass filter frequency (16Hz at 50Hz odr)
        update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, FXAS21002_HPF_STRONG); //gyroscope high pass filter frequency (0.481 Hz at 50Hz odr)

        //After setting default settings, attempt to read the whoAmI register
        uint8_t whoamI;
        imu_comm->gyr_comm.write_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_WHOAMI, &whoamI);
        if (whoamI == FXAS21002_WHO_AM_I) SEGGER_RTT_WriteString(0, "FXAS21002 Gyr discovered.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't find FXAS21002 Gyr.\n");
    }
}

int32_t fxas21002_idle_mode_enable()
{
    //The fxas21002 has two idle modes, there's a standby mode which is very low power and
    //only the i2c bus is active, and there's a ready mode where the gyro is on but in a state
    //of low power. We opt to go into ready mode here so the gyro remains "warmed up"
    return fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_READY);
}

int32_t fxas21002_active_mode_enable()
{
    //We first apply the ODR setting, as if this will put the chip in standby/ready mode so
    //we can safely alter the settings. The last setting we update is the high-pass filter settings
    //because changing the ODR or the power mode causes these settings to reset.
    int32_t ret = 0;
    ret |= fxas21002_data_rate_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + ODR]);
    ret |= fxas21002_full_scale_range_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + FS_RANGE]);
    ret |= fxas21002_lp_filter_bw_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + LOW_PASS_FILTER]);

    //Alter the hp filter settings last so they don't get overwritten
    ret |= fxas21002_filter_out_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + FILTER_SELECTION]);
    ret |= fxas21002_hp_filter_bw_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + HIGH_PASS_FILTER]);

    //Finally we put the sensor into active mode
    ret |= fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_ACTIVE);

    if (ret != 0) SEGGER_RTT_WriteString(0, "Error: FXAS21002 enabled with incorrect settings.\n");

    return ret;
}

int32_t fxas21002_get_gyr_data(uint8_t* pBuff, uint8_t offset)
{
    //Data for the x, y and z axes are read in a single burst and put directly
    //into the buffer with the given offset. The data is in little endian by
    //default so if we want Big-Endian then we need to swap bytes manually
    uint8_t ret = imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_OUTXMSB, pBuff + offset, 6);

    //If we want big-endian data then we need to swap bytes
    if (__BYTE_ORDER__ != 1234)
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            uint8_t temp = pBuff[2 * i + offset];
            pBuff[2 * i + offset] = pBuff[2 * i + 1 + offset];
            pBuff[2 * i + 1 + offset] = temp;
        }
    }

    return ret;
}