#include "lsm9ds1.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

//set up pointers to TWI instance for chip communication
const nrf_drv_twi_t* p_twi;
volatile bool* p_xfer_done;

//set up settings variables
static imu_communication_t* imu_comm;

static uint8_t* p_sensor_settings;
static uint8_t IMU_Address;
static uint8_t MAG_Address;

static stmdev_ctx_t lsm9ds1_imu;                                          /**< LSM9DS1 accelerometer/gyroscope instance. */
static stmdev_ctx_t lsm9ds1_mag;                                          /**< LSM9DS1 magnetometer instance. */

static lsm9ds1_id_t whoamI;

static bool lsm9ds1_register_auto_increment = true;                       /**register auto increment function for multiple byte reads of LSM9DS1 chip **/

void lsm9ds1_init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings,
                  const nrf_drv_twi_t* twi, volatile bool* xfer_done, bool external_board)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip
    imu_comm = comm;

    //initialize read/write methods, addresses, and default settings for acc + gyro
    if (sensors & 0b110)
    {
        lsm9ds1_imu.read_reg = lsm9ds1_read_imu;
        lsm9ds1_imu.write_reg = lsm9ds1_write_imu;
        IMU_Address = comm->acc_comm.address;

        //Apply default settings for the acc and gyro
        update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, LSM9DS1_4g); //accelerometer full scale range (+/- 4 g)
        update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, LSM9DS1_2000dps); //gyroscope full scale range (+/- 2000 degrees/s)

        update_sensor_setting(p_sensor_settings + ACC_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
        update_sensor_setting(p_sensor_settings + GYR_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
        update_sensor_setting(p_sensor_settings + ACC_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
        update_sensor_setting(p_sensor_settings + GYR_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)

        update_sensor_setting(p_sensor_settings + ACC_START, FILTER_SELECTION, LSM9DS1_LP_OUT); //accelerometer filter selection (low pass filter only)
        update_sensor_setting(p_sensor_settings + ACC_START, LOW_PASS_FILTER, LSM9DS1_LP_DISABLE); //accelerometer low pass filter setting (frequency is automatically tied to ODR)
        update_sensor_setting(p_sensor_settings + ACC_START, HIGH_PASS_FILTER, 0); //accelerometer high pass filter setting (only takes effect when HP_OUT is set for accelerometer)
        update_sensor_setting(p_sensor_settings + ACC_START, EXTRA_FILTER, LSM9DS1_50Hz); //accelerometer anti-aliasing bandwidth (50 Hz bandwidth)

        update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, LSM9DS1_LPF1_HPF_OUT); //Gyroscope filter selection (low pass filter 1 and HPF)
        update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, 0); //gyroscope low pass filter setting (only takes effect when LPF2 is set in gyro filter path)
        update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, LSM9DS1_HP_MEDIUM); //gyroscope high pass filter setting (at an ODR of 59.5 Hz this correlates to 0.5 Hz cut-off)

        //After setting default settings, attempt to read the whoAmI register
        uint32_t ret = lsm9ds1_dev_id_get(NULL, &lsm9ds1_imu, &whoamI);
        if (whoamI.imu == LSM9DS1_IMU_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Acc + Gyro discovered.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't find LSM9DS1 Acc + Gyro.\n");
    }

    //initialize read/write methods, address, and default settings for mag
    if (sensors & 0b001)
    {
        lsm9ds1_mag.read_reg = lsm9ds1_read_mag;
        lsm9ds1_mag.write_reg = lsm9ds1_write_mag;
        MAG_Address = comm->mag_comm.address;

        //Apply default settings for the mag
        update_sensor_setting(p_sensor_settings + MAG_START, FS_RANGE, LSM9DS1_4Ga); //magnetometer full scale range (+/- 4 Gauss)

        update_sensor_setting(p_sensor_settings + MAG_START, ODR, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)
        update_sensor_setting(p_sensor_settings + MAG_START, POWER, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)

        //After setting default settings, attempt to read the whoAmI register
        uint32_t ret = lsm9ds1_dev_id_get(&lsm9ds1_mag, NULL, &whoamI);
        if (whoamI.mag == LSM9DS1_MAG_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Mag discovered.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't find LSM9DS1 Mag.\n");
    }
}

int32_t lsm9ds1_idle_mode_enable(uint8_t sensors)
{
    //Set the sensors in the sensor variable to sleep (0b100 = acc, 0b010 = gyr, 0b001 = mag)
    int32_t ret = 0;
    if (sensors & 0b110) ret = lsm9ds1_imu_data_rate_set(&lsm9ds1_imu, LSM9DS1_IMU_OFF);
    if (sensors & 0b001) ret = lsm9ds1_mag_data_rate_set(&lsm9ds1_mag, LSM9DS1_MAG_POWER_DOWN);

    return ret;
}

int32_t lsm9ds1_active_mode_enable(uint8_t sensors)
{
    //Applies all of the settings stored in the settings array to the LSM9DS1
    //SEGGER_RTT_WriteString(0, "lsm9ds1 activated with the following settings:\n[");
    int32_t ret = 0;
    for (int i = 0; i < SENSOR_SETTINGS_LENGTH / 3; i++)
    {
        if (sensors & 0b100) ret = lsm9ds1_acc_apply_setting(i + ACC_START);
        if (sensors & 0b010) ret = lsm9ds1_gyr_apply_setting(i + GYR_START);
        if (sensors & 0b001) ret = lsm9ds1_mag_apply_setting(i + MAG_START);
        //SEGGER_RTT_printf(0, "0x%x ", p_sensor_settings[i]);
    }
    //SEGGER_RTT_WriteString(0, "\n\n");
    return ret;
}

