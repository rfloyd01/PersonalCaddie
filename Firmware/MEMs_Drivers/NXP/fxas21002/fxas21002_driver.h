#ifndef FXAS21002_DRIVER_H_
#define FXAS21002_DRIVER_H_

#include "fxas21002_regdef.h"
#include "sensor_communication.h"

int32_t fxas21002_data_rate_set(sensor_communication_t* gyr_comm, fxas21002_odr_t val);
int32_t fxas21002_data_rate_get(sensor_communication_t* gyr_comm, fxas21002_odr_t* val);

int32_t fxas21002_power_mode_set(sensor_communication_t* gyr_comm, fxas21002_power_t val);
int32_t fxas21002_power_mode_get(sensor_communication_t* gyr_comm, fxas21002_power_t* val);

int32_t fxas21002_full_scale_range_set(sensor_communication_t* gyr_comm, fxas21002_fs_range_t val);
int32_t fxas21002_full_scale_range_get(sensor_communication_t* gyr_comm, fxas21002_fs_range_t* val);

int32_t fxas21002_filter_out_set(sensor_communication_t* gyr_comm, fxas21002_filter_select_t val);
int32_t fxas21002_filter_out_get(sensor_communication_t* gyr_comm, fxas21002_filter_select_t* val);

int32_t fxas21002_lp_filter_bw_set(sensor_communication_t* gyr_comm, fxas21002_lpf_bandwidth_t val);
int32_t fxas21002_lp_filter_bw_get(sensor_communication_t* gyr_comm, fxas21002_lpf_bandwidth_t* val);

int32_t fxas21002_hp_filter_bw_set(sensor_communication_t* gyr_comm, fxas21002_hpf_bandwidth_t val);
int32_t fxas21002_hp_filter_bw_get(sensor_communication_t* gyr_comm, fxas21002_hpf_bandwidth_t* val);

#endif /* FXAS21002_DRIVER_H_ */
