#ifndef FXAS21002_H__
#define FXAS21002_H__

#include <stdint.h>
#include <stdbool.h>
#include "Bosch/bmi270.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#ifdef __cplusplus
extern "C" {
#else

void bmi270init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t bmi270_idle_mode_enable();
int32_t bmi270_active_mode_enable();

void bmi270_get_actual_settings();

int32_t bmi270_get_acc_data(uint8_t* pBuff, uint8_t offset);
int32_t bmi270_get_gyr_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // FXAS21002_H__

/** @} */