int32_t lsm9ds1_acc_apply_setting(uint8_t setting)
{
    //the lsm9ds1 driver has different methods for applying different settings. This function looks at the 
    //setting that we wish to apply that's currently saved in the settings array and calls the appropriate
    //method in the driver.

    int32_t err_code;
    switch (setting)
    {
        case (FS_RANGE + ACC_START):
            err_code = lsm9ds1_xl_full_scale_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (ODR + ACC_START):
            err_code = lsm9ds1_imu_data_rate_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (FILTER_SELECTION + ACC_START):
            err_code = lsm9ds1_xl_filter_out_path_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (LOW_PASS_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_lp_bandwidth_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (HIGH_PASS_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_hp_bandwidth_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (EXTRA_FILTER + ACC_START):
            err_code = lsm9ds1_xl_filter_aalias_bandwidth_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        default:
            err_code = 0; //no methods were called so set the error code to success
            break;
    }

    if (err_code != 0)
    {
        SEGGER_RTT_WriteString(0, "There was an issue trying to start lsm9ds1 with current settings.\n");
    }

    return err_code;
}

int32_t lsm9ds1_gyr_apply_setting(uint8_t setting)
{
    //the lsm9ds1 driver has different methods for applying different settings. This function looks at the 
    //setting that we wish to apply that's currently saved in the settings array and calls the appropriate
    //method in the driver.

    int32_t err_code;
    switch (setting)
    {
        case (FS_RANGE + GYR_START):
            err_code = lsm9ds1_gy_full_scale_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (ODR + GYR_START):
            err_code = lsm9ds1_imu_data_rate_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (FILTER_SELECTION + GYR_START):
            err_code = lsm9ds1_gy_filter_out_path_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (LOW_PASS_FILTER + GYR_START):
            err_code = lsm9ds1_gy_filter_lp_bandwidth_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        case (HIGH_PASS_FILTER + GYR_START):
            err_code = lsm9ds1_gy_filter_hp_bandwidth_set(&lsm9ds1_imu, p_sensor_settings[setting]);
            break;
        default:
            err_code = 0; //no methods were called so set the error code to success
            break;
    }

    if (err_code != 0)
    {
        SEGGER_RTT_WriteString(0, "There was an issue trying to start lsm9ds1 with current settings.\n");
    }

    return err_code;
}

int32_t lsm9ds1_mag_apply_setting(uint8_t setting)
{
    ///the lsm9ds1 driver has different methods for applying different settings. This function looks at the 
    //setting that we wish to apply that's currently saved in the settings array and calls the appropriate
    //method in the driver.

    int32_t err_code;
    switch (setting)
    {
        case (FS_RANGE + MAG_START):
            err_code = lsm9ds1_mag_full_scale_set(&lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        case (ODR + MAG_START):
            err_code = lsm9ds1_mag_data_rate_set(&lsm9ds1_mag, p_sensor_settings[setting]);
            break;
        default:
            err_code = 0; //no methods were called so set the error code to success
            break;
    }

    if (err_code != 0)
    {
        SEGGER_RTT_WriteString(0, "There was an issue trying to start lsm9ds1 with current settings.\n");
    }

    return err_code;
}

static int32_t lsm9ds1_read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    //The lsm9ds1 driver methods for updating registers rely on a struct called stmdev_ctx_t
    //which essentially just holds methods for reading and writing. In order to take advantage
    //of the driver methods, this method acts as a passthrough, converting their reading method
    //to my own. I also don't allow for the splitting of the lsm9ds1 acc and gyro (since the 
    //the gyro can't be run without the acc on) which means they have the same read/write methods
    //so we only call the acc method here.
    imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus, IMU_Address, reg, bufp, len);
}
static int32_t lsm9ds1_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    //The lsm9ds1 driver methods for updating registers rely on a struct called stmdev_ctx_t
    //which essentially just holds methods for reading and writing. In order to take advantage
    //of the driver methods, this method acts as a passthrough, converting their writing method
    //to my own. I only allow for writing a single byte at a time so the len variable gets dropped.
    //I also don't allow for the splitting of the lsm9ds1 acc and gyro (since the 
    //the gyro can't be run without the acc on) which means they have the same read/write methods
    //so we only call the acc method here.
    imu_comm->acc_comm.write_register((void*)imu_comm->acc_comm.twi_bus, IMU_Address, reg, bufp);
}
static int32_t lsm9ds1_read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    //The lsm9ds1 driver methods for updating registers rely on a struct called stmdev_ctx_t
    //which essentially just holds methods for reading and writing. In order to take advantage
    //of the driver methods, this method acts as a passthrough, converting their reading method
    //to my own.
    imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus, MAG_Address, reg, bufp, len);
}
static int32_t lsm9ds1_write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    //The lsm9ds1 driver methods for updating registers rely on a struct called stmdev_ctx_t
    //which essentially just holds methods for reading and writing. In order to take advantage
    //of the driver methods, this method acts as a passthrough, converting their writing method
    //to my own. I only allow for writing a single byte at a time so the len variable gets dropped.
    imu_comm->mag_comm.write_register((void*)imu_comm->mag_comm.twi_bus, MAG_Address, reg, bufp);
}