#ifndef LSM9DS1_H__
#define LSM9DS1_H__

#include <stdint.h>
#include <stdbool.h>
#include "lsm9ds1_reg.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#ifdef __cplusplus
extern "C" {
#else

void lsm9ds1_init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t lsm9ds1_idle_mode_enable(uint8_t sensors);
int32_t lsm9ds1_active_mode_enable(uint8_t sensors);

int32_t lsm9ds1_acc_apply_setting(uint8_t setting);
int32_t lsm9ds1_gyr_apply_setting(uint8_t setting);
int32_t lsm9ds1_mag_apply_setting(uint8_t setting);

//Register reading and writing methods
static int32_t lsm9ds1_read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm9ds1_write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // LSM9DS1_H__

/** @} */