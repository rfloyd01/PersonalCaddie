#include "fxas21002.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"
#include "personal_caddie_operating_modes.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;

void fxas21002init(imu_communication_t* comm, uint8_t* settings)
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
        update_sensor_setting(p_sensor_settings + GYR_START, POWER,  FXAS21002_POWER_READY); //gyroscope ODR (50 Hz)

        update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, FXAS21002_FILTER_LPF); //gyroscope filter selection (low pass filter active)
        update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, FXAS21002_LPF_STRONG); //gyroscope low pass filter frequency (16Hz at 50Hz odr)
        update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, FXAS21002_HPF_STRONG); //gyroscope high pass filter frequency (0.481 Hz at 50Hz odr)

        //After setting default settings, attempt to read the whoAmI register
        uint8_t whoamI;
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_WHOAMI, &whoamI, 1);
        if (whoamI == FXAS21002_WHO_AM_I) SEGGER_RTT_WriteString(0, "FXAS21002 Gyr initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize FXAS21002 Gyr.\n");
    }
}

int32_t fxas21002_connected_mode_enable()
{
    //In connected mode the FXAS21002 will be placed into its "Standby" power mode which has
    //an expected current draw of 2.8 uA (0.0000028 A). In this mode the registers of the 
    //sensor can be read/written with the TWI interface but that's about it. The sensor enters
    //this mode by default when it first gets powered on so there's no need to do anything the
    //first time connected mode is entered.
    if (imu_comm->sensor_model[GYR_SENSOR] != FXAS21002_GYR) return 0; //only carry out this method if an FXAS sensor is active

    uint8_t ret = fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_STANDBY); //First put the chip in standby mode
    if (ret != 0) SEGGER_RTT_WriteString(0, "Error: Couldn't place FXAS21002 into connected mode.\n");

    return ret;
}

int32_t fxas21002_idle_mode_enable()
{
    //In sensor idle mode the FXAS21002 will be placed into its "Ready" power mode which has
    //an expected current draw of 1.6 mA (0.0016 A). In this mode we can't actively take 
    //gyroscope readings, however, the time to transition to active mode is much less than if
    //we went from standby mode.
    if (imu_comm->sensor_model[GYR_SENSOR] != FXAS21002_GYR) return 0;

    uint8_t ret = fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_READY);
    if (ret != 0) SEGGER_RTT_WriteString(0, "Error: Couldn't place FXAS21002 into sensor idle mode.\n");

    return ret;
}

int32_t fxas21002_active_mode_enable(int current_mode)
{
    //In sensor active mode the FXAS21002 is placed into its "Active" power mode. The expected
    //current draw in this mode is 2.7 mA (0.0027 A). This mode is used to actively take
    //measurements of the gyroscope. To ensure that the gyroscope has enough time to ramp up
    //to full power we need to delay for a little bit after turning it on. If active mode is
    //reached from connected mode the time required for this is 61 milliseconds and if active
    //mode is reached from idle mode the time required for this is only 6 milliseconds.
    
    if (imu_comm->sensor_model[GYR_SENSOR] != FXAS21002_GYR) return 0;

    //We first apply the ODR setting as this will put the chip in standby/ready mode so
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

    //After all settings have been applied and the sensor is turned on, wait for the 
    //necessary amount of time to ensure proper readings
    if (current_mode = CONNECTED_MODE) imu_comm->gyr_comm.delay(FXAS21002_STANDBY_TO_ACTIVE_DELAY_US);
    else if (current_mode = SENSOR_IDLE_MODE) imu_comm->gyr_comm.delay(FXAS21002_READY_TO_ACTIVE_DELAY_US);

    return ret;
}

void fxas21002_get_actual_settings()
{
    //For debugging purposes it's nice to see that the settings we have stored in the sensor array
    //physically make their way onto the chip. This method prints out the current register values
    //of some of the more important registers
    uint8_t reg_val;

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[GYR_SENSOR] == FXAS21002_GYR)
    {
        SEGGER_RTT_WriteString(0, "FXAS21002 Gyr. Register Values:\n");
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG0, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG0 Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG1, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG1 Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG2, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG2 Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG3, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG3 Register: 0x%x\n\n", reg_val);
    }
}

int32_t fxas21002_get_gyr_data(uint8_t* pBuff, uint8_t offset)
{
    //Data for the x, y and z axes are read in a single burst and put directly
    //into the buffer with the given offset. The data is in little endian by
    //default so if we want Big-Endian then we need to swap bytes manually
    uint8_t ret = imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_OUTXMSB, pBuff + offset, 6);

    //Unlike the FXOS sensors, the FXAS gives data in big-endian so it must
    //be manually swapped to little endian
    if (__BYTE_ORDER__ == 1234)
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