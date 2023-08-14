#ifndef LSM9DS1_H__
#define LSM9DS1_H__

#include <stdint.h>
#include <stdbool.h>
#include "lsm9ds1_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

void lsm9ds1_active_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, uint8_t* sensor_settings);

#ifdef __cplusplus
}
#endif

#endif // LSM9DS1_H__

/** @} */