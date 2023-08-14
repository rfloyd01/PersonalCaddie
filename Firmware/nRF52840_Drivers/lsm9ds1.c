#include "lsm9ds1.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

//set up pointers to TWI instance for chip communication
const nrf_drv_twi_t* p_twi;
volatile bool* p_xfer_done;

//set up settings variables
static uint8_t* p_sensor_settings;
static uint8_t sensor_settings_length;

static bool lsm9ds1_register_auto_increment = true;                       /**register auto increment function for multiple byte reads of LSM9DS1 chip **/

void lsm9ds1_init(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, uint8_t* settings, const uint8_t settings_length, 
                  const nrf_drv_twi_t* twi, volatile bool* xfer_done)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;
    sensor_settings_length = settings_length;

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
    lsm9ds1_update_setting(LSM9DS1_ACC_MODEL, LSM9DS1_ACC);
    lsm9ds1_update_setting(LSM9DS1_GYR_MODEL, LSM9DS1_GYR);
    lsm9ds1_update_setting(LSM9DS1_MAG_MODEL, LSM9DS1_MAG);

    //Populate the sensor settings array with default settings for the sensor
    lsm9ds1_update_setting(LSM9DS1_ACC_FS_RANGE, LSM9DS1_4g); //accelerometer full scale range (+/- 4 g)
    lsm9ds1_update_setting(LSM9DS1_GYR_FS_RANGE, LSM9DS1_2000dps); //gyroscope full scale range (+/- 2000 degrees/s)
    lsm9ds1_update_setting(LSM9DS1_MAG_FS_RANGE, LSM9DS1_4Ga); //magnetometer full scale range (+/- 4 Gauss)

    lsm9ds1_update_setting(LSM9DS1_ACC_GYR_ODR_POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
    lsm9ds1_update_setting(LSM9DS1_MAG_ODR_POWER, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)

    lsm9ds1_update_setting(LSM9DS1_ACC_FILTER_SELECTION, LSM9DS1_LP_OUT); //accelerometer filter selection (low pass filter only)
    lsm9ds1_update_setting(LSM9DS1_ACC_LOW_PASS_FILTER, LSM9DS1_LP_DISABLE); //accelerometer low pass filter setting (frequency is automatically tied to ODR)
    lsm9ds1_update_setting(LSM9DS1_ACC_HIGH_PASS_FILTER, 0); //accelerometer high pass filter setting (only takes effect when HP_OUT is set for accelerometer)
    lsm9ds1_update_setting(LSM9DS1_ACC_ANTI_ALIASING_FILTER, LSM9DS1_AUTO); //accelerometer anti-aliasing bandwidth (automatically set based on current ODR)

    lsm9ds1_update_setting(LSM9DS1_GYR_FILTER_SELECTION, LSM9DS1_LPF1_OUT); //Gyroscope filter selection (low pass filter 1 only)
    lsm9ds1_update_setting(LSM9DS1_GYR_LOW_PASS_FILTER, 0); //gyroscope low pass filter setting (only takes effect when LPF2 is set in gyro filter path)
    lsm9ds1_update_setting(LSM9DS1_GYR_HIGH_PASS_FILTER, 0); //gyroscope high pass filter setting (only takes effect when HPF is set in gyro filter path)
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
    for (int i = 0; i < sensor_settings_length; i++) lsm9ds1_apply_setting(lsm9ds1_imu, lsm9ds1_mag, i);
}

void lsm9ds1_update_setting(lsm9ds1_settings_t setting, uint8_t value)
{
    //Updates the desired setting in the settings array with the given value. Only update the array 
    //if an actual setting is given.
    switch (setting)
    {
        case LSM9DS1_ACC_FS_RANGE:
        case LSM9DS1_GYR_FS_RANGE:
        case LSM9DS1_MAG_FS_RANGE:
        case LSM9DS1_ACC_GYR_ODR_POWER:
        case LSM9DS1_MAG_ODR_POWER:
        case LSM9DS1_ACC_FILTER_SELECTION:
        case LSM9DS1_ACC_LOW_PASS_FILTER:
        case LSM9DS1_ACC_HIGH_PASS_FILTER:
        case LSM9DS1_ACC_ANTI_ALIASING_FILTER:
        case LSM9DS1_GYR_FILTER_SELECTION:
        case LSM9DS1_GYR_LOW_PASS_FILTER:
        case LSM9DS1_GYR_HIGH_PASS_FILTER:
            p_sensor_settings[setting] = value;
            break;
        default:
            break; //don't do anything
    }
    
}

void lsm9ds1_apply_setting(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, lsm9ds1_settings_t setting)
{
    //the lsm9ds1 driver has different methods for applying different settings. This function looks at the 
    //setting that we wish to apply and calls the appropriate method in the driver.
    int32_t err_code;
    switch (setting)
    {
        case LSM9DS1_ACC_FS_RANGE:
            err_code = lsm9ds1_xl_full_scale_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_GYR_FS_RANGE:
            err_code = lsm9ds1_gy_full_scale_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_MAG_FS_RANGE:
            err_code = lsm9ds1_mag_full_scale_set(lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        case LSM9DS1_ACC_GYR_ODR_POWER:
            err_code = lsm9ds1_imu_data_rate_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_MAG_ODR_POWER:
            err_code = lsm9ds1_mag_data_rate_set(lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        case LSM9DS1_ACC_FILTER_SELECTION:
            err_code = lsm9ds1_xl_filter_out_path_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_ACC_LOW_PASS_FILTER:
            err_code = lsm9ds1_xl_filter_lp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_ACC_HIGH_PASS_FILTER:
            err_code = lsm9ds1_xl_filter_hp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_ACC_ANTI_ALIASING_FILTER:
            err_code = lsm9ds1_xl_filter_aalias_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_GYR_FILTER_SELECTION:
            err_code = lsm9ds1_gy_filter_out_path_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_GYR_LOW_PASS_FILTER:
            err_code = lsm9ds1_gy_filter_lp_bandwidth_set(lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case LSM9DS1_GYR_HIGH_PASS_FILTER:
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