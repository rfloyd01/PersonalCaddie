#include "lsm9ds1.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

void lsm9ds1_active_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, uint8_t* sensor_settings)
{
    //looks at the settings in the sensor_settings array and uses the TWI bus to apply them to the LSM9DS1.
    //This function will only work if the sensor has been placed in sensor_active mode
    int32_t ret = 0;

    //set the full scale ranges
    ret = lsm9ds1_xl_full_scale_set(lsm9ds1_imu, sensor_settings[1]);
    ret = lsm9ds1_gy_full_scale_set(lsm9ds1_imu, sensor_settings[2]);
    ret = lsm9ds1_mag_full_scale_set(lsm9ds1_mag, sensor_settings[3]);

    //set the odr and power modes
    ret = lsm9ds1_imu_data_rate_set(lsm9ds1_imu, sensor_settings[4]);
    ret = lsm9ds1_mag_data_rate_set(lsm9ds1_mag, sensor_settings[5]);

    //set the filter settings
    ret = lsm9ds1_gy_filter_out_path_set(lsm9ds1_imu, sensor_settings[6]);
    ret = lsm9ds1_gy_filter_lp_bandwidth_set(lsm9ds1_imu, sensor_settings[7]);
    ret = lsm9ds1_gy_filter_hp_bandwidth_set(lsm9ds1_imu, sensor_settings[8]);
    ret = lsm9ds1_xl_filter_out_path_set(lsm9ds1_imu, sensor_settings[9]);
    ret = lsm9ds1_xl_filter_lp_bandwidth_set(lsm9ds1_imu, sensor_settings[10]);
    ret = lsm9ds1_xl_filter_hp_bandwidth_set(lsm9ds1_imu, sensor_settings[11]);
    ret = lsm9ds1_xl_filter_aalias_bandwidth_set(lsm9ds1_imu, sensor_settings[12]);
}