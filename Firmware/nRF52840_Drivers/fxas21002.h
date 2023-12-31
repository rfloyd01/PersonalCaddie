#ifndef FXAS21002_H__
#define FXAS21002_H__

#include <stdint.h>
#include <stdbool.h>
#include "NXP/fxas21002/fxas21002_driver.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#define FXAS21002_STANDBY_TO_ACTIVE_DELAY_US 61000
#define FXAS21002_READY_TO_ACTIVE_DELAY_US 6000

#ifdef __cplusplus
extern "C" {
#else

void fxas21002init(imu_communication_t* comm, uint8_t* settings);
#endif

int32_t fxas21002_connected_mode_enable();
int32_t fxas21002_idle_mode_enable();
int32_t fxas21002_active_mode_enable(int current_mode);

void fxas21002_get_actual_settings();

int32_t fxas21002_get_gyr_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // FXAS21002_H__

/** @} */