#include "bmi270_drv.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;

static struct bmi2_dev    bmi270; //driver defined struct for holding functional pointers and other info

void bmi270init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip. The FXOS driver comes with their
    //own communication struct which we must utilize to communicate with the
    //chip. If both the FXOS acc and mag ar in use then the built-in communication
    //struct will default to the magnetometer.
    imu_comm = comm;

    //initialize functional pointers in the driver defined struct for things like
    //reading and writing
    bmi270.intf = BMI2_I2C_INTF; //set the I2C interface
    bmi270.read = bmi270_read_register; //set the I2C read functional pointer
    bmi270.write = bmi270_write_register; //set the I2C write functional pointer
    bmi270.delay_us = bmi270_delay; //set the microsecond delay functional pointer

    int8_t rslt;
    rslt = bmi270_init(&bmi270);

    if (rslt == BMI2_OK) SEGGER_RTT_WriteString(0, "BMI270 initialized.\n");
    

    //initialize read/write methods, address, and default settings for acc
    //if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    //{
    //    update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, FXAS21002_RANGE_2000DPS); //gyroscope full scale range (+/- 2000 dps)

    //    update_sensor_setting(p_sensor_settings + GYR_START, ODR, FXAS21002_ODR_50_HZ); //gyroscope ODR (50 Hz)
    //    update_sensor_setting(p_sensor_settings + GYR_START, POWER,  FXAS21002_POWER_READY); //gyroscope ODR (50 Hz)

    //    update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, FXAS21002_FILTER_LPF); //gyroscope filter selection (low pass filter active)
    //    update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, FXAS21002_LPF_STRONG); //gyroscope low pass filter frequency (16Hz at 50Hz odr)
    //    update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, FXAS21002_HPF_STRONG); //gyroscope high pass filter frequency (0.481 Hz at 50Hz odr)
    //}

    ////initialize read/write methods, address, and default settings for gyro
    //if (imu_comm->sensor_model[GYR_SENSOR] == FXAS21002_GYR)
    //{
    //    //Apply default settings for the acc
    //    update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, FXAS21002_RANGE_2000DPS); //gyroscope full scale range (+/- 2000 dps)

    //    update_sensor_setting(p_sensor_settings + GYR_START, ODR, FXAS21002_ODR_50_HZ); //gyroscope ODR (50 Hz)
    //    update_sensor_setting(p_sensor_settings + GYR_START, POWER,  FXAS21002_POWER_READY); //gyroscope ODR (50 Hz)

    //    update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, FXAS21002_FILTER_LPF); //gyroscope filter selection (low pass filter active)
    //    update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, FXAS21002_LPF_STRONG); //gyroscope low pass filter frequency (16Hz at 50Hz odr)
    //    update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, FXAS21002_HPF_STRONG); //gyroscope high pass filter frequency (0.481 Hz at 50Hz odr)

    //    //After setting default settings, attempt to read the whoAmI register
    //    uint8_t whoamI;
    //    imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_WHOAMI, &whoamI, 1);
    //    if (whoamI == FXAS21002_WHO_AM_I) SEGGER_RTT_WriteString(0, "FXAS21002 Gyr initialized.\n");
    //    else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize FXAS21002 Gyr.\n");
    //}
}

int32_t bmi270_idle_mode_enable()
{
    //The fxas21002 has two idle modes, there's a standby mode which is very low power and
    //only the i2c bus is active, and there's a ready mode where the gyro is on but in a state
    //of low power. We opt to go into ready mode here so the gyro remains "warmed up"
    //if (imu_comm->sensor_model[GYR_SENSOR] != FXAS21002_GYR) return 0;
    //return fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_READY);
    return 0;
}

int32_t bmi270_active_mode_enable()
{
    //We first apply the ODR setting, as if this will put the chip in standby/ready mode so
    //we can safely alter the settings. The last setting we update is the high-pass filter settings
    //because changing the ODR or the power mode causes these settings to reset.
    if (imu_comm->sensor_model[GYR_SENSOR] != FXAS21002_GYR) return 0;

    int32_t ret = 0;
    //ret |= fxas21002_data_rate_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + ODR]);
    //ret |= fxas21002_full_scale_range_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + FS_RANGE]);
    //ret |= fxas21002_lp_filter_bw_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + LOW_PASS_FILTER]);

    ////Alter the hp filter settings last so they don't get overwritten
    //ret |= fxas21002_filter_out_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + FILTER_SELECTION]);
    //ret |= fxas21002_hp_filter_bw_set(&imu_comm->gyr_comm, p_sensor_settings[GYR_START + HIGH_PASS_FILTER]);

    ////Finally we put the sensor into active mode
    //ret |= fxas21002_power_mode_set(&imu_comm->gyr_comm, FXAS21002_POWER_ACTIVE);

    //if (ret != 0) SEGGER_RTT_WriteString(0, "Error: FXAS21002 enabled with incorrect settings.\n");

    return ret;
}

