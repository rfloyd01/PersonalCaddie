#ifndef LSM9DS1_H__
#define LSM9DS1_H__

#include <stdint.h>
#include <stdbool.h>
#include "lsm9ds1_reg.h"
#include "sensor_settings.h"

#ifdef __cplusplus
extern "C" {
#else
#include "nrf_drv_twi.h" //don't want to include this on the C++ side
#endif

void lsm9ds1_init(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag,  uint8_t* settings, const uint8_t settings_length, 
                  const nrf_drv_twi_t* twi, volatile bool* xfer_done);

void lsm9ds1_idle_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag);
void lsm9ds1_active_mode_enable(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag);
void lsm9ds1_update_setting(lsm9ds1_settings_t setting, uint8_t value);
void lsm9ds1_apply_setting(stmdev_ctx_t* lsm9ds1_imu, stmdev_ctx_t* lsm9ds1_mag, lsm9ds1_settings_t setting);

//Register reading and writing methods
static int32_t lsm9ds1_read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);

float lsm9ds1_odr_calculate(uint8_t imu_odr_setting, uint8_t mag_odr_setting);

#ifdef __cplusplus
}
#endif

#endif // LSM9DS1_H__

/** @} */