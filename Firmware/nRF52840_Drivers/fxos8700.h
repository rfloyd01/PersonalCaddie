#ifndef FXOS8700_H__
#define FXOS8700_H__

#include <stdint.h>
#include <stdbool.h>
#include "NXP/fxos8700/src/fxos8700_driver.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#ifdef __cplusplus
extern "C" {
#else

void fxos8700init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t fxos8700_idle_mode_enable();
int32_t fxos8700_active_mode_enable();

int32_t fxos8700_acc_apply_setting(uint8_t setting);
int32_t fxos8700_mag_apply_setting(uint8_t setting);

//Register reading and writing methods
//static int32_t fxos8700_read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
//static int32_t fxos8700_write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
//static int32_t fxos8700_read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
//static int32_t fxos8700_write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);

//TODO: There are separate functions for getting data from the acc and the mag, I should look into 
//using a single function for when both acc and mag are active.
int32_t fxos8700_get_acc_data(uint8_t* pBuff, uint8_t offset);
int32_t fxos8700_get_mag_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // FXOS8700_H__

/** @} */