void bmi270_get_actual_settings()
{
    //For debugging purposes it's nice to see that the settings we have stored in the sensor array
    //physically make their way onto the chip. This method prints out the current register values
    //of some of the more important registers
    uint8_t reg_val;

    //initialize read/write methods, address, and default settings for mag
    //if (imu_comm->sensor_model[GYR_SENSOR] == FXAS21002_GYR)
    //{
    //    SEGGER_RTT_WriteString(0, "FXAS21002 Gyr. Register Values:\n");
    //    imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG0, &reg_val, 1);
    //    SEGGER_RTT_printf(0, "CTRL_REG0 Register: 0x%x\n", reg_val);
    //    imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG1, &reg_val, 1);
    //    SEGGER_RTT_printf(0, "CTRL_REG1 Register: 0x%x\n", reg_val);
    //    imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG2, &reg_val, 1);
    //    SEGGER_RTT_printf(0, "CTRL_REG2 Register: 0x%x\n", reg_val);
    //    imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, FXAS21002_REG_CTRL_REG3, &reg_val, 1);
    //    SEGGER_RTT_printf(0, "CTRL_REG3 Register: 0x%x\n\n", reg_val);
    //}
}

int8_t bmi270_read_register(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C read into the form required
    //by Bosch's drivers
    return (int8_t) imu_comm->acc_comm.read_register(intf_ptr, BMI2_I2C_PRIM_ADDR, reg_addr, reg_data, (uint16_t)len);
}

int8_t bmi270_write_register(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C write into the form required
    //by Bosch's drivers. The Bosch Driver requires a length parameter, however, my
    //method only allows for writing 1 byte at a time so I don't use the len parameter.
    return (int8_t) imu_comm->acc_comm.write_register(intf_ptr, BMI2_I2C_PRIM_ADDR, reg_addr, reg_data);
}

void bmi270_delay(uint32_t period, void *intf_ptr)
{
    //The Bosch drivers sometimes force delays so they need a method that can implement
    //this. Since the nRF chip has the BLE stack initialized I can't take advantage of
    //the simple nrf_delay() method and instead need to use the nrf_drv_timer library
    //which is a little more complex (although has a resolution of 62.5 nanoseconds).
    imu_comm->acc_comm.delay(period);
}

int32_t bmi270_get_acc_data(uint8_t* pBuff, uint8_t offset)
{
    //Data for the x, y and z axes are read in a single burst and put directly
    //into the buffer with the given offset. The data is in little endian by
    //default so if we want Big-Endian then we need to swap bytes manually
    //uint8_t ret = imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_OUTXMSB, pBuff + offset, 6);

    ////Unlike the FXOS sensors, the FXAS gives data in big-endian so it must
    ////be manually swapped to little endian
    //if (__BYTE_ORDER__ == 1234)
    //{
    //    for (uint8_t i = 0; i < 3; i++)
    //    {
    //        uint8_t temp = pBuff[2 * i + offset];
    //        pBuff[2 * i + offset] = pBuff[2 * i + 1 + offset];
    //        pBuff[2 * i + 1 + offset] = temp;
    //    }
    //}

    //return ret;
    return 0;
}

int32_t bmi270_get_gyr_data(uint8_t* pBuff, uint8_t offset)
{
    //Data for the x, y and z axes are read in a single burst and put directly
    //into the buffer with the given offset. The data is in little endian by
    //default so if we want Big-Endian then we need to swap bytes manually
    //uint8_t ret = imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, FXAS21002_REG_OUTXMSB, pBuff + offset, 6);

    ////Unlike the FXOS sensors, the FXAS gives data in big-endian so it must
    ////be manually swapped to little endian
    //if (__BYTE_ORDER__ == 1234)
    //{
    //    for (uint8_t i = 0; i < 3; i++)
    //    {
    //        uint8_t temp = pBuff[2 * i + offset];
    //        pBuff[2 * i + offset] = pBuff[2 * i + 1 + offset];
    //        pBuff[2 * i + 1 + offset] = temp;
    //    }
    //}

    //return ret;
    return 0;
}