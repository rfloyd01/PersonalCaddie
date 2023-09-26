#ifndef FXOS8700_H__
#define FXOS8700_H__

#include <stdint.h>
#include <stdbool.h>
#include "NXP/fxas21002/fxas21002_driver.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#ifdef __cplusplus
extern "C" {
#else

void fxas21002init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t fxas21002_idle_mode_enable();
int32_t fxas21002_active_mode_enable();

int32_t fxas21002_get_gyr_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // FXOS8700_H__

/** @} */