#include "lsm9ds1.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "personal_caddie_operating_modes.h"

//set up settings variables
static imu_communication_t* imu_comm;

static uint8_t* p_sensor_settings;
static uint8_t IMU_Address;
static uint8_t MAG_Address;

static stmdev_ctx_t lsm9ds1_imu;                                          /**< LSM9DS1 accelerometer/gyroscope instance. */
static stmdev_ctx_t lsm9ds1_mag;                                          /**< LSM9DS1 magnetometer instance. */

static lsm9ds1_id_t whoamI;

static bool lsm9ds1_register_auto_increment = true;                       /**register auto increment function for multiple byte reads of LSM9DS1 chip **/

void lsm9ds1_init(imu_communication_t* comm, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip
    imu_comm = comm;

    //initialize read/write methods, addresses, and default settings for acc
    if (imu_comm->sensor_model[ACC_SENSOR] == LSM9DS1_ACC)
    {
        lsm9ds1_imu.read_reg = lsm9ds1_read_imu;
        lsm9ds1_imu.write_reg = lsm9ds1_write_imu;
        IMU_Address = comm->acc_comm.address;

        //Apply default settings for the acc
        update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, LSM9DS1_4g); //accelerometer full scale range (+/- 4 g)

        update_sensor_setting(p_sensor_settings + ACC_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
        update_sensor_setting(p_sensor_settings + ACC_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)

        update_sensor_setting(p_sensor_settings + ACC_START, FILTER_SELECTION, LSM9DS1_LP_OUT); //accelerometer filter selection (low pass filter only)
        update_sensor_setting(p_sensor_settings + ACC_START, LOW_PASS_FILTER, LSM9DS1_LP_DISABLE); //accelerometer low pass filter setting (frequency is automatically tied to ODR)
        update_sensor_setting(p_sensor_settings + ACC_START, HIGH_PASS_FILTER, 0); //accelerometer high pass filter setting (only takes effect when HP_OUT is set for accelerometer)
        update_sensor_setting(p_sensor_settings + ACC_START, EXTRA_FILTER, LSM9DS1_50Hz); //accelerometer anti-aliasing bandwidth (50 Hz bandwidth)

        //After setting default settings, attempt to read the whoAmI register
        uint32_t ret = lsm9ds1_dev_id_get(NULL, &lsm9ds1_imu, &whoamI);
        if (whoamI.imu == LSM9DS1_IMU_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Acc initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize LSM9DS1 Acc.\n");
    }

    //initialize read/write methods, addresses, and default settings for gyro
    if (imu_comm->sensor_model[GYR_SENSOR] == LSM9DS1_GYR)
    {
        lsm9ds1_imu.read_reg = lsm9ds1_read_imu;
        lsm9ds1_imu.write_reg = lsm9ds1_write_imu;
        IMU_Address = comm->gyr_comm.address;

        //Apply default settings for the acc and gyro
        update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, LSM9DS1_2000dps); //gyroscope full scale range (+/- 2000 degrees/s)

        update_sensor_setting(p_sensor_settings + GYR_START, ODR, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)
        update_sensor_setting(p_sensor_settings + GYR_START, POWER, LSM9DS1_IMU_59Hz5); //accelerometer/gyroscope ODR and Power (59.5 Hz, gyroscope in standard power mode)

        update_sensor_setting(p_sensor_settings + GYR_START, FILTER_SELECTION, LSM9DS1_LPF1_OUT); //Gyroscope filter selection (low pass filter 1 and HPF)
        update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, 0); //gyroscope low pass filter setting (only takes effect when LPF2 is set in gyro filter path)
        update_sensor_setting(p_sensor_settings + GYR_START, HIGH_PASS_FILTER, LSM9DS1_HP_MEDIUM); //gyroscope high pass filter setting (at an ODR of 59.5 Hz this correlates to 0.5 Hz cut-off)

        //After setting default settings, attempt to read the whoAmI register
        uint32_t ret = lsm9ds1_dev_id_get(NULL, &lsm9ds1_imu, &whoamI);
        if (whoamI.imu == LSM9DS1_IMU_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Gyro initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize LSM9DS1 Gyro.\n");
    }

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[MAG_SENSOR] == LSM9DS1_MAG)
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
        if (whoamI.mag == LSM9DS1_MAG_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Mag initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize LSM9DS1 Mag.\n");
    }
}

int32_t lsm9ds1_connected_mode_enable()
{
    //In connected mode the LSM9DS1 is placed into its "Power Down" mode. The expected current
    //draw for this mode isn't given in the data sheet (maybe I should measure it with the 
    //power profiler)
    int32_t ret = 0;
    if (imu_comm->sensor_model[ACC_SENSOR] == LSM9DS1_ACC || imu_comm->sensor_model[GYR_SENSOR] == LSM9DS1_GYR) ret = lsm9ds1_imu_data_rate_set(&lsm9ds1_imu, LSM9DS1_IMU_OFF);
    if (imu_comm->sensor_model[MAG_SENSOR] == LSM9DS1_MAG) ret = lsm9ds1_mag_data_rate_set(&lsm9ds1_mag, LSM9DS1_MAG_POWER_DOWN);

    return ret;
}

