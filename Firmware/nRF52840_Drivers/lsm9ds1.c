#include "lsm9ds1.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

//set up pointers to TWI instance for chip communication
const nrf_drv_twi_t* p_twi;
volatile bool* p_xfer_done;

//set up settings variables
static uint8_t* p_sensor_settings;

static bool lsm9ds1_register_auto_increment = true;                       /**register auto increment function for multiple byte reads of LSM9DS1 chip **/

void lsm9ds1_init(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, uint8_t* settings, 
                  const nrf_drv_twi_t* twi, volatile bool* xfer_done)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip
    p_xfer_done = xfer_done;
    p_twi = twi;

    //initialize read/write methods for acc./gyro.
    lsm9ds1_imu->read_reg = lsm9ds1_read_imu;
    lsm9ds1_imu->write_reg = lsm9ds1_write_imu;

    //initialize read/write methods for mag.
    lsm9ds1_mag->read_reg = lsm9ds1_read_mag;
    lsm9ds1_mag->write_reg = lsm9ds1_write_mag;

    //update the sensor models in the settings array
    update_sensor_setting(p_sensor_settings + ACC_START, SENSOR_MODEL, LSM9DS1_ACC);
    update_sensor_setting(p_sensor_settings + GYR_START, SENSOR_MODEL, LSM9DS1_GYR);
    update_sensor_setting(p_sensor_settings + MAG_START, SENSOR_MODEL, LSM9DS1_MAG);

    //Populate the sensor settings array with default settings for the sensor
    update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, LSM9DS1_4g); //accelerometer full scale range (+/- 4 g)
    update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, LSM9DS1_2000dps); //gyroscope full scale range (+/- 2000 degrees/s)
    update_sensor_setting(p_sensor_settings + MAG_START, FS_RANGE, LSM9DS1_4Ga); //magnetometer full scale range (+/- 4 Gauss)

    //in the lsm9ds1, ODR and power settings are combined, furthermore, acc and gyro odr and power are combined
    //into a singly byte. For this reason, the acc_odr, gyr_odr, acc_power and gyr_power all get the same values,
    //and mag_odr + mag_power get the same values
    update_sensor_setting(p_sensor_settings + ACC_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
    update_sensor_setting(p_sensor_settings + GYR_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
    update_sensor_setting(p_sensor_settings + ACC_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
    update_sensor_setting(p_sensor_settings + GYR_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
    update_sensor_setting(p_sensor_settings + MAG_START, ODR, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)
    update_sensor_setting(p_sensor_settings + MAG_START, POWER, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)

    update_sensor_setting(p_sensor_settings + ACC_START, FILTER_SELECTION, LSM9DS1_LP_OUT); //accelerometer filter selection (low pass filter only)
    update_sensor_setting(p_sensor_settings + ACC_START, LOW_PASS_FILTER, LSM9DS1_LP_DISABLE); //accelerometer low pass filter setting (frequency is automatically tied to ODR)
    update_sensor_setting(p_sensor_settings + ACC_START, HIGH_PASS_FILTER, 0); //accelerometer high pass filter setting (only takes effect when HP_OUT is set for accelerometer)
    update_sensor_setting(p_sensor_settings + ACC_START, EXTRA_FILTER, LSM9DS1_AUTO); //accelerometer anti-aliasing bandwidth (automatically set based on current ODR)

    update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, LSM9DS1_LPF1_OUT); //Gyroscope filter selection (low pass filter 1 only)
    update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, 0); //gyroscope low pass filter setting (only takes effect when LPF2 is set in gyro filter path)
    update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, 0); //gyroscope high pass filter setting (only takes effect when HPF is set in gyro filter path)
}

void lsm9ds1_idle_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag)
{
    //set the power to sleep for both acc/gyr and mag sensors
    int32_t ret = 0;
    ret = lsm9ds1_imu_data_rate_set(lsm9ds1_imu, LSM9DS1_IMU_OFF);
    ret = lsm9ds1_mag_data_rate_set(lsm9ds1_mag, LSM9DS1_MAG_POWER_DOWN);
}

void lsm9ds1_active_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag)
{
    //Applies all of the settings stored in the settings array to the LSM9DS1
    for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) lsm9ds1_apply_setting(lsm9ds1_imu, lsm9ds1_mag, i);
}

void lsm9ds1_apply_setting(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, sensor_settings_t setting)
{
    //the lsm9ds1 driver has different methods for applying different settings. This function looks at the 
    //setting that we wish to apply and calls the appropriate method in the driver.

    int32_t err_code;
    switch (setting)
    {
        case (FS_RANGE + ACC_START):
            err_code = lsm9ds1_xl_full_scale_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (FS_RANGE + GYR_START):
            err_code = lsm9ds1_gy_full_scale_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (FS_RANGE + MAG_START):
            err_code = lsm9ds1_mag_full_scale_set(lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        case (ODR + GYR_START):
            err_code = lsm9ds1_imu_data_rate_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (ODR + MAG_START):
            err_code = lsm9ds1_mag_data_rate_set(lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        case (FILTER_SELECTION + ACC_START):
            err_code = lsm9ds1_xl_filter_out_path_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (LOW_PASS_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_lp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (HIGH_PASS_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_hp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (EXTRA_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_aalias_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (FILTER_SELECTION + GYR_START):
            err_code = lsm9ds1_gy_filter_out_path_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (LOW_PASS_FILTER + GYR_START):
            err_code = lsm9ds1_gy_filter_lp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (HIGH_PASS_FILTER + GYR_START):
            err_code = lsm9ds1_gy_filter_hp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        default:
            break; //don't do anything
    }
}

static int32_t lsm9ds1_read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    //the primary buffer only holds the address of the register we want to read and therefore only has a length of 1 byte
    //the secondary buffer is where the data will get read into
    const nrf_drv_twi_xfer_desc_t lsm9ds1_imu_read = {
        .address = (LSM9DS1_IMU_I2C_ADD_H >> 1),
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    *p_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(p_twi, &lsm9ds1_imu_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*p_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t lsm9ds1_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    uint8_t register_and_data[2] = {reg, bufp[0]};

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t lsm9ds1_imu_write = {
        .address = (LSM9DS1_IMU_I2C_ADD_H >> 1),
        .primary_length = 2,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    *p_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(p_twi, &lsm9ds1_imu_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*p_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t lsm9ds1_read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    //the logic for handling multiple byte reads must be handled in software for the magnetometer, whereas, the accelerometer
    //and gyroscope can do it via hardware
    const nrf_drv_twi_xfer_desc_t lsm9ds1_mag_read = {
        .address = (((uint8_t)lsm9ds1_register_auto_increment << 7) | (LSM9DS1_MAG_I2C_ADD_H >> 1)),
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    *p_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(p_twi, &lsm9ds1_mag_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*p_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t lsm9ds1_write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    uint8_t register_and_data[2] = {reg, bufp[0]};

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t lsm9ds1_mag_write = {
        .address = (LSM9DS1_MAG_I2C_ADD_H >> 1),
        .primary_length = 2,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    *p_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(p_twi, &lsm9ds1_mag_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*p_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}