int32_t lsm9ds1_idle_mode_enable(int current_mode)
{
    //Unlike other sensors, the LSM9DS1 doesn't have an intermediate power mode. If we would
    //normally be going into sensor idle mode from connected mode, we instead just call the 
    //active mode enable method. If we'd noramlly be entering sensor idle mode from sensor
    //active mode we don't do anything here.
    if (imu_comm->sensor_model[ACC_SENSOR] != LSM9DS1_ACC && imu_comm->sensor_model[GYR_SENSOR] != LSM9DS1_GYR &&
        imu_comm->sensor_model[MAG_SENSOR] != LSM9DS1_MAG) return 0; //only carry out this method if an LSM9DS1 sensor is active

    if (current_mode == CONNECTED_MODE) return lsm9ds1_active_mode_enable();
    else return 0;
}

int32_t lsm9ds1_active_mode_enable()
{
    //Applies all of the settings stored in the settings array to the LSM9DS1
    int32_t ret = 0;
    for (int i = 0; i < SENSOR_SETTINGS_LENGTH / 3; i++)
    {
        if (imu_comm->sensor_model[ACC_SENSOR] == LSM9DS1_ACC) ret |= lsm9ds1_acc_apply_setting(i + ACC_START);
        if (imu_comm->sensor_model[GYR_SENSOR] == LSM9DS1_GYR) ret |= lsm9ds1_gyr_apply_setting(i + GYR_START);
        if (imu_comm->sensor_model[MAG_SENSOR] == LSM9DS1_MAG) ret |= lsm9ds1_mag_apply_setting(i + MAG_START);
    }

    //The datasheet doesn't mention how much time we should wait before actually
    //taking readings so to be safe just wait for 10 milliseconds.
    imu_comm->acc_comm.delay(10000); //can use acc, gyr or mag here, it shouldn't matter

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
    //to my own. It's possible for this method to be called by both the accelerometer and the 
    //gyroscope, and since we can have an acc and gyro that are on different chips, we need to 
    //figure out which one actually called this method.

    if (imu_comm->sensor_model[ACC_SENSOR] == LSM9DS1_ACC) imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus, IMU_Address, reg, bufp, len);
    else if (imu_comm->sensor_model[GYR_SENSOR] == LSM9DS1_GYR) imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, IMU_Address, reg, bufp, len);
}
static int32_t lsm9ds1_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    //The lsm9ds1 driver methods for updating registers rely on a struct called stmdev_ctx_t
    //which essentially just holds methods for reading and writing. In order to take advantage
    //of the driver methods, this method acts as a passthrough, converting their writing method
    //to my own. I only allow for writing a single byte at a time so the len variable gets dropped.
    //It's possible for this method to be called by both the accelerometer and the 
    //gyroscope, and since we can have an acc and gyro that are on different chips, we need to 
    //figure out which one actually called this method.
    if (imu_comm->sensor_model[ACC_SENSOR] == LSM9DS1_ACC) imu_comm->acc_comm.write_register((void*)imu_comm->acc_comm.twi_bus, IMU_Address, reg, bufp, 1);
    else if (imu_comm->sensor_model[GYR_SENSOR] == LSM9DS1_GYR) imu_comm->gyr_comm.write_register((void*)imu_comm->gyr_comm.twi_bus, IMU_Address, reg, bufp, 1);
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
    imu_comm->mag_comm.write_register((void*)imu_comm->mag_comm.twi_bus, MAG_Address, reg, bufp, 1);
}

int32_t lsm9ds1_get_acc_data(uint8_t* pBuff, uint8_t offset)
{
    int32_t ret = imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus, imu_comm->acc_comm.address, LSM9DS1_OUT_X_L_XL, pBuff + offset, 6);
    return ret;
}

int32_t lsm9ds1_get_gyr_data(uint8_t* pBuff, uint8_t offset)
{
    int32_t ret = imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus, imu_comm->gyr_comm.address, LSM9DS1_OUT_X_L_G, pBuff + offset, 6);
    return ret;
}

int32_t lsm9ds1_get_mag_data(uint8_t* pBuff, uint8_t offset)
{
    //The mag address needs a bit added to the front to allow continuous reading
    uint8_t auto_inc_address = ((lsm9ds1_register_auto_increment << 7) | imu_comm->mag_comm.address);
    int32_t ret = imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus, auto_inc_address, LSM9DS1_OUT_X_L_M, pBuff + offset, 6);
}